#include "convenience.h"

bool isBoundaryVertex(Vertex *v) {
  return !getCCWBoundaryEdge(v->out)->polygon;
}

bool isBoundaryEdge(HalfEdge *e) {
  return !(e->polygon && e->twin->polygon);
}

bool isSmoothVertex(Vertex *v) {
  int sharpCount = 0;
  foreach (HalfEdge *e, getVertexEdges(v->out))
    if (isSharpEdge(e))
      ++sharpCount;
  return sharpCount <= 1;
}

bool isSharpEdge(HalfEdge *e) {
  return e->isSharp || e->twin->isSharp || !e->polygon || !e->twin->polygon;
}

HalfEdge *getCWBoundaryEdge(HalfEdge *e) {
  Vertex *v = e->prev->target;
  for (int i = 0; i < v->val && e->twin->polygon; ++i)
    e = e->twin->next;
  return e;
}

HalfEdge *getCCWBoundaryEdge(HalfEdge *e) {
  Vertex *v = e->prev->target;
  for (int i = 0; i < v->val && e->polygon; ++i)
    e = e->prev->twin;
  return e;
}

HalfEdge *getCWSharpEdge(HalfEdge *e) {
  Vertex *v = e->prev->target;
  for (int i = 0; i < v->val && !isSharpEdge(e); ++i)
    e = e->twin->next;
  return e;
}

HalfEdge *getCCWSharpEdge(HalfEdge *e) {
  Vertex *v = e->prev->target;
  for (int i = 0; i < v->val && !isSharpEdge(e); ++i)
    e = e->prev->twin;
  return e;
}

int getColorVertexVal(HalfEdge *e) {
  Vertex *v = e->prev->target;
  if (isSmoothVertex(v))
    return v->val;
  e = getCWSharpEdge(e)->prev->twin;
  int val;
  for (val = 2; !isSharpEdge(e); ++val)
    e = e->prev->twin;
  return val;
}

QVector<HalfEdge *> getVertexEdges(HalfEdge *e) {
  QVector<HalfEdge *> vertexHalfEdges;
  Vertex *v = e->prev->target;
  for (int i = 0; i < v->val; ++i) {
    vertexHalfEdges << e;
    e = e->prev->twin;
  }
  return vertexHalfEdges;
}

QVector<HalfEdge *> getFaceEdges(HalfEdge *e) {
  QVector<HalfEdge *> faceHalfEdges;
  Face *f = e->polygon;
  for (int i = 0; i < f->val; ++i) {
    faceHalfEdges << e;
    e = e->next;
  }
  return faceHalfEdges;
}

QSet<Vertex *> getVertices(QSet<Face *> inputFaces) {
  QSet<Vertex *> containedVertices;
  foreach (Face *f, inputFaces)
    foreach (HalfEdge *e, getFaceEdges(f->side))
      containedVertices << e->target;
  return containedVertices;
}

QSet<HalfEdge *> getBoundaryEdges(QSet<Face *> inputFaces) {
  QSet<HalfEdge *> boundaryEdges;
  foreach (Face *f, inputFaces) {
    foreach (HalfEdge *e, getFaceEdges(f->side)) {
      if (!e->twin->polygon || !inputFaces.contains(e->twin->polygon))
        boundaryEdges << e;
    }
  }
  return boundaryEdges;
}

QSet<Face *> getFaces(Mesh *mesh, QSet<int> faceIndices) {
  QSet<Face *> faces;
  foreach (int faceIndex, faceIndices)
    faces << &mesh->Faces[faceIndex];
  return faces;
}

QSet<int> getIndices(QSet<Face *> faces) {
  QSet<int> faceIndices;
  foreach (Face *f, faces)
    faceIndices << f->index;
  return faceIndices;
}

QSet<Face *> getPadded(QSet<Vertex *> inputVertices, int n) {
  QSet<Vertex *> processedVertices, unprocessedVertices;
  QSet<Face *> processedFaces, unprocessedFaces;
  foreach (Vertex *v, inputVertices)
    unprocessedVertices << v;

  // Compute n rings
  for (int i = 0; i < n; ++i) {
    // Get faces surrounding vertices
    unprocessedFaces.clear();
    foreach (Vertex *v, unprocessedVertices) {
      processedVertices << v;
      foreach (HalfEdge *e, getVertexEdges(v->out)) {
        if (e->polygon && !processedFaces.contains(e->polygon))
          unprocessedFaces << e->polygon;
      }
    }

    // Get vertices of faces
    unprocessedVertices.clear();
    foreach (Face *f, unprocessedFaces) {
      processedFaces << f;
      foreach (HalfEdge *e, getFaceEdges(f->side)) {
        if (!processedVertices.contains(e->target))
          unprocessedVertices << e->target;
      }
    }
  }

  return processedFaces;
}

QHash<int,QSet<Face *>> getColorAffectedFaces(QSet<int> selectedEdges, Mesh &mesh, int level) {
    QHash<int, QSet<Face *>> faces;
    foreach (int j, selectedEdges) {
        int edgeIndex = j * pow(4, level);
        HalfEdge *limitEdge = &mesh.HalfEdges[edgeIndex];
        Vertex *v = limitEdge->prev->target;
        QSet<Face *> affectedFaces = getPadded(QSet<Vertex *>({v}), 3 * pow(2, level + 1));
        faces[edgeIndex] = affectedFaces;
    }
    return faces;
}

bool hasIrregularDirectNeighbour(Face f) {
  foreach (HalfEdge *e, getFaceEdges(f.side)) {
    if (e->twin->polygon && e->twin->polygon->val != 4)
      return true;
  }
  return false;
}
