#include "clusteredarranger.h"
#include <memory>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QFormLayout>
#include <QGraphicsScene>
#include <QTextStream>
#include <QTime>
#include "segment.h"
#include "segmentlist.h"

ClusteredArranger::ClusteredArranger() :
   Arranger("Clustered Arranger (FD)")
{
   populateSettingsLayout();
}

QGraphicsScene * ClusteredArranger::arrange(SegmentList const & segments) const {
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

   // find clusters
   time.restart();
   QList<SegmentList> clusters = meanShift(segmentsWOBack);
   qDebug("Segments clustered in %f seconds", time.restart()/1000.0);
   qDebug("  %d clusters found", clusters.size());

   // refine clusters
   //int counter = 0;
   foreach (SegmentList cluster, clusters) {
      if (clusterBox->currentIndex() == 0) {
         refineLayoutCircles(cluster);
      }
      else if (clusterBox->currentIndex() == 1) {
         refineLayoutPiles(cluster);
      }

      // debug output
      /*QGraphicsScene scene;
      scene.setBackgroundBrush(QBrush(QColor(255, 255, 255)));
      foreach(Segment * const segment, cluster) {
         scene.addItem(segment->toQGraphicsItem());
         // without the following line QPainter tends to crash
         scene.width();
      }
      ++counter;
      saveScene(&scene, QString("Test%1.png").arg(counter, 2));*/
   }

   // refine layout
   if (clusterBox->currentIndex() == 0) {
      refineLayoutByPlace(clusters);
   }
   else if (clusterBox->currentIndex() == 1) {
      refineLayoutBySize(clusters);
   }

   // convert the segments to QGraphicsItems and add to QGraphicsScene
   foreach(Segment const * const segment, segmentsWOBack) {
      arrangement->addItem(segment->toQGraphicsItem());
      // without the following line QPainter tends to crash
      arrangement->width();
   }
   saveScene(arrangement, "FD3_post.png");

   return arrangement;
}

void ClusteredArranger::arrangeBatch(SegmentList const & segments, QString const & name) const {
   QDir dir;
   dir.mkpath(name + "/ClusteredArranger");

   QFile file(name + "/ClusteredArranger" + "/output.txt");
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

         // initialize layout
         initializeLayout(segmentsWOBack, i, j);

         // find clusters
         time.restart();
         QList<SegmentList> clusters = meanShift(segmentsWOBack);
         out << "   Segments clustered in " << time.restart()/1000.0 << " seconds" << endl;
         out << "      " << clusters.size() << " clusters found" << endl;

         // CIRCLES
         // refine clusters
         segmentsWOBack.resetAngles();
         time.restart();
         foreach (SegmentList cluster, clusters) {
            refineLayoutCircles(cluster);
         }
         // refine layout
         refineLayoutByPlace(clusters);
         out << "   Sphere clusteres refined in " << time.restart()/1000.0 << " seconds" << endl;
         // convert the segments to QGraphicsItems and add to QGraphicsScene
         QGraphicsScene * arrangement = new QGraphicsScene();
         arrangement->setBackgroundBrush(QBrush(QColor(background->color().toQRgb())));
         foreach(Segment const * const segment, segmentsWOBack) {
            arrangement->addItem(segment->toQGraphicsItem());
            // without the following line QPainter tends to crash
            arrangement->width();
         }
         saveScene(arrangement, name + "/ClusteredArranger" +
                                QString("/%1_%2_Circles.png").arg(FeatureVector::toString(i))
                                                     .arg(FeatureVector::toString(j)));

         // PILES
         delete arrangement;
         initializeLayout(segmentsWOBack, i, j);
         segmentsWOBack.resetAngles();
         // refine clusters
         time.restart();
         foreach (SegmentList cluster, clusters) {
            refineLayoutPiles(cluster);
         }
         // refine layout
         refineLayoutBySize(clusters);
         out << "   Piles clusteres refined in " << time.restart()/1000.0 << " seconds" << endl;
         // convert the segments to QGraphicsItems and add to QGraphicsScene
         arrangement = new QGraphicsScene();
         arrangement->setBackgroundBrush(QBrush(QColor(background->color().toQRgb())));
         foreach(Segment const * const segment, segmentsWOBack) {
            arrangement->addItem(segment->toQGraphicsItem());
            // without the following line QPainter tends to crash
            arrangement->width();
         }
         saveScene(arrangement, name +
                                "/ClusteredArranger" +
                                QString("/%1_%2_Piles.png").arg(FeatureVector::toString(i))
                                                     .arg(FeatureVector::toString(j)));

         delete arrangement;
         out << endl;
      }
   }

   file.close();
}

