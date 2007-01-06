//===========================================================================
//
//    ladspapandelay
//
//    Version 0.0.1
//
//
//
//
//  Copyright (c) 2006 Nil Geisweiller
//
//
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA or point your web browser to http://www.gnu.org.
//===========================================================================

#include "ladspapandelay.h"

//---------------------------------------------------------
//     PanDelay
//---------------------------------------------------------

LADSPAPanDelay::LADSPAPanDelay(unsigned long samplerate) 
  : PanDelayModel(samplerate) {
  //TODO init param 
}

LADSPAPanDelay::~LADSPAPanDelay() {
}

//---------------------------------------------------------
//   activate
//---------------------------------------------------------

void LADSPAPanDelay::activate() {
  *port[4] = param[0];
  *port[5] = param[1];
  *port[6] = param[2];
  *port[7] = param[3];
  *port[8] = param[4];
}

void LADSPAPanDelay::updateParameters() {
  if (param[0] != *port[4]) {
    param[0] = *port[4];
    setDelayTime(param[0]);
  }
  if (param[1] != *port[5]) {
    param[1] = *port[5];
    setFeedback(param[1]);
  }
  if (param[2] != *port[6]) {
    param[2] = *port[6];
    setPanLFOFreq(param[2]);
  }
  if (param[3] != *port[7]) {
    param[3] = *port[7];
    setPanLFODepth(param[3]);
  }
  if (param[4] != *port[8]) {
    param[4] = *port[8];
    setDryWet(param[4]);
  }
}

//---------------------------------------------------------
//   processReplace
//---------------------------------------------------------

void LADSPAPanDelay::processReplace(long n) {
  updateParameters();
  PanDelayModel::processReplace(port[0], port[1], port[2], port[3], n);
}

//---------------------------------------------------------
//   processMix
//---------------------------------------------------------

void LADSPAPanDelay::processMix(long n) {
  updateParameters();
  PanDelayModel::processMix(port[0], port[1], port[2], port[3], n);
}
