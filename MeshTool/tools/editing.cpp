#include "editing.h"
#include "convenience.h"
#include "subdivision.h"
#include "renderers/acc1renderer.h"
#include <QtMath>
#include <QStack>
#include <QElapsedTimer>

bool isSelfIntersecting(Face *f) {
    // Collect ACC1 control points per ribbon
    QVector<QVector2D> controlPointsCoords;
    foreach (HalfEdge *e, getFaceEdges(f->side)) {
        controlPointsCoords << ACC1Renderer::computeCornerPoint(e).coords();
        controlPointsCoords << ACC1Renderer::computeEdgePoint(e, true).coords();
        controlPointsCoords << ACC1Renderer::computeEdgePoint(e->twin, false).coords();
        controlPointsCoords << ACC1Renderer::computeInteriorPoint(e).coords();
    }

    // Mapping from indices to grid location
    //
    // 12--10--9---8
    // |   |   |   |
    // 13--15--11--6
    // |   |   |   |
    // 14--3---7---5
    // |   |   |   |
    // 0---1---2---4

    // Collect partial derivative vectors in u direction
    QVector<QVector2D> derivativesU;
    derivativesU << controlPointsCoords[1] - controlPointsCoords[0];
    derivativesU << controlPointsCoords[2] - controlPointsCoords[1];
    derivativesU << controlPointsCoords[4] - controlPointsCoords[2];
    derivativesU << controlPointsCoords[3] - controlPointsCoords[14];
    derivativesU << controlPointsCoords[7] - controlPointsCoords[3];
    derivativesU << controlPointsCoords[5] - controlPointsCoords[7];
    derivativesU << controlPointsCoords[15] - controlPointsCoords[13];
    derivativesU << controlPointsCoords[11] - controlPointsCoords[15];
    derivativesU << controlPointsCoords[6] - controlPointsCoords[11];
    derivativesU << controlPointsCoords[10] - controlPointsCoords[12];
    derivativesU << controlPointsCoords[9] - controlPointsCoords[10];
    derivativesU << controlPointsCoords[8] - controlPointsCoords[9];

    // Collect partial derivative vectors in v direction
    QVector<QVector2D> derivativesV;
    derivativesV << controlPointsCoords[14] - controlPointsCoords[0];
    derivativesV << controlPointsCoords[13] - controlPointsCoords[14];
    derivativesV << controlPointsCoords[12] - controlPointsCoords[13];
    derivativesV << controlPointsCoords[3] - controlPointsCoords[1];
    derivativesV << controlPointsCoords[15] - controlPointsCoords[3];
    derivativesV << controlPointsCoords[10] - controlPointsCoords[15];
    derivativesV << controlPointsCoords[7] - controlPointsCoords[2];
    derivativesV << controlPointsCoords[11] - controlPointsCoords[7];
    derivativesV << controlPointsCoords[9] - controlPointsCoords[11];
    derivativesV << controlPointsCoords[5] - controlPointsCoords[4];
    derivativesV << controlPointsCoords[6] - controlPointsCoords[5];
    derivativesV << controlPointsCoords[8] - controlPointsCoords[6];

    // Determine if polygon is self-intersecting
    float magnitude1 = derivativesU[0].x() * derivativesV[0].y() - derivativesU[0].y() * derivativesV[0].x();
    foreach (QVector2D derivativeU, derivativesU) {
        foreach (QVector2D derivativeV, derivativesV) {
            // The cross product of a pair of 2D vectors is: (a,b,0)x(c,d,0) = (0,0,ad-bc)
            float magnitude = derivativeU.x() * derivativeV.y() - derivativeU.y() * derivativeV.x();
            // Check if the signs are equal (equivalent to checking signs are positive, as the polygons are left rotating)
            if (magnitude * magnitude1 <= 0)
                return true;
        }
    }
    return false;
}

