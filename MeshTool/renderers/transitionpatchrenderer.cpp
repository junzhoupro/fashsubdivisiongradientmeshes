#include "transitionpatchrenderer.h"
#include "renderers/acc1renderer.h"
#include "renderers/acc2renderer.h"
#include "tools/tools.h"
#include <QVector3D>

TransitionPatchRenderer::TransitionPatchRenderer(QOpenGLFunctions_4_1_Core *functions) : SurfaceRenderer(functions) {
  // Create shader programs
  shaderPrograms["ACC1-C1"] = new QOpenGLShaderProgram();
  shaderPrograms["ACC1-C1"]->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/TP/vertshader.glsl");
  shaderPrograms["ACC1-C1"]->addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/shaders/TP/ACC1/controlshader.glsl");
  shaderPrograms["ACC1-C1"]->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/shaders/TP/ACC1/constellation1/evalshader.glsl");
  shaderPrograms["ACC1-C1"]->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshadershared.glsl");
  shaderPrograms["ACC1-C1"]->link();

  shaderPrograms["ACC1-C2"] = new QOpenGLShaderProgram();
  shaderPrograms["ACC1-C2"]->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/TP/vertshader.glsl");
  shaderPrograms["ACC1-C2"]->addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/shaders/TP/ACC1/controlshader.glsl");
  shaderPrograms["ACC1-C2"]->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/shaders/TP/ACC1/constellation2/evalshader.glsl");
  shaderPrograms["ACC1-C2"]->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshadershared.glsl");
  shaderPrograms["ACC1-C2"]->link();

  shaderPrograms["ACC1-C3"] = new QOpenGLShaderProgram();
  shaderPrograms["ACC1-C3"]->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/TP/vertshader.glsl");
  shaderPrograms["ACC1-C3"]->addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/shaders/TP/ACC1/controlshader.glsl");
  shaderPrograms["ACC1-C3"]->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/shaders/TP/ACC1/constellation3/evalshader.glsl");
  shaderPrograms["ACC1-C3"]->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshadershared.glsl");
  shaderPrograms["ACC1-C3"]->link();

  shaderPrograms["ACC1-C4-Quad"] = new QOpenGLShaderProgram();
  shaderPrograms["ACC1-C4-Quad"]->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/TP/vertshader.glsl");
  shaderPrograms["ACC1-C4-Quad"]->addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/shaders/TP/ACC1/controlshader.glsl");
  shaderPrograms["ACC1-C4-Quad"]->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/shaders/TP/ACC1/constellation4/quad/evalshader.glsl");
  shaderPrograms["ACC1-C4-Quad"]->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshadershared.glsl");
  shaderPrograms["ACC1-C4-Quad"]->link();

  shaderPrograms["ACC1-C4-Triangles"] = new QOpenGLShaderProgram();
  shaderPrograms["ACC1-C4-Triangles"]->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/TP/vertshader.glsl");
  shaderPrograms["ACC1-C4-Triangles"]->addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/shaders/TP/ACC1/controlshader.glsl");
  shaderPrograms["ACC1-C4-Triangles"]->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/shaders/TP/ACC1/constellation4/triangles/evalshader.glsl");
  shaderPrograms["ACC1-C4-Triangles"]->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshadershared.glsl");
  shaderPrograms["ACC1-C4-Triangles"]->link();

  shaderPrograms["ACC1-C5"] = new QOpenGLShaderProgram();
  shaderPrograms["ACC1-C5"]->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/TP/vertshader.glsl");
  shaderPrograms["ACC1-C5"]->addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/shaders/TP/ACC1/controlshader.glsl");
  shaderPrograms["ACC1-C5"]->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/shaders/TP/ACC1/constellation5/evalshader.glsl");
  shaderPrograms["ACC1-C5"]->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshadershared.glsl");
  shaderPrograms["ACC1-C5"]->link();

  shaderPrograms["ACC2-C1"] = new QOpenGLShaderProgram();
  shaderPrograms["ACC2-C1"]->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/TP/vertshader.glsl");
  shaderPrograms["ACC2-C1"]->addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/shaders/TP/ACC2/controlshader.glsl");
  shaderPrograms["ACC2-C1"]->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/shaders/TP/ACC2/constellation1/evalshader.glsl");
  shaderPrograms["ACC2-C1"]->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshadershared.glsl");
  shaderPrograms["ACC2-C1"]->link();

  shaderPrograms["ACC2-C2"] = new QOpenGLShaderProgram();
  shaderPrograms["ACC2-C2"]->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/TP/vertshader.glsl");
  shaderPrograms["ACC2-C2"]->addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/shaders/TP/ACC2/controlshader.glsl");
  shaderPrograms["ACC2-C2"]->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/shaders/TP/ACC2/constellation2/evalshader.glsl");
  shaderPrograms["ACC2-C2"]->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshadershared.glsl");
  shaderPrograms["ACC2-C2"]->link();

  shaderPrograms["ACC2-C3"] = new QOpenGLShaderProgram();
  shaderPrograms["ACC2-C3"]->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/TP/vertshader.glsl");
  shaderPrograms["ACC2-C3"]->addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/shaders/TP/ACC2/controlshader.glsl");
  shaderPrograms["ACC2-C3"]->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/shaders/TP/ACC2/constellation3/evalshader.glsl");
  shaderPrograms["ACC2-C3"]->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshadershared.glsl");
  shaderPrograms["ACC2-C3"]->link();

  shaderPrograms["ACC2-C4-Quad"] = new QOpenGLShaderProgram();
  shaderPrograms["ACC2-C4-Quad"]->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/TP/vertshader.glsl");
  shaderPrograms["ACC2-C4-Quad"]->addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/shaders/TP/ACC2/controlshader.glsl");
  shaderPrograms["ACC2-C4-Quad"]->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/shaders/TP/ACC2/constellation4/quad/evalshader.glsl");
  shaderPrograms["ACC2-C4-Quad"]->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshadershared.glsl");
  shaderPrograms["ACC2-C4-Quad"]->link();

  shaderPrograms["ACC2-C4-Triangles"] = new QOpenGLShaderProgram();
  shaderPrograms["ACC2-C4-Triangles"]->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/TP/vertshader.glsl");
  shaderPrograms["ACC2-C4-Triangles"]->addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/shaders/TP/ACC2/controlshader.glsl");
  shaderPrograms["ACC2-C4-Triangles"]->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/shaders/TP/ACC2/constellation4/triangles/evalshader.glsl");
  shaderPrograms["ACC2-C4-Triangles"]->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshadershared.glsl");
  shaderPrograms["ACC2-C4-Triangles"]->link();

  shaderPrograms["ACC2-C5"] = new QOpenGLShaderProgram();
  shaderPrograms["ACC2-C5"]->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/TP/vertshader.glsl");
  shaderPrograms["ACC2-C5"]->addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/shaders/TP/ACC2/controlshader.glsl");
  shaderPrograms["ACC2-C5"]->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/shaders/TP/ACC2/constellation5/evalshader.glsl");
  shaderPrograms["ACC2-C5"]->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshadershared.glsl");
  shaderPrograms["ACC2-C5"]->link();

  // Create VAO ACC1
  functions->glGenVertexArrays(1, &VAOACC1);
  functions->glBindVertexArray(VAOACC1);

  // Create VBO ACC1
  functions->glGenBuffers(1, &VBOACC1);
  functions->glBindBuffer(GL_ARRAY_BUFFER, VBOACC1);
  functions->glEnableVertexAttribArray(0);
  functions->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
  functions->glEnableVertexAttribArray(1);
  functions->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (2 * sizeof(float)));

  // Release
  functions->glBindVertexArray(0);

  // Create VAO ACC2
  functions->glGenVertexArrays(1, &VAOACC2);
  functions->glBindVertexArray(VAOACC2);

  // Create VBO ACC2
  functions->glGenBuffers(1, &VBOACC2);
  functions->glBindBuffer(GL_ARRAY_BUFFER, VBOACC2);
  functions->glEnableVertexAttribArray(0);
  functions->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
  functions->glEnableVertexAttribArray(1);
  functions->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (2 * sizeof(float)));

  // Release
  functions->glBindVertexArray(0);
}

