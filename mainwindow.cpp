#include "mainwindow.h"
#include <QAction>
#include <QBoxLayout>
#include <QComboBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QGraphicsView>
#include <QGroupBox>
#include <QImageReader>
#include <QLabel>
#include <QMenuBar>
#include <QPushButton>
#include <QSplitter>
#include <QStackedLayout>
#include "meanshiftdecomposer.h"
#include "watersheddecomposer.h"
#include "forcedirectedarranger.h"
#include "clusteredarranger.h"

MainWindow::MainWindow(QWidget * parent) :
   QMainWindow(parent), arrangement(nullptr)
{
   setWindowTitle(tr("tidy"));
   resize(512, 384);

   decomposers << new MeanShiftDecomposer()
               << new WaterShedDecomposer();

   arrangers << new ForceDirectedArranger()
             << new ClusteredArranger();

   createActions();
   createMenues();
   createCentralWidget();
   createDockWidgets();
}

MainWindow::~MainWindow() {
   // delete segments
   segments.deleteAndClear();
   delete arrangement;

   // delete decomposers
   for (int i=0; i<decomposers.size(); ++i) {
      delete decomposers[i];
   }

   // delete arrangers
   for (int i=0; i<arrangers.size(); ++i) {
      delete arrangers[i];
   }
}

void MainWindow::createActions() {
   openAction = new QAction(QIcon(":/icons/open16"), tr("&Open image"), this);
   openAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
   connect(openAction, SIGNAL(triggered()), this, SLOT(openImage()));

   quitAction = new QAction(QIcon(":/icons/quit16"), tr("&Quit"), this);
   quitAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
   connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));


   runDecomposerAction = new QAction(QIcon(":/icons/run16"), tr("Run decomposer"), this);
   connect(runDecomposerAction, SIGNAL(triggered()), this, SLOT(runDecomposer()));

   runArrangerAction = new QAction(QIcon(":/icons/run16"), tr("Run arranger"), this);
   connect(runArrangerAction, SIGNAL(triggered()), this, SLOT(runArranger()));

   runAllAction = new QAction(QIcon(":/icons/runall16"), tr("&Run all"), this);
   runAllAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
   connect(runAllAction, SIGNAL(triggered()), this, SLOT(runAll()));

   runBatchAction = new QAction(QIcon(":/icons/runall16"), tr("&Run batch"), this);
   connect(runBatchAction, SIGNAL(triggered()), this, SLOT(runBatch()));
}

QWidget * MainWindow::createArrangerWidget() {
   QWidget * widget = new QWidget();
   QVBoxLayout * mainLayout = new QVBoxLayout();

   arrangerBox = new QComboBox();
   mainLayout->addWidget(arrangerBox);

   QGroupBox * arrangerSettings = new QGroupBox(tr("Settings"));
   QStackedLayout * arrangSetLayout = new QStackedLayout();
   arrangerSettings->setLayout(arrangSetLayout);
   mainLayout->addWidget(arrangerSettings);

   QWidget * widgetSet;
   foreach (Arranger const * const arranger, arrangers) {
      arrangerBox->insertItem(arrangerBox->count(), arranger->getName());
      widgetSet = new QWidget();
      widgetSet->setLayout(arranger->getSettingsLayout());
      arrangSetLayout->addWidget(widgetSet);
   }
   connect(arrangerBox, SIGNAL(currentIndexChanged(int)), arrangSetLayout, SLOT(setCurrentIndex(int)));

   QPushButton * runBtn = new QPushButton(tr("Run arranger"));
   connect(runBtn, SIGNAL(clicked()), this, SLOT(runArranger()));
   mainLayout->addWidget(runBtn);

   mainLayout->addStretch();
   widget->setLayout(mainLayout);
   return widget;
}

void MainWindow::createCentralWidget() {
   QSplitter * splitter = new QSplitter();

    imgOrigLbl = new QLabel();
    splitter->addWidget(imgOrigLbl);

    imgSegmLbl = new QLabel();
    splitter->addWidget(imgSegmLbl);

    graphicsView = new QGraphicsView();
    graphicsView->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    splitter->addWidget(graphicsView);

   setCentralWidget(splitter);
}

