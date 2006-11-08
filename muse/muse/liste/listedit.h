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

#ifndef __LISTEDIT_H__
#define __LISTEDIT_H__

namespace AL {
      class Pos;
      };

class Track;
class Ctrl;
class CtrlListEditor;

//---------------------------------------------------------
//   ListEdit
//---------------------------------------------------------

class ListEdit : public QWidget {
      Q_OBJECT;

      QStackedWidget* stack;
      QTreeWidget* list;
      CtrlListEditor* ctrlPanel;

      void buildList();

   private slots:
      void itemChanged(QTreeWidgetItem*, QTreeWidgetItem*);
      void itemExpanded(QTreeWidgetItem*);

   public:
      ListEdit(QWidget* parent = 0);
      void selectItem(const AL::Pos&, Track*, Ctrl*);
      };

#endif

