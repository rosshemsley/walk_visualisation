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

MainWindow::MainWindow()
{
    // This is where we draw items to.
    scene = new QGraphicsScene();
    view  = new QGraphicsView(scene);
  
    // Define extend of the viewport.
    view->setSceneRect(-500,-500,1000,1000);
        
    // The central widget in the mainwindow is the graphics view.
    setCentralWidget(view);  

    // Create and draw a random triangulation to the graphics view.
    randomTriangulation(*scene);
}

/*****************************************************************************/

// Use this to draw a triangle specified by a face.
void MainWindow::drawTriangle(Face_handle f, QGraphicsScene &scene)
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
    }
}

/*****************************************************************************/

// Do a straight walk between points p and q, and then draw the result.
void MainWindow::straightWalk(Point p, Point q)
{
    // Create a circulator describing the walk.
    Line_face_circulator lfc = dt.line_walk (p,q), done(lfc);
    
    // Draw all the triangles in the circulator.
    do 
    {    
        drawTriangle(lfc, *scene);
        ++lfc;
    } while (lfc != done);   
}

/*****************************************************************************/

void MainWindow::randomTriangulation(QGraphicsScene &scene)
{   
    // Generate a random pointset to triangulate.
    CGAL::Random_points_in_square_2<Point,Creator> g(500.);
    CGAL::copy_n( g, 100, std::back_inserter(dt) );

    // Create a triangulation and add to the scene
    QTriangulationGraphics *tgi = new QTriangulationGraphics(&dt);
    tgi->setVerticesPen(QPen(Qt::red, 10, Qt::SolidLine, 
                                          Qt::RoundCap, 
                                          Qt::RoundJoin ));
    scene.addItem(tgi);

    
    dt.locate(Point(100,200));

}

/*****************************************************************************/



