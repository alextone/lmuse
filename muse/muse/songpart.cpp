//=============================================================================
//  MusE
//  Linux Music Editor
//  $Id:$
//
//  Copyright (C) 2002-2006 by Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "song.h"
#include "part.h"
#include "audio.h"

//---------------------------------------------------------
//   cmdAddPart
//    GUI context + startUndo/endUndo
//---------------------------------------------------------

void Song::cmdAddPart(Part* part)
      {
      Track* track = part->track();
      //
      // create default name:
      //
      for (int i = 1;;++i) {
            PartList* pl = track->parts();
            bool found = false;
            QString name = QString("Part-%1").arg(i);
            for (iPart ip = pl->begin(); ip != pl->end(); ++ip) {
                  if (name == ip->second->name()) {
                        found = true;
                        break;
                        }
                  }
            if (!found) {
                  part->setName(name);
                  break;
                  }
            }
      startUndo();
      addPart(part);
      endUndo(0);
      track->partListChanged();
      }

//---------------------------------------------------------
//   addPart
//	GUI context
//---------------------------------------------------------

void Song::addPart(Part* part)
      {
      AudioMsg msg;
      msg.id = SEQM_ADD_PART;
      msg.p1 = part;
      audio->sendMessage(&msg, false);
      undoOp(UndoOp::AddPart, part);
      updateFlags |= SC_PART_INSERTED;
      if (len() < part->endTick())
            setLen(part->endTick());
      }

//---------------------------------------------------------
//   cmdRemovePart
//---------------------------------------------------------

void Song::cmdRemovePart(Part* part)
      {
      startUndo();
      removePart(part);
      endUndo(0);
      part->track()->partListChanged();
      }

//---------------------------------------------------------
//   removePart
//---------------------------------------------------------

void Song::removePart(Part* part)
      {
      AudioMsg msg;
      msg.id = SEQM_REMOVE_PART;
      msg.p1 = part;
      audio->sendMessage(&msg, false);
      undoOp(UndoOp::DeletePart, part);
      part->events()->incARef(-1);
      updateFlags |= SC_PART_REMOVED;
      }

//---------------------------------------------------------
//   cmdRemoveParts
//    remove selected parts
//---------------------------------------------------------

void Song::cmdRemoveParts()
      {
      TrackList* tl = song->tracks();
      PartList pl;

      for (iTrack it = tl->begin(); it != tl->end(); ++it) {
      	PartList* pl2 = (*it)->parts();
            for (iPart ip = pl2->begin(); ip != pl2->end(); ++ip) {
            	if (ip->second->selected())
                  	pl.add(ip->second);
                  }
            }
      for (iPart ip = pl.begin(); ip != pl.end(); ++ip)
		removePart(ip->second);
      }

//---------------------------------------------------------
//   cmdChangePart
//	realtime context
//---------------------------------------------------------

void Song::cmdChangePart(Part* oldPart, Part* newPart)
      {
      changePart(oldPart, newPart);
      undoOp(UndoOp::ModifyPart, oldPart, newPart);
      oldPart->events()->incARef(-1);
      updateFlags = SC_PART_MODIFIED;
      }

//---------------------------------------------------------
//   changePart
//---------------------------------------------------------

void Song::changePart(Part* oldPart, Part* newPart)
      {
      Part part = *newPart;
      *newPart  = *oldPart;
      *oldPart  = part;
      }

//---------------------------------------------------------
//   changePart
//    extend/shrink part in front or at end
//---------------------------------------------------------

void Song::changePart(Part* oPart, unsigned pos, unsigned len)
      {
      startUndo();
      //
      // move events so they stay at same position in song
      //
      int delta    = oPart->tick() - pos;
      EventList* d = new EventList();
      EventList* s = oPart->events();
      for (iEvent ie = s->begin(); ie != s->end(); ++ie) {
            int tick = ie->first + delta;
            if (tick >= 0 && tick < int(len)) {
                  Event ev = ie->second.clone();
                  ev.move(delta);
                  d->add(ev, unsigned(tick));
                  }
            }
      if (oPart->fillLen() > 0 && len < (unsigned)oPart->fillLen())
            oPart->setFillLen(len);
      if (oPart->lenTick() < len && oPart->fillLen() > 0) {
            unsigned loop = oPart->fillLen();
            unsigned fillLen = len - oPart->lenTick();
            for (unsigned i = 0; i < fillLen / loop; ++i) {
                  int start = oPart->lenTick() + loop * i;
                  for (iEvent ie = s->begin(); ie != s->end(); ++ie) {
                        if (ie->first >= loop)
                              break;
                  	Event ev = ie->second.clone();
                        ev.move(start);
                        d->add(ev, ie->first + start);
                        }
                  }
            }
      Part* nPart = new Part(*oPart, d);
      nPart->setLenTick(len);
      nPart->setTick(pos);
      audio->msgChangePart(oPart, nPart, false);
      endUndo(SC_PART_MODIFIED);
      oPart->track()->partListChanged();
      if (unsigned(_len) < oPart->endTick())  // update song len
            setLen(oPart->endTick());
      }

