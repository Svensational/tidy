#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "image.h"
#include "segmentlist.h"

class QLabel;
class QComboBox;
class QGraphicsView;
class QGraphicsScene;
class Decomposer;
class Arranger;

class MainWindow : public QMainWindow {

   Q_OBJECT

public:
   MainWindow(QWidget * parent = 0);
   ~MainWindow();

private:
   QAction * openAction;
   QAction * quitAction;
   QAction * runDecomposerAction;
   QAction * runArrangerAction;
   QAction * runAllAction;
   QAction * runBatchAction;
   QLabel * imgOrigLbl;
   QLabel * imgSegmLbl;
   QGraphicsView * graphicsView;
   QComboBox * decomposerBox;
   QComboBox * arrangerBox;

   QString name;
   ImageColor image;
   SegmentList segments;
   QGraphicsScene * arrangement;
   QList<Decomposer *> decomposers;
   QList<Arranger *> arrangers;

   void createActions();
   void createMenues();
   void createCentralWidget();
   void createDockWidgets();
   QWidget * createDecomposerWidget();
   QWidget * createArrangerWidget();
   QString supportedImageReaderFormatsFilter() const;

private slots:
   void openImage();
   void runDecomposer();
   void runArranger();
   void runAll();
   void runBatch();
};

#endif // MAINWINDOW_H
