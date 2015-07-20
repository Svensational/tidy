#ifndef ARRANGER_H
#define ARRANGER_H

#include <QString>

class QFormLayout;
class QGraphicsScene;
class QLayout;
class Segment;
class SegmentList;

class Arranger {

public:
   explicit Arranger(QString const & name);
   virtual ~Arranger();
   virtual QGraphicsScene * arrange(SegmentList const & segments) const = 0;
   virtual void arrangeBatch(SegmentList const & segments, QString const & name) const = 0;
   QString getName() const;
   QLayout * getSettingsLayout() const;

protected:
   QString name;
   QFormLayout * settingsLayout;

   Segment * determineBackground(SegmentList const & segments) const;
   SegmentList removeBackground(SegmentList const & segments,
                                Segment * const background) const;
   void initializeLayout(SegmentList & segments, int featX, int featY) const;
   void saveScene(QGraphicsScene * const scene, QString const & filename) const;
};

#endif // ARRANGER_H
