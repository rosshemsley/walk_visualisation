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
    
    // Small trick to stop any faces being displayed before any mouse over events
    // occur.
    points[0] = QPoint(-1000,-1000);
    
    // Clear any old walk graphics from the scene.
    updateScene();    
}

/*****************************************************************************/

void MainWindow::straightWalk_checkbox_change(int state)
{
    drawStraightWalk = state;
    updateScene();
}

/*****************************************************************************/

void MainWindow::visibilityWalk_checkbox_change(int state)
{
    drawVisibilityWalk = state;    
    updateScene();
}

/*****************************************************************************/

void MainWindow::pivotWalk_checkbox_change(int state)
{
    drawPivotWalk = state;    
    updateScene();
}

/*****************************************************************************/

void MainWindow::updateScene()
{
        
    // Style for points.
    QPen   pen(Qt::black);
    QBrush brush(Qt::blue);
    
    QString details;
        
    // Remove all the previous walkItems, that is, the previous triangles
    // drawn as part of the walk.
    while (! walkItems.isEmpty() )
        scene->removeItem(walkItems.takeFirst());
                
    if (inputPoints >= 0)
    {
        // Find the face we are hovering over.
        Face_handle f = dt->locate(c(points[0]));
        
        // Check the face is finite, and then draw it.
        if (!dt->is_infinite(f))
        {        
            QGraphicsItem *tr = Walk<Delaunay>::drawTriangle(f,QPen(),QColor("#D2D2D2"));
            scene->addItem(tr);
            walkItems.append(tr);
        }
    }        
        
    // If we have enough data to plot a walk, then do so.
    if (inputPoints > 0) 
    {
        Face_handle f = dt->locate(c(points[0]));
        Face_handle g = dt->locate(c(points[1]));
        
        if ( !dt->is_infinite(f) && !dt->is_infinite(g) )
        {                
            if (drawStraightWalk)
            {
                StraightWalk<Delaunay> w(c(points[1]), dt, f);
                QGraphicsItem* walkGraphics = w.getGraphics(QPen(),QColor("#EBEBD2"));
                walkItems.append(walkGraphics);
                scene->addItem(walkGraphics);     

                details += "<b>Straight Walk</b><br>";                
                details += "Orientations: ";
                details += QString::number(w.getNumOrientationsPerformed());
                details += "<br>Triangles Visited: ";
                details += QString::number(w.getNumTrianglesVisited());
                details += "<br><br>";
                
            }

            if (drawVisibilityWalk)
            {
                VisibilityWalk<Delaunay> w(c(points[1]), dt, f);
                QGraphicsItem* walkGraphics = w.getGraphics(QPen(),
                                                            QColor("#D2D2EB"));
                walkItems.append(walkGraphics);
                scene->addItem(walkGraphics);   

                details += "<b>Visibility Walk</b><br>";                
                details += "Orientations: ";
                details += QString::number(w.getNumOrientationsPerformed());
                details += "<br>Triangles Visited: ";
                details += QString::number(w.getNumTrianglesVisited());
                details += "<br><br>";

            }   
    
            if (drawPivotWalk)
            {
                PivotWalk<Delaunay> w(c(points[1]), dt, f);
                QGraphicsItem* walkGraphics = w.getGraphics(QPen(),
                                                            QColor("#EBD2D2"));
                walkItems.append(walkGraphics);
                scene->addItem(walkGraphics);       
                         
                details += "<b>Pivot Walk</b><br>";        
                details += "Orientations: ";
                details += QString::number(w.getNumOrientationsPerformed());
                details += "<br>Triangles Visited: ";
                details += QString::number(w.getNumTrianglesVisited());
                details += "<br><br>";
                                         
            }                                     
        }     
    }
    
        
    if (inputPoints == 2)
    {
        QPoint p = points[1];
        walkItems.append(scene->addEllipse(QRect(p, QSize(10,10)),pen,brush));        
    }
    
    status->setText(details);
    
    

}

/*****************************************************************************/

