/******************************************************************************
* Written by Ross Hemsley for INRIA.fr. 
* This is a class heirachy to perform different walks on triangulations,
* providing methods to create a QGraphicsItem so that they can be drawn
* directly to a QGraphicsScene.
*
* We also consider the number of triangles and properties of each walk.
*
******************************************************************************/

#ifndef WALK_H
#define WALK_H

/*****************************************************************************/

#include <CGAL/Qt/Converter.h>
#include <CGAL/Qt/GraphicsViewNavigation.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_2.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Qt/TriangulationGraphicsItem.h>
#include <CGAL/point_generators_2.h>
#include <boost/format.hpp>

#include <QList>

/*****************************************************************************/

/******************************************************************************
* Abstract class to contain different walking strategies
******************************************************************************/

template <typename T>
class Walk
{
    typedef typename T::Face                            Face;
    typedef typename T::Face_handle                     Face_handle;
    typedef typename T::Line_face_circulator            Lfc;
    typedef typename T::Geom_traits                     Gt;
    
public:
    // Create a graphics item for drawing this triangulation.
    QGraphicsItem*                  getGraphics();    
    int                             getNumTrianglesVisited();
    
    // Static helper function to draw 2D faces to QgrahpicsItems.
    static QGraphicsPolygonItem*    drawTriangle(Face_handle f);
    
protected:
    // Pointer to the triangluation this walk is on.
    T*                              dt;
    // List of faces this walk intersects.
    QList<Face_handle>              faces;
    
    // This allows subclasses to add faces to the current walk. 
    // Doing this enables the base-class functions to work.
    void                            addToWalk(Face_handle f);
};

/******************************************************************************
* Straight walk strategy
******************************************************************************/

template <typename T>
class StraightWalk : public Walk<T> 
{
    typedef typename T::Face                            Face;
    typedef typename T::Point                           Point;    
    typedef typename T::Face_handle                     Face_handle;
    typedef typename T::Line_face_circulator            Lfc;
    typedef typename T::Geom_traits                     Gt;
    
public:    
    StraightWalk(Point p, T* dt, Face_handle f=Face_handle());  
};

/******************************************************************************
* Visibility walk strategy
******************************************************************************/

template <typename T>
class VisibilityWalk : public Walk<T>
{
    typedef typename T::Face                            Face;
    typedef typename T::Point                           Point;    
    typedef typename T::Face_handle                     Face_handle;
    typedef typename T::Geom_traits                     Gt;    
    
public:    
    VisibilityWalk(Point p, T* dt, Face_handle f=Face_handle());
};
/*****************************************************************************/  



/******************************************************************************
*
* Implementations
*
******************************************************************************/



/******************************************************************************
* Straight Walk
******************************************************************************/  
  
// Perform a straight walk, storing triangles visited in base class.
template <typename T>
StraightWalk<T>::StraightWalk(Point p, T* dt, Face_handle f)
{
    // Store a reference to the triangulation.
    this->dt = dt;
        
    // Create a circulator describing the walk.
    CGAL::Qt::Converter<Gt> c;
 
    if (f==Face_handle())
        f=dt->infinite_face();
 
    Point x = f->vertex(0)->point();
    
    // Use CGAL built-in line walk.
    Lfc lfc = dt->line_walk (x,p), done(lfc);     
  
    // Take all the items from the circulator and add them to a list.
    if (lfc == 0) return;
    
    do {
        Face_handle f = lfc;
        addToWalk(f);        
    } while (++lfc != done);          
}

/******************************************************************************
* Visibility Walk
******************************************************************************/

// Perform a visibility walk, storing triangles visited in base class.
template <typename T>
VisibilityWalk<T>::VisibilityWalk(Point p, T* dt, Face_handle f)
{
    
    // The user did not provide a face handle. So just use the infinite face.
    if (f==Face_handle())
        f=dt->infinite_face();
        
    // This is where we store the current face.
    Face_handle c = f;    
    
    // Create a binary random number generator.
    boost::rand48 rng;
    boost::uniform_smallint<> two(0, 1);
    boost::variate_generator<boost::rand48&, boost::uniform_smallint<> > coin(rng, two);    
  
    // Loop until we find our destination point.
    while(1)
    {
        if (dt->is_infinite(c)) break;
        
        int left_first   = coin() % 2;

        const Point & p0 = c->vertex( 0 )->point();
        const Point & p1 = c->vertex( 1 )->point();
        const Point & p2 = c->vertex( 2 )->point();
        
        CGAL::Orientation o0, o1, o2;
        
    	o0 = orientation(p0,p1,p);    	    	
    	if ( o0 == CGAL::NEGATIVE ) {  
            c = c->neighbor(2);            
            addToWalk(c);
            continue;
	    }
	    
	    	    
    	o0 = orientation(p2,p0,p);    	    	
    	if ( o0 == CGAL::NEGATIVE ) {  
            c = c->neighbor(1);            
            addToWalk(c);
            continue;
	    }
	    
	    	    
    	o0 = orientation(p1,p2,p);    	    	
    	if ( o0 == CGAL::NEGATIVE ) {  
            c = c->neighbor(0);
            addToWalk(c);
            continue;
	    }
	    
	    // We are done.
        break;
  
    }

    
} 

/******************************************************************************
* Walk base-class functions
*
* These provide basic functionality that is common to all of the walk types
* so that we do not have to re-implement this functionality multiple times.
*
******************************************************************************/

template <typename T>  
void Walk<T>::addToWalk(Face_handle f)
{
    faces.append(f);
}

/*****************************************************************************/  

// Return the number of faces visited in this walk.
template <typename T>  
int Walk<T>::getNumTrianglesVisited()
{
    return faces.size();
}

/*****************************************************************************/  

// Create a graphics item representing this walk.
template <typename T>  
QGraphicsItem* Walk<T>::getGraphics()
{
    // This GraphicsItem Group will store the triangles from the walk.
    QGraphicsItemGroup* g = new QGraphicsItemGroup();
    
    // Iterate over faces in this walk.
    typename QList<typename T::Face_handle>::const_iterator i;
    for (i = faces.begin(); i != faces.end(); ++i)
    {
        // Draw this triangle in the walk.
        if (! dt->is_infinite( *i ) ) 
        {
            QGraphicsPolygonItem *tr = drawTriangle(*i);         
            g->addToGroup(tr);        
        }
    }

    return g;
}

/*****************************************************************************/  

// Helper-function to create a triangle graphics item.
// Note that this is publically accessible and static.
template <typename T>
QGraphicsPolygonItem* Walk<T>::drawTriangle(Face_handle f)
{
    // Helper to convert between different point types.
    CGAL::Qt::Converter<Gt> c;

    // We store this triangle as a polygonItem.
    QGraphicsPolygonItem *polygonItem = 0;

    // Convert a face into a polygon for plotting.
    QVector<QPointF>      polygon;    

    polygon << c(f->vertex(0)->point()) 
            << c(f->vertex(1)->point()) 
            << c(f->vertex(2)->point());

    polygonItem = new QGraphicsPolygonItem(QPolygonF(polygon));

    // The "look" of the triangle.
    polygonItem->setPen( QPen(Qt::darkGreen) );
    polygonItem->setBrush( QColor("#D2D2EB") );            
    
    return polygonItem;
}

/*****************************************************************************/

#endif

/*****************************************************************************/