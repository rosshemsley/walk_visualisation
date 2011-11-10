#include <iostream>
#include <fstream>
#include <boost/format.hpp>

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


#include "mainwindow.h"


MainWindow::MainWindow()
{
    
    scene = new QGraphicsScene();

    view  = new QGraphicsView(scene);
  
    CGAL::Qt::GraphicsViewNavigation navigation;
    view->installEventFilter(&navigation);
    view->viewport()->installEventFilter(&navigation);
    view->setRenderHint(QPainter::Antialiasing);

    
    setCentralWidget(view);
    

}


void MainWindow::drawTriangle(Delaunay::Face_handle &f, QGraphicsScene &scene)
{
    QVector<QPointF> polygon;
    
    polygon << c(f->vertex(0)->point()) << c(f->vertex(1)->point()) << c(f->vertex(2)->point());
    
    QGraphicsPolygonItem *polygonItem = new QGraphicsPolygonItem(QPolygonF(polygon), 0, &scene );
    polygonItem->setPen( QPen(Qt::darkGreen) );
    polygonItem->setBrush( Qt::yellow );    
}

void MainWindow::randomTriangulation(QGraphicsScene &scene)
{
    CGAL::Random_points_in_square_2<Point,Creator> g(50.);

    CGAL::copy_n( g, 100, std::back_inserter(dt));

    QTriangulationGraphics *tgi = new QTriangulationGraphics(&dt);

    tgi->setVerticesPen(QPen(Qt::red, 10, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));


    scene.addItem(tgi);


    Delaunay::Face_handle fh = dt.finite_faces_begin();    
    
      drawTriangle(fh, scene);
    // Delaunay::Vertex_handle v = dt.nearest_vertex ( Point(2,3) );
    
}





