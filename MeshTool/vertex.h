#ifndef VERTEX
#define VERTEX

#include <QVector2D>
#include <QVector3D>
#include <QDebug>

class HalfEdge;

class Vertex {
public:
  QVector2D coords;
  HalfEdge* out;
  unsigned short val;
  unsigned int index;

  Vertex() {
    coords = QVector2D();
    out = nullptr;
    val = 0;
    index = 0;
  }

  Vertex(QVector2D vcoords, HalfEdge* vout, unsigned short vval, unsigned int vindex) {
    coords = vcoords;
    out = vout;
    val = vval;
    index = vindex;
  }

};

#endif // VERTEX
