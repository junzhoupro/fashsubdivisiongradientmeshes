#ifndef ACC2RENDERER_H
#define ACC2RENDERER_H

#include "surfacerenderer.h"
#include "mesh.h"
#include "qvector5d.h"
#include <QVector>
#include <QVector2D>

class ACC2Renderer : public SurfaceRenderer {

public:
  ACC2Renderer(QOpenGLFunctions_4_1_Core *functions);
  ~ACC2Renderer();
  void setData(QVector<float> dataTriangles, QVector<float> dataQuads);
  void setMesh(Mesh& mesh);
  void updateMeshCoords(Mesh& mesh, QVector<int>& influencedFacesIndices);
  void updateMeshColors(Mesh& mesh, QVector<int>& influencedFacesIndices);
  void render();
  QHash<QString, int> getCountInfo();

  static QVector5D computeCornerPoint(HalfEdge *inputEdge);
  static QVector5D computeEdgePoint(HalfEdge *inputEdge, QVector5D p, bool forward);
  static QVector5D computeFacePoint(HalfEdge *inputEdge, QVector5D ep, QVector5D em, double d, bool forward);
  static void addControlPoints(Face f, QVector<float> *data);
  static void updateControlPoints(Face &f, QVector<float> &data, int faceIndex, int coorsOrColor);

private:
  GLuint VAO, VBO;
  int controlPointsQuadsSize, controlPointsTrianglesSize;

  static float sigma(int n);
  static float lambda(int n);

  QHash<int, int> dataTrianglesIndices;
  QHash<int, int> dataQuadsIndices;
  QVector<float> dataTriangles;
  QVector<float> dataQuads;

};

#endif // ACC2RENDERER_H
