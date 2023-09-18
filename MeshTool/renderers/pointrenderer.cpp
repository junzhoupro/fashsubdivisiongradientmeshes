#include "pointrenderer.h"

PointRenderer::PointRenderer(QOpenGLFunctions_4_1_Core *functions) {
  // Set OpenGL functions
  this->functions = functions;

  // Create shader program for filled circles
  shaderProgramFilled = new QOpenGLShaderProgram();
  shaderProgramFilled->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/point/vertshader.glsl");
  shaderProgramFilled->addShaderFromSourceFile(QOpenGLShader::Geometry, ":/shaders/point/geomshaderfilled.glsl");
  shaderProgramFilled->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/point/fragshader.glsl");
  shaderProgramFilled->link();

  // Create shader program for outline
  shaderProgramLine = new QOpenGLShaderProgram();
  shaderProgramLine->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/point/vertshader.glsl");
  shaderProgramLine->addShaderFromSourceFile(QOpenGLShader::Geometry, ":/shaders/point/geomshaderline.glsl");
  shaderProgramLine->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/point/fragshader.glsl");
  shaderProgramLine->link();

  // Create VAO
  functions->glGenVertexArrays(1, &VAO);
  functions->glBindVertexArray(VAO);

  // Create vertex buffer
  functions->glGenBuffers(1, &VBO);
  functions->glBindBuffer(GL_ARRAY_BUFFER, VBO);
  functions->glEnableVertexAttribArray(0);
  functions->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

  // Create index buffer
  functions->glGenBuffers(1, &IBO);
  functions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

  // Release
  functions->glBindVertexArray(0);
}

PointRenderer::~PointRenderer() {
  delete shaderProgramFilled;
  delete shaderProgramLine;
  functions->glDeleteBuffers(1, &IBO);
  functions->glDeleteBuffers(1, &VBO);
  functions->glDeleteVertexArrays(1, &VAO);
}

void PointRenderer::setScaling(QVector2D scaling) {
  shaderProgramFilled->bind();
  shaderProgramFilled->setUniformValue("scaling", scaling);
  shaderProgramFilled->release();
  shaderProgramLine->bind();
  shaderProgramLine->setUniformValue("scaling", scaling);
  shaderProgramLine->release();
}

void PointRenderer::setDisplacement(QVector2D displacement) {
  shaderProgramFilled->bind();
  shaderProgramFilled->setUniformValue("displacement", displacement);
  shaderProgramFilled->release();
  shaderProgramLine->bind();
  shaderProgramLine->setUniformValue("displacement", displacement);
  shaderProgramLine->release();
}

void PointRenderer::setColor(QVector3D color) {
  shaderProgramFilled->bind();
  shaderProgramFilled->setUniformValue("color", color);
  shaderProgramFilled->release();
  shaderProgramLine->bind();
  shaderProgramLine->setUniformValue("color", color);
  shaderProgramLine->release();
}

void PointRenderer::setFilled(bool filled) {
  this->filled = filled;
}

void PointRenderer::setRadius(float radius) {
  shaderProgramFilled->bind();
  shaderProgramFilled->setUniformValue("radius", radius);
  shaderProgramFilled->release();
  shaderProgramLine->bind();
  shaderProgramLine->setUniformValue("radius", radius);
  shaderProgramLine->release();
}

void PointRenderer::setVertexCoords(QVector<QVector2D> vertexCoords) {
  functions->glBindBuffer(GL_ARRAY_BUFFER, VBO);
  functions->glBufferData(GL_ARRAY_BUFFER, sizeof(QVector2D) * vertexCoords.size(), vertexCoords.data(), GL_DYNAMIC_DRAW);
};

void PointRenderer::setVertexIndices(QVector<int> vertexIndices) {
  indicesSize = vertexIndices.size();
  functions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
  functions->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * vertexIndices.size(), vertexIndices.data(), GL_DYNAMIC_DRAW);
}

void PointRenderer::render() {
  // Bind
  functions->glBindVertexArray(VAO);

  // Set environment
  functions->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // Draw points
  if (filled) {
    shaderProgramFilled->bind();
    functions->glDrawElements(GL_POINTS, indicesSize, GL_UNSIGNED_INT, nullptr);
    shaderProgramFilled->release();
  } else {
    shaderProgramLine->bind();
    functions->glDrawElements(GL_POINTS, indicesSize, GL_UNSIGNED_INT, nullptr);
    shaderProgramLine->release();
  }

  // Release
  functions->glBindVertexArray(0);
}
