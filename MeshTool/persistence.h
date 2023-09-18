#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "mesh.h"
#include <QVector>
#include <QString>
#include <QHash>
#include "coordsedit.h"
#include "coloredit.h"

void load(QString fileName, Mesh *mesh, QHash<int, QHash<int, CoordsEdit>> *coordsEdits, QHash<int, QHash<int, ColorEdit>> *colorEdits);
void save(QString fileName, Mesh mesh, QHash<int, QHash<int, CoordsEdit>> coordsEdits, QHash<int, QHash<int, ColorEdit>> colorEdits);

#endif // PERSISTENCE_H