bool isSelfIntersecting(Mesh *inputMesh, int selectedVertex) {
    // Settings
    int MAX_LEVEL = 5;

    // Initialize
    Mesh curMesh = *inputMesh;
    QSet<int> paddedFaceIndicesCur;

    for (int i = 0; /* specified below */; ++i) {
        // Find self-intersecting face indices in current mesh
        QSet<int> selfIntersectingFaceIndicesCur;
        if(i == 0) {
            Vertex *selectedOne = &curMesh.Vertices[selectedVertex];
            QSet<Face *> faces = getPadded(QSet<Vertex *>({selectedOne}), 2);
            foreach (Face *f, faces){
                if (!paddedFaceIndicesCur.contains(f->index) && (f->val != 4 || isSelfIntersecting(f)))
                    selfIntersectingFaceIndicesCur << f->index;
            }
        }
        else {
            foreach (Face f, curMesh.Faces)
                if (!paddedFaceIndicesCur.contains(f.index) && (f.val != 4 || isSelfIntersecting(&f)))
                    selfIntersectingFaceIndicesCur << f.index;
        }

        // Return conditions
        if (selfIntersectingFaceIndicesCur.isEmpty())
            return false;
        if (i == MAX_LEVEL)
            return true;

        // Add padding
        paddedFaceIndicesCur = getIndices(getPadded(getVertices(getFaces(&curMesh, selfIntersectingFaceIndicesCur)), 1)) - selfIntersectingFaceIndicesCur;

        // Compute submesh
        Mesh subMesh;
        QHash<int, int> edgeMap;
        computeSubMesh(&curMesh, paddedFaceIndicesCur + selfIntersectingFaceIndicesCur, &subMesh, &edgeMap);

        // Catmull-Clark subdivide
        Mesh subdivMesh;
        subdivideCatmullClark(&subMesh, &subdivMesh);

        // Map padded faces from current to subdivided mesh
        QSet<int> paddedFaceIndicesSubdiv;
        foreach (int paddedFaceIndexCur, paddedFaceIndicesCur) {
            Face *paddedFaceCur = &curMesh.Faces[paddedFaceIndexCur];

            // Map face from current mesh to submesh (using mapping of face sides)
            Face *paddedFaceSub = subMesh.HalfEdges[edgeMap[paddedFaceCur->side->index]].polygon;

            // Map edges of submesh to faces of subdivmesh
            foreach (HalfEdge *e, getFaceEdges(paddedFaceSub->side))
                paddedFaceIndicesSubdiv << e->index;
        }

        // Update
        curMesh = subdivMesh;
        paddedFaceIndicesCur = paddedFaceIndicesSubdiv;
    }
}

int swapParentEdgeIndex(int childIndex, int parentIndex, int levels) {
    QStack<int> childOffsets;
    for (int i = 0; i < levels; ++i) {
        childOffsets.push(childIndex % 4);
        childIndex /= 4;
    }
    for (int i = 0; i < levels; ++i)
        parentIndex = parentIndex * 4 + childOffsets.pop();
    return parentIndex;
}

int computeParentEdgeIndex(int index, int levels) {
    for (int i = 0; i < levels; ++i)
        index /= 4;
    return index;
}

