//=============================================================================
//  MusE
//  Linux Music Editor
//  $Id:$
//
//  Copyright (C) 2006 by Werner Schweer and others
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

#include "projectdialog.h"
#include "gconfig.h"
#include "song.h"
#include "icons.h"

//
// entry types for projectTree tree widget:
//
enum { DIR_TYPE, PROJECT_TYPE };

//---------------------------------------------------------
//   ProjectTree
//---------------------------------------------------------

ProjectTree::ProjectTree(QWidget* parent)
   : QTreeWidget(parent)
      {
      setAcceptDrops(true);
      }

//---------------------------------------------------------
//   itemPath
//---------------------------------------------------------

QString ProjectTree::itemPath(QTreeWidgetItem* item) const
      {
      QString path;
      QTreeWidgetItem* ti = item;
      QStringList dirComponent;
      do {
            dirComponent.prepend(ti->text(0));
            ti = ti->parent();
            } while (ti);
      foreach (QString s, dirComponent) {
            if (!path.isEmpty())
                  path += "/";
            path += s;
            }
      return path;
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void ProjectTree::mousePressEvent(QMouseEvent* event)
      {
      dragStartPosition = event->pos(); 
      QTreeWidget::mousePressEvent(event);
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void ProjectTree::mouseMoveEvent(QMouseEvent* ev)
      {
      QTreeWidgetItem* item = itemAt(ev->pos());
      if (item == 0 || item->type() != PROJECT_TYPE) // || ev->button() != Qt::LeftButton)
            return;
      if ((ev->pos() - dragStartPosition).manhattanLength()
         < QApplication::startDragDistance())
            return;
      QDrag* drag = new QDrag(this);
      QMimeData* mimeData = new QMimeData;
      mimeData->setText(itemPath(item));
      drag->setMimeData(mimeData);
      drag->setPixmap(style()->standardPixmap(QStyle::SP_FileIcon));
      Qt::DropAction dropAction = drag->start(Qt::MoveAction);
      if (dropAction == Qt::IgnoreAction)
            return;
      printf("drop action\n");
      if (dropAction == Qt::MoveAction) {
            printf("move action\n");
            }
      }

//---------------------------------------------------------
//   dragEnterEvent
//---------------------------------------------------------

void ProjectTree::dragEnterEvent(QDragEnterEvent* event)
      {
      if (event->mimeData()->hasFormat("text/plain")) {
            event->acceptProposedAction();
            }
      }

//---------------------------------------------------------
//   dragMoveEvent
//---------------------------------------------------------

void ProjectTree::dragMoveEvent(QDragMoveEvent* event)
      {
      QTreeWidgetItem* item = itemAt(event->pos());
      event->setAccepted(item && item->type() == DIR_TYPE);
      }

//---------------------------------------------------------
//   searchItem
//---------------------------------------------------------

QTreeWidgetItem* ProjectTree::searchItem(const QString& s, QTreeWidgetItem* p)
      {
      int n = p->childCount();      
      for (int i = 0; i < n; ++i) {
            QTreeWidgetItem* item = p->child(i);
            if (s == itemPath(item))
                  return p->takeChild(i);
            item = searchItem(s, item);
            if (item)
                  return item;
            }
      return 0;
      }
            
//---------------------------------------------------------
//   dropEvent
//---------------------------------------------------------

void ProjectTree::dropEvent(QDropEvent* event)
      {
      QTreeWidgetItem* dstItem = itemAt(event->pos());
      if (dstItem && dstItem->type() == DIR_TYPE) {
            QString src = event->mimeData()->text();
            QTreeWidgetItem* srcItem;
            int n = topLevelItemCount();
            for (int i = 0; i < n; ++i) {
                  QTreeWidgetItem* item = topLevelItem(i);
                  if (src == itemPath(item)) {
                        srcItem = takeTopLevelItem(i);
                        break;
                        }
                  srcItem = searchItem(src, item);
                  if (srcItem)
                        break;
                  }
            if (srcItem == 0) {
                  printf("src item not found\n");
                  return;
                  }
            QString dst = itemPath(dstItem);
            dstItem->addChild(srcItem);
            //
            // TODO:    - actual move project
            //          - look for name conflicts
            //          - do nothing if src==dst
            //
            src = QDir::homePath() + "/" + config.projectPath + "/" + src;
            dst = QDir::homePath() + "/" + config.projectPath + "/" + dst 
               + "/" + srcItem->text(0);
            if (src != dst) {
                  QDir dir;
                  if (!dir.rename(src, dst)) {
printf("Rename <%s> -> <%s> failed\n", src.toLatin1().data(), dst.toLatin1().data());
                        event->acceptProposedAction();
                        }
                  }
            }
      }

//---------------------------------------------------------
//   processSubdirectories
//---------------------------------------------------------

void ProjectDialog::processSubdir(QTreeWidgetItem* item, const QString& p, 
   const QString& subdir, QTreeWidgetItem** current)
      {
      QString path(p + "/" + subdir); 
      QFile pf(path + "/" + subdir + ".med");
      if (pf.exists()) {
            QTreeWidgetItem* pi = new QTreeWidgetItem(item, PROJECT_TYPE);
            pi->setText(0, subdir);
            pi->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
            if (path == song->absoluteProjectPath())
                  *current = pi;
            }
      else {
            QDir sd(path);
            QStringList dl = sd.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
            QTreeWidgetItem* pi = new QTreeWidgetItem(item, DIR_TYPE);
            pi->setText(0, subdir);
            itemCollapsed(pi);
            foreach (QString s, dl)
                  processSubdir(pi, path, s, current);
            }
      }

//---------------------------------------------------------
//   ProjectDialog
//---------------------------------------------------------

ProjectDialog::ProjectDialog(QWidget* parent)
  : QDialog(parent)
      {
      setupUi(this);
      projectTree->setSelectionBehavior(QAbstractItemView::SelectRows);
      projectTree->setSelectionMode(QAbstractItemView::SingleSelection);
      QDir pd(QDir::homePath() + "/" + config.projectPath);
      QStringList el = pd.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
      QTreeWidgetItem* current = 0;
      foreach (QString s, el) {
            QString path(QDir::homePath() + "/" + config.projectPath + "/" + s);
            QFile pf(path + "/" + s + ".med");
            if (pf.exists()) {
                  QTreeWidgetItem* pi = new QTreeWidgetItem(projectTree, PROJECT_TYPE);
                  pi->setText(0, s);
                  pi->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
                  if (path == song->absoluteProjectPath())
                        current = pi;
                  }
            else {
                  QDir sd(path);
                  QStringList dl = sd.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
                  if (!dl.isEmpty()) {
                        QTreeWidgetItem* pi = new QTreeWidgetItem(projectTree, DIR_TYPE);
                        pi->setText(0, s);
                        itemCollapsed(pi);
                        foreach (QString s, dl) {
                              processSubdir(pi, path, s, &current);
                              }
                        }
                  }
            }
      connect(projectTree, 
         SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), 
         SLOT(currentChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
      connect(projectTree, 
         SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
         SLOT(itemDoubleClicked(QTreeWidgetItem*, int)));
      connect(projectTree,
         SIGNAL(itemCollapsed(QTreeWidgetItem*)),
         SLOT(itemCollapsed(QTreeWidgetItem*)));
      connect(projectTree,
         SIGNAL(itemExpanded(QTreeWidgetItem*)),
         SLOT(itemExpanded(QTreeWidgetItem*)));
      connect(projectNameEntry,
         SIGNAL(textEdited(const QString&)),
         SLOT(projectNameEdited(const QString&)));
      connect(newFolder, 
         SIGNAL(clicked()),
         SLOT(newFolderClicked()));
        
      if (current)
            projectTree->setCurrentItem(current); 
      else
            currentChanged(0, 0);
      }

//---------------------------------------------------------
//   itemCollapsed
//---------------------------------------------------------

void ProjectDialog::itemCollapsed(QTreeWidgetItem* item)
      {
      item->setIcon(0, style()->standardIcon(QStyle::SP_DirClosedIcon));
      }

//---------------------------------------------------------
//   itemExpanded
//---------------------------------------------------------

void ProjectDialog::itemExpanded(QTreeWidgetItem* item)
      {
      item->setIcon(0, style()->standardIcon(QStyle::SP_DirOpenIcon));
      }

//---------------------------------------------------------
//   currentChanged
//---------------------------------------------------------

void ProjectDialog::currentChanged(QTreeWidgetItem* item, QTreeWidgetItem*)
      {
      bool enable = (item != 0) && (item->type() == PROJECT_TYPE);
      createdDate->setEnabled(enable);
      modifiedDate->setEnabled(enable);
      comment->setEnabled(enable);
      length->setEnabled(enable);
      
      // newFolder->setEnabled(item == 0 || item->type() == DIR_TYPE);
      if (!enable)
            return;

      projectNameEntry->setText(item->text(0));
      QString pd(QDir::homePath() + "/" + config.projectPath + "/");

      pd += "/" + projectTree->itemPath(item);

      QFileInfo pf(pd + "/" + item->text(0) + ".med");
      modifiedDate->setDateTime(pf.lastModified());

      QTime time(0, 0, 0);
      QDateTime date;

      QFile f(pf.filePath());
      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(&f, false, &err, &line, &column)) {
            QString col, ln, error;
            col.setNum(column);
            ln.setNum(line);
            error = err + "\n    at line: " + ln + " col: " + col;
            printf("error reading med file: %s\n", error.toLatin1().data());
            return;
            }
      for (QDomNode node = doc.documentElement(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            if (e.tagName() == "muse") {
                  QString sversion = e.attribute("version", "1.0");
                  int major=0, minor=0;
                  sscanf(sversion.toLatin1().data(), "%d.%d", &major, &minor);
                  int version = major << 8 + minor;
                  if (version >= 0x200) {
                        for (QDomNode n1 = node.firstChild(); !n1.isNull(); n1 = n1.nextSibling()) {
                              QDomElement e = n1.toElement();
                              if (e.tagName() == "song") {
                                    for (QDomNode n2 = n1.firstChild(); !n2.isNull(); n2 = n2.nextSibling()) {
                                          QDomElement e = n2.toElement();
                                          QString tag(e.tagName());
                                          QString s(e.text());
                                          if (tag == "comment")
                                                comment->setPlainText(s);
                                          else if (tag == "createDate")
                                                date = QDateTime::fromString(e.text(), Qt::ISODate);
                                          else if (tag == "LenInSec") {
                                                int sec = s.toInt();
                                                time = time.addSecs(sec);
                                                break;
                                                }
                                          }
                                    }
                              }
                        }
                  break;
                  }
            }
      length->setTime(time);
      createdDate->setDateTime(date);
      }

//---------------------------------------------------------
//   projectNameEdited
//---------------------------------------------------------

void ProjectDialog::projectNameEdited(const QString&)
      {
      QTreeWidgetItem* item = projectTree->currentItem();
      if (item && item->type() == PROJECT_TYPE) {
            projectTree->setItemSelected(item, false);
            projectTree->setCurrentItem(0);
            }
      }

//---------------------------------------------------------
//   projectPath
//---------------------------------------------------------

QString ProjectDialog::projectPath() const
      {
      QTreeWidgetItem* item = projectTree->currentItem();
      QString s;
      if (item) {
            if (item->type() == PROJECT_TYPE)
                  s = projectTree->itemPath(item);
            else
                  s = projectTree->itemPath(item) + "/" + projectNameEntry->text();
            }
      else
            s = projectNameEntry->text(); 
      return s;
      }

//---------------------------------------------------------
//   newFolderClicked
//---------------------------------------------------------

void ProjectDialog::newFolderClicked()
      {
      QString title(tr("MusE: create new folder"));
      QString folder = QInputDialog::getText(this, title, tr("new folder:"));
      if (folder.isEmpty())
            return;
      QString path;
      QTreeWidgetItem* item = projectTree->currentItem();
      if (item) {
            QStringList sl = projectTree->itemPath(item).split("/");
            int n = sl.size() - 1;
            for (int i = 0; i < n; ++i) {
                  if (!path.isEmpty())
                        path += "/";
                  path += sl[i];
                  }
            if (!path.isEmpty())
                  path += "/";
            }
      path += folder;

      QDir d;
      if (!d.mkpath(QDir::homePath() + "/" + config.projectPath + "/" + path)) {
            QString s("Creating new project folder <%1> failed");
            QMessageBox::critical(this, title, s.arg(path));
            }
      else {
            if (item)
                  item = item->parent();
            QStringList pathElements = folder.split("/");
            foreach(QString s, pathElements) {
                  QTreeWidgetItem* pi;
                  if (item == 0)
                        pi = new QTreeWidgetItem(projectTree, DIR_TYPE);
                  else
                        pi = new QTreeWidgetItem(item, DIR_TYPE);
                  pi->setText(0, s);
                  itemCollapsed(pi);
                  item = pi;
                  }
            projectTree->setCurrentItem(item);
            }
      }

//---------------------------------------------------------
//   itemDoubleClicked
//---------------------------------------------------------

void ProjectDialog::itemDoubleClicked(QTreeWidgetItem* item, int)
      {
      if (item->type() == PROJECT_TYPE)
            accept();      
      }

