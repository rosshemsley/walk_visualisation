/******************************************************************************
* Written by Ross Hemsley for INRIA.fr. 
* A simple application visualise different walks on Delaunay Triangulations.
******************************************************************************/

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
#include "walk.h"


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
    // Style for points.
    QPen   pen(Qt::blue);
    QBrush brush(Qt::blue);
    
    // Remove all the previous walkItems, that is, the previous triangles
    // drawn as part of the walk.
    while (! walkItems.isEmpty() )
        scene->removeItem(walkItems.takeFirst());
        
    // If we have enough data to plot a walk, then do so.
    if (inputPoints > 0) 
    {
        Face_handle f = dt->locate(c(points[0]));
        if (!dt->is_infinite(f))
        {        
            StraightWalk<Delaunay> w(c(points[1]), dt, f);
            QGraphicsItem* walkGraphics = w.getGraphics();
        
            walkItems.append(walkGraphics);
            scene->addItem(walkGraphics);        
        }
    }

    if (inputPoints == 0)
    {
        // Find the face we are hovering over.
        Face_handle f = dt->locate(c(points[0]));
        
        // Check the face is finite, and then draw it.
        if (!dt->is_infinite(f))
        {        
            QGraphicsItem *tr = Walk<Delaunay>::drawTriangle(f);
            scene->addItem(tr);
            walkItems.append(tr);
        }        
    }
    
    if (inputPoints >= 1)
    {
        QPoint p = points[1];
        walkItems.append(scene->addEllipse(QRect(p, QSize(10,10)),pen,brush));        
    }

}

/*****************************************************************************/

MainWindow::MainWindow()
{
    
    dt=new Delaunay();
    
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
   
    view->setRenderHint(QPainter::Antialiasing);
   
   
    // Container widget for the layout.
    QWidget *widget = new QWidget;
    setCentralWidget(widget);

    // Upper and lower filler widgets.
    QWidget *topFiller = new QWidget;
    topFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    QWidget *bottomFiller = new QWidget;
    bottomFiller->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);    
        
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(5);
   // layout->addWidget(topFiller);
    layout->addWidget(view);
//    layout->addWidget(bottomFiller);
    widget->setLayout(layout);    
        
    createActions();
    createMenus();

    QString message = tr("A context menu is available by right-clicking");
    statusBar()->showMessage(message);    

    // Create and draw a random triangulation to the graphics view.
    randomTriangulation();    
}

/*****************************************************************************/

void MainWindow::resizeEvent (QResizeEvent * event)
{    
    view->fitInView(tgi->boundingRect(), Qt::KeepAspectRatio);    
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
                QPoint pos = mouseEvent->scenePos().toPoint();

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
                mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);                            
                QPoint pos = mouseEvent->scenePos().toPoint();
                
                if (inputPoints == 0)
                 {

                     points[0] = pos;
                     updateScene();
                 }
                
                if (inputPoints == 1)
                {
                    points[1] = pos;
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

void MainWindow::randomTriangulation()
{   

    // Generate a random pointset to triangulate.
    CGAL::Random_points_in_square_2<Point,Creator> g(400.);
    CGAL::copy_n( g, 100, std::back_inserter(*dt) );

    // Create a triangulation and add to the scene
    tgi = new QTriangulationGraphics(dt);
    tgi->setVerticesPen(QPen(Qt::red, 5 , Qt::SolidLine, 
                                          Qt::RoundCap, 
                                          Qt::RoundJoin ));
    scene->addItem(tgi);

    view->setSceneRect(tgi->boundingRect());
    view->fitInView(tgi->boundingRect(), Qt::KeepAspectRatio);
    
}

/*****************************************************************************/