void updateOriginalCoords(Mesh* subdivMesh, Mesh* inputMesh, int selectedVertex, int level) {
    QSet<Face *> faces = getPadded(QSet<Vertex *>({&inputMesh->Vertices[selectedVertex]}), pow(2, level));

    // Compute sum of face valences
    int sumFaceVal = 0;
    for (int i = 0; i < inputMesh->Faces.size(); ++i)
        sumFaceVal += inputMesh->Faces[i].val;

    // Compute edge to vertex mapping for vertices 'b'
    QHash<int, int> edgeVertexMapping;
    int idx = inputMesh->Vertices.size();
    for (int i = 0; i < sumFaceVal; ++i) {
        HalfEdge e = inputMesh->HalfEdges[i];
        if (e.index > e.twin->index)
            continue;
        edgeVertexMapping[e.index] = idx++;
    }


    // --- UPDATE VERTICES ---

    // Vertices 'c'

    foreach (Face *fPointer, faces) {
        Face f = *fPointer;
        // Compute mean coords and color
        QVector2D coord = computeMeanFaceCoords(f.side);

        // Assign vertex
        int idx = inputMesh->Vertices.size() + inputMesh->HalfEdges.size() / 2 + f.index;
        subdivMesh->Vertices[idx].coords = coord;
    }

    // Vertices 'b'

    foreach (Face *fPointer, faces) {
        QVector<HalfEdge *> edges = getFaceEdges(fPointer->side);
        foreach (HalfEdge *ee, edges) {
            HalfEdge e = *ee;

            if (!e.polygon)
                break;
            if (e.index > e.twin->index)
                continue;

            // Compute coordinates (average of the new neighbouring face points and its two original endpoints)
            QVector2D coord;

            if (e.polygon && e.twin->polygon) {
                coord += e.target->coords;
                coord += e.twin->target->coords;
                coord += subdivMesh->Vertices[inputMesh->Vertices.size() + inputMesh->HalfEdges.size() / 2 + e.polygon->index].coords;
                coord += subdivMesh->Vertices[inputMesh->Vertices.size() + inputMesh->HalfEdges.size() / 2 + e.twin->polygon->index].coords;
                coord /= 4;

            } else {
                coord = (e.target->coords + e.twin->target->coords) / 2;
            }

            // Assign
            int idx = edgeVertexMapping[e.index];
            subdivMesh->Vertices[idx].coords = coord;
        }
    }

    // Vertices 'a'

    foreach (Face *fPointer, faces) {
        QVector<HalfEdge *> edges = getFaceEdges(fPointer->side);
        foreach(HalfEdge *ePointer, edges) {
            Vertex v = *ePointer->target;

            // Compute coordinates
            QVector2D coord;

            HalfEdge *e = getCCWBoundaryEdge(v.out);
            if (!e->polygon) {
                if (e->twin->target->val == 2) {
                    coord = e->twin->target->coords;
                } else {
                    coord  = (e->target->coords + 6 * e->twin->target->coords + e->prev->twin->target->coords) / 8;
                }
            } else {
                QVector2D sumStarCoords, sumFaceCoords;
                QVector3D sumStarColors, sumFaceColors;
                foreach (HalfEdge *e, getVertexEdges(v.out)) {
                    sumStarCoords += e->target->coords;
                    sumStarColors += e->next->color;
                    sumFaceCoords += subdivMesh->Vertices[inputMesh->Vertices.size() + inputMesh->HalfEdges.size() / 2 + e->polygon->index].coords;
                    sumFaceColors += subdivMesh->Vertices[inputMesh->Vertices.size() + inputMesh->HalfEdges.size() / 2 + e->polygon->index].out->color;
                }
                int n = v.val;
                coord = ((n - 2) * v.coords + sumStarCoords / n + sumFaceCoords / n) / n;
            }

            // Assign
            int idx = v.index;
            subdivMesh->Vertices[idx].coords = coord;
        }
    }
}

