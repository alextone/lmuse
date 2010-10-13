//=============================================================================
//  MusE
//  Linux Music Editor
//  $Id: dssihost.cpp,v 1.15.2.16 2009/12/15 03:39:58 terminator356 Exp $
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

#include "config.h"
#ifdef DSSI_SUPPORT

// Turn on debugging messages
//#define DSSI_DEBUG 

// Support vst state saving/loading with vst chunks. Requires patches to DSSI and DSSI-vst!
//#define DSSI_VST_CHUNK_SUPPORT

#include <string.h>
#include <signal.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <sys/stat.h>
//#include <dssi.h>
//#include <alsa/asoundlib.h>
#include <qdir.h>
//#include <qstringlist.h>
#include <QFileInfo>
#include <q3popupmenu.h>
//#include <qprocess.h>

#include "dssihost.h"
#include "synth.h"
#include "audio.h"
#include "jackaudio.h"
#include "midi.h"
#include "midiport.h"
#include "stringparam.h"
#include "plugin.h"
//#include "al/al.h"
//#include "al/xml.h"
#include "xml.h"
#include "song.h"
//#include "midictrl.h"
//#include "ladspaplugin.h"

#include "app.h"
#include "globals.h"
#include "globaldefs.h"
//#include "al/dsp.h"
#include "gconfig.h"

/*
static lo_server_thread serverThread;
static char osc_path_tmp[1024];
static char* url;

//---------------------------------------------------------
//   oscError
//---------------------------------------------------------

static void oscError(int num, const char *msg, const char *path)
      {
      fprintf(stderr, "MusE: liblo server error %d in path %s: %s\n",
          num, path, msg);
      }

//---------------------------------------------------------
//   oscDebugHandler
//---------------------------------------------------------

static int oscDebugHandler(const char* path, const char* types, lo_arg** argv,
   int argc, void*, void*)
      {
      printf("MusE: got unhandled OSC message:\n   path: <%s>\n", path);
      for (int i = 0; i < argc; i++) {
            printf("   arg %d '%c' ", i, types[i]);
            lo_arg_pp(lo_type(types[i]), argv[i]);
            printf("\n");
            }
      return 1;
      }

//---------------------------------------------------------
//   oscUpdate
//---------------------------------------------------------

int DssiSynthIF::oscUpdate(lo_arg **argv)
      {
      const char *url = (char *)&argv[0]->s;

      if (uiTarget)
            lo_address_free(uiTarget);
      char* host = lo_url_get_hostname(url);
      char* port = lo_url_get_port(url);
      uiTarget   = lo_address_new(host, port);
      free(host);
      free(port);

      if (uiOscPath)
            free(uiOscPath);
      uiOscPath = lo_url_get_path(url);
      int pl = strlen(uiOscPath);

      if (uiOscControlPath)
            free(uiOscControlPath);
      uiOscControlPath = (char *)malloc(pl + 10);
      sprintf(uiOscControlPath, "%s/control", uiOscPath);

      if (uiOscConfigurePath)
            free(uiOscConfigurePath);
      uiOscConfigurePath = (char *)malloc(pl + 12);
      sprintf(uiOscConfigurePath, "%s/configure", uiOscPath);

      if (uiOscProgramPath)
            free(uiOscProgramPath);
      uiOscProgramPath = (char *)malloc(pl + 10);
      sprintf(uiOscProgramPath, "%s/program", uiOscPath);

      if (uiOscShowPath)
            free(uiOscShowPath);
      uiOscShowPath = (char *)malloc(pl + 10);
      sprintf(uiOscShowPath, "%s/show", uiOscPath);

      // At this point a more substantial host might also call
      // configure() on the UI to set any state that it had remembered
      // for the plugin instance.  But we don't remember state for
      // plugin instances (see our own configure() implementation in
      // osc_configure_handler), and so we have nothing to send except
      // the optional project directory.
      

      #ifdef DSSI_DEBUG 
      printf("DssiSynthIF::oscUpdate synth name:%s url:%s uiTarget:%p uiOscPath:%s uiOscConfigurePath:%s museProject:%s\n", synti->name().ascii(), url, uiTarget, uiOscPath, uiOscConfigurePath, museProject.ascii());
      #endif
      
      //lo_send(uiTarget, uiOscConfigurePath, "ss",
         //DSSI_PROJECT_DIRECTORY_KEY, song->projectPath().toAscii().data());
      lo_send(uiTarget, uiOscConfigurePath, "ss",
         DSSI_PROJECT_DIRECTORY_KEY, museProject.ascii());

#if 0
      // Send current bank/program  (-FIX- another race...) 
      if (instance->pendingProgramChange < 0) {
            unsigned long bank = instance->currentBank;
            unsigned long program = instance->currentProgram;
            instance->uiNeedsProgramUpdate = 0;
            if (instance->uiTarget) {
                  lo_send(instance->uiTarget, instance->ui_osc_program_path, "ii", bank, program);
                  }
            }

      // Send control ports 
      for (i = 0; i < instance->plugin->controlIns; i++) {
            int in = i + instance->firstControlIn;
            int port = pluginControlInPortNumbers[in];
            lo_send(instance->uiTarget, instance->ui_osc_control_path, "if", port,
               pluginControlIns[in]);
            // Avoid overloading the GUI if there are lots and lots of ports 
            if ((i+1) % 50 == 0)
                  usleep(300000);
            }
#endif
      return 0;
      }

//---------------------------------------------------------
//   oscMessageHandler
//---------------------------------------------------------

int oscMessageHandler(const char* path, const char* types, lo_arg** argv,
   int argc, void* data, void* user_data)
      {
      const char* p = path;
      
      #ifdef DSSI_DEBUG 
      if(argc) 
      {
          printf("oscMessageHandler: path:%s argc:%d\n", path, argc);
          for(int i = 0; i < argc; ++i) 
          {
            printf(" ");
            lo_arg_pp((lo_type)types[i], argv[i]);
          }
          printf("\n");
      } 
      else 
      {
          printf("%s\n", path);
          printf("oscMessageHandler: no args, path:%s\n", path);
      }
      #endif  
        
      if (strncmp(p, "/dssi/", 6))
            return oscDebugHandler(path, types, argv, argc, data, user_data);
      
      p += 6;
      //p = strrchr(p, "/");

      SynthIList* sl = song->syntis();
      DssiSynthIF* instance = 0;
      SynthI* synti = 0;

      #ifdef DSSI_DEBUG 
      fprintf(stderr, "oscMessageHandler: song->syntis() size:%d\n", sl->size());
      #endif
        
      for(int retry = 0; retry < 5; ++retry) 
      {
        #ifdef DSSI_DEBUG 
        fprintf(stderr, "oscMessageHandler: search retry number:%d ...\n", retry);
        #endif
        
        //if(uiOscPath)
        //  break;
      
        for(iSynthI si = sl->begin(); si != sl->end(); ++si) 
        {
          #ifdef DSSI_DEBUG 
          fprintf(stderr, "oscMessageHandler: searching for synth p:%s: checking instances:%s\n", p, (*si)->name().ascii());
          #endif
          
          //int l = strlen((*si)->name().toAscii().data());
          //if (!strncmp(p, (*si)->name().toAscii().data(), l)) {
          //int l = strlen((*si)->name().ascii());
          const char* sub = strstr(p, (*si)->name().ascii());
          
          //if(!strncmp(p, (*si)->name().ascii(), l)) 
          if(sub != NULL) 
          {
            synti = *si;
            instance = (DssiSynthIF*)(synti->sif());
            
            //p += l;
            p = sub + strlen((*si)->name().ascii());
            
            break;
          }
        }
        if(instance)
          break;
          
        sleep(1);
      }
      
      if(!instance)
      {
        fprintf(stderr, "oscMessageHandler: error: no instance\n");
        return oscDebugHandler(path, types, argv, argc, data, user_data);
      }
      
      if (*p != '/' || *(p + 1) == 0)
      {
        fprintf(stderr, "oscMessageHandler: error: end or no /\n");
        return oscDebugHandler(path, types, argv, argc, data, user_data);
      }
            
      ++p;

      #ifdef DSSI_DEBUG 
      fprintf(stderr, "oscMessageHandler: method:%s\n", p);
      #endif
      
      if (!strcmp(p, "configure") && argc == 2 && !strcmp(types, "ss"))
            return instance->oscConfigure(argv);
      else if (!strcmp(p, "control") && argc == 2 && !strcmp(types, "if"))
            return instance->oscControl(argv);
      else if (!strcmp(p, "midi") && argc == 1 && !strcmp(types, "m"))
            return instance->oscMidi(argv);
      else if (!strcmp(p, "program") && argc == 2 && !strcmp(types, "ii"))
            return instance->oscProgram(argv);
      else if (!strcmp(p, "update") && argc == 1 && !strcmp(types, "s"))
            return instance->oscUpdate(argv);
      else if (!strcmp(p, "exiting") && argc == 0)
            return instance->oscExiting(argv);
      return oscDebugHandler(path, types, argv, argc, data, user_data);
      }
*/

//---------------------------------------------------------
//   scanDSSILib
//---------------------------------------------------------

static void scanDSSILib(QFileInfo& fi) // ddskrjo removed const for argument
      {
      //void* handle = dlopen(fi.filePath().toAscii().data(), RTLD_NOW);
      void* handle = dlopen(fi.filePath().latin1(), RTLD_NOW);
      //void* handle = dlopen(fi.absFilePath().latin1(), RTLD_NOW);
      
      if (handle == 0) {
            fprintf(stderr, "scanDSSILib: dlopen(%s) failed: %s\n",
              //fi.filePath().toAscii().data(), dlerror());
              fi.filePath().latin1(), dlerror());
              //fi.absFilePath().latin1(), dlerror());
              
            return;
            }
      DSSI_Descriptor_Function dssi = (DSSI_Descriptor_Function)dlsym(handle, "dssi_descriptor");

      if (!dssi) 
      {
          /*
          const char *txt = dlerror();
          if (txt) 
          {
            fprintf(stderr,
                "Unable to find dssi_descriptor() function in plugin "
                "library file \"%s\": %s.\n"
                "Are you sure this is a DSSI plugin file?\n",
                //fi.filePath().toAscii().data(),
                fi.filePath().ascii(),
                //fi.absFilePath().latin1(),
                
                txt);
            dlclose(handle);
            exit(1);
          }
          */
        dlclose(handle);
        return;
      }
      else
      {
        const DSSI_Descriptor* descr;
        for (int i = 0;; ++i) 
        {
          descr = dssi(i);
          if (descr == 0)
                break;
          
          #ifdef DSSI_DEBUG 
          fprintf(stderr, "scanDSSILib: name:%s inPlaceBroken:%d\n", descr->LADSPA_Plugin->Name, LADSPA_IS_INPLACE_BROKEN(descr->LADSPA_Plugin->Properties));
          #endif
          
          // Listing synths only while excluding effect plugins:
          // Do the exact opposite of what dssi-vst.cpp does for listing ladspa plugins.
          // That way we cover all bases - effect plugins and synths. 
          // Non-synths will show up in the ladspa effect dialog, while synths will show up here...
          // There should be nothing left out...
          if(descr->run_synth ||
            descr->run_synth_adding ||
            descr->run_multiple_synths ||
            descr->run_multiple_synths_adding) 
          
          {
            const QString label(descr->LADSPA_Plugin->Label);
            
            // Make sure it doesn't already exist.
            std::vector<Synth*>::iterator is;
            for(is = synthis.begin(); is != synthis.end(); ++is)
            {
              Synth* s = *is;
              //#ifdef DSSI_DEBUG 
              //  fprintf(stderr, "scanDSSILib: name:%s listname:%s lib:%s listlib:%s\n", 
              //          label.latin1(), s->name().latin1(), fi.baseName(true).latin1(), s->baseName().latin1());
              //#endif

              if(s->name() == label && s->baseName() == fi.baseName(true))
                break;
            }
            if(is != synthis.end())
              continue;
            
            DssiSynth* s = new DssiSynth(fi, descr);
            
            if(debugMsg)
            {
              fprintf(stderr, "scanDSSILib: name:%s listname:%s lib:%s listlib:%s\n", 
                      label.latin1(), s->name().latin1(), fi.baseName(true).latin1(), s->baseName().latin1());
              int ai = 0, ao = 0, ci = 0, co = 0;
              for(unsigned long pt = 0; pt < descr->LADSPA_Plugin->PortCount; ++pt)
              {
                LADSPA_PortDescriptor pd = descr->LADSPA_Plugin->PortDescriptors[pt];
                if(LADSPA_IS_PORT_INPUT(pd) && LADSPA_IS_PORT_AUDIO(pd))
                  ai++;
                else  
                if(LADSPA_IS_PORT_OUTPUT(pd) && LADSPA_IS_PORT_AUDIO(pd))
                  ao++;
                else  
                if(LADSPA_IS_PORT_INPUT(pd) && LADSPA_IS_PORT_CONTROL(pd))
                  ci++;
                else  
                if(LADSPA_IS_PORT_OUTPUT(pd) && LADSPA_IS_PORT_CONTROL(pd))
                  co++;
              }  
              fprintf(stderr, "audio ins:%d outs:%d control ins:%d outs:%d\n", ai, ao, ci, co);
            }
            
            synthis.push_back(s);
          }
          else
          {
            // NOTE: Just a test
            //QFileInfo ffi(fi);
            //plugins.add(&ffi, LADSPA_Descriptor_Function(NULL), descr->LADSPA_Plugin, false);
            //plugins.add(&ffi, descr, false);
          }
        }
      }  
      dlclose(handle);
      }

