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
    
    
    
    qDebug() << "Clicked ";
}

/*****************************************************************************/

void MainWindow::updateScene()
{
    qDebug() << "Updating scene";
    

    while (!walk.isEmpty())
        scene->removeItem(walk.takeFirst());

  //  scene->addItem(tgi);

    if (inputPoints==1) 
    {
          straightWalk(c(points[0]), c(points[1]));    
          qDebug() << "Drawing walk";
    }
}

/*****************************************************************************/

MainWindow::MainWindow()
{
    inputPoints = -1;

    // This is where we draw items to.
    scene = new QGraphicsScene();
    view  = new QGraphicsView(scene);
  
  
    scene->installEventFilter(this);
    
    view->installEventFilter(this);
    view->setMouseTracking(true); 
    
  
    // Define extend of the viewport.
    view->setSceneRect(-400,-400,800,800);
   
    QWidget *widget = new QWidget;
    setCentralWidget(widget);

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
    
    
    //view->setCursor(Qt::CrossCursor);
    
    createActions();
    createMenus();

    QString message = tr("A context menu is available by right-clicking");
    statusBar()->showMessage(message);    

    // Create and draw a random triangulation to the graphics view.
    randomTriangulation(*scene);
    
    
}

/*****************************************************************************/

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    
    qDebug() << "input points:" << inputPoints << endl;
    if (obj == scene)
    {
        switch(event->type())
        {
            
            case QEvent::GraphicsSceneMouseRelease:
            {
                QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);

                if (mouseEvent->button() == Qt::LeftButton)
                {
                    // If we are adding points to the scene
                    if (inputPoints >=0)
                    {
                        if (inputPoints==2) 
                        {
                            view->setCursor(Qt::ArrowCursor);                         
                            straightWalk(c(points[0]), c(points[1]));
                            inputPoints = -1;                     
                        } else {
                            points[inputPoints] = mouseEvent->scenePos();       
                            
                            inputPoints ++;
                        }                        
                    }
                    qDebug() << "Ate key press";
                }
                return true;
                break;
            }
            
            case QEvent::GraphicsSceneMouseMove:
            {
                QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
                
                points[1]           = mouseEvent->scenePos();
                
                qDebug() << "mouse move";
                if (inputPoints>=0)
                    updateScene();            
                return true;
                break;
            }
            
            default:
                // pass the event on to the parent class
                return QMainWindow::eventFilter(obj, event);  
        }
    } else {
        // pass the event on to the parent class
        return QMainWindow::eventFilter(obj, event);  
    }
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
    newAct = new QAction(tr("&New"), this);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newWalk()));
}

/*****************************************************************************/

// Use this to draw a triangle specified by a face.
QGraphicsPolygonItem* MainWindow::drawTriangle(Face_handle f, QGraphicsScene &scene)
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
                
        polygonItem = new QGraphicsPolygonItem(QPolygonF(polygon), 0, &scene);
    
        // The "look" of the triangle.
        polygonItem->setPen( QPen(Qt::darkGreen) );
        polygonItem->setBrush( Qt::yellow );    
    
        return polygonItem;
    }
}

/*****************************************************************************/

// Do a straight walk between points p and q, and then draw the result.
void MainWindow::straightWalk(Point p, Point q)
{
    // Create a circulator describing the walk.
    Line_face_circulator lfc = dt.line_walk (p,q), done(lfc);
    
   // walk = scene->createItemGroup(list);
    
    // Draw all the triangles in the circulator.
    do 
    {
        if (lfc==0) break;
        walk << drawTriangle(lfc, *scene);
        ++lfc;
    } while (lfc != done);   
}

/*****************************************************************************/

void MainWindow::randomTriangulation(QGraphicsScene &scene)
{   
    // Generate a random pointset to triangulate.
    CGAL::Random_points_in_square_2<Point,Creator> g(400.);
    CGAL::copy_n( g, 100, std::back_inserter(dt) );

    // Create a triangulation and add to the scene
    tgi = new QTriangulationGraphics(&dt);
    tgi->setVerticesPen(QPen(Qt::red, 10, Qt::SolidLine, 
                                          Qt::RoundCap, 
                                          Qt::RoundJoin ));
    scene.addItem(tgi);

    
    view->fitInView(tgi->boundingRect(), Qt::KeepAspectRatio);  
    
    
    dt.locate(Point(100,200));

}

/*****************************************************************************/