bool ClusteredArranger::biggerAreaThan(SegmentList const & listA, SegmentList const & listB) {
   return listA.area() > listB.area();
}

bool ClusteredArranger::biggerThan(SegmentList const & listA, SegmentList const & listB) {
   return listA.size() > listB.size();
}

QList<SegmentList> ClusteredArranger::meanShift(SegmentList const & segments) const {
   double const sigma = segments.area()>>5;
   QList<Position> modi;
   QList<int> ids;

   // filter the data
   Position center;
   Position nextCenter;
   int count;
   foreach (Segment const * const segment, segments) {
      nextCenter = segment->position();
      do {
         center = nextCenter;
         nextCenter = Position();
         count = 0;
         foreach (Segment const * const other, segments) {
            if ((other->position()-center).magnitudeSquared() < sigma) {
               nextCenter += other->position();
               ++count;
            }
         }
         nextCenter /= double(count);
      } while ((nextCenter - center).magnitudeSquared() > 0.01);
      modi << nextCenter;
      ids << ids.size();
   }

   // gather the modi
   int oldID;
   for (int i=0; i<modi.size()-1; ++i) {
      for (int j=i+1; j<modi.size(); ++j) {
         if (ids.at(i)!=ids.at(j) &&
             (modi.at(i)-modi.at(j)).magnitudeSquared() < sigma) {
            // merge
            oldID = ids.at(j);
            for (int k=0; k<modi.size(); ++k) {
               if (ids.at(k) == oldID) {
                  ids[k] = ids.at(i);
               }
            }
         }
      }
   }

   // group the segments
   QList<SegmentList> list;
   for (int i=0; i<ids.size(); ++i) {
      if (ids.contains(i)) {
         SegmentList cluster;
         for (int j=0; j<ids.size(); ++j) {
            if (ids.at(j) == i) {
               cluster << segments.at(j);
            }
         }
         list << cluster;
      }
   }

   // sort the cluster-list (descending by size)
   std::sort(list.begin(), list.end(), biggerThan);

   // merge small clusters
   int const minSize = std::max(3, modi.size()/100);
   double minD;
   double tempMinD;
   int minI = -1;
   while (list.size()>1 && list.last().size() < minSize) {
      // find nearest neighbour
      minD = std::numeric_limits<double>::max();
      for (int i=0; i<list.size()-1; ++i) {
         tempMinD = minDist(list.at(i), list.last());
         if (tempMinD < minD) {
            minD = tempMinD;
            minI = i;
         }
      }
      // merge
      list[minI].append(list.takeLast());
   }

   return list;
}

double ClusteredArranger::minDist(SegmentList const & listA, SegmentList const & listB) const {
   double min = std::numeric_limits<double>::max();

   foreach (Segment const * const segmentA, listA) {
      foreach (Segment const * const segmentB, listB) {
         min = std::min(min, (segmentA->position()-segmentB->position()).magnitude());
      }
   }

   return min;
}

void ClusteredArranger::populateSettingsLayout() {
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

   clusterBox = new QComboBox();
   clusterBox->insertItem(0, "Circles");
   clusterBox->insertItem(1, "Piles");
   settingsLayout->addRow(QObject::tr("Shape"), clusterBox);
}

void ClusteredArranger::refineLayoutByPlace(QList<SegmentList> & clusters) const {
   int const count = clusters.size();
   std::unique_ptr<Position[]> centers(new Position[count]);
   std::unique_ptr<double[]> radii(new double[count]);
   QRectF rect;
   for (int i=0; i<count; ++i) {
      centers[i] = clusters.at(i).center();
      rect = clusters.at(i).rect();
      radii[i] = std::max(rect.width(), rect.height()) / 2.0;
   }

   std::unique_ptr<Position[]> forces(new Position[count]);
   int collisions;
   Position forceVec;
   Position center;
   int pass = 0;

   do {
      // reset collisions and forces
      collisions = 0;
      for (int i=0; i<count; ++i) {
         forces[i] = Position();
      }

      // find collisions
      for (int i=0; i<count-1; ++i) {
         for (int j=i+1; j<count; ++j) {
            if ((centers[i]-centers[j]).magnitude() < (radii[i]+radii[j])*1.1) {
               ++collisions;
               forceVec = (centers[i]-centers[j]).normalized() * 5.0;
               forces[i] += forceVec;
               forces[j] -= forceVec;
            }
         }
      }

      // apply forces
      for (int i=0; i<count; ++i) {
         clusters[i].translate(forces[i]);
         centers[i] = clusters.at(i).center();
      }

      // calculate center of clusters
      center = Position();
      int size = 0;
      for (int i=0; i<count; ++i) {
         center += centers[i] * clusters.at(i).area();
         size += clusters.at(i).area();
      }
      center /= double(size);

      // make it more compact
      if (pass < 10) {
         for (int i=0; i<count; ++i) {
            forceVec = (center - centers[i]) * 0.05 * (10-pass);
            clusters[i].translate(forceVec);
         }
         ++collisions;
      }

      ++pass;
   } while (collisions > 0);
}

