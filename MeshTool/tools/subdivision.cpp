#include "subdivision.h"
#include "convenience.h"

bool isRegularVertex(HalfEdge *inputEdge) {
  Vertex *origin = inputEdge->prev->target;

  // Check if all adjacent faces are quads
  foreach (HalfEdge *e, getVertexEdges(inputEdge)) {
    if (e->polygon && e->polygon->val != 4)
      return false;
  }

  // Check vertex valency
  if (isBoundaryVertex(origin)) {
    if (origin->val > 3)
      return false;
  } else {
    if (origin->val != 4)
      return false;
  }

  // Check color valency
  if (!isSmoothVertex(origin)) {
    if (getColorVertexVal(inputEdge) > 3)
      return false;
  } else {
    if (getColorVertexVal(inputEdge) != 4)
      return false;
  }

  return true;
}

bool isRegularFace(Face f) {
  // Check face valency
  if (f.val != 4)
    return false;

  // Check if all face vertices are regular
  foreach (HalfEdge *e, getFaceEdges(f.side)) {
    if (!isRegularVertex(e))
      return false;
  }

  return true;
}

Mesh computeLimitMesh(Mesh inputMesh) {
  Mesh limitMesh = inputMesh.copy();
  for (int i = 0; i < inputMesh.Vertices.size(); ++i)
    limitMesh.Vertices[i].coords = computeLimitPointCoords(inputMesh.Vertices[i].out);
  for (int i = 0; i < inputMesh.HalfEdges.size(); ++i)
    limitMesh.HalfEdges[i].color = computeLimitPointColor(&inputMesh.HalfEdges[i]);
  return limitMesh;
}

QVector2D computeMeanFaceCoords(HalfEdge *inputEdge) {
  QVector2D sum;
  foreach (HalfEdge *e, getFaceEdges(inputEdge))
    sum += e->target->coords;
  return sum / inputEdge->polygon->val;
}

QVector3D computeMeanFaceColor(HalfEdge *inputEdge) {
  QVector3D sum;
  foreach (HalfEdge *e, getFaceEdges(inputEdge))
    sum += e->color;
  return sum / inputEdge->polygon->val;
}

QVector2D computeEdgeMidpointCoords(HalfEdge *e) {
  return (e->prev->target->coords + e->target->coords) / 2;
}

QVector3D computeEdgeMidpointColor(HalfEdge *e) {
  return (e->color + e->next->color) / 2;
}

QVector2D computeLimitPointCoords(HalfEdge *inputEdge) {
  Vertex *v = inputEdge->prev->target;
  int n = v->val;

  if (!isBoundaryVertex(v)) {
    // Non-boundary case (ACC2 paper section 3.2)
    QVector2D sum;
    foreach (HalfEdge *e, getVertexEdges(v->out))
      sum += computeMeanFaceCoords(e) + computeEdgeMidpointCoords(e);
    return (n - 3.0) / (n + 5) * v->coords + 4.0 / (n * (n + 5)) * sum;
  } else if (n == 2) {
    // Corner case (ACC2 paper section 4.6)
    return v->coords;
  } else {
    // Other boundary cases (ACC2 paper section 4.6)
    Vertex *first = getCCWBoundaryEdge(inputEdge)->target;
    Vertex *last = getCWBoundaryEdge(inputEdge)->target;
    return (first->coords + 4 * v->coords + last->coords) / 6;
  }
}

QVector3D computeLimitPointColor(HalfEdge *inputEdge) {
  Vertex *v = inputEdge->prev->target;
  int n = v->val;

  if (isSmoothVertex(v)) {
    // Similar to non-boundary case
    QVector3D sum;
    foreach (HalfEdge *e, getVertexEdges(v->out))
      sum += computeMeanFaceColor(e) + (computeEdgeMidpointColor(e) + computeEdgeMidpointColor(e->twin)) / 2; // Note: Fix for dart vertices
    return (n - 3.0) / (n + 5) * inputEdge->color + 4.0 / (n * (n + 5)) * sum;
  } else if (isSharpEdge(inputEdge) && isSharpEdge(inputEdge->prev)) {
    // Similar to corner case
    return inputEdge->color;
  } else {
    // Similar to other boundary cases
    HalfEdge *first = getCCWSharpEdge(inputEdge->prev->twin);
    HalfEdge *last = getCWSharpEdge(inputEdge);
    return (first->twin->color + 4 * inputEdge->color + last->next->color) / 6;
  }
}

QVector2D computeInvertedLimitPointCoords(Vertex *v, QVector2D limitPoint) {
  int n = v->val;

  if (!isBoundaryVertex(v)) {
    // Non-boundary case
    QVector2D sum;
    float vContrib = 0;
    foreach (HalfEdge *e, getVertexEdges(v->out)) {
      sum += computeMeanFaceCoords(e) + computeEdgeMidpointCoords(e);
      vContrib += 1.0 / e->polygon->val + .5;
    }

    // Compute sum without contribution of v
    QVector2D sumWithoutV = sum - vContrib * v->coords;

    // Take corresponding formula from computeLimitPoint function, split 'sum' into 'sumWithoutV + v * vContrib', and extract v.
    return (limitPoint - 4.0 / (n * (n + 5)) * sumWithoutV) / ((n - 3.0) / (n + 5) + 4.0 / (n * (n + 5)) * vContrib);
  } else if (n == 2) {
    // Corner case
    return limitPoint;
  } else {
    // Other boundary cases (invert corresponding formula in computeLimitPoint)
    Vertex *first = getCCWBoundaryEdge(v->out)->target;
    Vertex *last = getCWBoundaryEdge(v->out)->target;
    return (6 * limitPoint - first->coords - last->coords) / 4;
  }
}

