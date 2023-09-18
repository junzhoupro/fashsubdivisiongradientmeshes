#ifndef LINERENDERER_H
#define LINERENDERER_H

#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions_4_1_Core>
#include <QVector>
#include <QVector2D>
#include <QVector3D>
#include <QSet>
#include "halfedge.h"

class LineRenderer {

public:
  LineRenderer(QOpenGLFunctions_4_1_Core *functions);
  ~LineRenderer();
  void setScaling(QVector2D scaling);
  void setDisplacement(QVector2D displacement);
  void setColor(QVector3D color);
  void setEdges(QSet<HalfEdge *> edges);
  void render();

private:
  QOpenGLFunctions_4_1_Core *functions;
  QOpenGLShaderProgram *shaderProgram;
  GLuint VAO, VBO;
  int drawCount;

};

#endif // LINERENDERER_H