Mesh computeEditedMesh(Mesh inputMesh, QHash<int, CoordsEdit> coordsEdits, QHash<int, ColorEdit> colorEdits) {
    Mesh outputMesh = inputMesh.copy();
    foreach (CoordsEdit ce, coordsEdits.values()) {
        if (!ce.boundary) {
            HalfEdge *e1 = &inputMesh.HalfEdges[ce.edgeIndex];
            HalfEdge *e2 = e1->prev->twin;
            Vertex v = inputMesh.Vertices[e1->twin->target->index];
            QVector2D vec1 = e1->target->coords - v.coords;
            QVector2D vec2 = e2->target->coords - v.coords;
            QVector2D deltaCoords = ce.val1 * vec1 + ce.val2 * vec2;
            outputMesh.Vertices[v.index].coords += deltaCoords;
        } else {
            HalfEdge *e1 = inputMesh.HalfEdges[ce.edgeIndex].twin;
            HalfEdge *e2 = e1->prev->twin;
            Vertex v = inputMesh.Vertices[e1->twin->target->index];
            QVector2D vec1 = e1->target->coords - v.coords;
            QVector2D vec2 = e2->target->coords - v.coords;
            // Compute angle between e1 and e2
            float alpha = atan2(vec2.y(), vec2.x()) - atan2(vec1.y(), vec1.x());
            if (alpha < 0)
                alpha += 2 * M_PI;
            // Infer angle between e1 and displacement
            float phi = alpha * ce.val1;
            // Compute displacement
            float angle = atan2(vec1.y(), vec1.x()) + phi; // Angle with respect to horizontal
            QVector2D direction(cos(angle), sin(angle));
            float length = sqrt(ce.val2 * sqrt(vec1.lengthSquared() * vec2.lengthSquared())); // Solve <equation in paper> = ce.b for delta p
            QVector2D deltaCoords = length * direction;
            // Update
            outputMesh.Vertices[v.index].coords += deltaCoords;
        }
    }

    // Assign colors to halfedges of single sector
    foreach (ColorEdit edit, colorEdits) {
        // Apply edit
        HalfEdge *editedEdge = &outputMesh.HalfEdges[edit.edgeIndex];
        editedEdge->color = edit.color;

        // Patch one ring neighbourhood
        foreach (HalfEdge *e, getVertexEdges(editedEdge)) {
            if (!e->polygon)
                continue;
            // v1
            e->next->color = e->color;
            e->next->twin->next->color = e->color;
            // v2
            e->next->next->color = e->color;
            e->next->twin->color = e->color;
            e->next->twin->prev->twin->color = e->color;
            e->next->next->twin->next->color = e->color;
            // v3
            e->prev->color = e->color;
            e->next->next->twin->color = e->color;
        }
    }
    // Assign edge sharpness
    foreach (ColorEdit edit, colorEdits) {
        HalfEdge *e = &outputMesh.HalfEdges[edit.edgeIndex];
        if (!e->twin->polygon || e->color != e->twin->next->color) {
            e->isSharp = true;
            e->twin->isSharp = true;
            e->next->twin->next->isSharp = true;
            e->next->twin->next->twin->isSharp = true;
            e->next->twin->next->next->twin->next->isSharp = true;
            e->next->twin->next->next->twin->next->twin->isSharp = true;
        }
        if (!e->prev->twin->polygon || e->color != e->prev->twin->color) {
            e->prev->isSharp = true;
            e->prev->twin->isSharp = true;
            e->prev->prev->twin->prev->isSharp = true;
            e->prev->prev->twin->prev->twin->isSharp = true;
            e->prev->prev->twin->prev->prev->twin->prev->isSharp = true;
            e->prev->prev->twin->prev->prev->twin->prev->twin->isSharp = true;
        }
    }

    return outputMesh;
}

//level = 0: at edit step
//level > 0: compute next steps of edit step
void updateEditedCoords(Mesh& originalMesh, Mesh& editedMesh, CoordsEdit& coordsEdit, int selectedVertex, int editFlag, QVector<int>& influencedFacesIndices, int level, bool curSubdivStep) {
    CoordsEdit ce = coordsEdit;
    Vertex *selectedOne = &editedMesh.Vertices[selectedVertex];

    QSet<Face *> faces = getPadded(QSet<Vertex *>({selectedOne}), pow(2, level+1));
    if(curSubdivStep) {
        foreach(Face *oneface, faces) {
            influencedFacesIndices.append(oneface->index);
        }
    }

    // Copy original points coords
    if(level > 0) {
        foreach(Face *oneface, faces) {
            HalfEdge *oneEdge = oneface->side;
            for(int i = 0; i < oneface->val; i ++) {
                Vertex *affectedVertex = oneEdge->target;
                int vertexIndex = affectedVertex->index;
                affectedVertex->coords = originalMesh.Vertices[vertexIndex].coords;
                oneEdge = oneEdge->next;
            }
            //            influencedFacesIndices.append(oneface->index);
        }
    }
    //Compute selected point coords
    else {


        if (!ce.boundary) {
            HalfEdge *e1 = &originalMesh.HalfEdges[ce.edgeIndex];
            HalfEdge *e2 = e1->prev->twin;
            Vertex v = originalMesh.Vertices[e1->twin->target->index];
            QVector2D vec1 = e1->target->coords - v.coords;
            QVector2D vec2 = e2->target->coords - v.coords;
            QVector2D deltaCoords = ce.val1 * vec1 + ce.val2 * vec2;
            editedMesh.Vertices[v.index].coords = originalMesh.Vertices[v.index].coords + deltaCoords * editFlag;
        } else {
            HalfEdge *e1 = originalMesh.HalfEdges[ce.edgeIndex].twin;
            HalfEdge *e2 = e1->prev->twin;
            Vertex v = originalMesh.Vertices[e1->twin->target->index];
            QVector2D vec1 = e1->target->coords - v.coords;
            QVector2D vec2 = e2->target->coords - v.coords;
            // Compute angle between e1 and e2
            float alpha = atan2(vec2.y(), vec2.x()) - atan2(vec1.y(), vec1.x());
            if (alpha < 0)
                alpha += 2 * M_PI;
            // Infer angle between e1 and displacement
            float phi = alpha * ce.val1;
            // Compute displacement
            float angle = atan2(vec1.y(), vec1.x()) + phi; // Angle with respect to horizontal
            QVector2D direction(cos(angle), sin(angle));
            float length = sqrt(ce.val2 * sqrt(vec1.lengthSquared() * vec2.lengthSquared())); // Solve <equation in paper> = ce.b for delta p
            QVector2D deltaCoords = length * direction;
            // Update
            editedMesh.Vertices[v.index].coords = originalMesh.Vertices[v.index].coords + deltaCoords * editFlag;
        }

    }
}