QWidget * MainWindow::createDecomposerWidget() {
   QWidget * widget = new QWidget();
   QVBoxLayout * mainLayout = new QVBoxLayout();

   decomposerBox = new QComboBox();
   mainLayout->addWidget(decomposerBox);

   QGroupBox * decomposerSettings = new QGroupBox(tr("Settings"));
   QStackedLayout * decompSetLayout = new QStackedLayout();
   decomposerSettings->setLayout(decompSetLayout);
   mainLayout->addWidget(decomposerSettings);

   QWidget * widgetSet;
   foreach (Decomposer const * const decomposer, decomposers) {
      decomposerBox->insertItem(decomposerBox->count(), decomposer->getName());
      widgetSet = new QWidget();
      widgetSet->setLayout(decomposer->getSettingsLayout());
      decompSetLayout->addWidget(widgetSet);
   }
   connect(decomposerBox, SIGNAL(currentIndexChanged(int)), decompSetLayout, SLOT(setCurrentIndex(int)));

   QPushButton * runBtn = new QPushButton(tr("Run decomposer"));
   connect(runBtn, SIGNAL(clicked()), this, SLOT(runDecomposer()));
   mainLayout->addWidget(runBtn);

   mainLayout->addStretch();
   widget->setLayout(mainLayout);
   return widget;
}

void MainWindow::createDockWidgets() {
   QDockWidget * decomposerDockWidget = new QDockWidget(tr("Decomposer"), this);
   decomposerDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
   decomposerDockWidget->setWidget(createDecomposerWidget());
   addDockWidget(Qt::LeftDockWidgetArea, decomposerDockWidget);

   QDockWidget * arrangerDockWidget = new QDockWidget(tr("Arranger"), this);
   arrangerDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
   arrangerDockWidget->setWidget(createArrangerWidget());
   addDockWidget(Qt::LeftDockWidgetArea, arrangerDockWidget);
}

void MainWindow::createMenues() {
   QMenu * fileMenu = menuBar()->addMenu(tr("File"));
   fileMenu->addAction(openAction);
   fileMenu->addSeparator();
   fileMenu->addAction(quitAction);

   QMenu * runMenu = menuBar()->addMenu(tr("Run"));
   runMenu->addAction(runDecomposerAction);
   runMenu->addAction(runArrangerAction);
   runMenu->addSeparator();
   runMenu->addAction(runAllAction);
   runMenu->addAction(runBatchAction);
}

void MainWindow::openImage() {
   static QString filter = supportedImageReaderFormatsFilter();

   QString filename = QFileDialog::getOpenFileName(this,
                                                   tr("Open image file"),
                                                   QString(),
                                                   filter);
   if (!filename.isNull()) {
      //load image
      image = ImageColor(filename);
      imgOrigLbl->setPixmap(QPixmap::fromImage(image.toQImage()));

      int begin = filename.lastIndexOf('/')+1;
      int width = filename.lastIndexOf('.') - begin;
      name = filename.mid(begin, width);
   }
}

void MainWindow::runAll() {
   runDecomposer();
   runArranger();
}

void MainWindow::runArranger() {
   QGraphicsScene * goner = arrangement;
   arrangement = arrangers.at(arrangerBox->currentIndex())->arrange(segments);

   // show the new arrangement
   graphicsView->setScene(arrangement);

   // delete the old arrangement
   delete goner;
}

void MainWindow::runBatch() {
   segments.deleteAndClear();
   segments = decomposers.at(decomposerBox->currentIndex())->decomposeBatch(image, name);
   segments.prepare();
   foreach (Arranger * const arranger, arrangers) {
      arranger->arrangeBatch(segments, name);
   }
   qDebug("Batchrun done");
}

void MainWindow::runDecomposer() {
   segments.deleteAndClear();
   segments = decomposers.at(decomposerBox->currentIndex())->decompose(image);
   segments.prepare();

   // show segmented image
   ImageColor resultImage(image.width(), image.height());
   segments.copyToImageAVG(resultImage);
   imgSegmLbl->setPixmap(QPixmap::fromImage(resultImage.toQImage()));
}

QString MainWindow::supportedImageReaderFormatsFilter() const {
   QString filter("Images (");
   foreach (QByteArray format, QImageReader::supportedImageFormats()) {
      filter.append("*." + format + ' ');
   }
   filter[filter.length()-1] = ')';
   return filter;
}
