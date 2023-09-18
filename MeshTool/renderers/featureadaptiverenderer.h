#ifndef FEATUREADAPTIVERENDERER_H
#define FEATUREADAPTIVERENDERER_H

#include "surfacerenderer.h"
#include "renderers/acc1renderer.h"
#include "renderers/acc2renderer.h"
#include "renderers/transitionpatchrenderer.h"
#include "mesh.h"
#include "qvector5d.h"
#include "coordsedit.h"
#include "coloredit.h"
#include <QVector>

class FeatureAdaptiveRenderer : public SurfaceRenderer {

public:
  FeatureAdaptiveRenderer(QOpenGLFunctions_4_1_Core *functions);
  ~FeatureAdaptiveRenderer() {}
  void setMesh(Mesh inputMesh, QHash<int, QHash<int, CoordsEdit>> coordsEdits, QHash<int, QHash<int, ColorEdit>> colorEdits);
  void render();
  QHash<QString, int> getCountInfo();

private:

  QSet<int> computeAffectedFaces(Mesh *curMesh, QHash<int, QHash<int, CoordsEdit>> coordsEdits, QHash<int, QHash<int, ColorEdit>> colorEdits);
  QSet<int> computePaddedFaces(Mesh mesh, QSet<int> inputFaces);
  QSet<int> computeTransitionFaces(Mesh mesh, QSet<int> affectedFaces);
  QSet<int> computeTransitionEdges(Face f, QSet<int> affectedFaces);

  QVector<float> dataACC1, dataACC2; // Quads only
  QHash<int, int> dataACC1Indices;
  QHash<int, int> dataACC2Indices;

};

#endif // FEATUREADAPTIVERENDERER_H