//Locally update coords
//If the user moves a point in edit step a,
//max subdivided level is b, b >= a,
//level = b-a
//level helps to make sure the affected faces in any subdivision step which is larger than the edit level.
void updateLimitCoords(Mesh &editedMesh, Mesh &limitMesh, int selectedVertex, QVector<int>& changedLimitCoordsIndices, int level, bool curSubdivStep) {
    //Update the coord of the selected vertex
    Vertex *selectedOne = &limitMesh.Vertices[selectedVertex];
    selectedOne->coords = computeLimitPointCoords(editedMesh.Vertices[selectedVertex].out);

    //Update the coords of the around vertex
    QSet<int> changedVertex;
    QSet<Face *> faces = getPadded(QSet<Vertex *>({selectedOne}), pow(2, level+1));

    if(curSubdivStep) {
        changedLimitCoordsIndices.append(selectedVertex);
        foreach(Face *oneface, faces) {
            HalfEdge *oneEdge = oneface->side;
            for(int i = 0; i < oneface->val; i ++) {
                if(oneEdge->target != selectedOne) {
                    Vertex *affectedVertex = oneEdge->target;
                    int vertexIndex = affectedVertex->index;
                    //                    affectedVertex->coords = computeLimitPointCoords(editedMesh.Vertices[vertexIndex].out);
                    changedLimitCoordsIndices.append(vertexIndex);
                }
                oneEdge = oneEdge->next;
            }
        }
    }

    foreach(Face *oneface, faces) {
        HalfEdge *oneEdge = oneface->side;
        for(int i = 0; i < oneface->val; i ++) {
            if(oneEdge->target != selectedOne) {
                Vertex *affectedVertex = oneEdge->target;
                int vertexIndex = affectedVertex->index;
                affectedVertex->coords = computeLimitPointCoords(editedMesh.Vertices[vertexIndex].out);
                changedLimitCoordsIndices.append(vertexIndex);
            }
            oneEdge = oneEdge->next;
        }
    }

}