MainWindow::MainWindow()
{
    
    dt  = new Delaunay();
    tgi = new QTriangulationGraphics(dt);
    
    tgi->setVerticesPen(QPen(Qt::red, 5 , Qt::SolidLine, 
                                          Qt::RoundCap, 
                                          Qt::RoundJoin ));

    
    
    
    // The default state is to not take new input points.
    inputPoints = -1;

    // This is where we draw items to.
    scene = new QGraphicsScene();
    view  = new QGraphicsView(scene);

    // Event filters for dealing with mouse input.
    // These are attached to both the GrahpicsView and GraphicsScene.
    scene->installEventFilter(this);    
    view->installEventFilter(this);
    
    scene->addItem(tgi);    
    
    view->setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
    view->setVerticalScrollBarPolicy   ( Qt::ScrollBarAlwaysOff );
    
    // This allows us to receive events when the mouse moves.
    view->setMouseTracking(true); 
      
    // Define extent of the viewport.
    view->setSceneRect(-400,-400,800,800);
   
    view->setRenderHint(QPainter::Antialiasing);
   
    QGroupBox    *groupBox            = new QGroupBox(tr("Walk Types"));
    QCheckBox    *checkBox_visibility = new QCheckBox(tr("Visibility Walk"));
    QCheckBox    *checkBox_pivot      = new QCheckBox(tr("Pivot Walk"));
    QCheckBox    *checkBox_straight   = new QCheckBox(tr("Straight Walk"));
    QPushButton  *button_new_walk     = new QPushButton(tr("New Walk"));
    QPushButton  *button_new_pointset = new QPushButton(tr("New Pointset"));

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget( button_new_walk     );
    hbox->addWidget( button_new_pointset );
    hbox->addWidget( checkBox_visibility );
    hbox->addWidget( checkBox_straight   );
    hbox->addWidget( checkBox_pivot      );    
    groupBox->setLayout(hbox);

    connect(button_new_walk,        SIGNAL(clicked()),
            this,                   SLOT(newWalk()                    ));

    connect(button_new_pointset,    SIGNAL(clicked()),
            this,                   SLOT(newPointset()                ));
    
    connect(checkBox_pivot,     SIGNAL(stateChanged(int)), 
            this,               SLOT(pivotWalk_checkbox_change(int)   ));
            
    connect(checkBox_straight,  SIGNAL(stateChanged(int)), 
            this,               SLOT(straightWalk_checkbox_change(int) ));
            
    connect(checkBox_visibility,SIGNAL(stateChanged(int)), 
            this,               SLOT(visibilityWalk_checkbox_change(int)));
   
   
    drawStraightWalk   = FALSE;
    drawVisibilityWalk = FALSE;
    drawPivotWalk      = FALSE;
    
   
    // Container widget for the layout.
    QWidget *widget = new QWidget;
    setCentralWidget(widget);

    // Upper and lower filler widgets.
    QWidget *topFiller = new QWidget;
    topFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    status = new QLabel;
    status->setAlignment(Qt::AlignTop);
    status->setTextFormat(Qt::RichText);
    //status->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);    
    
        
    QGridLayout* layout = new QGridLayout;
        
    layout->setMargin(5);        
    layout->addWidget(groupBox,0,0,1,2);
    layout->addWidget(view,    1,0);
    layout->addWidget(status,  1,1);    
    layout->setColumnMinimumWidth(1,150);        
    widget->setLayout(layout);    
        
    createActions();
    createMenus();

    QString message = tr("Select the walks to draw and then click New Walk.");
    statusBar()->showMessage(message);    

    dialog_newPointset = new PointGeneratorDialog();

    connect(dialog_newPointset, SIGNAL(valueChanged(int)), this, SLOT(randomTriangulation(int)));


    // Create and draw a random triangulation to the graphics view.
    randomTriangulation(100);    
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
            
            default:
                return QMainWindow::eventFilter(obj, event);
        }
    }
    
    // pass the event on to the parent class
    return QMainWindow::eventFilter(obj, event);
}

/*****************************************************************************/

void MainWindow::newPointset()
{     
    dialog_newPointset->show();
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

void MainWindow::randomTriangulation(int points)
{   
    dt->clear();

    // Generate a random pointset to triangulate.
    CGAL::Random_points_in_square_2<Point,Creator> g(400.);
    CGAL::copy_n( g, points, std::back_inserter(*dt) );

    emit tgi->modelChanged();
   
    // Clear old walk.
    inputPoints=-1;
    updateScene();

    view->setSceneRect(tgi->boundingRect());
    view->fitInView(tgi->boundingRect(), Qt::KeepAspectRatio);
    
    
}

/*****************************************************************************/

