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
// Dialog to allow a new random pointset to be generated.

class PointGeneratorDialog : public QDialog
{
    Q_OBJECT
           
signals:
    void valueChanged(int value);
    
private slots:
    void buttonClicked()
    {
        emit valueChanged( text_numPoints->text().toInt() );
    }
    
private:
    QLineEdit*   text_numPoints;
    QPushButton* button_create;
    QLabel*      label_info;    
    QGridLayout* layout;    
    
public:
    void show()
    {
        text_numPoints->clear();
        QDialog::show();
    }
    
    PointGeneratorDialog()
    {
        
        layout = new QGridLayout;

        label_info     = new QLabel(tr("Number of points to add"));
        text_numPoints = new QLineEdit();
        button_create  = new QPushButton(tr("Generate"));
        
        text_numPoints->setInputMask("9999999999");

        layout->addWidget(label_info,    0,0,1,2);
        layout->addWidget(text_numPoints,1,0);
        layout->addWidget(button_create, 1,1);

        this->setWindowTitle(tr("Generate points."));
        this->setLayout(layout);
                
        connect(button_create, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    }
};

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
    void                            newPointset(); 
    void                            updateScene();
    void                            straightWalk_checkbox_change(int state);
    void                            visibilityWalk_checkbox_change(int state);
    void                            pivotWalk_checkbox_change(int state);


private:
    void                            createMenus();
    void                            createActions();    
    void                            randomTriangulation(int points);    

    bool                            drawPivotWalk;
    bool                            drawStraightWalk;
    bool                            drawVisibilityWalk;
    PointGeneratorDialog*           dialog_newPointset;
    QMenu*                          fileMenu;
    QLabel*                         status;    
    QAction*                        newAct;        
    QGraphicsView*                  view;
    QGraphicsScene*                 scene;    
    Delaunay*                       dt;
    CGAL::Qt::Converter<K>          c;
    QTriangulationGraphics*         tgi; 
    QList<QGraphicsItem*>           walkItems;
     
    // When we are taking points as input we use the following.
    // if inputPoints < 0 we are not learning points..
    // if it is positive we are learning points[inputPoints]
    int                             inputPoints;
    QPoint                          points[2];

}; 


/*****************************************************************************/

#endif

/*****************************************************************************/