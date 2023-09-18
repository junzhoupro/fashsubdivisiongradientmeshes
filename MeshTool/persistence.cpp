#include "persistence.h"
#include "vertex.h"
#include "tools/tools.h"
#include <QFile>

void load(QString fileName, Mesh *mesh, QHash<int, QHash<int, CoordsEdit>> *coordsEdits, QHash<int, QHash<int, ColorEdit>> *colorEdits) {
  qDebug() << ":: Loading" << fileName;

  // Clean
  mesh->Vertices.clear();
  mesh->Vertices.squeeze();
  mesh->Faces.clear();
  mesh->Faces.squeeze();
  mesh->HalfEdges.clear();
  mesh->HalfEdges.squeeze();
  coordsEdits->clear();
  coordsEdits->squeeze();
  colorEdits->clear();
  colorEdits->squeeze();

  // Open file
  QFile file(fileName);
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
      qDebug() << " * Could not open file " << fileName << " for reading";
      return;
  }

  // Create filestream
  QTextStream in(&file);

  // First pass to reserve enough memory
  int nVertices = 0;
  int nFaces = 0;
  int sumFaceVal = 0;
  while(!in.atEnd()) {
    QStringList values = in.readLine().split(' ');
    if (values[0] == "v")
      ++nVertices;
    else if (values[0] == "f") {
      ++nFaces;
      sumFaceVal += values.size() - 1;
    }
  }
  mesh->Vertices.reserve(nVertices);
  mesh->Faces.reserve(nFaces);
  mesh->HalfEdges.reserve(2 * sumFaceVal); // Worst case scenario

  // Second pass to construct mesh
  in.seek(0);
  while(!in.atEnd()) {
    QString line = in.readLine();
    QStringList values = line.split(" ");

    if (values[0] == "v") {
      // Add vertex (out and val assigned later)
      mesh->Vertices.append(Vertex());
      Vertex *v = &mesh->Vertices.last();
      v->coords = QVector2D(values[1].toFloat(), values[2].toFloat());
      v->index = mesh->Vertices.size() - 1;
    } else if (values[0] == "f") {
      // Get vertex indices
      QVector<int> vertexIndices;
      for (int i = 1; i < values.size(); ++i)
        vertexIndices.append(values[i].toInt() - 1);

      // Keep index of first added halfedge
      int firstEdgeIdx = mesh->HalfEdges.size();

      // Initialize halfedges of face
      mesh->HalfEdges.resize(mesh->HalfEdges.size() + vertexIndices.size());

      // Add and initialize face
      mesh->Faces.append(Face());
      Face *f = &mesh->Faces.last();
      f->side = &mesh->HalfEdges[firstEdgeIdx];
      f->val = vertexIndices.size();
      f->index = mesh->Faces.size() - 1;

      // Initialize halfedges
      for (int i = 0; i < vertexIndices.size(); ++i) {
        // Update vertex values
        if (!mesh->Vertices[vertexIndices[i]].out)
          mesh->Vertices[vertexIndices[i]].out = &mesh->HalfEdges[firstEdgeIdx + (i + 1) % vertexIndices.size()];
        ++mesh->Vertices[vertexIndices[i]].val;

        // Assign halfedges (twins added later)
        mesh->HalfEdges[firstEdgeIdx + i].target = &mesh->Vertices[vertexIndices[i]];
        mesh->HalfEdges[firstEdgeIdx + i].next = &mesh->HalfEdges[firstEdgeIdx + (i + 1) % vertexIndices.size()];
        mesh->HalfEdges[firstEdgeIdx + i].prev = &mesh->HalfEdges[firstEdgeIdx + (i - 1 + vertexIndices.size()) % vertexIndices.size()];
        mesh->HalfEdges[firstEdgeIdx + i].polygon = f;
        mesh->HalfEdges[firstEdgeIdx + i].index = firstEdgeIdx + i;
      }
    } else if (values[0] == "ve") {
      (*coordsEdits)[values[1].toInt()][values[2].toInt()] = CoordsEdit(values[3].toInt(), values[4].toFloat(), values[5].toFloat(), values[6].toInt());
    } else if (values[0] == "ce") {
      (*colorEdits)[values[1].toInt()][values[2].toInt()] = ColorEdit(values[2].toInt(), QVector3D(values[3].toFloat(), values[4].toFloat(), values[5].toFloat()));
    }
  }

  // Close
  file.close();

  // Assign interior twins
  for (int i = 0; i < mesh->HalfEdges.size(); ++i) {
    HalfEdge *e1 = &mesh->HalfEdges[i];
    for (int j = i + 1; j < mesh->HalfEdges.size(); ++j) {
      HalfEdge *e2 = &mesh->HalfEdges[j];
      if (e1->target == e2->prev->target && e1->prev->target == e2->target) {
        e1->twin = e2;
        e2->twin = e1;
      }
    }
  }

  // Add and assign boundary halfedges (prev and next assigned later)
  for (int i = 0; i < sumFaceVal; ++i) {
    HalfEdge *e1 = &(mesh->HalfEdges[i]);
    if (!e1->twin) {
      mesh->HalfEdges.append(HalfEdge());
      HalfEdge *e2 = &mesh->HalfEdges.last();
      e1->twin = e2;
      e2->twin = e1;
      e2->target = e1->prev->target;
      ++e2->target->val;
      e2->index = mesh->HalfEdges.size() - 1;
    }
  }

  // Assign prev and next of boundary halfedges
  for (int i = sumFaceVal; i < mesh->HalfEdges.size(); ++i) {
    HalfEdge *e1 = &mesh->HalfEdges[i];
    if (!e1->next) {
      HalfEdge *e2 = getCCWBoundaryEdge(e1->target->out);
      e1->next = e2;
      e2->prev = e1;
    }
  }

  // Obtain max edit level
  int maxEditLevel = 0;
  foreach (int level, coordsEdits->keys())
    if (level > maxEditLevel)
      maxEditLevel = level;
  foreach (int level, colorEdits->keys())
    if (level > maxEditLevel)
      maxEditLevel = level;

  // Obtain meshes
  Mesh afterTernaryMesh;
  subdivideTernaryStep(mesh, &afterTernaryMesh);
  QVector<Mesh> meshes;
  meshes.append(afterTernaryMesh);
  for (int i = 0; i < maxEditLevel; ++i) {
    Mesh subdivMesh;
    subdivideCatmullClark(&meshes[i], &subdivMesh);
    meshes.append(subdivMesh);
  }

  // Update affected edges for coordinate edits
  foreach (int level, coordsEdits->keys()) {
    foreach (int vertexIndex, (*coordsEdits)[level].keys()) {
      CoordsEdit *edit = &(*coordsEdits)[level][vertexIndex];
      HalfEdge *e = edit->boundary ? meshes[level].HalfEdges[edit->edgeIndex].twin : &meshes[level].HalfEdges[edit->edgeIndex];
      Vertex *v = e->twin->target;
      QSet<Face *> twoRingFaces = getPadded(QSet<Vertex *>({v}), 2);
      foreach (Face *f, twoRingFaces)
        edit->affectedEdgeIndices << f->side->index;
    }
  }

  // Update affected edges for color edits
  foreach (int level, colorEdits->keys()) {
    foreach (int edgeIndex, (*colorEdits)[level].keys()) {
      ColorEdit *edit = &(*colorEdits)[level][edgeIndex];
      Vertex *v = meshes[level].HalfEdges[edit->edgeIndex].twin->target;
      QSet<Face *> threeRingFaces = getPadded(QSet<Vertex *>({v}), 3);
      foreach (Face *f, threeRingFaces)
        edit->affectedEdgeIndices << f->side->index;
    }
  }

}

