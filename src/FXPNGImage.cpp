/********************************************************************************
*                                                                               *
*                          P N G   I m a g e   O b j e c t                      *
*                                                                               *
*********************************************************************************
* Copyright (C) 1999,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* This library is free software; you can redistribute it and/or                 *
* modify it under the terms of the GNU Lesser General Public                    *
* License as published by the Free Software Foundation; either                  *
* version 2.1 of the License, or (at your option) any later version.            *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU             *
* Lesser General Public License for more details.                               *
*                                                                               *
* You should have received a copy of the GNU Lesser General Public              *
* License along with this library; if not, write to the Free Software           *
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.    *
*********************************************************************************
* $Id: FXPNGImage.cpp,v 1.14 2002/01/18 22:43:01 jeroen Exp $                   *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXRegistry.h"
#include "FXApp.h"
#include "FXPNGImage.h"



/*
  Notes:
  - FXPNGImage has an alpha channel
*/


/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXPNGImage,FXImage,NULL,0)


// Initialize
FXPNGImage::FXPNGImage(FXApp* a,const void *pix,FXuint opts,FXint w,FXint h):
  FXImage(a,NULL,opts|IMAGE_ALPHA,w,h){
  if(pix){
    FXMemoryStream ms;
    FXColor clearcolor;
    ms.open((FXuchar*)pix,FXStreamLoad);
    fxloadPNG(ms,data,clearcolor,width,height);
    options|=IMAGE_OWNED;
    ms.close();
    }
  }


// Save the pixels only
void FXPNGImage::savePixels(FXStream& store) const {
  FXColor clearcolor=FXRGB(192,192,192);
  FXASSERT(options&IMAGE_ALPHA);
  fxsavePNG(store,data,clearcolor,width,height);
  }



// Load pixels only
void FXPNGImage::loadPixels(FXStream& store){
  FXColor clearcolor;
  if(options&IMAGE_OWNED){FXFREE(&data);}
  fxloadPNG(store,data,clearcolor,width,height);
  options|=IMAGE_ALPHA;
  options|=IMAGE_OWNED;
  }


// Clean up
FXPNGImage::~FXPNGImage(){
  }
