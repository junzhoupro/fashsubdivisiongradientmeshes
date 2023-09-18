#ifndef POINTRENDERER_H
#define POINTRENDERER_H

#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions_4_1_Core>
#include <QVector>
#include <QVector2D>
#include <QVector3D>

class PointRenderer {

public:
  PointRenderer(QOpenGLFunctions_4_1_Core *functions);
  ~PointRenderer();
  void setScaling(QVector2D scaling);
  void setDisplacement(QVector2D displacement);
  void setColor(QVector3D color);
  void setFilled(bool filled);
  void setRadius(float radius);
  void setVertexCoords(QVector<QVector2D> vertexCoords);
  void setVertexIndices(QVector<int> vertexIndices);
  void render();

private:
  QOpenGLFunctions_4_1_Core *functions;
  QOpenGLShaderProgram *shaderProgramFilled, *shaderProgramLine;
  GLuint VAO, IBO, VBO;
  int indicesSize;
  bool filled = true;

};

#endif // POINTRENDERER_H
