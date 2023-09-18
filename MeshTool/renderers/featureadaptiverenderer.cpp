#include "featureadaptiverenderer.h"
#include "tools/tools.h"
#include <QVector3D>
#include <QSet>
#include <QtMath>
#include <QStack>

FeatureAdaptiveRenderer::FeatureAdaptiveRenderer(QOpenGLFunctions_4_1_Core *functions) : SurfaceRenderer(functions) {
    renderers["ACC1"] = new ACC1Renderer(functions);
    renderers["ACC2"] = new ACC2Renderer(functions);
    renderers["TP"] = new TransitionPatchRenderer(functions);
}

QSet<int> FeatureAdaptiveRenderer::computeAffectedFaces(Mesh *curMesh, QHash<int, QHash<int, CoordsEdit>> coordsEdits, QHash<int, QHash<int, ColorEdit>> colorEdits) {
    QSet<int> affectedEdges;

    // Coordinate edits
    foreach (int level, coordsEdits.keys()) {
        if (level == 0)
            continue;
        foreach (const CoordsEdit edit, coordsEdits[level]) {
            foreach (int edgeIndex, edit.affectedEdgeIndices)
                affectedEdges << computeParentEdgeIndex(edgeIndex, level);
        }
    }

    // Color edits
    foreach (int level, colorEdits.keys()) {
        if (level == 0)
            continue;
        foreach (const ColorEdit edit, colorEdits[level]) {
            foreach (int edgeIndex, edit.affectedEdgeIndices)
                affectedEdges << computeParentEdgeIndex(edgeIndex, level);
        }
    }

    // Convert affected edges to affected faces
    QSet<int> affectedFaces;
    foreach (int affectedEdge, affectedEdges)
        affectedFaces << curMesh->HalfEdges[affectedEdge].polygon->index;

    // Add non-quad faces, and faces with non-quad neighbours
    foreach (Face f, curMesh->Faces) {
        if (f.val != 4 || hasIrregularDirectNeighbour(f))
            affectedFaces << f.index;
    }
    return affectedFaces;
}

QSet<int> computeIrregularityCascadedFaces(Mesh *curMesh, QSet<int> faces) {
    QSet<int> cascadedFaces;
    QStack<int> unprocessedFaces;
    foreach (int faceIndex, faces)
        unprocessedFaces.push(faceIndex);

    while (!unprocessedFaces.isEmpty()) {
        int faceIndex = unprocessedFaces.pop();
        cascadedFaces << faceIndex;
        foreach (HalfEdge *faceEdge, getFaceEdges(curMesh->Faces[faceIndex].side)) {
            if (isRegularVertex(faceEdge))
                continue;
            // Add faces surrounding vertex
            foreach (HalfEdge *vertexEdge, getVertexEdges(faceEdge)) {
                if (vertexEdge->polygon && !cascadedFaces.contains(vertexEdge->polygon->index))
                    unprocessedFaces.push(vertexEdge->polygon->index);
            }
        }
    }
    return cascadedFaces;
}

QSet<int> FeatureAdaptiveRenderer::computePaddedFaces(Mesh mesh, QSet<int> inputFaces) {
    // Collect unique vertices of input faces
    QSet<int> vertices;
    foreach (int i, inputFaces) {
        Face f = mesh.Faces[i];
        HalfEdge *e = f.side;
        for (int j = 0; j < f.val; ++j) {
            vertices << e->target->index;
            e = e->next;
        }
    }
    // Collect unique faces adjacent to vertices
    QSet<int> paddedFaces;
    foreach (int i, vertices) {
        Vertex v = mesh.Vertices[i];
        HalfEdge *e = v.out;
        for (int j = 0; j < v.val; ++j) {
            if (e->polygon)
                paddedFaces << e->polygon->index;
            e = e->prev->twin;
        }
    }
    return paddedFaces;
}

QSet<int> FeatureAdaptiveRenderer::computeTransitionFaces(Mesh mesh, QSet<int> affectedFaces) {
    QSet<int> transitionFaces;
    foreach (int faceIndex, affectedFaces) {
        Face f = mesh.Faces[faceIndex];
        foreach (HalfEdge *e, getFaceEdges(f.side)) {
            if (e->twin->polygon)
                transitionFaces << e->twin->polygon->index;
        }
    }
    transitionFaces -= affectedFaces;
    return transitionFaces;
}

