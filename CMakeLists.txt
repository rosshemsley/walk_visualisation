# Created by the script cgal_create_cmake_script
# This is the CMake script for compiling a CGAL application.


project( walk_visualisation )

cmake_minimum_required(VERSION 2.6.2)
if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" VERSION_GREATER 2.6)
  if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER 2.8.3)
    cmake_policy(VERSION 2.8.4)
  else()
    cmake_policy(VERSION 2.6)
  endif()
endif()

find_package(CGAL COMPONENTS Qt4)

include(${CGAL_USE_FILE})


set( QT_USE_QTXML    TRUE )
set( QT_USE_QTMAIN   TRUE )
set( QT_USE_QTSCRIPT  TRUE )
set( QT_USE_QTOPENGL  TRUE )

find_package(Qt4)

if ( CGAL_FOUND AND CGAL_Qt4_FOUND AND QT4_FOUND )

  include(${QT_USE_FILE})


	SET(walk_visualisation_SOURCES main.cpp mainwindow.cpp)
	SET(walk_visualisation_HEADERS mainwindow.h walk.h)

	QT4_WRAP_CPP(walk_visualisation_HEADERS_MOC ${walk_visualisation_HEADERS})


	INCLUDE(${QT_USE_FILE})
	ADD_DEFINITIONS(${QT_DEFINITIONS})
	
	ADD_EXECUTABLE(walk_visualisation ${walk_visualisation_SOURCES} 
	    ${walk_visualisation_HEADERS_MOC})
	
	
  


else()
  
    message(STATUS "This program requires the CGAL library, and will not be compiled.")
  
endif()