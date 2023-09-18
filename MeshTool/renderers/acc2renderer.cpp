#include "acc2renderer.h"
#include "tools/tools.h"
#include <QVector3D>
#include <QtMath>

ACC2Renderer::ACC2Renderer(QOpenGLFunctions_4_1_Core *functions) : SurfaceRenderer(functions) {
    // Create quads shader program
    shaderPrograms["quads"] = new QOpenGLShaderProgram();
    shaderPrograms["quads"]->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/ACC2/vertshader.glsl");
    shaderPrograms["quads"]->addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/shaders/ACC2/quads/controlshader.glsl");
    shaderPrograms["quads"]->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/shaders/ACC2/quads/evalshader.glsl");
    shaderPrograms["quads"]->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshadershared.glsl");
    shaderPrograms["quads"]->link();

    // Create triangles shader program
    shaderPrograms["triangles"] = new QOpenGLShaderProgram();
    shaderPrograms["triangles"]->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/ACC2/vertshader.glsl");
    shaderPrograms["triangles"]->addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/shaders/ACC2/triangles/controlshader.glsl");
    shaderPrograms["triangles"]->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/shaders/ACC2/triangles/evalshader.glsl");
    shaderPrograms["triangles"]->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshadershared.glsl");
    shaderPrograms["triangles"]->link();

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

ACC2Renderer::~ACC2Renderer() {
    functions->glDeleteBuffers(1, &VBO);
    functions->glDeleteVertexArrays(1, &VAO);
}

void ACC2Renderer::setData(QVector<float> dataTriangles, QVector<float> dataQuads) {
    QVector<float> data = dataTriangles + dataQuads;
    controlPointsTrianglesSize = dataTriangles.size() / 5;
    controlPointsQuadsSize = dataQuads.size() / 5;
    functions->glBindBuffer(GL_ARRAY_BUFFER, VBO);
    functions->glBufferData(GL_ARRAY_BUFFER, sizeof(float) * data.size(), data.data(), GL_DYNAMIC_DRAW);
};

void ACC2Renderer::setMesh(Mesh& mesh) {
    dataTriangles.clear();
    dataQuads.clear();
    dataTrianglesIndices.clear();
    dataQuadsIndices.clear();
    int triangleIndex = 0, quadIndex = 0;

    // Collect data
    //  QVector<float> dataTriangles, dataQuads;
    foreach (Face f, mesh.Faces) {
        if (f.val == 3) {
            ACC2Renderer::addControlPoints(f, &dataTriangles);
            dataTrianglesIndices[f.index] = triangleIndex;
            triangleIndex += 75;
        }
        else if (f.val == 4) {
            ACC2Renderer::addControlPoints(f, &dataQuads);
            dataQuadsIndices[f.index] = quadIndex;
            quadIndex += 100;
        }

    }

    // Set data
    setData(dataTriangles, dataQuads);
}

void ACC2Renderer::updateMeshCoords(Mesh& mesh, QVector<int>& influencedFacesIndices) {

    foreach (int i, influencedFacesIndices) {
        if (mesh.Faces[i].val == 3)
            updateControlPoints(mesh.Faces[i], dataTriangles, dataTrianglesIndices[i], 1);
        if (mesh.Faces[i].val == 4)
            updateControlPoints(mesh.Faces[i], dataQuads, dataQuadsIndices[i], 1);
    }

    // Set data
    setData(dataTriangles, dataQuads);
}

void ACC2Renderer::updateMeshColors(Mesh& mesh, QVector<int>& influencedFacesIndices) {

    foreach (int i, influencedFacesIndices) {
        if (mesh.Faces[i].val == 3)
            updateControlPoints(mesh.Faces[i], dataTriangles, dataTrianglesIndices[i], 2);
        if (mesh.Faces[i].val == 4)
            updateControlPoints(mesh.Faces[i], dataQuads, dataQuadsIndices[i], 2);
    }

    // Set data
    setData(dataTriangles, dataQuads);
}

void ACC2Renderer::render() {
    // Bind
    functions->glBindVertexArray(VAO);
    shaderPrograms["triangles"]->bind();

    // Draw triangles
    functions->glPatchParameteri(GL_PATCH_VERTICES, 15);
    functions->glPolygonMode(GL_FRONT_AND_BACK, showWireframe ? GL_LINE : GL_FILL);
    functions->glDrawArrays(GL_PATCHES, 0, controlPointsTrianglesSize);

    // Switch shader program
    shaderPrograms["triangles"]->release();
    shaderPrograms["quads"]->bind();

    // Draw quads
    functions->glPatchParameteri(GL_PATCH_VERTICES, 20);
    functions->glPolygonMode(GL_FRONT_AND_BACK, showWireframe ? GL_LINE : GL_FILL);
    functions->glDrawArrays(GL_PATCHES, controlPointsTrianglesSize, controlPointsQuadsSize);

    // Release
    shaderPrograms["triangles"]->release();
    functions->glBindVertexArray(0);
}

QHash<QString, int> ACC2Renderer::getCountInfo() {
    QHash<QString, int> countInfo;
    countInfo["faces"] = controlPointsQuadsSize / 20 + controlPointsTrianglesSize / 15;
    countInfo["control points"] = controlPointsQuadsSize + controlPointsTrianglesSize;
    return countInfo;
}

QVector5D ACC2Renderer::computeCornerPoint(HalfEdge *inputEdge) {
    return QVector5D(computeLimitPointCoords(inputEdge), computeLimitPointColor(inputEdge));
}

QVector5D ACC2Renderer::computeEdgePoint(HalfEdge *inputEdge, QVector5D p, bool forward) {
    Vertex *v = inputEdge->prev->target;
    int n = v->val;

    // Compute coords
    QVector2D coords;
    if (!isBoundaryVertex(v)) {
        // Non-boundary case (ACC2 paper section 3.3)

        // Compute limit tangent
        QVector2D q;
        HalfEdge *e = inputEdge;
        for (int i = 0; i < n; ++i) {
            q += (1 - sigma(n) * cos(M_PI / n)) * cos(2 * M_PI * i / n) * computeEdgeMidpointCoords(e) + 2 * sigma(n) * cos((2 * M_PI * i + M_PI) / n) * computeMeanFaceCoords(e);
            e = e->prev->twin;
        }
        q *= 2.0 / n;

        // Compute coordinates
        coords = p.coords() + 2 * lambda(n) * q / 3;
    } else if (n == 2) {
        // Corner case

        // Compute limit tangent (ACC1 paper section A.2 case k=1)
        QVector2D q = inputEdge->target->coords - v->coords;

        // Compute coordinates (ACC2 paper section 3.3 formula e0+)
        coords = p.coords() + 2 * lambda(4) * q / 3;
    } else {
        // Other boundary cases (ACC1 paper section A.2 tangent vector along jth edge)

        // Compute first and last edge
        HalfEdge *firstEdge = getCCWBoundaryEdge(v->out);
        HalfEdge *lastEdge = getCWBoundaryEdge(v->out);

        // Find index j of input edge
        int j = 0;
        HalfEdge *e = lastEdge;
        while (e != inputEdge) {
            e = e->prev->twin;
            ++j;
        }

        // Compute r0
        QVector2D r0 = (lastEdge->target->coords - firstEdge->target->coords) / 2;

        // Compute c and s
        int k = n - 1;
        float c = cos(M_PI / k);
        float s = sin(M_PI / k);

        // Initialize r1 with gamma component
        QVector2D r1 = -4 * s / (3 * k + c) * v->coords;

        // Update r1 with alpha and beta components
        e = lastEdge;
        for (int i = 0; i <= k; ++i) {
            // Compute s(i) and s(i+1)
            float si = sin(M_PI * i / k);
            float si1 = sin(M_PI * (i + 1) / k);

            // Add alpha(i) components to r1
            if (i == 0 || i == k)
                r1 += -((1 + 2 * c) * sqrt(1 + c)) / ((3 * k + c) * sqrt(1 - c)) * e->target->coords;
            else
                r1 += 4 * si / (3 * k + c) * e->target->coords;

            // Add beta(i) components to r1
            if (i != k) {
                if (e->polygon->val == 4)      // Quad face
                    r1 += (si + si1) / (3 * k + c) * e->next->target->coords;
                else if (e->polygon->val == 3) // Triangle face (empirical formula, not proven)
                    r1 += (si + si1) / (3 * k + c) * (3 * (computeMeanFaceCoords(e) - v->coords) + v->coords);
                else                           // Arbitrary valency face (empirical formula, not proven)
                    r1 += (si + si1) / (3 * k + c) * (2 * (computeMeanFaceCoords(e) - v->coords) + v->coords);
            }

            // Next vertex edge
            e = e->prev->twin;
        }

        // Compute limit tangent
        QVector2D q = cos(M_PI * j / k) * r0 + sin(M_PI * j / k) * r1;

        // Compute coordinates (ACC2 paper section 3.3 formula e0+)
        coords = p.coords() + 2 * lambda(2 * k) * q / 3;
    }

    QVector3D originColor = forward ? inputEdge->color : inputEdge->twin->next->color;
    QVector3D targetColor = forward ? inputEdge->next->color : inputEdge->twin->color;
    int originValColor = getColorVertexVal(forward ? inputEdge : inputEdge->twin->next);

    // Compute color
    QVector3D color;
    if (isSharpEdge(inputEdge)) {
        // Empirical fix for darts (similar to ACC1)

        // Compute color
        color = forward ? ((2 * inputEdge->color + inputEdge->next->color) / 3) : ((2 * inputEdge->twin->next->color + inputEdge->twin->color) / 3);
    } else if (isSmoothVertex(v)) {
        // Non-boundary case (ACC2 paper section 3.3)

        // Compute limit tangent
        QVector3D q;
        HalfEdge *e = inputEdge;
        for (int i = 0; i < n; ++i) {
            q += (1 - sigma(n) * cos(M_PI / n)) * cos(2 * M_PI * i / n) * computeEdgeMidpointColor(e) + 2 * sigma(n) * cos((2 * M_PI * i + M_PI) / n) * computeMeanFaceColor(e);
            e = e->prev->twin;
        }
        q *= 2.0 / n;

        // Compute color
        color = p.color() + 2 * lambda(n) * q / 3;
    } else if (originValColor == 2) {
        // Corner case

        // Compute limit tangent (ACC1 paper section A.2 case k=1)
        QVector3D q = targetColor - originColor;

        // Compute color (ACC2 paper section 3.3 formula e0+)
        color = p.color() + 2 * lambda(4) * q / 3;
    } else {
        // Other boundary cases (ACC1 paper section A.2 tangent vector along jth edge)

        // Compute first and last edge
        HalfEdge *firstEdge = getCCWSharpEdge(forward ? inputEdge->prev->twin : inputEdge);
        HalfEdge *lastEdge = getCWSharpEdge(forward ? inputEdge : inputEdge->twin->next);

        // Find index j of input edge
        int j = 0;
        HalfEdge *e = lastEdge;
        while (e != inputEdge) {
            e = e->prev->twin;
            ++j;
        }

        // Compute r0
        QVector3D r0 = (lastEdge->next->color - firstEdge->twin->color) / 2;

        // Compute c and s
        int k = originValColor - 1;
        float c = cos(M_PI / k);
        float s = sin(M_PI / k);

        // Initialize r1 with gamma component
        QVector3D r1 = -4 * s / (3 * k + c) * originColor;

        // Update r1 with alpha and beta components
        e = lastEdge;
        for (int i = 0; i <= k; ++i) {
            // Compute s(i) and s(i+1)
            float si = sin(M_PI * i / k);
            float si1 = sin(M_PI * (i + 1) / k);

            // Add alpha(i) components to r1
            if (i == 0 || i == k)
                r1 += -((1 + 2 * c) * sqrt(1 + c)) / ((3 * k + c) * sqrt(1 - c)) * (i == 0 ? e->next->color : e->twin->color);
            else
                r1 += 4 * si / (3 * k + c) * e->next->color;

            // Add beta(i) components to r1
            if (i != k) {
                if (e->polygon->val == 4)      // Quad face
                    r1 += (si + si1) / (3 * k + c) * e->next->next->color;
                else if (e->polygon->val == 3) // Triangle face (empirical formula, not proven)
                    r1 += (si + si1) / (3 * k + c) * (3 * (computeMeanFaceColor(e) - e->color) + e->color);
                else                           // Arbitrary valency face (empirical formula, not proven)
                    r1 += (si + si1) / (3 * k + c) * (2 * (computeMeanFaceColor(e) - e->color) + e->color);
            }

            // Next vertex edge
            e = e->prev->twin;
        }

        // Compute limit tangent
        QVector3D q = cos(M_PI * j / k) * r0 + sin(M_PI * j / k) * r1;

        // Compute color (ACC2 paper section 3.3 formula e0+)
        color = p.color() + 2 * lambda(2 * k) * q / 3;
    }

    return QVector5D(coords, color);
}

QVector5D ACC2Renderer::computeFacePoint(HalfEdge *inputEdge, QVector5D ep, QVector5D em, double d, bool forward) {
    Vertex *origin = inputEdge->prev->target;
    Vertex *target = inputEdge->target;

    // Compute coords
    QVector2D coords;
    if (inputEdge->polygon && inputEdge->twin->polygon) {
        // Non-boundary case (ACC2 paper section 3.4)

        // Compute valences (ACC1 paper section A.2 substitutions n)
        int originVal = !isBoundaryVertex(origin) ? origin->val : (2 * origin->val - 2);
        int targetVal = !isBoundaryVertex(target) ? target->val : (2 * target->val - 2);

        // Compute c0 and c1
        float c0 = cos(2 * M_PI / originVal);
        float c1 = cos(2 * M_PI / targetVal);

        // Compute r0+
        QVector2D rp = (computeEdgeMidpointCoords(inputEdge->prev) - computeEdgeMidpointCoords(inputEdge->twin->next)) / 3 + 2 * (computeMeanFaceCoords(inputEdge) - computeMeanFaceCoords(inputEdge->twin)) / 3;

        // Flip the direction of r0+ depending on which face we are considering
        if (!forward)
            rp = -rp;

        // Compute f0+
        coords = (c1 * origin->coords + (d - 2 * c0 - c1) * ep.coords() + 2 * c0 * em.coords() + rp) / d;
    } else {
        // Boundary case

        // Compute valences (ACC1 paper section A.2 substitutions n)
        int originVal = origin->val == 2 ? 4 : (2 * origin->val - 2);
        int targetVal = target->val == 2 ? 4 : (2 * target->val - 2);

        // Compute c0 and c1
        float c0 = cos(2 * M_PI / originVal);
        float c1 = cos(2 * M_PI / targetVal);

        // ??? Reference ???
        QVector2D faceComponent, midComponent;
        if (!inputEdge->polygon) {
            QVector2D meanFace = computeMeanFaceCoords(inputEdge->twin);
            faceComponent = 2 * (meanFace - (inputEdge->target->coords + origin->coords) / 2);
            midComponent = inputEdge->twin->next->target->coords - origin->coords;
        } else {
            QVector2D meanFace = computeMeanFaceCoords(inputEdge);
            faceComponent = 2 * (meanFace - (inputEdge->target->coords + origin->coords) / 2);
            midComponent = inputEdge->prev->twin->target->coords - origin->coords;
        }

        // Compute transversal vector (ACC2 paper section 3.4 formula r0+)
        QVector2D rp = midComponent / 3 + 2 * faceComponent / 3;

        // Compute face point (ACC2 paper section 3.4 formula f0+)
        coords = (c1 * origin->coords + (d - 2 * c0 - c1) * ep.coords() + 2 * c0 * em.coords() + rp) / d;
    }

    // Compute color
    QVector3D color;
    if (!isSharpEdge(inputEdge)) {
        // Non-boundary case (ACC2 paper section 3.4)

        // Compute valences (ACC1 paper section A.2 substitutions n)
        int originValColor = getColorVertexVal(forward ? inputEdge : inputEdge->twin->next);
        int targetValColor = getColorVertexVal(forward ? inputEdge->next : inputEdge->twin);
        originValColor = isSmoothVertex(origin) ? originValColor : (2 * originValColor - 2);
        targetValColor = isSmoothVertex(inputEdge->target) ? targetValColor : (2 * targetValColor - 2);

        // Compute c0 and c1
        float c0 = cos(2 * M_PI / originValColor);
        float c1 = cos(2 * M_PI / targetValColor);

        // Compute r0+
        QVector3D rp = (computeEdgeMidpointColor(inputEdge->prev) - computeEdgeMidpointColor(inputEdge->twin->next)) / 3 + 2 * (computeMeanFaceColor(inputEdge) - computeMeanFaceColor(inputEdge->twin)) / 3;

        // Flip the direction of r0+ depending on which face we are considering
        if (!forward)
            rp = -rp;

        // Compute f0+
        color = (c1 * inputEdge->color + (d - 2 * c0 - c1) * ep.color() + 2 * c0 * em.color() + rp) / d;
    } else {
        // Boundary case

        // Compute valences (ACC1 paper section A.2 substitutions n)
        int originValColor = getColorVertexVal(forward ? inputEdge : inputEdge->twin->next);
        int targetValColor = getColorVertexVal(forward ? inputEdge->next : inputEdge->twin);
        originValColor = isSmoothVertex(origin) ? originValColor : (originValColor == 2 ? 4 : (2 * originValColor - 2));
        targetValColor = isSmoothVertex(target) ? targetValColor : (targetValColor == 2 ? 4 : (2 * targetValColor - 2));

        // Compute c0 and c1
        float c0 = cos(2 * M_PI / originValColor);
        float c1 = cos(2 * M_PI / targetValColor);

        // ??? Reference ???
        QVector3D faceComponent, midComponent;
        if (forward) {
            faceComponent = 2 * (computeMeanFaceColor(inputEdge) - computeEdgeMidpointColor(inputEdge));
            midComponent = inputEdge->prev->color - inputEdge->color;
        } else {
            faceComponent = 2 * (computeMeanFaceColor(inputEdge->twin) - computeEdgeMidpointColor(inputEdge->twin));
            midComponent = inputEdge->twin->next->next->color - inputEdge->twin->next->color;
        }

        // Compute transversal vector (ACC2 paper section 3.4 formula r0+)
        QVector3D rp = midComponent / 3 + 2 * faceComponent / 3;

        // Compute face point (ACC2 paper section 3.4 formula f0+)
        QVector3D originColor = forward ? inputEdge->color : inputEdge->twin->next->color;
        color = (c1 * originColor + (d - 2 * c0 - c1) * ep.color() + 2 * c0 * em.color() + rp) / d;
    }

    return QVector5D(coords, color);
}

void ACC2Renderer::addControlPoints(Face f, QVector<float> *data) {
    HalfEdge *e = f.side;

    // Pre-compute p(i) (ACC2 paper section 3.2)
    QVector<QVector5D> cornerPoints;
    foreach (HalfEdge *e, getFaceEdges(f.side))
        cornerPoints << computeCornerPoint(e);

    // Compute e(i)+, e(i+1)-, f(i)+ and f(i+1)- (ACC2 paper section 3.4)
    for (int i = 0; i < f.val; ++i) {
        QVector5D p = cornerPoints[i];
        QVector5D p1 = cornerPoints[(i + 1) % f.val];
        QVector5D ep = computeEdgePoint(e, p, true);
        QVector5D em = computeEdgePoint(e->twin, p1, false);
        QVector5D fp = computeFacePoint(e, ep, em, f.val == 3 ? 4 : 3, true);
        QVector5D fm = computeFacePoint(e->twin, em, ep, f.val == 3 ? 4 : 3, false);
        *data << p;
        *data << ep;
        *data << em;
        *data << fp;
        *data << fm;
        e = e->next;
    }
}

void ACC2Renderer::updateControlPoints(Face &f, QVector<float> &data, int faceIndex, int coorsOrColor) {
    HalfEdge *e = f.side;

    // Pre-compute p(i) (ACC2 paper section 3.2)
    QVector<QVector5D> cornerPoints;

    foreach (HalfEdge *e, getFaceEdges(f.side))
        cornerPoints << computeCornerPoint(e);

    // update coords
    if (coorsOrColor == 1) {
        // Compute e(i)+, e(i+1)-, f(i)+ and f(i+1)- (ACC2 paper section 3.4)
        for (int i = 0; i < f.val; ++i) {
            QVector5D p = cornerPoints[i];
            QVector5D p1 = cornerPoints[(i + 1) % f.val];
            QVector5D ep = computeEdgePoint(e, p, true);
            QVector5D em = computeEdgePoint(e->twin, p1, false);
            QVector5D fp = computeFacePoint(e, ep, em, f.val == 3 ? 4 : 3, true);
            QVector5D fm = computeFacePoint(e->twin, em, ep, f.val == 3 ? 4 : 3, false);

            data[faceIndex + 25 * i] = p.x();
            data[faceIndex + 25 * i + 1] = p.y();

            data[faceIndex + 25 * i + 5] = ep.x();
            data[faceIndex + 25 * i + 6] = ep.y();

            data[faceIndex + 25 * i + 10] = em.x();
            data[faceIndex + 25 * i + 11] = em.y();

            data[faceIndex + 25 * i + 15] = fp.x();
            data[faceIndex + 25 * i + 16] = fp.y();

            data[faceIndex + 25 * i + 20] = fm.x();
            data[faceIndex + 25 * i + 21] = fm.y();

            e = e->next;
        }
    }
    // update color
    else {
        // Compute e(i)+, e(i+1)-, f(i)+ and f(i+1)- (ACC2 paper section 3.4)
        for (int i = 0; i < f.val; ++i) {
            QVector5D p = cornerPoints[i];
            QVector5D p1 = cornerPoints[(i + 1) % f.val];
            QVector5D ep = computeEdgePoint(e, p, true);
            QVector5D em = computeEdgePoint(e->twin, p1, false);
            QVector5D fp = computeFacePoint(e, ep, em, f.val == 3 ? 4 : 3, true);
            QVector5D fm = computeFacePoint(e->twin, em, ep, f.val == 3 ? 4 : 3, false);

            data[faceIndex + 25 * i + 2] = p.r();
            data[faceIndex + 25 * i + 3] = p.g();
            data[faceIndex + 25 * i + 4] = p.b();

            data[faceIndex + 25 * i + 7] = ep.r();
            data[faceIndex + 25 * i + 8] = ep.g();
            data[faceIndex + 25 * i + 9] = ep.b();

            data[faceIndex + 25 * i + 12] = em.r();
            data[faceIndex + 25 * i + 13] = em.g();
            data[faceIndex + 25 * i + 14] = em.b();

            data[faceIndex + 25 * i + 17] = fp.r();
            data[faceIndex + 25 * i + 18] = fp.g();
            data[faceIndex + 25 * i + 19] = fp.b();

            data[faceIndex + 25 * i + 22] = fm.r();
            data[faceIndex + 25 * i + 23] = fm.g();
            data[faceIndex + 25 * i + 24] = fm.b();

            e = e->next;
        }
    }


}

float ACC2Renderer::sigma(int n) {
    // (ACC2 paper section 3.3)
    return pow(4 + pow(cos(M_PI / n), 2), -.5);
}

float ACC2Renderer::lambda(int n) {
    // (ACC2 paper section 3.3)
    return (5 + cos(2 * M_PI / n) + cos(M_PI / n) * sqrt(18 + 2 * cos(2 * M_PI / n))) / 16;
}
