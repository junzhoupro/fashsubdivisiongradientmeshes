#ifndef SURFACERENDERER_H
#define SURFACERENDERER_H

#include "mesh.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions_4_1_Core>
#include <QVector2D>

class SurfaceRenderer {

public:
  SurfaceRenderer(QOpenGLFunctions_4_1_Core *functions);
  virtual ~SurfaceRenderer();
  virtual void setShowWireframe(bool showWireframe);
  virtual void setScaling(QVector2D scaling);
  virtual void setDisplacement(QVector2D displacement);
  virtual void setColorBands(int colorBands);
  virtual void setTessLevel(int tessLevel);
  virtual void setComputeDiff(int computeDiff);
  virtual void setDiffScaling(float diffScaling);
  virtual void setMesh(Mesh& mesh) {}
  virtual void updateMeshCoords(Mesh& mesh, QVector<int>& changedLimitCoordsIndices) {}
  virtual void updateMeshColors(Mesh& mesh, QVector<int>& changedEdgesIndices) {}
  virtual void render() = 0;
  virtual QHash<QString, int> getCountInfo() = 0;

protected:
  bool showWireframe = false;
  QOpenGLFunctions_4_1_Core *functions;
  QHash<QString, QOpenGLShaderProgram *> shaderPrograms;
  QHash<QString, SurfaceRenderer *> renderers;

};

#endif // SURFACERENDERER_H