void deleteEditedCoords(Mesh& originalMesh, Mesh& editedMesh, CoordsEdit& coordsEdit) {
    CoordsEdit *ce = &coordsEdit;
    if (!ce->boundary) {
        HalfEdge *e1 = &originalMesh.HalfEdges[ce->edgeIndex];
        HalfEdge *e2 = e1->prev->twin;
        Vertex v = originalMesh.Vertices[e1->twin->target->index];
        QVector2D vec1 = e1->target->coords - v.coords;
        QVector2D vec2 = e2->target->coords - v.coords;
        QVector2D deltaCoords = ce->val1 * vec1 + ce->val2 * vec2;

        editedMesh.Vertices[v.index].coords = originalMesh.Vertices[v.index].coords;
    } else {
        HalfEdge *e1 = originalMesh.HalfEdges[ce->edgeIndex].twin;
        HalfEdge *e2 = e1->prev->twin;
        Vertex v = originalMesh.Vertices[e1->twin->target->index];
        QVector2D vec1 = e1->target->coords - v.coords;
        QVector2D vec2 = e2->target->coords - v.coords;
        // Compute angle between e1 and e2
        float alpha = atan2(vec2.y(), vec2.x()) - atan2(vec1.y(), vec1.x());
        if (alpha < 0)
            alpha += 2 * M_PI;
        // Infer angle between e1 and displacement
        float phi = alpha * ce->val1;
        // Compute displacement
        float angle = atan2(vec1.y(), vec1.x()) + phi; // Angle with respect to horizontal
        QVector2D direction(cos(angle), sin(angle));
        float length = sqrt(ce->val2 * sqrt(vec1.lengthSquared() * vec2.lengthSquared())); // Solve <equation in paper> = ce.b for delta p
        QVector2D deltaCoords = length * direction;
        // Update
        editedMesh.Vertices[v.index].coords = originalMesh.Vertices[v.index].coords;
    }
}

void updateOriginalColor(Mesh* subdivMesh, Mesh* inputMesh, QSet<int>& selectedEdges, int level, QHash<int, QSet<Face *>> affectedFaces) {

    // Compute sum of face valences
    int sumFaceVal = 0;
    for (int i = 0; i < inputMesh->Faces.size(); ++i)
        sumFaceVal += inputMesh->Faces[i].val;

    // Compute edge to vertex mapping for vertices 'b'
    QHash<int, int> edgeVertexMapping;
    int idx = inputMesh->Vertices.size();
    for (int i = 0; i < sumFaceVal; ++i) {
        HalfEdge e = inputMesh->HalfEdges[i];
        if (e.index > e.twin->index)
            continue;
        edgeVertexMapping[e.index] = idx++;
    }

    // Vertices 'c'
    foreach (QSet<Face *> oneEdgeAffectedfaces, affectedFaces.values()) {
        foreach(Face *oneFace, oneEdgeAffectedfaces) {
            Face f = *oneFace;

            // Compute mean coords and color
            QVector3D color = computeMeanFaceColor(f.side);

            // Assign vertex
            int idx = inputMesh->Vertices.size() + inputMesh->HalfEdges.size() / 2 + f.index;

            // Assign color to halfedges
            foreach (HalfEdge *e, getVertexEdges(subdivMesh->Vertices[idx].out))
                e->color = color;
        }
    }

    // Assign color to halfedges
    foreach (QSet<Face *> oneEdgeAffectedfaces, affectedFaces.values()) {
        foreach(Face *oneFace, oneEdgeAffectedfaces) {
            foreach (HalfEdge *edge, getFaceEdges(oneFace->side)) {
                HalfEdge e = *edge;
                if (!e.polygon)
                    break;

                QVector3D color;
                if (!isSharpEdge(&e)) {
                    color += e.color;
                    color += e.next->color;
                    color += subdivMesh->Vertices[inputMesh->Vertices.size() + inputMesh->HalfEdges.size() / 2 + e.polygon->index].out->color;
                    color += subdivMesh->Vertices[inputMesh->Vertices.size() + inputMesh->HalfEdges.size() / 2 + e.twin->polygon->index].out->color;
                    color /= 4;
                } else {
                    color = (e.color + e.next->color) / 2;
                }

                subdivMesh->HalfEdges[4 * e.index + 1].color = color;
                subdivMesh->HalfEdges[4 * e.index + 2].color = color;
            }

        }

    }

    foreach (QSet<Face *> oneEdgeAffectedfaces, affectedFaces.values()) {
        foreach(Face *oneFace, oneEdgeAffectedfaces) {
            foreach (HalfEdge *edge, getFaceEdges(oneFace->side)) {
                HalfEdge e = *edge;
                if (!e.polygon)
                    break;
                Vertex *v = e.prev->target;

                QVector3D color;
                if (isSmoothVertex(v)) {
                    // Non-boundary case
                    HalfEdge *e = v->out;
                    QVector3D sumStarColors, sumFaceColors;
                    foreach (HalfEdge *e, getVertexEdges(v->out)) {
                        sumStarColors += (e->next->color + e->twin->color) / 2; // Note: Fix for dart vertices
                        sumFaceColors += subdivMesh->Vertices[inputMesh->Vertices.size() + inputMesh->HalfEdges.size() / 2 + e->polygon->index].out->color;
                    }
                    int n = v->val;
                    color = ((n - 2) * e->color + sumStarColors / n + sumFaceColors / n) / n;
                } else if (isSharpEdge(&e) && isSharpEdge(e.prev)) {
                    // Corner case
                    color = e.color;
                } else {
                    // Other boundary case
                    HalfEdge *firstEdge = getCCWSharpEdge(e.prev->twin);
                    HalfEdge *lastEdge = getCWSharpEdge(&e);
                    color = (firstEdge->twin->color + 6 * e.color + lastEdge->next->color) / 8;
                }

                // Assign
                subdivMesh->HalfEdges[4 * e.index].color = color;
            }
        }
    }
}

