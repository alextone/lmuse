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

#ifndef __EVENT_H__
#define __EVENT_H__

#include "wave.h"   // wg. SndFile
#include "al/xml.h"
#include "al/pos.h"
#include "evdata.h"

using AL::Xml;
using AL::Pos;

enum EventType { Note, Controller, Sysex, PAfter, CAfter, Meta, Wave };
class EventBase;

//---------------------------------------------------------
//   Event
//---------------------------------------------------------

class Event {
      EventBase* ev;

   public:
      Event() { ev = 0; }
      Event(EventType t);
      Event(const Event& e);
      Event(EventBase* eb);
      virtual ~Event();

      bool empty() const;
      EventType type() const;
      QString eventTypeName() const;

      void setType(EventType t);
      Event& operator=(const Event& e);
      bool operator==(const Event& e) const;

      int getRefCount() const;
      bool selected() const;
      void setSelected(bool val);
      void move(int offset);

      void read(QDomNode);
      void write(AL::Xml& xml, const Pos& offset) const;
      void dump(int n = 0) const;
      Event clone() const;
      Event mid(unsigned a, unsigned b);

      bool isNote() const;
      bool isNoteOff() const;
      bool isNoteOff(const Event& e) const;
      int dataA() const;
      int pitch() const;
      void setA(int val);
      void setPitch(int val);
      int dataB() const;
      int velo() const;
      void setB(int val);
      void setVelo(int val);
      int dataC() const;
      int veloOff() const;
      void setC(int val);
      void setVeloOff(int val);

      const unsigned char* data() const;
      int dataLen() const;
      void setData(const unsigned char* data, int len);
      const EvData eventData() const;

      const QString name() const;
      void setName(const QString& s);
      int spos() const;
      void setSpos(int s);
      SndFileR sndFile() const;
      virtual void setSndFile(SndFileR& sf);
      virtual void read(unsigned offset, float** bpp, int channels, int nn);

      Pos pos() const;
      Pos end() const;

      void setTick(unsigned val);
      unsigned tick() const;
      unsigned frame() const;
      void setFrame(unsigned val);
      void setLenTick(unsigned val);
      void setLenFrame(unsigned val);
      unsigned lenTick() const;
      unsigned lenFrame() const;
      unsigned endTick() const;
      unsigned endFrame() const;
      void setPos(const Pos& p);
      };

typedef std::multimap<unsigned, Event, std::less<unsigned> > EL;
typedef EL::iterator iEvent;
typedef EL::reverse_iterator riEvent;
typedef EL::const_iterator ciEvent;
typedef std::pair <iEvent, iEvent> EventRange;

//---------------------------------------------------------
//   EventList
//    tick sorted list of events
//---------------------------------------------------------

class EventList : public EL {
   public:
      int cloneCount;
      EventList()           { cloneCount = 0; }
      iEvent find(const Event&);
      iEvent add(Event& event);
      iEvent add(Event& event, unsigned tick);
      void move(Event& event, unsigned tick);
      void dump() const;
      void read(QDomNode, bool midi);
      };

#endif

