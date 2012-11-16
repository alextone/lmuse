//=========================================================
//  MusE
//  Linux Music Editor
//  $Id: popupmenu.h,v 1.1.1.1 2010/07/18 03:18:00 terminator356 Exp $
//
//  (C) Copyright 1999-2010 Werner Schweer (ws@seh.de)
//
//  PopupMenu sub-class of QMenu created by Tim.
//  (C) Copyright 2010-2011 Tim E. Real (terminator356 A T sourceforge D O T net)
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

#ifndef __POPUPMENU_H__
#define __POPUPMENU_H__

// Just in case Qt ever adds these features natively, we would need to turn our features off!
//#define POPUP_MENU_DISABLE_STAY_OPEN    
//#define POPUP_MENU_DISABLE_AUTO_SCROLL

#include <QMenu>
#ifndef POPUP_MENU_DISABLE_AUTO_SCROLL  
  #include <QTimer>
#endif

//#include <QMouseEvent>
//#include <QColumnView>

class QWidget;
class QMouseEvent;
class QVariant;
class QAction;
class QEvent;

namespace MusEGui {

/** offers a QMenu-like menu, which stays open once the user
 *  clicked a checkable action. */
class PopupMenu : public QMenu
{
Q_OBJECT
  
    bool _stayOpen;
    #ifndef POPUP_MENU_DISABLE_AUTO_SCROLL  
    QTimer* timer;
    #endif
    int moveDelta;
    PopupMenu* _cur_menu; // For auto-breakup.
    int _cur_menu_count;
    int _cur_item_width;
    int _cur_col_count;

    void init();
    // Auto-breakup a too-wide menu.
    PopupMenu* getMenu();
    
  private slots:
    #ifndef POPUP_MENU_DISABLE_AUTO_SCROLL  
    void popHovered(QAction*);
    void timerHandler();
    #endif
  
  protected:
    void mouseReleaseEvent(QMouseEvent *);
    bool event(QEvent*);   
  
  public:
    PopupMenu(bool stayOpen);
    PopupMenu(QWidget* parent=0, bool stayOpen = false);
    PopupMenu(const QString& title, QWidget* parent = 0, bool stayOpen = false);
    //void clear();
    QAction* findActionFromData(const QVariant&) const;
    bool stayOpen() const { return _stayOpen; }
    void clearAllChecks() const;

    // Need to catch these to auto-breakup a too-big menu.
    QAction* addAction(const QString& text);
    QAction* addAction(const QIcon& icon, const QString& text);
    QAction* addAction(const QString& text, const QObject* receiver, const char* member, const QKeySequence& shortcut = 0);
    QAction* addAction(const QIcon& icon, const QString& text, const QObject* receiver, const char* member, const QKeySequence& shortcut = 0);
    void     addAction(QAction* action);
};


/*
class PopupView : public QColumnView
{
  Q_OBJECT
  private:  
    QStandardItemModel* _model;
    
  protected:
  
  public:
    PopupView(QWidget* parent=0);
    ~PopupView();
    
    void clear();
    QStandardItemModel* model() { return _model; }
};
*/

} // namespace MusEGui

#endif

