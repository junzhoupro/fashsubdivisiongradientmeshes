#ifndef ACC1RENDERER_H
#define ACC1RENDERER_H

#include "surfacerenderer.h"
#include "mesh.h"
#include "qvector5d.h"
#include <QVector>

class ACC1Renderer : public SurfaceRenderer {

public:
  ACC1Renderer(QOpenGLFunctions_4_1_Core *functions);
  ~ACC1Renderer();
  void setData(QVector<float> data);
  void setMesh(Mesh& mesh);
  void updateMeshCoords(Mesh& mesh, QVector<int>& influencedFacesIndices);
  void updateMeshColors(Mesh& mesh, QVector<int>& influencedFacesIndices);
  void render();
  QHash<QString, int> getCountInfo();

  static QVector5D computeInteriorPoint(HalfEdge *inputEdge);
  static QVector5D computeEdgePoint(HalfEdge *inputEdge, bool forward);
  static QVector5D computeCornerPoint(HalfEdge *inputEdge);
  static void addControlPoints(Face f, QVector<float> *data);
  static void updateControlPoints(Face &f, QVector<float> &data, int faceIndex, int coordsOrColor);

private:
  GLuint VAO, VBO;
  int controlPointsSize;

  QHash<int, int> facesIndices;
  QVector<float> data;
};

#endif // ACC1RENDERER_H
