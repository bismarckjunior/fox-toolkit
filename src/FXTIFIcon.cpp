/********************************************************************************
*                                                                               *
*                          T I F F  I c o n   O b j e c t                       *
*                                                                               *
*********************************************************************************
* Copyright (C) 2001,2002 Eric Gillet.   All Rights Reserved.                   *
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
* $Id: FXTIFIcon.cpp,v 1.6 2002/01/18 22:55:04 jeroen Exp $                     *
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
#include "FXTIFIcon.h"


/*
  Notes:
  - FXTIFIcon has an alpha channel.
*/


/*******************************************************************************/

FXIMPLEMENT(FXTIFIcon,FXIcon,NULL,0)


// Initialize
FXTIFIcon::FXTIFIcon(FXApp* a,const void *pix,FXColor clr,FXuint opts,FXint w,FXint h):
  FXIcon(a,NULL,clr,opts|IMAGE_ALPHA,w,h){
  codec=0;
  if(pix){
    FXMemoryStream ms;
    ms.open((FXuchar *)pix,FXStreamLoad);
    loadPixels(ms);
    ms.close();
    }
  }


// Save pixels only
void FXTIFIcon::savePixels(FXStream& store) const {
  FXASSERT(options&IMAGE_ALPHA);
  fxsaveTIF(store,data,transp,width,height,codec);
  }


// Load pixels only
void FXTIFIcon::loadPixels(FXStream& store){
  FXColor clearcolor=0;
  if(options&IMAGE_OWNED){FXFREE(&data);}
  fxloadTIF(store,data,clearcolor,width,height,codec);
  if(!(options&IMAGE_ALPHACOLOR)) transp=clearcolor;
  if(options&IMAGE_ALPHAGUESS) transp=guesstransp();
  options|=IMAGE_ALPHA;
  options|=IMAGE_OWNED;
  }


// Clean up
FXTIFIcon::~FXTIFIcon(){
  }
