#include "mesh.h"
#include "math.h"

Mesh Mesh::copy() {
  Mesh mesh;
  mesh.Vertices.resize(this->Vertices.size());
  mesh.Faces.resize(this->Faces.size());
  mesh.HalfEdges.resize(this->HalfEdges.size());

  for (int i = 0; i < this->Vertices.size(); ++i) {
    mesh.Vertices[i] = this->Vertices[i];
    mesh.Vertices[i].out = &mesh.HalfEdges[this->Vertices[i].out->index];
  }

  for (int i = 0; i < this->Faces.size(); ++i) {
    mesh.Faces[i] = this->Faces[i];
    mesh.Faces[i].side = &mesh.HalfEdges[this->Faces[i].side->index];
  }

  for (int i = 0; i < this->HalfEdges.size(); ++i) {
    mesh.HalfEdges[i] = this->HalfEdges[i];
    mesh.HalfEdges[i].target = &mesh.Vertices[this->HalfEdges[i].target->index];
    mesh.HalfEdges[i].next = &mesh.HalfEdges[this->HalfEdges[i].next->index];
    mesh.HalfEdges[i].prev = &mesh.HalfEdges[this->HalfEdges[i].prev->index];
    mesh.HalfEdges[i].twin = &mesh.HalfEdges[this->HalfEdges[i].twin->index];
    mesh.HalfEdges[i].polygon = this->HalfEdges[i].polygon ? &mesh.Faces[this->HalfEdges[i].polygon->index] : nullptr;
  }

  return mesh;
}
