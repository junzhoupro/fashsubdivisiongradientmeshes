#ifndef GGRENDERER_H
#define GGRENDERER_H

#include "surfacerenderer.h"
#include "mesh.h"
#include "qvector5d.h"
#include <QVector>

class GGRenderer : public SurfaceRenderer {

public:
  GGRenderer(QOpenGLFunctions_4_1_Core *functions);
  ~GGRenderer();
  void setScaling(QVector2D scaling);
  void setDisplacement(QVector2D displacement);
  void setColorBands(int colorBands);
  void setTessLevel(int tessLevel);
  void setComputeDiff(bool computeDiff);
  void setDiffScaling(float diffScaling);
  void setMesh(Mesh& mesh);
  void updateMeshCoords(Mesh& mesh, QVector<int>& influencedFacesIndices);
  void updateMeshColors(Mesh& mesh, QVector<int>& influencedFacesIndices);
  void render();
  QHash<QString, int> getCountInfo();

private:
  GLuint VAO, VBO;
  QHash<int, int> controlPointsSizes, controlPointsOffsets;

  // Copy of uniform values (because of dynamic shader additions)
  QVector2D scaling;
  QVector2D displacement;
  int colorBands;
  int tessLevel;
  bool computeDiff;
  float diffScaling;

  QVector<float> data;
  QHash<int, QHash<int, int>> datasIndices;

  void setData(QVector<float> data);
  QOpenGLShaderProgram *makeShaderProgram(int N);

};

#endif // GGRENDERER_H
