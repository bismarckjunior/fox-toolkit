/********************************************************************************
*                                                                               *
*                     A p p l i c a t i o n   O b j e c t                       *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997,2004 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* Major Contributions for Windows NT by Lyle Johnson                            *
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
* $Id: FXApp.cpp,v 1.458 2004/05/03 20:42:55 fox Exp $                          *
********************************************************************************/
#ifdef WIN32
#if _WIN32_WINNT < 0x0400
#define _WIN32_WINNT 0x0400
#endif
#endif
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxkeys.h"
#include "fxpriv.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXObject.h"
#include "FXSettings.h"
#include "FXRegistry.h"
#include "FXAccelTable.h"
#include "FXHash.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXDC.h"
#include "FXDCWindow.h"
#include "FXVisual.h"
#include "FXCursor.h"
#include "FXFont.h"
#include "FXDrawable.h"
#include "FXBitmap.h"
#include "FXImage.h"
#include "FXIcon.h"
#include "FXGIFIcon.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXString.h"
#include "FXLabel.h"
#include "FXButton.h"
#include "FXComposite.h"
#include "FXRootWindow.h"
#include "FXShell.h"
#include "FXTopWindow.h"
#include "FXPopup.h"
#include "FXMenuPane.h"
#include "FXDialogBox.h"
#include "FXMessageBox.h"

/*
  Notes:

  - Should not be able to DND drop into some windows while modal window is up.

  - Refresh() now forces total GUI updating; flush() updates display only.

  - Sender is the FXApp, but in case of DND ops, we might have a special stand-in object
    as the sender (e.g. so we can send it messages).

  - Application should have a target to be notified of certain app-wide events,
    such as activation of [a window of] the application etc.

  - Need way to specify visual on command line (X11).

  - Need to be able to run event loop w/o display connection (just I/O, and timers).

  - Need to be able to ``detach'' from GUI more cleanly.

  - FXApp::exit() should be called by AFTER returning from FXApp::run(); also,
    FXApp::exit() should not call the global exit():- just because we're done doing
    GUI things does not mean we're done with the program!

  - When timer, signal, I/O, chore callback fires, need to go once round the event
    loop, because these callbacks may set a flag to break out of the event loop.
    This is done by letting getNextEvent() return FALSE if it returns with NO
    event [The alternative would be to dispatch these events via dispatchEvent()
    which is currently a bit difficult].

  - Event logging (journalling) and playback.  We need some basic capabilities for
    journalling and playback of user-inputs [basically, mouse and keyboard events].
    How exactly this is going to work is not entirely clear, but there are a couple
    of issues:

      - Mapping window ID's to something we can save on the file. FXWindow now
        generates a window-key which can identify each window by means of a kind of
        Dewey Decimal system.
      - Which events need logging? Of course, mouse buttons, motion, and keyboard, but
        how about enter/leave? The real mouse may be moving around too while playing
        back!
      - And what should be recorded.
      - Plus, some difficulty with event dispatch on Windows.

  - Pre- and post-dispatch hooks. In the pre-dispatch hook, the event is presented to
    some user-defined function (or maybe message handler) and can be inspected prior to
    dispatch.  Returning a TRUE or FALSE from the pre-dispatch hook function will cause
    the actual dispatch to be blocked or not.

    Thus, the predispatch hook can act as an event filter, and events which are passed are
    dispatched normally.

    The post-dispatch hook is presented with the event AFTER it was dispatched.
    Either we present the event to the post-dispatch hook only when it was
    actually handled somewhere in the GUI, or we always present it and pass
    a flag that says whether it has been handled by some widget or not. The chief purpose
    of a post-dispatch hook is for event logging purposes.

  - Make sure keyboard gets dispatched to modal window [dialog or popup or whatever].

  - FXInvocation into a class to its dtor will be called [just in case someone tries to
    throw an exception].   Having a dtor allows clean up without using a try-catch
    construct.

  - Modal modes for FXInvocation:

      - Non-modal for unconstrained model loops, like e.g. toplevel loop.
      - Modal for window, typically a dialog but generally modal for any
        window and its inferiors.  Clicking outside the modal window will
        cause a beep.
      - Application modal, i.e. always beep no matter which window.  This
        is useful for complete blocking of user-events while still performing
        layouts and repaints and so on.
      - Popup modal.  Very similar to Modal for a window, except when clicking
        outside the popup stack is closed instead of issuing a beep.

*/

// Just in case
#define bzero(ptr,size) memset(ptr,0,size)


// Regular define
#define SELECT(n,r,w,e,t)  select(n,r,w,e,t)


// FIX for HPUX
#ifdef _HPUX_SOURCE
#ifndef _XPG4_EXTENDED    // HPUX 9.07
#undef SELECT
#define SELECT(n,r,w,e,t)  select(n,(int*)(r),(int*)(w),(int*)(e),t)
#endif
#endif


// FIX for AIX 3.x
#ifndef _XOPEN_SOURCE_EXTENDED
#if defined(_POWER) || defined(_IBMR2)
#undef SELECT
#define SELECT(n,r,w,e,t)  select(n,(void*)(r),(void*)(w),(void*)(e),t)
extern "C" int gettimeofday(struct timeval *tv, struct timezone *tz);
#endif
#endif

// Default maximum number of colors to allocate
#define MAXCOLORS 125

// Largest number of signals on this system
#define MAXSIGNALS 64

using namespace FX;

/*******************************************************************************/

namespace FX {

// Horizontal splitter cursor
#include "hsplit.xbm"
#include "hsplit_mask.xbm"

// Vertical splitter cursor
#include "vsplit.xbm"
#include "vsplit_mask.xbm"

// Cross splitter cursor
#include "xsplit.xbm"
#include "xsplit_mask.xbm"

// Color swatch drag-and-drop cursor
#include "swatch.xbm"
#include "swatch_mask.xbm"

// NO DROP drag-and-drop cursor
#include "dontdrop.xbm"
#include "dontdrop_mask.xbm"

// Upper or lower side MDI resize cursor
#include "resizetop.xbm"
#include "resizetop_mask.xbm"

// Right MDI resize cursor
#include "resizetopright.xbm"
#include "resizetopright_mask.xbm"

// Left MDI resize cursor
#include "resizetopleft.xbm"
#include "resizetopleft_mask.xbm"

// Left or right side MDI resize cursor
#include "resizeleft.xbm"
#include "resizeleft_mask.xbm"

// Move cursor
#include "drag.xbm"
#include "drag_mask.xbm"

// Drag and drop COPY
#include "dndcopy.xbm"
#include "dndcopy_mask.xbm"

// Drag and drop LINK
#include "dndlink.xbm"
#include "dndlink_mask.xbm"

// Drag and drop MOVE
#include "dndmove.xbm"
#include "dndmove_mask.xbm"

// Crosshair
#include "crosshair.xbm"
#include "crosshair_mask.xbm"

// NE,NW,SE,SW corner cursors
#include "ne.xbm"
#include "ne_mask.xbm"
#include "nw.xbm"
#include "nw_mask.xbm"
#include "se.xbm"
#include "se_mask.xbm"
#include "sw.xbm"
#include "sw_mask.xbm"

// Help arrow
#include "helparrow.xbm"
#include "helparrow_mask.xbm"

// Rotate cursor
#include "rotate.xbm"
#include "rotate_mask.xbm"

// Hand cursor
#include "hand.xbm"
#include "hand_mask.xbm"


/*******************************************************************************/

// Callback Record
struct FXCBSpec {
  FXObject      *target;            // Receiver object
  FXSelector     message;           // Message sent to receiver
  };


// Timer record
struct FXTimer {
  FXTimer       *next;              // Next timeout in list
  FXObject      *target;            // Receiver object
  void          *data;              // User data
  FXSelector     message;           // Message sent to receiver
#ifndef WIN32
  struct timeval due;               // When timer is due
#else
//  long           due;               // When timer is due (ms)
  FXlong         due;               // When timer is due (ms)
#endif
  };


// Signal record
struct FXSignal {
  FXObject      *target;            // Receiver object
  FXSelector     message;           // Message sent to receiver
  FXbool         handlerset;        // Handler was already set
  FXbool         notified;          // Signal has fired
  };


// Idle record
struct FXChore {
  FXChore       *next;              // Next chore in list
  FXObject      *target;            // Receiver object
  void          *data;              // User data
  FXSelector     message;           // Message sent to receiver
  };


// Input record
struct FXInput {
  FXCBSpec       read;              // Callback spec for read
  FXCBSpec       write;             // Callback spec for write
  FXCBSpec       excpt;             // Callback spec for except
  };


// A repaint event record
struct FXRepaint {
  FXRepaint     *next;              // Next repaint in list
  FXID           window;            // Window ID of the dirty window
  FXRectangle    rect;              // Dirty rectangle
  FXint          hint;              // Hint for compositing
  FXbool         synth;             // Synthetic expose event or real one?
  };


// Recursive Event Loop Invocation
struct FXInvocation {
  FXInvocation **invocation;  // Pointer to variable holding pointer to current invocation
  FXInvocation  *upper;       // Invocation above this one
  FXWindow      *window;      // Modal window (if any)
  FXModality     modality;    // Modality mode
  FXint          code;        // Return code
  FXbool         done;        // True if breaking out

  // Enter modal loop
  FXInvocation(FXInvocation** inv,FXModality mode,FXWindow* win):invocation(inv),upper(*inv),window(win),modality(mode),code(0),done(FALSE){
    *invocation=this;
    }

  // Exit modal loop
 ~FXInvocation(){
    *invocation=upper;
    }
  };


/*******************************************************************************/


// Application object
FXApp* FXApp::app=NULL;


// Copyright notice
const FXuchar FXApp::copyright[]="Copyright (C) 1997,2003 Jeroen van der Zijp. All Rights Reserved.";


#ifndef WIN32

// 17 stipple patterns which match up exactly with the 4x4 dither kernel
static const unsigned char stipple_patterns[17][8]={
  {0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00},   // 0 (white)
  {0x00,0x00,0x00,0x88, 0x00,0x00,0x00,0x88},
  {0x00,0x22,0x00,0x88, 0x00,0x22,0x00,0x88},
  {0x00,0x22,0x00,0xaa, 0x00,0x22,0x00,0xaa},
  {0x00,0xaa,0x00,0xaa, 0x00,0xaa,0x00,0xaa},
  {0x00,0xaa,0x44,0xaa, 0x00,0xaa,0x44,0xaa},
  {0x11,0xaa,0x44,0xaa, 0x11,0xaa,0x44,0xaa},
  {0x11,0xaa,0x55,0xaa, 0x11,0xaa,0x55,0xaa},
  {0x55,0xaa,0x55,0xaa, 0x55,0xaa,0x55,0xaa},   // 8 (50% grey)
  {0x55,0xaa,0x55,0xee, 0x55,0xaa,0x55,0xee},
  {0x55,0xbb,0x55,0xee, 0x55,0xbb,0x55,0xee},
  {0x55,0xbb,0x55,0xff, 0x55,0xbb,0x55,0xff},
  {0x55,0xff,0x55,0xff, 0x55,0xff,0x55,0xff},
  {0x55,0xff,0xdd,0xff, 0x55,0xff,0xdd,0xff},
  {0x77,0xff,0xdd,0xff, 0x77,0xff,0xdd,0xff},
  {0x77,0xff,0xff,0xff, 0x77,0xff,0xff,0xff},
  {0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff}    // 16 (black)
  };


// Standard-issue cross hatch pattern
static const unsigned char cross_bits[] = {
  0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0xff, 0xff, 0xff,
  0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
  0x08, 0x82, 0x20, 0xff, 0xff, 0xff, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
  0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0xff, 0xff, 0xff,
  0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
  0x08, 0x82, 0x20, 0xff, 0xff, 0xff, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20
  };

// Standard-issue diagonal cross hatch pattern
static const unsigned char crossdiag_bits[] = {
  0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14, 0x22, 0x22, 0x41, 0x41,
  0x80, 0x80, 0x41, 0x41, 0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14,
  0x22, 0x22, 0x41, 0x41, 0x80, 0x80, 0x41, 0x41
  };

// Standard-issue diagonal hatch pattern
static const unsigned char diag_bits[] = {
  0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04, 0x02, 0x02, 0x01, 0x01,
  0x80, 0x80, 0x40, 0x40, 0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04,
  0x02, 0x02, 0x01, 0x01, 0x80, 0x80, 0x40, 0x40
  };

// Standard-issue horizontal hatch pattern
static const unsigned char hor_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };

// Standard-issue reverse diagonal hatch pattern
static const unsigned char revdiag_bits[] = {
  0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40,
  0x80, 0x80, 0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10,
  0x20, 0x20, 0x40, 0x40, 0x80, 0x80, 0x01, 0x01
  };

// Standard-issue vertical hatch pattern
static const unsigned char ver_bits[] = {
  0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
  0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
  0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
  0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
  0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
  0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20
  };

#else

// 17 stipple patterns which match up exactly with the 4x4 dither kernel
// Note that each scan line must be word-aligned so we pad to the right
// with zeroes.
static const BYTE stipple_patterns[17][16]={
  {0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00, 0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00},   // 0 (white)
  {0xff,0x00,0xff,0x00,0xff,0x00,0x77,0x00, 0xff,0x00,0xff,0x00,0xff,0x00,0x77,0x00},
  {0xff,0x00,0xdd,0x00,0xff,0x00,0x77,0x00, 0xff,0x00,0xdd,0x00,0xff,0x00,0x77,0x00},
  {0xff,0x00,0xdd,0x00,0xff,0x00,0x55,0x00, 0xff,0x00,0xdd,0x00,0xff,0x00,0x55,0x00},
  {0xff,0x00,0x55,0x00,0xff,0x00,0x55,0x00, 0xff,0x00,0x55,0x00,0xff,0x00,0x55,0x00},
  {0xff,0x00,0x55,0x00,0xbb,0x00,0x55,0x00, 0xff,0x00,0x55,0x00,0xbb,0x00,0x55,0x00},
  {0xee,0x00,0x55,0x00,0xbb,0x00,0x55,0x00, 0xee,0x00,0x55,0x00,0xbb,0x00,0x55,0x00},
  {0xee,0x00,0x55,0x00,0xaa,0x00,0x55,0x00, 0xee,0x00,0x55,0x00,0xaa,0x00,0x55,0x00},
  {0xaa,0x00,0x55,0x00,0xaa,0x00,0x55,0x00, 0xaa,0x00,0x55,0x00,0xaa,0x00,0x55,0x00},   // 8 (50% grey)
  {0xaa,0x00,0x55,0x00,0xaa,0x00,0x11,0x00, 0xaa,0x00,0x55,0x00,0xaa,0x00,0x11,0x00},
  {0xaa,0x00,0x44,0x00,0xaa,0x00,0x11,0x00, 0xaa,0x00,0x44,0x00,0xaa,0x00,0x11,0x00},
  {0xaa,0x00,0x44,0x00,0xaa,0x00,0x00,0x00, 0xaa,0x00,0x44,0x00,0xaa,0x00,0x00,0x00},
  {0xaa,0x00,0x00,0x00,0xaa,0x00,0x00,0x00, 0xaa,0x00,0x00,0x00,0xaa,0x00,0x00,0x00},
  {0xaa,0x00,0x00,0x00,0x22,0x00,0x00,0x00, 0xaa,0x00,0x00,0x00,0x22,0x00,0x00,0x00},
  {0x88,0x00,0x00,0x00,0x22,0x00,0x00,0x00, 0x88,0x00,0x00,0x00,0x22,0x00,0x00,0x00},
  {0x88,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x88,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}    // 16 (black)
  };

#endif

/*******************************************************************************/


// Map
FXDEFMAP(FXApp) FXAppMap[]={
  FXMAPFUNC(SEL_TIMEOUT,FXApp::ID_QUIT,FXApp::onCmdQuit),
  FXMAPFUNC(SEL_SIGNAL,FXApp::ID_QUIT,FXApp::onCmdQuit),
  FXMAPFUNC(SEL_CHORE,FXApp::ID_QUIT,FXApp::onCmdQuit),
  FXMAPFUNC(SEL_COMMAND,FXApp::ID_QUIT,FXApp::onCmdQuit),
  FXMAPFUNC(SEL_COMMAND,FXApp::ID_DUMP,FXApp::onCmdDump),
  };


// Implementation
FXIMPLEMENT(FXApp,FXObject,FXAppMap,ARRAYNUMBER(FXAppMap))


/*******************************************************************************/


// Initialize application object
FXApp::FXApp(const FXString& name,const FXString& vendor):registry(name,vendor){

  // Test if application object already exists
  if(app){fxerror("Warning: Trying to construct multiple application objects.\n");}

  // Initialize private platform independent data
  display=NULL;                           // Display connection
  dpy=":0.0";                             // Initial display guess
  focusWindow=NULL;                       // Window which has focus
  cursorWindow=NULL;                      // Window under the cursor
  mouseGrabWindow=NULL;                   // Window which grabbed mouse
  keyboardGrabWindow=NULL;                // Window which grabbed keyboard
  keyWindow=NULL;                         // Window in which keyboard key was pressed
  selectionWindow=NULL;                   // Window which has the selection
  clipboardWindow=NULL;                   // Window which has the clipboard
  dragWindow=NULL;                        // Drop target window
  dropWindow=NULL;                        // Drag source window
  refresher=NULL;                         // GUI refresher pointer
  refresherstop=NULL;                     // GUI refresher end pointer
  popupWindow=NULL;                       // No popup windows
  timers=NULL;                            // No timers present
  chores=NULL;                            // No chores present
  repaints=NULL;                          // No outstanding repaints
  timerrecs=NULL;                         // No timer records
  chorerecs=NULL;                         // No chore records
  repaintrecs=NULL;                       // No repaint records
  invocation=NULL;                        // Modal loop invocation
  FXCALLOC(&inputs,FXInput,8);            // Input file descriptors
  ninputs=8;                              // Number of these
  maxinput=-1;                            // Maximum input number
  signals=NULL;                           // Signals array
  nsignals=0;                             // Number of signal handlers set
  maxcolors=MAXCOLORS;                    // Maximum number of colors to allocate
  ddeData=NULL;                           // Data exchange array
  ddeSize=0;                              // Data exchange array size
  appArgc=0;                              // Number of program arguments
  appArgv=NULL;                           // Program arguments
  waitCount=0;                            // Cursor wait count
  initialized=FALSE;                      // Not yet initialized

  // Clear event structure
  event.type=0;
  event.time=0;
  event.win_x=0;
  event.win_y=0;
  event.root_x=0;
  event.root_y=0;
  event.state=0;
  event.code=0;
  event.last_x=0;
  event.last_y=0;
  event.click_x=0;
  event.click_y=0;
  event.rootclick_x=0;
  event.rootclick_y=0;
  event.click_time=0;
  event.click_button=0;
  event.click_count=0;
  event.moved=0;
  event.rect.x=0;
  event.rect.y=0;
  event.rect.w=0;
  event.rect.h=0;
  event.synthetic=0;
  event.target=0;

  // Monochrome visual
  monoVisual=new FXVisual(this,VISUAL_MONOCHROME);

  // Default visual
  defaultVisual=new FXVisual(this,VISUAL_DEFAULT);

  // Wait cursor initialization
  waitCursor=new FXCursor(this,CURSOR_WATCH);

  // Make some cursors
  cursor[DEF_ARROW_CURSOR]=new FXCursor(this,CURSOR_ARROW);
  cursor[DEF_RARROW_CURSOR]=new FXCursor(this,CURSOR_RARROW);
  cursor[DEF_TEXT_CURSOR]=new FXCursor(this,CURSOR_IBEAM);

  // Cursors for splitter
  cursor[DEF_HSPLIT_CURSOR]=new FXCursor(this,hsplit_bits,hsplit_mask_bits,hsplit_width,hsplit_height,hsplit_x_hot,hsplit_y_hot);
  cursor[DEF_VSPLIT_CURSOR]=new FXCursor(this,vsplit_bits,vsplit_mask_bits,vsplit_width,vsplit_height,vsplit_x_hot,vsplit_y_hot);
  cursor[DEF_XSPLIT_CURSOR]=new FXCursor(this,xsplit_bits,xsplit_mask_bits,xsplit_width,xsplit_height,xsplit_x_hot,xsplit_y_hot);

  // Color swatch
  cursor[DEF_SWATCH_CURSOR]=new FXCursor(this,swatch_bits,swatch_mask_bits,swatch_width,swatch_height,swatch_x_hot,swatch_y_hot);

  // Move
  cursor[DEF_MOVE_CURSOR]=new FXCursor(this,drag_bits,drag_mask_bits,drag_width,drag_height,drag_x_hot,drag_y_hot);

  // Dragging edges/corners
  cursor[DEF_DRAGH_CURSOR]=new FXCursor(this,resizetop_bits,resizetop_mask_bits,resizetop_width,resizetop_height,resizetop_x_hot,resizetop_y_hot);
  cursor[DEF_DRAGV_CURSOR]=new FXCursor(this,resizeleft_bits,resizeleft_mask_bits,resizeleft_width,resizeleft_height,resizeleft_x_hot,resizeleft_y_hot);
  cursor[DEF_DRAGTR_CURSOR]=new FXCursor(this,resizetopright_bits,resizetopright_mask_bits,resizetopright_width,resizetopright_height,resizetopright_x_hot,resizetopright_y_hot);
  cursor[DEF_DRAGTL_CURSOR]=new FXCursor(this,resizetopleft_bits,resizetopleft_mask_bits,resizetopleft_width,resizetopleft_height,resizetopleft_x_hot,resizetopleft_y_hot);

  // DND actions
  cursor[DEF_DNDSTOP_CURSOR]=new FXCursor(this,dontdrop_bits,dontdrop_mask_bits,dontdrop_width,dontdrop_height,dontdrop_x_hot,dontdrop_y_hot);
  cursor[DEF_DNDCOPY_CURSOR]=new FXCursor(this,dndcopy_bits,dndcopy_mask_bits,dndcopy_width,dndcopy_height,dndcopy_x_hot,dndcopy_y_hot);
  cursor[DEF_DNDMOVE_CURSOR]=new FXCursor(this,dndmove_bits,dndmove_mask_bits,dndmove_width,dndmove_height,dndmove_x_hot,dndmove_y_hot);
  cursor[DEF_DNDLINK_CURSOR]=new FXCursor(this,dndlink_bits,dndlink_mask_bits,dndlink_width,dndlink_height,dndlink_x_hot,dndlink_y_hot);

  // Crosshairs
  cursor[DEF_CROSSHAIR_CURSOR]=new FXCursor(this,crosshair_bits,crosshair_mask_bits,crosshair_width,crosshair_height,crosshair_x_hot,crosshair_y_hot);

  // NE,NW,SE,SW corners
  cursor[DEF_CORNERNE_CURSOR]=new FXCursor(this,ne_bits,ne_mask_bits,ne_width,ne_height,ne_x_hot,ne_y_hot);
  cursor[DEF_CORNERNW_CURSOR]=new FXCursor(this,nw_bits,nw_mask_bits,nw_width,nw_height,nw_x_hot,nw_y_hot);
  cursor[DEF_CORNERSE_CURSOR]=new FXCursor(this,se_bits,se_mask_bits,se_width,se_height,se_x_hot,se_y_hot);
  cursor[DEF_CORNERSW_CURSOR]=new FXCursor(this,sw_bits,sw_mask_bits,sw_width,sw_height,sw_x_hot,sw_y_hot);

  // Help arrow cursor
  cursor[DEF_HELP_CURSOR]=new FXCursor(this,helparrow_bits,helparrow_mask_bits,helparrow_width,helparrow_height,helparrow_x_hot,helparrow_y_hot);

  // Hand cursor
  cursor[DEF_HAND_CURSOR]=new FXCursor(this,hand_bits,hand_mask_bits,hand_width,hand_height,hand_x_hot,hand_y_hot);

  // Rotate
  cursor[DEF_ROTATE_CURSOR]=new FXCursor(this,rotate_bits,rotate_mask_bits,rotate_width,rotate_height,rotate_x_hot,rotate_y_hot);

  // Root window
  root=new FXRootWindow(this,defaultVisual);

  // X Window specific inits
#ifndef WIN32
  wmDeleteWindow=0;                       // Window Manager stuff
  wmQuitApp=0;
  wmProtocols=0;
  wmMotifHints=0;
  wmTakeFocus=0;
  wmState=0;

  wmNetSupported=0;                       // Extended Window Manager stuff
  wmNetState=0;
  wmNetHMaximized=0;
  wmNetVMaximized=0;

  // DDE
  ddeTargets=0;                           // Data exchange to get list of types
  ddeAtom=0;                              // Data exchange atom
  ddeDelete=0;                            // Data exchange delete request
  ddeTypeList=NULL;                       // Data types list available
  ddeNumTypes=0;                          // Number of data types
  ddeAction=DRAG_REJECT;                  // Drag and drop action requested
  ansAction=DRAG_REJECT;                  // Drag and drop action suggested

  // CLIPBOARD
  xcbSelection=0;                         // Clipboard selection atom
  xcbTypeList=NULL;                       // List of clipboard types
  xcbNumTypes=0;                          // How many types clipped

  // SELECTION
  xselTypeList=NULL;                      // List of primary selection types
  xselNumTypes=0;                         // How many types in list

  // XDND
  xdndTypeList=NULL;                      // List of XDND types
  xdndNumTypes=0;                         // How many types in list
  xdndProxy=0;                            // XDND proxy atom
  xdndAware=0;                            // XDND awareness atom
  xdndEnter=0;                            // XDND Message types
  xdndLeave=0;
  xdndPosition=0;
  xdndStatus=0;
  xdndDrop=0;
  xdndFinished=0;
  xdndSelection=0;
  xdndActionCopy=0;                       // XDND Move action
  xdndActionMove=0;                       // XDND Copy action
  xdndActionLink=0;                       // XDND Link action
  xdndActionPrivate=0;                    // XDND Private action
  xdndTypes=0;
  xdndSource=0;                           // XDND drag source window
  xdndTarget=0;                           // XDND drop target window
  xdndProxyTarget=0;                      // XDND window to set messages to
  xdndStatusPending=FALSE;                // XDND waiting for status feedback
  xdndStatusReceived=FALSE;               // XDND received at least one status
  xdndWantUpdates=TRUE;                   // XDND target always wants new positions
  xdndRect.x=0;                           // XDND motion rectangle
  xdndRect.y=0;
  xdndRect.w=0;
  xdndRect.h=0;

  // File descriptors
  FXCALLOC(&r_fds,fd_set,1);              // Read File Descriptor set
  FXCALLOC(&w_fds,fd_set,1);              // Write File Descriptor set
  FXCALLOC(&e_fds,fd_set,1);              // Except File Descriptor set

  // Clear input method stuff
  xim=NULL;
  xic=NULL;

  // Miscellaneous stuff
  shmi=TRUE;
  shmp=TRUE;
  synchronize=FALSE;

  // MS-Windows specific inits
#else

  // DDE
  ddeTargets=0;                           // Data exchange to get list of types
  ddeAtom=0;                              // Data exchange atom
  ddeDelete=0;                            // Data exchange delete request
  ddeTypeList=NULL;                       // Data types list available
  ddeNumTypes=0;                          // Number of data types
  ddeAction=DRAG_REJECT;                  // Drag and drop action requested
  ansAction=DRAG_REJECT;                  // Drag and drop action suggested

  // SELECTION
  xselTypeList=NULL;                      // List of primary selection types
  xselNumTypes=0;                         // How many types in list

  // XDND
  xdndFinishPending=FALSE;                // XDND waiting for drop-confirmation
  xdndAware=0;                            // XDND awareness atom
  xdndTypes=NULL;
  xdndSource=0;                           // XDND drag source window
  xdndTarget=0;                           // XDND drop target window
  xdndStatusPending=FALSE;                // XDND waiting for status feedback
  xdndStatusReceived=FALSE;               // XDND received at least one status
  xdndRect.x=0;                           // XDND motion rectangle
  xdndRect.y=0;
  xdndRect.w=0;
  xdndRect.h=0;
  FXCALLOC(&handles,void*,ninputs);       // Same size as inputs array

#endif

  // Other settings
  typingSpeed=1000;
  clickSpeed=400;
  scrollSpeed=80;
  scrollDelay=600;
  blinkSpeed=500;
  animSpeed=10;
  menuPause=400;
  tooltipPause=800;
  tooltipTime=3000;
  dragDelta=6;
  wheelLines=10;

  // Make font
  normalFont=new FXFont(this,"helvetica,90,bold");

  // We delete the stock font
  stockFont=normalFont;

  // Init colors
  borderColor=FXRGB(0,0,0);
  baseColor=FXRGB(212,208,200);
  hiliteColor=makeHiliteColor(baseColor);
  shadowColor=makeShadowColor(baseColor);
  backColor=FXRGB(255,255,255);
  foreColor=FXRGB(0,0,0);
  selforeColor=FXRGB(255,255,255);
  selbackColor=FXRGB(10,36,106);
  tipforeColor=FXRGB(0,0,0);
  tipbackColor=FXRGB(255,255,225);

  // Pointer to FXApp
  app=this;
  }

