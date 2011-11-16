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

/*****************************************************************************/


/******************************************************************************
* Abstract class to contain different walking strategies
******************************************************************************/

template <typename T>
class Walk
{
    typedef typename T::Face                                         Face;
    typedef typename T::Face::Face_handle                            Face_handle;
    typedef typename T::Line_face_circulator                         Line_face_circulator;
    typedef typename T::Geom_traits                                  Gt;
    
    
public:
    virtual QGraphicsItem*          getGraphics()=0;    
    
protected:
    T*                              dt;
    QGraphicsPolygonItem*           drawTriangle(Face_handle f);
};

/******************************************************************************
* Straight walk strategy
******************************************************************************/

template <typename T>
class StraightWalk : public Walk<T> 
{
    typedef typename T::Face                                         Face;
    typedef typename T::Face::Face_handle                            Face_handle;
    typedef typename T::Line_face_circulator                         Line_face_circulator;
    typedef typename T::Geom_traits                                  Gt;
    
public:    
                                    StraightWalk(QPoint p, QPoint q, T* dt);
    QGraphicsItem*                  getGraphics();
    
private:
    Line_face_circulator            lfc;
    
};

/******************************************************************************
* Visibility walk strategy
******************************************************************************/

template <typename T>
class VisibillityWalk : public Walk<T>
{
    typedef typename T::Face                                         Face;
    typedef typename T::Face::Face_handle                            Face_handle;
    typedef typename T::Line_face_circulator                         Line_face_circulator;
    typedef typename T::Geom_traits                                  Gt;
public:    
                                    VisibillityWalk(QPoint q, QPoint q, T* dt);
    QGraphicsItem*                  getGraphics();
};


/*****************************************************************************/  

#include "walk.h"

/******************************************************************************
* Straight Walk
******************************************************************************/  
  
template <typename T>
StraightWalk<T>::StraightWalk(QPoint p, QPoint q, T *dt)
{
  this->dt = dt;
  // Create a circulator describing the walk.
  CGAL::Qt::Converter<Gt>         c;
 
  lfc = dt->line_walk (c(p),c(q));      
}

/*****************************************************************************/  
  
template <typename T>  
QGraphicsItem* StraightWalk<T>::getGraphics()
{
    QGraphicsItemGroup* g = new QGraphicsItemGroup();

    Line_face_circulator done(lfc);

    // Ignore empty walks.
    if (lfc==0) return 0;

    do 
    {
        // Draw this triangle in the walk.
        QGraphicsPolygonItem *tr = drawTriangle(lfc); 

        // If the triangle is non-empty, add it to the graphics item.
        if (tr!=0)
            g->addToGroup(tr);

        ++lfc;
    } while (lfc != done);        

    return g;
}   

/*****************************************************************************/  



/******************************************************************************
*
* Implementations
*
******************************************************************************/




/******************************************************************************
* Visibility Walk
******************************************************************************/

template <typename T>
VisibillityWalk<T>::VisibillityWalk(QPoint x, QPoint y, T* dt)
{
    
} 

/*****************************************************************************/

template <typename T>
QGraphicsItem* VisibillityWalk<T>::getGraphics()
{
    
}

/******************************************************************************
* Walk functions
******************************************************************************/

template <typename T>
QGraphicsPolygonItem* Walk<T>::drawTriangle(Face_handle f)
{
    CGAL::Qt::Converter<Gt>         c;


    // Ignore infinite faces.
    if (! dt->is_infinite( f ) ) 
    {
        // Convert a face into a polygon for plotting.
        QGraphicsPolygonItem *polygonItem;
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

    return 0;
}

/*****************************************************************************/

#endif

/*****************************************************************************/