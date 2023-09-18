#include "acc1renderer.h"
#include "tools/tools.h"
#include <QVector3D>
#include <QElapsedTimer>

ACC1Renderer::ACC1Renderer(QOpenGLFunctions_4_1_Core *functions) : SurfaceRenderer(functions) {
    // Create shader program
    shaderPrograms["ACC1"] = new QOpenGLShaderProgram();
    shaderPrograms["ACC1"]->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/ACC1/vertshader.glsl");
    shaderPrograms["ACC1"]->addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/shaders/ACC1/controlshader.glsl");
    shaderPrograms["ACC1"]->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/shaders/ACC1/evalshader.glsl");
    shaderPrograms["ACC1"]->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshadershared.glsl");
    shaderPrograms["ACC1"]->link();

    // Create VAO
    functions->glGenVertexArrays(1, &VAO);
    functions->glBindVertexArray(VAO);

    // Create VBO
    functions->glGenBuffers(1, &VBO);
    functions->glBindBuffer(GL_ARRAY_BUFFER, VBO);
    functions->glEnableVertexAttribArray(0);
    functions->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    functions->glEnableVertexAttribArray(1);
    functions->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (2 * sizeof(float)));

    // Release
    functions->glBindVertexArray(0);
}

ACC1Renderer::~ACC1Renderer() {
    functions->glDeleteBuffers(1, &VBO);
    functions->glDeleteVertexArrays(1, &VAO);
}

void ACC1Renderer::setData(QVector<float> data) {
    functions->glBindBuffer(GL_ARRAY_BUFFER, VBO);
    functions->glBufferData(GL_ARRAY_BUFFER, sizeof(float) * data.size(), data.data(), GL_DYNAMIC_DRAW);
    controlPointsSize = data.size() / 5;
};

void ACC1Renderer::setMesh(Mesh& mesh) {
    data.clear();
    facesIndices.clear();
    int index = 0;

    // Collect data
    //  QVector<float> data;
    foreach (Face f, mesh.Faces) {
        if (f.val == 4) {
            addControlPoints(f, &data);
            facesIndices[f.index] = index ++;
        }
    }
    // Set data
    setData(data);
}

void ACC1Renderer::updateMeshCoords(Mesh& mesh, QVector<int>& influencedFacesIndices) {
    foreach (int i, influencedFacesIndices) {
        if (mesh.Faces[i].val == 4)
            updateControlPoints(mesh.Faces[i], data, facesIndices[i], 1);
    }

    // Set data
    setData(data);
}

void ACC1Renderer::updateMeshColors(Mesh& mesh, QVector<int>& influencedFacesIndices) {
    foreach (int i, influencedFacesIndices) {
        if (mesh.Faces[i].val == 4)
            updateControlPoints(mesh.Faces[i], data, facesIndices[i], 2);
    }

    // Set data
    setData(data);
}

void ACC1Renderer::render() {
    // Bind
    functions->glBindVertexArray(VAO);
    shaderPrograms["ACC1"]->bind();

    // Draw
    functions->glPatchParameteri(GL_PATCH_VERTICES, 16);
    functions->glPolygonMode(GL_FRONT_AND_BACK, showWireframe ? GL_LINE : GL_FILL);
    functions->glDrawArrays(GL_PATCHES, 0, controlPointsSize);

    // Release
    shaderPrograms["ACC1"]->release();
    functions->glBindVertexArray(0);
}

QHash<QString, int> ACC1Renderer::getCountInfo() {
    QHash<QString, int> countInfo;
    countInfo["faces"] = controlPointsSize / 16;
    countInfo["control points"] = controlPointsSize;
    return countInfo;
}

