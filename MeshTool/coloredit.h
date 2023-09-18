#ifndef COLOREDIT_H
#define COLOREDIT_H

#include <QVector3D>
#include <QVector>

class ColorEdit {
  public:
    int edgeIndex;
    QVector3D color;
    QVector<int> affectedEdgeIndices;

    ColorEdit() {
      edgeIndex = -1;
      color = QVector3D(0, 0, 0);
    }

    ColorEdit(int edgeIndex, QVector3D color) {
      this->edgeIndex = edgeIndex;
      this->color = color;
    }
};

#endif // COLOREDIT_H