void save(QString fileName, Mesh mesh, QHash<int, QHash<int, CoordsEdit>> coordsEdits, QHash<int, QHash<int, ColorEdit>> colorEdits) {
  qDebug() << ":: Saving" << fileName;

  // Open file
  QFile file(fileName);
  if(!file.open(QFile::WriteOnly | QFile::Text)) {
      qDebug() << " * Could not open file " << fileName << " for writing";
      return;
  }

  // Create filestream
  QTextStream out(&file);

  // Write coordinates
  for (int i = 0; i < mesh.Vertices.size(); ++i)
    out << "v " << mesh.Vertices[i].coords.x() << " " << mesh.Vertices[i].coords.y() << "\n";

  // Write faces
  for (int i = 0; i < mesh.Faces.size(); ++i) {
    out << "f";
    HalfEdge *e = mesh.Faces[i].side;
    for (int j = 0; j < mesh.Faces[i].val; ++j) {
      out << " " << e->target->index + 1;
      e = e->next;
    }
    out << "\n";
  }

  // Write coordinate edits
  for (int i = 0; i < coordsEdits.size(); ++i) {
    int editLevel = coordsEdits.keys()[i];
    QHash<int, CoordsEdit> hash = coordsEdits[editLevel];
    for (int j = 0; j < hash.size(); ++j) {
      int vertexIdx = hash.keys()[j];
      CoordsEdit coordsEdit = hash[vertexIdx];
      out << "ve " << editLevel << " " << vertexIdx << " " << coordsEdit.edgeIndex << " " << coordsEdit.val1 << " " << coordsEdit.val2 << " " << coordsEdit.boundary << "\n";
    }
  }

  // Write color edits
  for (int i = 0; i < colorEdits.size(); ++i) {
    int editLevel = colorEdits.keys()[i];
    QHash<int, ColorEdit> hash = colorEdits[editLevel];
    for (int j = 0; j < hash.size(); ++j) {
      int vertexIdx = hash.keys()[j];
      ColorEdit colorEdit = hash[vertexIdx];
      out << "ce " << editLevel << " " << colorEdit.edgeIndex << " " << colorEdit.color.x() << " " << colorEdit.color.y() << " " << colorEdit.color.z() << "\n";
    }
  }

  // Close
  file.flush();
  file.close();
}