void ClusteredArranger::refineLayoutBySize(QList<SegmentList> & clusters) const {
   QRectF rect;
   double x = 0.0;

   // sort the cluster-list (descending by area)
   std::sort(clusters.begin(), clusters.end(), biggerAreaThan);

   foreach (SegmentList cluster, clusters) {
      rect = cluster.rect();
      Position bl(rect.left(), rect.bottom());

      cluster.translate(-bl + Position(x, 0.0));
      x += rect.width() + 50.0;
   }
}

void ClusteredArranger::refineLayoutCircles(SegmentList & segments) const {
   int const count = segments.size();
   std::unique_ptr<Position[]> forces(new Position[count]);
   int collisions;
   Position forceVec;
   int pass = 0;

   do {
      // reset collisions and forces
      collisions = 0;
      for (int i=0; i<count; ++i) {
         forces[i] = Position();
      }

      // find collisions
      for (int i=0; i<count-1; ++i) {
         for (int j=i+1; j<count; ++j) {
            if (segments.at(i)->collides(segments.at(j))) {
               // calculate force
               forceVec = (segments.at(i)->position() - segments.at(j)->position()).normalized() * 2.0;
               forces[i] += forceVec;
               forces[j] -= forceVec;
               ++collisions;
            }
         }
      }

      // apply forces
      for (int i=0; i<count; ++i) {
         segments[i]->translate(forces[i]);
      }

      // rotate (align to tangent)
      Position center = segments.center();
      foreach (Segment * const segment, segments) {
         forceVec = segment->position() - center; // not actually a force here, just recycling
         segment->setAngle(atan(forceVec.y / forceVec.x) + 1.570796327);
      }

      // make it more compact
      if (pass < 10) {
         foreach (Segment * const segment, segments) {
            forceVec = (center - segment->position()) * 0.05 * (10-pass);
            segment->translate(forceVec);
         }
      }

      ++pass;
   } while (collisions > 0 || pass<10);
}

void ClusteredArranger::refineLayoutPiles(SegmentList & segments) const {
   int const count = segments.size();
   std::unique_ptr<Position[]> forces(new Position[count]);
   int collisions;
   Position forceVec;
   int pass = 0;
   int const maxPass = 20;

   // rotate (align horizontal)
   foreach (Segment * const segment, segments) {
      segment->setAngle(0.0);
   }

   do {
      // reset collisions and forces
      collisions = 0;
      for (int i=0; i<count; ++i) {
         forces[i] = Position();
      }

      // find collisions
      for (int i=0; i<count-1; ++i) {
         for (int j=i+1; j<count; ++j) {
            if (segments.at(i)->collides(segments.at(j))) {
               // calculate force
               forceVec = (segments.at(i)->position() - segments.at(j)->position()).normalized() * 2.0;
               forces[i] += forceVec * Position(0.1, 1.0);
               forces[j] -= forceVec * Position(0.1, 1.0);
               ++collisions;
            }
         }
      }

      // apply forces
      for (int i=0; i<count; ++i) {
         segments[i]->translate(forces[i]);
      }

      // make it more compact
      Position center = segments.topCenter();
      if (pass < maxPass) {
         foreach (Segment * const segment, segments) {
            forceVec = Position((center.x - segment->position().x) * 0.9 * (maxPass-pass)/double(maxPass),
                                (center.y - segment->position().y) * 0.05 * (maxPass-pass)/double(maxPass));
            segment->translate(forceVec);
         }
      }

      ++pass;
   } while (collisions > 0 || pass<maxPass);
}
