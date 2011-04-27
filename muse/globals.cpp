//=========================================================
//  MusE
//  Linux Music Editor
//  $Id: globals.cpp,v 1.15.2.11 2009/11/25 09:09:43 terminator356 Exp $
//
//  (C) Copyright 1999-2004 Werner Schweer (ws@seh.de)
//=========================================================

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include <QActionGroup>
#include <QDir>
#include <QFileInfo>

#include "globals.h"
#include "config.h"

int recFileNumber = 1;

int sampleRate   = 44100;
unsigned segmentSize  = 1024U;    // segmentSize in frames (set by JACK)
unsigned fifoLength =  128;       // 131072/segmentSize
                                  // 131072 - magic number that gives a sufficient buffer size
int segmentCount = 2;

// denormal bias value used to eliminate the manifestation of denormals by
// lifting the zero level slightly above zero
// denormal problems occur when values get extremely close to zero
const float denormalBias=1e-18;

bool overrideAudioOutput = false;
bool overrideAudioInput = false;

QTimer* heartBeatTimer;

bool hIsB = true;             // call note h "b"

const signed char sharpTab[14][7] = {
      { 0, 3, -1, 2, 5, 1, 4 },
      { 0, 3, -1, 2, 5, 1, 4 },
      { 0, 3, -1, 2, 5, 1, 4 },
      { 0, 3, -1, 2, 5, 1, 4 },
      { 2, 5,  1, 4, 7, 3, 6 },
      { 2, 5,  1, 4, 7, 3, 6 },
      { 2, 5,  1, 4, 7, 3, 6 },
      { 4, 0,  3, -1, 2, 5, 1 },
      { 7, 3,  6, 2, 5, 1, 4 },
      { 5, 8,  4, 7, 3, 6, 2 },
      { 3, 6,  2, 5, 1, 4, 7 },
      { 1, 4,  0, 3, 6, 2, 5 },
      { 6, 2,  5, 1, 4, 0, 3 },
      { 0, 3, -1, 2, 5, 1, 4 },
      };
const signed char flatTab[14][7]  = {
      { 4, 1, 5, 2, 6, 3, 7 },
      { 4, 1, 5, 2, 6, 3, 7 },
      { 4, 1, 5, 2, 6, 3, 7 },
      { 4, 1, 5, 2, 6, 3, 7 },
      { 6, 3, 7, 4, 8, 5, 9 },
      { 6, 3, 7, 4, 8, 5, 9 },
      { 6, 3, 7, 4, 8, 5, 9 },

      { 1, 5, 2, 6, 3, 7, 4 },
      { 4, 1, 5, 2, 6, 3, 7 },
      { 2, 6, 3, 7, 4, 8, 5 },
      { 7, 4, 1, 5, 2, 6, 3 },
      { 5, 2, 6, 3, 7, 4, 8 },
      { 3, 0, 4, 1, 5, 2, 6 },
      { 4, 1, 5, 2, 6, 3, 7 },
      };

QString museGlobalLib;
QString museGlobalShare;
QString museUser;
QString museProject;
QString museProjectInitPath("./");
QString configName = QString(getenv("HOME")) + QString("/.config/MusE/MusE.cfg");
QString configPath = QFileInfo(configName).absoluteDir().absolutePath();
QString museInstruments;
QString museUserInstruments;

QString lastWavePath(".");
QString lastMidiPath(".");

bool debugMode = false;
bool debugMsg = false;
bool heavyDebugMsg = false;
bool midiInputTrace = false;
bool midiOutputTrace = false;
bool realTimeScheduling = false;
int realTimePriority = 40;  // 80
int midiRTPrioOverride = -1;
bool loadPlugins = true;
bool loadVST = true;
bool loadDSSI = true;
bool usePythonBridge = false;
bool useLASH = true;

/*
const char* midi_file_pattern[] = {
      QT_TRANSLATE_NOOP("@default", "Midi/Kar (*.mid *.MID *.kar *.KAR *.mid.gz *.mid.bz2)"),
      QT_TRANSLATE_NOOP("@default", "Midi (*.mid *.MID *.mid.gz *.mid.bz2)"),
      QT_TRANSLATE_NOOP("@default", "Karaoke (*.kar *.KAR *.kar.gz *.kar.bz2)"),
      QT_TRANSLATE_NOOP("@default", "All Files (*)"),
      0
      };
*/      
const QStringList midi_file_pattern =  
      QT_TRANSLATE_NOOP("@default", 
      QString("Midi/Kar (*.mid *.MID *.kar *.KAR *.mid.gz *.mid.bz2);;") +
      QString("Midi (*.mid *.MID *.mid.gz *.mid.bz2);;") +
      QString("Karaoke (*.kar *.KAR *.kar.gz *.kar.bz2);;") +
      QString("All Files (*)")).split(";;");

