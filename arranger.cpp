#include "arranger.h"
#include <QFormLayout>
#include <QGraphicsScene>
#include <QPainter>
#include <QPixmap>
#include "segment.h"
#include "segmentlist.h"

Arranger::Arranger(QString const & name) :
   name(name), settingsLayout(new QFormLayout())
{
}

Arranger::~Arranger() {
   delete settingsLayout;
}

Segment * Arranger::determineBackground(SegmentList const & segments) const {
   // determin background according to neighbor count and area
   Segment * backgroundNeighbors = nullptr;
   int maxNeighbors = 0;
   Segment * backgroundArea = nullptr;
   int maxArea = 0;
   foreach (Segment * const segment, segments) {
      if (segment->neighbours().size() > maxNeighbors) {
         maxNeighbors = segment->neighbours().size();
         backgroundNeighbors = segment;
      }
      if (segment->area() > maxArea) {
         maxArea = segment->area();
         backgroundArea = segment;
      }
   }

   // if determination is unanimous
   if (backgroundNeighbors == backgroundArea) {
      return backgroundNeighbors;
   }
   else {
      // calculate expected values
      double eNeighbors = 0.0;
      double eArea = 0.0;
      foreach (Segment const * const segment, segments) {
         eNeighbors += segment->neighbours().size();
         eArea += segment->area();
      }
      eNeighbors /= double(segments.size());
      eArea /= double(segments.size());

      // calculate standard deviations
      double sigmaNeighbors = 0.0;
      double sigmaArea = 0.0;
      foreach (Segment const * const segment, segments) {
         sigmaNeighbors += pow(segment->neighbours().size() - eNeighbors, 2.0);
         sigmaArea = pow(segment->area() - eArea, 2.0);
      }
      sigmaNeighbors = sqrt(sigmaNeighbors / double(segments.size()));
      sigmaArea = sqrt(sigmaArea / double(segments.size()));

      // choose which feature is more significant
      if (fabs(maxNeighbors-eNeighbors)/sigmaNeighbors >
          fabs(maxArea-eArea)/sigmaArea) {
         return backgroundNeighbors;
      }
      else {
         return backgroundArea;
      }
   }
}

QString Arranger::getName() const {
   return name;
}

QLayout * Arranger::getSettingsLayout() const {
   return settingsLayout;
}

void Arranger::initializeLayout(SegmentList & segments, int featX, int featY) const {
   double const edgelength = sqrt(segments.area()<<2);
   foreach (Segment * const segment, segments) {
      segment->setPosition(Position(segment->features()[featX] * edgelength,
                                    segment->features()[featY] * edgelength));
   }
}

SegmentList Arranger::removeBackground(SegmentList const & segments,
                                       Segment * const background) const {
   SegmentList remain;

   foreach(Segment * const segment, segments) {
      if ((segment->color() - background->color()).magnitudeSquared() > 25.0) {
         remain << segment;
      }
   }

   return remain;
}

void Arranger::saveScene(QGraphicsScene * const scene, QString const & filename) const {
   QPixmap pixmap(scene->sceneRect().size().toSize());

   QPainter painter(&pixmap);
   scene->render(&painter);

   pixmap.save(filename);
}
