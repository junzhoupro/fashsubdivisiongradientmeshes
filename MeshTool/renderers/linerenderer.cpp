#include "linerenderer.h"
#include "vertex.h"

LineRenderer::LineRenderer(QOpenGLFunctions_4_1_Core *functions) {
  // Set OpenGL functions
  this->functions = functions;

  // Create shader program for lines
  shaderProgram = new QOpenGLShaderProgram();
  shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/line/vertshader.glsl");
  shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/line/fragshader.glsl");
  shaderProgram->link();

  // Create VAO
  functions->glGenVertexArrays(1, &VAO);
  functions->glBindVertexArray(VAO);

  // Create vertex buffer
  functions->glGenBuffers(1, &VBO);
  functions->glBindBuffer(GL_ARRAY_BUFFER, VBO);
  functions->glEnableVertexAttribArray(0);
  functions->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

  // Release
  functions->glBindVertexArray(0);
}

LineRenderer::~LineRenderer() {
  delete shaderProgram;
  functions->glDeleteBuffers(1, &VBO);
  functions->glDeleteVertexArrays(1, &VAO);
}

void LineRenderer::setScaling(QVector2D scaling) {
  shaderProgram->bind();
  shaderProgram->setUniformValue("scaling", scaling);
  shaderProgram->release();
}

void LineRenderer::setDisplacement(QVector2D displacement) {
  shaderProgram->bind();
  shaderProgram->setUniformValue("displacement", displacement);
  shaderProgram->release();
}

void LineRenderer::setColor(QVector3D color) {
  shaderProgram->bind();
  shaderProgram->setUniformValue("color", color);
  shaderProgram->release();
}

void LineRenderer::setEdges(QSet<HalfEdge *> edges) {
  QVector<QVector2D> coords;
  foreach (HalfEdge *e, edges) {
    coords << e->prev->target->coords;
    coords << e->target->coords;
  }
  drawCount = coords.size();
  functions->glBindBuffer(GL_ARRAY_BUFFER, VBO);
  functions->glBufferData(GL_ARRAY_BUFFER, sizeof(QVector2D) * coords.size(), coords.data(), GL_DYNAMIC_DRAW);
};

void LineRenderer::render() {
  functions->glBindVertexArray(VAO);
  shaderProgram->bind();
  functions->glDrawArrays(GL_LINES, 0, drawCount);
  shaderProgram->release();
  functions->glBindVertexArray(0);
}