void updateEditedColor(Mesh& originalMesh, Mesh& editedMesh, QHash<int, ColorEdit>& colorEdits, QSet<int>& selectedEdges, int level, QHash<int, QSet<Face *>> affectedFaces, bool curSubdivStep, QVector<int>& changedFacesIndices) {
    if(curSubdivStep) {
        foreach (QSet<Face *> oneEdgeAffectedfaces, affectedFaces.values()) {
            foreach(Face *oneFace, oneEdgeAffectedfaces) {
                changedFacesIndices << oneFace->index;
            }
        }
    }
    // Copy original edges colors
    if (level > 0) {
        foreach (QSet<Face *> oneEdgeAffectedfaces, affectedFaces.values()) {
            foreach(Face *oneFace, oneEdgeAffectedfaces) {
                foreach (HalfEdge *edge, getFaceEdges(oneFace->side)) {
                    int index = edge->index;
                    edge->color = originalMesh.HalfEdges[index].color;
                }
            }
        }
    }
    //Compute selected edges colors
    else {
        foreach (int i, selectedEdges) {
            HalfEdge *editedEdge = &editedMesh.HalfEdges[i];
            editedEdge->color = colorEdits[i].color;

            // Patch one ring neighbourhood
            HalfEdge *e = editedEdge;
            if (!e->polygon)
                continue;
            // v1
            e->next->color = e->color;
            e->next->twin->next->color = e->color;
            // v2
            e->next->next->color = e->color;
            e->next->twin->color = e->color;
            e->next->twin->prev->twin->color = e->color;
            e->next->next->twin->next->color = e->color;
            // v3
            e->prev->color = e->color;
            e->next->next->twin->color = e->color;

            // Assign edge sharpness

            if (!e->twin->polygon || e->color != e->twin->next->color) {
                e->isSharp = true;
                e->twin->isSharp = true;
                e->next->twin->next->isSharp = true;
                e->next->twin->next->twin->isSharp = true;
                e->next->twin->next->next->twin->next->isSharp = true;
                e->next->twin->next->next->twin->next->twin->isSharp = true;
            }
            if (!e->prev->twin->polygon || e->color != e->prev->twin->color) {
                e->prev->isSharp = true;
                e->prev->twin->isSharp = true;
                e->prev->prev->twin->prev->isSharp = true;
                e->prev->prev->twin->prev->twin->isSharp = true;
                e->prev->prev->twin->prev->prev->twin->prev->isSharp = true;
                e->prev->prev->twin->prev->prev->twin->prev->twin->isSharp = true;
            }

        }
    }
}