//FIXME: By T356 01/19/2010
// If saving as a compressed file (gz or bz2),
//  the file is a pipe, and pipes can't seek !
// This results in a corrupted midi file from MidiFile::writeTrack(). 
// So exporting compressed midi has simply been disabled here for now...
/*
const char* midi_file_save_pattern[] = {
      "Midi (*.mid)",
      "gzip compressed Midi (*.mid.gz)",
      "bzip2 compressed Midi (*.mid.bz2)",
      "Karaoke (*.kar)",
      "gzip compressed karaoke (*.kar.gz)",
      "bzip2 compressed karaoke (*.kar.bz2)",
      "All Files (*)",
      0
      };
QStringList midi_file_save_pattern =  
      QStringList::split(";;", QT_TRANSLATE_NOOP("@default", 
      QString("Midi (*.mid);;") +
      QString("gzip compressed Midi (*.mid.gz);;") +
      QString("bzip2 compressed Midi (*.mid.bz2);;") +
      QString("Karaoke (*.kar);;") +
      QString("gzip compressed karaoke (*.kar.gz);;") +
      QString("bzip2 compressed karaoke (*.kar.bz2);;") +
      QString("All Files (*)")) );
*/
/*
const char* midi_file_save_pattern[] = {
      QT_TRANSLATE_NOOP("@default", "Midi (*.mid)"),
      QT_TRANSLATE_NOOP("@default", "Karaoke (*.kar)"),
      QT_TRANSLATE_NOOP("@default", "All Files (*)"),
      0
      };
*/
const QStringList midi_file_save_pattern =  
      QT_TRANSLATE_NOOP("@default", 
      QString("Midi (*.mid);;") +
      QString("Karaoke (*.kar);;") +
      QString("All Files (*)")).split(";;");

/*
const char* med_file_pattern[] = {
      QT_TRANSLATE_NOOP("@default", "med Files (*.med *.med.gz *.med.bz2)"),
      QT_TRANSLATE_NOOP("@default", "Uncompressed med Files (*.med)"),
      QT_TRANSLATE_NOOP("@default", "gzip compressed med Files (*.med.gz)"),
      QT_TRANSLATE_NOOP("@default", "bzip2 compressed med Files (*.med.bz2)"),
      QT_TRANSLATE_NOOP("@default", "All Files (*)"),
      0
      };
const char* med_file_save_pattern[] = {
      QT_TRANSLATE_NOOP("@default", "Uncompressed med Files (*.med)"),
      QT_TRANSLATE_NOOP("@default", "gzip compressed med Files (*.med.gz)"),
      QT_TRANSLATE_NOOP("@default", "bzip2 compressed med Files (*.med.bz2)"),
      QT_TRANSLATE_NOOP("@default", "All Files (*)"),
      0
      };
*/      
const QStringList med_file_pattern =  
      QT_TRANSLATE_NOOP("@default", 
      QString("med Files (*.med *.med.gz *.med.bz2);;") +
      QString("Uncompressed med Files (*.med);;") +
      QString("gzip compressed med Files (*.med.gz);;") +
      QString("bzip2 compressed med Files (*.med.bz2);;") +
      QString("All Files (*)")).split(";;");
const QStringList med_file_save_pattern =  
      QT_TRANSLATE_NOOP("@default", 
      QString("Uncompressed med Files (*.med);;") +
      QString("gzip compressed med Files (*.med.gz);;") +
      QString("bzip2 compressed med Files (*.med.bz2);;") +
      QString("All Files (*)")).split(";;");

/*
const char* image_file_pattern[] = {
      QT_TRANSLATE_NOOP("@default", "(*.jpg *.gif *.png)"),
      QT_TRANSLATE_NOOP("@default", "(*.jpg)"),
      QT_TRANSLATE_NOOP("@default", "(*.gif)"),
      QT_TRANSLATE_NOOP("@default", "(*.png)"),
      QT_TRANSLATE_NOOP("@default", "All Files (*)"),
      0
      };
*/      
const QStringList image_file_pattern =  
      QT_TRANSLATE_NOOP("@default", 
      QString("(*.jpg *.gif *.png);;") +
      QString("(*.jpg);;") +
      QString("(*.gif);;") +
      QString("(*.png);;") +
      QString("All Files (*)")).split(";;");