//---------------------------------------------------------
//   scanVstDir
//---------------------------------------------------------

static void scanDSSIDir(QString& s) // ddskrjo removed const for argument
{
      if(debugMsg)
        //printf("scan DSSI plugin dir <%s>\n", s.toAscii().data());
        printf("scanDSSIDir: scan DSSI plugin dir <%s>\n", s.latin1());

#ifdef __APPLE__
      QDir pluginDir(s, QString("*.dylib"), QDir::Unsorted, QDir::Files);
#else
      QDir pluginDir(s, QString("*.so"), QDir::Unsorted, QDir::Files);
#endif
      if(!pluginDir.exists())
        return;

      //const QFileInfoList list = pluginDir.entryInfoList();
      //for (int i = 0; i < list.size(); ++i) {
      	//QFileInfo fi = list.at(i);
            //scanDSSILib(fi);
            //}
      
      QStringList list = pluginDir.entryList();
      for(unsigned int i = 0; i < list.count(); ++i) 
      {
        if(debugMsg)
          printf("scanDSSIDir: found %s\n", (s + QString("/") + list[i]).latin1());

        QFileInfo fi(s + QString("/") + list[i]);
        scanDSSILib(fi);
      }
}

//---------------------------------------------------------
//   initDSSI
//---------------------------------------------------------

void initDSSI()
      {
      const char* dssiPath = getenv("DSSI_PATH");
      if (dssiPath == 0)
            dssiPath = "/usr/local/lib64/dssi:/usr/lib64/dssi:/usr/local/lib/dssi:/usr/lib/dssi";

      //const char* ladspaPath = getenv("LADSPA_PATH");
      //if (ladspaPath == 0)
      //      ladspaPath = "/usr/local/lib64/ladspa:/usr/lib64/ladspa:/usr/local/lib/ladspa:/usr/lib/ladspa";
      
      const char* p = dssiPath;
      //QString pth = QString(dssiPath) + QString(":") + QString(ladspaPath);
      //const char* p = pth.latin1();
      while (*p != '\0') {
            const char* pe = p;
            while (*pe != ':' && *pe != '\0')
                  pe++;

            int n = pe - p;
            if (n) {
                  char* buffer = new char[n + 1];
                  strncpy(buffer, p, n);
                  buffer[n] = '\0';
                  QString tmpStr(buffer);
                  scanDSSIDir(tmpStr);
                  delete[] buffer;
                  }
            p = pe;
            if (*p == ':')
                  p++;
            }
      
      // Create OSC thread
      //serverThread = lo_server_thread_new(0, oscError);
      //snprintf(osc_path_tmp, 31, "/dssi");
      //char* tmp = lo_server_thread_get_url(serverThread);
      //url = (char *)malloc(strlen(tmp) + strlen(osc_path_tmp));
      //sprintf(url, "%s%s", tmp, osc_path_tmp + 1);
      //free(tmp);
      //lo_server_thread_add_method(serverThread, 0, 0, oscMessageHandler, 0);
      //lo_server_thread_start(serverThread);
      }

//---------------------------------------------------------
//   DssiSynth
//   Synth.label   =  plug.Label 
//   Synth.descr   =  plug.Name
//   Synth.maker   =  plug.maker 
//   Synth.version =  nil (no such field in ladspa, maybe try copyright instead)
//---------------------------------------------------------

DssiSynth::DssiSynth(QFileInfo& fi, const DSSI_Descriptor* d) : // ddskrjo removed const from QFileInfo
  //Synth(fi, label, descr, maker, ver) 
  Synth(fi, QString(d->LADSPA_Plugin->Label), QString(d->LADSPA_Plugin->Name), QString(d->LADSPA_Plugin->Maker), QString()) 
{
  df = 0;
  handle = 0;
  dssi = 0;
  _hasGui = false;
  
  const LADSPA_Descriptor* descr = d->LADSPA_Plugin;
  
  _portCount = descr->PortCount;
  //_portDescriptors = 0;
  //if(_portCount)
  //  _portDescriptors = new LADSPA_PortDescriptor[_portCount];
  
  _inports = 0;
  _outports = 0;
  _controlInPorts = 0;
  _controlOutPorts = 0;
  for(unsigned long k = 0; k < _portCount; ++k) 
  {
    LADSPA_PortDescriptor pd = descr->PortDescriptors[k];
    //_portDescriptors[k] = pd;
    if(pd & LADSPA_PORT_AUDIO)
    {
      if(pd & LADSPA_PORT_INPUT)
        ++_inports;
      else
      if(pd & LADSPA_PORT_OUTPUT)
        ++_outports;
    }    
    else
    if(pd & LADSPA_PORT_CONTROL)
    {
      if(pd & LADSPA_PORT_INPUT)
        ++_controlInPorts;
      else
      if(pd & LADSPA_PORT_OUTPUT)
        ++_controlOutPorts;
    }    
  }
  
  _inPlaceCapable = !LADSPA_IS_INPLACE_BROKEN(descr->Properties);
  
  // Blacklist vst plugins in-place configurable for now. 
  if ((_inports != _outports) || (fi.baseName(true) == QString("dssi-vst") && !config.vstInPlace))
        _inPlaceCapable = false;
}

DssiSynth::~DssiSynth() 
{ 

}

//---------------------------------------------------------
//   createSIF
//---------------------------------------------------------

SynthIF* DssiSynth::createSIF(SynthI* synti)
{
      if (_instances == 0) 
      {
        //handle = dlopen(info.filePath().toAscii().data(), RTLD_NOW);
        handle = dlopen(info.filePath().latin1(), RTLD_NOW);
        //handle = dlopen(info.absFilePath().latin1(), RTLD_NOW);
        
        if (handle == 0) 
        {
              fprintf(stderr, "DssiSynth::createSIF dlopen(%s) failed: %s\n",
                //info.filePath().toAscii().data(), dlerror());
                info.filePath().latin1(), dlerror());
                //info.absFilePath().latin1(), dlerror());
                
              return 0;
        }
        df = (DSSI_Descriptor_Function)dlsym(handle, "dssi_descriptor");

        if (!df) {
              const char *txt = dlerror();
              fprintf(stderr,
                  "Unable to find dssi_descriptor() function in plugin "
                  "library file \"%s\": %s.\n"
                  "Are you sure this is a DSSI plugin file?\n",
                  //info.filePath().toAscii().data(),
                  info.filePath().latin1(),
                  //info.absFilePath().latin1(),
                  
                  txt ? txt : "?");
              dlclose(handle);
              handle = 0;
              return 0;
              }
        for (int i = 0;; ++i) {
              dssi = df(i);
              if (dssi == 0)
                    break;
              QString label(dssi->LADSPA_Plugin->Label);
              if (label == _name)
                    break;
              }

        if(dssi != 0)
        {
          _inports    = 0;
          _outports   = 0;
          _controlInPorts = 0;
          _controlOutPorts = 0;

          pIdx.clear(); 
          opIdx.clear();
          iIdx.clear(); 
          oIdx.clear(); 
          rpIdx.clear();
          iUsedIdx.clear();
          midiCtl2PortMap.clear();
          port2MidiCtlMap.clear();
          //synti->_guiUpdateControls.clear();
          
          const LADSPA_Descriptor* descr = dssi->LADSPA_Plugin;
          //#ifdef DSSI_DEBUG 
          //  printf("DssiSynth::createSIF ladspa plugin PortCount:%lu\n", d->PortCount);
          //#endif
          
          _portCount = descr->PortCount;
          
          for (unsigned long k = 0; k < _portCount; ++k) 
          {
            LADSPA_PortDescriptor pd = descr->PortDescriptors[k];
            
            #ifdef DSSI_DEBUG 
            printf("DssiSynth::createSIF ladspa plugin Port:%ld Name:%s descriptor:%x\n", k, descr->PortNames[k], pd);
            #endif
            
            if (LADSPA_IS_PORT_AUDIO(pd)) 
            {
              if (LADSPA_IS_PORT_INPUT(pd)) 
              {
                ++_inports;
                iIdx.push_back(k);
                iUsedIdx.push_back(false); // Start out with all false.
              }
              else if (LADSPA_IS_PORT_OUTPUT(pd)) 
              {
                ++_outports;
                oIdx.push_back(k);
              }
              
              rpIdx.push_back((unsigned long)-1);
            }
            else if (LADSPA_IS_PORT_CONTROL(pd)) 
            {
              if (LADSPA_IS_PORT_INPUT(pd)) 
              {
                rpIdx.push_back(_controlInPorts);
                ++_controlInPorts;
                pIdx.push_back(k);
                // Set to false at first.
                //synti->_guiUpdateControls.push_back(false);
              }
              else if (LADSPA_IS_PORT_OUTPUT(pd))
              {
                rpIdx.push_back((unsigned long)-1);
                ++_controlOutPorts;
                opIdx.push_back(k);
              }
            }
          }
          
          _inPlaceCapable = !LADSPA_IS_INPLACE_BROKEN(descr->Properties);
          // Blacklist vst plugins in-place configurable for now. 
          if((_inports != _outports) || (info.baseName(true) == QString("dssi-vst") && !config.vstInPlace))
            _inPlaceCapable = false;
        }  
      }  
      
      if (dssi == 0) 
      {
        //fprintf(stderr, "cannot found DSSI synti %s\n", _name.toAscii().data());
        fprintf(stderr, "cannot find DSSI synti %s\n", _name.latin1());
        dlclose(handle);
        handle = 0;
        df     = 0;
        return 0;
      }
      
      DssiSynthIF* sif = new DssiSynthIF(synti);
      ++_instances;
      sif->init(this);

      //_plugin->incInstances(1);



//      static char oscUrl[1024];
      //snprintf(oscUrl, 1024, "%s/%s", url, synti->name().toAscii().data());
      //snprintf(oscUrl, 1024, "%s/%s", url, synti->name().latin1());
//      snprintf(oscUrl, 1024, "%s/%s/%s", url, info.baseName().latin1(), synti->name().latin1());
      //QString guiPath(info.path() + "/" + info.baseName());
      QString guiPath(info.dirPath() + "/" + info.baseName());
      QDir guiDir(guiPath, "*", QDir::Unsorted, QDir::Files);
      _hasGui = guiDir.exists();
      
      //sif->initGui();
      
      return sif;
}

//---------------------------------------------------------
//   guiVisible
//---------------------------------------------------------

bool DssiSynthIF::guiVisible() const
      {
      //return _guiVisible;
      #ifdef OSC_SUPPORT
      return _oscif.oscGuiVisible();
      #endif
      return false;
      }

//---------------------------------------------------------
//   showGui
//---------------------------------------------------------

void DssiSynthIF::showGui(bool v)
      {
      #ifdef OSC_SUPPORT
      
      #ifdef DSSI_DEBUG 
      printf("DssiSynthIF::showGui(): v:%d visible:%d\n", v, guiVisible());
      #endif
      
      _oscif.oscShowGui(v);
      
      #endif // OSC_SUPPORT
      
      /*
      if (v == guiVisible())
            return;
      
      //if(guiPid == -1)
      if((guiQProc == 0) || (!guiQProc->isRunning()))
      {
        // We need an indicator that update was called - update must have been called to get new path etc...
        // If the process is not running this path is invalid, right?
        if(uiOscPath)
          free(uiOscPath);
        uiOscPath = 0;  
          
        #ifdef DSSI_DEBUG 
        printf("DssiSynthIF::showGui(): No QProcess or process not running. Starting gui...\n");
        #endif
        
        initGui();
      }  
      
      //for (int i = 0; i < 5; ++i) {
      for (int i = 0; i < 10; ++i) {    // Give it a wee bit more time?
            if (uiOscPath)
                  break;
            sleep(1);
            }
      if (uiOscPath == 0) {
            printf("DssiSynthIF::showGui(): no uiOscPath. Error: Timeout - synth gui did not start within 10 seconds.\n");
            return;
            }
      
      char uiOscGuiPath[strlen(uiOscPath)+6];
      sprintf(uiOscGuiPath, "%s/%s", uiOscPath, v ? "show" : "hide");
      #ifdef DSSI_DEBUG 
      printf("DssiSynthIF::showGui(): Sending show/hide uiOscGuiPath:%s\n", uiOscGuiPath);
      #endif
      
      lo_send(uiTarget, uiOscGuiPath, "");
      _guiVisible = v;
      */
      }

//---------------------------------------------------------
//   receiveEvent
//---------------------------------------------------------

