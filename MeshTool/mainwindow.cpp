#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "persistence.h"
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent) :  QMainWindow(parent), ui(new Ui::MainWindow) {
  qDebug() << "✓✓ MainWindow constructor";
  ui->setupUi(this);
  ui->Renderer->addItem("Default");
  ui->Renderer->addItem("ACC1");
  ui->Renderer->addItem("ACC2");
  ui->Renderer->addItem("GG");
  ui->Renderer->addItem("Feature Adaptive");
  ui->ColormapPicture->setPixmap(QPixmap("./../MeshTool/images/plasma.png"));
  setButtonColor(ui->EditColorButton, QColor(Qt::white));
  setColormapMaxLabel(ui->DiffScale->value());
  setMaxVisibleDiffLabel(0);
  setInfoLabel("");
  ui->MainDisplay->setMainWindow(this);
  transparent = false;

}

MainWindow::~MainWindow() {
  qDebug() << "✗✗ MainWindow destructor";
  delete ui;
}

void MainWindow::importOBJ(QString filename) {
  ui->MainDisplay->setMesh(filename);
}

void MainWindow::on_Load_clicked() {
  QString fileName = QFileDialog::getOpenFileName(this, "Load File", MODEL_PATH, tr("Obj Files (*.obj)"));
  if (fileName.isNull())
    return;

  importOBJ(fileName);
  ui->MainDisplay->update();
}

void MainWindow::on_Save_clicked() {
  if (!ui->MainDisplay->isModelLoaded())
    return;
  QString fileName = QFileDialog::getSaveFileName(this, "Save File", MODEL_PATH, tr("Obj Files (*.obj)"));
  if (fileName.isNull())
    return;
  save(fileName, ui->MainDisplay->inputMesh, ui->MainDisplay->coordsEdits, ui->MainDisplay->colorEdits);
}

void MainWindow::on_loadTriangleButton_clicked() {
  importOBJ(TRAINGLE_PATH);
  ui->MainDisplay->update();
}

void MainWindow::on_loadQuadButton_clicked() {
  importOBJ(QUAD_PATH);
  ui->MainDisplay->update();
}

void MainWindow::on_loadPentagonButton_clicked() {
  importOBJ(PENTAGON_PATH);
  ui->MainDisplay->update();
}

void MainWindow::on_loadHexagonButton_clicked() {
  importOBJ(HEXAGON_PATH);
  ui->MainDisplay->update();
}

void MainWindow::on_SubdivSteps_valueChanged(int value) {
    if (!ui->MainDisplay->isModelLoaded()){
        return;
    }
  // Delegate signal
  if (value < ui->EditSteps->value()) {
    ui->MainDisplay->updateMeshForCurrentRenderer(false);
    ui->EditSteps->setValue(value);
    return;
  }

  // Check if meshes have to be updated
  if (value > ui->MainDisplay->getMaxComputedSubdivLevel()) {
      //ui->MainDisplay->recomputeMeshes();
      //only do needed subdivision instead of recomputing everything, save memory
      ui->MainDisplay->subdivide();
  }

  // Update
  ui->MainDisplay->updateMeshForCurrentRenderer(false);
  ui->MainDisplay->update();
}

void MainWindow::on_EditSteps_valueChanged(int value) {
  // Clear selection
  ui->MainDisplay->clearSelection();

  // Delegate signal
  if (value > ui->SubdivSteps->value()) {
    ui->SubdivSteps->setValue(value);
    return;
  }

  // Update
  ui->MainDisplay->update();
}

//brush size
void MainWindow::on_BrushRadiusSpinBox_valueChanged(double value) {
  ui->MainDisplay->update();
}

void MainWindow::on_EnableEditing_toggled(bool checked) {
  ui->MainDisplay->update();
}

void MainWindow::on_ShowWireframe_toggled(bool checked) {
  ui->MainDisplay->updateShowWireframe();
  ui->MainDisplay->update();
}

void MainWindow::on_Renderer_currentIndexChanged(int index) {
  ui->MainDisplay->updateMeshForCurrentRenderer(false);
  ui->MainDisplay->update();
}

void MainWindow::on_TessellationLevel_valueChanged(int value) {
  ui->MainDisplay->updateTessLevel();
  ui->MainDisplay->update();
}

void MainWindow::setButtonColor(QPushButton *button, QColor color) {
  QPalette pal = button->palette();
  pal.setColor(QPalette::Button, color);
  button->setAutoFillBackground(true);
  button->setFlat(true);
  button->setPalette(pal);
  button->update();
}

void MainWindow::on_EditColorButton_clicked() {
  QColor color = QColorDialog::getColor(QColor(255, 255, 255), 0);
  if (color.isValid())
    setButtonColor(ui->EditColorButton, color);
}

void MainWindow::on_ColorBands_valueChanged(int value) {
  ui->MainDisplay->updateColorBands();
  ui->MainDisplay->update();
}

void MainWindow::on_DisplayDifference_toggled(bool checked) {
    if (!ui->MainDisplay->isModelLoaded()){
        return;
    }
  if (!checked)
    setMaxVisibleDiffLabel(0);
  else if (ui->DiffSubdivSteps->value() > ui->MainDisplay->getMaxComputedSubdivLevel())
      ui->MainDisplay->subdivide();
//    ui->MainDisplay->recomputeMeshes();

  ui->MainDisplay->updateMeshLimitRenderer();
  ui->MainDisplay->update();
}

void MainWindow::on_DiffSubdivSteps_valueChanged(int value) {
  if (!ui->DisplayDifference->checkState())
    return;
  if (value > ui->MainDisplay->getMaxComputedSubdivLevel())
      ui->MainDisplay->subdivide();
//    ui->MainDisplay->recomputeMeshes();
  ui->MainDisplay->updateMeshLimitRenderer();
  ui->MainDisplay->update();
}

void MainWindow::on_DiffScale_valueChanged(int value) {
  ui->MainDisplay->updateDiffScaling();
  setColormapMaxLabel(value);
  ui->MainDisplay->update();
}

void MainWindow::setColormapMaxLabel(int diffScale) {
  ui->ColormapMaxLabel->setText("<html><head/><body>Colormap ( 0 &le; difference &le; " + QString::number(diffScale == 0 ? 0 : 1.0 / diffScale, 'e', 2) + " )</body></html>");
}

void MainWindow::setMaxVisibleDiffLabel(float maxVisibleDiff) {
  ui->MaxDiff->setText(QString::number(maxVisibleDiff, 'e', 2));
}

void MainWindow::setInfoLabel(QString text) {
  ui->InfoLabel->setText(text);
}

void MainWindow::on_screenShot_clicked() {
    // a function that returns the current widget.
    QWidget* widget = ui->MainDisplay;
    if (not widget)
      return;

    QPixmap screenshot = widget->grab(widget->rect());
    if (screenshot.isNull())
      printf("Error printing widget");

    QDateTime d = QDateTime::currentDateTime();
    QString date = d.toString("dd.MM.yyyy.hh:mm:ss.zzz") + ".png";
    QString fileName =  "./../../../../MeshTool/examples/" + date;

    bool s = screenshot.save(fileName, "PNG", 100);
    if (not s)
      qDebug() << "Error printing widget " << date;
    else
      qDebug() << "Sreenshot saved as " << date;
}

//Turn the window transparent, for painting.
void MainWindow::on_background_clicked() {
    // a function that returns the current widget.
    QWidget* widget = ui->MainDisplay;
    if (not widget)
      return;

    if(transparent) {
        setWindowOpacity(1);
        transparent = false;
    }
    else {
        setWindowOpacity(0.7);
        transparent = true;
    }

}

void MainWindow::on_scaleSpinBox_valueChanged(double value) {
        ui->MainDisplay->updateScale(value);
    }