// Not used.
/*
const char* ctrl_file_pattern[] = {
      "ctrl Files (*.ctrl *.ctrl.gz *.ctrl.bz2)",
      "All Files (*)",
      0
      };
*/

/*
const char* part_file_pattern[] = {
      //QT_TRANSLATE_NOOP("@default", "part Files (*.mpt)"),
      QT_TRANSLATE_NOOP("@default", "part Files (*.mpt *.mpt.gz *.mpt.bz2)"),
      QT_TRANSLATE_NOOP("@default", "All Files (*)"),
      0
      };
const char* part_file_save_pattern[] = {
      QT_TRANSLATE_NOOP("@default", "part Files (*.mpt)"),
      QT_TRANSLATE_NOOP("@default", "gzip compressed part Files (*.mpt.gz)"),
      QT_TRANSLATE_NOOP("@default", "bzip2 compressed part Files (*.mpt.bz2)"),
      QT_TRANSLATE_NOOP("@default", "All Files (*)"),
      0
      };
*/
const QStringList part_file_pattern =  
      QT_TRANSLATE_NOOP("@default", 
      QString("part Files (*.mpt *.mpt.gz *.mpt.bz2);;") +
      QString("All Files (*)")).split(";;");

const QStringList part_file_save_pattern =  
      QT_TRANSLATE_NOOP("@default", 
      QString("part Files (*.mpt);;") +
      QString("gzip compressed part Files (*.mpt.gz);;") +
      QString("bzip2 compressed part Files (*.mpt.bz2);;") +
      QString("All Files (*)")).split(";;");

/*
const char* plug_file_pattern[] = {
      QT_TRANSLATE_NOOP("@default", "part Files (*.pre)"),
      QT_TRANSLATE_NOOP("@default", "All Files (*)"),
      0
      };
*/
/*
const char* preset_file_pattern[] = {
      QT_TRANSLATE_NOOP("@default", "Presets (*.pre *.pre.gz *.pre.bz2)"),
      QT_TRANSLATE_NOOP("@default", "All Files (*)"),
      0
      };

const char* preset_file_save_pattern[] = {
      QT_TRANSLATE_NOOP("@default", "Presets (*.pre)"),
      QT_TRANSLATE_NOOP("@default", "gzip compressed presets (*.pre.gz)"),
      QT_TRANSLATE_NOOP("@default", "bzip2 compressed presets (*.pre.bz2)"),
      QT_TRANSLATE_NOOP("@default", "All Files (*)"),
      0
      };
*/
const QStringList preset_file_pattern =  
      QT_TRANSLATE_NOOP("@default", 
      QString("Presets (*.pre *.pre.gz *.pre.bz2);;") +
      QString("All Files (*)")).split(";;");

const QStringList preset_file_save_pattern =  
      QT_TRANSLATE_NOOP("@default", 
      QString("Presets (*.pre);;") +
      QString("gzip compressed presets (*.pre.gz);;") +
      QString("bzip2 compressed presets (*.pre.bz2);;") +
      QString("All Files (*)")).split(";;");

const QStringList drum_map_file_pattern =  
      QT_TRANSLATE_NOOP("@default", 
      QString("Presets (*.map *.map.gz *.map.bz2);;") +
      QString("All Files (*)")).split(";;");
const QStringList drum_map_file_save_pattern =  
      QT_TRANSLATE_NOOP("@default", 
      QString("Presets (*.map);;") +
      QString("gzip compressed presets (*.map.gz);;") +
      QString("bzip2 compressed presets (*.map.bz2);;") +
      QString("All Files (*)")).split(";;");

const QStringList audio_file_pattern =  
      QT_TRANSLATE_NOOP("@default", 
      QString("Wave/Binary (*.wav *.ogg *.bin);;") +
      QString("Wave (*.wav *.ogg);;") +
      QString("Binary (*.bin);;") +
      QString("All Files (*)")).split(";;");

///Qt::ButtonState globalKeyState;
Qt::KeyboardModifiers globalKeyState;