/*******************************************************************************/


// Find window from id
FXWindow* FXApp::findWindowWithId(FXID xid) const {
  return (FXWindow*)hash.find((void*)xid);
  }


// Find window from root x,y, starting from given window
FXWindow* FXApp::findWindowAt(FXint rx,FXint ry,FXID window) const {
  if(initialized){
#ifndef WIN32
    Window rootwin=XDefaultRootWindow((Display*)display);
    Window child;
    int wx,wy;
    if(!window) window=rootwin;
    while(1){
      if(!XTranslateCoordinates((Display*)display,rootwin,window,rx,ry,&wx,&wy,&child)) return NULL;
      if(child==None) break;
      window=child;
      }
#else
    POINT point;
    point.x=rx;
    point.y=ry;
    window=WindowFromPoint(point);      // FIXME this finds only enabled/visible windows
#endif
    return findWindowWithId(window);
    }
  return NULL;
  }


/*******************************************************************************/

#ifndef WIN32

// Perhaps should do something else...
static int xerrorhandler(Display* dpy,XErrorEvent* eev){
  char buf[256];

  // A BadWindow due to X_SendEvent is likely due to XDND
  if(eev->error_code==BadWindow && eev->request_code==25) return 0;

  // WM_TAKE_FOCUS causes sporadic errors for X_SetInputFocus
  if(eev->request_code==42) return 0;

  // Get error codes
  XGetErrorText(dpy,eev->error_code,buf,sizeof(buf));

  // Print out meaningful warning
  fxwarning("X Error: code %d major %d minor %d: %s.\n",eev->error_code,eev->request_code,eev->minor_code,buf);
  return 1;
  }


// Fatal error (e.g. lost connection)
static int xfatalerrorhandler(Display*){
  fxerror("X Fatal error.\n");
  return 1;
  }


#endif

/*******************************************************************************/