QVector5D ACC1Renderer::computeInteriorPoint(HalfEdge *inputEdge) {
    Vertex *v = inputEdge->prev->target;
    int n = v->val;

    // Assign coords
    QVector2D coords;
    if (!isBoundaryVertex(v)) {
        // Non-boundary case (ACC1 paper figure 4a)
        coords = (n * inputEdge->prev->target->coords + 2 * inputEdge->target->coords + inputEdge->next->target->coords + 2 * inputEdge->next->next->target->coords) / (n + 5);
    } else if (n == 2) {
        // Corner case (ACC1 paper figure 21b)
        coords = (4 * inputEdge->prev->target->coords + 2 * inputEdge->target->coords + inputEdge->next->target->coords + 2 * inputEdge->next->next->target->coords) / 9;
    } else {
        // Other boundary case (ACC1 paper figure 21a)
        int k = n - 1;
        coords = (2 * k * inputEdge->prev->target->coords + 2 * inputEdge->target->coords + inputEdge->next->target->coords + 2 * inputEdge->next->next->target->coords) / (5 + 2 * k);
    }

    // Assign color
    QVector3D color;
    if (isSmoothVertex(v)) {
        // Similar to non-boundary case
        color = (n * inputEdge->color + 2 * inputEdge->next->color + inputEdge->next->next->color + 2 * inputEdge->prev->color) / (n + 5);
    } else if (isSharpEdge(inputEdge) && isSharpEdge(inputEdge->prev)) {
        // Similar to corner case
        color = (4 * inputEdge->color + 2 * inputEdge->next->color + inputEdge->next->next->color + 2 * inputEdge->prev->color) / 9;
    } else {
        // Similar to other boundary case
        int k = getColorVertexVal(inputEdge) - 1;
        color = (2 * k * inputEdge->color + 2 * inputEdge->next->color + inputEdge->next->next->color + 2 * inputEdge->prev->color) / (5 + 2 * k);
    }

    return QVector5D(coords, color);
}

QVector5D ACC1Renderer::computeEdgePoint(HalfEdge *inputEdge, bool forward) {
    Vertex *v = inputEdge->prev->target;

    // Assign coords
    QVector2D coords;
    if (!isBoundaryEdge(inputEdge)) {
        // Non-boundary case (ACC1 paper figure 4b, section A.1)
        int n = !isBoundaryVertex(v) ? v->val : (2 * v->val - 2);
        coords = (2 * n * inputEdge->prev->target->coords + 4 * inputEdge->target->coords + inputEdge->next->target->coords + 2 * inputEdge->next->next->target->coords + 2 * inputEdge->twin->next->target->coords + inputEdge->twin->next->next->target->coords) / (2 * n + 10);
    } else {
        // Boundary case (ACC1 paper figure 20a)
        coords = (2 * inputEdge->prev->target->coords + inputEdge->target->coords) / 3;
    }

    // Assign color
    QVector3D color;
    if (!isSharpEdge(inputEdge)) {
        // Similar to non-boundary case
        int n = isSmoothVertex(v) ? getColorVertexVal(inputEdge) : 2 * (getColorVertexVal(inputEdge) - 1);
        color = (2 * n * inputEdge->color + 4 * inputEdge->next->color + inputEdge->next->next->color + 2 * inputEdge->prev->color + 2 * inputEdge->twin->next->next->color + inputEdge->twin->prev->color) / (2 * n + 10);
    } else {
        // Similar to boundary case
        color = forward ? ((2 * inputEdge->color + inputEdge->next->color) / 3) : ((2 * inputEdge->twin->next->color + inputEdge->twin->color) / 3);
    }

    return QVector5D(coords, color);
}

QVector5D ACC1Renderer::computeCornerPoint(HalfEdge *inputEdge) {
    Vertex *v = inputEdge->prev->target;
    int n = v->val;

    // Assign coords
    QVector2D coords;
    if (!isBoundaryVertex(v)) {
        // Non-boundary case (ACC1 paper figure 4c)
        coords = n * n * v->coords;
        foreach (HalfEdge *e, getVertexEdges(inputEdge))
            coords += 4 * e->target->coords + e->next->target->coords;
        coords /= n * n + 5 * n;
    } else if (n == 2) {
        // Corner case (ACC1 paper figure 20c)
        coords = v->coords;
    } else {
        // Other boundary case (ACC1 paper figure 20b)
        coords = (4 * v->coords + getCCWBoundaryEdge(inputEdge)->target->coords + getCWBoundaryEdge(inputEdge)->target->coords) / 6;
    }

    // Assign color
    QVector3D color;
    if (isSmoothVertex(v)) {
        // Similar to non-boundary case
        color = n * n * inputEdge->color;
        foreach (HalfEdge *e, getVertexEdges(inputEdge))
            color += 4 * e->next->color + e->next->next->color;
        color /= n * n + 5 * n;
    } else if (getColorVertexVal(inputEdge) == 2) {
        // Similar to corner case
        color = inputEdge->color;
    } else {
        // Similar to other boundary case
        color = (4 * inputEdge->color + getCCWSharpEdge(inputEdge->prev->twin)->twin->color + getCWSharpEdge(inputEdge)->next->color) / 6;
    }

    return QVector5D(coords, color);
}

