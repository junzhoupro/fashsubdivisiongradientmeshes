#ifndef TRANSITIONPATCHRENDERER_H
#define TRANSITIONPATCHRENDERER_H

#include "surfacerenderer.h"
#include "mesh.h"
#include "qvector5d.h"
#include <QVector>
#include <QVector2D>

class TransitionPatchRenderer : public SurfaceRenderer {

public:
  TransitionPatchRenderer(QOpenGLFunctions_4_1_Core *functions);
  ~TransitionPatchRenderer();
  void render();
  void clearControlPoints();
  void addControlPoints(Face f, QSet<int> transitionEdges);
  void setData();
  QHash<QString, int> getCountInfo();

private:
  GLuint VAOACC1, VBOACC1;
  GLuint VAOACC2, VBOACC2;
  QHash<QString, QVector<float>> datasACC1, datasACC2;
  QHash<QString, int> controlPointsOffsetsACC1, controlPointsOffsetsACC2;
  QHash<QString, int> controlPointsSizesACC1, controlPointsSizesACC2;

};

#endif // TRANSITIONPATCHRENDERER_H