// Open the display
FXbool FXApp::openDisplay(const FXchar* dpyname){
  if(!initialized){

    // What's going on
    FXTRACE((100,"%s::openDisplay: opening display.\n",getClassName()));
#ifndef WIN32

    // Set error handler
    XSetErrorHandler(xerrorhandler);

    // Set fatal handler
    XSetIOErrorHandler(xfatalerrorhandler);

    // Revert to default
    if(!dpyname) dpyname=dpy;

    // Open display
    display=XOpenDisplay(dpyname);
    if(!display) return FALSE;

    // For debugging
    if(synchronize) XSynchronize((Display*)display,TRUE);

#ifdef HAVE_XSHM_H

    // See if we wanted/have XSHM
    FXTRACE((100,"Checking for shared memory\n"));

    // Displaying remotely turns it off for sure
    if(!(dpyname[0]==':' && isdigit((FXuchar)dpyname[1]))){
      shmi=FALSE;
      shmp=FALSE;
      }

    // Its potentially on, see if local display groks it
    if(shmi || shmp){
      int maj,min,dum; Bool pm;
      shmi=FALSE;
      shmp=FALSE;
      if(XQueryExtension((Display*)display,"MIT-SHM",&dum,&dum,&dum)){
        if(XShmQueryVersion((Display*)display,&maj,&min,&pm)){
          shmp=pm && (XShmPixmapFormat((Display*)display)==ZPixmap);
          shmi=TRUE;
          }
        }
      }

    // Report the result
    FXTRACE((100,"Shared Images  = %d\n",shmi));
    FXTRACE((100,"Shared Pixmaps = %d\n",shmp));

#else

    // Don't have it!
    FXTRACE((100,"Shared memory not available\n"));
    shmi=FALSE;
    shmp=FALSE;
#endif

    // Initialize Xft and fontconfig
#ifdef HAVE_XFT_H
    if(!XftInit(NULL)) return FALSE;
#endif

    // Open input method
#ifndef NO_XIM
    xic=NULL;
    xim=(void*)XOpenIM((Display*)display,NULL,NULL,NULL);
    if(xim){
#ifndef _AIX
      xic=XCreateIC((XIM)xim,XNInputStyle,XIMPreeditNothing|XIMStatusNothing,XNClientWindow,XDefaultRootWindow((Display*)display),NULL);
#else
      // Suggested by Terry Bezue <terryb@nc.rr.com> for AIX
      xic=XCreateIC((XIM)xim,XNInputStyle,XIMPreeditNothing|XIMStatusNone,XNClientWindow,XDefaultRootWindow((Display*)display),NULL);
#endif
      if(xic) XmbResetIC((XIC)xic);
      }
#endif

    // Window Manager communication
    wmDeleteWindow=XInternAtom((Display*)display,"WM_DELETE_WINDOW",0);
    wmQuitApp=XInternAtom((Display*)display,"_WM_QUIT_APP",0);
    wmProtocols=XInternAtom((Display*)display,"WM_PROTOCOLS",0);
    wmMotifHints=XInternAtom((Display*)display,"_MOTIF_WM_HINTS",0);
    wmTakeFocus=XInternAtom((Display*)display,"WM_TAKE_FOCUS",0);
    wmState=XInternAtom((Display*)display,"WM_STATE",0);

    // Extended Window Manager support
    wmNetSupported=XInternAtom((Display*)display,"_NET_SUPPORTED",0);
    wmNetState=XInternAtom((Display*)display,"_NET_WM_STATE",0);
    wmNetHMaximized=XInternAtom((Display*)display,"_NET_WM_STATE_MAXIMIZED_HORZ",0);
    wmNetVMaximized=XInternAtom((Display*)display,"_NET_WM_STATE_MAXIMIZED_VERT",0);

    // DDE property
    ddeAtom=(FXID)XInternAtom((Display*)display,"_FOX_DDE",0);
    ddeDelete=(FXID)XInternAtom((Display*)display,"DELETE",0);
    ddeTargets=(FXID)XInternAtom((Display*)display,"TARGETS",0);
    ddeIncr=(FXID)XInternAtom((Display*)display,"INCR",0);

    // Clipboard
    xcbSelection=(FXID)XInternAtom((Display*)display,"CLIPBOARD",0);

    // XDND protocol awareness
    xdndProxy=(FXID)XInternAtom((Display*)display,"XdndProxy",0);
    xdndAware=(FXID)XInternAtom((Display*)display,"XdndAware",0);

    // XDND Messages
    xdndEnter=(FXID)XInternAtom((Display*)display,"XdndEnter",0);
    xdndLeave=(FXID)XInternAtom((Display*)display,"XdndLeave",0);
    xdndPosition=(FXID)XInternAtom((Display*)display,"XdndPosition",0);
    xdndStatus=(FXID)XInternAtom((Display*)display,"XdndStatus",0);
    xdndDrop=(FXID)XInternAtom((Display*)display,"XdndDrop",0);
    xdndFinished=(FXID)XInternAtom((Display*)display,"XdndFinished",0);

    // XDND Selection atom
    xdndSelection=(FXID)XInternAtom((Display*)display,"XdndSelection",0);

    // XDND Actions
    xdndActionCopy=(FXID)XInternAtom((Display*)display,"XdndActionCopy",0);
    xdndActionMove=(FXID)XInternAtom((Display*)display,"XdndActionMove",0);
    xdndActionLink=(FXID)XInternAtom((Display*)display,"XdndActionLink",0);
    xdndActionPrivate=(FXID)XInternAtom((Display*)display,"XdndActionPrivate",0);

    // XDND Types list
    xdndTypes=(FXID)XInternAtom((Display*)display,"XdndTypeList",0);

    // Standard stipples
    stipples[STIPPLE_0]=(FXID)XCreateBitmapFromData((Display*)display,XDefaultRootWindow((Display*)display),(char*)stipple_patterns[STIPPLE_0],8,8);
    stipples[STIPPLE_1]=(FXID)XCreateBitmapFromData((Display*)display,XDefaultRootWindow((Display*)display),(char*)stipple_patterns[STIPPLE_1],8,8);
    stipples[STIPPLE_2]=(FXID)XCreateBitmapFromData((Display*)display,XDefaultRootWindow((Display*)display),(char*)stipple_patterns[STIPPLE_2],8,8);
    stipples[STIPPLE_3]=(FXID)XCreateBitmapFromData((Display*)display,XDefaultRootWindow((Display*)display),(char*)stipple_patterns[STIPPLE_3],8,8);
    stipples[STIPPLE_4]=(FXID)XCreateBitmapFromData((Display*)display,XDefaultRootWindow((Display*)display),(char*)stipple_patterns[STIPPLE_4],8,8);
    stipples[STIPPLE_5]=(FXID)XCreateBitmapFromData((Display*)display,XDefaultRootWindow((Display*)display),(char*)stipple_patterns[STIPPLE_5],8,8);
    stipples[STIPPLE_6]=(FXID)XCreateBitmapFromData((Display*)display,XDefaultRootWindow((Display*)display),(char*)stipple_patterns[STIPPLE_6],8,8);
    stipples[STIPPLE_7]=(FXID)XCreateBitmapFromData((Display*)display,XDefaultRootWindow((Display*)display),(char*)stipple_patterns[STIPPLE_7],8,8);
    stipples[STIPPLE_8]=(FXID)XCreateBitmapFromData((Display*)display,XDefaultRootWindow((Display*)display),(char*)stipple_patterns[STIPPLE_8],8,8);
    stipples[STIPPLE_9]=(FXID)XCreateBitmapFromData((Display*)display,XDefaultRootWindow((Display*)display),(char*)stipple_patterns[STIPPLE_9],8,8);
    stipples[STIPPLE_10]=(FXID)XCreateBitmapFromData((Display*)display,XDefaultRootWindow((Display*)display),(char*)stipple_patterns[STIPPLE_10],8,8);
    stipples[STIPPLE_11]=(FXID)XCreateBitmapFromData((Display*)display,XDefaultRootWindow((Display*)display),(char*)stipple_patterns[STIPPLE_11],8,8);
    stipples[STIPPLE_12]=(FXID)XCreateBitmapFromData((Display*)display,XDefaultRootWindow((Display*)display),(char*)stipple_patterns[STIPPLE_12],8,8);
    stipples[STIPPLE_13]=(FXID)XCreateBitmapFromData((Display*)display,XDefaultRootWindow((Display*)display),(char*)stipple_patterns[STIPPLE_13],8,8);
    stipples[STIPPLE_14]=(FXID)XCreateBitmapFromData((Display*)display,XDefaultRootWindow((Display*)display),(char*)stipple_patterns[STIPPLE_14],8,8);
    stipples[STIPPLE_15]=(FXID)XCreateBitmapFromData((Display*)display,XDefaultRootWindow((Display*)display),(char*)stipple_patterns[STIPPLE_15],8,8);
    stipples[STIPPLE_16]=(FXID)XCreateBitmapFromData((Display*)display,XDefaultRootWindow((Display*)display),(char*)stipple_patterns[STIPPLE_16],8,8);

    // Hatch patterns
    stipples[STIPPLE_HORZ]=(FXID)XCreateBitmapFromData((Display*)display,XDefaultRootWindow((Display*)display),(char*)hor_bits,24,24);
    stipples[STIPPLE_VERT]=(FXID)XCreateBitmapFromData((Display*)display,XDefaultRootWindow((Display*)display),(char*)ver_bits,24,24);
    stipples[STIPPLE_CROSS]=(FXID)XCreateBitmapFromData((Display*)display,XDefaultRootWindow((Display*)display),(char*)cross_bits,24,24);
    stipples[STIPPLE_DIAG]=(FXID)XCreateBitmapFromData((Display*)display,XDefaultRootWindow((Display*)display),(char*)diag_bits,16,16);
    stipples[STIPPLE_REVDIAG]=(FXID)XCreateBitmapFromData((Display*)display,XDefaultRootWindow((Display*)display),(char*)revdiag_bits,16,16);
    stipples[STIPPLE_CROSSDIAG]=(FXID)XCreateBitmapFromData((Display*)display,XDefaultRootWindow((Display*)display),(char*)crossdiag_bits,16,16);

#else

    // Set to HINSTANCE on Windows
    display=GetModuleHandle(NULL);

    // TARGETS
    ddeTargets=GlobalAddAtom("TARGETS");

    // XDND protocol awareness
    xdndAware=GlobalAddAtom("XdndAware");

    // DDE property
    ddeDelete=RegisterClipboardFormat("DELETE");

    // Standard stipples
    stipples[STIPPLE_0]=CreateBitmap(8,8,1,1,stipple_patterns[STIPPLE_0]);
    stipples[STIPPLE_1]=CreateBitmap(8,8,1,1,stipple_patterns[STIPPLE_1]);
    stipples[STIPPLE_2]=CreateBitmap(8,8,1,1,stipple_patterns[STIPPLE_2]);
    stipples[STIPPLE_3]=CreateBitmap(8,8,1,1,stipple_patterns[STIPPLE_3]);
    stipples[STIPPLE_4]=CreateBitmap(8,8,1,1,stipple_patterns[STIPPLE_4]);
    stipples[STIPPLE_5]=CreateBitmap(8,8,1,1,stipple_patterns[STIPPLE_5]);
    stipples[STIPPLE_6]=CreateBitmap(8,8,1,1,stipple_patterns[STIPPLE_6]);
    stipples[STIPPLE_7]=CreateBitmap(8,8,1,1,stipple_patterns[STIPPLE_7]);
    stipples[STIPPLE_8]=CreateBitmap(8,8,1,1,stipple_patterns[STIPPLE_8]);
    stipples[STIPPLE_9]=CreateBitmap(8,8,1,1,stipple_patterns[STIPPLE_9]);
    stipples[STIPPLE_10]=CreateBitmap(8,8,1,1,stipple_patterns[STIPPLE_10]);
    stipples[STIPPLE_11]=CreateBitmap(8,8,1,1,stipple_patterns[STIPPLE_11]);
    stipples[STIPPLE_12]=CreateBitmap(8,8,1,1,stipple_patterns[STIPPLE_12]);
    stipples[STIPPLE_13]=CreateBitmap(8,8,1,1,stipple_patterns[STIPPLE_13]);
    stipples[STIPPLE_14]=CreateBitmap(8,8,1,1,stipple_patterns[STIPPLE_14]);
    stipples[STIPPLE_15]=CreateBitmap(8,8,1,1,stipple_patterns[STIPPLE_15]);
    stipples[STIPPLE_16]=CreateBitmap(8,8,1,1,stipple_patterns[STIPPLE_16]);

    // Register our child window classes
    WNDCLASSEX wndclass;


    // Child window
    wndclass.cbSize=sizeof(WNDCLASSEX);
    wndclass.style=CS_HREDRAW|CS_VREDRAW;     // Set to 0 for bit_gravity
    //wndclass.style=0;
    wndclass.lpfnWndProc=(WNDPROC) FXApp::wndproc;
    wndclass.cbClsExtra=0;
    wndclass.cbWndExtra=sizeof(FXWindow*);
    wndclass.hInstance=(HINSTANCE)display;
    wndclass.hIcon=NULL;
    wndclass.hIconSm=NULL;
    wndclass.hCursor=NULL;
    wndclass.hbrBackground=NULL;
    wndclass.lpszMenuName=NULL;
    wndclass.lpszClassName="FXWindow";
    RegisterClassEx(&wndclass);


    // Top window class
    wndclass.cbSize=sizeof(WNDCLASSEX);
    wndclass.style=CS_HREDRAW|CS_VREDRAW;
    //wndclass.style=0;
    wndclass.lpfnWndProc=(WNDPROC) FXApp::wndproc;
    wndclass.cbClsExtra=0;
    wndclass.cbWndExtra=sizeof(FXWindow*);
    wndclass.hInstance=(HINSTANCE)display;
    wndclass.hIcon=LoadIcon((HINSTANCE)display,IDI_APPLICATION);
    if(wndclass.hIcon==NULL) wndclass.hIcon=LoadIcon(NULL,IDI_APPLICATION);
    wndclass.hIconSm=(HICON)LoadImage((HINSTANCE)display,IDI_APPLICATION,IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
    if(wndclass.hIconSm==NULL) wndclass.hIconSm=wndclass.hIcon;
    wndclass.hCursor=NULL;
    wndclass.hbrBackground=NULL;
    wndclass.lpszMenuName=NULL;
    wndclass.lpszClassName="FXTopWindow";
    RegisterClassEx(&wndclass);


    // OpenGL window class
    wndclass.cbSize=sizeof(WNDCLASSEX);
    wndclass.style=CS_HREDRAW|CS_VREDRAW|CS_OWNDC;   // Redraw all when resized, OWNER DC for speed
    wndclass.lpfnWndProc=(WNDPROC) FXApp::wndproc;
    wndclass.cbClsExtra=0;
    wndclass.cbWndExtra=sizeof(FXWindow*);
    wndclass.hInstance=(HINSTANCE)display;
    wndclass.hIcon=NULL;
    wndclass.hIconSm=NULL;
    wndclass.hCursor=LoadCursor(NULL,IDC_ARROW);
    wndclass.hbrBackground=NULL;
    wndclass.lpszMenuName=NULL;
    wndclass.lpszClassName="FXGLCanvas";
    RegisterClassEx(&wndclass);


    // Popup window class
    wndclass.cbSize=sizeof(WNDCLASSEX);
    wndclass.style=CS_HREDRAW|CS_VREDRAW|CS_SAVEBITS;   // Do save-under's
    wndclass.lpfnWndProc=(WNDPROC) FXApp::wndproc;
    wndclass.cbClsExtra=0;
    wndclass.cbWndExtra=sizeof(FXWindow*);
    wndclass.hInstance=(HINSTANCE)display;
    wndclass.hIcon=NULL;
    wndclass.hIconSm=NULL;
    wndclass.hCursor=LoadCursor(NULL,IDC_ARROW);
    wndclass.hbrBackground=NULL;
    wndclass.lpszMenuName=NULL;
    wndclass.lpszClassName="FXPopup";
    RegisterClassEx(&wndclass);

    // This should prevent the Abort/Retry/Ignore message
    // when switching to drive w/no media mounted in it...
    SetErrorMode(SEM_FAILCRITICALERRORS);

#endif

    // We have been initialized
    initialized=TRUE;
    }
  return TRUE;
  }


// Close display
FXbool FXApp::closeDisplay(){
  if(initialized){

    // What's going on
    FXTRACE((100,"%s::closeDisplay: closing display.\n",getClassName()));

    // Clear up the rest
#ifndef WIN32
    FXASSERT(display);

    // Free standard stipples
    XFreePixmap((Display*)display,stipples[STIPPLE_0]);
    XFreePixmap((Display*)display,stipples[STIPPLE_1]);
    XFreePixmap((Display*)display,stipples[STIPPLE_2]);
    XFreePixmap((Display*)display,stipples[STIPPLE_3]);
    XFreePixmap((Display*)display,stipples[STIPPLE_4]);
    XFreePixmap((Display*)display,stipples[STIPPLE_5]);
    XFreePixmap((Display*)display,stipples[STIPPLE_6]);
    XFreePixmap((Display*)display,stipples[STIPPLE_7]);
    XFreePixmap((Display*)display,stipples[STIPPLE_8]);
    XFreePixmap((Display*)display,stipples[STIPPLE_9]);
    XFreePixmap((Display*)display,stipples[STIPPLE_10]);
    XFreePixmap((Display*)display,stipples[STIPPLE_11]);
    XFreePixmap((Display*)display,stipples[STIPPLE_12]);
    XFreePixmap((Display*)display,stipples[STIPPLE_13]);
    XFreePixmap((Display*)display,stipples[STIPPLE_14]);
    XFreePixmap((Display*)display,stipples[STIPPLE_15]);
    XFreePixmap((Display*)display,stipples[STIPPLE_16]);

    // Free hatch patterns
    XFreePixmap((Display*)display,stipples[STIPPLE_HORZ]);
    XFreePixmap((Display*)display,stipples[STIPPLE_VERT]);
    XFreePixmap((Display*)display,stipples[STIPPLE_CROSS]);
    XFreePixmap((Display*)display,stipples[STIPPLE_DIAG]);
    XFreePixmap((Display*)display,stipples[STIPPLE_REVDIAG]);
    XFreePixmap((Display*)display,stipples[STIPPLE_CROSSDIAG]);

    // Destroy input method context
#ifndef NO_XIM
    if(xim){
      if(xic) XDestroyIC((XIC)xic);
      XCloseIM((XIM)xim);
      }
#endif

    // Close display
    XCloseDisplay((Display*)display);
#else

    // Atoms created using GlobalCreateAtom() are reference-counted by
    // the system; calling GlobalDeleteAtom() here just decrements the
    // reference count but doesn't necessarily free the memory.
    GlobalDeleteAtom(ddeTargets);
    GlobalDeleteAtom(xdndAware);

    // Free standard stipples
    DeleteObject(stipples[STIPPLE_0]);
    DeleteObject(stipples[STIPPLE_1]);
    DeleteObject(stipples[STIPPLE_2]);
    DeleteObject(stipples[STIPPLE_3]);
    DeleteObject(stipples[STIPPLE_4]);
    DeleteObject(stipples[STIPPLE_5]);
    DeleteObject(stipples[STIPPLE_6]);
    DeleteObject(stipples[STIPPLE_7]);
    DeleteObject(stipples[STIPPLE_8]);
    DeleteObject(stipples[STIPPLE_9]);
    DeleteObject(stipples[STIPPLE_10]);
    DeleteObject(stipples[STIPPLE_11]);
    DeleteObject(stipples[STIPPLE_12]);
    DeleteObject(stipples[STIPPLE_13]);
    DeleteObject(stipples[STIPPLE_14]);
    DeleteObject(stipples[STIPPLE_15]);
    DeleteObject(stipples[STIPPLE_16]);

#endif
    display=NULL;
    initialized=FALSE;
    }
  return TRUE;
  }


/*******************************************************************************/


#ifndef WIN32

// Compare times
static inline int operator<(const struct timeval& a,const struct timeval& b){
  return (a.tv_sec<b.tv_sec) || (a.tv_sec==b.tv_sec && a.tv_usec<b.tv_usec);
  }

#endif

#ifdef WIN32

// Return 64-bit tick count (ms)
static inline FXlong getticktime(){
  FXlong now;
  GetSystemTimeAsFileTime((FILETIME*)&now);
  return now/10000;
  }

/*
// Return 64-bit tick count (ms)
// The advantage of using TSC register is that it
// isn't changed when the user changes the date or time.
static inline FXlong getticktime(){
  LARGE_INTEGER now,hz;
  QueryPerformanceFrequency(&hz);
  QueryPerformanceCounter(&now);
  return (1000*now)/hz;
  }
*/

#endif


// Add timeout, sorted by time
FXTimer* FXApp::addTimeout(FXObject* tgt,FXSelector sel,FXuint ms,void* ptr){
  register FXTimer *t,**tt;
  for(tt=&timers; (t=*tt)!=NULL; tt=&t->next){
    if(t->target==tgt && t->message==sel){ *tt=t->next; goto a; }
    }
  if(timerrecs){
    t=timerrecs;
    timerrecs=t->next;
    }
  else{
    t=new FXTimer;
    }
a:t->data=ptr;
  t->target=tgt;
#ifndef WIN32
  gettimeofday(&t->due,NULL);
  t->due.tv_sec+=ms/1000;
  t->due.tv_usec+=(ms%1000)*1000;
  if(t->due.tv_usec>=1000000){
    t->due.tv_usec-=1000000;
    t->due.tv_sec+=1;
    }
#else
  t->due=getticktime();
  t->due+=ms;
#endif
  t->message=sel;
  for(tt=&timers; *tt && ((*tt)->due < t->due); tt=&(*tt)->next);
  t->next=*tt;
  *tt=t;
  return t;
  }


// Remove timeout from the list
FXTimer* FXApp::removeTimeout(FXTimer *t){
  register FXTimer **tt;
  for(tt=&timers; *tt; tt=&(*tt)->next){
    if(*tt==t){
      *tt=t->next; t->next=timerrecs; timerrecs=t;
      break;
      }
    }
  return NULL;
  }


// Remove timeout identified by tgt and sel from the list
FXTimer* FXApp::removeTimeout(FXObject* tgt,FXSelector sel){
  register FXTimer *t,**tt;
  for(tt=&timers; (t=*tt)!=NULL; tt=&t->next){
    if(t->target==tgt && t->message==sel){
      *tt=t->next; t->next=timerrecs; timerrecs=t;
      break;
      }
    }
  return NULL;
  }


// Check if timeout identified by tgt and sel has been set
FXbool FXApp::hasTimeout(FXObject* tgt,FXSelector sel) const {
  for(register FXTimer *t=timers; t; t=t->next){
    if(t->target==tgt && t->message==sel) return TRUE;
    }
  return FALSE;
  }


// Return, in ms, the time remaining until the given timer fires.
FXuint FXApp::remainingTimeout(FXObject *tgt,FXSelector sel) const {
  register FXuint remaining=4294967295U;
  for(register FXTimer *t=timers; t; t=t->next){
    if(t->target==tgt && t->message==sel){
      remaining=0;
#ifndef WIN32
      struct timeval now;
      gettimeofday(&now,NULL);
      if(now < t->due){
        now.tv_sec=t->due.tv_sec-now.tv_sec;
        now.tv_usec=t->due.tv_usec-now.tv_usec;
        if(now.tv_usec<0){
          now.tv_usec+=1000000;
          now.tv_sec-=1;
          }
        remaining=now.tv_sec*1000+now.tv_usec/1000;
        }
#else
      FXlong now=getticktime();
      if(now < t->due){
        remaining=t->due-now;
        }
#endif
      break;
      }
    }
  return remaining;
  }


// Return, in ms, the time remaining until the given timer fires.
FXuint FXApp::remainingTimeout(FXTimer *t) const {
  register FXuint remaining=4294967295U;
  for(register FXTimer *tt=timers; tt; tt=tt->next){
    if(tt==t){
      remaining=0;
#ifndef WIN32
      struct timeval now;
      gettimeofday(&now,NULL);
      if(now < t->due){
        now.tv_sec=t->due.tv_sec-now.tv_sec;
        now.tv_usec=t->due.tv_usec-now.tv_usec;
        if(now.tv_usec<0){
          now.tv_usec+=1000000;
          now.tv_sec-=1;
          }
        remaining=now.tv_sec*1000+now.tv_usec/1000;
        }
#else
      FXlong now=getticktime();
      if(now < t->due){
        remaining=t->due-now;
        }
#endif
      break;
      }
    }
  return remaining;
  }

/*******************************************************************************/


// Signal handler; note this is a single write operation
// which can not be interrupted by another handler!
void FXApp::signalhandler(int sig){
  app->signals[sig].notified=TRUE;
  }


// This signal handler is potentially dangerous as it dispatches
// the message to the target right here in the handler; you probably
// want to use this one only in ``desperate'' situations.
void FXApp::immediatesignalhandler(int sig){
  if(app->signals[sig].target) app->signals[sig].target->handle(app,FXSEL(SEL_SIGNAL,app->signals[sig].message),(void*)(FXival)sig);
  }


// Add a signal message
void FXApp::addSignal(FXint sig,FXObject* tgt,FXSelector sel,FXbool immediate,FXuint flags){
  void (*handler)(int);
  if(sig<0 || MAXSIGNALS<sig){ fxerror("%s::addSignal: bad signal number\n",getClassName()); }

  // First signal added allocates the array
  if(nsignals==0){FXCALLOC(&signals,FXSignal,MAXSIGNALS);}

  // May have just changed the message and/or target
  signals[sig].target=tgt;
  signals[sig].message=sel;

  // Has handler been set?
  if(!signals[sig].handlerset){
    if(immediate)
      handler=immediatesignalhandler;
    else
      handler=signalhandler;
#ifdef WIN32
#ifdef __IBMCPP__
    if(signal(sig,(_SigFunc)handler)==SIG_ERR){ fxwarning("%s::addSignal: error setting signal handler\n",getClassName()); }
#else
    if(signal(sig,handler)==SIG_ERR){ fxwarning("%s::addSignal: error setting signal handler\n",getClassName()); }
#endif
#else
#if defined(_POSIX_SOURCE) || defined(_INCLUDE_POSIX_SOURCE)
    struct sigaction sigact;
    sigact.sa_handler=handler;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags=flags;
    if(sigaction(sig,&sigact,NULL)!=0){ fxwarning("%s::addSignal: error setting signal handler\n",getClassName()); }
#else
    signal(sig,handler);
#endif
#endif
    signals[sig].notified=FALSE;
    signals[sig].handlerset=TRUE;
    nsignals++;
    }
  }


// Remove all signal messages for signal sig
void FXApp::removeSignal(FXint sig){
  if(sig<0 || MAXSIGNALS<sig){ fxerror("%s::removeSignal: bad signal number\n",getClassName()); }

  // Has a handler been set?
  if(signals[sig].handlerset){
#ifdef WIN32
    if(signal(sig,SIG_DFL)==SIG_ERR){ fxwarning("%s::removeSignal: error removing signal handler\n",getClassName()); }
#else
#if defined(_POSIX_SOURCE) || defined(_INCLUDE_POSIX_SOURCE)
    struct sigaction sigact;
    sigact.sa_handler=SIG_DFL;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags=0;
    if(sigaction(sig,&sigact,NULL)!=0){ fxwarning("%s::removeSignal: error removing signal handler\n",getClassName()); }
#else
    signal(sig,SIG_DFL);
#endif
#endif
    signals[sig].target=NULL;
    signals[sig].message=0;
    signals[sig].handlerset=FALSE;
    signals[sig].notified=FALSE;
    nsignals--;

    // Last signal removed deletes the array
    if(nsignals==0){FXFREE(&signals);}
    }
  }


/*******************************************************************************/


// Add chore to the END of the list
FXChore* FXApp::addChore(FXObject* tgt,FXSelector sel,void *ptr){
  register FXChore *c,**cc;
  for(cc=&chores; (c=*cc)!=NULL; cc=&c->next){
    if(c->target==tgt && c->message==sel){ *cc=c->next; goto a; }
    }
  if(chorerecs){
    c=chorerecs;
    chorerecs=c->next;
    }
  else{
    c=new FXChore;
    }
a:c->data=ptr;
  c->target=tgt;
  c->message=sel;
  for(cc=&chores; *cc; cc=&(*cc)->next);
  c->next=NULL;
  *cc=c;
  return c;
  }


// Remove chore from the list
FXChore* FXApp::removeChore(FXChore *c){
  register FXChore **cc;
  for(cc=&chores; *cc; cc=&(*cc)->next){
    if(*cc==c){
      *cc=c->next; c->next=chorerecs; chorerecs=c;
      break;
      }
    }
  return NULL;
  }


// Remove chore identified by tgt and sel from the list
FXChore* FXApp::removeChore(FXObject* tgt,FXSelector sel){
  register FXChore *c,**cc;
  for(cc=&chores; (c=*cc)!=NULL; cc=&c->next){
    if(c->target==tgt && c->message==sel){
      *cc=c->next; c->next=chorerecs; chorerecs=c;
      break;
      }
    }
  return NULL;
  }


// Check if chore identified by tgt and sel has been set
FXbool FXApp::hasChore(FXObject* tgt,FXSelector sel) const {
  for(register FXChore *c=chores; c; c=c->next){
    if(c->target==tgt && c->message==sel) return TRUE;
    }
  return FALSE;
  }

/*******************************************************************************/


// Add input
FXbool FXApp::addInput(FXInputHandle fd,FXuint mode,FXObject *tgt,FXSelector sel){
  if(mode==INPUT_NONE) return FALSE;
#ifndef WIN32
  if(fd<0 || fd>=FD_SETSIZE) return FALSE;
  if(fd>=ninputs){                    // Grow table of callbacks
    FXRESIZE(&inputs,FXInput,fd+1);
    memset(&inputs[ninputs],0,sizeof(FXInput)*(fd+1-ninputs));
    ninputs=fd+1;
    }
  FXASSERT(inputs);
  FXASSERT(fd<ninputs);
  if(mode&INPUT_READ){
    inputs[fd].read.target=tgt;
    inputs[fd].read.message=sel;
    FD_SET(fd,(fd_set*)r_fds);
    }
  if(mode&INPUT_WRITE){
    inputs[fd].write.target=tgt;
    inputs[fd].write.message=sel;
    FD_SET(fd,(fd_set*)w_fds);
    }
  if(mode&INPUT_EXCEPT){
    inputs[fd].excpt.target=tgt;
    inputs[fd].excpt.message=sel;
    FD_SET(fd,(fd_set*)e_fds);
    }
  if(fd>maxinput) maxinput=fd;
#else
  register FXint in;
  if(fd==INVALID_HANDLE_VALUE) return FALSE;
  for(in=0; in<=maxinput; in++){      // See if existing handle
    if(handles[in]==fd) goto r;       // If existing handle, just replace callbacks
    }
  if(maxinput+1>=MAXIMUM_WAIT_OBJECTS) return FALSE;
  if(maxinput+1>=ninputs){            // Grow table of callbacks
    FXRESIZE(&inputs,FXInput,ninputs+1);
    memset(&inputs[ninputs],0,sizeof(FXInput));
    FXRESIZE(&handles,void*,ninputs+1);
    ninputs=ninputs+1;
    }
  in=++maxinput;                      // One more handle
  handles[in]=fd;
r:FXASSERT(in<ninputs);
  if(mode&INPUT_READ){                // Replace callbacks
    inputs[in].read.target=tgt;
    inputs[in].read.message=sel;
    }
  if(mode&INPUT_WRITE){
    inputs[in].write.target=tgt;
    inputs[in].write.message=sel;
    }
  if(mode&INPUT_EXCEPT){
    inputs[in].excpt.target=tgt;
    inputs[in].excpt.message=sel;
    }
#endif
  return TRUE;
  }


// Remove input
FXbool FXApp::removeInput(FXInputHandle fd,FXuint mode){
  if(mode==INPUT_NONE) return FALSE;
#ifndef WIN32
  if(fd<0 || fd>maxinput) return FALSE;
  if(mode&INPUT_READ){
    inputs[fd].read.target=NULL;
    inputs[fd].read.message=0;
    FD_CLR(fd,(fd_set*)r_fds);
    }
  if(mode&INPUT_WRITE){
    inputs[fd].write.target=NULL;
    inputs[fd].write.message=0;
    FD_CLR(fd,(fd_set*)w_fds);
    }
  if(mode&INPUT_EXCEPT){
    inputs[fd].excpt.target=NULL;
    inputs[fd].excpt.message=0;
    FD_CLR(fd,(fd_set*)e_fds);
    }
  while(0<=maxinput){                   // Limit number of fd's to test if possible
    if(inputs[maxinput].read.target || inputs[maxinput].write.target || inputs[maxinput].excpt.target) break;
    maxinput--;
    }
#else
  register FXint in;
  if(fd==INVALID_HANDLE_VALUE) return FALSE;
  for(in=0; in<=maxinput; in++){        // See if existing handle
    if(handles[in]==fd) goto r;
    }
  return FALSE;                         // Handle didn't exist, so nothing to remove
r:if(mode&INPUT_READ){
    inputs[in].read.target=NULL;
    inputs[in].read.message=0;
    }
  if(mode&INPUT_WRITE){
    inputs[in].write.target=NULL;
    inputs[in].write.message=0;
    }
  if(mode&INPUT_EXCEPT){
    inputs[in].excpt.target=NULL;
    inputs[in].excpt.message=0;
    }
  if(!inputs[in].read.target && !inputs[in].write.target && !inputs[in].excpt.target){  // Removed them all
    handles[in]=handles[maxinput];      // Compact handle table
    inputs[in]=inputs[maxinput];
    maxinput--;
    }
#endif
  return TRUE;
  }


/*******************************************************************************/


#ifndef WIN32


// Smart rectangle compositing algorithm
void FXApp::addRepaint(FXID win,FXint x,FXint y,FXint w,FXint h,FXbool synth){
  register FXint px,py,pw,ph,hint,area;
  register FXRepaint *r,**pr;
  hint=w*h;
  w+=x;
  h+=y;
  do{

    // Find overlap with outstanding rectangles
    for(r=repaints,pr=&repaints; r; pr=&r->next,r=r->next){
      if(r->window==win){

        // Tentatively conglomerate rectangles
        px=FXMIN(x,r->rect.x);
        py=FXMIN(y,r->rect.y);
        pw=FXMAX(w,r->rect.w);
        ph=FXMAX(h,r->rect.h);
        area=(pw-px)*(ph-py);

        // New area MUCH bigger than sum; forget about it
        if(area > (hint+r->hint)*2) continue;

        // Take old paintrect out of the list
        *pr=r->next;
        r->next=repaintrecs;
        repaintrecs=r;

        // New rectangle
        synth|=r->synth;        // Synthethic is preserved!
        hint=area;
        x=px;
        y=py;
        w=pw;
        h=ph;
        break;
        }
      }
    }
  while(r);

  // Get rectangle, recycled if possible
  if(repaintrecs){
    r=repaintrecs;
    repaintrecs=r->next;
    }
  else{
    r=new FXRepaint;
    }

  // Fill it
  r->window=win;
  r->rect.x=x;
  r->rect.y=y;
  r->rect.w=w;
  r->rect.h=h;
  r->hint=hint;
  r->synth=synth;
  r->next=NULL;
  *pr=r;
  }


// Remove repaints by dispatching them
void FXApp::removeRepaints(FXID win,FXint x,FXint y,FXint w,FXint h){
  FXRepaint *r,**rr;
  XEvent ev;

  w+=x;
  h+=y;

  // Flush the buffer and wait till the X server catches up;
  // resulting events, if any, are buffered in the client.
  XSync((Display*)display,FALSE);

  // Fish out the expose events and compound them
  while(XCheckMaskEvent((Display*)display,ExposureMask,&ev)){
    if(ev.xany.type==NoExpose) continue;
    addRepaint(ev.xexpose.window,ev.xexpose.x,ev.xexpose.y,ev.xexpose.width,ev.xexpose.height,0);
    }

  // Then process events pertaining to window win and overlapping
  // with the given rectangle; other events are left in the queue.
  rr=&repaints;
  while((r=*rr)!=NULL){
    if(!win || (win==r->window && x<r->rect.w && y<r->rect.h && r->rect.x<w && r->rect.y<h)){
      *rr=r->next;
      ev.xany.type=Expose;
      ev.xexpose.window=r->window;
      ev.xexpose.x=r->rect.x;
      ev.xexpose.y=r->rect.y;
      ev.xexpose.width=r->rect.w-r->rect.x;
      ev.xexpose.height=r->rect.h-r->rect.y;
      r->next=repaintrecs;
      repaintrecs=r;
      dispatchEvent(ev);
      continue;
      }
    rr=&r->next;
    }

  // Flush the buffer again
  XFlush((Display*)display);
  }


// Scroll repaint rectangles; some slight trickyness here:- the
// rectangles don't just move, they stretch in the scroll direction
// This means the original dirty area will remain part of the area to
// be painted.
void FXApp::scrollRepaints(FXID win,FXint dx,FXint dy){
  register FXRepaint *r;
  for(r=repaints; r; r=r->next){
    if(r->window==win){
      if(dx>0) r->rect.w+=dx; else r->rect.x+=dx;
      if(dy>0) r->rect.h+=dy; else r->rect.y+=dy;
      }
    }
  }

#endif


/*******************************************************************************/


#ifndef WIN32

// Get an event
FXbool FXApp::getNextEvent(FXRawEvent& ev,FXbool blocking){
  struct timeval now,delta;
  register int ticks;
  XEvent e;

  // Set to no-op just in case
  ev.xany.type=0;

  // Handle all past due timers
  gettimeofday(&now,NULL);
  while(timers){
    register FXTimer* t=timers;
    if(now < t->due) break;
    timers=t->next;
    if(t->target && t->target->handle(this,FXSEL(SEL_TIMEOUT,t->message),t->data)) refresh();
    t->next=timerrecs;
    timerrecs=t;
    }

  // Check non-immediate signals that may have fired
  if(nsignals){
    for(FXint sig=0; sig<MAXSIGNALS; sig++){
      if(signals[sig].notified){
        signals[sig].notified=FALSE;
        if(signals[sig].target && signals[sig].target->handle(this,FXSEL(SEL_SIGNAL,signals[sig].message),(void*)(FXival)sig)){
          refresh();
          return FALSE;
          }
        }
      }
    }

  // Are there no events already queued up?
  if(!initialized || !XEventsQueued((Display*)display,QueuedAfterFlush)){
    register int maxfds;
    register int nfds;
    fd_set readfds;
    fd_set writefds;
    fd_set exceptfds;

    // Prepare fd's to check
    maxfds=maxinput;
    readfds=*((fd_set*)r_fds);
    writefds=*((fd_set*)w_fds);
    exceptfds=*((fd_set*)e_fds);

    // Add connection to display if its open
    if(initialized){
      FD_SET(ConnectionNumber((Display*)display),&readfds);
      if(ConnectionNumber((Display*)display)>maxfds) maxfds=ConnectionNumber((Display*)display);
      }

    delta.tv_usec=0;
    delta.tv_sec=0;

    // Do a quick poll for any ready events or inputs
    nfds=SELECT(maxfds+1,&readfds,&writefds,&exceptfds,&delta);

    // Nothing to do, so perform idle processing
    if(nfds==0){

      // Release the expose events
      if(repaints){
        register FXRepaint *r=repaints;
        ev.xany.type=Expose;
        ev.xexpose.window=r->window;
        ev.xexpose.send_event=r->synth;
        ev.xexpose.x=r->rect.x;
        ev.xexpose.y=r->rect.y;
        ev.xexpose.width=r->rect.w-r->rect.x;
        ev.xexpose.height=r->rect.h-r->rect.y;
        repaints=r->next;
        r->next=repaintrecs;
        repaintrecs=r;
        return TRUE;
        }

      // Do our chores :-)
      if(chores){
        register FXChore *c=chores;
        chores=c->next;
        if(c->target && c->target->handle(this,FXSEL(SEL_CHORE,c->message),c->data)) refresh();
        c->next=chorerecs;
        chorerecs=c;
        }

      // GUI updating:- walk the whole widget tree.
      if(refresher){
        refresher->handle(this,FXSEL(SEL_UPDATE,0),NULL);
        if(refresher->getFirst()){
          refresher=refresher->getFirst();
          }
        else{
          while(refresher->getParent()){
            if(refresher->getNext()){
              refresher=refresher->getNext();
              break;
              }
            refresher=refresher->getParent();
            }
          }
        FXASSERT(refresher);
        if(refresher!=refresherstop) return FALSE;
        refresher=refresherstop=NULL;
        }

      // There are more chores to do
      if(chores) return FALSE;

      // We're not blocking
      if(!blocking) return FALSE;

      // Now, block till timeout, i/o, or event
      maxfds=maxinput;
      readfds=*((fd_set*)r_fds);
      writefds=*((fd_set*)w_fds);
      exceptfds=*((fd_set*)e_fds);

      // Add connection to display if its open
      if(initialized){
        FD_SET(ConnectionNumber((Display*)display),&readfds);
        if(ConnectionNumber((Display*)display)>maxfds) maxfds=ConnectionNumber((Display*)display);
        }

      // If there are timers, we block only for a little while.
      if(timers){

        // All that testing above may have taken some time...
        gettimeofday(&now,NULL);

        // Compute how long to wait
        delta.tv_usec=timers->due.tv_usec-now.tv_usec;
        delta.tv_sec=timers->due.tv_sec-now.tv_sec;
        while(delta.tv_usec<0){
          delta.tv_usec+=1000000;
          delta.tv_sec-=1;
          }

        // Some timers are already due; do them right away!
        if(delta.tv_sec<0 || (delta.tv_sec==0 && delta.tv_usec==0)) return FALSE;

        // Block till timer or event or interrupt
        nfds=SELECT(maxfds+1,&readfds,&writefds,&exceptfds,&delta);
        }

      // If no timers, we block till event or interrupt
      else{
        nfds=SELECT(maxfds+1,&readfds,&writefds,&exceptfds,NULL);
        }
      }

    // Timed out or interrupted
    if(nfds<=0){
      if(nfds<0 && errno!=EAGAIN && errno!=EINTR){fxerror("Application terminated: interrupt or lost connection errno=%d\n",errno);}
      return FALSE;
      }

    // Any other file descriptors set?
    if(0<=maxinput){

      // Try I/O channels if any are set
      for(FXInputHandle fff=0; fff<=maxinput; fff++){

        // Copy the record as the callbacks may try to change things
        FXInput in=inputs[fff];

        // Skip the display connection, which is treated differently
        if(initialized && (fff==ConnectionNumber((Display*)display))) continue;

        // Check file descriptors
        if(FD_ISSET(fff,&readfds)){
          if(in.read.target && in.read.target->handle(this,FXSEL(SEL_IO_READ,in.read.message),(void*)(FXival)fff)){
            refresh();
            }
          }
        if(FD_ISSET(fff,&writefds)){
          if(in.write.target && in.write.target->handle(this,FXSEL(SEL_IO_WRITE,in.write.message),(void*)(FXival)fff)){
            refresh();
            }
          }
        if(FD_ISSET(fff,&exceptfds)){
          if(in.excpt.target && in.excpt.target->handle(this,FXSEL(SEL_IO_EXCEPT,in.read.message),(void*)(FXival)fff)){
            refresh();
            }
          }
        }
      }

    // If there is no event, we're done
    if(!initialized || !FD_ISSET(ConnectionNumber((Display*)display),&readfds) || !XEventsQueued((Display*)display,QueuedAfterReading)) return FALSE;
    }

  // Get an event
  XNextEvent((Display*)display,&ev);

  // Filter event through input method context, if any
  if(xim && XFilterEvent(&ev,XDefaultRootWindow((Display*)display))) return FALSE;

  // Save expose events for later...
  if(ev.xany.type==Expose || ev.xany.type==GraphicsExpose){
    addRepaint((FXID)ev.xexpose.window,ev.xexpose.x,ev.xexpose.y,ev.xexpose.width,ev.xexpose.height,0);
    return FALSE;
    }

  // Compress motion events
  if(ev.xany.type==MotionNotify){
    while(XPending((Display*)display)){
      XPeekEvent((Display*)display,&e);
      if((e.xany.type!=MotionNotify) || (ev.xmotion.window!=e.xmotion.window) || (ev.xmotion.state != e.xmotion.state)) break;
      XNextEvent((Display*)display,&ev);
      }
    }

  // Compress wheel events
  else if((ev.xany.type==ButtonPress) && (ev.xbutton.button==Button4 || ev.xbutton.button==Button5)){
    ticks=1;
    while(XPending((Display*)display)){
      XPeekEvent((Display*)display,&e);
      if((e.xany.type!=ButtonPress && e.xany.type!=ButtonRelease) || (ev.xany.window!=e.xany.window) || (ev.xbutton.button != e.xbutton.button)) break;
      ticks+=(e.xany.type==ButtonPress);
      XNextEvent((Display*)display,&ev);
      }
    ev.xbutton.subwindow=(Window)ticks;   // Stick it here for later
    }

  // Compress configure events
  else if(ev.xany.type==ConfigureNotify){
    while(XCheckTypedWindowEvent((Display*)display,ev.xconfigure.window,ConfigureNotify,&e)){
      ev.xconfigure.width=e.xconfigure.width;
      ev.xconfigure.height=e.xconfigure.height;
      if(e.xconfigure.send_event){
        ev.xconfigure.x=e.xconfigure.x;
        ev.xconfigure.y=e.xconfigure.y;
        }
      }
    }

  // Regular event
  return TRUE;
  }



/*******************************************************************************/


// Peek for event
FXbool FXApp::peekEvent(){
  if(initialized){
    struct timeval delta;
    struct timeval now;
    fd_set readfds;
    fd_set writefds;
    fd_set exceptfds;
    int    maxfds;
    int    nfds;

    // Outstanding repaints
    if(repaints) return TRUE;

    // Still need GUI update
    if(refresher) return TRUE;

    // Outstanding chores
    if(chores) return TRUE;

    // Timers are due?
    if(timers){
      gettimeofday(&now,NULL);
      if(timers->due < now) return TRUE;
      }

    // Events queued up in client already (Shouldn't this not be QueuedAlready?)
    if(XEventsQueued((Display*)display,QueuedAfterFlush)) return TRUE;

    // Prepare fd's to watch
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);
    FD_SET(ConnectionNumber((Display*)display),&readfds);

    // Add other inputs...
    maxfds=ConnectionNumber((Display*)display);

    delta.tv_usec=0;
    delta.tv_sec=0;

    // Do a quick poll for any ready events
    nfds=SELECT(maxfds+1,&readfds,&writefds,&exceptfds,&delta);

    // Interrupt
    if(nfds<0 && errno!=EAGAIN && errno!=EINTR){
      fxerror("Application terminated: interrupt or lost connection errno=%d\n",errno);
      }

    // I/O activity
    if((nfds>0) && FD_ISSET(ConnectionNumber((Display*)display),&readfds)){
      if(XEventsQueued((Display*)display,QueuedAfterReading)) return TRUE;
      }
    }
  return FALSE;
  }


