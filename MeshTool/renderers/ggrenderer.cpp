#include "ggrenderer.h"
#include "tools/tools.h"
#include "renderers/acc2renderer.h"
#include <QtMath>
#include <QVector3D>
#include <QFile>

GGRenderer::GGRenderer(QOpenGLFunctions_4_1_Core *functions) : SurfaceRenderer(functions) {
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

GGRenderer::~GGRenderer() {
  functions->glDeleteBuffers(1, &VBO);
  functions->glDeleteVertexArrays(1, &VAO);
}

void GGRenderer::setScaling(QVector2D scaling) {
  this->scaling = scaling;
  SurfaceRenderer::setScaling(scaling);
}

void GGRenderer::setDisplacement(QVector2D displacement) {
  this->displacement = displacement;
  SurfaceRenderer::setDisplacement(displacement);
}

void GGRenderer::setColorBands(int colorBands) {
  this->colorBands = colorBands;
  SurfaceRenderer::setColorBands(colorBands);
}

void GGRenderer::setTessLevel(int tessLevel) {
  this->tessLevel = tessLevel;
  SurfaceRenderer::setTessLevel(tessLevel);
}

void GGRenderer::setComputeDiff(bool computeDiff) {
  this->computeDiff = computeDiff;
  SurfaceRenderer::setComputeDiff(computeDiff);
}

void GGRenderer::setDiffScaling(float diffScaling) {
  this->diffScaling = diffScaling;
  SurfaceRenderer::setDiffScaling(diffScaling);
}

void GGRenderer::setMesh(Mesh& mesh) {
    data.clear();
    datasIndices.clear();

  // Collect data per valency
  QHash<int, QVector<float>> datas;
  int index = 0;
  foreach (Face f, mesh.Faces) {
    ACC2Renderer::addControlPoints(f, &datas[f.val]);
    datasIndices[f.val][f.index] = index;
    index += 25 * f.val;
  }

  // Collect all data and keep track of sizes
  controlPointsSizes.clear();
  controlPointsOffsets.clear();
//  QVector<float> data;
  int offset = 0;
  foreach (int val, datas.keys()) {
    // Add to data
    data.append(datas[val]);

    // Set number of control points
    controlPointsOffsets[val] = offset;
    controlPointsSizes[val] = datas[val].size() / 5;
    offset += controlPointsSizes[val];

    // Add shader program for valency if not present
    if (!shaderPrograms.contains(QString::number(val))) {
      shaderPrograms[QString::number(val)] = makeShaderProgram(val);
      shaderPrograms[QString::number(val)]->link();
      shaderPrograms[QString::number(val)]->bind();
      shaderPrograms[QString::number(val)]->setUniformValue("scaling", scaling);
      shaderPrograms[QString::number(val)]->setUniformValue("displacement", displacement);
      shaderPrograms[QString::number(val)]->setUniformValue("colorBands", colorBands);
      shaderPrograms[QString::number(val)]->setUniformValue("tessLevel", tessLevel);
      shaderPrograms[QString::number(val)]->setUniformValue("computeDiff", computeDiff);
      shaderPrograms[QString::number(val)]->setUniformValue("diffScaling", diffScaling);
      shaderPrograms[QString::number(val)]->release();
    }
  }

  // Set data
  setData(data);
}

void GGRenderer::updateMeshCoords(Mesh& mesh, QVector<int>& influencedFacesIndices) {
    foreach (int i, influencedFacesIndices) {
        ACC2Renderer::updateControlPoints(mesh.Faces[i], data, datasIndices[mesh.Faces[i].val][i], 1);
    }

  // Set data
  setData(data);

}

void GGRenderer::updateMeshColors(Mesh& mesh, QVector<int>& influencedFacesIndices) {
    foreach (int i, influencedFacesIndices) {
        ACC2Renderer::updateControlPoints(mesh.Faces[i], data, datasIndices[mesh.Faces[i].val][i], 2);
    }

  // Set data
  setData(data);
}

void GGRenderer::render() {
  // Bind
  functions->glBindVertexArray(VAO);

  // Draw independent for each valency
  foreach (int val, controlPointsOffsets.keys()) {
    // Bind
    shaderPrograms[QString::number(val)]->bind();

    // Draw
    functions->glPolygonMode(GL_FRONT_AND_BACK, showWireframe ? GL_LINE : GL_FILL);
    functions->glPatchParameteri(GL_PATCH_VERTICES, 5 * val);
    functions->glDrawArraysInstanced(GL_PATCHES, controlPointsOffsets[val], controlPointsSizes[val], val > 4 ? val : 1);

    // Release
    shaderPrograms[QString::number(val)]->release();
  }

  // Release
  functions->glBindVertexArray(0);
}

QHash<QString, int> GGRenderer::getCountInfo() {
  int controlPoints = 0, faces = 0, invocations = 0;
  foreach (int val, controlPointsSizes.keys()) {
    controlPoints += controlPointsSizes[val];
    faces += controlPointsSizes[val] / (5 * val);
    invocations += controlPointsSizes[val] / (val > 4 ? 5 : 5 * val);
  }
  QHash<QString, int> countInfo;
  countInfo["faces"] = faces;
  countInfo["control points"] = controlPoints;
  countInfo["invocations"] = invocations;
  return countInfo;
}

void GGRenderer::setData(QVector<float> data) {
  functions->glBindBuffer(GL_ARRAY_BUFFER, VBO);
  functions->glBufferData(GL_ARRAY_BUFFER, sizeof(float) * data.size(), data.data(), GL_DYNAMIC_DRAW);
};

QOpenGLShaderProgram *GGRenderer::makeShaderProgram(int N) {
  if (N == 3) {
    QOpenGLShaderProgram *shaderProgram = new QOpenGLShaderProgram();
    shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/ACC2/vertshader.glsl");
    shaderProgram->addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/shaders/ACC2/triangles/controlshader.glsl");
    shaderProgram->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/shaders/ACC2/triangles/evalshader.glsl");
    shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshadershared.glsl");
    shaderProgram->link();
    return shaderProgram;
  } else if (N == 4) {
    QOpenGLShaderProgram *shaderProgram = new QOpenGLShaderProgram();
    shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/ACC2/vertshader.glsl");
    shaderProgram->addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/shaders/ACC2/quads/controlshader.glsl");
    shaderProgram->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/shaders/ACC2/quads/evalshader.glsl");
    shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshadershared.glsl");
    shaderProgram->link();
    return shaderProgram;
  }

  // Initialize with vertex and fragment shader
  QOpenGLShaderProgram *shaderProgram = new QOpenGLShaderProgram();
  shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/GG/vertshader.glsl");
  shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshadershared.glsl");

  // Construct control shader
  QFile controlShaderFile(":/shaders/GG/controlshader.glsl");
  controlShaderFile.open(QFile::ReadOnly | QFile::Text);
  QString controlShaderCode = QString(controlShaderFile.readAll());
  controlShaderFile.close();
  controlShaderCode = controlShaderCode.replace("/* vertices */", QString::number(5 * N));

  // Add control shader
  shaderProgram->addShaderFromSourceCode(QOpenGLShader::TessellationControl, controlShaderCode);

  // Construct evaluation shader
  QFile evalShaderFile(":/shaders/GG/evalshader.glsl");
  evalShaderFile.open(QFile::ReadOnly | QFile::Text);
  QString evalShaderCode = QString(evalShaderFile.readAll());
  evalShaderFile.close();
  evalShaderCode = evalShaderCode.replace("/* N */", QString::number(N));

  // Add evaluation shader
  shaderProgram->addShaderFromSourceCode(QOpenGLShader::TessellationEvaluation, evalShaderCode);
  shaderProgram->link();

  return shaderProgram;
}