//MidiEvent DssiSynthIF::receiveEvent()
//      {
//      return MidiEvent();
//      }
MidiPlayEvent DssiSynthIF::receiveEvent()
      {
      return MidiPlayEvent();
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

bool DssiSynthIF::init(DssiSynth* s)
      {
      #ifdef DSSI_DEBUG 
      printf("DssiSynthIF::init\n");
      #endif
      
      synth = s;
      const DSSI_Descriptor* dssi = synth->dssi;
      const LADSPA_Descriptor* ld = dssi->LADSPA_Plugin;
      handle = ld->instantiate(ld, sampleRate);

      #ifdef OSC_SUPPORT
      _oscif.oscSetSynthIF(this);
      #endif
      
      queryPrograms();

      int inports = synth->_inports;
      if(inports != 0)
      {
        audioInBuffers = new float*[inports];
        for(int k = 0; k < inports; ++k)
        {
          //audioInBuffers[k] = new LADSPA_Data[segmentSize];
          //posix_memalign((void**)(audioInBuffers + k), 16, sizeof(float) * segmentSize);
          posix_memalign((void**)&audioInBuffers[k], 16, sizeof(float) * segmentSize);
          memset(audioInBuffers[k], 0, sizeof(float) * segmentSize);
          ld->connect_port(handle, synth->iIdx[k], audioInBuffers[k]);
        }  
      }
      
      int outports = synth->_outports;
      if(outports != 0)
      {
        audioOutBuffers = new float*[outports];
        for(int k = 0; k < outports; ++k)
        {
          //audioOutBuffers[k] = new LADSPA_Data[segmentSize];
          //posix_memalign((void**)(audioOutBuffers + k), 16, sizeof(float) * segmentSize);
          posix_memalign((void**)&audioOutBuffers[k], 16, sizeof(float) * segmentSize);
          memset(audioOutBuffers[k], 0, sizeof(float) * segmentSize);
          ld->connect_port(handle, synth->oIdx[k], audioOutBuffers[k]);
          //printf("DssiSynthIF::init output port name: %s\n", ld->PortNames[synth->oIdx[k]]); // out1, out2, out3 etc
        }  
      }
      
      int controlPorts = synth->_controlInPorts;
      int controlOutPorts = synth->_controlOutPorts;
      
      if(controlPorts != 0)
        controls = new Port[controlPorts];
      else
        controls = 0;
          
      if(controlOutPorts != 0)
        controlsOut = new Port[controlOutPorts];
      else
        controlsOut = 0;

      synth->midiCtl2PortMap.clear();
      synth->port2MidiCtlMap.clear();
      synti->_guiUpdateControls.clear();
      synti->_guiUpdateProgram = false;
                
      for (int k = 0; k < controlPorts; ++k) {
                int i = synth->pIdx[k];
                //controls[k].val = ladspaDefaultValue(ld, i);
                ladspaDefaultValue(ld, i, &controls[k].val);
		
                // Set to false at first.
                synti->_guiUpdateControls.push_back(false);
              
                #ifdef DSSI_DEBUG 
                printf("DssiSynthIF::init control port:%d port idx:%d name:%s\n", k, i, ld->PortNames[i]);
                #endif
                
                // This code is duplicated in ::getControllerInfo()
                //
                
                int ctlnum = DSSI_NONE;
                if(dssi->get_midi_controller_for_port)
                  ctlnum = dssi->get_midi_controller_for_port(handle, i);
                
                // No controller number? Try to give it a unique one...
                if(ctlnum == DSSI_NONE)
                {
                  // FIXME: Be more careful. Must make sure to pick numbers not already chosen or which WILL BE chosen.
                  // Simple but flawed solution: Start them at 0x60000 + 0x2000 = 0x62000. Max NRPN number is 0x3fff.
                  // TODO: Update: Actually we want to try to use CC Controller7 controllers if possible (or a choice) because what if
                  //  the user's controller hardware doesn't support RPN?
                  // If CC Controller7 is chosen we must make sure to use only non-common numbers. An already limited range
                  //  of 127 now becomes narrower. See the cool document midi-controllers.txt in the DSSI source for a 
                  //  nice roundup of numbers and how to choose them and how they relate to synths and DSSI synths etc. !
                  ctlnum = CTRL_NRPN14_OFFSET + 0x2000 + k; 
                }
                else
                {
                  int c = ctlnum;
                  // Can be both CC and NRPN! Prefer CC over NRPN.
                  if(DSSI_IS_CC(ctlnum))
                  {
                    #ifdef DSSI_DEBUG 
                    printf("DssiSynthIF::init is CC control\n");
                    #endif
                    
                    ctlnum = DSSI_CC_NUMBER(c);
                    #ifdef DSSI_DEBUG 
                    if(DSSI_IS_NRPN(ctlnum))
                      printf("DssiSynthIF::init is also NRPN control. Using CC.\n");
                    #endif  
                  }
                  else
                  if(DSSI_IS_NRPN(ctlnum))
                  {
                    #ifdef DSSI_DEBUG 
                    printf("DssiSynthIF::init  is NRPN control\n");
                    #endif
                    
                    ctlnum = DSSI_NRPN_NUMBER(c) + CTRL_NRPN14_OFFSET;
                  }  
                    
                }
                
                #ifdef DSSI_DEBUG 
                printf("DssiSynthIF::init inserting to midiCtl2PortMap: ctlnum:%d k:%d\n", ctlnum, k);
                #endif
                
                // We have a controller number! Insert it and the DSSI port number into both maps.
                synth->midiCtl2PortMap.insert(std::pair<int, int>(ctlnum, k));
                synth->port2MidiCtlMap.insert(std::pair<int, int>(k, ctlnum));
                ld->connect_port(handle, i, &controls[k].val);
            }

      for (int k = 0; k < controlOutPorts; ++k) {
                int i = synth->opIdx[k];
    
                #ifdef DSSI_DEBUG 
                printf("DssiSynthIF::init control output port:%d port idx:%d name:%s\n", k, i, ld->PortNames[i]);
                #endif
                
                // p3.3.39 Removed.
                /*
                
                //controls[k].val = ladspaDefaultValue(ld, i);
                ladspaDefaultValue(ld, i, &controlsOut[k].val);
                
                // This code is duplicated in ::getControllerInfo()
                //
                
                int ctlnum = DSSI_NONE;
                if(dssi->get_midi_controller_for_port)
                  ctlnum = dssi->get_midi_controller_for_port(handle, i);
                
                // No controller number? Try to give it a unique one...
                if(ctlnum == DSSI_NONE)
                {
                  // FIXME: Be more careful. Must make sure to pick numbers not already chosen or which WILL BE chosen.
                  // Simple but flawed solution: Start them at 0x60000 + 0x3000 = 0x63000. Max NRPN number is 0x3fff.
                  // TODO: CC etc. etc.
                  ctlnum = CTRL_NRPN14_OFFSET + 0x3000 + k; 
                }
                else
                {
                  int c = ctlnum;
                  // Can be both CC and NRPN! Prefer CC over NRPN.
                  if(DSSI_IS_CC(ctlnum))
                  {
                    #ifdef DSSI_DEBUG 
                    printf("DssiSynthIF::init is CC control\n");
                    #endif
                    
                    ctlnum = DSSI_CC_NUMBER(c);
                    
                    #ifdef DSSI_DEBUG 
                    if(DSSI_IS_NRPN(ctlnum))
                      printf("DssiSynthIF::init is also NRPN control. Using CC.\n");
                    #endif  
                  }
                  else
                  if(DSSI_IS_NRPN(ctlnum))
                  {
                    #ifdef DSSI_DEBUG 
                    printf("DssiSynthIF::init  is NRPN control\n");
                    #endif
                    
                    ctlnum = DSSI_NRPN_NUMBER(c) + CTRL_NRPN14_OFFSET;
                  }  
                    
                }
                
                #ifdef DSSI_DEBUG 
                printf("DssiSynthIF::init inserting to midiCtl2PortMap: ctlnum:%d k:%d\n", ctlnum, k);
                #endif
                
                // We have a controller number! Insert it and the DSSI port number into the map.
                // p3.3.39 Removed. Doesn't say whether it's in or out! Don't need this for now. 
                //synth->midiCtl2PortMap.insert(std::pair<int, int>(ctlnum, k));
                    
                */    
                    
                //  - Control outs are not handled but still must be connected to something.
                ld->connect_port(handle, i, &controlsOut[k].val);
            }

      // Set the latency to zero.
      //controls[controlPorts].val = 0.0;
      // Insert a controller for latency and the DSSI port number into the map.
      //synth->midiCtl2PortMap.insert(std::pair<int, int>(CTRL_NRPN14_OFFSET + 0x2000, controlPorts));
      // Connect the port.
      //ld->connect_port(handle, controlPorts, &controls[controlPorts].val);
      
      // Just a test. It works! We can instantiate a ladspa plugin for the synth. But it needs more work...
      //plugins.add(&synth->info, LADSPA_Descriptor_Function(NULL), ld, false);
            
      if (ld->activate)
            ld->activate(handle);

      // Set current configuration values.
      if(dssi->configure) 
      {
        char *rv = dssi->configure(handle, DSSI_PROJECT_DIRECTORY_KEY,
            museProject.latin1()); //song->projectPath()
        
        if(rv)
        {
          fprintf(stderr, "MusE: Warning: plugin doesn't like project directory: \"%s\"\n", rv);
          free(rv);
        }          
        
        for(ciStringParamMap r = synti->_stringParamMap.begin(); r != synti->_stringParamMap.end(); ++r) 
        {
          rv = 0;
          rv = dssi->configure(handle, r->first.c_str(), r->second.c_str());
          if(rv)
          {
            fprintf(stderr, "MusE: Warning: plugin config key: %s value: %s \"%s\"\n", r->first.c_str(), r->second.c_str(), rv);
            free(rv);
          }  
        }
      }
            
      // Set current program.
      if(dssi->select_program)
        dssi->select_program(handle, synti->_curBankL, synti->_curProgram);
      
      //
      // For stored initial control values, let SynthI::initInstance() take care of that via ::setParameter().
      //
        
      return true;
      }

//---------------------------------------------------------
//   DssiSynthIF
//---------------------------------------------------------

DssiSynthIF::DssiSynthIF(SynthI* s)
   : SynthIF(s)
      {
      #ifdef DSSI_DEBUG 
      printf("DssiSynthIF::DssiSynthIF\n");
      #endif
      
      synth = 0;
      handle = NULL;
      controls = 0;
      controlsOut = 0;
      
      //_curBank = 0;
      //_curProgram = 0;
      
      //#ifdef OSC_SUPPORT
      //_oscif.setSynthIF(this);
      //#endif
      
      //_guiVisible = false;
      //uiTarget = 0;
      //uiOscShowPath = 0;
      //uiOscControlPath = 0;
      //uiOscConfigurePath = 0;
      //uiOscProgramPath = 0;
      //uiOscPath = 0;
      //guiPid = -1;
      //guiQProc = 0;
      
      audioInBuffers = 0;
      audioOutBuffers = 0;
      }

//---------------------------------------------------------
//   ~DssiSynthIF
//---------------------------------------------------------

DssiSynthIF::~DssiSynthIF()
{
      #ifdef DSSI_DEBUG 
      printf("DssiSynthIF::~DssiSynthIF\n");
      #endif
      
      if(synth)
      {
        #ifdef DSSI_DEBUG 
        printf("DssiSynthIF::~DssiSynthIF synth:%p\n", synth);
        #endif
        
        if(synth->dssi)
        {
          #ifdef DSSI_DEBUG 
          printf("DssiSynthIF::~DssiSynthIF synth->dssi:%p\n", synth->dssi);
          #endif
       
          if(synth->dssi->LADSPA_Plugin)
          {
            #ifdef DSSI_DEBUG 
            printf("DssiSynthIF::~DssiSynthIFsynth->dssi->LADSPA_Plugin:%p\n", synth->dssi->LADSPA_Plugin);
            #endif
          }
        }
      }
      
      if(synth && synth->dssi && synth->dssi->LADSPA_Plugin)
      {
        const DSSI_Descriptor* dssi = synth->dssi;
        const LADSPA_Descriptor* descr = dssi->LADSPA_Plugin;

        #ifdef DSSI_DEBUG 
        printf("DssiSynthIF::~DssiSynthIF checking cleanup function exists\n");
        #endif
        
        if(descr->cleanup)
        {
          #ifdef DSSI_DEBUG 
          printf("DssiSynthIF::~DssiSynthIF calling cleanup function\n");
          #endif
            
          descr->cleanup(handle);
        }    
      }
      
      /*
      //if (guiPid != -1)
      //      kill(guiPid, SIGHUP);
      if(guiQProc)
      {
        if(guiQProc->isRunning())
        {
          #ifdef DSSI_DEBUG 
          printf("DssiSynthIF::~DssiSynthIF killing guiQProc\n");
          #endif
          
          guiQProc->kill();
        }  
        
        //delete guiQProc;
      }
      
      if(uiOscShowPath)
        free(uiOscShowPath);
      if(uiOscControlPath)
        free(uiOscControlPath);
      if(uiOscConfigurePath)
        free(uiOscConfigurePath);
      if(uiOscProgramPath)
        free(uiOscProgramPath);
      if(uiOscPath)
        free(uiOscPath);
      if(uiTarget)
        lo_address_free(uiTarget);
      */
      
      if(audioInBuffers)
      {
        //for(int i = 0; i < synth->_inports; ++i)
        //{
        //  if(audioInBuffers[i])
        //    delete[] audioInBuffers[i];
        //}  
        for(unsigned long i = 0; i < synth->_inports; ++i) 
        {
          if(audioInBuffers[i])
            free(audioInBuffers[i]);
        }
        delete[] audioInBuffers;
      }  
      
      if(audioOutBuffers)
      {
        //for(int i = 0; i < synth->_outports; ++i)
        //{
        //  if(audioOutBuffers[i])
        //    delete[] audioOutBuffers[i];
        //}  
        for(unsigned long i = 0; i < synth->_outports; ++i) 
        {
          if(audioOutBuffers[i])
            free(audioOutBuffers[i]);
        }
        delete[] audioOutBuffers;
      }  
      
      if(controls)
        delete[] controls;
        
      if(controlsOut)
        delete[] controlsOut;
}

//---------------------------------------------------------
//   getParameter
//---------------------------------------------------------

float DssiSynthIF::getParameter(unsigned long n) const
{
  if(n >= synth->_controlInPorts)
  {
    printf("DssiSynthIF::getParameter param number %ld out of range of ports:%ld\n", n, synth->_controlInPorts);
    return 0.0;
  }
  
  if(!controls)
    return 0.0;
  
  return controls[n].val;
}

//---------------------------------------------------------
//   setParameter
//---------------------------------------------------------

void DssiSynthIF::setParameter(unsigned long n, float v)
{
  if(n >= synth->_controlInPorts)
  {
    printf("DssiSynthIF::setParameter param number %ld out of range of ports:%ld\n", n, synth->_controlInPorts);
    return;
  }
  
  if(!controls)
    return;
  
  controls[n].val = v;
  
  // Notify that changes are to be sent upon heartbeat.
  // TODO: No, at least not for now. So far, setParameter is only called during loading of stored params,
  //  and we don't want this interfering with oscUpdate which also sends the values.
  //synti->_guiUpdateControls[n] = true;
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

//void DssiSynthIF::write(Xml&) const
void DssiSynthIF::write(int level, Xml& xml) const
{
      //bool vstsaved = false;

#ifdef DSSI_VST_CHUNK_SUPPORT
      //---------------------------------------------
      // dump current state of synth
      //---------------------------------------------
      printf("dumping DSSI custom data! %d\n", synth->dssi->getCustomData);

      // this is only needed and supported if
      // we are talking to a VST plugin at the other end.
      std::string name = synth->dssi->LADSPA_Plugin->Name;
      if ((name.length()> 4) && name.substr(name.length() - 4) == " VST")
      {
        printf("is vst plugin, commencing data dump, apiversion=%d!\n", synth->dssi->DSSI_API_Version);
        unsigned long len = 0;
        void* p = 0;
        synth->dssi->getCustomData(handle,&p, &len);
        if (len) {
              xml.tag(level++, "midistate");
              xml.nput(level++, "<event type=\"%d\"", Sysex);
              xml.nput(" datalen=\"%d\">\n", len+7 /*VSTSAVE*/);
              xml.nput(level, "");
              xml.nput("56 53 54 53 41 56 45 "); // embed a save marker "string 'VSTSAVE'
              for (long unsigned int i = 0; i < len; ++i) {
                    if (i && (((i+7) % 16) == 0)) {
                          xml.nput("\n");
                          xml.nput(level, "");
                          }
                    xml.nput("%02x ", ((char*)(p))[i] & 0xff);
                    }
              xml.nput("\n");
              xml.tag(level--, "/event");
              xml.etag(level--, "midistate");
              //vstsaved = true;
              }
      }
#else
      printf("support for vst chunks not compiled in!\n");
#endif

      /*
      // p3.3.39 Store the state of current program and bank and all input control values, but only if VSTSAVE above didn't do it already! 
      // TODO: Not quite good enough, we would want to store all controls for EACH program, not just the current one. 
      // Need to modify controls array to be inside a program array and act as a cache when the user changes a control on a particular program.
      if(!vstsaved)
      {
        if(synth->_controlInPorts)
        {
          // TODO: Hmm, what if these sizes change (platform etc.)? Hard code? Not good - need to store complete value.
          const int fs = sizeof(float);
          const int uls = sizeof(unsigned long);
          
          // Data length: Version major and minor bytes, bank + program, and controllers. 
          const unsigned long len = 2 + 2 * uls + synth->_controlInPorts * fs; 
          
          unsigned long prog = _curBank; 
          unsigned long bnk = _curProgram;
          
          xml.tag(level++, "midistate");
          xml.nput(level++, "<event type=\"%d\"", Sysex);
          xml.nput(" datalen=\"%d\">\n", len+9); //  "PARAMSAVE" length + data length.
          xml.nput(level, "");
          xml.nput("50 41 52 41 4d 53 41 56 45 "); // Embed a save marker string "PARAMSAVE".
          
          unsigned long i = 9;
          
          // Store PARAMSAVE version major...
          char uc = DSSI_PARAMSAVE_VERSION_MAJOR;
          if(i && ((i % 16) == 0)) 
          {
            xml.nput("\n");
            xml.nput(level, "");
          }
          xml.nput("%02x ", uc & 0xff);
          ++i;
          
          // Store PARAMSAVE version minor...
          uc = DSSI_PARAMSAVE_VERSION_MINOR;
          if(i && ((i % 16) == 0)) 
          {
            xml.nput("\n");
            xml.nput(level, "");
          }
          xml.nput("%02x ", uc & 0xff);
          ++i;
          
          // Store bank...
          void* p = &bnk;
          for(int j = 0; j < uls; ++j)
          {
            if(i && ((i % 16) == 0)) 
            {
              xml.nput("\n");
              xml.nput(level, "");
            }
            xml.nput("%02x ", ((char*)(p))[j] & 0xff);
            ++i;
          }  
          
          // Store program...
          p = &prog;
          for(int j = 0; j < uls; ++j)
          {
            if(i && ((i % 16) == 0)) 
            {
              xml.nput("\n");
              xml.nput(level, "");
            }
            xml.nput("%02x ", ((char*)(p))[j] & 0xff);
            ++i;
          }  
          
          // Store controls...
          for(unsigned long c = 0; c < synth->_controlInPorts; ++c)
          {
            float v = controls[c].val;
            p = &v;
            for(int j = 0; j < fs; ++j)
            {
              if(i && ((i % 16) == 0)) 
              {
                xml.nput("\n");
                xml.nput(level, "");
              }
              xml.nput("%02x ", ((char*)(p))[j] & 0xff);
              ++i;
            }  
          }
          xml.nput("\n");
          xml.tag(level--, "/event");
          xml.etag(level--, "midistate");
        }
      }
      */
      
      // Store controls as parameters...
      for(unsigned long c = 0; c < synth->_controlInPorts; ++c)
      {
        float f = controls[c].val;
        xml.floatTag(level, "param", f);
        //xml.tag(level, "param name=\"%s\" val=\"%s\"/", name, r->first.c_str(), r->second.c_str());
      }  
}

//---------------------------------------------------------
//   preProcessAlways
//---------------------------------------------------------

void DssiSynthIF::preProcessAlways()
{

}

//---------------------------------------------------------
//   processEvent
//   Return true if event pointer filled.
//--------------------------------------------------------

bool DssiSynthIF::processEvent(const MidiPlayEvent& e, snd_seq_event_t* event)
{
  const DSSI_Descriptor* dssi = synth->dssi;
  
  int chn = e.channel();
  int a   = e.dataA();
  int b   = e.dataB();
  //for sysex
  //QByteArray ba = QByteArray((const char*)e.data(), e.len());
  //we must had 0xF0 at the beginning and 0xF7 at the end of e.data()
  //ba.push_front(0xF0);
  //ba.push_back(0xF7);
  
  //QByteArray ba();
  ////ba.assign((const char*)e.data(), e.len());
  ////ba.duplicate((const char*)e.data(), e.len());
  ////ba.setRawData((const char*)e.data(), e.len());
  //int len = e.len() + 2;
  
  int len = e.len();
  char ca[len + 2];
  
  ca[0] = 0xF0;
  memcpy(ca + 1, (const char*)e.data(), len);
  ca[len + 1] = 0xF7;

  len += 2;

  //snd_seq_event_t* event = &events[nevents];
  event->queue = SND_SEQ_QUEUE_DIRECT;

  #ifdef DSSI_DEBUG 
  fprintf(stderr, "DssiSynthIF::processEvent midi event type:%d chn:%d a:%d b:%d\n", e.type(), chn, a, b);
  #endif
  
  switch(e.type()) 
  {
    case ME_NOTEON:
      #ifdef DSSI_DEBUG 
      fprintf(stderr, "DssiSynthIF::processEvent midi event is ME_NOTEON\n");
      #endif
          
      if(b)
        snd_seq_ev_set_noteon(event, chn, a, b);
      else
        snd_seq_ev_set_noteoff(event, chn, a, 0);
    break;
    case ME_NOTEOFF:
      snd_seq_ev_set_noteoff(event, chn, a, 0);
    break;
    case ME_PROGRAM:
    {
      #ifdef DSSI_DEBUG 
      fprintf(stderr, "DssiSynthIF::processEvent midi event is ME_PROGRAM\n");
      #endif
      
      int bank = (a >> 8) & 0xff;
      int prog = a & 0xff;
      //_curBank = bank;
      //_curProgram = prog;
      synti->_curBankH = 0;
      synti->_curBankL = bank;
      synti->_curProgram = prog;
      
      if(dssi->select_program)
      {
        dssi->select_program(handle, bank, prog);
        // Notify that changes are to be sent upon heartbeat.
        synti->_guiUpdateProgram = true;
      }  
      // Event pointer not filled. Return false.
      return false;
    }    
    break;
    case ME_CONTROLLER:
    {
      #ifdef DSSI_DEBUG 
      fprintf(stderr, "DssiSynthIF::processEvent midi event is ME_CONTROLLER\n");
      #endif
      
      if((a == 0) || (a == 32))
        return false;
        
      if(a == CTRL_PROGRAM) 
      {
        #ifdef DSSI_DEBUG 
        fprintf(stderr, "DssiSynthIF::processEvent midi event is ME_CONTROLLER, dataA is CTRL_PROGRAM\n");
        #endif
        
        int bank = (b >> 8) & 0xff;
        int prog = b & 0xff;
        
        //_curBank = bank;
        //_curProgram = prog;
        synti->_curBankH = 0;
        synti->_curBankL = bank;
        synti->_curProgram = prog;
        
        if(dssi->select_program)
        {
          dssi->select_program(handle, bank, prog);
          // Notify that changes are to be sent upon heartbeat.
          synti->_guiUpdateProgram = true;
        }  
        // Event pointer not filled. Return false.
        return false;
      }
          
      if(a == CTRL_PITCH) 
      {
        #ifdef DSSI_DEBUG 
        fprintf(stderr, "DssiSynthIF::processEvent midi event is ME_CONTROLLER, dataA is CTRL_PITCH\n");
        #endif
        
        b &= 0x3fff;
        snd_seq_ev_set_pitchbend(event, chn, b);
        // Event pointer filled. Return true.
        return true;
      }
          
      const LADSPA_Descriptor* ld = dssi->LADSPA_Plugin;
      
      ciMidiCtl2LadspaPort ip = synth->midiCtl2PortMap.find(a);
      // Is it just a regular midi controller, not mapped to a LADSPA port (either by the plugin or by us)?
      // NOTE: There's no way to tell which of these controllers is supported by the plugin.
      // For example sustain footpedal or pitch bend may be supported, but not mapped to any LADSPA port.
      if(ip == synth->midiCtl2PortMap.end())
      {
        // p3.3.39 Changed to return false because of crashes with unknown controllers when switching a midi track 
        //  among different dssi synths and regular synths etc. For example high RPN offset numbers (set by another 
        //  device selected into the midi port before selecting this synth) were passing through here when in fact 
        //  the particular synth had no such midi controllers. 
        // ========================== No, that leaves out regular controllers like footpedal
        //#ifdef DSSI_DEBUG 
        //fprintf(stderr, "DssiSynthIF::processEvent dataA:%d not found in map (not a ladspa controller). Ignoring.\n", a);
        //#endif
        //return false;
        
        //#ifdef DSSI_DEBUG 
        //fprintf(stderr, "DssiSynthIF::processEvent dataA:%d not found in map (not a ladspa controller). Filling event as regular controller.\n", a);
        //#endif
        //snd_seq_ev_set_controller(event, chn, a, b);
        //return true;
        
        int ctlnum = a;
        //switch(midiControllerType(a))
        if(midiControllerType(a) != MidiController::Controller7)
          return false;
        else  
        {
          /*
          case MidiController::NRPN14:
          case MidiController::Controller14:
          case MidiController::Pitch:
          case MidiController::Program:
          case MidiController::RPN:
          case MidiController::RPN14:
          case MidiController::NRPN:
                #ifdef DSSI_DEBUG 
                fprintf(stderr, "DssiSynthIF::processEvent non-ladspa midi event controller unsupported. DataA:%d\n", a);
                #endif
                return false;
          */
          
          //case MidiController::Controller7:
                #ifdef DSSI_DEBUG 
                //fprintf(stderr, "DssiSynthIF::processEvent midi event is Controller7. Changing to DSSI_CC type. Current dataA:%d\n", a);
                fprintf(stderr, "DssiSynthIF::processEvent non-ladspa midi event is Controller7. Current dataA:%d\n", a);
                #endif  
                //a = DSSI_CC(a);
                a &= 0x7f;
                ctlnum = DSSI_CC_NUMBER(ctlnum);
          //      break;
          
          /*
          case MidiController::NRPN14:
                #ifdef DSSI_DEBUG 
                //  fprintf(stderr, "DssiSynthIF::processEvent midi event is NRPN. Changing to DSSI_NRPN type. Current dataA:%d\n", a);
                fprintf(stderr, "DssiSynthIF::processEvent non-ladspa midi event is NRPN. Current dataA:%d\n", a);
                #endif  
                //a = DSSI_NRPN(a - CTRL_NRPN14_OFFSET);
                a &= 0x3fff;
                ctlnum = DSSI_NRPN_NUMBER(ctlnum);
                break;
          case MidiController::Controller14:
                a &= 0x7f;
                break;
          case MidiController::Pitch:
                // Should be caught above!
                #ifdef DSSI_DEBUG 
                //fprintf(stderr, "DssiSynthIF::processEvent non-ladspa midi event is Pitch. DataA:%d\n", a);
                fprintf(stderr, "DssiSynthIF::processEvent Error! non-ladspa midi event is Pitch. Should have been caught already! DataA:%d\n", a);
                #endif
                //a &= 0x3fff;
                //snd_seq_ev_set_pitchbend(event, chn, b);
                // Event pointer filled. Return true.
                //return true;
                // Event pointer not filled. Return false.
                return false;
          case MidiController::Program:
                // Should be caught above!
                #ifdef DSSI_DEBUG 
                fprintf(stderr, "DssiSynthIF::processEvent Error! non-ladspa midi event is Program. Should have been caught already! DataA:%d\n", a);
                #endif
                return false;
          case MidiController::RPN:
          case MidiController::RPN14:
          case MidiController::NRPN:
          default: 
                #ifdef DSSI_DEBUG 
                fprintf(stderr, "DssiSynthIF::processEvent non-ladspa midi event is RPN, RPN14, or NRPN type. DataA:%d\n", a);
                #endif
                break;      
          */      
        }
        
        // Verify it's the same number.
        //if(ctlnum != a)
        //{
        //  #ifdef DSSI_DEBUG 
        //  printf("DssiSynthIF::processEvent Error! non-ladspa midi ctlnum:%d != event dataA:%d\n", ctlnum, a);
        //  #endif
          // Event not filled. Return false.
          
          // TEMP: TODO: Turn on later
          //return false;
        //}  
        
        // Fill the event.
        #ifdef DSSI_DEBUG 
        printf("DssiSynthIF::processEvent non-ladspa filling midi event chn:%d dataA:%d dataB:%d\n", chn, a, b);
        #endif
        snd_seq_ev_set_controller(event, chn, a, b);
        return true;
      }
      
      //int num = ip->first;
      unsigned long k = ip->second;
      
      unsigned long i = synth->pIdx[k];
      
      int ctlnum = DSSI_NONE;
      if(dssi->get_midi_controller_for_port)
        ctlnum = dssi->get_midi_controller_for_port(handle, i);
        
      // No midi controller for the ladspa port? Send to ladspa control.
      if(ctlnum == DSSI_NONE)
      {
        // Sanity check.
        if(k > synth->_controlInPorts)
          return false;
          
        // TODO: If necessary... choose non-existing numbers...
        //for(int k = 0; k < controlPorts; ++k) 
        //{
        //  int i = synth->pIdx[k];
        //}
        
        // Simple but flawed solution: Start them at 0x60000 + 0x2000 = 0x62000. Max NRPN number is 0x3fff.
        ctlnum = k + (CTRL_NRPN14_OFFSET + 0x2000);
      }  
      // p3.3.39
      else
      {
        #ifdef DSSI_DEBUG 
        printf("DssiSynthIF::processEvent plugin requests DSSI-style ctlnum:%x(h) %d(d) be mapped to control port:%ld...\n", ctlnum, ctlnum, i);
        #endif
        
        int c = ctlnum;
        // Can be both CC and NRPN! Prefer CC over NRPN.
        if(DSSI_IS_CC(ctlnum))
        {
          ctlnum = DSSI_CC_NUMBER(c);
          
          #ifdef DSSI_DEBUG 
          printf("DssiSynthIF::processEvent is CC ctlnum:%d\n", ctlnum);
          #endif
          
          #ifdef DSSI_DEBUG 
          if(DSSI_IS_NRPN(ctlnum))
            printf("DssiSynthIF::processEvent is also NRPN control. Using CC.\n");
          #endif  
        }
        else
        if(DSSI_IS_NRPN(ctlnum))
        {
          ctlnum = DSSI_NRPN_NUMBER(c) + CTRL_NRPN14_OFFSET;
          
          #ifdef DSSI_DEBUG 
          printf("DssiSynthIF::processEvent is NRPN ctlnum:%x(h) %d(d)\n", ctlnum, ctlnum);
          #endif
        }  
      
      }
      
      //{  
        float val = midi2LadspaValue(ld, i, ctlnum, b); 
        
        #ifdef DSSI_DEBUG 
        //fprintf(stderr, "DssiSynthIF::processEvent No midi controller for control port:%d port:%d dataA:%d Converting val from:%d to ladspa:%f\n", i, k, a, b, val);
        fprintf(stderr, "DssiSynthIF::processEvent control port:%ld port:%ld dataA:%d Converting val from:%d to ladspa:%f\n", i, k, a, b, val);
        #endif
        
        // Set the ladspa port value.
        controls[k].val = val;
        // FIXME: Testing - Works but is this safe in a RT process callback? Try hooking into gui heartbeat timer instead...
        //lo_send(uiTarget, uiOscControlPath, "if", i, val);
        // Notify that changes are to be sent upon heartbeat.
        synti->_guiUpdateControls[k] = true;
        
        // Since we absorbed the message as a ladspa control change, return false - the event is not filled.
        return false;
      //}
     
      // p3.3.39 Removed.
      // "Hosts should not deliver through run_synth any MIDI controller events that have already
      //   been mapped to control port values."
      // D'oh! My mistake, did not understand that the mapping is only a *request* that the app map MIDI 
      //  controller events to a LADSPA port, and must do the conversion, not to actually *send* them via MIDI...
      /*
      else
      {
        switch(midiControllerType(a))
        {
          case MidiController::Controller7:
                #ifdef DSSI_DEBUG 
                //fprintf(stderr, "DssiSynthIF::processEvent midi event is Controller7. Changing to DSSI_CC type. Current dataA:%d\n", a);
                fprintf(stderr, "DssiSynthIF::processEvent midi event is Controller7. Current dataA:%d\n", a);
                #endif  
                //a = DSSI_CC(a);
                a &= 0x7f;
                ctlnum = DSSI_CC_NUMBER(ctlnum);
                break;
          case MidiController::NRPN14:
                #ifdef DSSI_DEBUG 
                //  fprintf(stderr, "DssiSynthIF::processEvent midi event is NRPN. Changing to DSSI_NRPN type. Current dataA:%d\n", a);
                fprintf(stderr, "DssiSynthIF::processEvent midi event is NRPN. Current dataA:%d\n", a);
                #endif  
                //a = DSSI_NRPN(a - CTRL_NRPN14_OFFSET);
                a &= 0x3fff;
                ctlnum = DSSI_NRPN_NUMBER(ctlnum);
                break;
          case MidiController::Controller14:
                a &= 0x7f;
                break;
          case MidiController::Pitch:
                #ifdef DSSI_DEBUG 
                fprintf(stderr, "DssiSynthIF::processEvent midi event is Pitch. DataA:%d\n", a);
                #endif
                a &= 0x3fff;
                break;
          case MidiController::Program:
                #ifdef DSSI_DEBUG 
                fprintf(stderr, "DssiSynthIF::processEvent midi event is Program. DataA:%d\n", a);
                #endif
                a &= 0x3fff;
                break;
          case MidiController::RPN:
          case MidiController::RPN14:
          case MidiController::NRPN:
          default: 
                #ifdef DSSI_DEBUG 
                fprintf(stderr, "DssiSynthIF::processEvent midi event is RPN, RPN14, or NRPN type. DataA:%d\n", a);
                #endif
                break;      
        }
        
        // Verify it's the same number.
        if(ctlnum != a)
        {
          #ifdef DSSI_DEBUG 
          printf("DssiSynthIF::processEvent Error! ctlnum:%d != event dataA:%d\n", ctlnum, a);
          #endif
          // Event not filled. Return false.
          
          // TEMP: TODO: Turn on later
          //return false;
        }  
        
        // Fill the event.
        // FIXME: Darn! We get to this point, but no change in sound (later). Nothing happens, at least with LTS - 
        //         which is the only one I found so far with midi controllers.
        //        Tried with/without converting to DSSI_CC and DSSI_NRPN. What could be wrong here?
        #ifdef DSSI_DEBUG 
        printf("DssiSynthIF::processEvent filling event chn:%d dataA:%d dataB:%d\n", chn, a, b);
        #endif
        snd_seq_ev_set_controller(event, chn, a, b);
      }
      */
      
    }
    break;
    case ME_PITCHBEND:
      snd_seq_ev_set_pitchbend(event, chn, a);
    break;
    case ME_AFTERTOUCH:
      snd_seq_ev_set_chanpress(event, chn, a);
    break;
    case ME_SYSEX:
      #ifdef DSSI_DEBUG 
      fprintf(stderr, "DssiSynthIF::processEvent midi event is ME_SYSEX\n");
      #endif
      
      if (QString((const char*)e.data()).startsWith("VSTSAVE")) {
#ifdef DSSI_VST_CHUNK_SUPPORT
        printf("loading chunk from sysex %s!\n", e.data()+7);
        dssi->setCustomData(handle, e.data()+7 /* len of str*/,e.len()-7);
#else
        printf("support for vst chunks not compiled in!\n");
#endif
        // Event not filled.
        return false;
      }
      /*
      // p3.3.39 Read the state of current bank and program and all input control values.
      // TODO: Needs to be better. See write().
      else 
      if (QString((const char*)e.data()).startsWith("PARAMSAVE")) 
      {
        #ifdef DSSI_DEBUG 
        fprintf(stderr, "DssiSynthIF::processEvent midi event is ME_SYSEX PARAMSAVE\n");
        #endif
        
        unsigned long dlen = e.len() - 9; // Minus "PARAMSAVE"
        if(dlen > 0)
        {
          //if(dlen < 2 * sizeof(unsigned long))
          if(dlen < (2 + 2 * sizeof(unsigned long))) // Version major and minor bytes, bank and program.
            printf("DssiSynthIF::processEvent Error: PARAMSAVE data length does not include at least version major and minor, bank and program!\n");
          else
          {
            // Not required, yet.
            //char vmaj = *((char*)(e.data() + 9));  // After "PARAMSAVE"
            //char vmin = *((char*)(e.data() + 10));
            
            unsigned long* const ulp = (unsigned long*)(e.data() + 11);  // After "PARAMSAVE" + version major and minor.
            // TODO: TODO: Set plugin bank and program.
            _curBank = ulp[0];
            _curProgram = ulp[1];
            
            dlen -= (2 + 2 * sizeof(unsigned long)); // After the version major and minor, bank and program.
            
            if(dlen > 0)
            {
              if((dlen % sizeof(float)) != 0)
                printf("DssiSynthIF::processEvent Error: PARAMSAVE float data length not integral multiple of float size!\n");
              else
              {
                const unsigned long n = dlen / sizeof(float);
                if(n != synth->_controlInPorts)
                  printf("DssiSynthIF::processEvent Warning: PARAMSAVE number of floats:%ld != number of controls:%ld\n", n, synth->_controlInPorts);
                
                // Point to location after "PARAMSAVE", version major and minor, bank and progam.
                float* const fp = (float*)(e.data() + 9 + 2 + 2 * sizeof(unsigned long)); 
                
                for(unsigned long i = 0; i < synth->_controlInPorts && i < n; ++i)
                {
                  const float v = fp[i];
                  controls[i].val = v;
                }
              }
            }  
          }  
        }  
        // Event not filled.
        return false;
      }
      */
      else
      {
        // NOTE: There is a limit on the size of a sysex. Got this: 
        // "DssiSynthIF::processEvent midi event is ME_SYSEX"
        // "WARNING: MIDI event of type ? decoded to 367 bytes, discarding"
        // That might be ALSA doing that.
        snd_seq_ev_set_sysex(event, len,
          //(unsigned char*)ba.data());
          (unsigned char*)ca);
      }
    break;
    default:
      if(debugMsg)
        fprintf(stderr, "DssiSynthIF::processEvent midi event unknown type:%d\n", e.type());
      // Event not filled.
      return false;
    break;
  }
  
  return true;
}

//---------------------------------------------------------
//   getData
//---------------------------------------------------------

//void DssiSynthIF::getData(MidiEventList* el, unsigned pos, int ch, unsigned samples, float** data)
iMPEvent DssiSynthIF::getData(MidiPort* /*mp*/, MPEventList* el, iMPEvent /*i*/, unsigned pos, int ports, unsigned n, float** buffer)
{
  //#ifdef DSSI_DEBUG 
  //  fprintf(stderr, "DssiSynthIF::getData elsize:%d pos:%d ports:%d samples:%d processed already?:%d\n", el->size(), pos, ports, n, synti->processed());
  //#endif
  
  //BEGIN: Process midi events
  
  // FIXME: Add 10(?) for good luck in case volatile size changes (increments) while we're processing.
  //unsigned long nevents = el->size();
  unsigned long nevents = el->size() + synti->putFifo.getSize() + 10; 

  /*
  while (!synti->putFifo.isEmpty()) {
        MidiEvent event = synti->putFifo.get();
        printf("Dssi: FIFO\n");
        }
  */
  
  snd_seq_event_t events[nevents];
  memset(events, 0, sizeof(events));
  nevents = 0;

  //int curPos      = pos;
  //unsigned endPos = pos + samples;
  unsigned endPos = pos + n;
  //int off         = pos;
  int frameOffset = audio->getFrameOffset();
  
  //iMidiEvent i = el->begin();
  iMPEvent i = el->begin();
  
  // Process event list events...
  for(; i != el->end(); ++i) 
  {
    //if(i->time() >= endPos)                // Doesn't work, at least here in muse-1. The event times are all 
                                            //  just slightly after the endPos, EVEN IF transport is stopped.
                                            // So it misses all the notes.
    if(i->time() >= (endPos + frameOffset))  // NOTE: frameOffset? Tested, examined printouts of times: Seems OK for playback.
      break;
      
    #ifdef DSSI_DEBUG 
    fprintf(stderr, "DssiSynthIF::getData eventlist event time:%d\n", i->time());
    #endif
    
    // p3.3.39 Update hardware state so knobs and boxes are updated. Optimize to avoid re-setting existing values.
    // Same code as in MidiPort::sendEvent()
    if(synti->midiPort() != -1)
    {
      MidiPort* mp = &midiPorts[synti->midiPort()];
      if(i->type() == ME_CONTROLLER) 
      {
        int da = i->dataA();
        int db = i->dataB();
        db = mp->limitValToInstrCtlRange(da, db);
        if(!mp->setHwCtrlState(i->channel(), da, db))
          continue;
        //mp->setHwCtrlState(i->channel(), da, db);
      }
      else
      if(i->type() == ME_PITCHBEND) 
      {
        int da = mp->limitValToInstrCtlRange(CTRL_PITCH, i->dataA());
        if(!mp->setHwCtrlState(i->channel(), CTRL_PITCH, da))
          continue;
        //mp->setHwCtrlState(i->channel(), CTRL_PITCH, da);
      }
      else
      if(i->type() == ME_PROGRAM) 
      {
        if(!mp->setHwCtrlState(i->channel(), CTRL_PROGRAM, i->dataA()))
          continue;
        //mp->setHwCtrlState(i->channel(), CTRL_PROGRAM, i->dataA());
      }
    }
        
    if(processEvent(*i, &events[nevents]))
      ++nevents;
  }
  
  // Now process putEvent events...
  while(!synti->putFifo.isEmpty()) 
  {
    MidiPlayEvent e = synti->putFifo.get();  
    
    #ifdef DSSI_DEBUG 
    fprintf(stderr, "DssiSynthIF::getData putFifo event time:%d\n", e.time());
    #endif
    
    // Set to the current time.
    // FIXME: FIXME: Wrong - we should be setting some kind of linear realtime wallclock here, not song pos.
    e.setTime(pos);
    if(processEvent(e, &events[nevents]))
      ++nevents;
  }
  
  // Now process OSC gui input control fifo events.
  // It is probably more important that these are processed last so that they take precedence over all other
  //  events because OSC + DSSI/DSSI-VST are fussy about receiving feedback via these control ports, from GUI changes.
  #ifdef OSC_SUPPORT
  unsigned long ctls = synth->_controlInPorts;
  for(unsigned long k = 0; k < ctls; ++k)
  {
    OscControlFifo* cfifo = _oscif.oscFifo(k);
    if(!cfifo)
      continue;
      
    // If there are 'events' in the fifo, get exactly one 'event' per control per process cycle...
    if(!cfifo->isEmpty()) 
    {
      OscControlValue v = cfifo->get();  
      
      #ifdef DSSI_DEBUG 
      fprintf(stderr, "DssiSynthIF::getData OscControlFifo event input control number:%ld value:%f\n", k, v.value);
      #endif
      
      // Set the ladspa control port value.
      controls[k].val = v.value;
      
      // TODO: (From plugin module, adapt for synth if/when our own plugin gui is added to synths).
      // Need to update the automation value, otherwise the block above overwrites with the last automation value.
      ///if(_track)
      ///{
        // Since we are now in the audio thread context, there's no need to send a message,
        //  just modify directly.
        //audio->msgSetPluginCtrlVal(this, genACnum(_id, i), controls[i].val);
      ///  _track->setPluginCtrlVal(k, v.value)
      ///}  
    }
  }  
  #endif
  
/*  // This is from MESS... Tried this here, didn't work, need to re-adapt, try again.
    int evTime = i->time(); 
    if(evTime == 0) 
    {
      printf("DssiSynthIF::getData - time is 0!\n");
      //continue;
      evTime=frameOffset; // will cause frame to be zero, problem?
    }
    
    int frame = evTime - frameOffset;
      
    if(frame >= endPos) 
    {
      printf("DssiSynthIF::getData frame > endPos!! frame = %d >= endPos %d, i->time() %d, frameOffset %d curPos=%d\n", frame, endPos, i->time(), frameOffset,curPos);
      continue;
    }
    
    if(frame > curPos) 
    {
      if(frame < pos)
        printf("DssiSynthIF::getData should not happen: missed event %d\n", pos -frame);
      else 
      {
*/        
      
/*     
      }
      curPos = frame;
    }  
*/  
//  }

  el->erase(el->begin(), i);
  //END: Process midi events
  
  //BEGIN: Run the synth
  // All ports must be connected to something!
  
  // First, copy the given input buffers to our local input buffers.
  unsigned long np, k;
  //np = portsin > synth->_inports ? synth->_inports : portsin;
  //for(k = 0; k < np; ++k)
  //  memcpy(audioInBuffers[k], inbuffer[k], sizeof(float) * n);
  //for(; k < portsin; ++k)
  //  memset(audioInBuffers[k], 0, sizeof(float) * n);
  
  // Watch our limits.
  np = ports > synth->_outports ? synth->_outports : ports;
  
  const DSSI_Descriptor* dssi = synth->dssi;
  const LADSPA_Descriptor* descr = dssi->LADSPA_Plugin;
  k = 0;
  // Connect the given buffers directly to the ports, up to a max of synth ports.
  for(; k < np; ++k)
    descr->connect_port(handle, synth->oIdx[k], buffer[k]);
  // Connect the remaining ports to some local buffers (not used yet).
  for(; k < synth->_outports; ++k)
    descr->connect_port(handle, synth->oIdx[k], audioOutBuffers[k]);
  
  /*
  //
  // p3.3.39 Handle inputs...
  //
  //if((song->bounceTrack != this) && !noInRoute()) 
  if(!((AudioTrack*)synti)->noInRoute()) 
  {
    RouteList* irl = ((AudioTrack*)synti)->inRoutes();
    iRoute i = irl->begin();
    if(!i->track->isMidiTrack())
    {
      //if(debugMsg)
        printf("DssiSynthIF::getData: Error: First route is a midi track route!\n");
    }
    else
    {
      int ch     = i->channel       == -1 ? 0 : i->channel;
      int remch  = i->remoteChannel == -1 ? 0 : i->remoteChannel;
      int chs    = i->channels      == -1 ? 0 : i->channels;
      
      // TODO:
      //if(ch >= synth->_inports)
      //iUsedIdx[ch] = true;
      //if(chs == 2)
      //  iUsedIdx[ch + 1] = true;
      
      //((AudioTrack*)i->track)->copyData(framePos, channels, nframe, bp);
      ((AudioTrack*)i->track)->copyData(pos, ports, 
                                      //(i->track->type() == Track::AUDIO_SOFTSYNTH && i->channel != -1) ? i->channel : 0, 
                                      i->channel, 
                                      i->channels,
                                      n, bp);
    }
    
    //unsigned pos, int ports, unsigned n, float** buffer    
    
    ++i;
    for(; i != irl->end(); ++i)
    {
      if(i->track->isMidiTrack())
      {
        //if(debugMsg)
          printf("DssiSynthIF::getData: Error: Route is a midi track route!\n");
        continue;
      }
      //((AudioTrack*)i->track)->addData(framePos, channels, nframe, bp);
      ((AudioTrack*)i->track)->addData(framePos, channels, 
                                        //(i->track->type() == Track::AUDIO_SOFTSYNTH && i->channel != -1) ? i->channel : 0, 
                                        i->channel, 
                                        i->channels,
                                        nframe, bp);
    }
  }  
  */  
    
  //#ifdef DSSI_DEBUG 
  //if(nevents)
  //  fprintf(stderr, "DssiSynthIF::getData run nevents:%d\n", nevents);
  //#endif
  
  // Run the synth for one segment. This processes events and gets/fills our local buffers...
  if(synth->dssi->run_synth)
  {
    synth->dssi->run_synth(handle, n, events, nevents);
    
    // NOTE: Just a test
    //for(int m = 0; m < n; ++m)
    //{
    //  synth->dssi->run_synth(handle, 1, events, nevents);
    //}  

  }  
  else if (synth->dssi->run_multiple_synths) 
  {
    snd_seq_event_t* ev = events;
    synth->dssi->run_multiple_synths(1, &handle, n, &ev, &nevents);
  }
  //END: Run the synth
  
  return i;
}

//---------------------------------------------------------
//   putEvent
//---------------------------------------------------------

//bool DssiSynthIF::putEvent(const MidiEvent& ev)
bool DssiSynthIF::putEvent(const MidiPlayEvent& ev)
      {
      #ifdef DSSI_DEBUG 
      fprintf(stderr, "DssiSynthIF::putEvent midi event time:%d chn:%d a:%d b:%d\n", ev.time(), ev.channel(), ev.dataA(), ev.dataB());
      #endif
      
      if (midiOutputTrace)
            ev.dump();
      
      return synti->putFifo.put(ev);
      
      //return false;
      }


//---------------------------------------------------------
//   incInstances
//---------------------------------------------------------

void DssiSynth::incInstances(int val)
{
      _instances += val;
      if (_instances == 0) 
      {
            if (handle)
            {
              #ifdef DSSI_DEBUG 
              fprintf(stderr, "DssiSynth::incInstances no more instances, closing library\n");
              #endif
              
              dlclose(handle);
            }
            handle = 0;
            dssi = NULL;
            df   = NULL;
            pIdx.clear(); 
            opIdx.clear();
            iIdx.clear(); 
            oIdx.clear(); 
            rpIdx.clear();
            iUsedIdx.clear();
            midiCtl2PortMap.clear();
            port2MidiCtlMap.clear();
            //synti->_guiUpdateControls.clear();
      }
}

//---------------------------------------------------------
//   initGui
//---------------------------------------------------------
bool DssiSynthIF::initGui()
{
      #ifdef OSC_SUPPORT
      return _oscif.oscInitGui();
      #endif
      
      return true;
      
      /*
      // Are we already running? We don't want to allow another process do we...
      if((guiQProc != 0) && (guiQProc->isRunning()))
        return true;
        
      //
      //  start gui
      //
      static char oscUrl[1024];
      //snprintf(oscUrl, 1024, "%s/%s", url, synti->name().toAscii().data());
      //snprintf(oscUrl, 1024, "%s/%s", url, synti->name().latin1());
      snprintf(oscUrl, 1024, "%s/%s/%s", url, synth->info.baseName().latin1(), synti->name().latin1());

      //QString guiPath(info.path() + "/" + info.baseName());
      QString guiPath(synth->info.dirPath() + "/" + synth->info.baseName());

      QDir guiDir(guiPath, "*", QDir::Unsorted, QDir::Files);
      if (guiDir.exists()) 
      {
            //const QFileInfoList list = guiDir.entryInfoList();
            QStringList list = guiDir.entryList();
            
            //for (int i = 0; i < list.size(); ++i) {
            for (unsigned int i = 0; i < list.count(); ++i) 
            {
                
                //QFileInfo fi = list.at(i);
                QFileInfo fi(guiPath + QString("/") + list[i]);
                  
                  QString gui(fi.filePath());
                  if (gui.contains('_') == 0)
                        continue;
                  struct stat buf;
                  
                  //if (stat(gui.toAscii().data(), &buf)) {
                  if (stat(gui.latin1(), &buf)) {
                  
                        perror("stat failed");
                        continue;
                        }

                  #ifdef DSSI_DEBUG 
                  fprintf(stderr, "DssiSynthIF::initGui  %s %s %s %s\n",
                      //fi.filePath().toAscii().data(),
                      //fi.fileName().toAscii().data(),
                      fi.filePath().latin1(),
                      //fi.fileName().latin1(),
                      
                      oscUrl,
                      
                      synth->info.filePath().latin1(),
                      
                      //name().toAscii().data(),
                      synth->name().latin1());
                  #endif
                      
                  if ((S_ISREG(buf.st_mode) || S_ISLNK(buf.st_mode)) &&
                     (buf.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))) 
                  {
                        // Changed by T356.
                        // fork + execlp were causing the processes to remain after closing gui, requiring manual kill.
                        // Changed to QProcess, works OK now. 
                        //if((guiPid = fork()) == 0) 
                        {
                              // No QProcess created yet? Do it now. Only once per SynthIF instance. Exists until parent destroyed.
                              if(guiQProc == 0)
                                guiQProc = new QProcess(muse);                        
                              
                              // Don't forget this, he he...
                              guiQProc->clearArguments();
                              
                              guiQProc->addArgument(fi.filePath());
                              //guiQProc->addArgument(fi.fileName()); // No conventional 'Arg0' here.
                              guiQProc->addArgument(QString(oscUrl));
                              guiQProc->addArgument(synth->info.filePath());
                              guiQProc->addArgument(synth->name());
                              guiQProc->addArgument(QString("channel 1"));
                              
                              #ifdef DSSI_DEBUG 
                              fprintf(stderr, "DssiSynthIF::initGui starting QProcess\n");
                              #endif
                                
                              if(guiQProc->start() == TRUE)
                              {
                                #ifdef DSSI_DEBUG 
                                fprintf(stderr, "DssiSynthIF::initGui started QProcess\n");
                                #endif
                                
                                //guiPid = guiQProc->processIdentifier();
                              }
                              else
                              {
                              
                                // execlp(
                                        // fi.filePath().toAscii().data(),
                                        // fi.fileName().toAscii().data(),
                                //        fi.filePath().latin1(),
                                //        fi.fileName().latin1(),
                                        
                                //        oscUrl,
                                        
                                        // info.filePath().toAscii().data(),
                                        // name().toAscii().data(),
                                //        synth->info.filePath().latin1(),
                                //        synth->name().latin1(),
                                        
                                //        "channel 1", (void*)0);
                                        
                                fprintf(stderr, "exec %s %s %s %s failed: %s\n",
                                        // fi.filePath().toAscii().data(),
                                        // fi.fileName().toAscii().data(),
                                        fi.filePath().latin1(),
                                        fi.fileName().latin1(),
                                        oscUrl,
                                        //  name().toAscii().data(),
                                        synth->name().latin1(),
                                        strerror(errno));
                                        
                                // It's Ok, Keep going. So nothing happens. So what. The timeout in showGui will just leave.
                                // Maybe it's a 'busy' issue somewhere - allow to try again later + save work now.
                                // exit(1);
                                
                              }
                              
                              #ifdef DSSI_DEBUG 
                              fprintf(stderr, "DssiSynthIF::initGui after QProcess\n");
                              #endif
                        }
                  }
            }
            //synth->_hasGui = true;
      }
      else {
            printf("%s: no dir for dssi gui found: %s\n",
               //name().toAscii().data(), guiPath.toAscii().data());
               synth->name().latin1(), guiPath.latin1());
            
            //synth->_hasGui = false;
            }
            
  return true;          
  */
}

//---------------------------------------------------------
//   guiHeartBeat
//---------------------------------------------------------

void DssiSynthIF::guiHeartBeat()
{
  #ifdef OSC_SUPPORT
  // Update the gui's program if needed.
  if(synti->_guiUpdateProgram)
  {
    _oscif.oscSendProgram(synti->_curProgram, synti->_curBankL);
    synti->_guiUpdateProgram = false;
  }
  
  // Update the gui's controls if needed.
  unsigned long ports = synth->_controlInPorts;
  if(ports > synti->_guiUpdateControls.size())
    return;
  for(unsigned long i = 0; i < ports; ++i)
  {
    if(synti->_guiUpdateControls[i])
    {
      unsigned long k = synth->pIdx[i];
      _oscif.oscSendControl(k, controls[i].val);
    
      // Reset.
      synti->_guiUpdateControls[i] = false;
    }
  }
  #endif
}

#ifdef OSC_SUPPORT
//---------------------------------------------------------
//   oscUpdate
//---------------------------------------------------------

int DssiSynthIF::oscUpdate()
{
      // Send project directory.
      _oscif.oscSendConfigure(DSSI_PROJECT_DIRECTORY_KEY, museProject.latin1());  // song->projectPath()
      
      // Send current string configuration parameters.
      //StringParamMap& map = synti->_stringParamMap;
      int i = 0;
      for(ciStringParamMap r = synti->_stringParamMap.begin(); r != synti->_stringParamMap.end(); ++r) 
      {
        _oscif.oscSendConfigure(r->first.c_str(), r->second.c_str());
        // Avoid overloading the GUI if there are lots and lots of params. 
        if((i+1) % 50 == 0)
          usleep(300000);
        ++i;      
      }  
      
      // Send current bank and program.
      //unsigned long bank, prog;
      //synti->currentProg(&prog, &bank, 0);
      //_oscif.oscSendProgram(prog, bank);
      _oscif.oscSendProgram(synti->_curProgram, synti->_curBankL);
      
      // Send current control values.
      unsigned long ports = synth->_controlInPorts;
      for(unsigned long i = 0; i < ports; ++i) 
      {
        unsigned long k = synth->pIdx[i];
        _oscif.oscSendControl(k, controls[i].val);
        // Avoid overloading the GUI if there are lots and lots of ports. 
        if((i+1) % 50 == 0)
          usleep(300000);
      }
      
      
#if 0
      /* Send current bank/program  (-FIX- another race...) */
      if (instance->pendingProgramChange < 0) {
            unsigned long bank = instance->currentBank;
            unsigned long program = instance->currentProgram;
            instance->uiNeedsProgramUpdate = 0;
            if (instance->uiTarget) {
                  lo_send(instance->uiTarget, instance->ui_osc_program_path, "ii", bank, program);
                  }
            }

      /* Send control ports */
      for (i = 0; i < instance->plugin->controlIns; i++) {
            int in = i + instance->firstControlIn;
            int port = pluginControlInPortNumbers[in];
            lo_send(instance->uiTarget, instance->ui_osc_control_path, "if", port,
               pluginControlIns[in]);
            /* Avoid overloading the GUI if there are lots and lots of ports */
            if ((i+1) % 50 == 0)
                  usleep(300000);
            }
#endif
      return 0;
}

//---------------------------------------------------------
//   oscProgram
//---------------------------------------------------------

int DssiSynthIF::oscProgram(unsigned long program, unsigned long bank)
      {
      //int bank    = argv[0]->i;
      //int program = argv[1]->i;
      
      int ch      = 0;        // TODO: ??
      
      int port    = synti->midiPort();        
      
      //_curBank = bank;
      //_curProgram = program;
      synti->_curBankH = 0;
      synti->_curBankL = bank;
      synti->_curProgram = program;
      
      bank    &= 0xff;
      program &= 0xff;
      
      //MidiEvent event(0, ch, ME_CONTROLLER, CTRL_PROGRAM, (bank << 8) + program);
      
      if(port != -1)
      {
        //MidiPlayEvent event(0, port, ch, ME_CONTROLLER, CTRL_PROGRAM, (bank << 8) + program);
        MidiPlayEvent event(0, port, ch, ME_PROGRAM, (bank << 8) + program, 0);
      
        #ifdef DSSI_DEBUG 
        fprintf(stderr, "DssiSynthIF::oscProgram midi event chn:%d a:%d b:%d\n", event.channel(), event.dataA(), event.dataB());
        #endif
        
        midiPorts[port].sendEvent(event);
      }
      
      
      
      //synti->playMidiEvent(&event); // TODO
      //
      //MidiDevice* md = dynamic_cast<MidiDevice*>(synti);
      //if(md)
      //  md->putEvent(event);
      //
      //synti->putEvent(event); 
      
      return 0;
      }

//---------------------------------------------------------
//   oscControl
//---------------------------------------------------------

int DssiSynthIF::oscControl(unsigned long port, float value)
      {
  //int port = argv[0]->i;
  //LADSPA_Data value = argv[1]->f;

  #ifdef DSSI_DEBUG 
  printf("DssiSynthIF::oscControl received oscControl port:%ld val:%f\n", port, value);
  #endif
  
  //int controlPorts = synth->_controlInPorts;
  
  //if(port >= controlPorts)
  //if(port < 0 || port >= synth->rpIdx.size())
  if(port >= synth->rpIdx.size())
  {
    //fprintf(stderr, "DssiSynthIF::oscControl: port number:%d is out of range of number of ports:%d\n", port, controlPorts);
    fprintf(stderr, "DssiSynthIF::oscControl: port number:%ld is out of range of index list size:%d\n", port, synth->rpIdx.size());
    return 0;
  }
  
  // Convert from DSSI port number to control input port index.
  unsigned long cport = synth->rpIdx[port];
  
  if((int)cport == -1)
  {
    fprintf(stderr, "DssiSynthIF::oscControl: port number:%ld is not a control input\n", port);
    return 0;
  }
  
  // p3.3.39 Set the DSSI control input port's value.
  // Observations: With a native DSSI synth like LessTrivialSynth, the native GUI's controls do not change the sound at all
  //  ie. they don't update the DSSI control port values themselves. 
  // Hence in response to the call to this oscControl, sent by the native GUI, it is required to that here.
///  controls[cport].val = value;
  // DSSI-VST synths however, unlike DSSI synths, DO change their OWN sound in response to their gui controls.
  // AND this function is called. 
  // Despite the descrepency we are STILL required to update the DSSI control port values here 
  //  because dssi-vst is WAITING FOR A RESPONSE. (A CHANGE in the control port value). 
  // It will output something like "...4 events expected..." and count that number down as 4 actual control port value CHANGES
  //  are done here in response. Normally it says "...0 events expected..." when MusE is the one doing the DSSI control changes.
  // TODO: (Done) May need FIFOs on each control(!) so that the control changes get sent one per process cycle. 
  // Observed countdown not actually going to zero upon string of changes.
  //
  // NOTE: NOTE: This line in RemoteVSTServer::setParameter(int p, float v) in dssi-vst-server.cpp :
  //
  //  " if (tv.tv_sec > m_lastGuiComms.tv_sec + 10) "
  //
  //  explains an observation that after ten seconds, the server automatically clears the expected number to 0.
  // TODO: Now MusE should forget about all the VST fifo events past ten+ (?) seconds. Add event timestamps...
  // You can't send any 'new' values until either you a): send all the expected events or b): wait ten seconds.
  // (Because the server simply ignores the 'expected' messages.)
  //
  // Well, at least here are the fifos. Try this ...
  OscControlFifo* cfifo = _oscif.oscFifo(cport); 
  if(cfifo)
  {
    OscControlValue cv;
    //cv.idx = cport;
    cv.value = value;
    if(cfifo->put(cv))
    {
      fprintf(stderr, "DssiSynthIF::oscControl: fifo overflow: in control number:%ld\n", cport);
    }
  }
   
  //const DSSI_Descriptor* dssi = synth->dssi;
  //const LADSPA_Descriptor* ld = dssi->LADSPA_Plugin;
  
  ciMidiCtl2LadspaPort ip = synth->port2MidiCtlMap.find(cport);
  if(ip != synth->port2MidiCtlMap.end())
  {
    // TODO: TODO: Update midi MusE's midi controller knobs, sliders, boxes etc with a call to the midi port's setHwCtrlState() etc.
    // But first we need a ladspa2MidiValue() function!  ... 
    //
    //
    //float val = ladspa2MidiValue(ld, i, ?, ?); 
  
  }

#if 0
      int port = argv[0]->i;
      LADSPA_Data value = argv[1]->f;

      if (port < 0 || port > instance->plugin->descriptor->LADSPA_Plugin->PortCount) {
            fprintf(stderr, "MusE: OSC: %s port number (%d) is out of range\n",
               instance->friendly_name, port);
            return 0;
            }
      if (instance->pluginPortControlInNumbers[port] == -1) {
            fprintf(stderr, "MusE: OSC: %s port %d is not a control in\n",
               instance->friendly_name, port);
            return 0;
            }
      pluginControlIns[instance->pluginPortControlInNumbers[port]] = value;
      if (verbose) {
            printf("MusE: OSC: %s port %d = %f\n",
               instance->friendly_name, port, value);
            }
#endif
      return 0;
      }

/*
//---------------------------------------------------------
//   oscExiting
//---------------------------------------------------------

int DssiSynthIF::oscExiting(lo_arg**)
      {
      //printf("not impl.: oscExiting\n");
      
      // The gui is gone now, right?
      _guiVisible = false;
      
      //const DSSI_Descriptor* dssi = synth->dssi;
      //const LADSPA_Descriptor* ld = dssi->LADSPA_Plugin;
      //if(ld->deactivate) 
      //  ld->deactivate(handle);
      
      if (uiOscPath == 0) {
            printf("DssiSynthIF::oscExiting(): no uiOscPath\n");
            return 1;
            }
      char uiOscGuiPath[strlen(uiOscPath)+6];
        
      sprintf(uiOscGuiPath, "%s/%s", uiOscPath, "quit");
      #ifdef DSSI_DEBUG 
      printf("DssiSynthIF::oscExiting(): sending quit to uiOscGuiPath:%s\n", uiOscGuiPath);
      #endif
      
      lo_send(uiTarget, uiOscGuiPath, "");
      
#if 0
      int i;

      if (verbose) {
            printf("MusE: OSC: got exiting notification for instance %d\n",
               instance->number);
            }

      if (instance->plugin) {

            // !!! No, this isn't safe -- plugins deactivated in this way
            //  would still be included in a run_multiple_synths call unless
            //  we re-jigged the instance array at the same time -- leave it
            //  for now
            //if (instance->plugin->descriptor->LADSPA_Plugin->deactivate) {
            //      instance->plugin->descriptor->LADSPA_Plugin->deactivate
            //         (instanceHandles[instance->number]);
            //      }
            // Leave this flag though, as we need it to determine when to exit 
            instance->inactive = 1;
            }

      // Do we have any plugins left running? 

      for (i = 0; i < instance_count; ++i) {
            if (!instances[i].inactive)
                  return 0;
            }

      if (verbose) {
            printf("MusE: That was the last remaining plugin, exiting...\n");
            }
      exiting = 1;
#endif
      return 0;
      }
*/

//---------------------------------------------------------
//   oscMidi
//---------------------------------------------------------

int DssiSynthIF::oscMidi(int a, int b, int c)
      {
      //int a = argv[0]->m[1];
      //int b = argv[0]->m[2];
      //int c = argv[0]->m[3];
      
      if (a == ME_NOTEOFF) {
            a = ME_NOTEON;
            c = 0;
            }
      int channel = 0;        // TODO: ??
      
      int port    = synti->midiPort();        
      
      //MidiEvent event(0, channel, a, b, c);
      
      if(port != -1)
      {
        MidiPlayEvent event(0, port, channel, a, b, c);
      
        #ifdef DSSI_DEBUG 
        fprintf(stderr, "DssiSynthIF::oscMidi midi event chn:%d a:%d b:%d\n", event.channel(), event.dataA(), event.dataB());
        #endif
        
        midiPorts[port].sendEvent(event);
      }
      
      //synti->playMidiEvent(&event); // TODO
      //
      //MidiDevice* md = dynamic_cast<MidiDevice*>(synti);
      //if(md)
      //  md->putEvent(event);
      //
      //synti->putEvent(event); 
      //
      
      return 0;
      }

//---------------------------------------------------------
//   oscConfigure
//---------------------------------------------------------

int DssiSynthIF::oscConfigure(const char *key, const char *value)
      {
      //const char *key = (const char *)&argv[0]->s;
      //const char *value = (const char *)&argv[1]->s;

      // This is pretty much the simplest legal implementation of
      // configure in a DSSI host. 

      // The host has the option to remember the set of (key,value)
      // pairs associated with a particular instance, so that if it
      // wants to restore the "same" instance on another occasion it can
      // just call configure() on it for each of those pairs and so
      // restore state without any input from a GUI.  Any real-world GUI
      // host will probably want to do that.  This host doesn't have any
      // concept of restoring an instance from one run to the next, so
      // we don't bother remembering these at all. 

      #ifdef DSSI_DEBUG 
      printf("DssiSynthIF::oscConfigure synth name:%s key:%s value:%s\n", synti->name().latin1(), key, value);
      #endif
      
      // Add or modify the configuration map item.
      synti->_stringParamMap.set(key, value);
      
      if (!strncmp(key, DSSI_RESERVED_CONFIGURE_PREFIX,
         strlen(DSSI_RESERVED_CONFIGURE_PREFIX))) {
            fprintf(stderr, "MusE: OSC: UI for plugin '%s' attempted to use reserved configure key \"%s\", ignoring\n",
               //synti->name().toAscii().data(), key);
               synti->name().latin1(), key);
               
            return 0;
            }

      if (!synth->dssi->configure)
            return 0;

      char* message = synth->dssi->configure(handle, key, value);
      if (message) {
            printf("MusE: on configure '%s' '%s', plugin '%s' returned error '%s'\n",
               //key, value, synti->name().toAscii().data(), message);
               key, value, synti->name().latin1(), message);
            
            free(message);
            }

      // also call back on UIs for plugins other than the one
      // that requested this:
      // if (n != instance->number && instances[n].uiTarget) {
      //      lo_send(instances[n].uiTarget,
      //      instances[n].ui_osc_configure_path, "ss", key, value);
      //      }

      // configure invalidates bank and program information, so
      //  we should do this again now: 
      queryPrograms();
      return 0;
      }
#endif // OSC_SUPPORT

//---------------------------------------------------------
//   queryPrograms
//---------------------------------------------------------

void DssiSynthIF::queryPrograms()
      {
      for (std::vector<DSSI_Program_Descriptor>::const_iterator i = programs.begin();
         i != programs.end(); ++i) {
            free((void*)(i->Name));
            }
      programs.clear();

      //if (!(synth->dssi->get_program && synth->dssi->select_program))
      if (!synth->dssi->get_program)
            return;

      for (int i = 0;; ++i) {
            const DSSI_Program_Descriptor* pd = synth->dssi->get_program(handle, i);
            if (pd == 0)
                  break;
            DSSI_Program_Descriptor d;
            d.Name    = strdup(pd->Name);
            d.Program = pd->Program;
            d.Bank    = pd->Bank;
            programs.push_back(d);
            }
      }

//---------------------------------------------------------
//   getPatchName
//---------------------------------------------------------

//QString DssiSynthIF::getPatchName(int, int prog)
const char* DssiSynthIF::getPatchName(int /*chan*/, int prog, MType /*type*/, bool /*drum*/)
      {
      unsigned program = prog & 0x7f;
      int lbank   = (prog >> 8) & 0xff;
      int hbank   = (prog >> 16) & 0xff;

      if (lbank == 0xff)
            lbank = 0;
      if (hbank == 0xff)
            hbank = 0;
      unsigned bank = (hbank << 8) + lbank;

      for (std::vector<DSSI_Program_Descriptor>::const_iterator i = programs.begin();
         i != programs.end(); ++i) {
            if (i->Bank == bank && i->Program ==program)
                  return i->Name;
            }
      return "?";
      }

//---------------------------------------------------------
//   populatePatchPopup
//---------------------------------------------------------

//void DssiSynthIF::populatePatchPopup(QMenu* menu, int)
void DssiSynthIF::populatePatchPopup(Q3PopupMenu* menu, int /*ch*/, MType /*type*/, bool /*drum*/)
      {
      // The plugin can change the programs, patches etc.
      // So make sure we're up to date by calling queryPrograms.
      queryPrograms();
      
      menu->clear();

      for (std::vector<DSSI_Program_Descriptor>::const_iterator i = programs.begin();
         i != programs.end(); ++i) {
            int bank = i->Bank;
            int prog = i->Program;
            int id   = (bank << 16) + prog;
            
            //QAction* a = menu->addAction(QString(i->Name));
            //a->setData(id);
            menu->insertItem(QString(i->Name), id);
            }
      }

int DssiSynthIF::getControllerInfo(int id, const char** name, int* ctrl, int* min, int* max, int* initval)
{
  int controlPorts = synth->_controlInPorts;
  if(id >= controlPorts)
  //if(id >= midiCtl2PortMap.size())
    return 0;

  const DSSI_Descriptor* dssi = synth->dssi;
  const LADSPA_Descriptor* ld = dssi->LADSPA_Plugin;
  
  // Hmm, <map> has a weird [] operator. Would it work?
  // For now just use duplicate code found in ::init()
  //iMidiCtl2LadspaPort ip = midiCtl2PortMap[id];
  //int ctlnum = ip->first;
  //int k = ip->second;
  
  int i = synth->pIdx[id];
  //int i = synth->pIdx[k];
  
  //ladspaDefaultValue(ld, i, &controls[id].val);
  
  #ifdef DSSI_DEBUG 
  printf("DssiSynthIF::getControllerInfo control port:%d port idx:%d name:%s\n", id, i, ld->PortNames[i]);
  #endif
  
  int ctlnum = DSSI_NONE;
  if(dssi->get_midi_controller_for_port)
    ctlnum = dssi->get_midi_controller_for_port(handle, i);
  
  
  // No controller number? Give it one.
  if(ctlnum == DSSI_NONE)
  {
    // TODO: If neccesary... choose non-existing numbers...
    //for(int k = 0; k < controlPorts; ++k) 
    //{
    //  int i = synth->pIdx[k];
    //}
    
    // Simple but flawed solution: Start them at 0x60000 + 0x2000 = 0x62000. Max NRPN number is 0x3fff.
    ctlnum = CTRL_NRPN14_OFFSET + 0x2000 + id;
  }
  else
  {
    #ifdef DSSI_DEBUG 
    printf("DssiSynthIF::getControllerInfo ctlnum:%d\n", ctlnum);
    #endif
     
    int c = ctlnum;
    // Can be both CC and NRPN! Prefer CC over NRPN.
    if(DSSI_IS_CC(ctlnum))
    {
      #ifdef DSSI_DEBUG 
      printf("DssiSynthIF::getControllerInfo is CC control\n");
      #endif
      
      ctlnum = DSSI_CC_NUMBER(c);
      
      #ifdef DSSI_DEBUG 
      if(DSSI_IS_NRPN(ctlnum))
        printf("DssiSynthIF::getControllerInfo is also NRPN control. Using CC.\n");
      #endif  
    }
    else
    if(DSSI_IS_NRPN(ctlnum))
    {
      #ifdef DSSI_DEBUG 
      printf("DssiSynthIF::getControllerInfo is NRPN control\n");
      #endif
      
      ctlnum = DSSI_NRPN_NUMBER(c) + CTRL_NRPN14_OFFSET;
    }  
  }
  
  int def = CTRL_VAL_UNKNOWN;
  if(ladspa2MidiControlValues(ld, i, ctlnum, min, max, &def))
    *initval = def;
  else
    *initval = CTRL_VAL_UNKNOWN;
    
  #ifdef DSSI_DEBUG 
  printf("DssiSynthIF::getControllerInfo passed ctlnum:%d min:%d max:%d initval:%d\n", ctlnum, *min, *max, *initval);
  #endif
  
  *ctrl = ctlnum;
  *name =  ld->PortNames[i];
  return ++id;

  /*
  // ...now create midi controllers for ports which did not define them ...
  for(int k = 0; k < controlPorts; ++k) 
  {
    int i = synth->pIdx[k];
    //controls[k].val = ladspaDefaultValue(ld, i);
    ladspaDefaultValue(ld, i, &controls[k].val);
    
    printf("DssiSynthIF::getControllerInfo #2 control port:%d port idx:%d name:%s\n", k, i, ld->PortNames[i]);
    
    if(!dssi->get_midi_controller_for_port || (dssi->get_midi_controller_for_port(handle, i) == DSSI_NONE))
    {
      int ctlnum;
      //printf("DssiSynthIF::getControllerInfo #2 midi controller number:%d\n", ctlnum);
      printf("DssiSynthIF::getControllerInfo #2 creating MidiController number:%d\n", ctlnum);
      MidiController* mc = ladspa2MidiController(ld, i, ctlnum); 
      // Add to MidiInstrument controller list.
      if(mc)
      {
        printf("DssiSynthIF::getControllerInfo #2 adding MidiController to instrument\n");
        ((MidiInstrument*)synti)->controller()->add(mc);
      }  
    }
    else
    {
    
    }
  }
  */

}

int DssiSynthIF::channels() const 
{ 
  return synth->_outports > MAX_CHANNELS ? MAX_CHANNELS : synth->_outports; 
}

int DssiSynthIF::totalOutChannels() const 
{ 
  return synth->_outports; 
}

int DssiSynthIF::totalInChannels() const 
{ 
  return synth->_inports; 
}

//--------------------------------
// Methods for PluginIBase:
//--------------------------------

bool DssiSynthIF::on() const                                 { return true; }  // Synth is not part of a rack plugin chain. Always on.
void DssiSynthIF::setOn(bool /*val*/)                        { }   
int DssiSynthIF::pluginID()                                  { return (synth && synth->dssi) ? synth->dssi->LADSPA_Plugin->UniqueID : 0; } 
int DssiSynthIF::id()                                        { return 0; } // Synth is not part of a rack plugin chain. Always 0.
QString DssiSynthIF::pluginLabel() const                     { return (synth && synth->dssi) ? QString(synth->dssi->LADSPA_Plugin->Label) : QString(); } 
QString DssiSynthIF::name() const                            { return synti->name(); }
AudioTrack* DssiSynthIF::track()                             { return (AudioTrack*)synti; }
void DssiSynthIF::enableController(int i, bool v)            { controls[i].enCtrl = v; } 
bool DssiSynthIF::controllerEnabled(int i) const             { return controls[i].enCtrl; }  
bool DssiSynthIF::controllerEnabled2(int i) const            { return controls[i].en2Ctrl; }   
void DssiSynthIF::updateControllers()                        { }
void DssiSynthIF::writeConfiguration(int /*level*/, Xml& /*xml*/)        { }
bool DssiSynthIF::readConfiguration(Xml& /*xml*/, bool /*readPreset*/) { return false; }
int DssiSynthIF::parameters() const                          { return synth ? synth->_controlInPorts : 0; }
void DssiSynthIF::setParam(int i, double val)                { setParameter(i, val); }
double DssiSynthIF::param(int i) const                       { return getParameter(i); }
const char* DssiSynthIF::paramName(int i)                    { return (synth && synth->dssi) ? synth->dssi->LADSPA_Plugin->PortNames[i] : 0; }
//LADSPA_PortRangeHint DssiSynthIF::range(int i)               { return (synth && synth->dssi) ? synth->dssi->LADSPA_Plugin->PortRangeHints[i] : 0; }
LADSPA_PortRangeHint DssiSynthIF::range(int i)               { return synth->dssi->LADSPA_Plugin->PortRangeHints[i]; }


#else //DSSI_SUPPORT
void initDSSI() {}
#endif