/*******************************************************************************/

// Dispatch event to widget
FXbool FXApp::dispatchEvent(FXRawEvent& ev){
  FXWindow *window;
  char      buf[20];
  KeySym    sym;
  Atom      answer;
  XEvent    se;
  Window    tmp;
  FXint     n;

  // Get window
  window=findWindowWithId(ev.xany.window);

  // Was one of our windows, so dispatch
  if(window){

    switch(ev.xany.type){

      // Repaint event
      case GraphicsExpose:
      case Expose:
        event.type=SEL_PAINT;
        event.rect.x=ev.xexpose.x;
        event.rect.y=ev.xexpose.y;
        event.rect.w=ev.xexpose.width;
        event.rect.h=ev.xexpose.height;
        event.synthetic=ev.xexpose.send_event;
        window->handle(this,FXSEL(SEL_PAINT,0),&event);
      case NoExpose:
        return TRUE;

      // Keyboard
      case KeyPress:
      case KeyRelease:
        event.type=SEL_KEYPRESS+ev.xkey.type-KeyPress;
        event.time=ev.xkey.time;
        event.win_x=ev.xkey.x;
        event.win_y=ev.xkey.y;
        event.root_x=ev.xkey.x_root;
        event.root_y=ev.xkey.y_root;
        event.state=ev.xkey.state&~(Button4Mask|Button5Mask);     // Exclude wheel buttons

        // Translate to keysym
        n=XLookupString(&ev.xkey,buf,sizeof(buf),&sym,NULL);
        event.code=sym;

        // Translate to string on KeyPress
        if(ev.xkey.type==KeyPress){
#ifndef NO_XIM
          if(xic){
            Status s;
            n=XmbLookupString((XIC)xic,(XKeyPressedEvent*)&ev.xkey.type,buf,sizeof(buf),&sym,&s);
            if(s!=XLookupChars && s!=XLookupBoth) n=0;
            }
#endif

          // Replace text string
          event.text.replace(0,1000,buf,n);
          }

        // Clear string on KeyRelease
        else{
          event.text.clear();
          }

        // Fix modifier state
        if(ev.xkey.type==KeyPress){
          if(sym==KEY_Shift_L) event.state|=SHIFTMASK;
          if(sym==KEY_Shift_R) event.state|=SHIFTMASK;
          if(sym==KEY_Control_L) event.state|=CONTROLMASK;
          if(sym==KEY_Control_R) event.state|=CONTROLMASK;
          if(sym==KEY_F13) event.state|=METAMASK;     // Key between Ctrl and Alt (on most keyboards)
          if(sym==KEY_Alt_L) event.state|=ALTMASK;
          if(sym==KEY_Alt_R) event.state|=ALTMASK;    // FIXME do we need ALTGR flag instead/in addition?
          }
        else{
          if(sym==KEY_Shift_L) event.state&=~SHIFTMASK;
          if(sym==KEY_Shift_R) event.state&=~SHIFTMASK;
          if(sym==KEY_Control_L) event.state&=~CONTROLMASK;
          if(sym==KEY_Control_R) event.state&=~CONTROLMASK;
          if(sym==KEY_F13) event.state&=~METAMASK;    // Key between Ctrl and Alt (on most keyboards)
          if(sym==KEY_Alt_L) event.state&=~ALTMASK;
          if(sym==KEY_Alt_R) event.state&=~ALTMASK;   // FIXME do we need ALTGR flag instead/in addition?
          }

        FXTRACE((100,"%s code=%04x state=%04x text=\"%s\"\n",(event.type==SEL_KEYPRESS)?"SEL_KEYPRESS":"SEL_KEYRELEASE",event.code,event.state,event.text.text()));

        // Dispatch to proper target
        if(keyboardGrabWindow){
          if(keyboardGrabWindow->handle(this,FXSEL(event.type,0),&event)) refresh();
          }
        else{
          if(ev.xkey.type==KeyPress) keyWindow=focusWindow;
          if(keyWindow){
            // FIXME doesSaveUnder test should go away
            if(!invocation || invocation->modality==MODAL_FOR_NONE || (invocation->window && invocation->window->isOwnerOf(keyWindow)) || keyWindow->getShell()->doesSaveUnder()){
              if(keyWindow->handle(this,FXSEL(event.type,0),&event)) refresh();
              }
            else{
              if(ev.xany.type==KeyPress) beep();
              }
            }
          }

/*
        // Try grab window
        if(keyboardGrabWindow){
          if(keyboardGrabWindow->handle(this,FXSEL(event.type,0),&event)) refresh();
          return TRUE;
          }

        // Try popup window
        if(popupWindow){
          if(popupWindow->handle(this,FXSEL(event.type,0),&event)) refresh();
          return TRUE;
          }

        // Attempt to ensure release goes to same window as press
        if(event.type==SEL_KEYPRESS) keyWindow=focusWindow;

        // Try to dispatch to the key window
        if(keyWindow){

          // Dispatch if not in a modal loop or in a modal loop for a window containing the focus window
          if(!invocation || invocation->modality==MODAL_FOR_NONE || (invocation->window && invocation->window->containsChild(keyWindow))){
            if(keyWindow->handle(this,FXSEL(event.type,0),&event)) refresh();
            return TRUE;
            }

          // Otherwise, beep
          if(ev.type==SEL_KEYPRESS) beep();
          }
*/
        return TRUE;

      // Motion
      case MotionNotify:
        event.type=SEL_MOTION;
        event.time=ev.xmotion.time;
        event.win_x=ev.xmotion.x;
        event.win_y=ev.xmotion.y;
        event.root_x=ev.xmotion.x_root;
        event.root_y=ev.xmotion.y_root;
        event.state=ev.xmotion.state&~(Button4Mask|Button5Mask);        // Exclude wheel buttons
        event.code=0;
        if((FXABS(event.root_x-event.rootclick_x)>=dragDelta) || (FXABS(event.root_y-event.rootclick_y)>=dragDelta)) event.moved=1;
        if(mouseGrabWindow){
          window->translateCoordinatesTo(event.win_x,event.win_y,mouseGrabWindow,event.win_x,event.win_y);
          if(mouseGrabWindow->handle(this,FXSEL(SEL_MOTION,0),&event)) refresh();
          }
        // FIXME doesSaveUnder test should go away
        else if(!invocation || invocation->modality==MODAL_FOR_NONE || (invocation->window && invocation->window->isOwnerOf(window)) || window->getShell()->doesSaveUnder()){
          if(window->handle(this,FXSEL(SEL_MOTION,0),&event)) refresh();
          }
        event.last_x=event.win_x;
        event.last_y=event.win_y;
        return TRUE;

      // Button
      case ButtonPress:
      case ButtonRelease:
        event.time=ev.xbutton.time;
        event.win_x=ev.xbutton.x;
        event.win_y=ev.xbutton.y;
        event.root_x=ev.xbutton.x_root;
        event.root_y=ev.xbutton.y_root;
        event.state=ev.xbutton.state&~(Button4Mask|Button5Mask);          // Exclude wheel buttons
        if(ev.xbutton.button==Button4 || ev.xbutton.button==Button5){     // Mouse wheel
          event.type=SEL_MOUSEWHEEL;
          event.code=((ev.xbutton.button==Button4)?120:-120)*ev.xbutton.subwindow;
/*
          do{
            if(window->handle(this,FXSEL(SEL_MOUSEWHEEL,0),&event)){ refresh(); break; }
            window=window->getParent();
            }
          while(window);
*/
          if(mouseGrabWindow){
            window->translateCoordinatesTo(event.win_x,event.win_y,mouseGrabWindow,event.win_x,event.win_y);
            if(mouseGrabWindow->handle(this,FXSEL(SEL_MOUSEWHEEL,0),&event)) refresh();
            }
          // FIXME doesSaveUnder test should go away
          else if(!invocation || invocation->modality==MODAL_FOR_NONE || (invocation->window && invocation->window->isOwnerOf(window)) || window->getShell()->doesSaveUnder()){
            if(window->handle(this,FXSEL(SEL_MOUSEWHEEL,0),&event)) refresh();
            }
          }
        else{                                                             // Mouse button
          event.code=ev.xbutton.button;
          if(ev.xbutton.type==ButtonPress){                               // Mouse button press
            if(ev.xbutton.button==Button1){event.type=SEL_LEFTBUTTONPRESS;event.state|=LEFTBUTTONMASK;}
            if(ev.xbutton.button==Button2){event.type=SEL_MIDDLEBUTTONPRESS;event.state|=MIDDLEBUTTONMASK;}
            if(ev.xbutton.button==Button3){event.type=SEL_RIGHTBUTTONPRESS;event.state|=RIGHTBUTTONMASK;}
            if(!event.moved && (event.time-event.click_time<clickSpeed) && (event.code==(FXint)event.click_button)){
              event.click_count++;
              event.click_time=event.time;
              }
            else{
              event.click_count=1;
              event.click_x=event.win_x;
              event.click_y=event.win_y;
              event.rootclick_x=event.root_x;
              event.rootclick_y=event.root_y;
              event.click_button=event.code;
              event.click_time=event.time;
              }
            if(!(ev.xbutton.state&(Button1Mask|Button2Mask|Button3Mask))) event.moved=0;
            }
          else{                                                           // Mouse button release
            if(ev.xbutton.button==Button1){event.type=SEL_LEFTBUTTONRELEASE;event.state&=~LEFTBUTTONMASK;}
            if(ev.xbutton.button==Button2){event.type=SEL_MIDDLEBUTTONRELEASE;event.state&=~MIDDLEBUTTONMASK;}
            if(ev.xbutton.button==Button3){event.type=SEL_RIGHTBUTTONRELEASE;event.state&=~RIGHTBUTTONMASK;}
            }

          if(mouseGrabWindow){
            window->translateCoordinatesTo(event.win_x,event.win_y,mouseGrabWindow,event.win_x,event.win_y);
            if(mouseGrabWindow->handle(this,FXSEL(event.type,0),&event)) refresh();
            }
          // FIXME doesSaveUnder test should go away
          else if(!invocation || invocation->modality==MODAL_FOR_NONE || (invocation->window && invocation->window->isOwnerOf(window)) || window->getShell()->doesSaveUnder()){
            if(window->handle(this,FXSEL(event.type,0),&event)) refresh();
            }
          else{
            if(ev.xany.type==ButtonPress) beep();
            }
          event.last_x=event.win_x;
          event.last_y=event.win_y;
          }
        return TRUE;

      // Crossing
      case EnterNotify:
      case LeaveNotify:
        event.time=ev.xcrossing.time;
        if(!mouseGrabWindow || mouseGrabWindow==window){
          if(ev.xcrossing.mode==NotifyGrab || ev.xcrossing.mode==NotifyUngrab || (ev.xcrossing.mode==NotifyNormal && ev.xcrossing.detail!=NotifyInferior)){
            event.type=SEL_ENTER+ev.xany.type-EnterNotify;
            event.win_x=ev.xcrossing.x;
            event.win_y=ev.xcrossing.y;
            event.root_x=ev.xcrossing.x_root;
            event.root_y=ev.xcrossing.y_root;
            event.code=ev.xcrossing.mode;
            if(window->handle(this,FXSEL(event.type,0),&event)) refresh();
            }
          }
        return TRUE;

      // Focus change on shell window
      case FocusIn:
      case FocusOut:
        window=window->getShell();
        if(ev.xfocus.type==FocusOut && focusWindow==window){
          event.type=SEL_FOCUSOUT;
          if(window->handle(this,FXSEL(SEL_FOCUSOUT,0),&event)) refresh();
          focusWindow=NULL;
          }
        if(ev.xfocus.type==FocusIn && focusWindow!=window){
          event.type=SEL_FOCUSIN;
          if(window->handle(this,FXSEL(SEL_FOCUSIN,0),&event)) refresh();
          focusWindow=window;
          }
        return TRUE;

      // Map
      case MapNotify:
        event.type=SEL_MAP;
        if(window->handle(this,FXSEL(SEL_MAP,0),&event)) refresh();
        return TRUE;

      // Unmap
      case UnmapNotify:
        event.type=SEL_UNMAP;
        if(window->handle(this,FXSEL(SEL_UNMAP,0),&event)) refresh();
        return TRUE;

      // Create
      case CreateNotify:
        event.type=SEL_CREATE;
        if(window->handle(this,FXSEL(SEL_CREATE,0),&event)) refresh();
        return TRUE;

      // Destroy
      case DestroyNotify:
        event.type=SEL_DESTROY;
        if(window->handle(this,FXSEL(SEL_DESTROY,0),&event)) refresh();
        return TRUE;

      // Configure
      case ConfigureNotify:
        event.type=SEL_CONFIGURE;
        // According to the ICCCM, if its synthetic, the coordinates are relative
        // to root window; otherwise, they're relative to the parent; so we use
        // the old coordinates if its not a synthetic configure notify
        if(window->getShell()==window && !ev.xconfigure.send_event){
          ev.xconfigure.x=window->getX();
          ev.xconfigure.y=window->getY();
          }
        event.rect.x=ev.xconfigure.x;
        event.rect.y=ev.xconfigure.y;
        event.rect.w=ev.xconfigure.width;
        event.rect.h=ev.xconfigure.height;
        event.synthetic=ev.xconfigure.send_event;
        if(window->handle(this,FXSEL(SEL_CONFIGURE,0),&event)) refresh();
        return TRUE;

      // Circulate
      case CirculateNotify:
        event.type=SEL_RAISED+(ev.xcirculate.place&1);
        if(window->handle(this,FXSEL(event.type,0),&event)) refresh();
        return TRUE;

      // Selection Clear
      case SelectionClear:
        if(ev.xselectionclear.selection==XA_PRIMARY){
          if(selectionWindow){
            event.type=SEL_SELECTION_LOST;
            event.time=ev.xselectionclear.time;
            if(selectionWindow->handle(this,FXSEL(SEL_SELECTION_LOST,0),&event)) refresh();
            selectionWindow=NULL;
            }
          FXFREE(&xselTypeList);
          xselNumTypes=0;
          }
        else if(ev.xselectionclear.selection==xcbSelection){
          if(clipboardWindow){
            event.time=ev.xselectionclear.time;
            event.type=SEL_CLIPBOARD_LOST;
            if(clipboardWindow->handle(this,FXSEL(SEL_CLIPBOARD_LOST,0),&event)) refresh();
            clipboardWindow=NULL;
            }
          FXFREE(&xcbTypeList);
          xcbNumTypes=0;
          }
        return TRUE;

      // Selection Request
      case SelectionRequest:
        answer=None;
        if(ev.xselectionrequest.selection==XA_PRIMARY){
          if(selectionWindow){
            if(ev.xselectionrequest.target==ddeTargets){            // Request for TYPES
              FXTRACE((100,"Window %ld being requested by window %ld for SELECTION TYPES; sending %d types\n",ev.xselectionrequest.owner,ev.xselectionrequest.requestor,xselNumTypes));
              answer=fxsendtypes((Display*)display,ev.xselectionrequest.requestor,ev.xselectionrequest.property,xselTypeList,xselNumTypes);
              }
            else{                                                   // Request for DATA
              event.type=SEL_SELECTION_REQUEST;
              event.time=ev.xselectionrequest.time;
              event.target=ev.xselectionrequest.target;
              ddeData=NULL;
              ddeSize=0;
              selectionWindow->handle(this,FXSEL(SEL_SELECTION_REQUEST,0),&event);
              FXTRACE((100,"Window %ld being requested by window %ld for SELECTION DATA of type %ld; sending %d bytes\n",ev.xselectionrequest.owner,ev.xselectionrequest.requestor,ev.xselectionrequest.target,ddeSize));
              answer=fxsenddata((Display*)display,ev.xselectionrequest.requestor,ev.xselectionrequest.property,ev.xselectionrequest.target,ddeData,ddeSize);
              FXFREE(&ddeData);
              ddeData=NULL;
              ddeSize=0;
              }
            }
          }
        else if(ev.xselectionrequest.selection==xcbSelection){
          if(clipboardWindow){
            if(ev.xselectionrequest.target==ddeTargets){            // Request for TYPES
              FXTRACE((100,"Window %ld being requested by window %ld for CLIPBOARD TYPES; sending %d types\n",ev.xselectionrequest.owner,ev.xselectionrequest.requestor,xcbNumTypes));
              answer=fxsendtypes((Display*)display,ev.xselectionrequest.requestor,ev.xselectionrequest.property,xcbTypeList,xcbNumTypes);
              }
            else{                                                   // Request for DATA
              event.type=SEL_CLIPBOARD_REQUEST;
              event.time=ev.xselectionrequest.time;
              event.target=ev.xselectionrequest.target;
              ddeData=NULL;
              ddeSize=0;
              clipboardWindow->handle(this,FXSEL(SEL_CLIPBOARD_REQUEST,0),&event);
              FXTRACE((100,"Window %ld being requested by window %ld for CLIPBOARD DATA of type %ld; sending %d bytes\n",ev.xselectionrequest.owner,ev.xselectionrequest.requestor,ev.xselectionrequest.target,ddeSize));
              answer=fxsenddata((Display*)display,ev.xselectionrequest.requestor,ev.xselectionrequest.property,ev.xselectionrequest.target,ddeData,ddeSize);
              FXFREE(&ddeData);
              ddeData=NULL;
              ddeSize=0;
              }
            }
          }
        else if(ev.xselectionrequest.selection==xdndSelection){
          if(dragWindow){
            if(ev.xselectionrequest.target==ddeTargets){            // Request for TYPES
              FXTRACE((100,"Window %ld being requested by window %ld for XDND TYPES; sending %d types\n",ev.xselectionrequest.owner,ev.xselectionrequest.requestor,xdndNumTypes));
              answer=fxsendtypes((Display*)display,ev.xselectionrequest.requestor,ev.xselectionrequest.property,xdndTypeList,xdndNumTypes);
              }
            else{                                                   // Request for DATA
              event.type=SEL_DND_REQUEST;
              event.time=ev.xselectionrequest.time;
              event.target=ev.xselectionrequest.target;
              ddeData=NULL;
              ddeSize=0;
              dragWindow->handle(this,FXSEL(SEL_DND_REQUEST,0),&event);
              FXTRACE((100,"Window %ld being requested by window %ld for XDND DATA of type %ld; sending %d bytes\n",ev.xselectionrequest.owner,ev.xselectionrequest.requestor,ev.xselectionrequest.target,ddeSize));
              answer=fxsenddata((Display*)display,ev.xselectionrequest.requestor,ev.xselectionrequest.property,ev.xselectionrequest.target,ddeData,ddeSize);
              FXFREE(&ddeData);
              ddeData=NULL;
              ddeSize=0;
              }
            }
          }
        FXTRACE((100,"Sending back response to requestor=%ld\n",ev.xselectionrequest.requestor));
        fxsendreply((Display*)display,ev.xselectionrequest.requestor,ev.xselectionrequest.selection,answer,ev.xselectionrequest.target,ev.xselectionrequest.time);
        return TRUE;

      // Selection Notify
      case SelectionNotify:
        return TRUE;

      // Client message
      case ClientMessage:

        // WM_PROTOCOLS
        if(ev.xclient.message_type==wmProtocols){
          if((FXID)ev.xclient.data.l[0]==wmDeleteWindow){           // WM_DELETE_WINDOW
            event.type=SEL_CLOSE;
            if(!invocation || invocation->modality==MODAL_FOR_NONE || (invocation->window && invocation->window->isOwnerOf(window))){
              if(window->handle(this,FXSEL(SEL_CLOSE,0),&event)) refresh();
              }
            else{
              beep();
              }
            }
          else if((FXID)ev.xclient.data.l[0]==wmQuitApp){           // WM_QUIT_APP
            event.type=SEL_CLOSE;
            if(!invocation || invocation->modality==MODAL_FOR_NONE || (invocation->window && invocation->window->isOwnerOf(window))){
              if(window->handle(this,FXSEL(SEL_CLOSE,0),&event)) refresh();
              }
            else{
              beep();
              }
            }
          else if((FXID)ev.xclient.data.l[0]==wmTakeFocus){         // WM_TAKE_FOCUS
            if(invocation && invocation->window && invocation->window->id()) ev.xclient.window=invocation->window->id();
            // Assign focus to innermost modal dialog, even when trying to focus
            // on another window; these other windows are dead to inputs anyway.
            // XSetInputFocus causes a spurious BadMatch error; we ignore this in xerrorhandler
            XSetInputFocus((Display*)display,ev.xclient.window,RevertToParent,ev.xclient.data.l[1]);
            }
          }

        // XDND Enter from source
        else if(ev.xclient.message_type==xdndEnter){
          FXint ver=(ev.xclient.data.l[1]>>24)&255;
          FXTRACE((100,"DNDEnter from remote window %ld\n",ev.xclient.data.l[0]));
          if(ver>XDND_PROTOCOL_VERSION) return TRUE;
          xdndSource=ev.xclient.data.l[0];                                  // Now we're talking to this guy
          if(ddeTypeList){FXFREE(&ddeTypeList);ddeNumTypes=0;}
          if(ev.xclient.data.l[1]&1){
            fxrecvtypes((Display*)display,xdndSource,xdndTypes,ddeTypeList,ddeNumTypes);
            }
          else{
            FXMALLOC(&ddeTypeList,FXDragType,3);
            ddeNumTypes=0;
            if(ev.xclient.data.l[2]){ddeTypeList[0]=ev.xclient.data.l[2];ddeNumTypes++;}
            if(ev.xclient.data.l[3]){ddeTypeList[1]=ev.xclient.data.l[3];ddeNumTypes++;}
            if(ev.xclient.data.l[4]){ddeTypeList[2]=ev.xclient.data.l[4];ddeNumTypes++;}
            }
          }

        // XDND Leave from source
        else if(ev.xclient.message_type==xdndLeave){
          FXTRACE((100,"DNDLeave from remote window %ld\n",ev.xclient.data.l[0]));
          if(xdndSource!=(FXID)ev.xclient.data.l[0]) return TRUE;   // We're not talking to this guy
          if(dropWindow){
            event.type=SEL_DND_LEAVE;
            if(dropWindow->handle(this,FXSEL(SEL_DND_LEAVE,0),&event)) refresh();
            dropWindow=NULL;
            }
          if(ddeTypeList){FXFREE(&ddeTypeList);ddeNumTypes=0;}
          xdndSource=0;
          }

        // XDND Position from source
        else if(ev.xclient.message_type==xdndPosition){
          FXTRACE((100,"DNDPosition from remote window %ld\n",ev.xclient.data.l[0]));
          if(xdndSource!=(FXID)ev.xclient.data.l[0]) return TRUE;   // We're not talking to this guy
          event.time=ev.xclient.data.l[3];
          event.root_x=((FXuint)ev.xclient.data.l[2])>>16;
          event.root_y=((FXuint)ev.xclient.data.l[2])&0xffff;
          // Search from target window down; there may be another window
          // (like e.g. the dragged shape window) right under the cursor.
          // Note this is the target window, not the proxy target....
          window=findWindowAt(event.root_x,event.root_y,ev.xclient.window);
          if((FXID)ev.xclient.data.l[4]==xdndActionCopy) ddeAction=DRAG_COPY;
          else if((FXID)ev.xclient.data.l[4]==xdndActionMove) ddeAction=DRAG_MOVE;
          else if((FXID)ev.xclient.data.l[4]==xdndActionLink) ddeAction=DRAG_LINK;
          else if((FXID)ev.xclient.data.l[4]==xdndActionPrivate) ddeAction=DRAG_PRIVATE;
          else ddeAction=DRAG_COPY;
          ansAction=DRAG_REJECT;
          xdndWantUpdates=TRUE;
          xdndRect.x=event.root_x;
          xdndRect.y=event.root_y;
          xdndRect.w=1;
          xdndRect.h=1;
          if(window!=dropWindow){
            if(dropWindow){
              event.type=SEL_DND_LEAVE;
              if(dropWindow->handle(this,FXSEL(SEL_DND_LEAVE,0),&event)) refresh();
              }
            dropWindow=NULL;
            if(window && window->isDropEnabled()){
              dropWindow=window;
              event.type=SEL_DND_ENTER;
              if(dropWindow->handle(this,FXSEL(SEL_DND_ENTER,0),&event)) refresh();
              }
            }
          if(dropWindow){
            event.type=SEL_DND_MOTION;
            XTranslateCoordinates((Display*)display,XDefaultRootWindow((Display*)display),dropWindow->id(),event.root_x,event.root_y,&event.win_x,&event.win_y,&tmp);
            if(dropWindow->handle(this,FXSEL(SEL_DND_MOTION,0),&event)) refresh();
            }
          se.xclient.type=ClientMessage;
          se.xclient.display=(Display*)display;
          se.xclient.message_type=xdndStatus;
          se.xclient.format=32;
          se.xclient.window=xdndSource;
          se.xclient.data.l[0]=ev.xclient.window;                   // Proxy Target window
          se.xclient.data.l[1]=0;
          if(ansAction!=DRAG_REJECT) se.xclient.data.l[1]|=1;       // Target accepted
          if(xdndWantUpdates) se.xclient.data.l[1]|=2;              // Target wants continuous position updates
          se.xclient.data.l[2]=MKUINT(xdndRect.y,xdndRect.x);
          se.xclient.data.l[3]=MKUINT(xdndRect.h,xdndRect.w);
          if(ansAction==DRAG_COPY) se.xclient.data.l[4]=xdndActionCopy; // Drag and Drop Action accepted
          else if(ansAction==DRAG_MOVE) se.xclient.data.l[4]=xdndActionMove;
          else if(ansAction==DRAG_LINK) se.xclient.data.l[4]=xdndActionLink;
          else if(ansAction==DRAG_PRIVATE) se.xclient.data.l[4]=xdndActionPrivate;
          else se.xclient.data.l[4]=None;
          XSendEvent((Display*)display,xdndSource,True,NoEventMask,&se);
          }

        // XDND Drop from source
        else if(ev.xclient.message_type==xdndDrop){
          FXTRACE((100,"DNDDrop from remote window %ld\n",ev.xclient.data.l[0]));
          if(xdndSource!=(FXID)ev.xclient.data.l[0]) return TRUE;   // We're not talking to this guy
          se.xclient.type=ClientMessage;
          se.xclient.display=(Display*)display;
          se.xclient.message_type=xdndFinished;
          se.xclient.format=32;
          se.xclient.window=xdndSource;
          se.xclient.data.l[0]=ev.xclient.window;                   // Proxy Target window
          se.xclient.data.l[1]=0;                                   // Assume drop not accepted
          se.xclient.data.l[2]=None;                                // Action performed by target
          se.xclient.data.l[3]=0;
          se.xclient.data.l[4]=0;
          if(dropWindow){
            event.type=SEL_DND_DROP;
            event.time=ev.xclient.data.l[2];
            if(dropWindow->handle(this,FXSEL(SEL_DND_DROP,0),&event)){
              se.xclient.data.l[1]|=1;                              // Drop was accepted (bit #0)
              if(ansAction==DRAG_COPY) se.xclient.data.l[2]=xdndActionCopy;             // Action performed by target
              else if(ansAction==DRAG_MOVE) se.xclient.data.l[2]=xdndActionMove;        // For now, same as our last
              else if(ansAction==DRAG_LINK) se.xclient.data.l[2]=xdndActionLink;        // answer from the xdndPosition
              else if(ansAction==DRAG_PRIVATE) se.xclient.data.l[2]=xdndActionPrivate;
              refresh();
              }
            dropWindow=NULL;
            }
          XSendEvent((Display*)display,xdndSource,True,NoEventMask,&se);
          if(ddeTypeList){FXFREE(&ddeTypeList);ddeNumTypes=0;}
          xdndSource=0;
          }

        // XDND Status from target
        else if(ev.xclient.message_type==xdndStatus){
          // We ignore ev.xclient.data.l[0], because some other
          // toolkits, e.g. Qt, do not place the proper value there;
          // the proper value is xdndTarget, NOT xdndProxyTarget or None
          //if(xdndTarget!=(FXID)ev.xclient.data.l[0]) return TRUE; // We're not talking to this guy
          ansAction=DRAG_REJECT;
          if(ev.xclient.data.l[1]&1){
            if((FXID)ev.xclient.data.l[4]==xdndActionCopy) ansAction=DRAG_COPY;
            else if((FXID)ev.xclient.data.l[4]==xdndActionMove) ansAction=DRAG_MOVE;
            else if((FXID)ev.xclient.data.l[4]==xdndActionLink) ansAction=DRAG_LINK;
            else if((FXID)ev.xclient.data.l[4]==xdndActionPrivate) ansAction=DRAG_PRIVATE;
            }
          xdndWantUpdates=ev.xclient.data.l[1]&2;
          xdndRect.x=((FXuint)ev.xclient.data.l[2])>>16;
          xdndRect.y=((FXuint)ev.xclient.data.l[2])&0xffff;
          xdndRect.w=((FXuint)ev.xclient.data.l[3])>>16;
          xdndRect.h=((FXuint)ev.xclient.data.l[3])&0xffff;
          xdndStatusReceived=TRUE;
          xdndStatusPending=FALSE;
          FXTRACE((100,"DNDStatus from remote window %ld action=%d rect=%d,%d,%d,%d updates=%d\n",ev.xclient.data.l[0],ansAction,xdndRect.x,xdndRect.y,xdndRect.w,xdndRect.h,xdndWantUpdates));
          }
        return TRUE;

      // Property change
      case PropertyNotify:
        event.time=ev.xproperty.time;
//         {char* atomname=XGetAtomName((Display*)display,ev.xproperty.atom);
//         FXTRACE((100,"PropertyNotify %s\n",atomname));
//         XFree(atomname);
//         }
//         if(ev.xproperty.atom==wmState){
//           FXTRACE((100,"Window State Change\n"));
//           }
        return TRUE;

      // Keyboard mapping
      case MappingNotify:
        FXTRACE((100,"MappingNotify\n"));
        if(ev.xmapping.request!=MappingPointer) XRefreshKeyboardMapping(&ev.xmapping);
        return TRUE;
      }
    }
  return FALSE;
  }


