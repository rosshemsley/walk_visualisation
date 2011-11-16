/******************************************************************************
* Written by Ross Hemsley for INRIA.fr. 
* A simple application visualise different walks on Delaunay Triangulations.
******************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/*****************************************************************************/

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

/*****************************************************************************/

struct K : CGAL::Exact_predicates_inexact_constructions_kernel {};

/*****************************************************************************/

typedef CGAL::Triangulation_2<K>                        Triangulation;
typedef CGAL::Delaunay_triangulation_2<K>               Delaunay;
typedef Triangulation::Point                            Point;
typedef Delaunay::Face                                  Face;
typedef Delaunay::Line_face_circulator                  Line_face_circulator;
typedef Face::Face_handle                               Face_handle;
typedef CGAL::Creator_uniform_2<double,Point>           Creator;
typedef CGAL::Qt::TriangulationGraphicsItem<Delaunay>   QTriangulationGraphics;

/*****************************************************************************/

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
                                    MainWindow();

protected:
    bool                            eventFilter(QObject *obj, QEvent *event);    
    void                            resizeEvent (QResizeEvent * event);

private slots:
    void                            newWalk();    
    void                            updateScene();

private:
    void                            createMenus();
    void                            createActions();    
    QGraphicsPolygonItem*           drawTriangle(Face_handle f);
    void                            randomTriangulation();
    void                            straightWalk(Point p, Point q);
    
    
    QMenu*                          fileMenu;
    QAction*                        newAct;        
    QGraphicsView*                  view;
    QGraphicsScene*                 scene;    
    Delaunay                        dt;
    CGAL::Qt::Converter<K>          c;
    QTriangulationGraphics*         tgi; 
    QList<QGraphicsItem*>           walkItems;
     
    // When we are taking points as input we use the following.
    // if inputPoints < 0 we are not learning points..
    // if it is positive we are learning points[inputPoints]
    int                             inputPoints;
    QPointF                         points[2];

}; 

/*****************************************************************************/

#endif

/*****************************************************************************/