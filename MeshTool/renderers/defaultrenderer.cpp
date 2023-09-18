#include "defaultrenderer.h"
#include "tools/tools.h"
#include "qvector5d.h"
#include <QVector3D>
#include <QElapsedTimer>

DefaultRenderer::DefaultRenderer(QOpenGLFunctions_4_1_Core *functions) : SurfaceRenderer(functions) {
    // Create shader program
    shaderPrograms["default"] = new QOpenGLShaderProgram();
    shaderPrograms["default"]->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/default/vertshader.glsl");
    shaderPrograms["default"]->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshadershared.glsl");
    shaderPrograms["default"]->link();

    // Initialize OpenGL
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex((unsigned int) -1);

    // Create VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Create VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(0); // Coordinates
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glEnableVertexAttribArray(1); // Colors
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (2 * sizeof(float)));

    // Create IBO
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    // Release
    glBindVertexArray(0);

}

DefaultRenderer::~DefaultRenderer() {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &IBO);
    glDeleteVertexArrays(1, &VAO);
}

void DefaultRenderer::setData(QVector<float> data) {
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * data.size(), data.data(), GL_DYNAMIC_DRAW);
};

void DefaultRenderer::setIndices(QVector<int> indices) {
    indicesSize = indices.size();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);
}

void DefaultRenderer::setMesh(Mesh& mesh) {
    // Initialize
    data.clear();
    indices.clear();
    vertexIndices.clear();
    edgesIndices.clear();
    int curIndex = 0;
    int index = 0;

    // Collect data
    foreach (Face f, mesh.Faces) {
        foreach (HalfEdge *e, getFaceEdges(f.side)) {
            data << QVector5D(e->prev->target->coords, e->color);
            indices << curIndex++;
            //            vertexIndices << e->prev->target->index;
            vertexIndices[e->prev->target->index].append(index);
            edgesIndices[e->index] = index;
            index += 5;
        }
        indices << (unsigned int) -1;
    }

    // Set data
    setData(data);
    setIndices(indices);
    controlPointsSize = data.size() / 5;
    faces = mesh.Faces.size();
}

void DefaultRenderer::updateMeshCoords(Mesh& mesh, QVector<int>& changedLimitCoordsIndices) {

    foreach (int i, changedLimitCoordsIndices) {
        foreach (int v, vertexIndices[i]) {
            data[v] = mesh.Vertices[i].coords.x();
            data[v + 1] = mesh.Vertices[i].coords.y();
        }
    }

    // Set data
    setData(data);
    setIndices(indices);
    controlPointsSize = data.size() / 5;
    faces = mesh.Faces.size();
}

void DefaultRenderer::updateMeshColors(Mesh& mesh, QVector<int>& changedEdgesIndices) {

    foreach (int i, changedEdgesIndices) {
        HalfEdge *e = &mesh.HalfEdges[i];
        int index = i;
        int eIndex = edgesIndices[index];
        data[eIndex + 2] = e->color.x();
        data[eIndex + 3] = e->color.y();
        data[eIndex + 4] = e->color.z();
    }
    // Set data
    setData(data);
    setIndices(indices);
    controlPointsSize = data.size() / 5;
    faces = mesh.Faces.size();
}

void DefaultRenderer::render() {
    // Bind
    glBindVertexArray(VAO);
    shaderPrograms["default"]->bind();

    // Draw
    if (showWireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_LINE_LOOP, indicesSize, GL_UNSIGNED_INT, 0);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawElements(GL_TRIANGLE_FAN, indicesSize, GL_UNSIGNED_INT, 0);
    }

    // Release
    shaderPrograms["default"]->release();
    glBindVertexArray(0);
}

QHash<QString, int> DefaultRenderer::getCountInfo() {
    QHash<QString, int> countInfo;
    countInfo["faces"] = faces;
    countInfo["control points"] = controlPointsSize;
    return countInfo;
}
