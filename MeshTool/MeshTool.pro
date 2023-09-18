#-------------------------------------------------
#
# Project created by QtCreator 2016-12-08T16:31:55
#
#-------------------------------------------------

QT       += core gui
#QT       += openglextensions

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MeshTool
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    mesh.cpp \
    mainview.cpp \
    persistence.cpp \
    qvector5d.cpp \
  renderers/acc1renderer.cpp \
  renderers/acc2renderer.cpp \
  renderers/defaultrenderer.cpp \
  renderers/featureadaptiverenderer.cpp \
  renderers/ggrenderer.cpp \
  renderers/linerenderer.cpp \
  renderers/pointrenderer.cpp \
  renderers/surfacerenderer.cpp \
  renderers/transitionpatchrenderer.cpp \
  tools/convenience.cpp \
  tools/editing.cpp \
  tools/subdivision.cpp

HEADERS  += mainwindow.h \
    coloredit.h \
    coordsedit.h \
    mesh.h \
    persistence.h \
    qvector5d.h \
    renderers/acc1renderer.h \
    renderers/acc2renderer.h \
    renderers/defaultrenderer.h \
    renderers/featureadaptiverenderer.h \
    renderers/ggrenderer.h \
    renderers/linerenderer.h \
    renderers/pointrenderer.h \
    renderers/surfacerenderer.h \
    renderers/transitionpatchrenderer.h \
    tools/convenience.h \
    tools/editing.h \
    tools/subdivision.h \
    tools/tools.h \
    vertex.h \
    halfedge.h \
    face.h \
    mainview.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc
