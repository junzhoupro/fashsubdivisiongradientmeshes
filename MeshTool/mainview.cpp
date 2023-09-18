#include "mainview.h"
#include "ui_mainwindow.h"
#include "persistence.h"
#include "tools/tools.h"
#include "tools/editing.h"
#include <QStack>
#include <QApplication>
#include <QElapsedTimer>
#include <QDateTime>

MainView::MainView(QWidget *Parent) : QOpenGLWidget(Parent) {
    qDebug() << "✓✓ MainView constructor";
}

MainView::~MainView() {
    qDebug() << "✗✗ MainView destructor";

    originalMeshes.clear();
    originalMeshes.squeeze();
    editedMeshes.clear();
    editedMeshes.squeeze();
    limitMeshes.clear();
    limitMeshes.squeeze();
    coordsEdits.clear();
    coordsEdits.squeeze();
    colorEdits.clear();
    colorEdits.squeeze();
    editableVertexIndices.clear();
    editableVertexIndices.squeeze();
    gradientVertexIndices.clear();
    gradientVertexIndices.squeeze();
    changedLimitCoordsIndices.clear();
    changedLimitCoordsIndices.squeeze();
    changedEdgesIndices.clear();
    changedEdgesIndices.squeeze();
    changedFacesIndices.clear();
    changedFacesIndices.squeeze();

    debugLogger->stopLogging();

    // Delete renderers
    foreach (SurfaceRenderer *renderer, renderers)
        delete renderer;
    delete pointRenderer;
    delete lineRenderer;

    // Delete difference framebuffer
    delete limitFramebuffer;
    delete diffFramebuffer;
}

// ---

void MainView::setMesh(QString fileName) {
    load(fileName, &inputMesh, &coordsEdits, &colorEdits);
    clearSelection();
    recomputeMeshes();
    updateMeshForCurrentRenderer(false);
    if (isDiffComputed())
        updateMeshLimitRenderer();
}

void MainView::clearSelection() {
    selectedVertex = -1;
    selectedEdges.clear();
    affectedVertex.clear();
}

int MainView::getSubdivSteps() {
    return mainWindow->ui->SubdivSteps->value();
}

int MainView::getEditSteps() {
    return mainWindow->ui->EditSteps->value();
}

int MainView::getLimitSubdivSteps() {
    return mainWindow->ui->DiffSubdivSteps->value();
}

bool MainView::isDiffComputed() {
    return mainWindow->ui->DisplayDifference->checkState();
}

bool MainView::isEditingEnabled() {
    return mainWindow->ui->EnableEditing->checkState();
}

float MainView::getBrushRadius() {
    return mainWindow->ui->BrushRadiusSpinBox->value();
}

QVector3D MainView::getBrushColor() {
    QColor color = mainWindow->ui->EditColorButton->palette().color(QPalette::Button);
    return QVector3D(color.red(), color.green(), color.blue()) / 255;
}

bool MainView::isModelLoaded() {
    return inputMesh.Vertices.size() > 0;
}

int MainView::getMaxComputedSubdivLevel() {
    return originalMeshes.size() - 1;
}

//update = 0, set mesh
//update = 1, update coords
//update = 2, update color
void MainView::updateMeshForCurrentRenderer(int update) {
    if (!isModelLoaded())
        return;

    // Set mesh for current renderer
    QString renderer = QString(mainWindow->ui->Renderer->currentText());
    if (renderer == "Default"){
        QElapsedTimer timer;
        timer.start();
        if (update == 0)
            renderers[renderer]->setMesh(limitMeshes[getSubdivSteps()]);
        else if(update == 1) //only update coords for changde points
            renderers[renderer]->updateMeshCoords(limitMeshes[getSubdivSteps()], changedLimitCoordsIndices);
        else //only update colors for changed points
            renderers[renderer]->updateMeshColors(limitMeshes[getSubdivSteps()], changedEdgesIndices);
        //        qDebug() << "Deafault Time elapsed:" << timer.elapsed() << "milliseconds";
    }
    else if (renderer == "ACC1" || renderer == "ACC2" || renderer == "GG") {
        QElapsedTimer timer;
        timer.start();
        if (update == 0)
            renderers[renderer]->setMesh(editedMeshes[getSubdivSteps()]);
        else if(update == 1) //only update coords for changde points
            renderers[renderer]->updateMeshCoords(editedMeshes[getSubdivSteps()], changedFacesIndices);
        else //only update colors for changed points
            renderers[renderer]->updateMeshColors(editedMeshes[getSubdivSteps()], changedFacesIndices);
        //        qDebug() << "ACC Time elapsed:" << timer.elapsed() << "milliseconds";
    }
    else if (renderer == "Feature Adaptive") {
        QElapsedTimer timer;
        timer.start();
        ((FeatureAdaptiveRenderer *) renderers[renderer])->setMesh(originalMeshes[0], coordsEdits, colorEdits);
        //        qDebug() << "Feature Adaptive Time elapsed:" << timer.elapsed() << "milliseconds";
    }
}

void MainView::updateMeshLimitRenderer() {
    renderers["Limit"]->setMesh(limitMeshes[getLimitSubdivSteps()]);
}

void MainView::recomputeMeshes() {
    // Clean
    originalMeshes.clear();
    editedMeshes.clear();
    limitMeshes.clear();
    editableVertexIndices.clear();
    gradientVertexIndices.clear();
    changedLimitCoordsIndices.clear();
    changedEdgesIndices.clear();
    changedFacesIndices.clear();

    // Initialize with ternary subdivision
    originalMeshes.append(Mesh());
    subdivideTernaryStep(&inputMesh, &originalMeshes[0]);
    editedMeshes.append(computeEditedMesh(originalMeshes[0], coordsEdits[0], colorEdits[0]));
    limitMeshes.append(computeLimitMesh(editedMeshes[0]));
    editableVertexIndices.append(getEditableVertexIndices(inputMesh.Vertices.size(), editedMeshes[0]));
    gradientVertexIndices.append(getGradientVertexIndices(inputMesh.Vertices.size(), editedMeshes[0]));

    // Add Catmull-Clark subdivision steps
    int requiredSubdivSteps = isDiffComputed() ? qMax(getSubdivSteps(), getLimitSubdivSteps()) : getSubdivSteps();
    for (int i = 1; i <= requiredSubdivSteps; ++i) {
        originalMeshes.append(Mesh());
        subdivideCatmullClark(&editedMeshes[i-1], &originalMeshes[i]);
        editedMeshes.append(computeEditedMesh(originalMeshes[i], coordsEdits[i], colorEdits[i]));
        limitMeshes.append(computeLimitMesh(editedMeshes[i]));
        editableVertexIndices.append(getEditableVertexIndices(inputMesh.Vertices.size(), editedMeshes[i]));
        gradientVertexIndices.append(getGradientVertexIndices(inputMesh.Vertices.size(), editedMeshes[i]));
    }
}

//only do needed subdivision instead of recomputing everything, save memory
void MainView::subdivide() {
    int requiredSubdivSteps = isDiffComputed() ? qMax(getSubdivSteps(), getLimitSubdivSteps()) : getSubdivSteps();
    for (int i = getMaxComputedSubdivLevel() + 1; i <= requiredSubdivSteps; ++i) {
        originalMeshes.append(Mesh());
        subdivideCatmullClark(&editedMeshes[i-1], &originalMeshes[i]);
        editedMeshes.append(computeEditedMesh(originalMeshes[i], coordsEdits[i], colorEdits[i]));
        limitMeshes.append(computeLimitMesh(editedMeshes[i]));
        editableVertexIndices.append(getEditableVertexIndices(inputMesh.Vertices.size(), editedMeshes[i]));
        gradientVertexIndices.append(getGradientVertexIndices(inputMesh.Vertices.size(), editedMeshes[i]));
    }
}

//update coords when a point is moved instead of recomputing everything
void MainView::updateCoords(int editFlag) {
    int curEditStep = getEditSteps();
    int curStep = getSubdivSteps();
    int maxComputedSubdivLevel = getMaxComputedSubdivLevel();

    //update coords of editedmesh & limitmesh at current edit step
    (curStep == curEditStep) ?
                updateEditedCoords(originalMeshes[curEditStep], editedMeshes[curEditStep], coordsEdits[curEditStep][selectedVertex], selectedVertex, editFlag, changedFacesIndices, 0, true) :
                updateEditedCoords(originalMeshes[curEditStep], editedMeshes[curEditStep], coordsEdits[curEditStep][selectedVertex], selectedVertex, editFlag, changedFacesIndices, 0, false);
    (curStep == curEditStep) ?
                updateLimitCoords(editedMeshes[curEditStep], limitMeshes[curEditStep], selectedVertex, changedLimitCoordsIndices, 0, true) :
                updateLimitCoords(editedMeshes[curEditStep], limitMeshes[curEditStep], selectedVertex, changedLimitCoordsIndices, 0, false);

    //update coords of originalmes & editedmesh & limitmesh from the next step of current edit step to max computed subdivision level
    int level = 1;
    for (int i = curEditStep + 1; i <= maxComputedSubdivLevel; i++) {
        updateOriginalCoords(&originalMeshes[i], &editedMeshes[i-1], selectedVertex, level);
        (curStep == i) ?
                    updateEditedCoords(originalMeshes[i], editedMeshes[i], coordsEdits[curEditStep][selectedVertex], selectedVertex, editFlag, changedFacesIndices, level, true) :
                    updateEditedCoords(originalMeshes[i], editedMeshes[i], coordsEdits[curEditStep][selectedVertex], selectedVertex, editFlag, changedFacesIndices, level, false);
        (curStep == i) ?
                    updateLimitCoords(editedMeshes[i], limitMeshes[i], selectedVertex, changedLimitCoordsIndices, level++, true) :
                    updateLimitCoords(editedMeshes[i], limitMeshes[i], selectedVertex, changedLimitCoordsIndices, level++, false);
    }
}

//update color when a color is modified instead of recomputing everything
void MainView::updateColor() {
    int curEditStep = getEditSteps();
    int curStep = getSubdivSteps();
    int maxComputedSubdivLevel = getMaxComputedSubdivLevel();

    //update color of editedmesh & limitmesh at current edit step
    QHash<int, QSet<Face *>> affectedEditedfaces = getColorAffectedFaces(selectedEdges, editedMeshes[curEditStep], 0);
    (curStep == curEditStep) ?
                updateEditedColor(originalMeshes[curEditStep], editedMeshes[curEditStep], colorEdits[curEditStep], selectedEdges, 0, affectedEditedfaces, true, changedFacesIndices) :
                updateEditedColor(originalMeshes[curEditStep], editedMeshes[curEditStep], colorEdits[curEditStep], selectedEdges, 0, affectedEditedfaces, false, changedFacesIndices);
    QHash<int, QSet<Face *>> affectedfaces = getColorAffectedFaces(selectedEdges, limitMeshes[curEditStep], 0);
    (curStep == curEditStep) ?
                updateLimitMeshColor(editedMeshes[curEditStep], limitMeshes[curEditStep], colorEdits[curEditStep], selectedEdges, changedEdgesIndices, 0, affectedfaces, true) :
                updateLimitMeshColor(editedMeshes[curEditStep], limitMeshes[curEditStep], colorEdits[curEditStep], selectedEdges, changedEdgesIndices, 0, affectedfaces, false);

    //update color of originalmes & editedmesh & limitmesh from the next step of current edit step to max computed subdivision level
    int level = 1;
    for (int i = curEditStep + 1; i <= maxComputedSubdivLevel; i++) {
        QHash<int, QSet<Face *>> affectedLimitedFaces = getColorAffectedFaces(selectedEdges, limitMeshes[i], level);
        QHash<int, QSet<Face *>> editedFaces = getColorAffectedFaces(selectedEdges, editedMeshes[i-1], level-1);
        QHash<int, QSet<Face *>> editedFaces2 = getColorAffectedFaces(selectedEdges, editedMeshes[i], level);

        updateOriginalColor(&originalMeshes[i], &editedMeshes[i-1], selectedEdges, level, editedFaces);
        (curStep == i) ?
                    updateEditedColor(originalMeshes[i], editedMeshes[i], colorEdits[curEditStep], selectedEdges, level, editedFaces2, true, changedFacesIndices) :
                    updateEditedColor(originalMeshes[i], editedMeshes[i], colorEdits[curEditStep], selectedEdges, level, editedFaces2, false, changedFacesIndices);
        (curStep == i) ?
                    updateLimitMeshColor(editedMeshes[i], limitMeshes[i], colorEdits[curEditStep], selectedEdges, changedEdgesIndices, level ++, affectedLimitedFaces, true) :
                    updateLimitMeshColor(editedMeshes[i], limitMeshes[i], colorEdits[curEditStep], selectedEdges, changedEdgesIndices, level ++, affectedLimitedFaces, false);
    }
}

void MainView::initializeGL() {
    // Initialize OpenGL
    initializeOpenGLFunctions();
    qDebug() << ":: OpenGL initialized (" << (const char *) glGetString(GL_VERSION) << ")";

    // Initialize Logger
    debugLogger = new QOpenGLDebugLogger();
    connect(debugLogger, SIGNAL(messageLogged(QOpenGLDebugMessage)), this, SLOT(onMessageLogged(QOpenGLDebugMessage)), Qt::DirectConnection);
    if (debugLogger->initialize()) {
        qDebug() << ":: Logging initialized";
        debugLogger->startLogging(QOpenGLDebugLogger::SynchronousLogging);
        debugLogger->enableMessages();
        debugLogger->disableMessages(QVector<GLuint>({131185, 131169, 131204}));
    }

    // Initialize renderers
    QOpenGLFunctions_4_1_Core *functions = (QOpenGLFunctions_4_1_Core *) this->context()->versionFunctions();
    renderers["Default"] = new DefaultRenderer(functions);
    renderers["ACC1"] = new ACC1Renderer(functions);
    renderers["ACC2"] = new ACC2Renderer(functions);
    renderers["GG"] = new GGRenderer(functions);
    renderers["Feature Adaptive"] = new FeatureAdaptiveRenderer(functions);
    renderers["Limit"] = new DefaultRenderer(functions);
    renderers["Limit"]->setComputeDiff(0);
    pointRenderer = new PointRenderer(functions);
    lineRenderer = new LineRenderer(functions);

    // Initialize renderers
    updateShowWireframe();
    updateScaling();
    updateDisplacement();
    updateShowWireframe();
    updateColorBands();
    updateTessLevel();
    updateDiffScaling();

    // Initialize framebuffers
    QOpenGLFramebufferObjectFormat format;
    format.setInternalTextureFormat(GL_RGBA32F);
    limitFramebuffer = new QOpenGLFramebufferObject(this->width(), this->height(), format);
    diffFramebuffer = new QOpenGLFramebufferObject(this->width(), this->height(), format);
}

void MainView::resizeGL(int width, int height) {
    qDebug() << ".. resizeGL";

    // Delete framebuffers
    delete limitFramebuffer;
    delete diffFramebuffer;

    // Initialize framebuffers
    QOpenGLFramebufferObjectFormat format;
    format.setInternalTextureFormat(GL_RGBA32F);
    limitFramebuffer = new QOpenGLFramebufferObject(width, height, format);
    diffFramebuffer = new QOpenGLFramebufferObject(width, height, format);

    updateScaling();
    update();
}

void MainView::paintGL() {
    if (!isModelLoaded()){
        return;
    }

    QString currentRenderer(mainWindow->ui->Renderer->currentText());

    if (isDiffComputed()) {
        // Render limit surface to framebuffer
        limitFramebuffer->bind();

        //On a Mac with a retina display, the frame buffer is larger than the window by 2x in each dimension.
        //I must adjust the size of the viewport to the size of the bound frame buffer before I’m about to draw something into a frame buffer.
        glViewport(0, 0, this->width(), this->height()); //you don't need this if you don't use MAC.
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderers["Limit"]->render();
        limitFramebuffer->release();

        // Set limit framebuffer as current texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, limitFramebuffer->texture());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);

        // Save texture as an image
        QImage image0 = limitFramebuffer->toImage();
        QDateTime d = QDateTime::currentDateTime();
        QString date = d.toString("dd.MM.yyyy.hh:mm:ss.zzz") + ".png";
        QString fileName =  "./../../../../MeshTool/examples/texture/" + date;
        //                image0.save(fileName, "PNG", 100);


        // Render difference to framebuffer
        diffFramebuffer->bind();

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderers[currentRenderer]->setComputeDiff(2);
        renderers[currentRenderer]->render();
        diffFramebuffer->release();

        // Update maximum difference value in interface
        QImage image = diffFramebuffer->toImage();
        d = QDateTime::currentDateTime();
        date = d.toString("dd.MM.yyyy.hh:mm:ss.zzzDDD") + ".png";
        fileName =  "./../../../../MeshTool/examples/texture/" + date;
        //                image.save(fileName, "PNG", 100);

        double maxDiff = 0;
        for (int i = 0; i < image.height(); ++i) {
            for (int j = 0; j < image.width(); ++j) {
                QColor color = image.pixelColor(j, i);
                maxDiff = qMax(maxDiff, color.red() / 255.0 + color.green() / (255.0 * 255.0) + color.blue() / (255.0 * 255.0 * 255.0));
            }
        }
        maxDiff /= sqrt(3);
        mainWindow->setMaxVisibleDiffLabel(maxDiff);
        qDebug() << "maxDiff=" << QString::number(maxDiff);

    }
    glViewport(0, 0, this->width()*2, this->height()*2); //you don't need this if you don't use MAC.

    // Clear
    //Black background
    glClearColor(0, 0, 0, 1);
    // White background
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set number of control points
    QHash<QString, int> countInfo = renderers[currentRenderer]->getCountInfo();
    QString label = "";
    QList<QString> units = countInfo.keys();
    units.sort();
    foreach (QString unit, units) {
        label += QString::number(countInfo[unit]) + " " + unit + "\n";
    }
    mainWindow->setInfoLabel(label);

    // Render using current renderer
    renderers[currentRenderer]->setComputeDiff((int) isDiffComputed());
    renderers[currentRenderer]->render();


    lineRenderer->setScaling(getScaleVector());
    lineRenderer->setDisplacement(displacement);

    // Render affected area by coordinate edit
    // green
    if (selectedVertex != -1) {
        lineRenderer->setColor(QVector3D(0, 1, 0));
        Vertex *v = &limitMeshes[getEditSteps()].Vertices[selectedVertex];
        lineRenderer->setEdges(getBoundaryEdges(getPadded(QSet<Vertex *>({v}), 2)));
        lineRenderer->render();
    }

    // Render affected area by color edit
    // blue
    if (selectedEdges.size() > 0) {
        lineRenderer->setColor(QVector3D(0, 0, 1));
        QSet<Face *> affectedFaces;
        foreach (int edgeIndex, selectedEdges) {
            HalfEdge *e = &limitMeshes[getEditSteps()].HalfEdges[edgeIndex];
            affectedFaces += computeColorEditAffectedFaces(e);
        }
        lineRenderer->setEdges(getBoundaryEdges(affectedFaces));
        lineRenderer->render();
    }

    // Render points
    renderPoints();

    if (isDiffComputed())
        glBindTexture(GL_TEXTURE_2D, 0);
}

QVector2D MainView::computeColorEditPointCoords(HalfEdge *e) {
    QVector2D vCoords = e->prev->target->coords;
    QVector2D v1Coords = e->target->coords;
    QVector2D v2Coords = e->prev->prev->target->coords;
    QVector2D direction = (v1Coords - vCoords).normalized() + (v2Coords - vCoords).normalized();
    return vCoords + 0.03 / getScaleVector().length() * direction;
}

void MainView::renderPoints() {
    if (!isEditingEnabled())
        return;

    // Initialize
    QVector<QVector2D> vertexCoords;
    for (int i = 0; i < limitMeshes[getEditSteps()].Vertices.size(); ++i)
        vertexCoords.append(limitMeshes[getEditSteps()].Vertices[i].coords);
    pointRenderer->setVertexCoords(vertexCoords);
    pointRenderer->setRadius(.01 / getScaleVector().length());

    // Draw editable vertices
    pointRenderer->setVertexIndices(editableVertexIndices[getEditSteps()]);
    pointRenderer->setColor(QVector3D(0, 0, 1)); //blue points
    pointRenderer->setFilled(true);
    pointRenderer->render();
    pointRenderer->setColor(QVector3D(0, 0, 0));
    pointRenderer->setFilled(false);
    pointRenderer->render();

    // Draw gradient vertices
    pointRenderer->setVertexIndices(gradientVertexIndices[getEditSteps()]);
    pointRenderer->setColor(QVector3D(0, 1, 0)); //green points
    pointRenderer->setFilled(true);
    pointRenderer->render();
    pointRenderer->setColor(QVector3D(0, 0, 0));
    pointRenderer->setFilled(false);
    pointRenderer->render();

    // Draw selected point
    if (selectedVertex != -1) {
        pointRenderer->setVertexIndices(QVector<int>({selectedVertex}));
        pointRenderer->setColor(QVector3D(1, 0, 0)); // red
        pointRenderer->setFilled(true);
        pointRenderer->render();
        pointRenderer->setColor(QVector3D(0, 0, 0));
        pointRenderer->setFilled(false);
        pointRenderer->render();
    }

    // Draw color edit points
    QVector<QVector2D> sectorPoints;
    QVector<int> sectorIndicesUnselected, sectorIndicesSelected;
    QSet<HalfEdge *> ff;
    int idx = 0;
    foreach (int vertexIndex, editableVertexIndices[getEditSteps()]) {
        Vertex v = limitMeshes[getEditSteps()].Vertices[vertexIndex];
        foreach (HalfEdge *e, getVertexEdges(v.out)) {
            if (!e->polygon)
                continue;
            QVector2D point = computeColorEditPointCoords(e);
            sectorPoints << point;
            if (selectedEdges.contains(e->index)) {
                ff << &limitMeshes[getEditSteps()].HalfEdges[idx];
                sectorIndicesSelected << idx++;
            }
            else
                sectorIndicesUnselected << idx++;
        }
    }
    pointRenderer->setVertexCoords(sectorPoints);
    pointRenderer->setRadius(.0075 / getScaleVector().length());
    pointRenderer->setVertexIndices(sectorIndicesUnselected);
    pointRenderer->setColor(QVector3D(0, 1, 1)); // sky blue
    pointRenderer->setFilled(true);
    pointRenderer->render();
    pointRenderer->setColor(QVector3D(0, 0, 0));
    pointRenderer->setFilled(false);
    pointRenderer->render();
    pointRenderer->setVertexIndices(sectorIndicesSelected);
    pointRenderer->setColor(QVector3D(1, 0, 0));
    pointRenderer->setFilled(true);
    pointRenderer->render();
    pointRenderer->setColor(QVector3D(0, 0, 0));
    pointRenderer->setFilled(false);
    pointRenderer->render();

    // Draw brush
    pointRenderer->setVertexCoords(QVector<QVector2D>({getWorldCoords(lastEventPos)}));
    pointRenderer->setVertexIndices(QVector<int>({0}));
    pointRenderer->setRadius(getBrushRadius() / getScaleVector().length());
    pointRenderer->setColor(QVector3D(.0, .0, .0)); //.5 .5 .5 for grey
    pointRenderer->setFilled(false);
    pointRenderer->render();
}

