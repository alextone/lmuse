//=========================================================
//  MusE
//  Linux Music Editor
//    $Id: dcanvas.cpp,v 1.16.2.10 2009/10/15 22:45:50 terminator356 Exp $
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

#include <QPainter>
#include <QApplication>
#include <QMessageBox>
#include <QClipboard>
#include <QDrag>
#include <QDragLeaveEvent>
#include <QPolygon>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QResizeEvent>

#include <stdio.h>
#include <values.h>
#include <errno.h>
//#include <sys/stat.h>
//#include <sys/mman.h>

#include "dcanvas.h"
#include "midieditor.h"
#include "drumedit.h"
#include "drummap.h"
#include "event.h"
#include "mpevent.h"
#include "xml.h"
#include "globals.h"
#include "midiport.h"
#include "audio.h"
#include "shortcuts.h"
#include "icons.h"
#include "functions.h"
#include "helper.h"

#define CARET   10
#define CARET2   5

using MusEGlobal::debugMsg;
using MusEGlobal::heavyDebugMsg;

namespace MusEGui {

//---------------------------------------------------------
//   DEvent
//---------------------------------------------------------

DEvent::DEvent(MusECore::Event e, MusECore::Part* p, int instr)
  : CItem(e, p)
      {
      int y  = instr * TH + TH/2;
      int tick = e.tick() + p->tick();
      setPos(QPoint(tick, y));
      setBBox(QRect(-CARET2, -CARET2, CARET, CARET));
      }

//---------------------------------------------------------
//   addItem
//---------------------------------------------------------

CItem* DrumCanvas::addItem(MusECore::Part* part, MusECore::Event& event)
      {
      if (signed(event.tick())<0) {
            printf("ERROR: trying to add event before current part!\n");
            return NULL;
      }
      
      int instr=pitch_and_track_to_instrument(event.pitch(), part->track());
      if (instr<0)
      {
        if (heavyDebugMsg) printf("trying to add event which is hidden or not in any part known to me\n");
        return NULL;
      }
      
      DEvent* ev = new DEvent(event, part, instr);
      items.add(ev);
      
      int diff = event.endTick()-part->lenTick();
      if (diff > 0)  {// too short part? extend it
            //printf("addItem - this code should not be run!\n");
            //MusECore::Part* newPart = part->clone();
            //newPart->setLenTick(newPart->lenTick()+diff);
            //MusEGlobal::audio->msgChangePart(part, newPart,false);
            //part = newPart;
            part->setLenTick(part->lenTick()+diff);
            }
      
      return ev;
      }

//---------------------------------------------------------
//   DrumCanvas
//---------------------------------------------------------

DrumCanvas::DrumCanvas(MidiEditor* pr, QWidget* parent, int sx,
   int sy, const char* name)
   : EventCanvas(pr, parent, sx, sy, name)
      {
      drumEditor=dynamic_cast<DrumEdit*>(pr);
      
      _setCurPartIfOnlyOneEventIsSelected=false;
      
      old_style_drummap_mode = drumEditor->old_style_drummap_mode();

      if (old_style_drummap_mode)
      {
        if (debugMsg) printf("DrumCanvas in old style drummap mode\n");
        ourDrumMap = MusEGlobal::drumMap;
        must_delete_our_drum_map=false;
        
        instrument_number_mapping_t temp;
        for (MusECore::ciPart it=drumEditor->parts()->begin(); it!=drumEditor->parts()->end(); it++)
          temp.tracks.insert(it->second->track());

        for (int i=0;i<DRUM_MAPSIZE;i++)
        {
          temp.pitch=i;
          instrument_map.append(temp);
        }
      }
      else
      {
        if (debugMsg) printf("DrumCanvas in new style drummap mode\n");
        ourDrumMap=NULL;
        rebuildOurDrumMap();
      }
      
      
      
      setVirt(false);
      cursorPos= QPoint(0,0);
      _stepSize=1;
      
      steprec=new MusECore::StepRec(NULL);
      
      songChanged(SC_TRACK_INSERTED);
      connect(MusEGlobal::song, SIGNAL(midiNote(int, int)), SLOT(midiNote(int,int)));
      }

DrumCanvas::~DrumCanvas()
{
  //items.clearDelete();
  
  if (must_delete_our_drum_map && ourDrumMap!=NULL)
    delete [] ourDrumMap;
  
  delete steprec;
}

//---------------------------------------------------------
//   moveCanvasItems
//---------------------------------------------------------

MusECore::Undo DrumCanvas::moveCanvasItems(CItemList& items, int dp, int dx, DragType dtype)
{      
  if(editor->parts()->empty())
    return MusECore::Undo(); //return empty list
  
  MusECore::PartsToChangeMap parts2change;
  MusECore::Undo operations;  
  
  for(MusECore::iPart ip = editor->parts()->begin(); ip != editor->parts()->end(); ++ip)
  {
    MusECore::Part* part = ip->second;
    if(!part)
      continue;
    
    int npartoffset = 0;
    for(iCItem ici = items.begin(); ici != items.end(); ++ici) 
    {
      CItem* ci = ici->second;
      if(ci->part() != part)
        continue;
      
      int x = ci->pos().x() + dx;
      int y = pitch2y(y2pitch(ci->pos().y()) + dp);
      QPoint newpos = raster(QPoint(x, y));
      
      // Test moving the item...
      DEvent* nevent = (DEvent*) ci;
      MusECore::Event event    = nevent->event();
      x              = newpos.x();
      if(x < 0)
        x = 0;
      int ntick = editor->rasterVal(x) - part->tick();
      if(ntick < 0)
        ntick = 0;
      int diff = ntick + event.lenTick() - part->lenTick();
      
      // If moving the item would require a new part size...
      if(diff > npartoffset)
        npartoffset = diff;
    }
        
    if(npartoffset > 0)
    {    
      MusECore::iPartToChange ip2c = parts2change.find(part);
      if(ip2c == parts2change.end())
      {
        MusECore::PartToChange p2c = {0, npartoffset};
        parts2change.insert(std::pair<MusECore::Part*, MusECore::PartToChange> (part, p2c));
      }
      else
        ip2c->second.xdiff = npartoffset;
    }
  }
  
  bool forbidden=false;
  for(MusECore::iPartToChange ip2c = parts2change.begin(); ip2c != parts2change.end(); ++ip2c)
  {
    MusECore::Part* opart = ip2c->first;
    if (opart->hasHiddenEvents())
    {
			forbidden=true;
			break;
		}
  }    

	
	if (!forbidden)
	{
		std::vector< CItem* > doneList;
		typedef std::vector< CItem* >::iterator iDoneList;
		
		for(iCItem ici = items.begin(); ici != items.end(); ++ici) 
		{
			CItem* ci = ici->second;
			
			int x = ci->pos().x();
			int y = ci->pos().y();
			int nx = x + dx;
			int ny = pitch2y(y2pitch(y) + dp);
			QPoint newpos = raster(QPoint(nx, ny));
			selectItem(ci, true);
			
			iDoneList idl;
			for(idl = doneList.begin(); idl != doneList.end(); ++idl)
				// This compares EventBase pointers to see if they're the same...
				if((*idl)->event() == ci->event())
					break;
				
			// Do not process if the event has already been processed (meaning it's an event in a clone part)...
			if (idl == doneList.end())
			{
				if (moveItem(operations, ci, newpos, dtype) == false) //error?
        {
          QMessageBox::warning(this, tr("Moving items failed"), tr("The selection couldn't be moved, because at least one note would be moved into a track which is different from both the original track and the current part's track.\nChanging the current part with ALT+LEFT/RIGHT may help."));
          return MusECore::Undo(); //return empty list
        }
				doneList.push_back(ci);
			}
			ci->move(newpos);
      
			if(moving.size() == 1) 
						itemReleased(curItem, newpos);

			if(dtype == MOVE_COPY || dtype == MOVE_CLONE)
						selectItem(ci, false);
		}  

		for(MusECore::iPartToChange ip2c = parts2change.begin(); ip2c != parts2change.end(); ++ip2c)
		{
			MusECore::Part* opart = ip2c->first;
			int diff = ip2c->second.xdiff;
			
			schedule_resize_all_same_len_clone_parts(opart, opart->lenTick() + diff, operations); 
		}    
					
  	return operations;
  }
  else
  {
		return MusECore::Undo(); //return empty list
	}
}
      
//---------------------------------------------------------
//   moveItem
//---------------------------------------------------------

bool DrumCanvas::moveItem(MusECore::Undo& operations, CItem* item, const QPoint& pos, DragType dtype)
      {
      DEvent* nevent   = (DEvent*) item;
      
      MusECore::MidiPart* part   = (MusECore::MidiPart*)nevent->part();
      MusECore::MidiPart* dest_part   = part;
      
      int nheight       = y2pitch(pos.y());
      if (nheight<0) nheight=0;
      if (nheight>=getOurDrumMapSize()) nheight=getOurDrumMapSize()-1;
            
      if (!instrument_map[nheight].tracks.contains(dest_part->track()))
      {
        if (debugMsg)
          printf("trying to move an event into a different track. checking if curPart is set correctly...\n");
        
        if (!instrument_map[nheight].tracks.contains(curPart->track()))
        {
          printf ("ERROR: tried to move an event into a track which is different from both the initial part's and the curPart's track! ignoring this one...\n");
          return false;
        }
        else
          dest_part=(MusECore::MidiPart*)curPart;
      }

      MusECore::Event event      = nevent->event();
      int x            = pos.x();
      if (x < 0)
            x = 0;
      int ntick        = editor->rasterVal(x) - dest_part->tick();
      if (ntick < 0)
            ntick = 0;
      MusECore::Event newEvent   = event.clone();

      
      int ev_pitch = instrument_map[nheight].pitch;
      
      newEvent.setPitch(ev_pitch);
      newEvent.setTick(ntick);

      // Added by T356, removed by flo93: with operation groups, it happens that the
      // part is too short right now, even if it's queued for being extended
      //if(((int)newEvent.endTick() - (int)part->lenTick()) > 0)  
      //  printf("DrumCanvas::moveItem Error! New event end:%d exceeds length:%d of part:%s\n", newEvent.endTick(), part->lenTick(), part->name().toLatin1().constData());
      
      if (dtype == MOVE_COPY || dtype == MOVE_CLONE)
            operations.push_back(MusECore::UndoOp(MusECore::UndoOp::AddEvent, newEvent, dest_part, false, false));
      else
      {
            if (dest_part == part)
                operations.push_back(MusECore::UndoOp(MusECore::UndoOp::ModifyEvent, newEvent, event, part, false, false));
            else
            {
                operations.push_back(MusECore::UndoOp(MusECore::UndoOp::DeleteEvent, event, part, false, false));
                operations.push_back(MusECore::UndoOp(MusECore::UndoOp::AddEvent, newEvent, dest_part, false, false));
            }
      }
      return true;
}

//---------------------------------------------------------
//   newItem
//---------------------------------------------------------

CItem* DrumCanvas::newItem(const QPoint& p, int state)
      {
      int instr = y2pitch(p.y());         //MusEGlobal::drumInmap[y2pitch(p.y())];
      if ((instr<0) || (instr>=getOurDrumMapSize()))
        return NULL;
      
      int velo  = ourDrumMap[instr].lv4;
      if (state == Qt::ShiftModifier)
            velo = ourDrumMap[instr].lv3;
      else if (state == Qt::ControlModifier)
            velo = ourDrumMap[instr].lv2;
      else if (state == (Qt::ControlModifier | Qt::ShiftModifier))
            velo = ourDrumMap[instr].lv1;
      int tick = editor->rasterVal(p.x());
      return newItem(tick, instr, velo);
      }

//---------------------------------------------------------
//   newItem
//---------------------------------------------------------

CItem* DrumCanvas::newItem(int tick, int instrument, int velocity)
{
  if ((instrument<0) || (instrument>=getOurDrumMapSize()))
    return NULL;

  if (!old_style_drummap_mode && !instrument_map[instrument].tracks.contains(curPart->track()))
  {
    if (debugMsg)
      printf("tried to create a new Item which cannot be inside the current track. looking for destination part...\n");
    
    QSet<MusECore::Part*> parts = parts_at_tick(tick, instrument_map[instrument].tracks);
    
    if (parts.count() != 1)
    {
      QMessageBox::warning(this, tr("Creating event failed"), tr("Couldn't create the event, because the currently selected part isn't the same track, and the selected instrument could be either on no or on multiple parts, which is ambiguous.\nSelect the destination part, then try again."));
      return NULL;
    }
    else
    {
      setCurrentPart(*parts.begin());
    }
  }
  // else or if we found an alternative part (which has now been set as curPart)

  tick    -= curPart->tick();
  MusECore::Event e(MusECore::Note);
  e.setTick(tick);
  e.setPitch(instrument_map[instrument].pitch);
  e.setVelo(velocity);
  e.setLenTick(ourDrumMap[instrument].len);

  return new DEvent(e, curPart, instrument);
}

//---------------------------------------------------------
//   resizeItem
//---------------------------------------------------------

void DrumCanvas::resizeItem(CItem* item, bool, bool)
      {
      DEvent* nevent = (DEvent*) item;
      MusECore::Event ev = nevent->event();
      // Indicate do undo, and do not do port controller values and clone parts. 
      MusEGlobal::audio->msgDeleteEvent(ev, nevent->part(), true, false, false);
      }

//---------------------------------------------------------
//   newItem
//---------------------------------------------------------
void DrumCanvas::newItem(CItem* item, bool noSnap) {
     newItem(item, noSnap,false);
}

void DrumCanvas::newItem(CItem* item, bool noSnap, bool replace)
{
   if (item)
   {
      DEvent* nevent = (DEvent*) item;
      MusECore::Event event    = nevent->event();
      int x = item->x();
      if (!noSnap)
            x = editor->rasterVal(x);
      event.setTick(x - nevent->part()->tick());
      int npitch = event.pitch();
      //event.setPitch(npitch); // commented out by flo: has no effect

      //
      // check for existing event
      //    if found change command semantic from insert to delete
      //
      MusECore::EventList* el = nevent->part()->events();
      MusECore::iEvent lower  = el->lower_bound(event.tick());
      MusECore::iEvent upper  = el->upper_bound(event.tick());

      for (MusECore::iEvent i = lower; i != upper; ++i) {
            MusECore::Event ev = i->second;
            // Added by T356. Only do notes.
            if(!ev.isNote())
              continue;
              
            if (ev.pitch() == npitch) {
                  // Indicate do undo, and do not do port controller values and clone parts. 
                  MusEGlobal::audio->msgDeleteEvent(ev, nevent->part(), true, false, false);
                  if (replace)
                    break;
                  else
                    return;

              }
            }

      // Added by T356.
      MusECore::Part* part = nevent->part();
      MusECore::Undo operations;
      int diff = event.endTick()-part->lenTick();
      
      if (! ((diff > 0) && part->hasHiddenEvents()) ) //operation is allowed
      {
        operations.push_back(MusECore::UndoOp(MusECore::UndoOp::AddEvent,event, part, false, false));
        
        if (diff > 0) // part must be extended?
        {
              schedule_resize_all_same_len_clone_parts(part, event.endTick(), operations);
              printf("newItem: extending\n");
        }
      }
      //else forbid action by not applying it
      MusEGlobal::song->applyOperationGroup(operations);
      songChanged(SC_EVENT_INSERTED); //this forces an update of the itemlist, which is neccessary
                                      //to remove "forbidden" events from the list again
   }
   else
    printf("THIS SHOULD NEVER HAPPEN: DrumCanvas::newItem called with NULL item!\n");
}

//---------------------------------------------------------
//   deleteItem
//---------------------------------------------------------

bool DrumCanvas::deleteItem(CItem* item)
      {
      MusECore::Event ev = ((DEvent*)item)->event();
      // Indicate do undo, and do not do port controller values and clone parts. 
      MusEGlobal::audio->msgDeleteEvent(ev, ((DEvent*)item)->part(), true, false, false);
      return false;
      }

//---------------------------------------------------------
//   drawItem
//---------------------------------------------------------

void DrumCanvas::drawItem(QPainter&p, const CItem*item, const QRect& rect)
      {
      DEvent* e   = (DEvent*) item;
      int x = 0, y = 0;
        x = mapx(item->pos().x());
        y = mapy(item->pos().y());
      QPolygon pa(4);
      pa.setPoint(0, x - CARET2, y);
      pa.setPoint(1, x,          y - CARET2);
      pa.setPoint(2, x + CARET2, y);
      pa.setPoint(3, x,          y + CARET2);
      QRect r(pa.boundingRect());
      r = r.intersect(rect);
      if(!r.isValid())
        return;
      
      p.setPen(Qt::black);
      
      if (e->part() != curPart)
      {
            if(item->isMoving()) 
              p.setBrush(Qt::gray);
            else if(item->isSelected()) 
              p.setBrush(Qt::black);
            else  
              p.setBrush(Qt::lightGray);
      }      
      else if (item->isMoving()) {
              p.setBrush(Qt::gray);
            }
      else if (item->isSelected())
      {
            p.setBrush(Qt::black);
      }
      else
      {
            int velo    = e->event().velo();
            MusECore::DrumMap* dm = &ourDrumMap[y2pitch(y)]; //Get the drum item
            QColor color;
            if (velo < dm->lv1)
                  color.setRgb(240, 240, 255);
            else if (velo < dm->lv2)
                  color.setRgb(200, 200, 255);
            else if (velo < dm->lv3)
                  color.setRgb(170, 170, 255);
            else
                  color.setRgb(0, 0, 255);
            p.setBrush(color);
      }
            
      p.drawPolygon(pa);
      }

//---------------------------------------------------------
//   drawMoving
//    draws moving items
//---------------------------------------------------------

void DrumCanvas::drawMoving(QPainter& p, const CItem* item, const QRect& rect)
    {
      QPolygon pa(4);
      QPoint pt = map(item->mp());
      int x = pt.x();
      int y = pt.y();
      pa.setPoint(0, x-CARET2,  y + TH/2);
      pa.setPoint(1, x,         y + TH/2+CARET2);
      pa.setPoint(2, x+CARET2,  y + TH/2);
      pa.setPoint(3, x,         y + (TH-CARET)/2);
      QRect mr(pa.boundingRect());
      mr = mr.intersect(rect);
      if(!mr.isValid())
        return;
      p.setPen(Qt::black);
      p.setBrush(Qt::black);
      p.drawPolygon(pa);
    }

//---------------------------------------------------------
//   drawCanvas
//---------------------------------------------------------

void DrumCanvas::drawCanvas(QPainter& p, const QRect& rect)
      {
      int x = rect.x();
      int y = rect.y();
      int w = rect.width();
      int h = rect.height();

      //---------------------------------------------------
      //  horizontal lines
      //---------------------------------------------------

      int yy  = ((y-1) / TH) * TH + TH;
      for (; yy < y + h; yy += TH) {
            p.setPen(Qt::gray);
            p.drawLine(x, yy, x + w, yy);
            }

      //---------------------------------------------------
      // vertical lines
      //---------------------------------------------------

      drawTickRaster(p, x, y, w, h, editor->raster());
      }

//---------------------------------------------------------
//   drawTopItem
//---------------------------------------------------------
void DrumCanvas::drawTopItem(QPainter& p, const QRect&)
{
  // draw cursor
  if (_tool == CursorTool) {
    p.setPen(Qt::black);

    int y = mapy(TH * cursorPos.y());

    p.drawPixmap(mapx(cursorPos.x())-TH/2,y,TH,TH, *cursorIcon);
    // need to figure out a coordinate system for the cursor, complicated stuff.
  }

}
//---------------------------------------------------------
//   y2pitch
//---------------------------------------------------------

int DrumCanvas::y2pitch(int y) const
      {
      int pitch = y/TH;
      if (pitch >= instrument_map.size())
            pitch = instrument_map.size()-1;
      else if (pitch<0)
            pitch = 0;
      return pitch;
      }

//---------------------------------------------------------
//   pitch2y
//---------------------------------------------------------

int DrumCanvas::pitch2y(int pitch) const
      {
      return pitch * TH;
      }

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void DrumCanvas::cmd(int cmd)
      {
      switch (cmd) {
            case CMD_SELECT_ALL:     // select all
                  for (iCItem k = items.begin(); k != items.end(); ++k) {
                        if (!k->second->isSelected())
                              selectItem(k->second, true);
                        }
                  break;
            case CMD_SELECT_NONE:     // select none
                  deselectAll();
                  break;
            case CMD_SELECT_INVERT:     // invert selection
                  for (iCItem k = items.begin(); k != items.end(); ++k) {
                        selectItem(k->second, !k->second->isSelected());
                        }
                  break;
            case CMD_SELECT_ILOOP:     // select inside loop
                  for (iCItem k = items.begin(); k != items.end(); ++k) {
                        DEvent* nevent =(DEvent*)(k->second);
                        MusECore::Part* part = nevent->part();
                        MusECore::Event event = nevent->event();
                        unsigned tick  = event.tick() + part->tick();
                        if (tick < MusEGlobal::song->lpos() || tick >= MusEGlobal::song->rpos())
                              selectItem(k->second, false);
                        else
                              selectItem(k->second, true);
                        }
                  break;
            case CMD_SELECT_OLOOP:     // select outside loop
                  for (iCItem k = items.begin(); k != items.end(); ++k) {
                        DEvent* nevent = (DEvent*)(k->second);
                        MusECore::Part* part     = nevent->part();
                        MusECore::Event event    = nevent->event();
                        unsigned tick  = event.tick() + part->tick();
                        if (tick < MusEGlobal::song->lpos() || tick >= MusEGlobal::song->rpos())
                              selectItem(k->second, true);
                        else
                              selectItem(k->second, false);
                        }
                  break;
            case CMD_SELECT_PREV_PART:     // select previous part
                  {
                      MusECore::Part* pt = editor->curCanvasPart();
                      MusECore::Part* newpt = pt;
                      MusECore::PartList* pl = editor->parts();
                      for(MusECore::iPart ip = pl->begin(); ip != pl->end(); ++ip)
                        if(ip->second == pt)
                        {
                          if(ip == pl->begin())
                            ip = pl->end();
                          --ip;
                          newpt = ip->second;
                          break;
                        }
                      if(newpt != pt)
                        editor->setCurCanvasPart(newpt);
                  }
                  break;
            case CMD_SELECT_NEXT_PART:     // select next part
                  {
                      MusECore::Part* pt = editor->curCanvasPart();
                      MusECore::Part* newpt = pt;
                      MusECore::PartList* pl = editor->parts();
                      for(MusECore::iPart ip = pl->begin(); ip != pl->end(); ++ip)
                        if(ip->second == pt)
                        {
                          ++ip;
                          if(ip == pl->end())
                            ip = pl->begin();
                          newpt = ip->second;
                          break;
                        }
                      if(newpt != pt)
                        editor->setCurCanvasPart(newpt);
                  }
                  break;

            case CMD_SAVE:
            case CMD_LOAD:
                  printf("DrumCanvas:: cmd not implemented %d\n", cmd);
                  break;

            case CMD_FIXED_LEN: //Set notes to the length specified in the drummap
                  if (!selectionSize())
                        break;
                  MusEGlobal::song->startUndo();
                  for (iCItem k = items.begin(); k != items.end(); ++k) {
                        if (k->second->isSelected()) {
                              DEvent* devent = (DEvent*)(k->second);
                              MusECore::Event event    = devent->event();
                              MusECore::Event newEvent = event.clone();
                              // newEvent.setLenTick(drumMap[event.pitch()].len);
                              newEvent.setLenTick(ourDrumMap[y2pitch(devent->y())].len);
                              // Indicate no undo, and do not do port controller values and clone parts. 
                              MusEGlobal::audio->msgChangeEvent(event, newEvent, devent->part(), false, false, false);
                              }
                        }
                  MusEGlobal::song->endUndo(SC_EVENT_MODIFIED);
                  break;
            case CMD_LEFT:
                  {
                      int spos = pos[0];
                      if(spos > 0)
                      {
                        spos -= 1;     // Nudge by -1, then snap down with raster1.
                        spos = AL::sigmap.raster1(spos, editor->rasterStep(pos[0]));
                      }
                      if(spos < 0)
                        spos = 0;
                      MusECore::Pos p(spos,true);
                      MusEGlobal::song->setPos(0, p, true, true, true);
                  }
                  break;
            case CMD_RIGHT:
                  {
                      int spos = AL::sigmap.raster2(pos[0] + 1, editor->rasterStep(pos[0]));    // Nudge by +1, then snap up with raster2.
                      MusECore::Pos p(spos,true);
                      MusEGlobal::song->setPos(0, p, true, true, true);
                  }
                  break;
            case CMD_LEFT_NOSNAP:
                  {
                    printf("left no snap\n");
                  int spos = pos[0] - editor->rasterStep(pos[0]);
                  if (spos < 0)
                        spos = 0;
                  MusECore::Pos p(spos,true);
                  MusEGlobal::song->setPos(0, p, true, true, true); //CDW
                  }
                  break;
            case CMD_RIGHT_NOSNAP:
                  {
                  MusECore::Pos p(pos[0] + editor->rasterStep(pos[0]), true);
                  MusEGlobal::song->setPos(0, p, true, true, true); //CDW
                  }
                  break;
            }
      updateSelection();
      redraw();
      }


//---------------------------------------------------------
//   startDrag
//---------------------------------------------------------

void DrumCanvas::startDrag(CItem* /* item*/, bool copymode)
      {
      QMimeData* md = selected_events_to_mime(partlist_to_set(editor->parts()), 1);
      
      if (md) {
            // "Note that setMimeData() assigns ownership of the QMimeData object to the QDrag object. 
            //  The QDrag must be constructed on the heap with a parent QWidget to ensure that Qt can 
            //  clean up after the drag and drop operation has been completed. "
            QDrag* drag = new QDrag(this);
            drag->setMimeData(md);
            
            if (copymode)
                  drag->exec(Qt::CopyAction);
            else
                  drag->exec(Qt::MoveAction);
            }
      }

//---------------------------------------------------------
//   dragEnterEvent
//---------------------------------------------------------

void DrumCanvas::dragEnterEvent(QDragEnterEvent* event)
      {
      event->acceptProposedAction();  // TODO CHECK Tim.
      }

//---------------------------------------------------------
//   dragMoveEvent
//---------------------------------------------------------

void DrumCanvas::dragMoveEvent(QDragMoveEvent*)
      {
      }

//---------------------------------------------------------
//   dragLeaveEvent
//---------------------------------------------------------

void DrumCanvas::dragLeaveEvent(QDragLeaveEvent*)
      {
      }


//---------------------------------------------------------
//   keyPressed - called from DList
//---------------------------------------------------------

void DrumCanvas::keyPressed(int index, int velocity)
      {
      using MusECore::MidiTrack;
      
      if ((index<0) || (index>=getOurDrumMapSize()))
        return;
      
      // called from DList - play event
      int port = old_style_drummap_mode ? ourDrumMap[index].port : dynamic_cast<MidiTrack*>(*instrument_map[index].tracks.begin())->outPort();
      int channel = old_style_drummap_mode ? ourDrumMap[index].channel : dynamic_cast<MidiTrack*>(*instrument_map[index].tracks.begin())->outChannel();
      int pitch = old_style_drummap_mode ? ourDrumMap[index].anote : instrument_map[index].pitch;

      // play note:
      MusECore::MidiPlayEvent e(0, port, channel, 0x90, pitch, velocity);
      MusEGlobal::audio->msgPlayMidiEvent(&e);
      
      
      if (_steprec) /* && pos[0] >= start_tick && pos[0] < end_tick [removed by flo93: this is handled in steprec->record] */
      {
        if ( curPart && instrument_map[index].tracks.contains(curPart->track()) )
            steprec->record(curPart,instrument_map[index].pitch,ourDrumMap[index].len,editor->raster(),velocity,MusEGlobal::globalKeyState&Qt::ControlModifier,MusEGlobal::globalKeyState&Qt::ShiftModifier, -1 /* invalid pitch as "really played" -> the "insert rest" feature is never triggered */);
        else
        {
          QSet<MusECore::Part*> parts = parts_at_tick(pos[0], instrument_map[index].tracks);
          
          if (parts.count() != 1)
            QMessageBox::warning(this, tr("Recording event failed"), tr("Couldn't record the event, because the currently selected part isn't the same track, and the instrument to be recorded could be either on no or on multiple parts, which is ambiguous.\nSelect the destination part, then try again."));
          else
            steprec->record(*parts.begin(), instrument_map[index].pitch,ourDrumMap[index].len,editor->raster(),velocity,MusEGlobal::globalKeyState&Qt::ControlModifier,MusEGlobal::globalKeyState&Qt::ShiftModifier, -1 /* invalid pitch as "really played" -> the "insert rest" feature is never triggered */);
          
        }
      }            
}

//---------------------------------------------------------
//   keyReleased
//---------------------------------------------------------

void DrumCanvas::keyReleased(int index, bool)
      {
      using MusECore::MidiTrack;
      
      if ((index<0) || (index>=getOurDrumMapSize()))
        return;
      
      // called from DList - silence playing event
      int port = old_style_drummap_mode ? ourDrumMap[index].port : dynamic_cast<MidiTrack*>(*instrument_map[index].tracks.begin())->outPort();
      int channel = old_style_drummap_mode ? ourDrumMap[index].channel : dynamic_cast<MidiTrack*>(*instrument_map[index].tracks.begin())->outChannel();
      int pitch = old_style_drummap_mode ? ourDrumMap[index].anote : instrument_map[index].pitch;

      // release note:
      MusECore::MidiPlayEvent e(0, port, channel, 0x90, pitch, 0);
      MusEGlobal::audio->msgPlayMidiEvent(&e);
      }

//---------------------------------------------------------
//   mapChanged
//---------------------------------------------------------

void DrumCanvas::mapChanged(int spitch, int dpitch)
{
   // spitch may be the same as dpitch! and something in here must be executed
   // even if they're same (i assume it's song->update(SC_DRUMMAP)) (flo93)
   
   if (old_style_drummap_mode)
   {
      MusECore::Undo operations;
      std::vector< std::pair<MusECore::Part*, MusECore::Event*> > delete_events;
      std::vector< std::pair<MusECore::Part*, MusECore::Event> > add_events;
      
      typedef std::vector< std::pair<MusECore::Part*, MusECore::Event*> >::iterator idel_ev;
      typedef std::vector< std::pair<MusECore::Part*, MusECore::Event> >::iterator iadd_ev;
      
      MusECore::MidiTrackList* tracks = MusEGlobal::song->midis();
      for (MusECore::ciMidiTrack t = tracks->begin(); t != tracks->end(); t++) {
            MusECore::MidiTrack* curTrack = *t;
            if (curTrack->type() != MusECore::Track::DRUM)
                  continue;

            MusECore::MidiPort* mp = &MusEGlobal::midiPorts[curTrack->outPort()];
            MusECore::PartList* parts= curTrack->parts();
            for (MusECore::iPart part = parts->begin(); part != parts->end(); ++part) {
                  MusECore::EventList* events = part->second->events();
                  MusECore::Part* thePart = part->second;
                  for (MusECore::iEvent i = events->begin(); i != events->end(); ++i) {
                        MusECore::Event event = i->second;
                        if(event.type() != MusECore::Controller && event.type() != MusECore::Note)
                          continue;
                        int pitch = event.pitch();
                        bool drc = false;
                        // Is it a drum controller event, according to the track port's instrument?
                        if(event.type() == MusECore::Controller && mp->drumController(event.dataA()))
                        {
                          drc = true;
                          pitch = event.dataA() & 0x7f;
                        }
                        
                        if (pitch == spitch) {
                              MusECore::Event* spitch_event = &(i->second);
                              delete_events.push_back(std::pair<MusECore::Part*, MusECore::Event*>(thePart, spitch_event));
                              MusECore::Event newEvent = spitch_event->clone();
                              if(drc)
                                newEvent.setA((newEvent.dataA() & ~0xff) | dpitch);
                              else
                                newEvent.setPitch(dpitch);
                              add_events.push_back(std::pair<MusECore::Part*, MusECore::Event>(thePart, newEvent));
                              }
                        else if (pitch == dpitch) {
                              MusECore::Event* dpitch_event = &(i->second);
                              delete_events.push_back(std::pair<MusECore::Part*, MusECore::Event*>(thePart, dpitch_event));
                              MusECore::Event newEvent = dpitch_event->clone();
                              if(drc)
                                newEvent.setA((newEvent.dataA() & ~0xff) | spitch);
                              else
                                newEvent.setPitch(spitch);
                              add_events.push_back(std::pair<MusECore::Part*, MusECore::Event>(thePart, newEvent));
                              }
                        }
                  }
            }

      for (idel_ev i = delete_events.begin(); i != delete_events.end(); i++) {
            MusECore::Part*  thePart = (*i).first;
            MusECore::Event* theEvent = (*i).second;
            operations.push_back(MusECore::UndoOp(MusECore::UndoOp::DeleteEvent, *theEvent, thePart, true, false));
            }

      MusECore::DrumMap dm = MusEGlobal::drumMap[spitch];
      MusEGlobal::drumMap[spitch] = MusEGlobal::drumMap[dpitch];
      MusEGlobal::drumMap[dpitch] = dm;
      MusEGlobal::drumInmap[int(MusEGlobal::drumMap[spitch].enote)]  = spitch;
      MusEGlobal::drumOutmap[int(MusEGlobal::drumMap[int(spitch)].anote)] = spitch;
      MusEGlobal::drumInmap[int(MusEGlobal::drumMap[int(dpitch)].enote)]  = dpitch;
      MusEGlobal::drumOutmap[int(MusEGlobal::drumMap[int(dpitch)].anote)] = dpitch;
            
      for (iadd_ev i = add_events.begin(); i != add_events.end(); i++) {
            MusECore::Part*  thePart = (*i).first;
            MusECore::Event& theEvent = (*i).second;
            operations.push_back(MusECore::UndoOp(MusECore::UndoOp::AddEvent, theEvent, thePart, true, false));
            }
      
      MusEGlobal::song->applyOperationGroup(operations, false); // do not indicate undo
      MusEGlobal::song->update(SC_DRUMMAP); //this update is neccessary, as it's not handled by applyOperationGroup()
   }
   else // if (!old_style_drummap_mode)
   {
      if (dpitch!=spitch)
      {
        using MusEGlobal::global_drum_ordering_t;
        using MusEGlobal::global_drum_ordering;
        
        MusECore::DrumMap dm_temp = ourDrumMap[spitch];
        instrument_number_mapping_t im_temp = instrument_map[spitch];

        global_drum_ordering_t order_temp;
        for (global_drum_ordering_t::iterator it=global_drum_ordering.begin(); it!=global_drum_ordering.end();)
        {
          if (im_temp.pitch==it->second && im_temp.tracks.contains(it->first))
          {
            order_temp.push_back(*it);
            it=global_drum_ordering.erase(it);
          }
          else
            it++;
        }
        
        // the instrument represented by instrument_map[dpitch] is always the instrument
        // which will be immediately AFTER our dragged instrument. or it's invalid
        if (dpitch < getOurDrumMapSize())
        {
          for (global_drum_ordering_t::iterator it=global_drum_ordering.begin(); it!=global_drum_ordering.end(); it++)
            if (instrument_map[dpitch].pitch==it->second && instrument_map[dpitch].tracks.contains(it->first))
            {
              while (!order_temp.empty())
                it=global_drum_ordering.insert(it, order_temp.takeLast());

              break;
            }
        }
        else
        {
          global_drum_ordering_t::iterator it=global_drum_ordering.end();
          while (!order_temp.empty())
            it=global_drum_ordering.insert(it, order_temp.takeLast());
        }





        if (dpitch > spitch)
        {
          for (int i=spitch; i<dpitch-1; i++)
          {
            ourDrumMap[i]=ourDrumMap[i+1];
            instrument_map[i]=instrument_map[i+1];
          }
          
          ourDrumMap[dpitch-1] = dm_temp;
          instrument_map[dpitch-1] = im_temp;
        }
        else if (spitch > dpitch)
        {
          for (int i=spitch; i>dpitch; i--)
          {
            ourDrumMap[i]=ourDrumMap[i-1];
            instrument_map[i]=instrument_map[i-1];
          }
          
          ourDrumMap[dpitch] = dm_temp;
          instrument_map[dpitch] = im_temp;
        }
      }
      
      
      MusEGlobal::song->update(SC_DRUMMAP); // this causes a complete rebuild of ourDrumMap
                                            // which also handles the changed order in all
                                            // other drum editors
   }
}

//---------------------------------------------------------
//   resizeEvent
//---------------------------------------------------------

void DrumCanvas::resizeEvent(QResizeEvent* ev)
      {
      if (ev->size().width() != ev->oldSize().width())
            emit newWidth(ev->size().width());
      EventCanvas::resizeEvent(ev);
      }


//---------------------------------------------------------
//   modifySelected
//---------------------------------------------------------

void DrumCanvas::modifySelected(NoteInfo::ValType type, int delta)
      {
      MusEGlobal::audio->msgIdle(true);
      MusEGlobal::song->startUndo();
      for (iCItem i = items.begin(); i != items.end(); ++i) {
            if (!(i->second->isSelected()))
                  continue;
            DEvent* e   = (DEvent*)(i->second);
            MusECore::Event event = e->event();
            if (event.type() != MusECore::Note)
                  continue;

            MusECore::MidiPart* part = (MusECore::MidiPart*)(e->part());
            MusECore::Event newEvent = event.clone();

            switch (type) {
                  case NoteInfo::VAL_TIME:
                        {
                        int newTime = event.tick() + delta;
                        if (newTime < 0)
                           newTime = 0;
                        newEvent.setTick(newTime);
                        }
                        break;
                  case NoteInfo::VAL_LEN:
                        printf("DrumCanvas::modifySelected - NoteInfo::VAL_LEN not implemented\n");
                        break;
                  case NoteInfo::VAL_VELON:
                        printf("DrumCanvas::modifySelected - NoteInfo::VAL_VELON not implemented\n");
                        break;
                  case NoteInfo::VAL_VELOFF:
                        printf("DrumCanvas::modifySelected - NoteInfo::VAL_VELOFF not implemented\n");
                        break;
                  case NoteInfo::VAL_PITCH:
                        if (old_style_drummap_mode)
                        {
                        int pitch = event.pitch() - delta; // Reversing order since the drumlist is displayed in increasing order
                        if (pitch > 127)
                              pitch = 127;
                        else if (pitch < 0)
                              pitch = 0;
                        newEvent.setPitch(pitch);
                        }
                        else
                          printf("DrumCanvas::modifySelected - MusEWidget::NoteInfo::VAL_PITCH not implemented for new style drum editors\n");
                        break;
                  }
            MusEGlobal::song->changeEvent(event, newEvent, part);
            // Indicate do not do port controller values and clone parts. 
            MusEGlobal::song->addUndo(MusECore::UndoOp(MusECore::UndoOp::ModifyEvent, newEvent, event, part, false, false));
            }
      MusEGlobal::song->endUndo(SC_EVENT_MODIFIED);
      MusEGlobal::audio->msgIdle(false);
      }

//---------------------------------------------------------
//   curPartChanged
//---------------------------------------------------------

void DrumCanvas::curPartChanged()
      {
      EventCanvas::curPartChanged();
      editor->setWindowTitle(getCaption());
      }

//---------------------------------------------------------
//   getNextStep - gets next tick in the chosen direction
//                 when raster and stepSize are taken into account
//---------------------------------------------------------
int DrumCanvas::getNextStep(unsigned int pos, int basicStep, int stepSize)
{
  int newPos = pos;
  for (int i =0; i<stepSize;i++) {
    if (basicStep > 0) { // moving right
      newPos = AL::sigmap.raster2(newPos + basicStep, editor->rasterStep(newPos));    // Nudge by +1, then snap up with raster2.
      if (unsigned(newPos) > curPart->endTick()- editor->rasterStep(curPart->endTick()))
        newPos = curPart->tick();
    }
    else { // moving left
      newPos = AL::sigmap.raster1(newPos + basicStep, editor->rasterStep(newPos));    // Nudge by -1, then snap up with raster1.
      if (unsigned(newPos) < curPart->tick() ) {
        newPos = AL::sigmap.raster1(curPart->endTick()-1, editor->rasterStep(curPart->endTick()));
      }
    }
  }
  return newPos;
}

//---------------------------------------------------------
//   keyPress
//---------------------------------------------------------
void DrumCanvas::keyPress(QKeyEvent* event)
{
  if (_tool == CursorTool) {

    int key = event->key();
    if (((QInputEvent*)event)->modifiers() & Qt::ShiftModifier)
          key += Qt::SHIFT;
    if (((QInputEvent*)event)->modifiers() & Qt::AltModifier)
          key += Qt::ALT;
    if (((QInputEvent*)event)->modifiers() & Qt::ControlModifier)
          key+= Qt::CTRL;

    // Select items by key (PianoRoll & DrumEditor)
    if (key == shortcuts[SHRT_SEL_RIGHT].key) {
      cursorPos.setX(getNextStep(cursorPos.x(),1));

      selectCursorEvent(getEventAtCursorPos());
      if (mapx(cursorPos.x()) < 0 || mapx(cursorPos.x()) > width())
        emit followEvent(cursorPos.x());
      update();
      return;
    }
    else if (key == shortcuts[SHRT_SEL_LEFT].key) {
      cursorPos.setX(getNextStep(cursorPos.x(),-1));

      selectCursorEvent(getEventAtCursorPos());
      if (mapx(cursorPos.x()) < 0 || mapx(cursorPos.x()) > width())
        emit followEvent(cursorPos.x());
      update();
      return;
    }
    else if (key == shortcuts[SHRT_ADDNOTE_1].key) {
          newItem(newItem(cursorPos.x(), cursorPos.y(), ourDrumMap[cursorPos.y()].lv1),false,true);
          keyPressed(cursorPos.y(), ourDrumMap[cursorPos.y()].lv1);
          keyReleased(cursorPos.y(), false);
          cursorPos.setX(getNextStep(cursorPos.x(),1, _stepSize));
          selectCursorEvent(getEventAtCursorPos());
          if (mapx(cursorPos.x()) < 0 || mapx(cursorPos.x()) > width())
            emit followEvent(cursorPos.x());
          return;
    }
    else if (key == shortcuts[SHRT_ADDNOTE_2].key) {
          newItem(newItem(cursorPos.x(), cursorPos.y(), ourDrumMap[cursorPos.y()].lv2),false,true);
          keyPressed(cursorPos.y(), ourDrumMap[cursorPos.y()].lv2);
          keyReleased(cursorPos.y(), false);
          cursorPos.setX(getNextStep(cursorPos.x(),1, _stepSize));
          selectCursorEvent(getEventAtCursorPos());
          if (mapx(cursorPos.x()) < 0 || mapx(cursorPos.x()) > width())
            emit followEvent(cursorPos.x());
          return;
    }
    else if (key == shortcuts[SHRT_ADDNOTE_3].key) {
          newItem(newItem(cursorPos.x(), cursorPos.y(), ourDrumMap[cursorPos.y()].lv3),false,true);
          keyPressed(cursorPos.y(), ourDrumMap[cursorPos.y()].lv3);
          keyReleased(cursorPos.y(), false);
          cursorPos.setX(getNextStep(cursorPos.x(),1, _stepSize));
          selectCursorEvent(getEventAtCursorPos());
          if (mapx(cursorPos.x()) < 0 || mapx(cursorPos.x()) > width())
            emit followEvent(cursorPos.x());
          return;
    }
    else if (key == shortcuts[SHRT_ADDNOTE_4].key) {
          newItem(newItem(cursorPos.x(), cursorPos.y(), ourDrumMap[cursorPos.y()].lv4),false,true);
          keyPressed(cursorPos.y(), ourDrumMap[cursorPos.y()].lv4);
          keyReleased(cursorPos.y(), false);
          cursorPos.setX(getNextStep(cursorPos.x(),1, _stepSize));
          selectCursorEvent(getEventAtCursorPos());
          if (mapx(cursorPos.x()) < 0 || mapx(cursorPos.x()) > width())
            emit followEvent(cursorPos.x());
          return;
    }
  }
  EventCanvas::keyPress(event);
}


//---------------------------------------------------------
//   setTool2
//---------------------------------------------------------
void DrumCanvas::setTool2(int)
{
  if (_tool == CursorTool)
    deselectAll();
  if (unsigned(cursorPos.x()) < curPart->tick())
    cursorPos.setX(curPart->tick());
  update();
}
//---------------------------------------------------------
//   setCurDrumInstrument
//---------------------------------------------------------
void DrumCanvas::setCurDrumInstrument(int i)
{
  cursorPos.setY(i);
  update();
}

//---------------------------------------------------------
//   setStep
//---------------------------------------------------------
void DrumCanvas::setStep(int v)
{
  _stepSize=v;
}

//---------------------------------------------------------
//   getEventAtCursorPos
//---------------------------------------------------------
MusECore::Event *DrumCanvas::getEventAtCursorPos()
{
    if (_tool != CursorTool)
      return 0;
    if (instrument_map[cursorPos.y()].tracks.contains(curPart->track()))
    {
      MusECore::EventList* el = curPart->events();
      MusECore::iEvent lower  = el->lower_bound(cursorPos.x()-curPart->tick());
      MusECore::iEvent upper  = el->upper_bound(cursorPos.x()-curPart->tick());
      int curPitch = instrument_map[cursorPos.y()].pitch;
      for (MusECore::iEvent i = lower; i != upper; ++i) {
        MusECore::Event &ev = i->second;
        if (ev.isNote()  &&  ev.pitch() == curPitch)
          return &ev;
      }
    }
    // else or if the for loop didn't find anything
    return 0;
}
//---------------------------------------------------------
//   selectCursorEvent
//---------------------------------------------------------
void DrumCanvas::selectCursorEvent(MusECore::Event *ev)
{
  for (iCItem i = items.begin(); i != items.end(); ++i)
  {
        MusECore::Event e = i->second->event();

        if (ev && ev->tick() == e.tick() && ev->pitch() == e.pitch() && e.isNote())
          i->second->setSelected(true);
        else
          i->second->setSelected(false);

  }
  updateSelection();
}


void DrumCanvas::moveAwayUnused()
{
  if (!old_style_drummap_mode)
  {
    printf("THIS SHOULD NEVER HAPPEN: DrumCanvas::moveAwayUnused() cannot be used in new style mode\n");
    return;
  }
	
	QSet<int> used;
	for (iCItem it=items.begin(); it!=items.end(); it++)
	{
		const MusECore::Event& ev=it->second->event();
		
		if (ev.type()==MusECore::Note)
			used.insert(ev.pitch());
	}
	
	int count=0;
	for (QSet<int>::iterator it=used.begin(); it!=used.end();)
	{
		while ((*it != count) && (used.find(count)!=used.end())) count++;
		
		if (*it != count)
			mapChanged(*it, count);

		count++;
		
		used.erase(it++);
	}
}


//---------------------------------------------------------
//   midiNote
//---------------------------------------------------------
void DrumCanvas::midiNote(int pitch, int velo)
{
      using MusECore::Track;
      using MusECore::Part;
      
      if (debugMsg) printf("DrumCanvas::midiNote: pitch=%i, velo=%i\n", pitch, velo);

      if (_midiin && _steprec && !MusEGlobal::audio->isPlaying() && velo && !(MusEGlobal::globalKeyState & Qt::AltModifier) /* && pos[0] >= start_tick && pos[0] < end_tick [removed by flo93: this is handled in steprec->record()] */ )
      {
         if (pitch == MusEGlobal::rcSteprecNote) // skip the fancy code below, simply record a rest
         {
           if (curPart)
             steprec->record(curPart,0xdead,0xbeef,editor->raster(),velo,MusEGlobal::globalKeyState&Qt::ControlModifier,MusEGlobal::globalKeyState&Qt::ShiftModifier, pitch);
         }
         else
         {
            QSet<Track*> possible_dest_tracks;
            Part* rec_part=NULL;
            int rec_index=-1;

            int ourDrumMapSize=getOurDrumMapSize();
            int i;
            for (i=0;i<ourDrumMapSize;i++)
            {
              if ( instrument_map[i].tracks.contains(curPart->track()) && ourDrumMap[i].enote==pitch)
              {
                rec_part=curPart;
                rec_index=i;
                break;
              }
              else if (ourDrumMap[i].enote==pitch)
                possible_dest_tracks.unite(instrument_map[i].tracks);
            }
            
            if (rec_part == NULL) // if recording to curPart isn't possible
            {
              QSet<Part*> possible_dest_parts = parts_at_tick(pos[0], possible_dest_tracks);
              
              if (possible_dest_parts.count() != 1)
                QMessageBox::warning(this, tr("Recording event failed"), tr("Couldn't record the event, because the currently selected part isn't the same track, and the instrument to be recorded could be either on no or on multiple parts, which is ambiguous.\nSelect the destination part, then try again."));
              else
              {
                rec_part = *possible_dest_parts.begin();
                Track* dest_track=rec_part->track();
                
                for (i=0;i<ourDrumMapSize;i++)
                  if ( instrument_map[i].tracks.contains(dest_track) && ourDrumMap[i].enote==pitch)
                  {
                    rec_index=i;
                    break;
                  }
                
                if (rec_index==-1)
                {
                  printf("ERROR: THIS SHOULD NEVER HAPPEN: i found a destination part for step recording, but now i can't find the instrument any more in DrumCanvas::midiNote()?!\n");
                  QMessageBox::critical(this, tr("Internal error"), tr("Wtf, some nasty internal error which is actually impossible occurred. Check console output. Nothing recorded."));
                  rec_part=NULL;
                }
              }
            }
            
            if (rec_part != NULL)
              steprec->record(rec_part,instrument_map[rec_index].pitch,ourDrumMap[rec_index].len,editor->raster(),velo,MusEGlobal::globalKeyState&Qt::ControlModifier,MusEGlobal::globalKeyState&Qt::ShiftModifier, pitch);
         }   
      }
}


int DrumCanvas::pitch_and_track_to_instrument(int pitch, MusECore::Track* track)
{
  for (int i=0; i<instrument_map.size(); i++)
    if (instrument_map[i].tracks.contains(track) && instrument_map[i].pitch==pitch)
      return i;
  
  if (heavyDebugMsg) printf("DrumCanvas::pitch_and_track_to_instrument() called with invalid arguments.\n");
  return -1;
}

void DrumCanvas::propagate_drummap_change(int instr, bool update_druminmap)
{
  const QSet<MusECore::Track*>& tracks=instrument_map[instr].tracks;
  int index=instrument_map[instr].pitch;
  
  for (QSet<MusECore::Track*>::const_iterator it = tracks.begin(); it != tracks.end(); it++)
  {
    dynamic_cast<MusECore::MidiTrack*>(*it)->drummap()[index] = ourDrumMap[instr];
    if (update_druminmap)
      dynamic_cast<MusECore::MidiTrack*>(*it)->update_drum_in_map();
  }
}


void DrumCanvas::rebuildOurDrumMap()
{
  using MusECore::drummaps_almost_equal;
  using MusECore::Track;
  using MusECore::MidiTrack;
  using MusECore::TrackList;
  using MusECore::ciTrack;
  using MusECore::ciPart;
  using MusECore::DrumMap;
  using MusEGlobal::global_drum_ordering_t;
  using MusEGlobal::global_drum_ordering;
  
  
  if (!old_style_drummap_mode)
  {
    bool need_update = false;
    
    TrackList* tl=MusEGlobal::song->tracks();
    QList< QSet<Track*> > track_groups;
    QVector<instrument_number_mapping_t> old_instrument_map = instrument_map;
    
    instrument_map.clear();

    for (ciTrack track = tl->begin(); track!=tl->end(); track++)
    {
      ciPart p_it;
      for (p_it=drumEditor->parts()->begin(); p_it!=drumEditor->parts()->end(); p_it++)
        if (p_it->second->track() == *track)
          break;
      
      if (p_it!=drumEditor->parts()->end()) // if *track is represented by some part in this editor
      {
        bool inserted=false;
        
        switch (drumEditor->group_mode())
        {
          case DrumEdit::GROUP_SAME_CHANNEL:
            for (QList< QSet<Track*> >::iterator group=track_groups.begin(); group!=track_groups.end(); group++)
              if ( ((MidiTrack*)*group->begin())->outChannel() == ((MidiTrack*)*track)->outChannel()  &&
                   ((MidiTrack*)*group->begin())->outPort() == ((MidiTrack*)*track)->outPort()  &&
                   (drummaps_almost_equal(((MidiTrack*)*group->begin())->drummap(), ((MidiTrack*)*track)->drummap())) )
              {
                group->insert(*track);
                inserted=true;
                break;
              }
            break;
          
          case DrumEdit::GROUP_MAX:
            for (QList< QSet<Track*> >::iterator group=track_groups.begin(); group!=track_groups.end(); group++)
              if (drummaps_almost_equal(((MidiTrack*)*group->begin())->drummap(), ((MidiTrack*)*track)->drummap()))
              {
                group->insert(*track);
                inserted=true;
                break;
              }
            break;
          
          case DrumEdit::DONT_GROUP: 
            inserted=false;
            break;
          
          default:
            printf("THIS SHOULD NEVER HAPPEN: group_mode() is invalid!\n");
            inserted=false;
        }

        if (!inserted)
        {
          QSet<Track*> temp;
          temp.insert(*track);
          track_groups.push_back(temp);
        }
      }
    }

    // from now, we assume that every track_group's entry only groups tracks with identical
    // drum maps, but not necessarily identical hide-lists together.
    QList< std::pair<MidiTrack*,int> > ignore_order_entries;
    for (global_drum_ordering_t::iterator order_it=global_drum_ordering.begin(); order_it!=global_drum_ordering.end(); order_it++)
    {
      // if this entry should be ignored, ignore it.
      if (ignore_order_entries.contains(*order_it))
        continue;
      
      // look if we have order_it->first (the MidiTrack*) in any of our track groups
      QList< QSet<Track*> >::iterator group;
      for (group=track_groups.begin(); group!=track_groups.end(); group++)
        if (group->contains(order_it->first))
          break;
      
      if (group!=track_groups.end()) // we have
      {
        int pitch=order_it->second;
        
        bool mute=true;
        bool hidden=true;
        
        if (drumEditor->ignore_hide()) hidden=false;
        
        for (QSet<Track*>::iterator track=group->begin(); track!=group->end() && (mute || hidden); track++)
        {
          if (dynamic_cast<MidiTrack*>(*track)->drummap()[pitch].mute == false)
            mute=false;
          
          if (dynamic_cast<MidiTrack*>(*track)->drummap_hidden()[pitch] == false)
            hidden=false;
        }

        if (!hidden)
        {
          for (QSet<Track*>::iterator track=group->begin(); track!=group->end(); track++)
          {
            DrumMap* dm = &dynamic_cast<MidiTrack*>(*track)->drummap()[pitch];
            if (dm->mute != mute)
            {
              dm->mute=mute; 
              need_update = true;
            }
          }
          
          if (dynamic_cast<MidiTrack*>(*group->begin())->drummap()[pitch].anote != pitch)
            printf("THIS SHOULD NEVER HAPPEN: track's_drummap[pitch].anote (%i)!= pitch (%i) !!!\n",dynamic_cast<MidiTrack*>(*group->begin())->drummap()[pitch].anote,pitch);
          
          instrument_map.append(instrument_number_mapping_t(*group, pitch));
        }
        
        for (QSet<Track*>::iterator track=group->begin(); track!=group->end(); track++)
          ignore_order_entries.append(std::pair<MidiTrack*,int>(dynamic_cast<MidiTrack*>(*track), pitch));
      }
      // else ignore it
    }


    // maybe delete and then populate ourDrumMap
    
    if (must_delete_our_drum_map && ourDrumMap!=NULL)
      delete [] ourDrumMap;
    
    int size = instrument_map.size();
    ourDrumMap=new DrumMap[size];
    must_delete_our_drum_map=true;

    for (int i=0;i<size;i++)
      ourDrumMap[i] = dynamic_cast<MidiTrack*>(*instrument_map[i].tracks.begin())->drummap()[instrument_map[i].pitch];  
    
    if (instrument_map!=old_instrument_map)
    {
    if (debugMsg) printf("rebuilt drummap and instrument map, size is now %i\n",size);
    
      songChanged(SC_EVENT_INSERTED); // force an update of the itemlist
      emit ourDrumMapChanged(true);
    }
    else
      emit ourDrumMapChanged(false);
    
    if (need_update)
      MusEGlobal::song->update(SC_DRUMMAP, true); // i know, this causes a recursion, which possibly
                                                  // isn't the most elegant solution here. but it will
                                                  // never be an infinite recursion
  }
}

} // namespace MusEGui
