#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include "mesh.h"

#define TRAINGLE_PATH ("./../../../../MeshTool/models/Triangle.obj")
#define QUAD_PATH ("./../../../../MeshTool/models/Quad.obj")
#define PENTAGON_PATH ("./../../../../MeshTool/models/Pentagon.obj")
#define HEXAGON_PATH ("./../../../../MeshTool/models/Hexagon.obj")
#define MODEL_PATH ("./../../../../MeshTool/models")


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {

  Q_OBJECT

public:
  Ui::MainWindow *ui;
  int transparent;

  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

  void importOBJ(QString filename);
  void setMaxVisibleDiffLabel(float maxVisibleDiff);
  void setInfoLabel(QString text);

private slots: // TODO: reorganize
  void on_Load_clicked();
  void on_Save_clicked();
  void on_loadTriangleButton_clicked();
  void on_loadQuadButton_clicked();
  void on_loadPentagonButton_clicked();
  void on_loadHexagonButton_clicked();
  void on_SubdivSteps_valueChanged(int value);
  void on_EditSteps_valueChanged(int value);
  void on_BrushRadiusSpinBox_valueChanged(double value);
  void on_EnableEditing_toggled(bool checked);
  void on_ShowWireframe_toggled(bool checked);
  void on_Renderer_currentIndexChanged(int index);
  void on_TessellationLevel_valueChanged(int value);
  void on_EditColorButton_clicked();
  void on_ColorBands_valueChanged(int value);
  void on_DisplayDifference_toggled(bool checked);
  void on_DiffSubdivSteps_valueChanged(int value);
  void on_DiffScale_valueChanged(int value);
  void on_screenShot_clicked();
  void on_background_clicked();
  void on_scaleSpinBox_valueChanged(double value);

private:

  static void setButtonColor(QPushButton *button, QColor color);
  void setColormapMaxLabel(int diffScale);

};

#endif // MAINWINDOW_H