// ---

QVector2D MainView::getScaleVector() {
    return this->width() < this->height() ? QVector2D(scale, scale * this->width() / this->height()) : QVector2D(scale * this->height() / this->width(), scale);
}

QVector2D MainView::getWorldCoords(QPoint point) {
    QVector2D scale_vector = getScaleVector();
    QVector2D clip_coords(-1.0f + 2.0f * point.x() / this->width(), 1.0f - 2.0f * point.y() / this->height());
    QVector2D world_coords = clip_coords / scale_vector - displacement;
    return world_coords;
}

void MainView::setSelected(QPoint point) {

    if (!(isModelLoaded() && isEditingEnabled()))
        return;

    QVector2D worldCoordsEvent = getWorldCoords(point);

    QVector<int> allEditable;
    allEditable.append(editableVertexIndices[getEditSteps()]);
    allEditable.append(gradientVertexIndices[getEditSteps()]);

    // Set selected vertex
    int nearestVertexIndex = -1;
    float nearestVertexDistance = std::numeric_limits<float>::max();
    foreach (int vertexIndex, allEditable) {
        float screenDistance = getScaleVector().length() * (limitMeshes[getEditSteps()].Vertices[vertexIndex].coords - worldCoordsEvent).length();
        if (screenDistance < getBrushRadius()) {
            if (screenDistance < nearestVertexDistance) {
                nearestVertexIndex = vertexIndex;
                nearestVertexDistance = screenDistance;
            }
        }
    }
    selectedVertex = nearestVertexIndex;

    // Set selected edges and vertex
    selectedEdges.clear();
    affectedVertex.clear();
    foreach (int vertexIndex, editableVertexIndices[getEditSteps()]) {
        Vertex v = limitMeshes[getEditSteps()].Vertices[vertexIndex];
        foreach (HalfEdge *e, getVertexEdges(v.out)) {
            if (!e->polygon)
                continue;
            QVector2D colorEditPointWorldCoords = computeColorEditPointCoords(e);
            float screenDistance = getScaleVector().length() * (colorEditPointWorldCoords - worldCoordsEvent).length();
            if (screenDistance < getBrushRadius()){
                selectedEdges << e->index;}
            affectedVertex << e->target->index;

        }
    }
}

void MainView::mousePressEvent(QMouseEvent *event) {
    lastEventPos = event->pos();
    setFocus();
    if (event->buttons() & Qt::RightButton && selectedEdges.size() > 0 && isEditingEnabled())
        editColor();
    update();
}

