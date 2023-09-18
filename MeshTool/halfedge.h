#ifndef HALFEDGE
#define HALFEDGE

#include <QVector3D>

class Vertex;
class Face;

class HalfEdge {

public:
  QVector3D color;
  Vertex* target;
  HalfEdge* next;
  HalfEdge* prev;
  HalfEdge* twin;
  Face* polygon;
  unsigned int index;
  bool isSharp;

  HalfEdge() {
    color = QVector3D(1, 1, 1);
    target = nullptr;
    next = nullptr;
    prev = nullptr;
    twin = nullptr;
    polygon = nullptr;
    index = 0;
    isSharp = false;
  }

};

#endif // HALFEDGE
