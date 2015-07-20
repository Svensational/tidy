#include "forcedirectedarranger.h"
#include <memory>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QFormLayout>
#include <QGraphicsScene>
#include <QTextStream>
#include "segment.h"
#include "segmentlist.h"

#include <QTime>

ForceDirectedArranger::ForceDirectedArranger() :
   Arranger("Force directed arranger")
{
   populateSettingsLayout();
}

QGraphicsScene * ForceDirectedArranger::arrange(SegmentList const & segments) const {
   QGraphicsScene * arrangement = new QGraphicsScene();

   QTime time;
   time.start();

   // determine background
   Segment * background = determineBackground(segments);
   SegmentList segmentsWOBack = removeBackground(segments, background);
   arrangement->setBackgroundBrush(QBrush(QColor(background->color().toQRgb())));
   segmentsWOBack.calculateFeatureVariances();

   // initialize layout
   //initializeLayout(segmentsWOBack, segmentsWOBack.featX(), segmentsWOBack.featY());
   initializeLayout(segmentsWOBack, xAxisBox->currentIndex(), yAxisBox->currentIndex());

   // refine layout
   time.restart();
   if (rotationCB->isChecked()) {
      refineLayoutWRotate(segmentsWOBack);
   }
   else {
      refineLayoutSimple(segmentsWOBack);
   }
   qDebug("Arrangement refined in %f seconds", time.restart()/1000.0);

   // convert the segments to QGraphicsItems and add to QGraphicsScene
   foreach(Segment const * const segment, segmentsWOBack) {
      arrangement->addItem(segment->toQGraphicsItem());
      // without the following line QPainter tends to crash
      arrangement->width();
   }
   saveScene(arrangement, "FD3_post.png");

   return arrangement;
}

void ForceDirectedArranger::arrangeBatch(SegmentList const & segments, QString const & name) const {
   QDir dir;
   dir.mkpath(name + "/ForceDirectedArranger");

   QFile file(name + "/ForceDirectedArranger" + "/output.txt");
   file.open(QFile::WriteOnly | QFile::Truncate);
   QTextStream out(&file);

   QTime time;
   time.start();

   // determine background
   Segment * background = determineBackground(segments);
   SegmentList segmentsWOBack = removeBackground(segments, background);
   segmentsWOBack.calculateFeatureVariances();
   out << "Feature Suggestion:" << endl;
   out << "   x = " << FeatureVector::toString(segmentsWOBack.featX()) << endl;
   out << "   y = " << FeatureVector::toString(segmentsWOBack.featY()) << endl;
   out << endl;

   for (int i=0; i<9; ++i) {
      for (int j=i+1; j<10; ++j) {
         out << "Arrangement:" << endl;
         out << "   x = " << FeatureVector::toString(i) << endl;
         out << "   y = " << FeatureVector::toString(j) << endl;
         QGraphicsScene * arrangement = new QGraphicsScene();
         arrangement->setBackgroundBrush(QBrush(QColor(background->color().toQRgb())));

         // initialize layout
         initializeLayout(segmentsWOBack, i, j);
         segmentsWOBack.resetAngles();

         // refine layout
         time.restart();
         if (i!= ANGLE && j != ANGLE) {
            refineLayoutWRotate(segmentsWOBack);
         }
         else {
            refineLayoutSimple(segmentsWOBack);
         }
         out << "   Arrangement refined in " << time.elapsed()/1000.0 << " seconds" << endl;

         // convert the segments to QGraphicsItems and add to QGraphicsScene
         foreach(Segment const * const segment, segmentsWOBack) {
            arrangement->addItem(segment->toQGraphicsItem());
            // without the following line QPainter tends to crash
            arrangement->width();
         }
         saveScene(arrangement, name +
                                "/ForceDirectedArranger" +
                                QString("/%1_%2.png").arg(FeatureVector::toString(i))
                                                     .arg(FeatureVector::toString(j)));

         delete arrangement;
         out << endl;
      }
   }

   file.close();
}

void ForceDirectedArranger::populateSettingsLayout() {
   xAxisBox = new QComboBox();
   for (int i=0; i<10; ++i) {
      xAxisBox->insertItem(i, FeatureVector::toString(i));
   }
   settingsLayout->addRow(QObject::tr("x axis"), xAxisBox);

   yAxisBox = new QComboBox();
   for (int i=0; i<10; ++i) {
      yAxisBox->insertItem(i, FeatureVector::toString(i));
   }
   yAxisBox->setCurrentIndex(1);
   settingsLayout->addRow(QObject::tr("y axis"), yAxisBox);

   rotationCB = new QCheckBox("allow");
   rotationCB->setChecked(true);
   settingsLayout->addRow(QObject::tr("Rotation"), rotationCB);
}

void ForceDirectedArranger::refineLayoutSimple(SegmentList & segments) const {
   int const count = segments.size();
   std::unique_ptr<Position[]> forces(new Position[count]);
   int collisions;
   Position forceVec;

   do {
      collisions = 0;
      for (int i=0; i<count; ++i) {
         forces[i] = Position();
      }
      for (int i=0; i<count-1; ++i) {
         for (int j=i+1; j<count; ++j) {
            if (segments.at(i)->collides(segments.at(j))) {
               ++collisions;
               forceVec = (segments.at(i)->position() - segments.at(j)->position()).normalized() * 2.0;
               forces[i] += forceVec;
               forces[j] -= forceVec;
            }
         }
      }
      for (int i=0; i<count; ++i) {
         segments[i]->translate(forces[i]);
      }
   } while (collisions > 0);
}

void ForceDirectedArranger::refineLayoutWRotate(SegmentList & segments) const {
   int const count = segments.size();
   std::unique_ptr<Position[]> forces(new Position[count]);
   std::unique_ptr<double[]> angles(new double[count]);
   int collisions;
   Position forceVec;
   double alpha;
   int counter = 0;
   do {
      collisions = 0;
      for (int i=0; i<count; ++i) {
         forces[i] = Position();
         angles[i] = 0.0;
      }
      for (int i=0; i<count-1; ++i) {
         for (int j=i+1; j<count; ++j) {
            if (segments.at(i)->collides(segments.at(j))) {
               ++collisions;
               // calculate forces
               forceVec = (segments.at(i)->position() - segments.at(j)->position()).normalized() * 2.0;
               forces[i] += forceVec;
               forces[j] -= forceVec;

               // calculate the angle of an orthogonal vector of the force vector
               alpha = atan(-forceVec.x/forceVec.y);
               if (segments.at(i)->angle() < alpha) {
                  angles[i] += 0.2;
               }
               else if (segments.at(i)->angle() > alpha) {
                  angles[i] -= 0.2;
               }
               if (segments.at(j)->angle() < alpha) {
                  angles[j] += 0.2;
               }
               else if (segments.at(j)->angle() > alpha) {
                  angles[j] -= 0.2;
               }
            }
         }
      }
      for (int i=0; i<count; ++i) {
         segments[i]->translate(forces[i]);
         segments[i]->rotate(angles[i]);
      }
      ++counter;
   } while (collisions > 0);
}