CoordsEdit MainView::computeCoordsEdit(Vertex v, QVector2D deltaCoords) {
    HalfEdge *e1 = v.out;
    for (int i = 0; i < v.val; ++i) {
        HalfEdge *e2 = e1->prev->twin;
        QVector2D vec1 = e1->target->coords - v.coords;
        QVector2D vec2 = e2->target->coords - v.coords;
        // Compute angle between e1 and e2
        float alpha = atan2(vec2.y(), vec2.x()) - atan2(vec1.y(), vec1.x());
        if (alpha < 0)
            alpha += 2 * M_PI;
        // Compute angle between e1 and displacement
        float phi = atan2(deltaCoords.y(), deltaCoords.x()) - atan2(vec1.y(), vec1.x());
        if (phi < 0)
            phi += 2 * M_PI;
        // Continue if wrong sector
        if (phi > alpha) {
            e1 = e1->prev->twin;
            continue;
        }
        // Check if boundary or interior point
        CoordsEdit ce;
        if (e1->polygon) {
            // Let WolframAlpha solve 'solve x = a x_1 + b x_2, y = a y_1 + b y_2 for a and b'
            float a = -(deltaCoords.x() * vec2.y() - vec2.x() * deltaCoords.y()) / (vec2.x() * vec1.y() - vec1.x() * vec2.y());
            float b = -(vec1.x() * deltaCoords.y() - deltaCoords.x() * vec1.y()) / (vec2.x() * vec1.y() - vec1.x() * vec2.y());
            ce.edgeIndex = e1->index;
            ce.val1 = a;
            ce.val2 = b;
            ce.boundary = false;
        } else {
            ce.edgeIndex = e1->twin->index;
            ce.val1 = phi / alpha;
            ce.val2 = deltaCoords.lengthSquared() / sqrt(vec1.lengthSquared() * vec2.lengthSquared());
            ce.boundary = true;
        }
        // Update affected edge indices
        QSet<Face *> twoRingFaces = getPadded(QSet<Vertex *>({&v}), 2);
        foreach (Face *f, twoRingFaces)
            ce.affectedEdgeIndices << f->side->index;
        //        for (int i = 0; i < changedFacesIndices.size(); i ++)
        //            ce.affectedEdgeIndices << originalMeshes[getEditSteps()].Faces[i].side->index;

        // Return
        return ce;
    }
    return CoordsEdit();
}

void MainView::mouseMoveEvent(QMouseEvent *event) {
    if (QApplication::keyboardModifiers() & Qt::ControlModifier && event->buttons() & (Qt::LeftButton | Qt::RightButton)) { // Editing
        if (event->buttons() & Qt::LeftButton) { // Coordinate editing
            if (selectedVertex != -1 && isEditingEnabled()){
                editCoords(event->pos());
            }
        } else if (event->buttons() & Qt::RightButton) { // Color editing
            setSelected(event->pos());
            if (!selectedEdges.isEmpty() && isEditingEnabled())
                editColor();
        }
    } else if (event->buttons() & Qt::LeftButton) { // Panning
        QVector2D lastEventWorldCoords = getWorldCoords(lastEventPos);
        QVector2D eventWorldCoords = getWorldCoords(event->pos());
        displacement += eventWorldCoords - lastEventWorldCoords;
        updateDisplacement();
    } else { // Hovering;
        setSelected(event->pos());
    }

    // Update
    lastEventPos = event->pos();
    update();
}

void MainView::wheelEvent(QWheelEvent* event) {
    if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
        //float brushRadius = qMax(0.0, getBrushRadius() - 0.0001 * event->delta());
//        mainWindow->ui->EditSteps->setValue(mainWindow->ui->EditSteps->value() + (event->delta() < 0 ? -1 : 1));
    } else {
        QVector2D eventWorldCoordsBefore = getWorldCoords(event->pos());
        scale *= static_cast<float>(qPow(1.001, event->delta()));
        QVector2D eventWorldCoordsAfter = getWorldCoords(event->pos());

        // Match event world coords before and after zooming (effectively zooming at event position)
        displacement += eventWorldCoordsAfter - eventWorldCoordsBefore;
        updateScaling();
        updateDisplacement();
    }
    setSelected(event->pos());
    update();
}

void MainView::keyPressEvent(QKeyEvent* event) {
    if (event->key() == 'Z') {
        mainWindow->ui->ShowWireframe->setChecked(!mainWindow->ui->ShowWireframe->checkState());
        update();
    }
    if (event->key() == 'S') {
        mainWindow->ui->screenShot->clicked(true);
        //        update();
    }
}