#else


// Get an event
FXbool FXApp::getNextEvent(FXRawEvent& msg,FXbool blocking){
  register FXint allinputs;
  register DWORD  signalled;
  FXlong now,delta;

  // Set to no-op just in case
  msg.message=0;

  // Handle all past due timers
  now=getticktime();
  while(timers){
    register FXTimer* t=timers;
    if(now < t->due) break;
    timers=t->next;
    if(t->target && t->target->handle(this,FXSEL(SEL_TIMEOUT,t->message),t->data)) refresh();
    t->next=timerrecs;
    timerrecs=t;
    }

  // Check non-immediate signals that may have fired
  if(nsignals){
    for(register FXint sig=0; sig<MAXSIGNALS; sig++){
      if(signals[sig].notified){
        signals[sig].notified=FALSE;
        if(signals[sig].target && signals[sig].target->handle(this,FXSEL(SEL_SIGNAL,signals[sig].message),(void*)(FXival)sig)){
          refresh();
          return FALSE;
          }
        }
      }
    }

  // Peek for messages; this marks the message queue as unsignalled, i.e.
  // MsgWaitForMultipleObjects would block even if there are unhandled events;
  // the fix is to call MsgWaitForMultipleObjects only AFTER having ascertained
  // that there are NO unhandled events queued up.
  if(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) return TRUE;

  // Poll to see if any waitable objects are signalled
  allinputs=maxinput+1;
  signalled=MsgWaitForMultipleObjects(allinputs,handles,FALSE,0,QS_ALLINPUT);

  // No objects were signalled, so perform background tasks now
  if(signalled==WAIT_TIMEOUT){

    // Do our chores :-)
    if(chores){
      register FXChore *c=chores;
      chores=c->next;
      if(c->target && c->target->handle(this,FXSEL(SEL_CHORE,c->message),c->data)) refresh();
      c->next=chorerecs;
      chorerecs=c;
      }

    // GUI updating:- walk the whole widget tree.
    if(refresher){
      refresher->handle(this,FXSEL(SEL_UPDATE,0),NULL);
      if(refresher->getFirst()){
        refresher=refresher->getFirst();
        }
      else{
        while(refresher->getParent()){
          if(refresher->getNext()){
            refresher=refresher->getNext();
            break;
            }
          refresher=refresher->getParent();
          }
        }
      FXASSERT(refresher);
      if(refresher!=refresherstop) return FALSE;
      refresher=refresherstop=NULL;
      }

    // There are more chores to do
    if(chores) return FALSE;

    // No updates or chores pending, so return at this point if not blocking
    if(!blocking) return FALSE;

    // One more call to PeekMessage here because the preceding idle
    // processing may have caused some more messages to be posted to
    // our message queue:- a call to MsgWaitForMultipleObjects when
    // there are events already in the queue would NOT immediately fall
    // through but block until the next event comes in.
    if(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) return TRUE;

    // If there are timers, block only a little time
    allinputs=maxinput+1;
    if(timers){

      // Read the clock again
      now=getticktime();

      // How long to wait
      delta=timers->due-now;

      // Some timers are already due, so go do them now
      if(delta<=0) return FALSE;

      // Now we will block...
      signalled=MsgWaitForMultipleObjects(allinputs,handles,FALSE,(DWORD)delta,QS_ALLINPUT);
      }

    // No timers, so block indefinitely
    else{

      // Now we will block...
      signalled=MsgWaitForMultipleObjects(allinputs,handles,FALSE,INFINITE,QS_ALLINPUT);
      }
    }

  // Timed out, so do timeouts
  if(signalled==WAIT_TIMEOUT) return FALSE;

  // Signallable object was signalled
  if(WAIT_OBJECT_0<=signalled && signalled<WAIT_OBJECT_0+allinputs){

    // Process ALL objects which are signalled after returning from
    // MsgWaitForMultipleObjects. We copy the stuff out of the arrays
    // before issueing callbacks, in case an entry is removed.
    for(FXint i=0; i<=maxinput; i++){
      register FXInputHandle fff=handles[i];
      if((i==signalled-WAIT_OBJECT_0) || (WaitForSingleObject(fff,0)==WAIT_OBJECT_0)){
        FXInput in=inputs[i];
        if(in.read.target && in.read.target->handle(this,FXSEL(SEL_IO_READ,in.read.message),(void*)(FXival)fff)){
          refresh();
          }
        if(in.write.target && in.write.target->handle(this,FXSEL(SEL_IO_WRITE,in.write.message),(void*)(FXival)fff)){
          refresh();
          }
        if(in.excpt.target && in.excpt.target->handle(this,FXSEL(SEL_IO_EXCEPT,in.excpt.message),(void*)(FXival)fff)){
          refresh();
          }
        }
      }
    }

  // Got message from the GUI?
  if(signalled!=WAIT_OBJECT_0+allinputs) return FALSE;

  // Get the event; this used to be GetMessage(&msg,NULL,0,0),
  // but for some reason, this occasionally blocks even though we
  // have tried to make sure an event was indeed available.
  // The new code will always fall through, with an event if there
  // is one, or without one if despite our efforts above there wasn't.
  // Thanks to Hodju Petri <phodju@cc.hut.fi> for this suggestion.
  return PeekMessage(&msg,NULL,0,0,PM_REMOVE);
  }


// Peek for event
FXbool FXApp::peekEvent(){
  if(initialized){
    FXlong now;
    MSG    msg;

    // Still need GUI update
    if(refresher) return TRUE;

    // Outstanding chores
    if(chores) return TRUE;

    // Timers are due?
    if(timers){
      now=getticktime();
      if(timers->due < now) return TRUE;
      }

    // Other events due?
    return PeekMessage(&msg,NULL,0,0,PM_NOREMOVE)!=0;
    }
  return FALSE;
  }


// Dispatch event to widget
FXbool FXApp::dispatchEvent(FXRawEvent& msg){
  DispatchMessage(&msg);
  // FIXME should return TRUE only when handled in FOX
  return TRUE;
  }


#endif

/*******************************************************************************/


// Flush pending repaints
void FXApp::flush(FXbool sync){
  if(initialized){
#ifndef WIN32
    if(sync)
      XSync((Display*)display,FALSE);
    else
      XFlush((Display*)display);
#else
    GdiFlush();
#endif
    }
  }


// Force GUI refresh of every widget in this application
void FXApp::forceRefresh(){
  getRootWindow()->forceRefresh();
  }


// Schedule a future refresh; if we were in the middle of
// one, we continue with the current cycle until we wrap
// around to the current widget about to be updated.
void FXApp::refresh(){
  if(!refresher) refresher=root;
  refresherstop=refresher;
  }


// Paint all windows marked for repainting
void FXApp::repaint(){
  if(initialized){
#ifndef WIN32
    removeRepaints(0,0,0,0,0);
#else
    for(FXWindow *top=root->getFirst(); top; top=top->getNext()){
      RedrawWindow((HWND)top->id(),NULL,NULL,RDW_ERASENOW|RDW_UPDATENOW|RDW_ALLCHILDREN);
      }
#endif
    }
  }


// Run application
FXint FXApp::run(){
  FXInvocation inv(&invocation,MODAL_FOR_NONE,NULL);
  FXTRACE((100,"Start run\n"));
  while(!inv.done){
    runOneEvent();
    }
  FXTRACE((100,"End run\n"));
  return inv.code;
  }


// Run till some flag becomes non-zero
FXint FXApp::runUntil(FXuint& condition){
  FXInvocation inv(&invocation,MODAL_FOR_NONE,NULL);
  FXTRACE((100,"Start runUntil\n"));
  while(!inv.done && condition==0){
    runOneEvent();
    }
  FXTRACE((100,"End runUntil\n"));
  return condition;
  }


// Run event loop while events are available
FXint FXApp::runWhileEvents(){
  FXInvocation inv(&invocation,MODAL_FOR_NONE,NULL);
  FXTRACE((100,"Start runWhileEvents\n"));
  while(!inv.done && runOneEvent(FALSE));
  FXTRACE((100,"End runWhileEvents\n"));
  return !inv.done;
  }


// Run event loop while events are available
FXint FXApp::runModalWhileEvents(FXWindow* window){
  FXInvocation inv(&invocation,MODAL_FOR_WINDOW,window);
  FXTRACE((100,"Start runModalWhileEvents\n"));
  while(!inv.done && runOneEvent(FALSE));
  FXTRACE((100,"End runModalWhileEvents\n"));
  return !inv.done;
  }


// Perform one event dispatch
FXbool FXApp::runOneEvent(FXbool blocking){
  FXRawEvent ev;
  if(getNextEvent(ev,blocking)){
    dispatchEvent(ev);
    return TRUE;
    }
  return FALSE;
  }


// Run modal event loop, blocking events to all windows, until stopModal is called.
FXint FXApp::runModal(){
  FXInvocation inv(&invocation,MODAL_FOR_WINDOW,NULL);
  FXTRACE((100,"Start runModal\n"));
  while(!inv.done){
    runOneEvent();
    }
  FXTRACE((100,"End runModal\n"));
  return inv.code;
  }


// Run modal for window
FXint FXApp::runModalFor(FXWindow* window){
  FXInvocation inv(&invocation,MODAL_FOR_WINDOW,window);
  FXTRACE((1,"Start runModalFor\n"));
  while(!inv.done){
    runOneEvent();
    }
  FXTRACE((1,"End runModalFor\n"));
  return inv.code;
  }


// Run modal while window is shown, or until stopModal is called
FXint FXApp::runModalWhileShown(FXWindow* window){
  FXInvocation inv(&invocation,MODAL_FOR_WINDOW,window);
  FXTRACE((100,"Start runModalWhileShown\n"));
  while(!inv.done && window->shown()){
    runOneEvent();
    }
  FXTRACE((100,"End runModalWhileShown\n"));
  return inv.code;
  }


// Run popup menu
FXint FXApp::runPopup(FXWindow* window){
  FXInvocation inv(&invocation,MODAL_FOR_POPUP,window);
  FXTRACE((100,"Start runPopup\n"));
  while(!inv.done && window->shown()){
    runOneEvent();
    }
  FXTRACE((100,"End runPopup\n"));
  return inv.code;
  }


// Test if the window is involved in a modal invocation
FXbool FXApp::isModal(FXWindow *window) const {
  register FXInvocation* inv;
  for(inv=invocation; inv; inv=inv->upper){
    if(inv->window==window && inv->modality!=MODAL_FOR_NONE) return TRUE;
    }
  return FALSE;
  }


// Return current modal window, if any
FXWindow* FXApp::getModalWindow() const {
  return invocation ? invocation->window : NULL;
  }


// Return mode of current modal loop
FXModality FXApp::getModality() const {
  return invocation ? invocation->modality : MODAL_FOR_NONE;
  }


// Break out of topmost event loop, closing all nested loops also
void FXApp::stop(FXint value){
  register FXInvocation* inv;
  for(inv=invocation; inv; inv=inv->upper){
    inv->done=TRUE;
    inv->code=0;
    if(inv->upper==NULL){
      inv->code=value;
      return;
      }
    }
  }


// Break out of modal loop matching window, and all deeper ones
void FXApp::stopModal(FXWindow* window,FXint value){
  register FXInvocation* inv;
  if(isModal(window)){
    for(inv=invocation; inv; inv=inv->upper){
      inv->done=TRUE;
      inv->code=0;
      if(inv->window==window && inv->modality!=MODAL_FOR_NONE){
        inv->code=value;
        return;
        }
      }
    }
  }


