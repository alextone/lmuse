//=========================================================
//  MusE
//  Linux Music Editor
//  $Id: gconfig.h,v 1.12.2.10 2009/12/01 03:52:40 terminator356 Exp $
//
//  (C) Copyright 1999-2003 Werner Schweer (ws@seh.de)
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

#ifndef __CONFIG_H__
#define __CONFIG_H__

#define NUM_PARTCOLORS 18
#define NUM_FONTS 7

#include <QColor>
#include <QFont>
#include <QRect>
#include <QString>

namespace MusECore {
class Xml;

enum newDrumRecordCondition_t
{
  REC_ALL = 0,
  DONT_REC_HIDDEN = 1,
  DONT_REC_MUTED = 2,
  DONT_REC_MUTED_OR_HIDDEN = 3
};

}

namespace MusEGlobal {

enum drumTrackPreference_t
{
  PREFER_OLD = 0,
  PREFER_NEW = 1,
  ONLY_OLD = 2,
  ONLY_NEW = 3
};

enum ExportPortsDevices_t
{
  EXPORT_PORTS_DEVICES_ALL = 0,
  PORT_NUM_META = 1,
  DEVICE_NAME_META = 2,
  EXPORT_PORTS_DEVICES_END = 3
};

enum ExportModeInstr_t
{
  EXPORT_MODE_INSTR_ALL = 0,
  MODE_SYSEX = 1,
  INSTRUMENT_NAME_META = 2,
  EXPORT_MODE_INSTR_END = 3
};

//---------------------------------------------------------
//   MixerConfig
//---------------------------------------------------------

struct MixerConfig {
      QString name;
      QRect geometry;
      bool showMidiTracks;
      bool showDrumTracks;
      bool showNewDrumTracks;
      bool showInputTracks;
      bool showOutputTracks;
      bool showWaveTracks;
      bool showGroupTracks;
      bool showAuxTracks;
      bool showSyntiTracks;

      void write(int level, MusECore::Xml& xml);
      void read(MusECore::Xml& xml);
      };

//---------------------------------------------------------
//   GlobalConfigValues
//---------------------------------------------------------

struct GlobalConfigValues {
      int globalAlphaBlend;
      QColor palette[16];
      QColor partColors[NUM_PARTCOLORS];
      QString partColorNames[NUM_PARTCOLORS];
      QColor transportHandleColor;
      QColor bigTimeForegroundColor;
      QColor bigTimeBackgroundColor;
      QColor waveEditBackgroundColor;
      QFont fonts[NUM_FONTS];
      QColor trackBg;
      QColor selectTrackBg;
      QColor selectTrackFg;
      
      QColor midiTrackLabelBg;
      QColor drumTrackLabelBg;
      QColor newDrumTrackLabelBg;
      QColor waveTrackLabelBg;
      QColor outputTrackLabelBg;
      QColor inputTrackLabelBg;
      QColor groupTrackLabelBg;
      QColor auxTrackLabelBg;
      QColor synthTrackLabelBg;

      QColor midiTrackBg;
      QColor drumTrackBg;
      QColor newDrumTrackBg;
      QColor waveTrackBg;
      QColor outputTrackBg;
      QColor inputTrackBg;
      QColor groupTrackBg;
      QColor auxTrackBg;
      QColor synthTrackBg;
      
      QColor partCanvasBg;
      QColor ctrlGraphFg;
      QColor mixerBg;

      QColor rulerBg;
      QColor rulerFg;
      QColor midiCanvasBg;
      QColor midiControllerViewBg;
      QColor drumListBg;
      QColor rulerCurrent;

      QColor waveNonselectedPart;
      QColor wavePeakColor;
      QColor waveRmsColor;
      QColor wavePeakColorSelected;
      QColor waveRmsColorSelected;

      QColor partWaveColorPeak;
      QColor partWaveColorRms;
      QColor partMidiDarkEventColor;
      QColor partMidiLightEventColor;

      int division;
      int rtcTicks;
      bool midiSendInit;         // Send instrument initialization sequences
      bool warnInitPending;      // Warn instrument initialization sequences pending
      bool midiSendCtlDefaults;  // Send instrument controller defaults at position 0 if none in song
      bool warnIfBadTiming;      // Warn if timer res not good
      bool velocityPerNote;      // Whether to show per-note or all velocities
      int minMeter;
      double minSlider;
      bool freewheelMode;
      int guiRefresh;
      QString userInstrumentsDir;  // Obsolete. Must keep for compatibility.

      bool extendedMidi;      // extended smf format
      int midiDivision;       // division for smf export
      QString copyright;      // copyright string for smf export
      int smfFormat;          // smf export file type
      bool exp2ByteTimeSigs;  // Export 2 byte time sigs instead of 4 bytes
      bool expOptimNoteOffs;  // Save space by replacing note offs with note on velocity 0
      bool importMidiSplitParts; // Split imported tracks into multiple parts.
      bool importMidiNewStyleDrum; // Use new style drum tracks
      bool importDevNameMetas;    // Import Prefer Device Name metas over port number metas if both exist.
      bool importInstrNameMetas;  // Import Prefer Instrument Name metas over Mode sysexes if both exist.
      ExportPortsDevices_t exportPortsDevices;  // Export port number metas and/or device name metas.
      bool exportPortDeviceSMF0;  // Export a port and/or device meta even for SMF0.
      ExportModeInstr_t exportModeInstr;   // Export mode sysexes and/or instrument name metas.
      QString importMidiDefaultInstr;  // Default to this instrument not Generic, if no match found
      
      int startMode;          // 0 - start with last song
                              // 1 - start with default template
                              // 2 - start with song
      QString startSong;      // path for start song
      bool startSongLoadConfig;  // Whether to load configuration with the start template or song
      int guiDivision;        // division for tick display

      QRect geometryMain;
      QRect geometryTransport;
      QRect geometryBigTime;
      MixerConfig mixer1;
      MixerConfig mixer2;
      bool transportVisible;
      bool bigTimeVisible;
      bool mixer1Visible;
      bool mixer2Visible;
      bool markerVisible;
      bool arrangerVisible;

      bool showSplashScreen;
      int canvasShowPartType;       // 1 - names, 2 events
      int canvasShowPartEvent;      //
      bool canvasShowGrid;
      QString canvasBgPixmap;
      QStringList canvasCustomBgList;
      QString styleSheetFile;
      QString style;

      QString externalWavEditor;
      bool useOldStyleStopShortCut;
      bool moveArmedCheckBox;
      bool useDenormalBias;
      bool useOutputLimiter;
      bool showDidYouKnow;
      bool vstInPlace; // Enable VST in-place processing
      int dummyAudioSampleRate; 
      int dummyAudioBufSize;
      QString projectBaseFolder;
      bool projectStoreInFolder;
      bool useProjectSaveDialog;
      unsigned long minControlProcessPeriod;
      bool popupsDefaultStayOpen;
      bool leftMouseButtonCanDecrease;
      bool rangeMarkerWithoutMMB;
      MusECore::newDrumRecordCondition_t newDrumRecordCondition;
      bool addHiddenTracks;
      bool unhideTracks;
      drumTrackPreference_t drumTrackPreference;
      bool smartFocus;
      int trackHeight;
      bool borderlessMouse;
      bool autoSave;
      };




extern GlobalConfigValues config;
} // namespace MusEGlobal

#endif

