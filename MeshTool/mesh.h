#ifndef MESH_H
#define MESH_H

#include <QVector>

#include "vertex.h"
#include "face.h"
#include "halfedge.h"

class Mesh {

public:
  QVector<Vertex> Vertices;
  QVector<Face> Faces;
  QVector<HalfEdge> HalfEdges;

  Mesh copy();

};

#endif // MESH_H