// Break out of innermost modal loop, and all deeper non-modal ones
void FXApp::stopModal(FXint value){
  register FXInvocation* inv;
  for(inv=invocation; inv; inv=inv->upper){
    inv->done=TRUE;
    inv->code=0;
    if(inv->modality!=MODAL_FOR_NONE){
      inv->code=value;
      return;
      }
    }
  }


// Initialize application
void FXApp::init(int& argc,char** argv,FXbool connect){
  const FXchar *fontspec,*d;
  FXuint maxcols=0;
  FXint i,j;

  // Verify implementation invariants
  FXASSERT(sizeof(FXuchar)==1);
  FXASSERT(sizeof(FXbool)==1);
  FXASSERT(sizeof(FXchar)==1);
  FXASSERT(sizeof(FXushort)==2);
  FXASSERT(sizeof(FXshort)==2);
  FXASSERT(sizeof(FXuint)==4);
  FXASSERT(sizeof(FXwchar)==4);
  FXASSERT(sizeof(FXint)==4);
  FXASSERT(sizeof(FXfloat)==4);
  FXASSERT(sizeof(FXdouble)==8);
  FXASSERT(sizeof(FXival)==sizeof(void*));
  FXASSERT(sizeof(FXuval)==sizeof(void*));
#ifdef WIN32
  FXASSERT(sizeof(HWND)==sizeof(FXID));
#else
  FXASSERT(sizeof(Window)==sizeof(FXID));
#endif

  // Long is not always available on all implementations
#ifdef FX_LONG
  FXASSERT(sizeof(FXulong)==8);
  FXASSERT(sizeof(FXlong)==8);
#endif

  // Initialize locale
#if defined(__BCPLUSPLUS__) || defined(__BORLANDC__)
  setlocale(LC_CTYPE,"C");  // Locale support broken under Borland C++ 5.5
#else
  setlocale(LC_CTYPE,"");
#endif

  // Try locate display
#ifndef WIN32
  if((d=getenv("DISPLAY"))!=NULL) dpy=d;
#endif

  //fxisconsole(argv[0]);

  // Parse out FOX args
  i=j=1;
  while(j<argc){

#ifndef WIN32

    // Start synchronized mode
    if(strcmp(argv[j],"-sync")==0){
      synchronize=TRUE;
      j++;
      continue;
      }

    // Do not use X shared memory extension, even if available
    if(strcmp(argv[j],"-noshm")==0){
      shmi=FALSE;
      shmp=FALSE;
      j++;
      continue;
      }

    // Force use X shared memory extension, if available
    if(strcmp(argv[j],"-shm")==0){
      shmi=TRUE;
      shmp=TRUE;
      j++;
      continue;
      }

    // Alternative display
    if(strcmp(argv[j],"-display")==0){
      j++;
      if(j>=argc){
        fxwarning("%s:init: missing argument for -display.\n",getClassName());
        ::exit(1);
        }
      if(argv[i]) dpy=argv[j];
      j++;
      continue;
      }

#endif

    // Set trace level
    if(strcmp(argv[j],"-tracelevel")==0){
      j++;
      if(j>=argc){
        fxwarning("%s:init: missing argument for -tracelevel.\n",getClassName());
        ::exit(1);
        }
      if(sscanf(argv[j],"%d",&fxTraceLevel)!=1){
        fxwarning("%s::init: expected trace level number.\n",getClassName());
        ::exit(1);
        }
      j++;
      continue;
      }

    // Set maximum number of colors
    if(strcmp(argv[j],"-maxcolors")==0){
      j++;
      if(j>=argc){
        fxwarning("%s:init: missing argument for -maxcolors.\n",getClassName());
        ::exit(1);
        }
      if(sscanf(argv[j],"%u",&maxcols)!=1 || maxcols<2){
        fxwarning("%s::init: expected number of colors > 2.\n",getClassName());
        ::exit(1);
        }
      j++;
      continue;
      }

    // Copy program arguments
    argv[i++]=argv[j++];
    }

  // Adjust argment count
  argv[i]=NULL;
  argc=i;

  // Remember arguments
  appArgv=argv;
  appArgc=argc;

  // Log message
  FXTRACE((100,"%s::init\n",getClassName()));

  // Read the registry
  registry.read();

#ifdef WIN32

  // Get font face and metrics
  NONCLIENTMETRICS ncm;
  FXFontDesc fontdesc;
  ncm.cbSize=sizeof(NONCLIENTMETRICS);
  SystemParametersInfo(SPI_GETNONCLIENTMETRICS,sizeof(NONCLIENTMETRICS),&ncm,0);
  strncpy(fontdesc.face,ncm.lfMenuFont.lfFaceName,sizeof(fontdesc.face));
  fontdesc.face[sizeof(fontdesc.face)-1]='\0';
  HDC hDC=CreateCompatibleDC(NULL);
  fontdesc.size=-10*MulDiv(ncm.lfMenuFont.lfHeight,72,GetDeviceCaps(hDC,LOGPIXELSY));
  DeleteDC(hDC);
  fontdesc.weight=ncm.lfMenuFont.lfWeight;
  fontdesc.slant=ncm.lfMenuFont.lfItalic?FONTSLANT_ITALIC:FONTSLANT_REGULAR;
  fontdesc.encoding=FONTENCODING_DEFAULT;
  fontdesc.setwidth=FONTSETWIDTH_DONTCARE;
  fontdesc.flags=0;

  // Set new font
  normalFont->setFontDesc(fontdesc);

  // Init colors
  DWORD dwColor;
  dwColor=GetSysColor(COLOR_3DFACE);
  baseColor=FXRGB(GetRValue(dwColor),GetGValue(dwColor),GetBValue(dwColor));
  dwColor=GetSysColor(COLOR_3DHILIGHT);
  hiliteColor=FXRGB(GetRValue(dwColor),GetGValue(dwColor),GetBValue(dwColor));
  dwColor=GetSysColor(COLOR_3DSHADOW);
  shadowColor=FXRGB(GetRValue(dwColor),GetGValue(dwColor),GetBValue(dwColor));
  dwColor=GetSysColor(COLOR_WINDOW);
  backColor=FXRGB(GetRValue(dwColor),GetGValue(dwColor),GetBValue(dwColor));
  dwColor=GetSysColor(COLOR_WINDOWFRAME);
  borderColor=FXRGB(GetRValue(dwColor),GetGValue(dwColor),GetBValue(dwColor));
  dwColor=GetSysColor(COLOR_BTNTEXT);
  foreColor=FXRGB(GetRValue(dwColor),GetGValue(dwColor),GetBValue(dwColor));
  dwColor=GetSysColor(COLOR_HIGHLIGHTTEXT);
  selforeColor=FXRGB(GetRValue(dwColor),GetGValue(dwColor),GetBValue(dwColor));
  dwColor=GetSysColor(COLOR_HIGHLIGHT);
  selbackColor=FXRGB(GetRValue(dwColor),GetGValue(dwColor),GetBValue(dwColor));
  dwColor=GetSysColor(COLOR_INFOTEXT);
  tipforeColor=FXRGB(GetRValue(dwColor),GetGValue(dwColor),GetBValue(dwColor));
  dwColor=GetSysColor(COLOR_INFOBK);
  tipbackColor=FXRGB(GetRValue(dwColor),GetGValue(dwColor),GetBValue(dwColor));

  // Get wheel lines
  SystemParametersInfo(SPI_GETWHEELSCROLLLINES,0,&wheelLines,0);

#endif

  // Parse font and change default font if set
  fontspec=registry.readStringEntry("SETTINGS","normalfont",NULL);
  if(fontspec){
    normalFont->setFont(fontspec);
    }

  // Change some settings
  typingSpeed=registry.readUnsignedEntry("SETTINGS","typingspeed",typingSpeed);
  clickSpeed=registry.readUnsignedEntry("SETTINGS","clickspeed",clickSpeed);
  scrollSpeed=registry.readUnsignedEntry("SETTINGS","scrollspeed",scrollSpeed);
  scrollDelay=registry.readUnsignedEntry("SETTINGS","scrolldelay",scrollDelay);
  blinkSpeed=registry.readUnsignedEntry("SETTINGS","blinkspeed",blinkSpeed);
  animSpeed=registry.readUnsignedEntry("SETTINGS","animspeed",animSpeed);
  menuPause=registry.readUnsignedEntry("SETTINGS","menupause",menuPause);
  tooltipPause=registry.readUnsignedEntry("SETTINGS","tippause",tooltipPause);
  tooltipTime=registry.readUnsignedEntry("SETTINGS","tiptime",tooltipTime);
  dragDelta=registry.readIntEntry("SETTINGS","dragdelta",dragDelta);
  wheelLines=registry.readIntEntry("SETTINGS","wheellines",wheelLines);

  // Colors; defaults are those values determined previously
  borderColor=registry.readColorEntry("SETTINGS","bordercolor",borderColor);
  baseColor=registry.readColorEntry("SETTINGS","basecolor",baseColor);
  hiliteColor=registry.readColorEntry("SETTINGS","hilitecolor",hiliteColor);
  shadowColor=registry.readColorEntry("SETTINGS","shadowcolor",shadowColor);
  backColor=registry.readColorEntry("SETTINGS","backcolor",backColor);
  foreColor=registry.readColorEntry("SETTINGS","forecolor",foreColor);
  selforeColor=registry.readColorEntry("SETTINGS","selforecolor",selforeColor);
  selbackColor=registry.readColorEntry("SETTINGS","selbackcolor",selbackColor);
  tipforeColor=registry.readColorEntry("SETTINGS","tipforecolor",tipforeColor);
  tipbackColor=registry.readColorEntry("SETTINGS","tipbackcolor",tipbackColor);

  // Maximum number of colors to allocate
  maxcolors=registry.readUnsignedEntry("SETTINGS","maxcolors",maxcolors);

  // Command line takes precedence
  if(maxcols) maxcolors=maxcols;

  // Set maximum number of colors in default visual to be nice to legacy
  // Motif applications which don't handle color allocation gracefully.
  getRootWindow()->getVisual()->setMaxColors(maxcolors);

  // Open display
  if(connect){
    if(!openDisplay(dpy)){
      fxwarning("%s::openDisplay: unable to open display %s\n",getClassName(),dpy);
      ::exit(1);
      }
    }
  }


// Exit application, but not the process
void FXApp::exit(FXint code){
  FXTRACE((100,"%s::exit\n",getClassName()));

  // Write the registry
  registry.write();

  // Exit the program
  stop(code);
  }


// Create application's windows
void FXApp::create(){
  FXTRACE((100,"%s::create\n",getClassName()));

  // Create visuals
  monoVisual->create();
  defaultVisual->create();

  // Create default font
  normalFont->create();
  stockFont->create();

  // Create cursors
  waitCursor->create();
  cursor[DEF_ARROW_CURSOR]->create();
  cursor[DEF_RARROW_CURSOR]->create();
  cursor[DEF_TEXT_CURSOR]->create();
  cursor[DEF_HSPLIT_CURSOR]->create();
  cursor[DEF_VSPLIT_CURSOR]->create();
  cursor[DEF_XSPLIT_CURSOR]->create();
  cursor[DEF_SWATCH_CURSOR]->create();
  cursor[DEF_MOVE_CURSOR]->create();
  cursor[DEF_DRAGH_CURSOR]->create();
  cursor[DEF_DRAGV_CURSOR]->create();
  cursor[DEF_DRAGTL_CURSOR]->create();
  cursor[DEF_DRAGTR_CURSOR]->create();
  cursor[DEF_DNDSTOP_CURSOR]->create();
  cursor[DEF_DNDCOPY_CURSOR]->create();
  cursor[DEF_DNDMOVE_CURSOR]->create();
  cursor[DEF_DNDLINK_CURSOR]->create();
  cursor[DEF_CROSSHAIR_CURSOR]->create();
  cursor[DEF_CORNERNE_CURSOR]->create();
  cursor[DEF_CORNERNW_CURSOR]->create();
  cursor[DEF_CORNERSE_CURSOR]->create();
  cursor[DEF_CORNERSW_CURSOR]->create();
  cursor[DEF_HELP_CURSOR]->create();
  cursor[DEF_HAND_CURSOR]->create();
  cursor[DEF_ROTATE_CURSOR]->create();

  // Create all windows
  root->create();
  }


// Detach application's windows
void FXApp::detach(){
  FXTRACE((100,"%s::detach\n",getClassName()));
  root->detach();

  // Detach default font
  normalFont->detach();
  stockFont->detach();

  // Detach cursors
  waitCursor->detach();
  cursor[DEF_ARROW_CURSOR]->detach();
  cursor[DEF_RARROW_CURSOR]->detach();
  cursor[DEF_TEXT_CURSOR]->detach();
  cursor[DEF_HSPLIT_CURSOR]->detach();
  cursor[DEF_VSPLIT_CURSOR]->detach();
  cursor[DEF_XSPLIT_CURSOR]->detach();
  cursor[DEF_SWATCH_CURSOR]->detach();
  cursor[DEF_MOVE_CURSOR]->detach();
  cursor[DEF_DRAGH_CURSOR]->detach();
  cursor[DEF_DRAGV_CURSOR]->detach();
  cursor[DEF_DRAGTL_CURSOR]->detach();
  cursor[DEF_DRAGTR_CURSOR]->detach();
  cursor[DEF_DNDSTOP_CURSOR]->detach();
  cursor[DEF_DNDCOPY_CURSOR]->detach();
  cursor[DEF_DNDMOVE_CURSOR]->detach();
  cursor[DEF_DNDLINK_CURSOR]->detach();
  cursor[DEF_CROSSHAIR_CURSOR]->detach();
  cursor[DEF_CORNERNE_CURSOR]->detach();
  cursor[DEF_CORNERNW_CURSOR]->detach();
  cursor[DEF_CORNERSE_CURSOR]->detach();
  cursor[DEF_CORNERSW_CURSOR]->detach();
  cursor[DEF_HELP_CURSOR]->detach();
  cursor[DEF_HAND_CURSOR]->detach();
  cursor[DEF_ROTATE_CURSOR]->detach();

  // Detach visuals
  monoVisual->detach();
  defaultVisual->detach();
  }


// Destroy application's windows
void FXApp::destroy(){
  FXTRACE((100,"%s::destroy\n",getClassName()));

  root->destroy();

  // Destroy default font
  normalFont->destroy();
  stockFont->destroy();

  // Destroy cursors
  waitCursor->destroy();
  cursor[DEF_ARROW_CURSOR]->destroy();
  cursor[DEF_RARROW_CURSOR]->destroy();
  cursor[DEF_TEXT_CURSOR]->destroy();
  cursor[DEF_HSPLIT_CURSOR]->destroy();
  cursor[DEF_VSPLIT_CURSOR]->destroy();
  cursor[DEF_XSPLIT_CURSOR]->destroy();
  cursor[DEF_SWATCH_CURSOR]->destroy();
  cursor[DEF_MOVE_CURSOR]->destroy();
  cursor[DEF_DRAGH_CURSOR]->destroy();
  cursor[DEF_DRAGV_CURSOR]->destroy();
  cursor[DEF_DRAGTL_CURSOR]->destroy();
  cursor[DEF_DRAGTR_CURSOR]->destroy();
  cursor[DEF_DNDSTOP_CURSOR]->destroy();
  cursor[DEF_DNDCOPY_CURSOR]->destroy();
  cursor[DEF_DNDMOVE_CURSOR]->destroy();
  cursor[DEF_DNDLINK_CURSOR]->destroy();
  cursor[DEF_CROSSHAIR_CURSOR]->destroy();
  cursor[DEF_CORNERNE_CURSOR]->destroy();
  cursor[DEF_CORNERNW_CURSOR]->destroy();
  cursor[DEF_CORNERSE_CURSOR]->destroy();
  cursor[DEF_CORNERSW_CURSOR]->destroy();
  cursor[DEF_HELP_CURSOR]->destroy();
  cursor[DEF_HAND_CURSOR]->destroy();
  cursor[DEF_ROTATE_CURSOR]->destroy();

  // Destroy visuals
  monoVisual->destroy();
  defaultVisual->destroy();
  }


#ifdef WIN32

// This window procedure is a static member function of class FXApp.
// Its sole purpose is to forward the message info on to FXApp::dispatchEvent().
long CALLBACK FXApp::wndproc(FXID hwnd,unsigned int iMsg,unsigned int wParam,long lParam){
  return app->dispatchEvent(hwnd,iMsg,wParam,lParam);
  }


#ifndef WM_SYNCPAINT
#define WM_SYNCPAINT 0x0088
#endif


// Generate SEL_LEAVE for windows wnd and its ancestors; note that the
// LEAVE events are generated in the order from child to parent
void FXApp::leaveWindow(FXWindow *win,FXWindow *anc){
  POINT pt;
  DWORD dwpts;
  if(!win || !win->getParent() || win==anc) return;
  event.type=SEL_LEAVE;
  dwpts=GetMessagePos();
  event.root_x=pt.x=((int)(short)LOWORD(dwpts));
  event.root_y=pt.y=((int)(short)HIWORD(dwpts));
  ScreenToClient((HWND)win->id(),&pt);
  event.win_x=pt.x;
  event.win_y=pt.y;
  win->handle(this,FXSEL(SEL_LEAVE,0),&event);
  leaveWindow(win->getParent(),anc);
  }


// Generate SEL_ENTER for windows and its ancestors; note that the
// ENTER events are generated in the order from parent to child
void FXApp::enterWindow(FXWindow *win,FXWindow *anc){
  POINT pt;
  DWORD dwpts;
  if(!win || !win->getParent() || win==anc) return;
  enterWindow(win->getParent(),anc);
  event.type=SEL_ENTER;
  dwpts=GetMessagePos();
  event.root_x=pt.x=((int)(short)LOWORD(dwpts));
  event.root_y=pt.y=((int)(short)HIWORD(dwpts));
  ScreenToClient((HWND)win->id(),&pt);
  event.win_x=pt.x;
  event.win_y=pt.y;
  win->handle(this,FXSEL(SEL_ENTER,0),&event);
  }


#define GETFOXWINDOW(hwnd) (((hwnd)&&IsWindow((HWND)(hwnd)))?(FXWindow*)GetWindowLong((HWND)(hwnd),0):NULL)