void subdivideTernaryStep(Mesh* inputMesh, Mesh* subdivMesh) {

  // --- EXPLANATION ---

  // Vertices
  // * First 'inputMesh->Vertices.size()' elements map input vertices 'a' to output vertices 'a' using 'a.index = a.index'
  // * Next 'inputMesh->HalfEdges.size()' elements map input halfedges 'e' to output vertices 'b' using 'b.index = inputMesh->Vertices.size() + e.index'
  // * Next 'sumFaceVal' elements map input non-boundary halfedge 'e' to output vertex 'c' using 'c.index = inputMesh->Vertices.size() + inputMesh->HalfEdges.size() + e.index'
  //
  // *-----c-----*-----*
  // |     |     |     |
  // |     |     |     |
  // |     |     |     |
  // a-----b-----*-----*
  //  --------e------->

  // Halfedges
  // * First '9 * sumFaceVal' elements map non-boundary halfedges 'e' to output halfedges x in [0, 8] using '0.index = 9 * e.index', '1.index = 9 * e.index + 1' etc.
  // * Next '3 * (inputMesh->HalfEdges.size() - sumFaceVal)' elements map boundary halfedges 'e' to output halfedges x in [0, 2] using '0.index = 6 * sumFaceVal + 3 * e.index', '1.index = 6 * sumFaceVal + 3 * e.index + 1' etc.
  //
  //       |  8  |
  // *-----*-----*-----*
  // |     |  7  |     |
  // |    3|4   5|6    |
  // |  0  |  1  |  2  |
  // *-----*-----*-----*
  //  --------e------->

  // Faces
  // * First '2 * sumFaceVal' elements map non-boundary halfedges 'e' to output faces 'a' and 'b' using 'a.index = 2 * e.index' and 'b.index = 2 * e.index + 1'
  // * Next 'inputMesh->Faces.size()' elements map input faces 'f' to output faces 'c' using 'c.index = 2 * sumFaceVal + f.index'
  //
  //       |  c  |
  // *-----*-----*-----*
  // |     |     |     |
  // |  a  |  b  |     |
  // |     |     |     |
  // *-----*-----*-----*
  //  --------e------->


  // --- INITIALIZE ---

  // Compute sum of face valences
  int sumFaceVal = 0;
  for (int i = 0; i < inputMesh->Faces.size(); ++i)
    sumFaceVal += inputMesh->Faces[i].val;

  // Resize
  subdivMesh->Vertices.resize(inputMesh->Vertices.size() + inputMesh->HalfEdges.size() + sumFaceVal);
  subdivMesh->HalfEdges.resize(6 * sumFaceVal + 3 * inputMesh->HalfEdges.size());
  subdivMesh->Faces.resize(2 * sumFaceVal + inputMesh->Faces.size());

  // --- ASSIGN VERTICES ---

  // Vertices 'a'
  for (int i = 0; i < inputMesh->Vertices.size(); ++i) {
    subdivMesh->Vertices[i].coords = inputMesh->Vertices[i].coords;
    subdivMesh->Vertices[i].out = &subdivMesh->HalfEdges[9 * inputMesh->Vertices[i].out->index];
    subdivMesh->Vertices[i].val = inputMesh->Vertices[i].val;
    subdivMesh->Vertices[i].index = i;
  }

  // Vertices 'b' ( Paper "A Colour Interpolation Scheme for Topologically Unrestricted Gradient Meshes" section 4.2 )
  for (int i = 0; i < inputMesh->HalfEdges.size(); ++i) {
    HalfEdge e = inputMesh->HalfEdges[i];

    Vertex V0 = subdivMesh->Vertices[e.twin->target->index];
    Vertex V3 = subdivMesh->Vertices[e.target->index];
    QVector2D P0 = V0.coords;
    QVector2D P3 = V3.coords;
    QVector2D D1 = (P3 - P0) / 3;
    QVector2D P1 = P0 + D1;

    int idx = inputMesh->Vertices.size() + e.index;
    subdivMesh->Vertices[idx].coords = P1;
    subdivMesh->Vertices[idx].out = &subdivMesh->HalfEdges[e.polygon ? (9 * e.index + 1) : (9 * e.twin->index + 2)];
    subdivMesh->Vertices[idx].val = (e.polygon && e.twin->polygon) ? 4 : 3;
    subdivMesh->Vertices[idx].index = idx;
  }

  // Vertices 'c' ( Paper "A Colour Interpolation Scheme for Topologically Unrestricted Gradient Meshes" section 4.3 )
  foreach (Face f, inputMesh->Faces) {

    // Compute face center
    QVector2D C = computeMeanFaceCoords(f.side);

    foreach (HalfEdge *e, getFaceEdges(f.side)) {
      // Compute coordinates
      QVector2D V0 = e->prev->target->coords;
      QVector2D V1 = e->target->coords;
      QVector2D V2 = e->prev->prev->target->coords;
      QVector2D D1 = (V1 - V0) / 3;
      QVector2D D2 = (V2 - V0) / 3;
      QVector2D E1 = V0 + (V0 - V1).length() * D1.normalized() / 2;
      QVector2D E2 = V0 + (V0 - V2).length() * D2.normalized() / 2;
      float d1 = 2 * D1.length() / (V0 - V1).length(); qDebug() << "d1 " << d1;
      float d2 = 2 * D2.length() / (V0 - V2).length(); qDebug() << "d2 " << d2;
      QVector2D F = (1 - d1) * (1 - d2) * V0 + d1 * d2 * C + d2 * (1 - d1) * E1 + d1 * (1 - d2) * E2;

      // Assign
      int idx = inputMesh->Vertices.size() + inputMesh->HalfEdges.size() + e->index;
      subdivMesh->Vertices[idx].coords = F;
      subdivMesh->Vertices[idx].out = &subdivMesh->HalfEdges[9 * e->index + 8];
      subdivMesh->Vertices[idx].val = 4;
      subdivMesh->Vertices[idx].index = idx;
    }
  }

  // --- ASSIGN HALFEDGES ---

  // Non-boundary halfedges
  for (int i = 0; i < sumFaceVal; ++i) {

    HalfEdge e = inputMesh->HalfEdges[i];

    // Halfedge '0'
    int idx = 9 * e.index;
    subdivMesh->HalfEdges[idx].color = e.color;
    subdivMesh->HalfEdges[idx].target = &subdivMesh->Vertices[inputMesh->Vertices.size() + e.index];
    subdivMesh->HalfEdges[idx].next = &subdivMesh->HalfEdges[9 * e.index + 3];
    subdivMesh->HalfEdges[idx].prev = &subdivMesh->HalfEdges[9 * e.prev->index + 2];
    subdivMesh->HalfEdges[idx].twin = &subdivMesh->HalfEdges[e.twin->polygon ? (9 * e.twin->index + 2) : (6 * sumFaceVal + 3 * e.twin->index + 2)];
    subdivMesh->HalfEdges[idx].polygon = &subdivMesh->Faces[2 * e.index];
    subdivMesh->HalfEdges[idx].index = idx;

    // Halfedge '1'
    idx = 9 * e.index + 1;
    subdivMesh->HalfEdges[idx].color = e.color;
    subdivMesh->HalfEdges[idx].target = &subdivMesh->Vertices[inputMesh->Vertices.size() + e.twin->index];
    subdivMesh->HalfEdges[idx].next = &subdivMesh->HalfEdges[9 * e.index + 5];
    subdivMesh->HalfEdges[idx].prev = &subdivMesh->HalfEdges[9 * e.index + 4];
    subdivMesh->HalfEdges[idx].twin = &subdivMesh->HalfEdges[e.twin->polygon ? (9 * e.twin->index + 1) : (6 * sumFaceVal + 3 * e.twin->index + 1)];
    subdivMesh->HalfEdges[idx].polygon = &subdivMesh->Faces[2 * e.index + 1];
    subdivMesh->HalfEdges[idx].index = idx;

    // Halfedge '2'
    idx = 9 * e.index + 2;
    subdivMesh->HalfEdges[idx].color = e.next->color;
    subdivMesh->HalfEdges[idx].target = &subdivMesh->Vertices[e.target->index];
    subdivMesh->HalfEdges[idx].next = &subdivMesh->HalfEdges[9 * e.next->index];
    subdivMesh->HalfEdges[idx].prev = &subdivMesh->HalfEdges[9 * e.index + 6];
    subdivMesh->HalfEdges[idx].twin = &subdivMesh->HalfEdges[e.twin->polygon ? (9 * e.twin->index) : (6 * sumFaceVal + 3 * e.twin->index)];
    subdivMesh->HalfEdges[idx].polygon = &subdivMesh->Faces[2 * e.next->index];
    subdivMesh->HalfEdges[idx].index = idx;

    // Halfedge '3'
    idx = 9 * e.index + 3;
    subdivMesh->HalfEdges[idx].color = e.color;
    subdivMesh->HalfEdges[idx].target = &subdivMesh->Vertices[inputMesh->Vertices.size() + inputMesh->HalfEdges.size() + e.index];
    subdivMesh->HalfEdges[idx].next = &subdivMesh->HalfEdges[9 * e.prev->index + 6];
    subdivMesh->HalfEdges[idx].prev = &subdivMesh->HalfEdges[9 * e.index];
    subdivMesh->HalfEdges[idx].twin = &subdivMesh->HalfEdges[9 * e.index + 4];
    subdivMesh->HalfEdges[idx].polygon = &subdivMesh->Faces[2 * e.index];
    subdivMesh->HalfEdges[idx].index = idx;

    // Halfedge '4'
    idx = 9 * e.index + 4;
    subdivMesh->HalfEdges[idx].color = e.color;
    subdivMesh->HalfEdges[idx].target = &subdivMesh->Vertices[inputMesh->Vertices.size() + e.index];
    subdivMesh->HalfEdges[idx].next = &subdivMesh->HalfEdges[9 * e.index + 1];
    subdivMesh->HalfEdges[idx].prev = &subdivMesh->HalfEdges[9 * e.index + 7];
    subdivMesh->HalfEdges[idx].twin = &subdivMesh->HalfEdges[9 * e.index + 3];
    subdivMesh->HalfEdges[idx].polygon = &subdivMesh->Faces[2 * e.index + 1];
    subdivMesh->HalfEdges[idx].index = idx;

    // Halfedge '5'
    idx = 9 * e.index + 5;
    subdivMesh->HalfEdges[idx].color = e.next->color;
    subdivMesh->HalfEdges[idx].target = &subdivMesh->Vertices[inputMesh->Vertices.size() + inputMesh->HalfEdges.size() + e.next->index];
    subdivMesh->HalfEdges[idx].next = &subdivMesh->HalfEdges[9 * e.index + 7];
    subdivMesh->HalfEdges[idx].prev = &subdivMesh->HalfEdges[9 * e.index + 1];
    subdivMesh->HalfEdges[idx].twin = &subdivMesh->HalfEdges[9 * e.index + 6];
    subdivMesh->HalfEdges[idx].polygon = &subdivMesh->Faces[2 * e.index + 1];
    subdivMesh->HalfEdges[idx].index = idx;

    // Halfedge '6'
    idx = 9 * e.index + 6;
    subdivMesh->HalfEdges[idx].color = e.next->color;
    subdivMesh->HalfEdges[idx].target = &subdivMesh->Vertices[inputMesh->Vertices.size() + e.twin->index];
    subdivMesh->HalfEdges[idx].next = &subdivMesh->HalfEdges[9 * e.index + 2];
    subdivMesh->HalfEdges[idx].prev = &subdivMesh->HalfEdges[9 * e.next->index + 3];
    subdivMesh->HalfEdges[idx].twin = &subdivMesh->HalfEdges[9 * e.index + 5];
    subdivMesh->HalfEdges[idx].polygon = &subdivMesh->Faces[2 * e.next->index];
    subdivMesh->HalfEdges[idx].index = idx;

    // Halfedge '7'
    idx = 9 * e.index + 7;
    subdivMesh->HalfEdges[idx].color = e.next->color;
    subdivMesh->HalfEdges[idx].target = &subdivMesh->Vertices[inputMesh->Vertices.size() + inputMesh->HalfEdges.size() + e.index];
    subdivMesh->HalfEdges[idx].next = &subdivMesh->HalfEdges[9 * e.index + 4];
    subdivMesh->HalfEdges[idx].prev = &subdivMesh->HalfEdges[9 * e.index + 5];
    subdivMesh->HalfEdges[idx].twin = &subdivMesh->HalfEdges[9 * e.index + 8];
    subdivMesh->HalfEdges[idx].polygon = &subdivMesh->Faces[2 * e.index + 1];
    subdivMesh->HalfEdges[idx].index = idx;

    // Halfedge '8'
    idx = 9 * e.index + 8;
    subdivMesh->HalfEdges[idx].color = e.color;
    subdivMesh->HalfEdges[idx].target = &subdivMesh->Vertices[inputMesh->Vertices.size() + inputMesh->HalfEdges.size() + e.next->index];
    subdivMesh->HalfEdges[idx].next = &subdivMesh->HalfEdges[9 * e.next->index + 8];
    subdivMesh->HalfEdges[idx].prev = &subdivMesh->HalfEdges[9 * e.prev->index + 8];
    subdivMesh->HalfEdges[idx].twin = &subdivMesh->HalfEdges[9 * e.index + 7];
    subdivMesh->HalfEdges[idx].polygon = &subdivMesh->Faces[2 * sumFaceVal + e.polygon->index];
    subdivMesh->HalfEdges[idx].index = idx;

  }

  // Boundary halfedges
  for (int i = 0; i < inputMesh->HalfEdges.size() - sumFaceVal; ++i) {
    HalfEdge e = inputMesh->HalfEdges[sumFaceVal + i];

    // Halfedge '0'
    int idx = 6 * sumFaceVal + 3 * e.index;
    subdivMesh->HalfEdges[idx].target = &subdivMesh->Vertices[inputMesh->Vertices.size() + e.index];
    subdivMesh->HalfEdges[idx].next = &subdivMesh->HalfEdges[6 * sumFaceVal + 3 * e.index + 1];
    subdivMesh->HalfEdges[idx].prev = &subdivMesh->HalfEdges[6 * sumFaceVal + 3 * e.prev->index + 2];
    subdivMesh->HalfEdges[idx].twin = &subdivMesh->HalfEdges[9 * e.twin->index + 2];
    subdivMesh->HalfEdges[idx].polygon = nullptr;
    subdivMesh->HalfEdges[idx].index = idx;

    // Halfedge '1'
    idx = 6 * sumFaceVal + 3 * e.index + 1;
    subdivMesh->HalfEdges[idx].target = &subdivMesh->Vertices[inputMesh->Vertices.size() + e.twin->index];
    subdivMesh->HalfEdges[idx].next = &subdivMesh->HalfEdges[6 * sumFaceVal + 3 * e.index + 2];
    subdivMesh->HalfEdges[idx].prev = &subdivMesh->HalfEdges[6 * sumFaceVal + 3 * e.index];
    subdivMesh->HalfEdges[idx].twin = &subdivMesh->HalfEdges[9 * e.twin->index + 1];
    subdivMesh->HalfEdges[idx].polygon = nullptr;
    subdivMesh->HalfEdges[idx].index = idx;

    // Halfedge '2'
    idx = 6 * sumFaceVal + 3 * e.index + 2;
    subdivMesh->HalfEdges[idx].target = &subdivMesh->Vertices[e.target->index];
    subdivMesh->HalfEdges[idx].next = &subdivMesh->HalfEdges[6 * sumFaceVal + 3 * e.next->index];
    subdivMesh->HalfEdges[idx].prev = &subdivMesh->HalfEdges[6 * sumFaceVal + 3 * e.index + 1];
    subdivMesh->HalfEdges[idx].twin = &subdivMesh->HalfEdges[9 * e.twin->index];
    subdivMesh->HalfEdges[idx].polygon = nullptr;
    subdivMesh->HalfEdges[idx].index = idx;
  }

  // --- ASSIGN FACES ---

  // Faces 'a' and 'b'
  for (int i = 0; i < sumFaceVal; ++i) {
    HalfEdge e = inputMesh->HalfEdges[i];

    // Face 'a'
    int idx = 2 * e.index;
    subdivMesh->Faces[idx].side = &subdivMesh->HalfEdges[9 * e.index];
    subdivMesh->Faces[idx].val = 4;
    subdivMesh->Faces[idx].index = idx;

    // Face 'b'
    idx = 2 * e.index + 1;
    subdivMesh->Faces[idx].side = &subdivMesh->HalfEdges[9 * e.index + 1];
    subdivMesh->Faces[idx].val = 4;
    subdivMesh->Faces[idx].index = idx;
  }

  // Faces 'c'
  for (int i = 0; i < inputMesh->Faces.size(); ++i) {
    Face f = inputMesh->Faces[i];

    int idx = 2 * sumFaceVal + f.index;
    subdivMesh->Faces[idx].side = &subdivMesh->HalfEdges[9 * f.side->index + 8];
    subdivMesh->Faces[idx].val = f.val;
    subdivMesh->Faces[idx].index = idx;
  }
}

