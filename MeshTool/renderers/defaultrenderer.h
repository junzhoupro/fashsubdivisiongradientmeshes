#ifndef DEFAULTRENDERER_H
#define DEFAULTRENDERER_H

#include "surfacerenderer.h"
#include "mesh.h"
#include <QVector>

class DefaultRenderer : public SurfaceRenderer {

public:
    DefaultRenderer(QOpenGLFunctions_4_1_Core *functions);
    ~DefaultRenderer();
    void setMesh(Mesh& mesh);
    void updateMeshCoords(Mesh& mesh, QVector<int>& changedLimitCoordsIndices);
    void updateMeshColors(Mesh& mesh, QVector<int>& changedEdgesIndices);
    void render();
    QHash<QString, int> getCountInfo();

private:
    void setData(QVector<float> data);
    void setIndices(QVector<int> indices);

    GLuint VAO, VBO, IBO;
    int indicesSize;
    int controlPointsSize;
    int faces;

    QHash<int, QVector<int>> vertexIndices;
    QHash<int, int> edgesIndices;
    QVector<float> data;
    QVector<int> indices;
};

#endif // DEFAULTRENDERER_H
