#include "surfacerenderer.h"
#include "tools/tools.h"
#include <QVector3D>

SurfaceRenderer::SurfaceRenderer(QOpenGLFunctions_4_1_Core *functions) {
  this->functions = functions;
}

SurfaceRenderer::~SurfaceRenderer() {
  foreach (QOpenGLShaderProgram *shaderProgram, shaderPrograms)
    delete shaderProgram;
  foreach (SurfaceRenderer *renderer, renderers)
    delete renderer;
}

void SurfaceRenderer::setShowWireframe(bool showWireframe) {
  this->showWireframe = showWireframe;
  foreach (SurfaceRenderer *renderer, renderers)
    renderer->setShowWireframe(showWireframe);
}

void SurfaceRenderer::setScaling(QVector2D scaling) {
  foreach (QOpenGLShaderProgram *shaderProgram, shaderPrograms) {
    shaderProgram->bind();
    shaderProgram->setUniformValue("scaling", scaling);
    shaderProgram->release();
  }
  foreach (SurfaceRenderer *renderer, renderers)
    renderer->setScaling(scaling);
}

void SurfaceRenderer::setDisplacement(QVector2D displacement) {
  foreach (QOpenGLShaderProgram *shaderProgram, shaderPrograms) {
    shaderProgram->bind();
    shaderProgram->setUniformValue("displacement", displacement);
    shaderProgram->release();
  }
  foreach (SurfaceRenderer *renderer, renderers)
    renderer->setDisplacement(displacement);
}

void SurfaceRenderer::setColorBands(int colorBands) {
  foreach (QOpenGLShaderProgram *shaderProgram, shaderPrograms) {
    shaderProgram->bind();
    shaderProgram->setUniformValue("colorBands", colorBands);
    shaderProgram->release();
  }
  foreach (SurfaceRenderer *renderer, renderers)
    renderer->setColorBands(colorBands);
}

void SurfaceRenderer::setTessLevel(int tessLevel) {
  foreach (QOpenGLShaderProgram *shaderProgram, shaderPrograms) {
    shaderProgram->bind();
    shaderProgram->setUniformValue("tessLevel", tessLevel);
    shaderProgram->release();
  }
  foreach (SurfaceRenderer *renderer, renderers)
    renderer->setTessLevel(tessLevel);
}

void SurfaceRenderer::setComputeDiff(int computeDiff) {
  foreach (QOpenGLShaderProgram *shaderProgram, shaderPrograms) {
    shaderProgram->bind();
    shaderProgram->setUniformValue("computeDiff", computeDiff);
    shaderProgram->release();
  }
  foreach (SurfaceRenderer *renderer, renderers)
    renderer->setComputeDiff(computeDiff);
}

void SurfaceRenderer::setDiffScaling(float diffScaling) {
  foreach (QOpenGLShaderProgram *shaderProgram, shaderPrograms) {
    shaderProgram->bind();
    shaderProgram->setUniformValue("diffScaling", diffScaling);
    shaderProgram->release();
  }
  foreach (SurfaceRenderer *renderer, renderers)
    renderer->setDiffScaling(diffScaling);
}