void subdivideCatmullClark(Mesh* inputMesh, Mesh* subdivMesh) {

  // --- EXPLANATION ---

  // Vertices
  // * First 'inputMesh->Vertices.size()' elements map input vertices 'a' to output vertices 'a' using 'a.index = a.index'
  // * Next 'inputMesh->HalfEdges.size() / 2' elements map input halfedges 'e' to output vertices 'b' using 'b.index = edgeVertexMapping[qMin(e.index, e.twin->index)]'
  // * Next 'inputMesh->Faces.size()' elements map input non-boundary halfedge 'e' to output vertex 'c' using 'c.index = inputMesh->Vertices.size() + inputMesh->HalfEdges.size() / 2 + e.index'
  //
  // *-----c-----*
  // |     |     |
  // |     |     |
  // |     |     |
  // a-----b-----*
  //  -----e---->

  // Halfedges
  // * First '4 * sumFaceVal' elements map non-boundary halfedges 'e' to output halfedges x in [0, 3] using '0.index = 4 * e.index', '1.index = 4 * e.index + 1' etc.
  // * Next '2 * (inputMesh->HalfEdges.size() - sumFaceVal)' elements map boundary halfedges 'e' to output halfedges x in [0, 2] using '0.index = 2 * sumFaceVal + 2 * e.index', '1.index = 2 * sumFaceVal + 2 * e.index + 1' etc.
  //
  // *-----*-----*
  // |     |     |
  // |    2|3    |
  // |  0  |  1  |
  // *-----*-----*
  //  -----e---->

  // Faces
  // * First 'sumFaceVal' elements map non-boundary halfedges 'e' to output faces 'a' using 'a.index = e.index'
  //
  // *-----*-----*
  // |     |     |
  // |  a  |     |
  // |     |     |
  // *-----*-----*
  //  -----e---->


  // --- INITIALIZE ---

  // Compute sum of face valences
  int sumFaceVal = 0;
  for (int i = 0; i < inputMesh->Faces.size(); ++i)
    sumFaceVal += inputMesh->Faces[i].val;

  // Resize
  subdivMesh->Vertices.resize(inputMesh->Vertices.size() + inputMesh->HalfEdges.size() / 2 + inputMesh->Faces.size());
  subdivMesh->HalfEdges.resize(2 * inputMesh->HalfEdges.size() + 2 * sumFaceVal);
  subdivMesh->Faces.resize(sumFaceVal);

  // Compute edge to vertex mapping for vertices 'b'
  QHash<int, int> edgeVertexMapping;
  int idx = inputMesh->Vertices.size();
  for (int i = 0; i < sumFaceVal; ++i) {
    HalfEdge e = inputMesh->HalfEdges[i];
    if (e.index > e.twin->index)
      continue;
    edgeVertexMapping[e.index] = idx++;
  }

  // --- ASSIGN HALFEDGES ---

  // Non-boundary halfedges
  for (int i = 0; i < sumFaceVal; ++i) {
    HalfEdge e = inputMesh->HalfEdges[i];

    // Halfedge '0'
    int idx = 4 * e.index;
    subdivMesh->HalfEdges[idx].target = &subdivMesh->Vertices[edgeVertexMapping[qMin(e.index, e.twin->index)]];
    subdivMesh->HalfEdges[idx].next = &subdivMesh->HalfEdges[4 * e.index + 2];
    subdivMesh->HalfEdges[idx].prev = &subdivMesh->HalfEdges[4 * e.prev->index + 1];
    subdivMesh->HalfEdges[idx].twin = &subdivMesh->HalfEdges[e.twin->polygon ? (4 * e.twin->index + 1) : (2 * sumFaceVal + 2 * e.twin->index + 1)];
    subdivMesh->HalfEdges[idx].polygon = &subdivMesh->Faces[e.index];
    subdivMesh->HalfEdges[idx].index = idx;
    subdivMesh->HalfEdges[idx].isSharp = e.isSharp;

    // Halfedge '1'
    idx = 4 * e.index + 1;
    subdivMesh->HalfEdges[idx].target = &subdivMesh->Vertices[e.target->index];
    subdivMesh->HalfEdges[idx].next = &subdivMesh->HalfEdges[4 * e.next->index];
    subdivMesh->HalfEdges[idx].prev = &subdivMesh->HalfEdges[4 * e.index + 3];
    subdivMesh->HalfEdges[idx].twin = &subdivMesh->HalfEdges[e.twin->polygon ? (4 * e.twin->index) : (2 * sumFaceVal + 2 * e.twin->index)];
    subdivMesh->HalfEdges[idx].polygon = &subdivMesh->Faces[e.next->index];
    subdivMesh->HalfEdges[idx].index = idx;
    subdivMesh->HalfEdges[idx].isSharp = e.isSharp;

    // Halfedge '2'
    idx = 4 * e.index + 2;
    subdivMesh->HalfEdges[idx].target = &subdivMesh->Vertices[inputMesh->Vertices.size() + inputMesh->HalfEdges.size() / 2 + e.polygon->index];
    subdivMesh->HalfEdges[idx].next = &subdivMesh->HalfEdges[4 * e.prev->index + 3];
    subdivMesh->HalfEdges[idx].prev = &subdivMesh->HalfEdges[4 * e.index];
    subdivMesh->HalfEdges[idx].twin = &subdivMesh->HalfEdges[4 * e.index + 3];
    subdivMesh->HalfEdges[idx].polygon = &subdivMesh->Faces[e.index];
    subdivMesh->HalfEdges[idx].index = idx;

    // Halfedge '3'
    idx = 4 * e.index + 3;
    subdivMesh->HalfEdges[idx].target = &subdivMesh->Vertices[edgeVertexMapping[qMin(e.index, e.twin->index)]];
    subdivMesh->HalfEdges[idx].next = &subdivMesh->HalfEdges[4 * e.index + 1];
    subdivMesh->HalfEdges[idx].prev = &subdivMesh->HalfEdges[4 * e.next->index + 2];
    subdivMesh->HalfEdges[idx].twin = &subdivMesh->HalfEdges[4 * e.index + 2];
    subdivMesh->HalfEdges[idx].polygon = &subdivMesh->Faces[e.next->index];
    subdivMesh->HalfEdges[idx].index = idx;

  }

  // Boundary halfedges
  for (int i = 0; i < inputMesh->HalfEdges.size() - sumFaceVal; ++i) {
    HalfEdge e = inputMesh->HalfEdges[sumFaceVal + i];

    // Halfedge '0'
    int idx = 2 * sumFaceVal + 2 * e.index;
    subdivMesh->HalfEdges[idx].target = &subdivMesh->Vertices[edgeVertexMapping[qMin(e.index, e.twin->index)]];
    subdivMesh->HalfEdges[idx].next = &subdivMesh->HalfEdges[2 * sumFaceVal + 2 * e.index + 1];
    subdivMesh->HalfEdges[idx].prev = &subdivMesh->HalfEdges[2 * sumFaceVal + 2 * e.prev->index + 1];
    subdivMesh->HalfEdges[idx].twin = &subdivMesh->HalfEdges[4 * e.twin->index + 1];
    subdivMesh->HalfEdges[idx].polygon = nullptr;
    subdivMesh->HalfEdges[idx].index = idx;

    // Halfedge '1'
    idx = 2 * sumFaceVal + 2 * e.index + 1;
    subdivMesh->HalfEdges[idx].target = &subdivMesh->Vertices[e.target->index];
    subdivMesh->HalfEdges[idx].next = &subdivMesh->HalfEdges[2 * sumFaceVal + 2 * e.next->index];
    subdivMesh->HalfEdges[idx].prev = &subdivMesh->HalfEdges[2 * sumFaceVal + 2 * e.index];
    subdivMesh->HalfEdges[idx].twin = &subdivMesh->HalfEdges[4 * e.twin->index];
    subdivMesh->HalfEdges[idx].polygon = nullptr;
    subdivMesh->HalfEdges[idx].index = idx;
  }

  // --- ASSIGN FACES ---

  // Faces 'a'
  for (int i = 0; i < sumFaceVal; ++i) {
    HalfEdge e = inputMesh->HalfEdges[i];

    int idx = e.index;
    subdivMesh->Faces[idx].side = &subdivMesh->HalfEdges[4 * e.index];
    subdivMesh->Faces[idx].val = 4;
    subdivMesh->Faces[idx].index = idx;
  }

  // --- ASSIGN VERTICES ---

  // Vertices 'c'
  foreach (Face f, inputMesh->Faces) {
    // Compute mean coords and color
    QVector2D coord = computeMeanFaceCoords(f.side);
    QVector3D color = computeMeanFaceColor(f.side);

    // Assign vertex
    int idx = inputMesh->Vertices.size() + inputMesh->HalfEdges.size() / 2 + f.index;
    subdivMesh->Vertices[idx].coords = coord;
    subdivMesh->Vertices[idx].out = &subdivMesh->HalfEdges[4 * f.side->index + 3];
    subdivMesh->Vertices[idx].val = f.val;
    subdivMesh->Vertices[idx].index = idx;

    // Assign color to halfedges
    foreach (HalfEdge *e, getVertexEdges(subdivMesh->Vertices[idx].out))
      e->color = color;
  }

  // Vertices 'b'
  foreach (HalfEdge e, inputMesh->HalfEdges) {
    if (!e.polygon)
      break;
    if (e.index > e.twin->index)
      continue;

    // Compute coordinates (average of the new neighbouring face points and its two original endpoints)
    QVector2D coord;
    QVector3D color;
    if (e.polygon && e.twin->polygon) {
      coord += e.target->coords;
      coord += e.twin->target->coords;
      coord += subdivMesh->Vertices[inputMesh->Vertices.size() + inputMesh->HalfEdges.size() / 2 + e.polygon->index].coords;
      coord += subdivMesh->Vertices[inputMesh->Vertices.size() + inputMesh->HalfEdges.size() / 2 + e.twin->polygon->index].coords;
      coord /= 4;
      color += e.color;
      color += e.next->color;
      color += subdivMesh->Vertices[inputMesh->Vertices.size() + inputMesh->HalfEdges.size() / 2 + e.polygon->index].out->color;
      color += subdivMesh->Vertices[inputMesh->Vertices.size() + inputMesh->HalfEdges.size() / 2 + e.twin->polygon->index].out->color;
      color /= 4;
    } else {
      coord = (e.target->coords + e.twin->target->coords) / 2;
      color = (e.color + e.next->color) / 2;
    }

    // Assign
    int idx = edgeVertexMapping[e.index];
    subdivMesh->Vertices[idx].coords = coord;
    subdivMesh->Vertices[idx].out = &subdivMesh->HalfEdges[4 * e.index + 2];
    subdivMesh->Vertices[idx].val = (e.polygon && e.twin->polygon) ? 4 : 3;
    subdivMesh->Vertices[idx].index = idx;
  }

  // Assign color to halfedges
  foreach (HalfEdge e, inputMesh->HalfEdges) {
    if (!e.polygon)
      break;

    QVector3D color;
    if (!isSharpEdge(&e)) {
      color += e.color;
      color += e.next->color;
      color += subdivMesh->Vertices[inputMesh->Vertices.size() + inputMesh->HalfEdges.size() / 2 + e.polygon->index].out->color;
      color += subdivMesh->Vertices[inputMesh->Vertices.size() + inputMesh->HalfEdges.size() / 2 + e.twin->polygon->index].out->color;
      color /= 4;
    } else {
      color = (e.color + e.next->color) / 2;
    }

    subdivMesh->HalfEdges[4 * e.index + 1].color = color;
    subdivMesh->HalfEdges[4 * e.index + 2].color = color;
  }

  // Vertices 'a'
  foreach (Vertex v, inputMesh->Vertices) {
    // Compute coordinates
    QVector2D coord;
    QVector3D color;
    HalfEdge *e = getCCWBoundaryEdge(v.out);
    if (!e->polygon) {
      if (e->twin->target->val == 2) {
        coord = e->twin->target->coords;
        color = e->color;
      } else {
        coord  = (e->target->coords + 6 * e->twin->target->coords + e->prev->twin->target->coords) / 8;
        color  = (e->prev->color + 6 * e->color + e->next->color) / 8;
      }
    } else {
      QVector2D sumStarCoords, sumFaceCoords;
      QVector3D sumStarColors, sumFaceColors;
      foreach (HalfEdge *e, getVertexEdges(v.out)) {
        sumStarCoords += e->target->coords;
        sumStarColors += e->next->color;
        sumFaceCoords += subdivMesh->Vertices[inputMesh->Vertices.size() + inputMesh->HalfEdges.size() / 2 + e->polygon->index].coords;
        sumFaceColors += subdivMesh->Vertices[inputMesh->Vertices.size() + inputMesh->HalfEdges.size() / 2 + e->polygon->index].out->color;
      }
      int n = v.val;
      coord = ((n - 2) * v.coords + sumStarCoords / n + sumFaceCoords / n) / n;
      color = ((n - 2) * e->color + sumStarColors / n + sumFaceColors / n) / n;
    }

    // Assign
    int idx = v.index;
    subdivMesh->Vertices[idx].coords = coord;
    subdivMesh->Vertices[idx].out = &subdivMesh->HalfEdges[4 * v.out->index];
    subdivMesh->Vertices[idx].val = v.val;
    subdivMesh->Vertices[idx].index = idx;
  }

  foreach (HalfEdge e, inputMesh->HalfEdges) {
    if (!e.polygon)
      break;
    Vertex *v = e.prev->target;

    QVector3D color;
    if (isSmoothVertex(v)) {
      // Non-boundary case
      HalfEdge *e = v->out;
      QVector3D sumStarColors, sumFaceColors;
      foreach (HalfEdge *e, getVertexEdges(v->out)) {
        sumStarColors += (e->next->color + e->twin->color) / 2; // Note: Fix for dart vertices
        sumFaceColors += subdivMesh->Vertices[inputMesh->Vertices.size() + inputMesh->HalfEdges.size() / 2 + e->polygon->index].out->color;
      }
      int n = v->val;
      color = ((n - 2) * e->color + sumStarColors / n + sumFaceColors / n) / n;
    } else if (isSharpEdge(&e) && isSharpEdge(e.prev)) {
      // Corner case
      color = e.color;
    } else {
      // Other boundary case
      HalfEdge *firstEdge = getCCWSharpEdge(e.prev->twin);
      HalfEdge *lastEdge = getCWSharpEdge(&e);
      color = (firstEdge->twin->color + 6 * e.color + lastEdge->next->color) / 8;
    }

    // Assign
    subdivMesh->HalfEdges[4 * e.index].color = color;
  }
}

