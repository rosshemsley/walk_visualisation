/******************************************************************************
* Written by Ross Hemsley for INRIA.fr. 
* A simple application visualise different walks on Delaunay Triangulations.
******************************************************************************/

#include <QApplication>
#include "mainwindow.h"

/*****************************************************************************/

int main(int argc, char **argv)
{    	    
    QApplication app(argc, argv);


    app.setOrganizationDomain("INRIA.fr");
    app.setOrganizationName("INRIA");
    app.setApplicationName("Walk Visualisation Demo");


    MainWindow *window = new MainWindow();   
    window->show();

    return app.exec();
}

/*****************************************************************************/