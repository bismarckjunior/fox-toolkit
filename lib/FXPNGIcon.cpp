/********************************************************************************
*                                                                               *
*                          P N G   I m a g e   O b j e c t                      *
*                                                                               *
*********************************************************************************
* Copyright (C) 1999,2015 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* This library is free software; you can redistribute it and/or modify          *
* it under the terms of the GNU Lesser General Public License as published by   *
* the Free Software Foundation; either version 3 of the License, or             *
* (at your option) any later version.                                           *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
* GNU Lesser General Public License for more details.                           *
*                                                                               *
* You should have received a copy of the GNU Lesser General Public License      *
* along with this program.  If not, see <http://www.gnu.org/licenses/>          *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXArray.h"
#include "FXHash.h"
#include "FXMutex.h"
#include "FXStream.h"
#include "FXMemoryStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXStringDictionary.h"
#include "FXSettings.h"
#include "FXRegistry.h"
#include "FXEvent.h"
#include "FXWindow.h"
#include "FXApp.h"
#include "FXPNGIcon.h"


/*
  Notes:
  - FXPNGIcon has an alpha channel
*/

using namespace FX;

/*******************************************************************************/

namespace FX {


// Suggested file extension
const FXchar FXPNGIcon::fileExt[]="png";


// Suggested mime type
const FXchar FXPNGIcon::mimeType[]="image/png";


// Object implementation
FXIMPLEMENT(FXPNGIcon,FXIcon,NULL,0)


#ifdef HAVE_PNG_H
const FXbool FXPNGIcon::supported=true;
#else
const FXbool FXPNGIcon::supported=false;
#endif


// Initialize
FXPNGIcon::FXPNGIcon(FXApp* a,const void *pix,FXColor clr,FXuint opts,FXint w,FXint h):FXIcon(a,NULL,clr,opts,w,h){
  if(pix){
    FXMemoryStream ms(FXStreamLoad,(FXuchar*)pix);
    loadPixels(ms);
    }
  }


// Save pixels only
FXbool FXPNGIcon::savePixels(FXStream& store) const {
  if(fxsavePNG(store,data,width,height)){
    return true;
    }
  return false;
  }


// Load pixels only
FXbool FXPNGIcon::loadPixels(FXStream& store){
  FXColor *pixels; FXint w,h;
  if(fxloadPNG(store,pixels,w,h)){
    setData(pixels,IMAGE_OWNED,w,h);
    if(options&IMAGE_ALPHAGUESS) setTransparentColor(guesstransp());
    if(options&IMAGE_THRESGUESS) setThresholdValue(guessthresh());
    return true;
    }
  return false;
  }


// Clean up
FXPNGIcon::~FXPNGIcon(){
  }

}
