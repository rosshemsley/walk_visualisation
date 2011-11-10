#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>

#include <QMainWindow>

#include <CGAL/Qt/Converter.h>
#include <CGAL/Qt/GraphicsViewNavigation.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_2.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Qt/TriangulationGraphicsItem.h>
#include <CGAL/point_generators_2.h>



#include <QLineF>
#include <QRectF>
#include <QGraphicsPolygonItem>


struct K : CGAL::Exact_predicates_inexact_constructions_kernel {};

typedef CGAL::Triangulation_2<K>                         Triangulation;
typedef CGAL::Delaunay_triangulation_2<K>                Delaunay;
typedef Triangulation::Point                             Point;
typedef Triangulation::Face                              Face;
typedef Face::Face_handle                                Face_handle;
typedef CGAL::Creator_uniform_2<double,Point>            Creator;
typedef CGAL::Qt::TriangulationGraphicsItem<Delaunay>    QTriangulationGraphics;



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();


private:
    void drawTriangle(Delaunay::Face_handle &f, QGraphicsScene &scene);
    void randomTriangulation(QGraphicsScene &scene);
    
    
    QGraphicsView*            view;
    QGraphicsScene*          scene;    
    Delaunay                    dt;
    CGAL::Qt::Converter<K>       c;
    

}; 


#endif
