#ifndef FACE
#define FACE

#include <QVector3D>

// Forward declaration
class HalfEdge;

class Face {

public:
  HalfEdge* side;
  unsigned short val;
  unsigned int index;

  // Inline constructors

  Face() {
    side = nullptr;
    val = 0;
    index = 0;
  }

  Face(HalfEdge* fside, unsigned short fval, unsigned int findex) {
    side = fside;
    val = fval;
    index = findex;
  }
};

#endif // FACE
