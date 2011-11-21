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
                                    Walk();
    QGraphicsItemGroup*             getGraphics();    
    int                             getNumTrianglesVisited();
    int                             getNumOrientationsPerformed();
    
    // Static helper function to draw 2D faces to QgrahpicsItems.
    static QGraphicsPolygonItem*    drawTriangle(Face_handle f,
                                                 QPen        pen   = QPen(), 
                                                 QBrush      brush = QBrush());
    
protected:
    // Pointer to the triangluation this walk is on.
    T*                              dt;
    
    // This allows subclasses to add faces to the current walk. 
    // Doing this enables the base-class functions to work.
    void                            addToWalk(Face_handle f);
    
    // A quick and dirty way to count orientations.
    void                            incOrientationCount();
    
    
private:
    // List of faces this walk intersects.
    QList<Face_handle>              faces;
    
    int o_count;
    
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
    StraightWalk(Point p, T* dt, Face_handle f=Face_handle())
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
        if (lfc != 0)
        {    
            do {
                Face_handle f = lfc;
                addToWalk(f);        
            } while (++lfc != done);          
        }
    }
};

/******************************************************************************
* Pivot Walk strategy
******************************************************************************/

template <typename T>
class PivotWalk : public Walk<T>
{
    typedef typename T::Face                            Face;
    typedef typename T::Point                           Point;    
    typedef typename T::Face_handle                     Face_handle;
    typedef typename T::Geom_traits                     Gt;    

private: 
    // We store the pivot points for this walk so we can draw them later.
    QList<Point> pivots;
    
public:    
    
    /*************************************************************************/
        
    PivotWalk(Point p, T* dt, Face_handle f=Face_handle())
    {
                
        this->dt = dt;

        // The user did not provide a face handle. So just use the infinite face.
        if (f==Face_handle())
            f=dt->infinite_face();

        // This is where we store the current face.
        Face_handle c    = f;    
        Face_handle prev = c;        


        // **     FIND FIRST FACE      ** //
        for (int i=0; i<3; i++)
        {
            const Point & p0 = c->vertex(i)->point();
            const Point & p1 = c->vertex(c->cw(i))->point();
            
            this->incOrientationCount();    
                        
            // If we have found a face that can see the point.
            if ( CGAL::orientation(p0,p1,p) == CGAL::POSITIVE )
            {
                c = c->neighbor(c->ccw(i));
                break;
            }
        }
        // ** END OF FIND FIRST FACE ** //

        addToWalk(c);
        
        
        // We swap direction every time we change pivot.
        bool clockwise = TRUE;

        while(1)
        {
            
            // We need to reverse handedness of orientation tests depending on
            // the directino we are going.   
            // NOTE. we can avoid several comparisons by unrolling the 
            // direction changing logic.         
            CGAL::Orientation direction;
            if (clockwise)
                direction = CGAL::NEGATIVE;
            else
                direction = CGAL::POSITIVE;            
            
            // Find the index of the previous face relative to us.
            // ** NOTE ** To get the cw face we use cw(i). to get 
            // the cw point (relative to this face) we use ccw(i).
            int i = c->index(prev);
                        
            // The first vertex is always the one opposite the previous face.
            const Point & p0 = c->vertex(i)->point();
            
            // p1 is the 'pivot' vertex.
            Point  p1;

            // The second vertex is either cw or ccw of this point.
            // This is the _Pivot_ point.        
            if (clockwise)
                p1 = c->vertex(c->cw(i))->point();
            else
                p1 = c->vertex(c->ccw(i))->point();            
                        
            // If we can't keep going in this direction,
            // then either we have arrived (one more orientation) or 
            // we need to jump to the next cell. (go through the remaining
            // face).            
            this->incOrientationCount();    
            
            if (( CGAL::orientation(p0,p1,p) == direction) )
            {
                Point p2;
                
                // The remaining point.
                if (clockwise)
                    p2 = c->vertex(c->ccw(i))->point();
                else
                    p2 = c->vertex(c->cw(i))->point();                
                
                this->incOrientationCount();    
                // If we can't see the final point still, we are done.
                if ( (CGAL::orientation(p0, p2, p) != direction) )
                {
                                        
                    pivots.append(p1);             
                    break;
                                        
                // New pivot.
                } else {                    
                    
                    pivots.append(p1);                                                     
                    prev = c;
                    addToWalk(c);
                    
                    if (clockwise)
                        c    = c->neighbor(c->cw(i));
                    else
                        c    = c->neighbor(c->ccw(i)); 
                        
                    // Change direction.    
                    clockwise = !clockwise;           
                                                 
                }
            } else {
                                
                prev = c;
                addToWalk(c);                
                
                if (clockwise)
                    c    = c->neighbor(c->ccw(i));
                else
                    c    = c->neighbor(c->cw(i));
                    
            }       
            
                                
        }
    }
    
    /*************************************************************************/
    