//---------------------------------------------------------
//   movePart
//---------------------------------------------------------

void Song::movePart(Part* oPart, unsigned pos, Track* track)
      {
      Track* oTrack = oPart->track();
      Part* nPart   = new Part(*oPart);
      nPart->setTrack(track);
      nPart->setTick(pos);
      startUndo();
      if (oPart->track() != track) {
	      removePart(oPart);
	      addPart(nPart);
            }
      else {
	      audio->msgChangePart(oPart, nPart, false);
            }
      endUndo(0);
      oTrack->partListChanged();
      if (len() < nPart->endTick())
            setLen(nPart->endTick());
      }

//---------------------------------------------------------
//   linkPart
//---------------------------------------------------------

void Song::linkPart(Part* sPart, unsigned pos, Track* track)
      {
      Part* dPart = track->newPart(sPart, true);
      dPart->setTick(pos);
      cmdAddPart(dPart);
      sPart->track()->partListChanged();
      dPart->track()->partListChanged();
      }

//---------------------------------------------------------
//   copyPart
//---------------------------------------------------------

void Song::copyPart(Part* sPart, unsigned pos, Track* track)
      {
      bool clone = sPart->events()->arefCount() > 1;
      Part* dPart = track->newPart(sPart, clone);
      dPart->setTick(pos);
      if (!clone) {
            //
            // Copy Events
            //
            EventList* se = sPart->events();
            EventList* de = dPart->events();
            for (iEvent i = se->begin(); i != se->end(); ++i) {
                  Event oldEvent = i->second;
                  Event ev = oldEvent.clone();
                  de->add(ev);
                  }
            }
      cmdAddPart(dPart);
      sPart->track()->partListChanged();
      dPart->track()->partListChanged();
      }

//---------------------------------------------------------
//   createLRPart
//---------------------------------------------------------

void Song::createLRPart(Track* track)
      {
      Part* part = track->newPart();
      if (part) {
            part->setTick(pos[1].tick());
            part->setLenTick(pos[2].tick()-pos[1].tick());
            part->setSelected(true);
            cmdAddPart(part);
            }
      }

//---------------------------------------------------------
//   selectPart
//---------------------------------------------------------

void Song::selectPart(Part* part, bool add)
      {
      if (add) {
            part->setSelected(!part->selected());
            part->track()->partListChanged();
            return;
            }
      for (iTrack it = _tracks.begin(); it != _tracks.end(); ++it) {
            PartList* pl = (*it)->parts();
            bool changed = false;
            for (iPart ip = pl->begin(); ip != pl->end(); ++ip) {
                  bool f = part == ip->second;
                  if (ip->second->selected() != f) {
                        ip->second->setSelected(f);
                        changed = true;
                        }
                  }
            if (changed)
                  (*it)->partListChanged();
            }
      }

//---------------------------------------------------------
//   cmdSplitPart
//---------------------------------------------------------

void Song::cmdSplitPart(Part* part, const Pos& pos)
      {
      int tick = pos.tick();
      int l1 = tick - part->tick();
      int l2 = part->lenTick() - l1;
      if (l1 <= 0 || l2 <= 0)
            return;
      Part* p1;
      Part* p2;
      part->track()->splitPart(part, tick, p1, p2);

      startUndo();
      audio->msgChangePart(part, p1, false);
      addPart(p2);
      endUndo(SC_TRACK_MODIFIED | SC_PART_MODIFIED | SC_PART_INSERTED);
      part->track()->partListChanged();
      }

//---------------------------------------------------------
//   cmdGluePart
//---------------------------------------------------------

void Song::cmdGluePart(Part* oPart)
      {
      Track* track   = oPart->track();
      PartList* pl   = track->parts();
      Part* nextPart = 0;

      for (iPart ip = pl->begin(); ip != pl->end(); ++ip) {
            if (ip->second == oPart) {
                  ++ip;
                  if (ip == pl->end())
                        return;
                  nextPart = ip->second;
                  break;
                  }
            }

      Part* nPart = track->newPart(oPart);
      nPart->setLenTick(nextPart->tick() + nextPart->lenTick() - oPart->tick());

      // populate nPart with Events from oPart and nextPart

      EventList* sl1 = oPart->events();
      EventList* dl  = nPart->events();

      for (iEvent ie = sl1->begin(); ie != sl1->end(); ++ie)
            dl->add(ie->second);

      EventList* sl2 = nextPart->events();
      int tickOffset = nextPart->tick() - oPart->tick();

      for (iEvent ie = sl2->begin(); ie != sl2->end(); ++ie) {
            Event event = ie->second.clone();
            event.move(tickOffset);
            dl->add(event);
            }
      startUndo();
      removePart(nextPart);
      audio->msgChangePart(oPart, nPart, false);
      endUndo(SC_PART_MODIFIED | SC_PART_REMOVED);
      track->partListChanged();
      }


