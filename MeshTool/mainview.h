#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLDebugLogger>
#include <QColorDialog>

#include <QOpenGLShaderProgram>
#include <QHash>

#include <QMouseEvent>
#include <QtMath>
#include <QOpenGLFramebufferObject>

#include <limits>
#include "mainwindow.h"
#include "mesh.h"
#include "tools/tools.h"
#include "coordsedit.h"
#include "coloredit.h"
#include "renderers/defaultrenderer.h"
#include "renderers/acc1renderer.h"
#include "renderers/acc2renderer.h"
#include "renderers/ggrenderer.h"
#include "renderers/featureadaptiverenderer.h"
#include "renderers/pointrenderer.h"
#include "renderers/linerenderer.h"

class MainView : public QOpenGLWidget, protected QOpenGLFunctions_4_1_Core {

  Q_OBJECT

public:
  MainView(QWidget *Parent = 0);
  ~MainView();

  void setMainWindow(MainWindow *mainWindow);

  // Meshes
  Mesh inputMesh;
  QVector<Mesh> originalMeshes, editedMeshes, limitMeshes;
  QVector<QVector<int>> editableVertexIndices;
  QVector<QVector<int>> gradientVertexIndices;
  QVector<int> changedLimitCoordsIndices;
  QVector<int> changedEdgesIndices;
  QVector<int> changedFacesIndices;

  void setMesh(QString fileName);

  void recomputeMeshes();
  void subdivide();
  void updateCoords(int editFlag);
  void updateColor();
  bool isModelLoaded();
  int getMaxComputedSubdivLevel();
  CoordsEdit computeCoordsEdit(Vertex v, QVector2D deltaCoords);


  // Editing
  QHash<int, QHash<int, CoordsEdit>> coordsEdits;
  QHash<int, QHash<int, ColorEdit>> colorEdits;
  void editCoords(QPoint eventPos);
  void editColor();
  void clearSelection();

  // Updating renderers
  void updateMeshForCurrentRenderer(int update);
  void updateMeshLimitRenderer();
  void updateShowWireframe();
  void updateScaling();
  void updateDisplacement();
  void updateColorBands();
  void updateTessLevel();
  void updateDiffScaling();

  void updateScale(float s);

protected:
  // OpenGL
  void initializeGL();
  void resizeGL(int width, int height);
  void paintGL();

  // Events
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void wheelEvent(QWheelEvent *event);
  void keyPressEvent(QKeyEvent *event);


private:
  // MainWindow
  MainWindow *mainWindow;

  // Logger
  QOpenGLDebugLogger* debugLogger;

  // Rendering
  QHash<QString, SurfaceRenderer*> renderers;
  PointRenderer *pointRenderer;
  LineRenderer *lineRenderer;
  void renderPoints();

  // Framebuffer
  QOpenGLFramebufferObject *limitFramebuffer, *diffFramebuffer;

  // Editing
  int selectedVertex = -1;
  QSet<int> selectedEdges;
  QSet<int> affectedVertex;
  QPoint lastEventPos;
  void setSelected(QPoint point);
  QVector2D computeColorEditPointCoords(HalfEdge *e);

  // Transformation
  float scale = 1.0;
  QVector2D displacement{0, 0};
  QVector2D getScaleVector();
  QVector2D getWorldCoords(QPoint point);

  // Get ui information
  int getSubdivSteps();
  int getEditSteps();
  int getLimitSubdivSteps();
  bool isDiffComputed();
  bool isEditingEnabled();
  float getBrushRadius();
  QVector3D getBrushColor();


private slots:
  void onMessageLogged(QOpenGLDebugMessage Message);

};

#endif // MAINVIEW_H
