#ifndef EDITING_H
#define EDITING_H

#include "mesh.h"
#include "coordsedit.h"
#include "coloredit.h"

bool isSelfIntersecting(Face *f);
bool isSelfIntersecting(Mesh *inputMesh, int selectedVertex);
int swapParentEdgeIndex(int childIndex, int parentIndex, int levels);
int computeParentEdgeIndex(int index, int levels);

Mesh computeEditedMesh(Mesh inputMesh, QHash<int, CoordsEdit> coordsEdits, QHash<int, ColorEdit> colorEdits);

void updateOriginalCoords(Mesh* originalMesh, Mesh* editedMesh, int selectedVertex, int level);
void updateEditedCoords(Mesh& originalMesh, Mesh& editedMesh, CoordsEdit& coordsEdit, int selectedVertex, int editFlag, QVector<int>& influencedFacesIndices, int level, bool curSubdivStep);
void updateLimitCoords(Mesh &editedMesh, Mesh &limitMesh, int selectedVertex, QVector<int>& changedLimitCoordsIndices, int level, bool curSubdivStep);

void updateOriginalColor(Mesh* originalMesh, Mesh* editedMesh, QSet<int>& selectedEdges, int level, QHash<int, QSet<Face *>> affectedFaces);
void updateEditedColor(Mesh& originalMesh, Mesh& editedMesh, QHash<int, ColorEdit>& colorEdits, QSet<int>& selectedEdges, int level, QHash<int, QSet<Face *>> affectedFaces, bool curSubdivStep, QVector<int>& changedFacesIndices);
void updateLimitMeshColor(Mesh& editedMesh, Mesh& limitMesh, QHash<int, ColorEdit>& colorEdits, QSet<int>& selectedEdges, QVector<int>& changedEdgesIndices, int level, QHash<int, QSet<Face *>> affectedFaces, bool curSubdivStep);

QSet<Face *> computeColorEditAffectedFaces(HalfEdge *inputEdge);
QSet<Face *> computeColorEditAffectedFaces(HalfEdge *inputEdge, int level);
QVector<int> getEditableVertexIndices(int inputMeshSize, Mesh& editedMesh);
QVector<int> getGradientVertexIndices(int inputMeshSize, Mesh editedMesh);

#endif // EDITING_H