void updateLimitMeshColor(Mesh& editedMesh, Mesh& limitMesh, QHash<int, ColorEdit>& colorEdits, QSet<int>& selectedEdges, QVector<int>& changedEdgesIndices, int level, QHash<int, QSet<Face *>> affectedFaces, bool curSubdivStep) {

    if (curSubdivStep) {
        foreach(QSet<Face *> ff, affectedFaces.values()) {
            foreach(Face *f, ff) {
                if(f->index >=0) {
                    HalfEdge *edge = f->side;

                    foreach (HalfEdge *fe, getFaceEdges(edge)) {

                        changedEdgesIndices.append(fe->index);
                    }
                }
            }

        }
    }
    foreach(QSet<Face *> ff, affectedFaces.values()) {
        foreach(Face *f, ff) {
            if(f->index >=0) {
                HalfEdge *edge = f->side;

                foreach (HalfEdge *fe, getFaceEdges(edge)) {
                    fe->color = computeLimitPointColor(&editedMesh.HalfEdges[fe->index]);
                }
            }
        }

    }

}

QSet<Face *> computeColorEditAffectedFaces(HalfEdge *inputEdge) {
    QSet<Face *> affectedFaces;
    Face *centerFace = inputEdge->next->twin->prev->twin->polygon;
    affectedFaces << centerFace;
    for (int i = 0; i < centerFace->val; ++i) {
        affectedFaces << inputEdge->polygon;
        inputEdge = inputEdge->next->twin->next;
        affectedFaces << inputEdge->polygon;
        inputEdge = inputEdge->next->twin->next->next;
    }
    return affectedFaces;
}

QSet<Face *> computeColorEditAffectedFaces(HalfEdge *inputEdge, int level) {
    int facesALine = 3*pow(2, level);
    QSet<Face *> affectedFaces;
    HalfEdge *edge = inputEdge;
    for(int i = 0; i < facesALine; i ++) {
        HalfEdge *e = edge;
        for(int j = 0; j < facesALine; j ++) {
            affectedFaces << e->polygon;
            e = e->next->twin->next;
        }
        edge = edge->next->next->twin;
    }
    return affectedFaces;
}

// Get the points for which both the geometry and colour can be edited
QVector<int> getEditableVertexIndices(int inputMeshSize, Mesh& editedMesh) {
    QElapsedTimer timer;
    timer.start();
    QStack<int> stack;
    QVector<bool> processed(editedMesh.Vertices.size());

    for (int i = 0; i < inputMeshSize; ++i) {
        stack.push(i);
        processed[i] = true;
    }

    // Perform depth first search, walking in steps of three consecutive edges
    while (!stack.isEmpty()) {
        int vertexIndex = stack.pop();
        Vertex v = editedMesh.Vertices[vertexIndex];
        HalfEdge *e = v.out;
        for (int i = 0; i < v.val; ++i) {
            int newVertexIndex = -1;
            if (!e->polygon)
                newVertexIndex = e->next->next->target->index;
            else
                newVertexIndex = e->next->twin->next->next->twin->next->target->index;
            if (!processed[newVertexIndex]) {
                stack.push(newVertexIndex);
                processed[newVertexIndex] = true;
            }
            e = e->prev->twin;
        }
    }
    // Processed vertices are the editable vertices
    QVector<int> indices;
    for (int i = 0; i < processed.size(); ++i) {
        if (processed[i])
            indices.append(i);
    }

    return indices;
}

// Get the points for which only geometry can be edited
QVector<int> getGradientVertexIndices(int inputMeshSize, Mesh editedMesh) {
    // Get editable vertex indices
    QVector<int> editableVertices = getEditableVertexIndices(inputMeshSize, editedMesh);

    // Create bool array indicating if a vertex is editable
    QVector<bool> isEditableVertex(editedMesh.Vertices.size());
    for (int i = 0; i < editableVertices.size(); ++i)
        isEditableVertex[editableVertices[i]] = true;

    // Remaining vertices are gradient vertices
    QVector<int> gradientVertexIndices;
    for (int i = 0; i < editedMesh.Vertices.size(); ++i) {
        if (!isEditableVertex[i])
            gradientVertexIndices.append(i);
    }

    return gradientVertexIndices;
}


