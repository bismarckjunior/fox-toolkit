/********************************************************************************
*                                                                               *
*                       M e n u   T i t l e   W i d g e t                       *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXMenuTitle.cpp,v 1.26 2002/01/18 22:43:01 jeroen Exp $                  *
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
#include "FXRegistry.h"
#include "FXAccelTable.h"
#include "FXApp.h"
#include "FXDCWindow.h"
#include "FXFont.h"
#include "FXIcon.h"
#include "FXMenuPane.h"
#include "FXMenuTitle.h"

/*
  Notes:
  - Accelerators.
  - Help text from constructor is third part; second part should be
    accelerator key combination.
  - When menu label changes, hotkey might have to be adjusted.
  - Fix it so menu stays up when after Alt-F, you press Alt-E.
  - Menu items should be derived from FXLabel.
  - Look into SEL_FOCUS_SELF some more...
  - New:- under W2K, the underscores for the accelerators no longer
    show up until the ALT key is pressed [but not sure if I like this,
    as it makes the accelerator undiscoverable].
*/


/*******************************************************************************/

// Map
FXDEFMAP(FXMenuTitle) FXMenuTitleMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXMenuTitle::onPaint),
  FXMAPFUNC(SEL_ENTER,0,FXMenuTitle::onEnter),
  FXMAPFUNC(SEL_LEAVE,0,FXMenuTitle::onLeave),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXMenuTitle::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXMenuTitle::onLeftBtnRelease),
  FXMAPFUNC(SEL_MIDDLEBUTTONPRESS,0,FXMenuTitle::onDefault),
  FXMAPFUNC(SEL_MIDDLEBUTTONRELEASE,0,FXMenuTitle::onDefault),
  FXMAPFUNC(SEL_RIGHTBUTTONPRESS,0,FXMenuTitle::onDefault),
  FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,0,FXMenuTitle::onDefault),
  FXMAPFUNC(SEL_KEYPRESS,0,FXMenuTitle::onKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,0,FXMenuTitle::onKeyRelease),
  FXMAPFUNC(SEL_KEYPRESS,FXWindow::ID_HOTKEY,FXMenuTitle::onHotKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,FXWindow::ID_HOTKEY,FXMenuTitle::onHotKeyRelease),
  FXMAPFUNC(SEL_FOCUS_UP,0,FXMenuTitle::onFocusUp),
  FXMAPFUNC(SEL_FOCUS_DOWN,0,FXMenuTitle::onFocusDown),
  FXMAPFUNC(SEL_FOCUSIN,0,FXMenuTitle::onFocusIn),
  FXMAPFUNC(SEL_FOCUSOUT,0,FXMenuTitle::onFocusOut),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_POST,FXMenuTitle::onCmdPost),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_UNPOST,FXMenuTitle::onCmdUnpost),
  };


// Object implementation
FXIMPLEMENT(FXMenuTitle,FXMenuCaption,FXMenuTitleMap,ARRAYNUMBER(FXMenuTitleMap))



// Make a menu title button
FXMenuTitle::FXMenuTitle(FXComposite* p,const FXString& text,FXIcon* ic,FXPopup* pup,FXuint opts):
  FXMenuCaption(p,text,ic,opts){
  flags|=FLAG_ENABLED;
  textColor=getApp()->getForeColor();
  seltextColor=getApp()->getForeColor();
  selbackColor=getApp()->getBaseColor();
  pane=pup;
  }


// Create window
void FXMenuTitle::create(){
  FXMenuCaption::create();
  if(pane) pane->create();
  }


// Create window
void FXMenuTitle::detach(){
  FXMenuCaption::detach();
  if(pane) pane->detach();
  }


// Get default width
FXint FXMenuTitle::getDefaultWidth(){
  FXint tw,iw;
  tw=iw=0;
  if(!label.empty()) tw=font->getTextWidth(label.text(),label.length());
  if(icon) iw=icon->getWidth();
  if(iw && tw) iw+=5;
  return tw+iw+12;
  }


// Get default height
FXint FXMenuTitle::getDefaultHeight(){
  FXint th,ih;
  th=ih=0;
  if(!label.empty()) th=font->getFontHeight();
  if(icon) ih=icon->getHeight();
  return FXMAX(th,ih)+4;
  }


// If window can have focus
FXbool FXMenuTitle::canFocus() const {
  return 1;
  }