QSet<int> FeatureAdaptiveRenderer::computeTransitionEdges(Face f, QSet<int> affectedFaces) {
    QSet<int> transitionEdges;
    foreach (HalfEdge *e, getFaceEdges(f.side)) {
        if (e->twin->polygon && affectedFaces.contains(e->twin->polygon->index))
            transitionEdges << e->index;
    }
    return transitionEdges;
}

void FeatureAdaptiveRenderer::setMesh(Mesh inputMesh, QHash<int, QHash<int, CoordsEdit>> coordsEdits, QHash<int, QHash<int, ColorEdit>> colorEdits) {
    // Initialize
    //  QVector<float> dataACC1, dataACC2; // Quads only
    dataACC1.clear();
    dataACC2.clear();
    dataACC1Indices.clear();
    dataACC2Indices.clear();
    int ACC1index = 0;

    ((TransitionPatchRenderer *) renderers["TP"])->clearControlPoints();
    Mesh curMesh = inputMesh;
    QSet<int> paddedFacesCur;

    for (int curLevel = 0; curMesh.Faces.size() > 0; ++curLevel) {
        // Apply edits of current level
        curMesh = computeEditedMesh(curMesh, coordsEdits[0], colorEdits[0]);

        // Compute faces affected by non-regularity or higher level edits
        QSet<int> affectedFacesCur = computeAffectedFaces(&curMesh, coordsEdits, colorEdits);
        affectedFacesCur = computeIrregularityCascadedFaces(&curMesh, affectedFacesCur);
        QSet<int> transitionFacesCur = computeTransitionFaces(curMesh, affectedFacesCur);

        // Render remaining faces
        foreach (Face f, curMesh.Faces) {
            if (!(affectedFacesCur.contains(f.index) || paddedFacesCur.contains(f.index))) {
                if (transitionFacesCur.contains(f.index))
                    ((TransitionPatchRenderer *) renderers["TP"])->addControlPoints(f, computeTransitionEdges(f, affectedFacesCur));
                else if (isRegularFace(f)) {
                    ACC1Renderer::addControlPoints(f, &dataACC1);
                    dataACC1Indices[f.index] = ACC1index ++;
                }
                else
                    ACC2Renderer::addControlPoints(f, &dataACC2);
            }
        }

        // Padd affected faces (to ensure correct subdivision)
        QSet<int> affectedFacesCurPadded = computePaddedFaces(curMesh, affectedFacesCur);

        // Compute submesh of affected (and padded) faces
        Mesh subMesh;
        QHash<int, int> currentToSubEdgeMap;
        computeSubMesh(&curMesh, affectedFacesCurPadded, &subMesh, &currentToSubEdgeMap);

        // Map higher level edits to subdivided mesh (note that from submesh to subdivided mesh only changes interpretation)
        QHash<int, QHash<int, CoordsEdit>> coordsEditsSubdiv;
        foreach (int level, coordsEdits.keys()) {
            if (level == 0)
                continue;
            foreach (int vertexIndex, coordsEdits[level].keys()) {
                CoordsEdit edit = coordsEdits[level][vertexIndex];
                int curEdgeIndex = computeParentEdgeIndex(edit.edgeIndex, level);
                int subEdgeIndex = currentToSubEdgeMap[curEdgeIndex];
                edit.edgeIndex = swapParentEdgeIndex(edit.edgeIndex, subEdgeIndex, level);
                QVector<int> newAffectedEdgeIndices;
                for (int i = 0; i < edit.affectedEdgeIndices.size(); ++i) {
                    int curEdgeIdx = computeParentEdgeIndex(edit.affectedEdgeIndices[i], level);
                    int subEdgeIdx = currentToSubEdgeMap[curEdgeIdx];
                    edit.affectedEdgeIndices[i] = swapParentEdgeIndex(edit.affectedEdgeIndices[i], subEdgeIdx, level);
                }
                // vertexIndex is now wrong but that's irrelevant
                coordsEditsSubdiv[level - 1][vertexIndex] = edit;
            }
        }
        QHash<int, QHash<int, ColorEdit>> colorEditsSubdiv;
        foreach (int level, colorEdits.keys()) {
            if (level == 0)
                continue;
            foreach (int vertexIndex, colorEdits[level].keys()) {
                ColorEdit edit = colorEdits[level][vertexIndex];
                int curEdgeIdx = computeParentEdgeIndex(edit.edgeIndex, level);
                int subEdgeIdx = currentToSubEdgeMap[curEdgeIdx];
                edit.edgeIndex = swapParentEdgeIndex(edit.edgeIndex, subEdgeIdx, level);
                for (int i = 0; i < edit.affectedEdgeIndices.size(); ++i) {
                    int curEdgeIdx = computeParentEdgeIndex(edit.affectedEdgeIndices[i], level);
                    int subEdgeIdx = currentToSubEdgeMap[curEdgeIdx];
                    edit.affectedEdgeIndices[i] = swapParentEdgeIndex(edit.affectedEdgeIndices[i], subEdgeIdx, level);
                }
                // vertexIndex is now wrong but that's irrelevant
                colorEditsSubdiv[level - 1][vertexIndex] = edit;
            }
        }

        // Catmull-Clark subdivide
        Mesh subdivMesh;
        subdivideCatmullClark(&subMesh, &subdivMesh);

        // Update padded faces
        QSet<int> paddedFacesSubdiv;
        foreach (int curFaceIndex, affectedFacesCurPadded - affectedFacesCur) {
            // Map face from current mesh to submesh (using mapping of face sides)
            Face *subFace = subMesh.HalfEdges[currentToSubEdgeMap[curMesh.Faces[curFaceIndex].side->index]].polygon;

            // Map halfedges of submesh to faces of subdivmesh
            HalfEdge *e = subFace->side;
            for (int i = 0; i < subFace->val; ++i) {
                paddedFacesSubdiv << e->index;
                e = e->next;
            }
        }

        // Update
        curMesh = subdivMesh;
        coordsEdits = coordsEditsSubdiv;
        colorEdits = colorEditsSubdiv;
        paddedFacesCur = paddedFacesSubdiv;
    }

    // Set coordinates and colors
    ((ACC1Renderer *) renderers["ACC1"])->setData(dataACC1);
    ((ACC2Renderer *) renderers["ACC2"])->setData(QVector<float>(), dataACC2);
    ((TransitionPatchRenderer *) renderers["TP"])->setData();
}

