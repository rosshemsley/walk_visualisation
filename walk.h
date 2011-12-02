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
    QGraphicsItemGroup*             getGraphics( QPen       pen   = QPen(),
                                                 QBrush     brush = QBrush());
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
    
    CGAL::Orientation               orientation(Point p, Point q, Point r);    
    
    
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
        
        // Create a binary random number generator.
        boost::rand48 rng;
        boost::uniform_smallint<> two(0, 1);
        boost::variate_generator<boost::rand48&, boost::uniform_smallint<> > coin(rng, two);    

        
        
        
        int pivots_passed     = 0;
        int triangles_visited = 0;
        int or_saved=0;
        int or_lost=0;
        
        qDebug() << "\n\n NEW WALK";
        
        bool new_pivot = true;
        
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
                                    
            // If we have found a face that can see the point.
            if ( orientation(p0,p1,p) == CGAL::POSITIVE )
            {
                c    = c->neighbor(c->ccw(i));
                break;
            }
        }
        // ** END OF FIND FIRST FACE ** //


        
        bool clockwise = TRUE;
        

        for (int x=0; x<100; x++)
        {
            
            clockwise = coin() % 2;
            
            addToWalk(c);
            
            // Assume we have just walked into a new cell. The first thing to do is decide a direction.
            
            // This is the index of the sink of the last cell relative to the current triangle.
            int i = c->index(prev);
            
            // First thing to do is choose a direction. We use the value of clockwise to decide this. 
            // But may have to swap depending on the configuation of the points.
            
            // Pivot point.
            const Point & p_pivot = c->vertex(i)->point();
            
            
            
            // Point linking the pivot to the clockwise face.
            // ** Note ** cw and ccw are reversed when we are converting between 
            // a face index and a point index.
            const Point & p_cw = c->vertex(c->ccw(i))->point();
            
            // Point linking the pivot to the counter-clockwise face.
            const Point & p_ccw = c->vertex(c->cw(i))->point();
            
            
            if (clockwise)
            {
                
                // If visibility does hold in this direction, continue walking around this point
                if (orientation(p_pivot, p_cw, p) == CGAL::RIGHT_TURN )
                {
                    prev = c;
                    c    = c->neighbor(c->cw(i));
                }

                // If visibility does hold in this direction, continue walking around this point
                else if ( orientation(p_pivot, p_ccw, p) == CGAL::LEFT_TURN )
                {
                    prev = c;
                    c    = c->neighbor(c->ccw(i));
                    
                    qDebug() <<  "EXTRA TEST";                    
                    // Swap the direction.
                    clockwise = !clockwise;
                }
                
                // If both of the above tests failed, then we know that the point is here.
                else 
                {
                    qDebug () << "Found at start";
                    break;
                }
              
            } else { // SAME BUT ORDER REVERSED //
                                
                // If visibility does hold in this direction, continue walking around this point
                if ( orientation(p_pivot, p_ccw, p) == CGAL::LEFT_TURN )
                {
                    prev = c;
                    c    = c->neighbor(c->ccw(i));
                }
                                
                // If visibility does hold in this direction, continue walking around this point
                else if ( orientation(p_pivot, p_cw, p) == CGAL::RIGHT_TURN )
                {
                    prev = c;
                    c    = c->neighbor(c->cw(i));

                    qDebug() <<  "EXTRA TEST";
                    // Swap the direction.
                    clockwise = !clockwise;                                                            
                }
                
                // If both of the above tests failed, then we know that the point is here.
                else 
                {
                    qDebug () << "Found at start";
                    break;
                }              
            }
            
            pivots.append(p_pivot);             
            
            addToWalk(c);            
            
            
            qDebug() << "\nStarting going around cell";
            
            // We should now be going in a good direction in the cell about some pivot point p_pivot.
            // We continue in the direction given by clockwise until we meet the first edge that is going in the wrong direction
            // When we meet this we have to test to see if the point is contained within this sink node, and then 
            // we go again from the start of the loop if it is not.
            bool done = false;
            
            
            
            pivots_passed++;

            // This is where we would have gone if the first test failed!
            Face_handle omitted_next;
            Point       p_omitted;
            Point       p_omitted_final;

            for (int y=0; y<100; y++)
            {
                // Index of the previous triangle relative to the current triangle.
                i = c->index(prev);
                
                triangles_visited++;
                
                if (y==2) or_saved++;
                if (clockwise)               
                { 
                    // This is the point on the edge that we are going to test.
                    const Point & p_current = c->vertex(i)->point();
                    
                    if (y == 0)
                    {
                        // We might need to come back to this test.
                        omitted_next = c->neighbor(c->cw(i));          
                        p_omitted    = p_current; 
                        p_omitted_final = c->vertex(c->ccw(i))->point();                        
                        prev = c;
                        c    = c->neighbor(c->ccw(i));                                 
                    }
                    // If we can see the point through this edge
                    else if (y!=0 && orientation(p_pivot, p_current, p) == CGAL::RIGHT_TURN )
                    {
                        // continue in this direction.
                        prev = c;
                        c    = c->neighbor(c->ccw(i));
                        
                    } else {
                        
                        // We skipped the first test. 
                        // We might sometimes have to go back and do it. In this case,
                        // We have to retrieve the missed test, do the orientation,
                        // and if we find we should have gone through the first 
                        // triangle, we have wasted one test, and have to move back.
                        // However, in most situations we do not have to go back.                                            
                        if (y == 1)
                        {
                            if (orientation(p_pivot, p_omitted, p) == CGAL::LEFT_TURN)
                            {
                                qDebug() << "Had to backtrack";
                                or_lost++;
                                
                                if (orientation(p_current, p_omitted_final, p) == CGAL::LEFT_TURN )                        
                                {
                                    // We are done;
                                    done = true;
                                    qDebug () << "Found at end";                            
                                    break;
                                }
                                
                                
                                // go backwards.
                                c         = omitted_next;
                                clockwise = !clockwise;
                                break;                                
                            }
                        }
                        
                        
                        // We have reached the sink node. Check to see if the point
                        // is contained. If not then start from the beginning.
                        const Point & p_final = c->vertex(c->ccw(i))->point();
                        if (orientation(p_current, p_final, p) == CGAL::LEFT_TURN )                        
                        {
                            // We are done;
                            done = true;
                            qDebug () << "Found at end";                            
                            break;
                            
                        } else {
                            // Start a new cell.
                            prev = c;
                            c    = c->neighbor(c->cw(i));
                            clockwise = !clockwise;
                            break;
                            
                        }
                    }
                    
                } else {
                    
                     // This is the point on the edge that we are going to test.
                        const Point & p_current = c->vertex(i)->point();


                        if (y == 0)
                        {
                            // We might need to come back to this test.
                            omitted_next = c->neighbor(c->ccw(i));          
                            p_omitted    = p_current;
                            p_omitted_final = c->vertex(c->cw(i))->point();
                            prev = c;
                            c    = c->neighbor(c->cw(i));

                        }                        
                        // If we can see the point through this edge
                        else if (y!=0 && orientation(p_pivot, p_current, p) == CGAL::LEFT_TURN )
                        {
                            // continue in this direction.
                            prev = c;
                            c    = c->neighbor(c->cw(i));

                        } else {
                            if (y==1)
                            {
                                if (orientation(p_pivot, p_omitted, p) == CGAL::RIGHT_TURN)
                                {
                                    or_lost++;
                                    
                                    
                                    if (orientation(p_current, p_omitted_final, p) == CGAL::RIGHT_TURN )                        
                                    {
                                        // We are done;
                                        done = true;
                                        qDebug () << "Found at end";                            
                                        break;
                                    }
                                    
                                    
                                    qDebug() << "Had to backtrack";
                                    // go backwards.
                                    c         = omitted_next;
                                    clockwise = !clockwise;
                                    break;                                
                                }
                            }
        
                            
                            
                            
                            // We have reached the sink node. Check to see if the point
                            // is contained. If not then start from the beginning.
                            const Point & p_final = c->vertex(c->cw(i))->point();
                            if ( orientation(p_current, p_final, p) == CGAL::RIGHT_TURN )                        
                            {
                                // We are done;
                                done = true;
                                qDebug () << "Found at end";                                
                                break;
                            } else {
                                // Start a new cell.
                                prev = c;
                                c    = c->neighbor(c->ccw(i));
                                clockwise = !clockwise;
                                break;
                                
                            }
                            
                        }
                        
                }
                
                addToWalk(c);            
                
            }
            
            
            if (done) 
                break;            
            
        }
        
        qDebug() << triangles_visited/(float)pivots_passed;


        qDebug() << "Lost: " << or_lost;
        qDebug() << "Saved: " << or_saved;
        /*
        
        // We swap direction every time we change pivot.
        bool clockwise = TRUE;

        for (int x = 0; x<100; x++)
        {
            qDebug() << "\n\nStarting new Triangle";
            if (clockwise)
                qDebug() << "Going Clockwise";
            else 
                qDebug() << "Going CounterClockwise";
            
            if (c == source)
                break;
            
            // We need to reverse handedness of orientation tests depending on
            // the directino we are going.   
            // NOTE. we can avoid several comparisons by unrolling the 
            // direction changing logic.         
            orientation direction;
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
            
            
            if (( orientation(p0,p1,p) == direction) )
            {
                qDebug() << "Failed orientation";
                // The first test has failed. Let's just try going in the opposite direction.
                if (new_pivot)
                {
                    qDebug() << "But we are in new pivot, so trying other direction...";
                    if (clockwise)
                        prev = c->neighbor(c->ccw(i));
                    else
                        prev = c->neighbor(c->cw(i));                    
                        
                    clockwise=!clockwise;
                    continue;
                }
                qDebug() << "Testing remaining point to see if we are done";
                Point p2;
                
                // The remaining point.
                if (clockwise)
                    p2 = c->vertex(c->ccw(i))->point();
                else
                    p2 = c->vertex(c->cw(i))->point();                
                
                
                
                this->incOrientationCount();    
                // If we can't see the final point still, we are done.
                if ((orientation(p0, p2, p) !=  direction) )
                {
                    qDebug() << "we are done";
                    pivots.append(p1);             
                    break;                                    
                }                  
                // New pivot.
                qDebug() << "Generating new Pivot";
                new_pivot=true;
                    
                    pivots.append(p1);                                                     
 
                    prev = c;
                    Face_handle tmp = prev;
                    
                                        
                    if (clockwise)
                    {
                        
                        c    = c->neighbor(c->cw(i));                                                                    
                        prev = c->neighbor(c->cw(c->index(prev)));
                        
         
                        
                    } else {
                        c    = c->neighbor(c->ccw(i));                         
                        prev = c->neighbor(c->ccw(c->index(prev)));                        
                    }

                    source = prev;
                    
                    
                    
                        
                    
                    addToWalk(c);
                        
                    // Change direction.    
                    clockwise = !clockwise;           
                 
            } else {
                
                qDebug() <<"Continuing around...";
                
                new_pivot=false;
                                
                prev = c;                
                if (clockwise)
                    c    = c->neighbor(c->ccw(i));
                else
                    c    = c->neighbor(c->cw(i));
                addToWalk(c);                
                    
            }       
            
                                
        }
        
        */
    }
    
    /*************************************************************************/
    
    // Overload get graphics to draw the pivots.    
    QGraphicsItemGroup* getGraphics( QPen pen=QPen(), QBrush brush=QBrush() )
    {
        
        CGAL::Qt::Converter<Gt> c;
        
        // Invoke the base-class drawing method to get
        // the triangles involved.
        QGraphicsItemGroup* g = Walk<T>::getGraphics(pen,brush);
        
        // The drawing style for the pivots.        
        QPen   e_pen(Qt::blue);
        QBrush e_brush(Qt::blue);
                
        // Iterate over faces in this walk.
        typename QList<typename T::Point>::const_iterator i;
        for (i = pivots.begin(); i != pivots.end(); ++i)
        {    
            QGraphicsEllipseItem *e = new QGraphicsEllipseItem( QRect(
                                            c(*i).toPoint() + QPoint(-6,-6), 
                                            QSize(12,12)                   ) );
            e->setBrush(e_brush);
            e->setPen(e_pen);
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
            
            
            // If we have found a face that can see the point.
            if ( orientation(p0,p1,p) == CGAL::POSITIVE )
            {
                c = c->neighbor(c->ccw(i));
                break;
            }
        }
        // ** END OF FIND FIRST FACE ** //


        // Loop until we find our destination point.
        while (1)
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
                
            	if ( orientation(p0,p1,p) == CGAL::POSITIVE ) {  
                    prev = c;
                    c = c->neighbor( dt->ccw(i) );  
                    continue;
        	    }                
        	    
            	if ( orientation(p2,p0,p) == CGAL::POSITIVE ) {  
                    prev = c;            	    
                    c = c->neighbor( dt->cw(i) );  
                    continue;
        	    }        	    
        	            	                    
            } else {
                                     	    
            	if ( orientation(p2,p0,p) == CGAL::POSITIVE ) {  
                    prev = c;            	    
                    c = c->neighbor( dt->cw(i) );  
                    continue;
        	    }
        	            	    
            	if ( orientation(p0,p1,p) == CGAL::POSITIVE ) {  
                    prev = c;            	    
                    c = c->neighbor( dt->ccw(i) );  
                    continue;
        	    }        	    
                                
            }

            // If neither of the above tests failed,
            // then we do one final test to check to see
            // whether or not we have arrived.
        	if ( orientation(p2,p1,p) == CGAL::POSITIVE ) {  
                prev = c;            	      
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
CGAL::Orientation Walk<T>::orientation(Point p, Point q, Point r)
{
    o_count++;
    
    return CGAL::orientation(p,q,r);
    
}


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
QGraphicsItemGroup* Walk<T>::getGraphics( QPen pen, QBrush brush )
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
            QGraphicsPolygonItem *tr = drawTriangle(*i,pen,brush);
            g->addToGroup(tr);        
        }
    }

    return g;
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