void MainView::editCoords(QPoint eventPos) {
    changedLimitCoordsIndices.clear();
    changedFacesIndices.clear();
    if (QApplication::keyboardModifiers() & Qt::ShiftModifier) { // Delete
        // Prevent multiple delete
        if (coordsEdits[getEditSteps()][selectedVertex].edgeIndex > 0) {
            // Update coords only
            updateCoords(0);
            coordsEdits[getEditSteps()].remove(selectedVertex);
        }
        else {
            return;
        }
    } else { // Edit
        // Compute delta in limit coordinate space
        QVector2D deltaLimitCoords = getWorldCoords(eventPos) - getWorldCoords(lastEventPos);

        // Compute destination in limit coordinate space
        QVector2D destinationLimitCoords = limitMeshes[getEditSteps()].Vertices[selectedVertex].coords + deltaLimitCoords;

        // Compute edited coords that result in desired limit coordinates for the selected vertex
        Vertex *editedVertex = &editedMeshes[getEditSteps()].Vertices[selectedVertex];
        QVector2D originalCoordsEvent = computeInvertedLimitPointCoords(editedVertex, destinationLimitCoords);

        // Check if edit is allowed
        editedVertex->coords = originalCoordsEvent; // Note: meshes need to be updated after this
//        if (!isSelfIntersecting(&editedMeshes[getEditSteps()], selectedVertex)) {
            // Compute coordinate edit
            Vertex originalVertex = originalMeshes[getEditSteps()].Vertices[selectedVertex];
            QVector2D originalCoordsDelta = originalCoordsEvent - originalVertex.coords;
            CoordsEdit *ce = &coordsEdits[getEditSteps()][selectedVertex];
            *ce = computeCoordsEdit(originalVertex, originalCoordsDelta);
            updateCoords(1);
//        }
        // Update coords only
        //        updateCoords(1);
        //        recomputeMeshes();
    }

    QElapsedTimer t;
    t.start();
    updateMeshForCurrentRenderer(1);
    //    qDebug() << "update mesh:" << t.elapsed() << "ms";
    if (isDiffComputed())
        updateMeshLimitRenderer();
}

void MainView::editColor() {
    changedFacesIndices.clear();
    changedEdgesIndices.clear();
    // Return conditions
    if (!(isModelLoaded() && isEditingEnabled() && selectedEdges.size() > 0))
        return;

    // Set color for selected edges
    foreach (int edgeIndex, selectedEdges) {
        ColorEdit *e = &colorEdits[getEditSteps()][edgeIndex];
        e->edgeIndex = edgeIndex;
        if (QApplication::keyboardModifiers() & Qt::ShiftModifier) {
            //            colorEdits[getEditSteps()].remove(edgeIndex);
            //            ColorEdit *e = &colorEdits[getEditSteps()][edgeIndex];
            //            e->edgeIndex = edgeIndex;
            //            e->color = getBrushColor();
            colorEdits[getEditSteps()][edgeIndex].color = QVector3D(1,1,1);

        } else {
            e->color = getBrushColor();
        }
        // Update affected edge indices
        Vertex *v = originalMeshes[getEditSteps()].HalfEdges[e->edgeIndex].twin->target;
        QSet<Face *> threeRingFaces = getPadded(QSet<Vertex *>({v}), 3);
        e->affectedEdgeIndices.clear();
        foreach (Face *f, threeRingFaces)
            e->affectedEdgeIndices << f->side->index;

    }

    // Update color only
    //    recomputeMeshes();
    updateColor();
    updateMeshForCurrentRenderer(2);
    if (isDiffComputed())
        updateMeshLimitRenderer();
}

void MainView::setMainWindow(MainWindow *mainWindow) {
    this->mainWindow = mainWindow;
}

void MainView::updateShowWireframe() {
    foreach (SurfaceRenderer *renderer, renderers)
        renderer->setShowWireframe(mainWindow->ui->ShowWireframe->checkState());
}

void MainView::updateScaling() {
    foreach (SurfaceRenderer *renderer, renderers)
        renderer->setScaling(getScaleVector());
    pointRenderer->setScaling(getScaleVector());
}

void MainView::updateDisplacement() {
    foreach (SurfaceRenderer *renderer, renderers)
        renderer->setDisplacement(displacement);
    pointRenderer->setDisplacement(displacement);
}

void MainView::updateColorBands() {
    foreach (SurfaceRenderer *renderer, renderers)
        renderer->setColorBands(mainWindow->ui->ColorBands->value());
}

void MainView::updateTessLevel() {
    foreach (SurfaceRenderer *renderer, renderers)
        renderer->setTessLevel(mainWindow->ui->TessellationLevel->value());
}

void MainView::updateDiffScaling() {
    foreach (SurfaceRenderer *renderer, renderers)
        renderer->setDiffScaling(mainWindow->ui->DiffScale->value());
}

// ---

void MainView::onMessageLogged( QOpenGLDebugMessage Message ) {
    qDebug() << " → Log:" << Message;
}

void MainView::updateScale(float s) {
    scale = s;
    //    QVector2D eventWorldCoordsBefore = getWorldCoords(event->pos());
    //    scale *= static_cast<float>(qPow(1.001, event->delta()));
    //    QVector2D eventWorldCoordsAfter = getWorldCoords(event->pos());

    //    // Match event world coords before and after zooming (effectively zooming at event position)
    //    displacement += eventWorldCoordsAfter - eventWorldCoordsBefore;

    updateScaling();
    updateDisplacement();

    //setSelected(event->pos());
    update();
}
