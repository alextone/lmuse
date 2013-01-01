//=========================================================
//  MusE
//  Linux Music Editor
//    $Id: dcanvas.h,v 1.8.2.2 2009/02/02 21:38:00 terminator356 Exp $
//  (C) Copyright 1999 Werner Schweer (ws@seh.de)
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

#ifndef __DCANVAS_H__
#define __DCANVAS_H__

#include "ecanvas.h"
#include "song.h"
#include "steprec.h"
#include <map>
#include <QList>
#include <QSet>

#define TH 18


class QResizeEvent;
class QDragEnterEvent;
class QDropEvent;
class QDragMoveEvent;
class QDragLeaveEvent;

class DrumMap;
namespace MusEGui {

class MidiEditor;
class DrumEdit;

//---------------------------------------------------------
//   DEvent
//    ''visual'' Drum Event
//---------------------------------------------------------

class DEvent : public CItem {
   public:
      DEvent(MusECore::Event e, MusECore::Part* p, int instr);
      };


struct instrument_number_mapping_t //FINDMICH TODO move into a suitable namespace!
{
  QSet<MusECore::Track*> tracks;
  int pitch;
  
  instrument_number_mapping_t()
  {
    pitch=-1;
    tracks.clear();
  }
  
  instrument_number_mapping_t(const QSet<MusECore::Track*>& tr, int p)
  {
    tracks=tr;
    pitch=p;
  }
  
  bool operator==(const instrument_number_mapping_t& that) //TODO maybe compare the Track* serial numbers, not the pointers themselves?
  {
    return (this->tracks == that.tracks && this->pitch==that.pitch);
  }
  
  bool operator!=(const instrument_number_mapping_t& that)
  {
    return !operator==(that);
  }
};

//---------------------------------------------------------
//   DrumCanvas
//---------------------------------------------------------

class DrumCanvas : public EventCanvas {
      Q_OBJECT
      
      bool old_style_drummap_mode;
      MusECore::DrumMap* ourDrumMap;
      bool must_delete_our_drum_map; //FINDMICH really delete it!
      QVector<instrument_number_mapping_t> instrument_map;
      
      DrumEdit* drumEditor;
      
      MusECore::StepRec* steprec;
      
      // Cursor tool position
      QPoint cursorPos;
      int _stepSize;

      
      virtual void drawCanvas(QPainter&, const QRect&);
      virtual void drawItem(QPainter&, const CItem*, const QRect&);
      void drawTopItem(QPainter& p, const QRect& rect);
      virtual void drawMoving(QPainter&, const CItem*, const QRect&);
      virtual MusECore::Undo moveCanvasItems(CItemList&, int, int, DragType);
      virtual bool moveItem(MusECore::Undo&, CItem*, const QPoint&, DragType);
      virtual CItem* newItem(const QPoint&, int);
      virtual void resizeItem(CItem*, bool, bool);
      virtual void newItem(CItem*, bool);
      virtual void newItem(CItem*, bool, bool replace );
      virtual bool deleteItem(CItem*);
      virtual void itemPressed(const CItem*);
      virtual void itemReleased(const CItem*, const QPoint&);

      CItem* newItem(int tick, int instrument, int velocity);

      int y2pitch(int y) const;
      int pitch2y(int pitch) const;
      void startDrag(CItem*, bool copymode);
      void dragEnterEvent(QDragEnterEvent* event);
      void dragMoveEvent(QDragMoveEvent*);
      void dragLeaveEvent(QDragLeaveEvent*);
      virtual CItem* addItem(MusECore::Part*, MusECore::Event&);
      virtual void resizeEvent(QResizeEvent*);
      virtual void curPartChanged();
      int getNextStep(unsigned int pos, int basicStep, int stepSize=1);
      
   signals:
      void newWidth(int);
      void ourDrumMapChanged(bool /*instrumentMap changed as well?*/);

   private slots:
      void midiNote(int pitch, int velo);
      
   public slots:
      void mapChanged(int, int);
      void keyPressed(int, int);
      void keyReleased(int, bool);
      void setTool2(int);
      void setCurDrumInstrument(int);
      virtual void setStep(int);
      void moveAwayUnused();

   public:
      enum {
         CMD_CUT, CMD_COPY, CMD_COPY_RANGE, CMD_PASTE, CMD_PASTE_DIALOG, CMD_SAVE, CMD_LOAD, CMD_RESET,
         CMD_SELECT_ALL, CMD_SELECT_NONE, CMD_SELECT_INVERT,
         CMD_SELECT_ILOOP, CMD_SELECT_OLOOP, CMD_SELECT_PREV_PART, CMD_SELECT_NEXT_PART, 
         CMD_DEL, CMD_FIXED_LEN, CMD_RIGHT, CMD_LEFT, CMD_RIGHT_NOSNAP, CMD_LEFT_NOSNAP, CMD_MODIFY_VELOCITY, CMD_CRESCENDO,
         CMD_QUANTIZE, CMD_ERASE_EVENT, CMD_NOTE_SHIFT, CMD_DELETE_OVERLAPS, CMD_REORDER_LIST,
         CMD_GROUP_NONE, CMD_GROUP_CHAN, CMD_GROUP_MAX
         };
      DrumCanvas(MidiEditor*, QWidget*, int, int,
         const char* name = 0);
      virtual ~DrumCanvas();
      void cmd(int);
      virtual void modifySelected(NoteInfo::ValType type, int val, bool delta_mode = true);
      virtual void keyPress(QKeyEvent* event);
      MusECore::Event *getEventAtCursorPos();
      void selectCursorEvent(MusECore::Event *ev);

      int pitch_and_track_to_instrument(int pitch, MusECore::Track* track);

      MusECore::DrumMap* getOurDrumMap() { return ourDrumMap; }
      int getOurDrumMapSize() { return instrument_map.size(); }
      QVector<instrument_number_mapping_t>& get_instrument_map() { return instrument_map; }
      void propagate_drummap_change(int instrument, bool update_druminmap);
      void rebuildOurDrumMap();
      DrumEdit* drumEdit() { return drumEditor; }
      };

} // namespace MusEGui

#endif

