/********************************************************************************
*                                                                               *
*                     D i a l o g   B o x    O b j e c t                        *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXDialogBox.cpp,v 1.19 2002/01/18 22:42:59 jeroen Exp $                  *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxkeys.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXSettings.h"
#include "FXRegistry.h"
#include "FXApp.h"
#include "FXCursor.h"
#include "FXDialogBox.h"

/*

  To do:
  - Iconified/normal
  - Unmap when main window unmapped
  - Transient For stuff
  - Place so that cursor over dialog
  - Hitting ESC will cancel out of the dialog
  - Hitting RETURN will localize the default button, and then send it a RETURN;
    Note that the default button is initially assigned, but whichever button
    has the focus will be the default button; default-ness moves between buttons.
*/

/*******************************************************************************/


// Map
FXDEFMAP(FXDialogBox) FXDialogBoxMap[]={
  FXMAPFUNC(SEL_KEYPRESS,0,FXDialogBox::onKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,0,FXDialogBox::onKeyRelease),
  FXMAPFUNC(SEL_CLOSE,0,FXDialogBox::onClose),
  FXMAPFUNC(SEL_COMMAND,FXDialogBox::ID_CANCEL,FXDialogBox::onCmdCancel),
  FXMAPFUNC(SEL_COMMAND,FXDialogBox::ID_ACCEPT,FXDialogBox::onCmdAccept),
  };


// Object implementation
FXIMPLEMENT(FXDialogBox,FXTopWindow,FXDialogBoxMap,ARRAYNUMBER(FXDialogBoxMap))


// Contruct free floating dialog
FXDialogBox::FXDialogBox(FXApp* a,const FXString& name,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs):
  FXTopWindow(a,name,NULL,NULL,opts,x,y,w,h,pl,pr,pt,pb,hs,vs){
  }


// Contruct dialog which will stay on top of owner
FXDialogBox::FXDialogBox(FXWindow* owner,const FXString& name,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs):
  FXTopWindow(owner,name,NULL,NULL,opts,x,y,w,h,pl,pr,pt,pb,hs,vs){
  }


// Close window & cancel out of dialog
long FXDialogBox::onClose(FXObject*,FXSelector,void*){
  if(target && target->handle(this,MKUINT(message,SEL_CLOSE),NULL)) return 1;
  handle(this,MKUINT(FXDialogBox::ID_CANCEL,SEL_COMMAND),NULL);
  return 1;
  }


// Close dialog with an accept
long FXDialogBox::onCmdAccept(FXObject*,FXSelector,void*){
  getApp()->stopModal(this,TRUE);
  hide();
  return 1;
  }


// Close dialog with a cancel
long FXDialogBox::onCmdCancel(FXObject*,FXSelector,void*){
  getApp()->stopModal(this,FALSE);
  hide();
  return 1;
  }


// Keyboard press; handle escape to close the dialog
long FXDialogBox::onKeyPress(FXObject* sender,FXSelector sel,void* ptr){
  if(FXTopWindow::onKeyPress(sender,sel,ptr)) return 1;
  if(((FXEvent*)ptr)->code==KEY_Escape){
    handle(this,MKUINT(ID_CANCEL,SEL_COMMAND),NULL);
    return 1;
    }
  return 0;
  }


// Keyboard release; handle escape to close the dialog
long FXDialogBox::onKeyRelease(FXObject* sender,FXSelector sel,void* ptr){
  if(FXTopWindow::onKeyRelease(sender,sel,ptr)) return 1;
  if(((FXEvent*)ptr)->code==KEY_Escape){
    return 1;
    }
  return 0;
  }



// Execute dialog box modally
FXuint FXDialogBox::execute(FXuint placement){
  create();
  show(placement);
  return getApp()->runModalFor(this);
  }