void ACC1Renderer::addControlPoints(Face f, QVector<float> *data) {
    // Add control points per ribbon (ACC1 paper figure 2)
    foreach (HalfEdge *e, getFaceEdges(f.side)) {
        *data << computeCornerPoint(e);
        *data << computeEdgePoint(e, true);
        *data << computeEdgePoint(e->twin, false);
        *data << computeInteriorPoint(e);

    }
}

void ACC1Renderer::updateControlPoints(Face &f, QVector<float> &data, int faceIndex, int coordsOrColor) {
    int edgeIndex = 0;
    // update coords
    if (coordsOrColor == 1) {
        foreach (HalfEdge *e, getFaceEdges(f.side)) {

            QVector5D cornerpoint = computeCornerPoint(e);
            data[faceIndex * 80 + 20 * edgeIndex] = cornerpoint.x();
            data[faceIndex * 80 + 20 * edgeIndex + 1] = cornerpoint.y();

            QVector5D edgepoint = computeEdgePoint(e, true);
            data[faceIndex * 80 + 20 * edgeIndex + 5] = edgepoint.x();
            data[faceIndex * 80 + 20 * edgeIndex + 6] = edgepoint.y();

            QVector5D twinEdgepoint = computeEdgePoint(e->twin, false);
            data[faceIndex * 80 + 20 * edgeIndex + 10] = twinEdgepoint.x();
            data[faceIndex * 80 + 20 * edgeIndex + 11] = twinEdgepoint.y();

            QVector5D interiorPoint = computeInteriorPoint(e);
            data[faceIndex * 80 + 20 * edgeIndex + 15] = interiorPoint.x();
            data[faceIndex * 80 + 20 * edgeIndex + 16] = interiorPoint.y();

            edgeIndex ++;

        }
    }
    // update color
    else {
        foreach (HalfEdge *e, getFaceEdges(f.side)) {

            QVector5D cornerpoint = computeCornerPoint(e);
            data[faceIndex * 80 + 20 * edgeIndex + 2] = cornerpoint.r();
            data[faceIndex * 80 + 20 * edgeIndex + 3] = cornerpoint.g();
            data[faceIndex * 80 + 20 * edgeIndex + 4] = cornerpoint.b();

            QVector5D edgepoint = computeEdgePoint(e, true);
            data[faceIndex * 80 + 20 * edgeIndex + 7] = edgepoint.r();
            data[faceIndex * 80 + 20 * edgeIndex + 8] = edgepoint.g();
            data[faceIndex * 80 + 20 * edgeIndex + 9] = edgepoint.b();

            QVector5D twinEdgepoint = computeEdgePoint(e->twin, false);
            data[faceIndex * 80 + 20 * edgeIndex + 12] = twinEdgepoint.r();
            data[faceIndex * 80 + 20 * edgeIndex + 13] = twinEdgepoint.g();
            data[faceIndex * 80 + 20 * edgeIndex + 14] = twinEdgepoint.b();

            QVector5D interiorPoint = computeInteriorPoint(e);
            data[faceIndex * 80 + 20 * edgeIndex + 17] = interiorPoint.r();
            data[faceIndex * 80 + 20 * edgeIndex + 18] = interiorPoint.g();
            data[faceIndex * 80 + 20 * edgeIndex + 19] = interiorPoint.b();

            edgeIndex ++;

        }
    }
}