TransitionPatchRenderer::~TransitionPatchRenderer() {
  functions->glDeleteBuffers(1, &VBOACC1);
  functions->glDeleteVertexArrays(1, &VAOACC1);
  functions->glDeleteBuffers(1, &VBOACC2);
  functions->glDeleteVertexArrays(1, &VAOACC2);
}

void TransitionPatchRenderer::clearControlPoints() {
  foreach (QString constellation, datasACC1.keys()) {
    datasACC1[constellation].clear();
    datasACC1[constellation].squeeze();
  }
  foreach (QString constellation, datasACC2.keys()) {
    datasACC2[constellation].clear();
    datasACC2[constellation].squeeze();
  }
}

void TransitionPatchRenderer::addControlPoints(Face f, QSet<int> transitionEdges) {
  // Find first transition edge
  HalfEdge *firstTransitionEdge = f.side;

  // Forward to first non-transition edge
  foreach (HalfEdge *e, getFaceEdges(f.side)) {
    if (!transitionEdges.contains(e->index)) {
      firstTransitionEdge = e;
      break;
    }
  }

  // Forward to first transition edge
  foreach (HalfEdge *e, getFaceEdges(firstTransitionEdge)) {
    if (transitionEdges.contains(e->index)) {
      firstTransitionEdge = e;
      break;
    }
  }

  // Compute constellation
  QString constellation;
  if (transitionEdges.size() == 1)
    constellation = "C1";
  else if (transitionEdges.size() == 2)
    constellation = transitionEdges.contains(firstTransitionEdge->next->index) ? "C2" : "C3";
  else if (transitionEdges.size() == 3)
    constellation = "C4";
  else if (transitionEdges.size() == 4)
    constellation = "C5";

  // Add control points;
  if (isRegularFace(f)) {
    foreach (HalfEdge *e, getFaceEdges(firstTransitionEdge)) {
      datasACC1[constellation] << ACC1Renderer::computeCornerPoint(e);
      datasACC1[constellation] << ACC1Renderer::computeEdgePoint(e, true);
      datasACC1[constellation] << ACC1Renderer::computeEdgePoint(e->twin, false);
      datasACC1[constellation] << ACC1Renderer::computeInteriorPoint(e);
    }
  } else {
    // Pre-compute p(i) (ACC2 paper section 3.2)
    QVector<QVector5D> cornerPoints;
    foreach (HalfEdge *e, getFaceEdges(firstTransitionEdge))
      cornerPoints << ACC2Renderer::computeCornerPoint(e);

    // Compute e(i)+, e(i+1)-, f(i)+ and f(i+1)- (ACC2 paper section 3.4)
    HalfEdge *e = firstTransitionEdge;
    for (int i = 0; i < f.val; ++i) {
      QVector5D p = cornerPoints[i];
      QVector5D p1 = cornerPoints[(i + 1) % f.val];
      QVector5D ep = ACC2Renderer::computeEdgePoint(e, p, true);
      QVector5D em = ACC2Renderer::computeEdgePoint(e->twin, p1, false);
      QVector5D fp = ACC2Renderer::computeFacePoint(e, ep, em, f.val == 3 ? 4 : 3, true);
      QVector5D fm = ACC2Renderer::computeFacePoint(e->twin, em, ep, f.val == 3 ? 4 : 3, false);
      datasACC2[constellation] << p;
      datasACC2[constellation] << ep;
      datasACC2[constellation] << em;
      datasACC2[constellation] << fp;
      datasACC2[constellation] << fm;
      e = e->next;
    }
  }
}