// Message dispatching
long FXApp::dispatchEvent(FXID hwnd,unsigned int iMsg,unsigned int wParam,long lParam){
  FXWindow *window,*oldwindow,*ancestor,*win;
  static HWND lastmovehwnd=0;
  static LPARAM lastmovelParam=0;
  static HWND oldhwnd=0;
  BOOL curinside,oldinside;
  TRACKMOUSEEVENT tme;
  POINT ptRoot, pt;
  DWORD dwpts;
  RECT rect;
  PAINTSTRUCT ps;
  FXuint state;
  HANDLE hMap;
  HANDLE answer;
  BYTE ks[256];
  UINT uScanCode;
  int n;
  char buf[10];

  if(!IsWindow((HWND)hwnd))
    return DefWindowProc((HWND)hwnd,iMsg,wParam,lParam);

  // Get window
  window=findWindowWithId(hwnd);

  if(window==0 && iMsg!=WM_CREATE)
    return DefWindowProc((HWND)hwnd,iMsg,wParam,lParam);

  // Translate Win32 message to FOX message type
  switch(iMsg){

    // Repaint event
    case WM_PAINT:
      // "Michael S. Harrison" <michaelh@inertiagames.com>:
      // An application should call the GetUpdateRect function to determine
      // whether the window has an update region. If GetUpdateRect returns zero,
      // the application should not call the BeginPaint and EndPaint functions.
      if(GetUpdateRect((HWND)hwnd,&rect,FALSE)){
        event.type=SEL_PAINT;
        event.synthetic=1;              // FIXME when is it non-synthetic?
        BeginPaint((HWND)hwnd,&ps);
        event.rect.x=(FXshort)rect.left;
        event.rect.y=(FXshort)rect.top;
        event.rect.w=(FXshort)(rect.right-rect.left);
        event.rect.h=(FXshort)(rect.bottom-rect.top);
        window->handle(this,FXSEL(SEL_PAINT,0),&event);
        EndPaint((HWND)hwnd,&ps);
        }
/*
      // Contributed by Daniel Gehriger <gehriger@linkcad.com>
      hRgn=CreateRectRgn(0,0,0,0);
      if(hRgn==NULL) return 0;
      switch(GetUpdateRgn((HWND)hwnd,hRgn,FALSE)){
        case SIMPLEREGION:
          GetRgnBox(hRgn,&rect);
          event.type=SEL_PAINT;
          event.synthetic=1;
          BeginPaint((HWND)hwnd,&ps);
          event.rect.h=(FXshort)(rect.bottom-rect.top);
          window->handle(this,FXSEL(SEL_PAINT,0),&event);
          EndPaint((HWND)hwnd,&ps);
          break;
        case COMPLEXREGION:
          dwCount=GetRegionData(hRgn,0,NULL);
          pRgnData=NULL;
          FXMALLOC(&pRgnData,BYTE,dwCount);
          if(pRgnData && GetRegionData(hRgn,dwCount,pRgnData)==dwCount){
            event.type=SEL_PAINT;
            event.synthetic=1;
            BeginPaint((HWND)hwnd,&ps);
            for(DWORD i=0; i<pRgnData->rdh.nCount; ++i){
              LPRECT pRect=(LPRECT)(pRgnData->Buffer)+i;
              event.rect.x=(FXshort)pRect->left;
              event.rect.y=(FXshort)pRect->top;
              event.rect.w=(FXshort)(pRect->right-pRect->left);
              event.rect.h=(FXshort)(pRect->bottom-pRect->top);
              window->handle(this,FXSEL(SEL_PAINT,0),&event);
              }
            EndPaint((HWND)hwnd,&ps);
            }
          FXFREE(&pRgnData);
          break;
        default: // NULLREGION or ERROR
          break;
        }
      DeleteObject(hRgn);
*/
      return 0;

    // Keyboard
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
      FXTRACE((100,"%s key= %c [%04x] \n",iMsg==WM_KEYDOWN?"WM_KEYDOWN":iMsg==WM_KEYUP?"WM_KEYUP":iMsg==WM_SYSKEYDOWN?"WM_SYSKEYDOWN":"WM_SYSKEYUP",wParam,wParam));
      event.type=((iMsg==WM_KEYUP)||(iMsg==WM_SYSKEYUP)) ? SEL_KEYRELEASE : SEL_KEYPRESS;
      event.time=GetMessageTime();
      dwpts=GetMessagePos();
      event.root_x=pt.x=((int)(short)LOWORD(dwpts));
      event.root_y=pt.y=((int)(short)HIWORD(dwpts));
      ScreenToClient((HWND)hwnd,&pt);
      event.win_x=pt.x;
      event.win_y=pt.y;
      event.state=fxmodifierkeys();

      // Translate to keysym
      event.code=wkbMapKeyCode((HWND)hwnd,iMsg,wParam,lParam);// FIXME not all codes match with those of X11

      // Translate to string on KeyPress
      uScanCode=HIWORD(lParam)&(KF_EXTENDED|KF_UP|0xff);
      GetKeyboardState(ks);
      //n=ToAscii(wParam,uScanCode,ks,(LPWORD)buf,0);
      n=ToAsciiEx(wParam,uScanCode,ks,(LPWORD)buf,0,GetKeyboardLayout(0));

      if(event.type==SEL_KEYPRESS){

        // Replace text string
        event.text.replace(0,1000,buf,n);
        }

      // Clear string on KeyRelease
      else{
        event.text.clear();
        }

      FXTRACE((100,"%s code=%04x state=%04x text=\"%s\"\n",(event.type==SEL_KEYPRESS)?"SEL_KEYPRESS":"SEL_KEYRELEASE",event.code,event.state,event.text.text()));

      // Dispatch to proper target
      if(keyboardGrabWindow){
        if(keyboardGrabWindow->handle(this,FXSEL(event.type,0),&event)) refresh();
        }
      else{
        if(event.type==SEL_KEYPRESS) keyWindow=focusWindow;
        if(keyWindow){
          // FIXME doesSaveUnder test should go away
          if(!invocation || invocation->modality==MODAL_FOR_NONE || (invocation->window && invocation->window->isOwnerOf(keyWindow)) || keyWindow->getShell()->doesSaveUnder()){
            if(keyWindow->handle(this,FXSEL(event.type,0),&event)) refresh();
            }
          else{
            if(event.type==SEL_KEYPRESS) beep();
            }
          }
        }
      return 0;

    // The grab might be broken; in FOX, we ignore this!!
    case WM_CANCELMODE:
      //FXTRACE((100,"WM_CANCELMODE\n"));
      return 0;

    // Capture changed
    case WM_CAPTURECHANGED:
      //FXTRACE((100,"WM_CAPTURECHANGED\n"));
      return 0;
/*
      event.time=GetMessageTime();
      // When the mouseGrabWindow looses the capture, generate a LEAVE event on it,
      // and determine if the window under the cursor location, if any, and
      // generate a ENTER event on it if it's one of our own windows.
      // Of course, start TrackMouseEvent again for this window!
      if(NULL==((HWND)lParam)){
        //FXTRACE((100,"capture lost\n"));
        oldhwnd=(HWND)hwnd;
        oldwindow=window;
        dwpts=GetMessagePos();
        pt.x=((int)(short)LOWORD(dwpts));
        pt.y=((int)(short)HIWORD(dwpts));
        hwnd=WindowFromPoint(pt);
        if(hwnd && (hwnd==oldhwnd || IsChild((HWND)window->getShell()->id(),(HWND)hwnd))){
          window=GETFOXWINDOW(hwnd);
          ancestor=FXWindow::commonAncestor(window,oldwindow);
          event.code=CROSSINGUNGRAB;
          leaveWindow(oldwindow,ancestor);
          enterWindow(window,ancestor);
          oldhwnd=(HWND)hwnd;
          }
        else{
          event.code=CROSSINGUNGRAB;
          leaveWindow(oldwindow,root);
          oldhwnd=0;
          }
        refresh();
        }
      // When the mouseGrabWindow gains the capture [we fake a WM_CAPTURECHANGED by
      // calling SetCapture twice], we need to generate LEAVE events on the
      // old window, if we had one.
      // Either way, we generate ENTER events on the window just captured.
      else if(hwnd==((HWND)lParam)){
        //FXTRACE((100,"capture gained\n"));
        if(oldhwnd && oldhwnd!=hwnd){
          oldwindow=GETFOXWINDOW(oldhwnd);
          ancestor=FXWindow::commonAncestor(window,oldwindow);
          event.code=CROSSINGGRAB;
          leaveWindow(oldwindow,ancestor);
          enterWindow(window,ancestor);
          refresh();
          }
        else{
          event.code=CROSSINGGRAB;
          enterWindow(window,root);
          refresh();
          }
        oldhwnd=(HWND)hwnd;
        }
      // When the capture is transferred between two windows, we generate a LEAVE
      // on the old window and an ENTER on the new one; the latter is actually done
      // in the previous branch, because FOX calls SetCapture twice!
      else{
        //FXTRACE((100,"capture transferred\n"));
        oldhwnd=(HWND)hwnd;
        oldwindow=window;
        event.code=CROSSINGGRAB;
        leaveWindow(oldwindow,root);// We shouldn't leave all the way...
        oldhwnd=0;
        refresh();
        }
      return 0;
*/

    // TrackMouseEvent
    case WM_MOUSELEAVE:
      //FXTRACE((100,"WM_MOUSELEAVE hwnd=%d x=%d y=%d \n",hwnd,event.root_x,event.root_y));
      // If we're still in a window, determine if the cursor is in some
      // other inferior window of this window's shell.  If not, that means
      // we left the shell and generate one final LEAVE event.
      // We do not generate LEAVE events here when moving between inferiors
      // because these WM_MOUSELEAVE events are generated out of sequence,
      // i.e. we will have received an WM_MOUSEMOVE on the new window prior
      // to receiving a WM_MOUSELEAVE on the old window, which is bad!
      if(oldhwnd){
        dwpts=GetMessagePos();
        pt.x=((int)(short)LOWORD(dwpts));
        pt.y=((int)(short)HIWORD(dwpts));
        hwnd=WindowFromPoint(pt);
        if(!hwnd || (window->getShell()->id()!=hwnd && !IsChild((HWND)window->getShell()->id(),(HWND)hwnd))){
          //FXTRACE((100,"mouse leave %08x\n",window));
          event.time=GetMessageTime();
          event.code=CROSSINGNORMAL;
          leaveWindow(window,root);
          oldhwnd=0;
          refresh();
          }
        }
      return 0;

    // Motion
    case WM_MOUSEMOVE:
      event.time=GetMessageTime();
      pt.x=ptRoot.x=(int)((short)LOWORD(lParam));
      pt.y=ptRoot.y=(int)((short)HIWORD(lParam));
      ClientToScreen((HWND)hwnd,&ptRoot);
      event.root_x=ptRoot.x;
      event.root_y=ptRoot.y;
      event.state=fxmodifierkeys();

      //FXTRACE((100,"WM_MOUSEMOVE hwnd=%d x=%d y=%d \n",hwnd,event.root_x,event.root_y));

      // Set moved flag
      if((FXABS(event.root_x-event.rootclick_x)>=dragDelta) || (FXABS(event.root_y-event.rootclick_y)>=dragDelta)) event.moved=1;

      // Was grabbed
      if(mouseGrabWindow){

        // Translate to grab window's coordinate system
        window->translateCoordinatesTo(event.win_x,event.win_y,mouseGrabWindow,pt.x,pt.y);

        // Moved out of/into rectangle of grabbed window
        GetClientRect((HWND)mouseGrabWindow->id(),&rect);
        curinside=(0<=event.win_x && event.win_x<rect.right && 0<=event.win_y && event.win_y<rect.bottom);
        oldinside=(0<=event.last_x && event.last_x<rect.right && 0<=event.last_y && event.last_y<rect.bottom);

        // Crossed window boundary
        if(curinside!=oldinside){
          if(curinside){
            event.type=SEL_ENTER;
            event.code=CROSSINGNORMAL;
            if(mouseGrabWindow->handle(this,FXSEL(SEL_ENTER,0),&event)) refresh();
            }
          else{
            event.type=SEL_LEAVE;
            event.code=CROSSINGNORMAL;
            if(mouseGrabWindow->handle(this,FXSEL(SEL_LEAVE,0),&event)) refresh();
            }
          }
        }

      // Not grabbed
      else{
        if(hwnd!=oldhwnd){
          if(oldhwnd){
            oldwindow=findWindowWithId(oldhwnd);
            ancestor=FXWindow::commonAncestor(window,oldwindow);
            event.code=CROSSINGNORMAL;
            leaveWindow(oldwindow,ancestor);
            enterWindow(window,ancestor);
            }
          else{
            enterWindow(window,root);
            }
          refresh();
          }
        }

      // Suppress spurious `tickling' motion events
      if(hwnd==lastmovehwnd && lParam==lastmovelParam) return 0;

      // Was still grabbed, but possibly new grab window!
      if(mouseGrabWindow){

        // Translate to grab window's coordinate system
        window->translateCoordinatesTo(event.win_x,event.win_y,mouseGrabWindow,pt.x,pt.y);

        // Set event data
        event.type=SEL_MOTION;
        event.code=0;

        // Dispatch to grab-window
        if(mouseGrabWindow->handle(this,FXSEL(SEL_MOTION,0),&event)) refresh();
        }

// FIXME Does window still exist?

      // Was not grabbed
      else if(!invocation || invocation->modality==MODAL_FOR_NONE || (invocation->window && invocation->window->isOwnerOf(window)) || window->getShell()->doesSaveUnder()){ // FIXME doesSaveUnder test should go away

        // Set event data
        event.type=SEL_MOTION;
        event.code=0;
        event.win_x=pt.x;
        event.win_y=pt.y;

        // Dispatch to window under cursor
        if(window->handle(this,FXSEL(SEL_MOTION,0),&event)) refresh();
        }

      // Update most recent mouse position
      event.last_x=pt.x;
      event.last_y=pt.y;

      // Set TrackMouseEvent on each window we enter, so we'll be notified when
      // we depart this window, because we will not know when we get the last
      // move event!
      if(oldhwnd!=hwnd){
        tme.cbSize=sizeof(TRACKMOUSEEVENT);
        tme.dwFlags=TME_LEAVE;
        tme.hwndTrack=(HWND)hwnd;
        tme.dwHoverTime=HOVER_DEFAULT;
#if defined(__IBMCPP__) ||  defined(__MINGW32__) || defined(__BORLANDC__) || defined(__SC__)
        TrackMouseEvent(&tme);
#else
        _TrackMouseEvent(&tme);
#endif
        oldhwnd=(HWND)hwnd;
        }

      // Remember this for tickling test
      lastmovehwnd=(HWND)hwnd;
      lastmovelParam=lParam;

      return 0;

    // Button
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
      event.time=GetMessageTime();
      event.win_x=pt.x=(int)((short)LOWORD(lParam));
      event.win_y=pt.y=(int)((short)HIWORD(lParam));
      ClientToScreen((HWND)hwnd,&pt);
      event.root_x=pt.x;
      event.root_y=pt.y;
      event.state=fxmodifierkeys();   // Get the state of the modifier keys and mouse buttons
      if(iMsg==WM_LBUTTONDOWN || iMsg==WM_MBUTTONDOWN || iMsg==WM_RBUTTONDOWN){
        if(iMsg==WM_LBUTTONDOWN){ event.type=SEL_LEFTBUTTONPRESS; event.code=LEFTBUTTON; }
        if(iMsg==WM_MBUTTONDOWN){ event.type=SEL_MIDDLEBUTTONPRESS; event.code=MIDDLEBUTTON; }
        if(iMsg==WM_RBUTTONDOWN){ event.type=SEL_RIGHTBUTTONPRESS; event.code=RIGHTBUTTON; }
        if(!event.moved && (event.time-event.click_time<clickSpeed) && (event.code==event.click_button)){
          event.click_count++;
          event.click_time=event.time;
          }
        else{
          event.click_count=1;
          event.click_x=event.win_x;
          event.click_y=event.win_y;
          event.rootclick_x=event.root_x;
          event.rootclick_y=event.root_y;
          event.click_button=event.code;
          event.click_time=event.time;
          }
        state=event.state&(LEFTBUTTONMASK|MIDDLEBUTTONMASK|RIGHTBUTTONMASK);
        if((state==LEFTBUTTONMASK)||(state==MIDDLEBUTTONMASK)||(state==RIGHTBUTTONMASK)) event.moved=0;
        }
      else{
        if(iMsg==WM_LBUTTONUP){ event.type=SEL_LEFTBUTTONRELEASE; event.code=LEFTBUTTON; }
        if(iMsg==WM_MBUTTONUP){ event.type=SEL_MIDDLEBUTTONRELEASE; event.code=MIDDLEBUTTON; }
        if(iMsg==WM_RBUTTONUP){ event.type=SEL_RIGHTBUTTONRELEASE; event.code=RIGHTBUTTON; }
        }
      if(mouseGrabWindow){
        window->translateCoordinatesTo(event.win_x,event.win_y,mouseGrabWindow,event.win_x,event.win_y);
        if(mouseGrabWindow->handle(this,FXSEL(event.type,0),&event)) refresh();
        }
      // FIXME doesSaveUnder test should go away
      else if(!invocation || invocation->modality==MODAL_FOR_NONE || (invocation->window && invocation->window->isOwnerOf(window)) || window->getShell()->doesSaveUnder()){
        if(window->handle(this,FXSEL(event.type,0),&event)) refresh();
        }
      else if(iMsg==WM_LBUTTONDOWN || iMsg==WM_MBUTTONDOWN || iMsg==WM_RBUTTONDOWN){
        beep();
        }
      event.last_x=event.win_x;
      event.last_y=event.win_y;
      return 0;

    // Mouse wheel
    case WM_MOUSEWHEEL:
      event.type=SEL_MOUSEWHEEL;
      event.time=GetMessageTime();
      event.root_x=pt.x=(int)((short)LOWORD(lParam));
      event.root_y=pt.y=(int)((short)HIWORD(lParam));
      event.code=(int)((short)HIWORD(wParam));
      event.state=fxmodifierkeys();   // Get the state of the modifier keys and mouse buttons
      window=findWindowAt(event.root_x,event.root_y);
      if(window){
        ScreenToClient((HWND)window->id(),&pt);
        event.win_x=pt.x;
        event.win_y=pt.y;
/*
        do{
          if(window->handle(this,FXSEL(SEL_MOUSEWHEEL,0),&event)){ refresh(); break; }
          window=window->getParent();
          }
        while(window);
*/
        if(mouseGrabWindow){
          window->translateCoordinatesTo(event.win_x,event.win_y,mouseGrabWindow,event.win_x,event.win_y);
          if(mouseGrabWindow->handle(this,FXSEL(SEL_MOUSEWHEEL,0),&event)) refresh();
          }
        // FIXME doesSaveUnder test should go away
        else if(!invocation || invocation->modality==MODAL_FOR_NONE || (invocation->window && invocation->window->isOwnerOf(window)) || window->getShell()->doesSaveUnder()){
          if(window->handle(this,FXSEL(SEL_MOUSEWHEEL,0),&event)) refresh();
          }
        }
      return 0;

    // Focus
    case WM_SETFOCUS:
      SendMessage((HWND)window,WM_NCACTIVATE,1,123456); // Suggestion from: Frank De prins <fdp@MCS.BE>
    case WM_KILLFOCUS:
      window=window->getShell();
      if(iMsg==WM_KILLFOCUS && focusWindow==window){
        event.type=SEL_FOCUSOUT;
        if(window->handle(this,FXSEL(SEL_FOCUSOUT,0),&event)) refresh();
        focusWindow=NULL;
        }
      if(iMsg==WM_SETFOCUS && focusWindow!=window){
        event.type=SEL_FOCUSIN;
        if(window->handle(this,FXSEL(SEL_FOCUSIN,0),&event)) refresh();
        focusWindow=window;
        }
      return 0;

    // Map or Unmap
    case WM_SHOWWINDOW:
      if(wParam){
        event.type=SEL_MAP;
        if(window->handle(this,FXSEL(SEL_MAP,0),&event)) refresh();
        }
      else{
        event.type=SEL_UNMAP;
        if(window->handle(this,FXSEL(SEL_UNMAP,0),&event)) refresh();
        }
      return DefWindowProc((HWND)hwnd,iMsg,wParam,lParam);

    // Create
    case WM_CREATE:
      event.type=SEL_CREATE;
      window=(FXWindow*)(((LPCREATESTRUCT)lParam)->lpCreateParams);
      hash.insert((void*)hwnd,window);
      if(window->handle(this,FXSEL(SEL_CREATE,0),&event)) refresh();
      return 0;

    // Close
    case WM_CLOSE:

      ////// Semantics: SEL_CLOSE is a suggestion that the window be closed;
      ////// SEL_DESTROY is a notify that destruction has already taken place.
      ////// Thus, a toplevel window gets a close, and MAY decide not to be closed;
      ////// If it thinks its OK to close, it gets a SEL_DESTROY also.
      event.type=SEL_CLOSE;
      if(!invocation || invocation->modality==MODAL_FOR_NONE || (invocation->window && invocation->window->isOwnerOf(window))){
        if(window->handle(this,FXSEL(SEL_CLOSE,0),&event)) refresh();
        }
      else{
        beep();
        }
      return 0;

    // Destroy
    case WM_DESTROY:
      event.type=SEL_DESTROY;
      if(window->handle(this,FXSEL(SEL_DESTROY,0),&event)) refresh();
      return 0;

    // Configure (size)
    case WM_SIZE:
      event.type=SEL_CONFIGURE;
      event.rect.x=window->getX();
      event.rect.y=window->getY();
      event.rect.w=LOWORD(lParam);
      event.rect.h=HIWORD(lParam);
      if(window->handle(this,FXSEL(SEL_CONFIGURE,0),&event)) refresh();
      return 0;

    // Configure (move)
    case WM_MOVE:
      event.type=SEL_CONFIGURE;
      event.rect.x=(short)LOWORD(lParam);
      event.rect.y=(short)HIWORD(lParam);
      event.rect.w=window->getWidth();
      event.rect.h=window->getHeight();
      if(window->handle(this,FXSEL(SEL_CONFIGURE,0),&event)) refresh();
      return 0;

    // Configure position and size
    case WM_WINDOWPOSCHANGED:
      return DefWindowProc((HWND)hwnd,iMsg,wParam,lParam);

    // Lost clipboard ownership
    case WM_DESTROYCLIPBOARD:
      if(clipboardWindow){
        event.time=GetMessageTime();
        event.type=SEL_CLIPBOARD_LOST;
        if(clipboardWindow->handle(this,FXSEL(SEL_CLIPBOARD_LOST,0),&event)) refresh();
        clipboardWindow=NULL;
        }
      return 0;

    // Safeguard it in the clipboard
    case WM_RENDERALLFORMATS:
      if(clipboardWindow){
        OpenClipboard((HWND)hwnd);
        EmptyClipboard();
        UINT iFormat=0;
        while((iFormat=EnumClipboardFormats(iFormat))!=0){
          event.type=SEL_CLIPBOARD_REQUEST;
          event.time=GetMessageTime();
          event.target=iFormat;
          if(clipboardWindow->handle(this,FXSEL(SEL_CLIPBOARD_REQUEST,0),&event)) refresh();
          FXTRACE((100,"Window %d being requested for CLIPBOARD DATA of type %d\n",hwnd,wParam));
          }
        CloseClipboard();
        }
      return 0;

    // We're asked to provide certain format to the clipboard
    case WM_RENDERFORMAT:
      if(clipboardWindow){
        event.type=SEL_CLIPBOARD_REQUEST;
        event.time=GetMessageTime();
        event.target=wParam;
        clipboardWindow->handle(this,FXSEL(SEL_CLIPBOARD_REQUEST,0),&event);
        FXTRACE((100,"Window %d being requested for CLIPBOARD DATA of type %d\n",hwnd,wParam));
        }
      return 0;

    // Change the cursor based on the window
    case WM_SETCURSOR:
      if(waitCount){
        SetCursor((HCURSOR)waitCursor->id());                     // Show wait cursor
        return 0;
        }
      if(!mouseGrabWindow && window->getDefaultCursor() && (LOWORD(lParam)==HTCLIENT)){
        SetCursor((HCURSOR)window->getDefaultCursor()->id());     // Show default cursor
        return 0;
        }
      return DefWindowProc((HWND)hwnd,iMsg,wParam,lParam);

    case WM_PALETTECHANGED:     // Suggested by Boris Kogan <bkogan@j51.com>
      if((HWND)wParam==hwnd) break;
    case WM_QUERYNEWPALETTE:
      if(window->getVisual()->colormap){
        HDC hdc=GetDC((HWND)hwnd);
        HPALETTE paletteold=SelectPalette(hdc,(HPALETTE)window->getVisual()->colormap,FALSE);
        UINT nchanged=RealizePalette(hdc);
        if(nchanged) InvalidateRect((HWND)hwnd,NULL,TRUE);
        SelectPalette(hdc,paletteold,TRUE);
        ReleaseDC((HWND)hwnd,hdc);
        return nchanged;
        }
      return 0;

    case WM_QUERYENDSESSION:    // Session will end if this app thinks its OK
      return 1;                 // Return 1 if OK to terminate

    case WM_ENDSESSION:         // Session will now end for sure
      exit(0);                  // Write registry and terminate
      return 0;

    case WM_STYLECHANGING:
    case WM_SIZING:
    case WM_MOVING:
    case WM_ERASEBKGND:         // Do nothing, erasing background causes flashing
      return 0;

    case WM_ACTIVATE:
      //FXTRACE((100,"WM_ACTIVATE %d\n",LOWORD(wParam)));
      if(window->isMemberOf(FXMETACLASS(FXTopWindow)) && focusWindow && focusWindow!=window){   // Suggestion from: Frank De prins <fdp@MCS.BE>
        SendMessage((HWND)focusWindow->id(),WM_NCACTIVATE,0,123456);
        }
      return DefWindowProc((HWND)hwnd,iMsg,wParam,lParam);

    case WM_GETMINMAXINFO:
      if(window->id() && window->shown() &&window->isMemberOf(FXMETACLASS(FXTopWindow))){
        RECT rect;
        //FXTRACE((100,"WM_GETMINMAXINFO ptMaxSize=%d,%d ptMinTrackSize=%d,%d ptMaxTrackSize=%d,%d\n",((MINMAXINFO*)lParam)->ptMaxSize.x,((MINMAXINFO*)lParam)->ptMaxSize.y,((MINMAXINFO*)lParam)->ptMinTrackSize.x,((MINMAXINFO*)lParam)->ptMinTrackSize.y,((MINMAXINFO*)lParam)->ptMaxTrackSize.x,((MINMAXINFO*)lParam)->ptMaxTrackSize.y));
        if(!(((FXTopWindow*)window)->getDecorations()&DECOR_SHRINKABLE)){
          SetRect(&rect,0,0,window->getDefaultWidth(),window->getDefaultHeight());
          AdjustWindowRectEx(&rect,GetWindowLong((HWND)hwnd,GWL_STYLE),FALSE,GetWindowLong((HWND)hwnd,GWL_EXSTYLE));
          ((MINMAXINFO*)lParam)->ptMinTrackSize.x=rect.right-rect.left;
          ((MINMAXINFO*)lParam)->ptMinTrackSize.y=rect.bottom-rect.top;
          }
        if(!(((FXTopWindow*)window)->getDecorations()&DECOR_STRETCHABLE)){
          SetRect(&rect,0,0,window->getDefaultWidth(),window->getDefaultHeight());
          AdjustWindowRectEx(&rect,GetWindowLong((HWND)hwnd,GWL_STYLE),FALSE,GetWindowLong((HWND)hwnd,GWL_EXSTYLE));
          ((MINMAXINFO*)lParam)->ptMaxTrackSize.x=rect.right-rect.left;
          ((MINMAXINFO*)lParam)->ptMaxTrackSize.y=rect.bottom-rect.top;
          }
        //FXTRACE((100,"width=%d height=%d\n",window->getWidth(),window->getHeight()));
        //FXTRACE((100,"WM_GETMINMAXINFO ptMaxSize=%d,%d ptMinTrackSize=%d,%d ptMaxTrackSize=%d,%d\n",((MINMAXINFO*)lParam)->ptMaxSize.x,((MINMAXINFO*)lParam)->ptMaxSize.y,((MINMAXINFO*)lParam)->ptMinTrackSize.x,((MINMAXINFO*)lParam)->ptMinTrackSize.y,((MINMAXINFO*)lParam)->ptMaxTrackSize.x,((MINMAXINFO*)lParam)->ptMaxTrackSize.y));
        }
      return 0;

    case WM_INITMENU:           // Patch from Robin Wilson <robin.wilson@abaqus.com>
    case WM_SYSCOMMAND:         // This pops down the menupane when clicking in non-client area
      for(win=root->getFirst(); win; win=win->getNext()){       // FIXME don't we already know popupWindow?
        if(win->shown() && win->isMemberOf(FXMETACLASS(FXMenuPane)) && window->containsChild(win->getOwner())) window->killFocus();
        }
      return DefWindowProc((HWND)hwnd,iMsg,wParam,lParam);
    case WM_ENTERMENULOOP:
    case WM_MENUCHAR:
    case WM_MENUSELECT:
    case WM_EXITMENULOOP:
    case WM_DISPLAYCHANGE:      // added by msh 2/DEC/99
    case WM_TIMER:              // added by msh 2/DEC/99
    case WM_ENTERIDLE:          // 24/NOV/99 md
    case WM_NCCREATE:
    case WM_NCDESTROY:
    case WM_ENTERSIZEMOVE:
    case WM_EXITSIZEMOVE:
    case WM_PARENTNOTIFY:
    case WM_SETTEXT:
    case WM_GETTEXT:
    case WM_QUERYOPEN:
    case WM_ENABLE:
    case WM_MOUSEACTIVATE:
    case WM_CHILDACTIVATE:
    case WM_SYNCPAINT:
    case WM_NCPAINT:
    case WM_NCHITTEST:
    case WM_NCMOUSEMOVE:
    case WM_NCCALCSIZE:
    case WM_NCLBUTTONDOWN:
    case WM_NCLBUTTONUP:
    case WM_WINDOWPOSCHANGING:  // Leave whatever placement is suggested
    case WM_STYLECHANGED:
      return DefWindowProc((HWND)hwnd,iMsg,wParam,lParam);

    case WM_NCACTIVATE:         // Suggestion from: Frank De prins <fdp@MCS.BE>
      if(lParam!=123456) wParam=1;
      return DefWindowProc((HWND)hwnd,iMsg,wParam,lParam);

    case WM_ACTIVATEAPP:        // Suggestion from: Frank De prins <fdp@MCS.BE>
      SendMessage((HWND)hwnd,WM_NCACTIVATE,wParam,123456);
      return DefWindowProc((HWND)hwnd,iMsg,wParam,lParam);

    case WM_DND_ENTER:
      FXTRACE((100,"DNDEnter from remote window %d\n",lParam));
      xdndSource=(FXID)lParam;
      if(ddeTypeList){FXFREE(&ddeTypeList);ddeNumTypes=0;}
      hMap=OpenFileMapping(FILE_MAP_READ,FALSE,"XdndTypeList");
      if(hMap){
        FXDragType *dragtypes=(FXDragType*)MapViewOfFile(hMap,FILE_MAP_READ,0,0,0);
        if(dragtypes){
          if(FXMALLOC(&ddeTypeList,FXDragType,dragtypes[0])){
            memcpy(ddeTypeList,&dragtypes[1],dragtypes[0]*sizeof(FXDragType));
            ddeNumTypes=dragtypes[0];
            }
          UnmapViewOfFile(dragtypes);
          }
        CloseHandle(hMap);
        }
      return 0;

    case WM_DND_LEAVE:
      FXTRACE((100,"DNDLeave from remote window %d\n",lParam));
      if(xdndSource!=(FXID)lParam) return 0;
      if(dropWindow){
        event.type=SEL_DND_LEAVE;
        if(dropWindow->handle(this,FXSEL(SEL_DND_LEAVE,0),&event)) refresh();
        dropWindow=NULL;
        }
      if(ddeTypeList){FXFREE(&ddeTypeList);ddeNumTypes=0;}
      xdndSource=0;
      return 0;

    case WM_DND_DROP:
      FXTRACE((100,"DNDDrop from remote window %d\n",lParam));
      if(xdndSource!=(FXID)lParam) return 0;
      if(dropWindow){
        event.type=SEL_DND_DROP;
        event.time=GetMessageTime();
        if(dropWindow->handle(this,FXSEL(SEL_DND_DROP,0),&event)) refresh();
        dropWindow=NULL;
        }
      PostMessage((HWND)xdndSource,WM_DND_FINISH,0,(LPARAM)hwnd);
      if(ddeTypeList){FXFREE(&ddeTypeList);ddeNumTypes=0;}
      xdndSource=0;
      return 0;

    case WM_DND_POSITION_REJECT:
    case WM_DND_POSITION_COPY:
    case WM_DND_POSITION_MOVE:
    case WM_DND_POSITION_LINK:
    case WM_DND_POSITION_PRIVATE:
      FXTRACE((100,"DNDPosition from remote window %d\n",lParam));
      if(xdndSource!=(FXID)lParam) return 0;
      event.time=GetMessageTime();
      event.root_x=(int)((short)LOWORD(wParam));
      event.root_y=(int)((short)HIWORD(wParam));
      win=findWindowAt(event.root_x,event.root_y);
      ddeAction=(FXDragAction)(iMsg-WM_DND_POSITION_REJECT);    // Action encoded in message
      ansAction=DRAG_REJECT;
      xdndRect.x=event.root_x;
      xdndRect.y=event.root_y;
      xdndRect.w=1;
      xdndRect.h=1;
      if(win!=dropWindow){
        if(dropWindow){
          event.type=SEL_DND_LEAVE;
          if(dropWindow->handle(this,FXSEL(SEL_DND_LEAVE,0),&event)) refresh();
          }
        dropWindow=NULL;
        if(win && win->isDropEnabled()){
          dropWindow=win;
          event.type=SEL_DND_ENTER;
          if(dropWindow->handle(this,FXSEL(SEL_DND_ENTER,0),&event)) refresh();
          }
        }
      if(dropWindow){
        pt.x=event.root_x;
        pt.y=event.root_y;
        ScreenToClient((HWND)dropWindow->id(),&pt);
        event.win_x=pt.x;
        event.win_y=pt.y;
        event.type=SEL_DND_MOTION;
        if(dropWindow->handle(this,FXSEL(SEL_DND_MOTION,0),&event)) refresh();
        }
      FXTRACE((100,"accepting %d\n",ansAction));
      PostMessage((HWND)xdndSource,WM_DND_STATUS_REJECT+ansAction,MAKELONG(xdndRect.x,xdndRect.y),MAKELONG(xdndRect.w,xdndRect.h));
      return 0;

    case WM_DND_STATUS_REJECT:
    case WM_DND_STATUS_COPY:
    case WM_DND_STATUS_MOVE:
    case WM_DND_STATUS_LINK:
    case WM_DND_STATUS_PRIVATE:
      ansAction=(FXDragAction)(iMsg-WM_DND_STATUS_REJECT);
      xdndRect.x=(int)((short)LOWORD(wParam));
      xdndRect.y=(int)((short)HIWORD(wParam));
      xdndRect.w=(int)((short)LOWORD(lParam));
      xdndRect.h=(int)((short)HIWORD(lParam));
      xdndStatusReceived=TRUE;
      xdndStatusPending=FALSE;
      FXTRACE((100,"DNDStatus from remote window action=%d rect x=%d y=%d w=%d h=%d\n",ansAction,xdndRect.x,xdndRect.y,xdndRect.w,xdndRect.h));
      return 0;

    case WM_DND_REQUEST:
      answer=0;
      if(dragWindow){
        event.type=SEL_DND_REQUEST;
        event.time=GetMessageTime();
        event.target=(FXDragType)wParam;
        ddeData=NULL;
        ddeSize=0;
        dragWindow->handle(this,FXSEL(SEL_DND_REQUEST,0),&event);
        FXTRACE((100,"Window %d being requested by window %d for XDND DATA of type %d; sending %d bytes\n",hwnd,lParam,wParam,ddeSize));
        answer=fxsenddata((HWND)lParam,ddeData,ddeSize);
        FXFREE(&ddeData);
        ddeData=NULL;
        ddeSize=0;
        }
      FXTRACE((100,"sending handle %d from window %d to %d\n",answer,hwnd,lParam));
      PostMessage((HWND)lParam,WM_DND_REPLY,(WPARAM)answer,(LPARAM)hwnd);
      return 0;
    }
  return DefWindowProc((HWND)hwnd,iMsg,wParam,lParam);
  }

