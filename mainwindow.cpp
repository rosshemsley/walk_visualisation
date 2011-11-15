#include <iostream>
#include <fstream>
#include <boost/format.hpp>

#include <QtGui>
#include <QMainWindow>
#include <QLineF>
#include <QRectF>
#include <QGraphicsPolygonItem>

#include <CGAL/Qt/Converter.h>
#include <CGAL/Qt/GraphicsViewNavigation.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_2.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Qt/TriangulationGraphicsItem.h>
#include <CGAL/point_generators_2.h>

#include "mainwindow.h"

/*****************************************************************************/

void MainWindow::newWalk()
{    
    // We are going to start taking point inputs.
    // This says that we are currently learning point 1.
    inputPoints = 0;
    
    // Set the mouse to a crosshair when moving over the grahpics view.
    view->setCursor(Qt::CrossCursor);   
    
    // Clear any old walk graphics from the scene.
    updateScene();    
}

/*****************************************************************************/

void MainWindow::updateScene()
{
    // Remove all the previous walkItems, that is, the previous triangles
    // drawn as part of the walk.
    while (! walkItems.isEmpty() )
        scene->removeItem(walkItems.takeFirst());

    // If we have enough data to plot a walk, then do so.
    if (inputPoints > 0) 
        straightWalk(c(points[0]), c(points[1]));    

    for (int i=0; i<inputPoints; i++)
    {
        QPen   pen(Qt::blue);
        QBrush brush(Qt::blue);
        
        QPoint p = points[i].toPoint();
        walkItems.append(scene->addEllipse(QRect(p, QSize(10,10)),pen,brush));
    }
}

/*****************************************************************************/

MainWindow::MainWindow()
{
    // The default state is to not take new input points.
    inputPoints = -1;

    // This is where we draw items to.
    scene = new QGraphicsScene();
    view  = new QGraphicsView(scene);

    // Event filters for dealing with mouse input.
    // These are attached to both the GrahpicsView and GraphicsScene.
    scene->installEventFilter(this);    
    view->installEventFilter(this);
    
    // This allows us to receive events when the mouse moves.
    view->setMouseTracking(true); 
      
    // Define extent of the viewport.
    view->setSceneRect(-400,-400,800,800);
   
    // Container widget for the layout.
    QWidget *widget = new QWidget;
    setCentralWidget(widget);

    // Upper and lower filler widgets.
    QWidget *topFiller = new QWidget;
    topFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    QWidget *bottomFiller = new QWidget;
    bottomFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);    
        
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(5);
    layout->addWidget(topFiller);
    layout->addWidget(view);
    layout->addWidget(bottomFiller);
    widget->setLayout(layout);    
        
    createActions();
    createMenus();

    QString message = tr("A context menu is available by right-clicking");
    statusBar()->showMessage(message);    

    // Create and draw a random triangulation to the graphics view.
    randomTriangulation();    
}

/*****************************************************************************/

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == scene)
    {
        QGraphicsSceneMouseEvent* mouseEvent;
        
        switch(event->type())
        {            
            // ** MOUSE BUTTON RELEASE ** //
            case QEvent::GraphicsSceneMouseRelease:
            {
                mouseEvent   = static_cast<QGraphicsSceneMouseEvent*>(event);
                QPointF pos = mouseEvent->scenePos();

                if (mouseEvent->button() == Qt::LeftButton)
                {
                    if (inputPoints == 0 || inputPoints == 1)
                        points[inputPoints++] = pos;
                    
                    if (inputPoints == 2)
                    {                        
                        // We've finished adding points now.
                        view->setCursor(Qt::ArrowCursor);                         
                        updateScene();
                    }   
                }
                return true;
            }             


            // ** MOUSE MOVED ** //            
            case QEvent::GraphicsSceneMouseMove:
            {
                if (inputPoints == 1)
                {
                    mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);            
                    points[1] = mouseEvent->scenePos();
                    updateScene();            
                }
                return true;
            }            
        }
    }
    
    // pass the event on to the parent class
    return QMainWindow::eventFilter(obj, event);
}

/*****************************************************************************/

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
}

/*****************************************************************************/

void MainWindow::createActions()
{
    newAct = new QAction(tr("&New Walk"), this);
    newAct->setStatusTip(tr("Create a new Walk"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newWalk()));
}

/*****************************************************************************/

// Use this to draw a triangle specified by a face.
QGraphicsPolygonItem* MainWindow::drawTriangle(Face_handle f)
{
    
    // Ignore infinite faces.
    if (! dt.is_infinite( f ) ) 
    {
        // Convert a face into a polygon for plotting.
        QGraphicsPolygonItem *polygonItem;
        QVector<QPointF>      polygon;    
            
        polygon << c(f->vertex(0)->point()) 
                << c(f->vertex(1)->point()) 
                << c(f->vertex(2)->point());
                
        polygonItem = new QGraphicsPolygonItem(QPolygonF(polygon));
    
        // The "look" of the triangle.
        polygonItem->setPen( QPen(Qt::darkGreen) );
        polygonItem->setBrush( QColor("#D2CAEB") );    
    
        return polygonItem;
    }
    
    return 0;
}

/*****************************************************************************/

// Do a straight walk between points p and q, and then draw the result.
void MainWindow::straightWalk(Point p, Point q)
{
    // Create a circulator describing the walk.
    Line_face_circulator lfc = dt.line_walk (p,q), done(lfc);
    
    // Ignore empty walks.
    if (lfc==0) return;
    
    do 
    {
        QGraphicsPolygonItem *tr = drawTriangle(lfc);
        if (tr!=0)
        {
            walkItems  << tr;
            scene->addItem(tr);
        }
        
        ++lfc;
    } while (lfc != done);   
}

/*****************************************************************************/

void MainWindow::randomTriangulation()
{   
    // Generate a random pointset to triangulate.
    CGAL::Random_points_in_square_2<Point,Creator> g(400.);
    CGAL::copy_n( g, 100, std::back_inserter(dt) );

    // Create a triangulation and add to the scene
    tgi = new QTriangulationGraphics(&dt);
    tgi->setVerticesPen(QPen(Qt::red, 5 , Qt::SolidLine, 
                                          Qt::RoundCap, 
                                          Qt::RoundJoin ));
    scene->addItem(tgi);

    view->setSceneRect(tgi->boundingRect());
    view->fitInView(tgi->boundingRect(), Qt::KeepAspectRatio);
    
    dt.locate(Point(100,200));

}

/*****************************************************************************/