    // Overload get graphics to draw the pivots.    
    QGraphicsItemGroup* getGraphics()
    {
        
        CGAL::Qt::Converter<Gt> c;
        
        // Invoke the base-class drawing method to get
        // the triangles involved.
        QGraphicsItemGroup* g = Walk<T>::getGraphics();
        
        // The drawing style for the pivots.        
        QPen   pen(Qt::blue);
        QBrush brush(Qt::blue);
                
        // Iterate over faces in this walk.
        typename QList<typename T::Point>::const_iterator i;
        for (i = pivots.begin(); i != pivots.end(); ++i)
        {    
            QGraphicsEllipseItem *e = new QGraphicsEllipseItem( QRect(
                                            c(*i).toPoint() + QPoint(-6,-6), 
                                            QSize(12,12)                   ) );
            e->setBrush(brush);
            e->setPen(pen);
            g->addToGroup(e);
        
        }
        return g;
    }
    
    /*************************************************************************/
    
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
    VisibilityWalk(Point p, T* dt, Face_handle f=Face_handle())
    {

        this->dt = dt;

        // The user did not provide a face handle. So just use the infinite face.
        if (f==Face_handle())
            f=dt->infinite_face();

        // This is where we store the current face.
        Face_handle    c = f;  
        Face_handle prev = c;  


        // Create a binary random number generator.
        boost::rand48 rng;
        boost::uniform_smallint<> two(0, 1);
        boost::variate_generator<boost::rand48&, boost::uniform_smallint<> > coin(rng, two);    


        // **     FIND FIRST FACE      ** //
        for (int i=0; i<3; i++)
        {
            const Point & p0 = c->vertex(i       )->point();
            const Point & p1 = c->vertex(c->cw(i))->point();
            
            this->incOrientationCount();    
            
            // If we have found a face that can see the point.
            if ( CGAL::orientation(p0,p1,p) == CGAL::POSITIVE )
            {
                c = c->neighbor(c->ccw(i));
                break;
            }
        }
        // ** END OF FIND FIRST FACE ** //


        // Loop until we find our destination point.
        for (int j=0; j<100; j++)
        {
            addToWalk(c);


            int i = c->index(prev);

            const Point & p0 = c->vertex( i          )->point();
            const Point & p1 = c->vertex( dt->cw(i)  )->point();
            const Point & p2 = c->vertex( dt->ccw(i) )->point();

            int left_first   = coin() % 2;

            // We randomise the order in which we test the 
            // faces we are walking through
            if (left_first)
            {
                
                this->incOrientationCount();
            	if ( orientation(p0,p1,p) == CGAL::POSITIVE ) {  
                    prev = c;
                    c = c->neighbor( dt->ccw(i) );  
                    continue;
        	    }                
        	    
                this->incOrientationCount();
            	if ( orientation(p2,p0,p) == CGAL::POSITIVE ) {  
                    prev = c;            	    
                    c = c->neighbor( dt->cw(i) );  
                    continue;
        	    }        	    
        	            	                    
            } else {
                                     	    
                this->incOrientationCount();
            	if ( orientation(p2,p0,p) == CGAL::POSITIVE ) {  
                    prev = c;            	    
                    c = c->neighbor( dt->cw(i) );  
                    continue;
        	    }
        	            	    
                this->incOrientationCount();
            	if ( orientation(p0,p1,p) == CGAL::POSITIVE ) {  
                    prev = c;            	    
                    c = c->neighbor( dt->ccw(i) );  
                    continue;
        	    }        	    
                                
            }

            // If neither of the above tests failed,
            // then we do one final test to check to see
            // whether or not we have arrived.
            this->incOrientationCount();  	    	
        	if ( orientation(p2,p1,p) == CGAL::POSITIVE ) {  
                prev = c;            	      
                addToWalk(c);
                break;
    	    }

        }    

    }
    

};

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
QGraphicsItemGroup* Walk<T>::getGraphics()
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
            QGraphicsPolygonItem *tr=drawTriangle(*i,QPen(),QColor("#D2D2EB"));         
            g->addToGroup(tr);        
        }
    }

    return g;
}

/*****************************************************************************/  

template <typename T>
void Walk<T>::incOrientationCount()
{
    o_count++;
}

/*****************************************************************************/  

template <typename T>
Walk<T>::Walk()
{
    o_count=0;
}

/*****************************************************************************/  

template <typename T>
int Walk<T>::getNumOrientationsPerformed()
{
    return o_count;
}

/*****************************************************************************/  

// Helper-function to create a triangle graphics item.
// Note that this is publically accessible and static.
template <typename T>
QGraphicsPolygonItem* Walk<T>::drawTriangle( Face_handle f,
                                             QPen        pen,
                                             QBrush      brush )
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
    polygonItem->setPen(pen);
    polygonItem->setBrush(brush);
    
    return polygonItem;
}

/*****************************************************************************/

#endif

/*****************************************************************************/