// When pressed, perform ungrab, then process normally
long FXMenuTitle::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONPRESS),ptr)) return 1;
    if(flags&FLAG_ACTIVE){
      handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),NULL);
      }
    else{
      handle(this,MKUINT(ID_POST,SEL_COMMAND),NULL);
      }
    return 1;
    }
  return 0;
  }


// Released left button
long FXMenuTitle::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONRELEASE),ptr)) return 1;
    if(event->moved){
      handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),ptr);
      }
    return 1;
    }
  return 0;
  }


// Keyboard press; forward to menu pane
long FXMenuTitle::onKeyPress(FXObject*,FXSelector sel,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  if(isEnabled()){
    FXTRACE((200,"%s::onKeyPress %p keysym=0x%04x state=%04x\n",getClassName(),this,event->code,event->state));
    if(target && target->handle(this,MKUINT(message,SEL_KEYPRESS),ptr)) return 1;
    if(pane && pane->shown() && pane->handle(pane,sel,ptr)) return 1;
    }
  return 0;
  }


// Keyboard release; forward to menu pane
long FXMenuTitle::onKeyRelease(FXObject*,FXSelector sel,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  if(isEnabled()){
    FXTRACE((200,"%s::onKeyRelease %p keysym=0x%04x state=%04x\n",getClassName(),this,event->code,event->state));
    if(target && target->handle(this,MKUINT(message,SEL_KEYRELEASE),ptr)) return 1;
    if(pane && pane->shown() && pane->handle(pane,sel,ptr)) return 1;
    }
  return 0;
  }


// Hot key combination pressed
long FXMenuTitle::onHotKeyPress(FXObject*,FXSelector,void* ptr){
  FXTRACE((200,"%s::onHotKeyPress %p\n",getClassName(),this));
  handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
  return 1;
  }


// Hot key combination released
long FXMenuTitle::onHotKeyRelease(FXObject*,FXSelector,void*){
  FXTRACE((200,"%s::onHotKeyRelease %p\n",getClassName(),this));
  if(isEnabled()){
    if(flags&FLAG_ACTIVE){
      handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),NULL);
      }
    else{
      handle(this,MKUINT(ID_POST,SEL_COMMAND),NULL);
      }
    }
  return 1;
  }


// Post the menu
long FXMenuTitle::onCmdPost(FXObject*,FXSelector,void*){
  FXint x,y;
  if(pane && !pane->shown()){
    translateCoordinatesTo(x,y,getRoot(),0,0);
    pane->popup(getParent(),x-1,y+height);
    if(!getParent()->grabbed()) getParent()->grab();
    }
  flags|=FLAG_ACTIVE;
  update();
  return 1;
  }


// Unpost the menu
long FXMenuTitle::onCmdUnpost(FXObject*,FXSelector,void*){
  if(pane && pane->shown()){
    pane->popdown();
    if(getParent()->grabbed()) getParent()->ungrab();
    }
  flags&=~FLAG_ACTIVE;
  update();
  return 1;
  }


// Focus moved down
long FXMenuTitle::onFocusDown(FXObject*,FXSelector,void*){
  if(pane && !pane->shown()){
    handle(this,MKUINT(ID_POST,SEL_COMMAND),NULL);
    return 1;
    }
  return 0;
  }


// Focus moved up
long FXMenuTitle::onFocusUp(FXObject*,FXSelector,void*){
  if(pane && pane->shown()){
    handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),NULL);
    return 1;
    }
  return 0;
  }


// Into focus chain
void FXMenuTitle::setFocus(){
  FXWindow *menuitem=getParent()->getFocus();
  FXbool active=menuitem && menuitem->isActive();
  FXMenuCaption::setFocus();
  if(active) handle(this,MKUINT(ID_POST,SEL_COMMAND),NULL);
  }



// Out of focus chain
void FXMenuTitle::killFocus(){
  FXMenuCaption::killFocus();
  handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),NULL);
  }


// Enter
long FXMenuTitle::onEnter(FXObject* sender,FXSelector sel,void* ptr){
  FXMenuCaption::onEnter(sender,sel,ptr);
  if(!isEnabled()) return 1;
  if(canFocus() && getParent()->getFocus()) setFocus();
  update();
  return 1;
  }


