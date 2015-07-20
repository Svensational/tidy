#include "decomposer.h"
#include <QFormLayout>
#include "segment.h"
#include "segmentlist.h"

Decomposer::Decomposer(QString const & name) :
   name(name), settingsLayout(new QFormLayout())
{
}

Decomposer::~Decomposer() {
   delete settingsLayout;
}

QString Decomposer::getName() const {
   return name;
}

QLayout * Decomposer::getSettingsLayout() const {
   return settingsLayout;
}

void Decomposer::merge(QList<QPair<Segment *, Segment *>> & mergelist, SegmentList & segments) const {
   QPair<Segment *, Segment *> pair;
   while (!mergelist.isEmpty()) {
      pair = mergelist.takeFirst();
      if (pair.first != pair.second) {
         // replace merged segment in the rest of the merge list
         for (int i=0; i<mergelist.size(); ++i) {
            if (mergelist[i].first == pair.second) {
               mergelist[i].first = pair.first;
            }
            if (mergelist[i].second == pair.second) {
               mergelist[i].second = pair.first;
            }
         }
         //merge
         pair.first->merge(pair.second);
         segments.removeAll(pair.second);
         delete pair.second;
      }
   }
}

void Decomposer::mergeSimiliarSegments(SegmentList & segments, double epsSquared) const {
   // find couples to merge
   QList<QPair<Segment *, Segment *>> mergelist;
   foreach (Segment * segment, segments) {
      foreach (Segment * neighbour, segment->neighbours()) {
         if ((segment->color()-neighbour->color()).magnitudeSquared() < epsSquared) {
            mergelist << QPair<Segment *, Segment *>(segment, neighbour);
         }
      }
   }

   // merge couples
   merge(mergelist, segments);
}

void Decomposer::mergeSmallSegments(SegmentList & segments, int minSize) const{
   // find couples to merge
   QList<QPair<Segment *, Segment *>> mergelist;
   double dist, minDist;
   Segment * nearestNeighbour;
   foreach (Segment * segment, segments) {
      if (segment->area() < minSize) {
         minDist = std::numeric_limits<double>::max();
         nearestNeighbour = nullptr;
         foreach (Segment * neighbour, segment->neighbours()) {
            dist = (segment->color()-neighbour->color()).magnitudeSquared();
            if (dist < minDist) {
               minDist = dist;
               nearestNeighbour = neighbour;
            }
         }
         mergelist << QPair<Segment *, Segment *>(nearestNeighbour, segment);
      }
   }

   // merge couples
   merge(mergelist, segments);
}
