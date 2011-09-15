//=========================================================
//  MusE
//  Linux Music Editor
//  $Id: astrip.h,v 1.8.2.6 2009/11/14 03:37:48 terminator356 Exp $
//
//  (C) Copyright 2000-2004 Werner Schweer (ws@seh.de)
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; version 2 of
//  the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//=========================================================

#ifndef __ASTRIP_H__
#define __ASTRIP_H__

#include <vector>

#include "strip.h"
//#include "route.h"

namespace MusEWidget {
class DoubleLabel;
class Slider;
class Knob;
class TransparentToolButton;
}

//class QDialog;
class QToolButton;
//class QAction;
//class QPopupMenu;
//class PopupMenu;
class QButton;
class AudioTrack;

namespace MusEMixer {

class EffectRack;
  
//---------------------------------------------------------
//   AudioStrip
//---------------------------------------------------------

class AudioStrip : public Strip {
      Q_OBJECT

      int channel;
      MusEWidget::Slider* slider;
      MusEWidget::DoubleLabel* sl;
      EffectRack* rack;

      MusEWidget::Knob* pan;
      MusEWidget::DoubleLabel* panl;

      std::vector<MusEWidget::Knob*> auxKnob;
      std::vector<MusEWidget::DoubleLabel*> auxLabel;

      QToolButton* stereo;
      QToolButton* pre;
      MusEWidget::TransparentToolButton* off;

      double volume;
      double panVal;

      //QToolButton* iR;
      //QToolButton* oR;
      
      MusEWidget::Knob* addKnob(int, int, MusEWidget::DoubleLabel**);
      
      void updateOffState();
      void updateVolume();
      void updatePan();
      void updateChannels();

   private slots:
      void stereoToggled(bool);
      void preToggled(bool);
      void offToggled(bool);
      void iRoutePressed();
      void oRoutePressed();
      void auxChanged(double, int);
      void volumeChanged(double);
      void volumePressed();
      void volumeReleased();
      void panChanged(double);
      void panPressed();
      void panReleased();
      void volLabelChanged(double);
      void panLabelChanged(double);
      void auxLabelChanged(double, unsigned int);
      void volumeRightClicked(const QPoint &);
      void panRightClicked(const QPoint &);

   protected slots:
      virtual void heartBeat();

   public slots:
      virtual void configChanged();
      virtual void songChanged(int);

   public:
      AudioStrip(QWidget* parent, AudioTrack*);
      ~AudioStrip();
      ///virtual QSize minimumSizeHint () const;
      //virtual QSize sizeHint () const;
      };

} // namespace MusEMixer

#endif

