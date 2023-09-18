#ifndef EDIT_H
#define EDIT_H

#include <QVector2D>
#include <QVector>

class CoordsEdit {
  public:
    int edgeIndex;
    float val1;
    float val2;
    bool boundary;
    QVector<int> affectedEdgeIndices;

    CoordsEdit() {
      edgeIndex = -1;
      val1 = 0;
      val2 = 0;
      boundary = false;
    }

    CoordsEdit(int edgeIndex, float val1, float val2, bool boundary) {
      this->edgeIndex = edgeIndex;
      this->val1 = val1;
      this->val2 = val2;
      this->boundary = boundary;
    }
};

#endif // EDIT_H