// Midi Filter Parameter
int midiInputPorts   = 0;    // receive from all devices
int midiInputChannel = 0;    // receive all channel
int midiRecordType   = 0;    // receive all events
int midiThruType = 0;        // transmit all events
int midiFilterCtrl1 = 0;
int midiFilterCtrl2 = 0;
int midiFilterCtrl3 = 0;
int midiFilterCtrl4 = 0;

QActionGroup* undoRedo;
QAction* undoAction;
QAction* redoAction;
QActionGroup* transportAction;
QAction* playAction;
QAction* startAction;
QAction* stopAction;
QAction* rewindAction;
QAction* forwardAction;
QAction* loopAction;
QAction* punchinAction;
QAction* punchoutAction;
QAction* recordAction;
QAction* panicAction;

//AudioMixerApp* audioMixer;
MusE* muse;

int preMeasures = 2;
unsigned char measureClickNote = 63;
unsigned char measureClickVelo = 127;
unsigned char beatClickNote    = 63;
unsigned char beatClickVelo    = 70;
unsigned char clickChan = 9;
unsigned char clickPort = 0;
bool precountEnableFlag = false;
bool precountFromMastertrackFlag = false;
int precountSigZ = 4;
int precountSigN = 4;
bool precountPrerecord = false;
bool precountPreroll = false;
bool midiClickFlag   = true;
bool audioClickFlag  = true;
float audioClickVolume = 0.5f;

bool rcEnable = false;
unsigned char rcStopNote = 28;
unsigned char rcRecordNote = 31;
unsigned char rcGotoLeftMarkNote = 33;
unsigned char rcPlayNote = 29;
bool automation = true;

QObject* gRoutingPopupMenuMaster = 0;
RouteMenuMap gRoutingMenuMap;
bool gIsOutRoutingPopupMenu = false;

uid_t euid, ruid;  // effective user id, real user id

bool midiSeqRunning = false;

//---------------------------------------------------------
//   doSetuid
//    Restore the effective UID to its original value.
//---------------------------------------------------------

void doSetuid()
      {
#ifndef RTCAP
      int status;
#ifdef _POSIX_SAVED_IDS
      status = seteuid (euid);
#else
      status = setreuid (ruid, euid);
#endif
      if (status < 0) {
            perror("doSetuid: Couldn't set uid");
            }
#endif
      }

//---------------------------------------------------------
//   undoSetuid
//    Set the effective UID to the real UID.
//---------------------------------------------------------

void undoSetuid()
      {
#ifndef RTCAP
      int status;

#ifdef _POSIX_SAVED_IDS
      status = seteuid (ruid);
#else
      status = setreuid (euid, ruid);
#endif
      if (status < 0) {
            fprintf(stderr, "undoSetuid: Couldn't set uid (eff:%d,real:%d): %s\n",
               euid, ruid, strerror(errno));
            exit (status);
            }
#endif
      }

//---------------------------------------------------------
//   getUniqueTmpfileName
//---------------------------------------------------------
bool getUniqueTmpfileName(QString subDir, QString ext,QString& newFilename)
      {
      // Check if tmp-directory exists under project path
      QString tmpInDir = museProject + "/" + subDir;
      QFileInfo tmpdirfi(tmpInDir);
      if (!tmpdirfi.isDir()) {
            // Try to create a tmpdir
            QDir projdir(museProject);
            if (!projdir.mkdir(subDir)) {
                  printf("Could not create tmp dir %s!\n", tmpInDir.toLatin1().data() );
                  return false;
                  }
            }


      tmpdirfi.setFile(tmpInDir);

      if (!tmpdirfi.isWritable()) {
            printf("Temp directory is not writable - aborting\n");
            return false;
            }

      QDir tmpdir = tmpdirfi.dir();

      // Find a new filename
      for (int i=0; i<10000; i++) {
            QString filename = "muse_tmp";
            filename.append(QString::number(i));
            if (!ext.startsWith("."))
                filename.append(".");
            filename.append(ext);

            if (!tmpdir.exists(tmpInDir +"/" + filename)) {
                  newFilename = tmpInDir + "/" + filename;
                  if (debugMsg)
                      printf("returning temporary filename %s\n", newFilename.toLatin1().data());
                  return true;
                  }

            }

      printf("Could not find a suitable tmpfilename (more than 10000 tmpfiles in tmpdir - clean up!\n");
      return false;
      }