#endif


// Handle quit
long FXApp::onCmdQuit(FXObject*,FXSelector,void*){
  exit(0);
  return 1;
  }


// Register DND type
FXDragType FXApp::registerDragType(const FXString& name) const {
  if(initialized){
#ifndef WIN32
    return (FXDragType)XInternAtom((Display*)display,name.text(),0);
#else
    return RegisterClipboardFormat(name.text());
#endif
    }
  return 0;
  }


// Get name of registered drag type
FXString FXApp::getDragTypeName(FXDragType type) const {
  if(initialized){
#ifndef WIN32
    FXchar *name=XGetAtomName((Display*)display,type);
    FXString dragtypename(name);
    XFree(name);
    return dragtypename;
#else
    if(0xC000<=type && type<=0xFFFF){
      char buffer[256];
      GetClipboardFormatName(type,buffer,sizeof(buffer));
      return buffer;
      }
    return "WIN32_DEFAULT_TYPE";
#endif
    }
  return FXString::null;
  }


/*******************************************************************************/

// Beep
void FXApp::beep(){
  if(initialized){
    FXTRACE((100,"Beep\n"));
#ifndef WIN32
    XBell((Display*)display,0);
#else
    MessageBeep(0);
#endif
    }
  }


// Dump widgets
long FXApp::onCmdDump(FXObject*,FXSelector,void*){
  dumpWidgets();
  return 1;
  }


// Dump widget information
void FXApp::dumpWidgets() const {
  const FXWindow *w=root;
  const FXObject *t;
  FXchar s;
  FXint lev=0;
  while(w){
    t=w->getTarget();
    s=w->shown()?'+':'-';
    if(t){
      fxmessage("%*c%s (%p): wk=%d id=%lu target=%s (%p) sel=%d x=%d y=%d w=%d h=%d\n",lev*2,s,w->getClassName(),w,w->getKey(),w->id(),t->getClassName(),t,w->getSelector(),w->getX(),w->getY(),w->getWidth(),w->getHeight());
      }
    else{
      fxmessage("%*c%s (%p): wk=%d id=%lu x=%d y=%d w=%d h=%d\n",lev*2,s,w->getClassName(),w,w->getKey(),w->id(),w->getX(),w->getY(),w->getWidth(),w->getHeight());
      }
    if(w->getFirst()){
      w=w->getFirst();
      lev++;
      continue;
      }
    while(!w->getNext() && w->getParent()){
      w=w->getParent();
      lev--;
      if(lev==1) fxmessage("\n");
      }
    w=w->getNext();
    }
  }


// Change default visual
void FXApp::setDefaultVisual(FXVisual* vis){
  if(!vis){ fxerror("%s::setDefaultVisual: NULL visual.\n",getClassName()); }
  defaultVisual=vis;
  }


// Change normal font
void FXApp::setNormalFont(FXFont* font){
  if(!font){ fxerror("%s::setNormalFont: NULL font.\n",getClassName()); }
  normalFont=font;
  }


// Begin of wait-cursor block; wait-cursor blocks may be nested.
void FXApp::beginWaitCursor(){
  if(initialized){
    if(waitCount==0){
      if(!waitCursor->id()){ fxerror("%s::beginWaitCursor: wait cursor not created yet.\n",getClassName()); }
#ifndef WIN32
      register FXWindow* child;
      FXASSERT(display);
      child=root->getFirst();
      while(child){
        if(child->id()){
          XDefineCursor((Display*)display,child->id(),waitCursor->id());
          if(child->getFirst()){child=child->getFirst();continue;}
          }
        while(!child->getNext()&&child->getParent()){child=child->getParent();}
        child=child->getNext();
        }
      XFlush((Display*)display);
#else
      SetCursor((HCURSOR)waitCursor->id());
#endif
      }
    waitCount++;
    }
  }


// End of wait-cursor block
void FXApp::endWaitCursor(){
  if(initialized){
    if(waitCount==0) return;
    waitCount--;
    if(waitCount==0){
      if(!waitCursor->id()){ fxerror("%s::endWaitCursor: wait cursor not created yet.\n",getClassName()); }
#ifndef WIN32
      register FXWindow* child;
      child=root->getFirst();
      while(child){
        if(child->id()){
          XDefineCursor((Display*)display,child->id(),child->getDefaultCursor()->id());
          if(child->getFirst()){child=child->getFirst();continue;}
          }
        while(!child->getNext()&&child->getParent()){child=child->getParent();}
        child=child->getNext();
        }
      XFlush((Display*)display);
#else
      if(cursorWindow){
        SetCursor((HCURSOR)cursorWindow->getDefaultCursor()->id());
        }
#endif
      }
    }
  }


// Change to a new wait cursor
void FXApp::setWaitCursor(FXCursor *cur){
  if(initialized){
    if(cur==NULL){ fxerror("%s::setWaitCursor: NULL wait cursor.\n",getClassName()); }
    if(waitCursor!=cur){
      waitCursor=cur;
      if(waitCount){
        if(!waitCursor->id()){ fxerror("%s::setWaitCursor: wait cursor not created yet.\n",getClassName()); }
#ifndef WIN32
        register FXWindow* child;
        child=root->getFirst();
        while(child){
          if(child->id()){
            XDefineCursor((Display*)display,child->id(),waitCursor->id());
            if(child->getFirst()){child=child->getFirst();continue;}
            }
          while(!child->getNext()&&child->getParent()){child=child->getParent();}
          child=child->getNext();
          }
        XFlush((Display*)display);
#else
        SetCursor((HCURSOR)waitCursor->id());
#endif
        }
      }
    }
  }


// Change default cursor
void FXApp::setDefaultCursor(FXDefaultCursor which,FXCursor* cur){
  if(!cur){ fxerror("%s::setDefaultCursor: NULL default cursor.\n",getClassName()); }
  cursor[which]=cur;
  }


// Save to stream
void FXApp::save(FXStream& store) const {
  FXObject::save(store);
  store << clickSpeed;
  store << animSpeed;
  store << scrollSpeed;
  store << blinkSpeed;
  store << menuPause;
  store << tooltipPause;
  store << tooltipTime;
  store << dragDelta;
  store << borderColor;
  store << baseColor;
  store << hiliteColor;
  store << shadowColor;
  store << backColor;
  store << foreColor;
  store << selforeColor;
  store << selbackColor;
  store << tipforeColor;
  store << tipbackColor;
  }


// Load from stream
void FXApp::load(FXStream& store){
  FXObject::load(store);
  store >> clickSpeed;
  store >> animSpeed;
  store >> scrollSpeed;
  store >> blinkSpeed;
  store >> menuPause;
  store >> tooltipPause;
  store >> tooltipTime;
  store >> dragDelta;
  store >> borderColor;
  store >> baseColor;
  store >> hiliteColor;
  store >> shadowColor;
  store >> backColor;
  store >> foreColor;
  store >> selforeColor;
  store >> selbackColor;
  store >> tipforeColor;
  store >> tipbackColor;
  }


// Change typing speed
void FXApp::setTypingSpeed(FXuint speed){
  typingSpeed=speed;
  registry.writeUnsignedEntry("SETTINGS","typingspeed",typingSpeed);
  }

// Change double-click speed
void FXApp::setClickSpeed(FXuint speed){
  clickSpeed=speed;
  registry.writeUnsignedEntry("SETTINGS","clickspeed",clickSpeed);
  }

// Change scroll speed
void FXApp::setScrollSpeed(FXuint speed){
  scrollSpeed=speed;
  registry.writeUnsignedEntry("SETTINGS","scrollspeed",scrollSpeed);
  }

// Change scroll delay
void FXApp::setScrollDelay(FXuint delay){
  scrollDelay=delay;
  registry.writeUnsignedEntry("SETTINGS","scrolldelay",scrollDelay);
  }

// Change cursor blink speed
void FXApp::setBlinkSpeed(FXuint speed){
  blinkSpeed=speed;
  registry.writeUnsignedEntry("SETTINGS","blinkspeed",blinkSpeed);
  }

// Change animation speed
void FXApp::setAnimSpeed(FXuint speed){
  animSpeed=speed;
  registry.writeUnsignedEntry("SETTINGS","animspeed",animSpeed);
  }

// Change menu popup delay
void FXApp::setMenuPause(FXuint pause){
  menuPause=pause;
  registry.writeUnsignedEntry("SETTINGS","menupause",menuPause);
  }

// Change tooltip popup pause
void FXApp::setTooltipPause(FXuint pause){
  tooltipPause=pause;
  registry.writeUnsignedEntry("SETTINGS","tippause",tooltipPause);
  }

// Change tooltip visibility time
void FXApp::setTooltipTime(FXuint time){
  tooltipTime=time;
  registry.writeUnsignedEntry("SETTINGS","tiptime",tooltipTime);
  }

// Change drag delta
void FXApp::setDragDelta(FXint delta){
  dragDelta=delta;
  registry.writeIntEntry("SETTINGS","dragdelta",dragDelta);
  }

// Change mouse wheel lines
void FXApp::setWheelLines(FXint lines){
  wheelLines=lines;
  registry.writeIntEntry("SETTINGS","wheellines",wheelLines);
  }

// Change border color
void FXApp::setBorderColor(FXColor color){
  borderColor=color;
  registry.writeColorEntry("SETTINGS","bordercolor",borderColor);
  }

// Change base color
void FXApp::setBaseColor(FXColor color){
  baseColor=color;
  registry.writeColorEntry("SETTINGS","basecolor",baseColor);
  }

// Change highlight color
void FXApp::setHiliteColor(FXColor color){
  hiliteColor=color;
  registry.writeColorEntry("SETTINGS","hilitecolor",hiliteColor);
  }

// Change shadow color
void FXApp::setShadowColor(FXColor color){
  shadowColor=color;
  registry.writeColorEntry("SETTINGS","shadowcolor",shadowColor);
  }

// Change background color
void FXApp::setBackColor(FXColor color){
  backColor=color;
  registry.writeColorEntry("SETTINGS","backcolor",backColor);
  }

// Change foreground color
void FXApp::setForeColor(FXColor color){
  foreColor=color;
  registry.writeColorEntry("SETTINGS","forecolor",foreColor);
  }

// Change selected foreground color
void FXApp::setSelforeColor(FXColor color){
  selforeColor=color;
  registry.writeColorEntry("SETTINGS","selforecolor",selforeColor);
  }

// Change selected background color
void FXApp::setSelbackColor(FXColor color){
  selbackColor=color;
  registry.writeColorEntry("SETTINGS","selbackcolor",selbackColor);
  }

// Change tip foreground color
void FXApp::setTipforeColor(FXColor color){
  tipforeColor=color;
  registry.writeColorEntry("SETTINGS","tipforecolor",tipforeColor);
  }

// Change tip background color
void FXApp::setTipbackColor(FXColor color){
  tipbackColor=color;
  registry.writeColorEntry("SETTINGS","tipbackcolor",tipbackColor);
  }


// Virtual destructor
FXApp::~FXApp(){
  register FXRepaint *r;
  register FXTimer *t;
  register FXChore *c;

  // Delete root window & its children
  delete root;

  // Delete visuals
  delete defaultVisual;
  delete monoVisual;

  // Delete stock font only
  delete stockFont;

  // Delete wait cursor
  delete waitCursor;

  // Delete cursors
  delete cursor[DEF_ARROW_CURSOR];
  delete cursor[DEF_RARROW_CURSOR];
  delete cursor[DEF_TEXT_CURSOR];
  delete cursor[DEF_HSPLIT_CURSOR];
  delete cursor[DEF_VSPLIT_CURSOR];
  delete cursor[DEF_XSPLIT_CURSOR];
  delete cursor[DEF_SWATCH_CURSOR];
  delete cursor[DEF_MOVE_CURSOR];
  delete cursor[DEF_DRAGH_CURSOR];
  delete cursor[DEF_DRAGV_CURSOR];
  delete cursor[DEF_DRAGTR_CURSOR];
  delete cursor[DEF_DRAGTL_CURSOR];
  delete cursor[DEF_DNDSTOP_CURSOR];
  delete cursor[DEF_DNDCOPY_CURSOR];
  delete cursor[DEF_DNDMOVE_CURSOR];
  delete cursor[DEF_DNDLINK_CURSOR];
  delete cursor[DEF_CROSSHAIR_CURSOR];
  delete cursor[DEF_CORNERNE_CURSOR];
  delete cursor[DEF_CORNERNW_CURSOR];
  delete cursor[DEF_CORNERSE_CURSOR];
  delete cursor[DEF_CORNERSW_CURSOR];
  delete cursor[DEF_HELP_CURSOR];
  delete cursor[DEF_HAND_CURSOR];
  delete cursor[DEF_ROTATE_CURSOR];

  // Free inputs
  FXFREE(&inputs);
#ifndef WIN32
  FXFREE(&r_fds);
  FXFREE(&w_fds);
  FXFREE(&e_fds);
#else
  FXFREE(&handles);
#endif

  // Free signals list
  FXFREE(&signals);
  nsignals=0;

  // Free left-over dde data
  FXFREE(&ddeData);
  ddeSize=0;

  // Free left-over selection type data
#ifndef WIN32
  FXFREE(&xselTypeList);
  FXFREE(&xcbTypeList);
  FXFREE(&xdndTypeList);
  FXFREE(&ddeTypeList);
#else
  FXFREE(&xselTypeList);
  FXFREE(&ddeTypeList);
#endif

  // Remove outstanding repaints
  while(repaints){
    r=repaints;
    repaints=repaints->next;
    delete r;
    }

  // Free recycled repaint records
  while(repaintrecs){
    r=repaintrecs;
    repaintrecs=repaintrecs->next;
    delete r;
    }

  // Kill outstanding timers
  while(timers){
    t=timers;
    timers=timers->next;
    delete t;
    }

  // Free recycled timer records
  while(timerrecs){
    t=timerrecs;
    timerrecs=timerrecs->next;
    delete t;
    }

  // Kill outstanding chores
  while(chores){
    c=chores;
    chores=chores->next;
    delete c;
    }

  // Free recycled chore records
  while(chorerecs){
    c=chorerecs;
    chorerecs=chorerecs->next;
    delete c;
    }

  // Close display
  closeDisplay();

  // Thrash dangling pointers
  root=(FXRootWindow*)-1L;
  defaultVisual=(FXVisual*)-1L;
  monoVisual=(FXVisual*)-1L;
  normalFont=(FXFont*)-1L;
  stockFont=(FXFont*)-1L;
  waitCursor=(FXCursor*)-1L;

  // Zap cursors
  cursor[DEF_ARROW_CURSOR]=(FXCursor*)-1L;
  cursor[DEF_RARROW_CURSOR]=(FXCursor*)-1L;
  cursor[DEF_TEXT_CURSOR]=(FXCursor*)-1L;
  cursor[DEF_HSPLIT_CURSOR]=(FXCursor*)-1L;
  cursor[DEF_VSPLIT_CURSOR]=(FXCursor*)-1L;
  cursor[DEF_XSPLIT_CURSOR]=(FXCursor*)-1L;
  cursor[DEF_SWATCH_CURSOR]=(FXCursor*)-1L;
  cursor[DEF_MOVE_CURSOR]=(FXCursor*)-1L;
  cursor[DEF_DRAGH_CURSOR]=(FXCursor*)-1L;
  cursor[DEF_DRAGV_CURSOR]=(FXCursor*)-1L;
  cursor[DEF_DRAGTL_CURSOR]=(FXCursor*)-1L;
  cursor[DEF_DRAGTR_CURSOR]=(FXCursor*)-1L;
  cursor[DEF_DNDSTOP_CURSOR]=(FXCursor*)-1L;
  cursor[DEF_DNDCOPY_CURSOR]=(FXCursor*)-1L;
  cursor[DEF_DNDMOVE_CURSOR]=(FXCursor*)-1L;
  cursor[DEF_DNDLINK_CURSOR]=(FXCursor*)-1L;
  cursor[DEF_CROSSHAIR_CURSOR]=(FXCursor*)-1L;
  cursor[DEF_CORNERNE_CURSOR]=(FXCursor*)-1L;
  cursor[DEF_CORNERNW_CURSOR]=(FXCursor*)-1L;
  cursor[DEF_CORNERSE_CURSOR]=(FXCursor*)-1L;
  cursor[DEF_CORNERSW_CURSOR]=(FXCursor*)-1L;
  cursor[DEF_HELP_CURSOR]=(FXCursor*)-1L;
  cursor[DEF_HAND_CURSOR]=(FXCursor*)-1L;
  cursor[DEF_ROTATE_CURSOR]=(FXCursor*)-1L;

  // Do this last
  app=NULL;
  }

}
