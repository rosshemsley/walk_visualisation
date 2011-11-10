#include <iostream>
#include <fstream>
#include <boost/format.hpp>

#include <QtGui>
#include <QGraphicsPolygonItem>


#include <CGAL/Qt/Converter.h>



#include <CGAL/Qt/GraphicsViewNavigation.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_2.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Qt/TriangulationGraphicsItem.h>
#include <CGAL/point_generators_2.h>

#include <QLineF>
#include <QRectF>

struct K : CGAL::Exact_predicates_inexact_constructions_kernel {};

typedef CGAL::Triangulation_2<K>                         Triangulation;
typedef CGAL::Delaunay_triangulation_2<K>                Delaunay;
typedef Triangulation::Point                             Point;
typedef Triangulation::Face                              Face;
typedef Face::Face_handle                                Face_handle;

typedef CGAL::Creator_uniform_2<double,Point>            Creator;
typedef CGAL::Qt::TriangulationGraphicsItem<Delaunay>    QTriangulationGraphics;


void drawTriangle(Delaunay::Face_handle &f, QGraphicsScene &scene)
{
    QVector<QPointF> polygon;
    
    CGAL::Qt::Converter<K> c;

    
    polygon << c(f->vertex(0)->point()) << c(f->vertex(1)->point()) << c(f->vertex(2)->point());
    

    QGraphicsPolygonItem *polygonItem = new QGraphicsPolygonItem(QPolygonF(polygon), 0, &scene );
    polygonItem->setPen( QPen(Qt::darkGreen) );
    polygonItem->setBrush( Qt::yellow );    
}

int main(int argc, char **argv)
{
	    
    CGAL::Random_points_in_square_2<Point,Creator> g(500.);


	Delaunay dt;

    CGAL::copy_n( g, 100, std::back_inserter(dt));

   // Delaunay::Vertex_handle v = dt.nearest_vertex ( Point(2,3) );


    Delaunay::Face_handle fh = dt.finite_faces_begin();
    

	
    QTriangulationGraphics *tgi = new QTriangulationGraphics(&dt);
    
    QApplication app(argc, argv);

    QGraphicsScene scene;

    tgi->setVerticesPen(QPen(Qt::red, 10, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    scene.addItem(tgi);

    


    drawTriangle(fh, scene);








    scene.setSceneRect (-250, -250, 1000, 1000);
    


    
    QGraphicsView* view = new QGraphicsView(&scene);
  

    CGAL::Qt::GraphicsViewNavigation navigation;

    view->installEventFilter(&navigation);
    view->viewport()->installEventFilter(&navigation);
    view->setRenderHint(QPainter::Antialiasing);



    view->show();
   

 return app.exec();
}