void TransitionPatchRenderer::setData() {
  // Collect ACC1 data
  QVector<float> dataACC1;
  int offsetACC1 = 0;
  foreach (QString constellation, datasACC1.keys()) {
    dataACC1.append(datasACC1[constellation]);
    controlPointsOffsetsACC1[constellation] = offsetACC1;
    controlPointsSizesACC1[constellation] = datasACC1[constellation].size() / 5;
    offsetACC1 += controlPointsSizesACC1[constellation];
  }

  // Collect ACC2 data
  QVector<float> dataACC2;
  int offsetACC2 = 0;
  foreach (QString constellation, datasACC2.keys()) {
    dataACC2.append(datasACC2[constellation]);
    controlPointsOffsetsACC2[constellation] = offsetACC2;
    controlPointsSizesACC2[constellation] = datasACC2[constellation].size() / 5;
    offsetACC2 += controlPointsSizesACC2[constellation];
  }

  // Set data ACC1
  functions->glBindBuffer(GL_ARRAY_BUFFER, VBOACC1);
  functions->glBufferData(GL_ARRAY_BUFFER, sizeof(float) * dataACC1.size(), dataACC1.data(), GL_DYNAMIC_DRAW);

  // Set data ACC2
  functions->glBindBuffer(GL_ARRAY_BUFFER, VBOACC2);
  functions->glBufferData(GL_ARRAY_BUFFER, sizeof(float) * dataACC2.size(), dataACC2.data(), GL_DYNAMIC_DRAW);
};

