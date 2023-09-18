#ifndef CONVENIENCE_H
#define CONVENIENCE_H

#include "mesh.h"

bool isBoundaryVertex(Vertex *v);
bool isBoundaryEdge(HalfEdge *e);
bool isSmoothVertex(Vertex *v);
bool isSharpEdge(HalfEdge *e);
HalfEdge *getCWBoundaryEdge(HalfEdge *e);
HalfEdge *getCCWBoundaryEdge(HalfEdge *e);
HalfEdge *getCWSharpEdge(HalfEdge *e);
HalfEdge *getCCWSharpEdge(HalfEdge *e);
int getColorVertexVal(HalfEdge *e);
QVector<HalfEdge *> getVertexEdges(HalfEdge *e);
QVector<HalfEdge *> getFaceEdges(HalfEdge *e);
QSet<Vertex *> getVertices(QSet<Face *> inputFaces);
QSet<HalfEdge *> getBoundaryEdges(QSet<Face *> inputFaces);
QSet<Face *> getFaces(Mesh *mesh, QSet<int> faceIndices);
QSet<int> getIndices(QSet<Face *> faces);
QSet<Face *> getPadded(QSet<Vertex *> inputVertices, int n);
QHash<int,QSet<Face *>> getColorAffectedFaces(QSet<int> selectedEdges, Mesh &mesh, int level);
bool hasIrregularDirectNeighbour(Face f);

#endif // CONVENIENCE_H
