#ifndef SUBDIVISION_H
#define SUBDIVISION_H

#include "mesh.h"

bool isRegularVertex(HalfEdge *inputEdge);
bool isRegularFace(Face f);
QVector2D computeMeanFaceCoords(HalfEdge *inputEdge);
QVector3D computeMeanFaceColor(HalfEdge *inputEdge);
QVector2D computeEdgeMidpointCoords(HalfEdge *e);
QVector3D computeEdgeMidpointColor(HalfEdge *e);
Mesh computeLimitMesh(Mesh inputMesh);

QVector2D computeLimitPointCoords(HalfEdge *e);
QVector3D computeLimitPointColor(HalfEdge *e);

QVector2D computeInvertedLimitPointCoords(Vertex *v, QVector2D limitPoint);
void subdivideTernaryStep(Mesh *inputMesh, Mesh *subdivMesh);
void subdivideCatmullClark(Mesh *inputMesh, Mesh *subdivMesh);
void computeSubMesh(Mesh *inputMesh, QSet<int> inputFaceIndices, Mesh *outputMesh, QHash<int, int> *outputEdgeMap);

#endif // SUBDIVISION_H