void TransitionPatchRenderer::render() {
  // Set environment
  functions->glPolygonMode(GL_FRONT_AND_BACK, showWireframe ? GL_LINE : GL_FILL);

  // Draw ACC1 patches
  functions->glBindVertexArray(VAOACC1);
  functions->glPatchParameteri(GL_PATCH_VERTICES, 16);

  shaderPrograms["ACC1-C1"]->bind();
  functions->glDrawArraysInstanced(GL_PATCHES, controlPointsOffsetsACC1["C1"], controlPointsSizesACC1["C1"], 3);
  shaderPrograms["ACC1-C1"]->release();

  shaderPrograms["ACC1-C2"]->bind();
  functions->glDrawArraysInstanced(GL_PATCHES, controlPointsOffsetsACC1["C2"], controlPointsSizesACC1["C2"], 4);
  shaderPrograms["ACC1-C2"]->release();

  shaderPrograms["ACC1-C3"]->bind();
  functions->glDrawArraysInstanced(GL_PATCHES, controlPointsOffsetsACC1["C3"], controlPointsSizesACC1["C3"], 2);
  shaderPrograms["ACC1-C3"]->release();

  shaderPrograms["ACC1-C4-Quad"]->bind();
  functions->glDrawArraysInstanced(GL_PATCHES, controlPointsOffsetsACC1["C4"], controlPointsSizesACC1["C4"], 1);
  shaderPrograms["ACC1-C4-Quad"]->release();

  shaderPrograms["ACC1-C4-Triangles"]->bind();
  functions->glDrawArraysInstanced(GL_PATCHES, controlPointsOffsetsACC1["C4"], controlPointsSizesACC1["C4"], 3);
  shaderPrograms["ACC1-C4-Triangles"]->release();

  shaderPrograms["ACC1-C5"]->bind();
  functions->glDrawArraysInstanced(GL_PATCHES, controlPointsOffsetsACC1["C5"], controlPointsSizesACC1["C5"], 4);
  shaderPrograms["ACC1-C5"]->release();

  // Draw ACC2 patches
  functions->glBindVertexArray(VAOACC2);
  functions->glPatchParameteri(GL_PATCH_VERTICES, 20);

  shaderPrograms["ACC2-C1"]->bind();
  functions->glDrawArraysInstanced(GL_PATCHES, controlPointsOffsetsACC2["C1"], controlPointsSizesACC2["C1"], 3);
  shaderPrograms["ACC2-C1"]->release();

  shaderPrograms["ACC2-C2"]->bind();
  functions->glDrawArraysInstanced(GL_PATCHES, controlPointsOffsetsACC2["C2"], controlPointsSizesACC2["C2"], 4);
  shaderPrograms["ACC2-C2"]->release();

  shaderPrograms["ACC2-C3"]->bind();
  functions->glDrawArraysInstanced(GL_PATCHES, controlPointsOffsetsACC2["C3"], controlPointsSizesACC2["C3"], 2);
  shaderPrograms["ACC2-C3"]->release();

  shaderPrograms["ACC2-C4-Quad"]->bind();
  functions->glDrawArraysInstanced(GL_PATCHES, controlPointsOffsetsACC2["C4"], controlPointsSizesACC2["C4"], 1);
  shaderPrograms["ACC2-C4-Quad"]->release();

  shaderPrograms["ACC2-C4-Triangles"]->bind();
  functions->glDrawArraysInstanced(GL_PATCHES, controlPointsOffsetsACC2["C4"], controlPointsSizesACC2["C4"], 3);
  shaderPrograms["ACC2-C4-Triangles"]->release();

  shaderPrograms["ACC2-C5"]->bind();
  functions->glDrawArraysInstanced(GL_PATCHES, controlPointsOffsetsACC2["C5"], controlPointsSizesACC2["C5"], 4);
  shaderPrograms["ACC2-C5"]->release();

  functions->glBindVertexArray(0);
}

QHash<QString, int> TransitionPatchRenderer::getCountInfo() {
  QHash<QString, int> countInfo;
  countInfo["ACC1 control points"] = controlPointsSizesACC1["C1"] + controlPointsSizesACC1["C2"] + controlPointsSizesACC1["C3"] + controlPointsSizesACC1["C4"] + controlPointsSizesACC1["C5"];
  countInfo["ACC2 control points"] = controlPointsSizesACC2["C1"] + controlPointsSizesACC2["C2"] + controlPointsSizesACC2["C3"] + controlPointsSizesACC2["C4"] + controlPointsSizesACC2["C5"];
  countInfo["ACC1 faces"] = countInfo["ACC1 control points"] / 16;
  countInfo["ACC2 faces"] = countInfo["ACC2 control points"] / 20;
  countInfo["ACC1 invocations"] = (3 * controlPointsSizesACC1["C1"] + 4 * controlPointsSizesACC1["C2"] + 2 * controlPointsSizesACC1["C3"] + 4 * controlPointsSizesACC1["C4"] + 4 * controlPointsSizesACC1["C5"]) / 16;
  countInfo["ACC2 invocations"] = (3 * controlPointsSizesACC2["C1"] + 4 * controlPointsSizesACC2["C2"] + 2 * controlPointsSizesACC2["C3"] + 4 * controlPointsSizesACC2["C4"] + 4 * controlPointsSizesACC2["C5"]) / 20;
  countInfo["faces"] = countInfo["ACC1 faces"] + countInfo["ACC2 faces"];
  countInfo["control points"] = countInfo["ACC1 control points"] + countInfo["ACC2 control points"];
  countInfo["invocations"] = countInfo["ACC1 invocations"] + countInfo["ACC2 invocations"];
  return countInfo;
}