void computeSubMesh(Mesh *inputMesh, QSet<int> inputFaceIndices, Mesh *outputMesh, QHash<int, int> *outputEdgeMap) {
  // Create input to output map for faces
  QHash<int, int> faceMap;
  int outputFaceIndex = 0;
  foreach (int inputFaceIndex, inputFaceIndices)
    faceMap[inputFaceIndex] = outputFaceIndex++;

  // Create input to output map for vertices and edges
  QHash<int, int> vertexMap, edgeMap;
  int outputVertexIndex = 0, outputEdgeIndex = 0;
  foreach (int inputFaceIndex, faceMap.keys()) {
    Face f = inputMesh->Faces[inputFaceIndex];
    foreach (HalfEdge *e, getFaceEdges(f.side)) {
      // Update vertex map
      if (!vertexMap.contains(e->target->index))
        vertexMap[e->target->index] = outputVertexIndex++;
      // Update edge map for non-boundary halfedges
      if (!edgeMap.contains(e->index))
        edgeMap[e->index] = outputEdgeIndex++;
    }
  }
  foreach (int inputFaceIndex, faceMap.keys()) {
    Face f = inputMesh->Faces[inputFaceIndex];
    foreach (HalfEdge *e, getFaceEdges(f.side)) {
      // Update edge map for boundary halfedges
      if (!edgeMap.contains(e->twin->index))
        edgeMap[e->twin->index] = outputEdgeIndex++;
    }
  }

  // Set output edge map
  *outputEdgeMap = edgeMap;

  // Initialize output mesh
  outputMesh->Faces.resize(faceMap.size());
  outputMesh->Vertices.resize(vertexMap.size());
  outputMesh->HalfEdges.resize(edgeMap.size());

  // Assign faces
  foreach (int inputFaceIndex, faceMap.keys()) {
    int outputFaceIndex = faceMap[inputFaceIndex];
    outputMesh->Faces[outputFaceIndex].side = &outputMesh->HalfEdges[edgeMap[inputMesh->Faces[inputFaceIndex].side->index]];
    outputMesh->Faces[outputFaceIndex].val = inputMesh->Faces[inputFaceIndex].val;
    outputMesh->Faces[outputFaceIndex].index = outputFaceIndex;
  }

  // Assign vertices (valency assigned later)
  foreach (int inputVertexIndex, vertexMap.keys()) {
    int outputVertexIndex = vertexMap[inputVertexIndex];
    outputMesh->Vertices[outputVertexIndex].coords = inputMesh->Vertices[inputVertexIndex].coords;
    HalfEdge *out = inputMesh->Vertices[inputVertexIndex].out;
    while (!out->polygon || !faceMap.contains(out->polygon->index))
      out = out->prev->twin;
    outputMesh->Vertices[outputVertexIndex].out = &outputMesh->HalfEdges[edgeMap[out->index]];
    outputMesh->Vertices[outputVertexIndex].index = outputVertexIndex;
  }

  // Assign halfedges
  foreach (int inputEdgeIndex, edgeMap.keys()) {
    int outputEdgeIndex = edgeMap[inputEdgeIndex];
    outputMesh->HalfEdges[outputEdgeIndex].color = inputMesh->HalfEdges[inputEdgeIndex].color;
    outputMesh->HalfEdges[outputEdgeIndex].target = &outputMesh->Vertices[vertexMap[inputMesh->HalfEdges[inputEdgeIndex].target->index]];
    if (inputMesh->HalfEdges[inputEdgeIndex].polygon && faceMap.contains(inputMesh->HalfEdges[inputEdgeIndex].polygon->index)) {
      // Non-boundary case
      outputMesh->HalfEdges[outputEdgeIndex].next = &outputMesh->HalfEdges[edgeMap[inputMesh->HalfEdges[inputEdgeIndex].next->index]];
      outputMesh->HalfEdges[outputEdgeIndex].prev = &outputMesh->HalfEdges[edgeMap[inputMesh->HalfEdges[inputEdgeIndex].prev->index]];
      outputMesh->HalfEdges[outputEdgeIndex].polygon = &outputMesh->Faces[faceMap[inputMesh->HalfEdges[inputEdgeIndex].polygon->index]];
    } else {
      // Boundary case
      HalfEdge *e = inputMesh->HalfEdges[inputEdgeIndex].next;
      while (!e->twin->polygon || !faceMap.contains(e->twin->polygon->index))
        e = e->twin->next;
      outputMesh->HalfEdges[outputEdgeIndex].next = &outputMesh->HalfEdges[edgeMap[e->index]];
      e = inputMesh->HalfEdges[inputEdgeIndex].prev;
      while (!e->twin->polygon || !faceMap.contains(e->twin->polygon->index))
        e = e->twin->prev;
      outputMesh->HalfEdges[outputEdgeIndex].prev = &outputMesh->HalfEdges[edgeMap[e->index]];
      outputMesh->HalfEdges[outputEdgeIndex].polygon = nullptr;
    }
    outputMesh->HalfEdges[outputEdgeIndex].twin = &outputMesh->HalfEdges[edgeMap[inputMesh->HalfEdges[inputEdgeIndex].twin->index]];
    outputMesh->HalfEdges[outputEdgeIndex].index = outputEdgeIndex;
    outputMesh->HalfEdges[outputEdgeIndex].isSharp = inputMesh->HalfEdges[inputEdgeIndex].isSharp;
  }

  // Update vertex valencies
  for (int i = 0; i < outputMesh->HalfEdges.size(); ++i)
    ++outputMesh->HalfEdges[i].target->val;
}
