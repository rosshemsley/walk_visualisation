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
    // We emit this when we have chosen a value.
    void valueChanged(int value);
    
private slots:
    // Internal handling of button click.
    void buttonClicked()
    {
        emit valueChanged( spin_numPoints->value() );
        this->hide();
    }
    
private:
    QSpinBox*    spin_numPoints;
    QPushButton* button_create;
    QLabel*      label_info;    
    QGridLayout* layout;    
    
public:
    void show()
    {
        spin_numPoints->setValue(100);
        QDialog::show();
    }
    
    PointGeneratorDialog()
    {
        // Set up the input form.
        layout = new QGridLayout;
        label_info     = new QLabel(tr("Number of points to add"));
        spin_numPoints = new QSpinBox();
        button_create  = new QPushButton(tr("Generate"));
        layout->addWidget(label_info,    0,0,1,2);
        layout->addWidget(spin_numPoints,1,0);
        layout->addWidget(button_create, 1,1);
        
        // Minimum size of the random pointset.
        spin_numPoints->setMinimum(1);
        spin_numPoints->setMaximum(999999);
        spin_numPoints->setSingleStep(10);
        
                
        this->setWindowTitle(tr("Generate points."));
        this->setLayout(layout);
        
        // Connect button to the buttonClicked slot.        
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

public slots:    
    void                            randomTriangulation(int points);    
    
private:
    void                            createMenus();
    void                            createActions();    
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