// Leave
long FXMenuTitle::onLeave(FXObject* sender,FXSelector sel,void* ptr){
  FXMenuCaption::onLeave(sender,sel,ptr);
  if(!isEnabled()) return 1;
  update();
  return 1;
  }


// Handle repaint
long FXMenuTitle::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent *ev=(FXEvent*)ptr;
  FXDCWindow dc(this,ev);
  FXint xx,yy;
  dc.setTextFont(font);
  xx=6;
  yy=0;
  if(isEnabled()){
    if(isActive()){
      dc.setForeground(selbackColor);
      dc.fillRectangle(1,1,width-2,height-2);
      //drawSunkenRectangle(dc,0,0,width,height);
      dc.setForeground(shadowColor);
      dc.fillRectangle(0,0,width,1);
      dc.fillRectangle(0,0,1,height);
      dc.setForeground(hiliteColor);
      dc.fillRectangle(0,height-1,width,1);
      dc.fillRectangle(width-1,0,1,height);
      xx++;
      yy++;
      }
    else if(underCursor()){
      dc.setForeground(backColor);
      dc.fillRectangle(1,1,width-2,height-2);
      //drawRaisedRectangle(dc,0,0,width,height);
      dc.setForeground(shadowColor);
      dc.fillRectangle(0,height-1,width,1);
      dc.fillRectangle(width-1,0,1,height);
      dc.setForeground(hiliteColor);
      dc.fillRectangle(0,0,width,1);
      dc.fillRectangle(0,0,1,height);
      }
    else{
      dc.setForeground(backColor);
      dc.fillRectangle(0,0,width,height);
      }
    if(icon){
      dc.drawIcon(icon,xx,yy+(height-icon->getHeight())/2);
      xx+=5+icon->getWidth();
      }
    if(!label.empty()){
      yy+=font->getFontAscent()+(height-font->getFontHeight())/2;
      dc.setForeground(isActive() ? seltextColor : textColor);
      dc.drawText(xx,yy,label.text(),label.length());
      if(0<=hotoff){
        dc.fillRectangle(xx+font->getTextWidth(&label[0],hotoff),yy+1,font->getTextWidth(&label[hotoff],1),1);
        }
      }
    }
  else{
    dc.setForeground(backColor);
    dc.fillRectangle(0,0,width,height);
    if(icon){
      dc.drawIconSunken(icon,xx,yy+(height-icon->getHeight())/2);
      xx+=5+icon->getWidth();
      }
    if(!label.empty()){
      yy+=font->getFontAscent()+(height-font->getFontHeight())/2;
      dc.setForeground(hiliteColor);
      dc.drawText(xx+1,yy+1,label.text(),label.length());
      if(0<=hotoff){
        dc.fillRectangle(xx+font->getTextWidth(&label[0],hotoff),yy+1,font->getTextWidth(&label[hotoff],1),1);
        }
      dc.setForeground(shadowColor);
      dc.drawText(xx,yy,label.text(),label.length());
      if(0<=hotoff){
        dc.fillRectangle(xx+font->getTextWidth(&label[0],hotoff),yy+1,font->getTextWidth(&label[hotoff],1),1);
        }
      }
    }
  return 1;
  }


// Gained focus
long FXMenuTitle::onFocusIn(FXObject* sender,FXSelector sel,void* ptr){
  FXMenuCaption::onFocusIn(sender,sel,ptr);
  update();
  return 1;
  }


// Lost focus
long FXMenuTitle::onFocusOut(FXObject* sender,FXSelector sel,void* ptr){
  FXMenuCaption::onFocusOut(sender,sel,ptr);
  update();
  return 1;
  }


// Test if logically inside
FXbool FXMenuTitle::contains(FXint parentx,FXint parenty) const {
  FXint x,y;
  if(FXMenuCaption::contains(parentx,parenty)) return 1;
  if(getMenu() && getMenu()->shown()){
    getParent()->translateCoordinatesTo(x,y,getRoot(),parentx,parenty);
    if(getMenu()->contains(x,y)) return 1;
    }
  return 0;
  }


// Save object to stream
void FXMenuTitle::save(FXStream& store) const {
  FXMenuCaption::save(store);
  store << pane;
  }


// Load object from stream
void FXMenuTitle::load(FXStream& store){
  FXMenuCaption::load(store);
  store >> pane;
  }


// Delete it
FXMenuTitle::~FXMenuTitle(){
  pane=(FXMenuPane*)-1;
  }