void FeatureAdaptiveRenderer::render() {
    renderers["ACC1"]->render();
    renderers["ACC2"]->render();
    renderers["TP"]->render();
}

QHash<QString, int> FeatureAdaptiveRenderer::getCountInfo() {
    QHash<QString, int> countInfo;

    QHash<QString, int> countInfoACC1 = renderers["ACC1"]->getCountInfo();
    QHash<QString, int> countInfoACC2 = renderers["ACC2"]->getCountInfo();
    QHash<QString, int> countInfoTP = renderers["TP"]->getCountInfo();

    countInfo["ACC1 control points"] = countInfoACC1["control points"];
    countInfo["ACC2 control points"] = countInfoACC2["control points"];
    countInfo["ACC1 TP control points"] = countInfoTP["ACC1 control points"];
    countInfo["ACC2 TP control points"] = countInfoTP["ACC2 control points"];
    countInfo["ACC1 faces"] = countInfoACC1["faces"];
    countInfo["ACC2 faces"] = countInfoACC2["faces"];
    countInfo["ACC1 TP faces"] = countInfoTP["ACC1 faces"];
    countInfo["ACC2 TP faces"] = countInfoTP["ACC2 faces"];
    countInfo["ACC1 TP invocations"] = countInfoTP["ACC1 invocations"];
    countInfo["ACC2 TP invocations"] = countInfoTP["ACC2 invocations"];
    countInfo["control points"] = countInfo["ACC1 control points"] + countInfo["ACC2 control points"] + countInfo["ACC1 TP control points"] + countInfo["ACC2 TP control points"];
    countInfo["faces"] = countInfo["ACC1 faces"] + countInfo["ACC2 faces"] + countInfo["ACC1 TP faces"] + countInfo["ACC2 TP faces"];
    countInfo["invocations"] = countInfoACC1["faces"] + countInfoACC2["faces"] + countInfo["ACC1 TP invocations"] + countInfo["ACC2 TP invocations"];

    return countInfo;
}
