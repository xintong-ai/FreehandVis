project(QtVTKRenderWindows)

cmake_minimum_required(VERSION 2.6)

Set(CMAKE_PREFIX_PATH "C:/Program Files (x86)/Windows Kits/8.0/Lib/win8/um/x64")

if(POLICY CMP0020)
  cmake_policy(SET CMP0020 NEW)
endif()

find_package(VTK 
#COMPONENTS
#  vtkCommonCore
#  vtkFiltersSources
#  vtkGUISupportQt
#  vtkIOImage
#  vtkInteractionImage
#  vtkRenderingVolumeOpenGL
)
include(${VTK_USE_FILE})

find_package(Leap)

if("${VTK_QT_VERSION}" STREQUAL "")
  message(FATAL_ERROR "VTK was not built with Qt")
endif()

# Set your files and resources here
set( Srcs QtVTKRenderWindowsApp.cxx QtVTKRenderWindows.cxx LeapListener.cxx HandWidget.cxx)

set( Hdrs QtVTKRenderWindows.h LeapInteraction.h LeapListener.h HandWidget.h)

set( MOC_Hdrs QtVTKRenderWindows.h LeapInteraction.h)

set( UIs QtVTKRenderWindows.ui )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${LEAP_INCLUDE_DIR}
)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

if(VTK_QT_VERSION VERSION_GREATER "4")
  find_package(Qt5Widgets REQUIRED QUIET)
  qt5_wrap_ui(UI_Srcs ${UIs})

  # CMAKE_AUTOMOC in ON so the MocHdrs will be automatically wrapped.
  add_executable(QtVTKRenderWindows ${Srcs} ${Hdrs} ${UI_Srcs} ${MOC_Hdrs})
  qt5_use_modules(QtVTKRenderWindows Core Gui Widgets)
  target_link_libraries(QtVTKRenderWindows ${VTK_LIBRARIES} ${LEAP_LIBRARY})
else()
  find_package(Qt4 REQUIRED)
  include(${QT_USE_FILE})
  # Use what VTK built with
  set(QT_QMAKE_EXECUTABLE ${VTK_QT_QMAKE_EXECUTABLE} CACHE FILEPATH "")
  set(QT_MOC_EXECUTABLE ${VTK_QT_MOC_EXECUTABLE} CACHE FILEPATH "")
  set(QT_UIC_EXECUTABLE ${VTK_QT_UIC_EXECUTABLE} CACHE FILEPATH "")
  qt4_wrap_ui(UI_Srcs ${UIs})

  add_executable(QtVTKRenderWindows ${Srcs} ${Hdrs} ${UI_Srcs} ${MOC_Hdrs})
  target_link_libraries(QtVTKRenderWindows ${QT_LIBRARIES} ${VTK_LIBRARIES})
endif()
