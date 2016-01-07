/********************************************************************************
*                                                                               *
*                               Test Splitter Window                            *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997,2016 by Jeroen van der Zijp.   All Rights Reserved.        *
********************************************************************************/
#include "fx.h"
#include <stdio.h>
#include <stdlib.h>

#include "FXFoldingList.h"

/*******************************************************************************/

/* Generated by reswrap from file minidoc1.gif */
const unsigned char minidoc1[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x10,0x00,0x10,0x00,0xf1,0x00,0x00,0xbf,0xbf,0xbf,
  0x00,0x00,0x00,0xff,0xff,0xff,0x7f,0x7f,0x7f,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,
  0x10,0x00,0x00,0x02,0x39,0x84,0x8f,0x89,0xc1,0x1d,0x7a,0x82,0x98,0x93,0x41,0x20,
  0x87,0x16,0xf2,0x29,0x49,0x71,0xcd,0x27,0x68,0x9b,0x16,0x0c,0x09,0x18,0x56,0xea,
  0x52,0x9a,0x5b,0xba,0xb6,0x14,0x0d,0xcb,0xf3,0x1b,0xd9,0x6e,0xad,0x1b,0x70,0x78,
  0x06,0x56,0x0b,0x17,0x71,0x28,0x89,0x86,0xa0,0xec,0x02,0x05,0x14,0x00,0x00,0x3b
  };

/* Generated by reswrap from file minifolderopen.gif */
const unsigned char minifolderopen[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x10,0x00,0x10,0x00,0xf2,0x00,0x00,0xb2,0xc0,0xdc,
  0x00,0x00,0x00,0x7f,0x7f,0x7f,0xff,0xff,0xff,0xd9,0xd9,0xd9,0xff,0xff,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,0x10,0x00,0x00,0x03,
  0x42,0x08,0xba,0xdc,0x2c,0x10,0xba,0x37,0x6a,0x15,0x13,0x88,0x41,0x4a,0x27,0x43,
  0x14,0x29,0x9b,0x67,0x82,0x56,0x18,0x68,0xdc,0xe9,0x12,0x42,0x20,0xce,0x62,0x11,
  0x6f,0x69,0x1e,0xc3,0x72,0xfb,0xb9,0xb2,0x18,0xeb,0x47,0xbc,0xad,0x4a,0xc4,0x93,
  0x6c,0xc5,0x7a,0x99,0x62,0x4c,0x1a,0x2d,0xc0,0x04,0x50,0xaf,0x58,0x6c,0x66,0xcb,
  0x6d,0x24,0x00,0x00,0x3b
  };

/* Generated by reswrap from file minifolderclosed.gif */
const unsigned char minifolderclosed[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x10,0x00,0x10,0x00,0xf2,0x00,0x00,0xb2,0xc0,0xdc,
  0x80,0x80,0x80,0xc0,0xc0,0xc0,0xff,0xff,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0x00,
  0x00,0x00,0x00,0x00,0x00,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,0x10,0x00,0x00,0x03,
  0x3b,0x08,0xba,0xdc,0x1b,0x10,0x3a,0x16,0xc4,0xb0,0x22,0x4c,0x50,0xaf,0xcf,0x91,
  0xc4,0x15,0x64,0x69,0x92,0x01,0x31,0x7e,0xac,0x95,0x8e,0x58,0x7b,0xbd,0x41,0x21,
  0xc7,0x74,0x11,0xef,0xb3,0x5a,0xdf,0x9e,0x1c,0x6f,0x97,0x03,0xba,0x7c,0xa1,0x64,
  0x48,0x05,0x20,0x38,0x9f,0x50,0xe8,0x66,0x4a,0x75,0x24,0x00,0x00,0x3b
  };

/* Generated by reswrap from file bigfolder.gif */
const unsigned char bigfolder[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x20,0x00,0x20,0x00,0xf2,0x00,0x00,0xb2,0xc0,0xdc,
  0x80,0x80,0x80,0xff,0xff,0xff,0xff,0xff,0x00,0xc0,0xc0,0xc0,0x00,0x00,0x00,0x80,
  0x80,0x00,0x00,0x00,0x00,0x2c,0x00,0x00,0x00,0x00,0x20,0x00,0x20,0x00,0x00,0x03,
  0x83,0x08,0xba,0xdc,0xfe,0x30,0xca,0x49,0x6b,0x0c,0x38,0x67,0x0b,0x83,0xf8,0x20,
  0x18,0x70,0x8d,0x37,0x10,0x67,0x8a,0x12,0x23,0x09,0x98,0xab,0xaa,0xb6,0x56,0x40,
  0xdc,0x78,0xae,0x6b,0x3c,0x5f,0xbc,0xa1,0xa0,0x70,0x38,0x2c,0x14,0x60,0xb2,0x98,
  0x32,0x99,0x34,0x1c,0x05,0xcb,0x28,0x53,0xea,0x44,0x4a,0xaf,0xd3,0x2a,0x74,0xca,
  0xc5,0x6a,0xbb,0xe0,0xa8,0x16,0x4b,0x66,0x7e,0xcb,0xe8,0xd3,0x38,0xcc,0x46,0x9d,
  0xdb,0xe1,0x75,0xba,0xfc,0x9e,0x77,0xe5,0x70,0xef,0x33,0x1f,0x7f,0xda,0xe9,0x7b,
  0x7f,0x77,0x7e,0x7c,0x7a,0x56,0x85,0x4d,0x84,0x82,0x54,0x81,0x88,0x62,0x47,0x06,
  0x91,0x92,0x93,0x94,0x95,0x96,0x91,0x3f,0x46,0x9a,0x9b,0x9c,0x9d,0x9e,0x9a,0x2e,
  0xa1,0xa2,0x13,0x09,0x00,0x3b
  };

/*******************************************************************************/


// Mini application object
class SplitterWindow : public FXMainWindow {
  FXDECLARE(SplitterWindow)
protected:
  FXMenuBar         *menubar;
  FXMenuPane        *filemenu;
  FXMenuPane        *modemenu;
  FXSplitter        *splitter;
  FXStatusBar       *status;
  FXVerticalFrame   *group1;
  FXVerticalFrame   *group2;
  FXVerticalFrame   *group3;
  FXGIFIcon         *folder_open;
  FXGIFIcon         *folder_closed;
  FXGIFIcon         *doc;
  FXGIFIcon         *bigdoc;

protected:
  SplitterWindow(){}

public:
  long onCmdReverse(FXObject*,FXSelector,void*);
  long onUpdReverse(FXObject*,FXSelector,void*);
  long onCmdNormal(FXObject*,FXSelector,void*);
  long onUpdNormal(FXObject*,FXSelector,void*);
  long onCmdHorizontal(FXObject*,FXSelector,void*);
  long onUpdHorizontal(FXObject*,FXSelector,void*);
  long onCmdVectical(FXObject*,FXSelector,void*);
  long onUpdVectical(FXObject*,FXSelector,void*);
  long onCmdTracking(FXObject*,FXSelector,void*);
  long onUpdTracking(FXObject*,FXSelector,void*);

public:
  enum {
    ID_REVERSE=FXMainWindow::ID_LAST,
    ID_NORMAL,
    ID_HORIZONTAL,
    ID_VERTICAL,
    ID_TRACKING,
    ID_LAST
    };

public:
  SplitterWindow(FXApp* a);
  virtual void create();
  virtual ~SplitterWindow();
  };



/*******************************************************************************/

// Map
FXDEFMAP(SplitterWindow) SplitterWindowMap[]={
  FXMAPFUNC(SEL_COMMAND,  SplitterWindow::ID_REVERSE,   SplitterWindow::onCmdReverse),
  FXMAPFUNC(SEL_UPDATE,   SplitterWindow::ID_REVERSE,   SplitterWindow::onUpdReverse),
  FXMAPFUNC(SEL_COMMAND,  SplitterWindow::ID_NORMAL,    SplitterWindow::onCmdNormal),
  FXMAPFUNC(SEL_UPDATE,   SplitterWindow::ID_NORMAL,    SplitterWindow::onUpdNormal),
  FXMAPFUNC(SEL_COMMAND,  SplitterWindow::ID_HORIZONTAL,SplitterWindow::onCmdHorizontal),
  FXMAPFUNC(SEL_UPDATE,   SplitterWindow::ID_HORIZONTAL,SplitterWindow::onUpdHorizontal),
  FXMAPFUNC(SEL_COMMAND,  SplitterWindow::ID_VERTICAL,  SplitterWindow::onCmdVectical),
  FXMAPFUNC(SEL_UPDATE,   SplitterWindow::ID_VERTICAL,  SplitterWindow::onUpdVectical),
  FXMAPFUNC(SEL_COMMAND,  SplitterWindow::ID_TRACKING,  SplitterWindow::onCmdTracking),
  FXMAPFUNC(SEL_UPDATE,   SplitterWindow::ID_TRACKING,  SplitterWindow::onUpdTracking),
  };


// Object implementation
FXIMPLEMENT(SplitterWindow,FXMainWindow,SplitterWindowMap,ARRAYNUMBER(SplitterWindowMap))


// Make some windows
SplitterWindow::SplitterWindow(FXApp* a):FXMainWindow(a,"Splitter Test",NULL,NULL,DECOR_ALL,0,0,800,600){
  int i;

  // Folder open icon
  folder_open=new FXGIFIcon(getApp(),minifolderopen);

  // Folder closed icon
  folder_closed=new FXGIFIcon(getApp(),minifolderclosed);

  // Document icon
  doc=new FXGIFIcon(getApp(),minidoc1);

  bigdoc=new FXGIFIcon(getApp(),bigfolder);

  // Menu bar
  menubar=new FXMenuBar(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X);

  // Status bar
  status=new FXStatusBar(this,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|STATUSBAR_WITH_DRAGCORNER);

  // File menu
  filemenu=new FXMenuPane(this);
  new FXMenuCommand(filemenu,"Quit\tCtl-Q",NULL,getApp(),FXApp::ID_QUIT);
  new FXMenuTitle(menubar,"&File",NULL,filemenu);

  // Main window interior
  splitter=new FXSplitter(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y|SPLITTER_REVERSED|SPLITTER_TRACKING);
  group1=new FXVerticalFrame(splitter,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0);
  group2=new FXVerticalFrame(splitter,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FIX_WIDTH);
  group3=new FXVerticalFrame(splitter,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y);

  // Mode menu
  modemenu=new FXMenuPane(this);
  new FXMenuRadio(modemenu,"Reverse\t\tReverse split order",this,ID_REVERSE);
  new FXMenuRadio(modemenu,"Normal\t\tNormal split order",this,ID_NORMAL);
  new FXMenuRadio(modemenu,"Horizontal\t\tHorizontal split",this,ID_HORIZONTAL);
  new FXMenuRadio(modemenu,"Vertical\t\tVertical split",this,ID_VERTICAL);
  new FXMenuCheck(modemenu,"Tracking\t\tToggle continuous tracking mode",this,ID_TRACKING);
  new FXMenuCheck(modemenu,"Toggle pane 1",group1,FXWindow::ID_TOGGLESHOWN);
  new FXMenuCheck(modemenu,"Toggle pane 2",group2,FXWindow::ID_TOGGLESHOWN);
  new FXMenuCheck(modemenu,"Toggle pane 3",group3,FXWindow::ID_TOGGLESHOWN);

  new FXMenuTitle(menubar,"&Mode",NULL,modemenu);

  FXTreeList *tree=new FXTreeList(group1,NULL,0,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_TOP|LAYOUT_RIGHT|TREELIST_SHOWS_BOXES|TREELIST_SHOWS_LINES|TREELIST_EXTENDEDSELECT);
  FXTreeItem *branch,*twig,*leaf,*topmost;

  topmost=tree->appendItem(0,"Top",folder_open,folder_closed);
  tree->expandTree(topmost);
    tree->appendItem(topmost,"First",doc,doc);
    tree->appendItem(topmost,"Second",doc,doc);
    tree->appendItem(topmost,"Third",bigdoc,bigdoc);
    branch=tree->appendItem(topmost,"Fourth",folder_open,folder_closed);
    tree->expandTree(branch);
      tree->appendItem(branch,"Fourth-First",doc,doc);
      tree->appendItem(branch,"Fourth-Second",doc,doc);
      twig=tree->appendItem(branch,"Fourth-Third",folder_open,folder_closed);
        tree->appendItem(twig,"Fourth-Third-First",doc,doc);
        tree->appendItem(twig,"Fourth-Third-Second",doc,doc);
        tree->appendItem(twig,"Fourth-Third-Third",doc,doc);
        leaf=tree->appendItem(twig,"Fourth-Third-Fourth",folder_open,folder_closed);
        leaf->setEnabled(false);
          tree->appendItem(leaf,"Fourth-Third-Fourth-First",doc,doc);
          tree->appendItem(leaf,"Fourth-Third-Fourth-Second",doc,doc);
          tree->appendItem(leaf,"Fourth-Third-Fourth-Third",doc,doc);
      twig=tree->appendItem(branch,"Fourth-Fourth",folder_open,folder_closed);
        tree->appendItem(twig,"Fourth-Fourth-First",doc,doc);
        tree->appendItem(twig,"Fourth-Fourth-Second",doc,doc);
        tree->appendItem(twig,"Fourth-Fourth-Third",doc,doc);
        for(i=0; i<10; i++){
          tree->prependItem(twig,FXString::value(i),doc,doc);
          }
      twig=tree->appendItem(branch,"Fourth-Fifth",folder_open,folder_closed);
      tree->expandTree(twig);
        tree->appendItem(twig,"Fourth-Fifth-First",doc,doc);
        tree->appendItem(twig,"Fourth-Fifth-Second",doc,doc);
        tree->appendItem(twig,"Fourth-Fifth-Third",doc,doc);
        for(i=0; i<10; i++){
          tree->appendItem(twig,FXString::value(i),doc,doc);
          }
    tree->appendItem(topmost,"Fifth",doc,doc);
    tree->appendItem(topmost,"Sixth",doc,doc);
    branch=tree->appendItem(topmost,"Seventh",folder_open,folder_closed);
      tree->appendItem(branch,"Seventh-First",doc,doc);
      tree->appendItem(branch,"Seventh-Second",doc,doc);
      tree->appendItem(branch,"Seventh-Third",doc,doc);
    tree->appendItem(topmost,"Eighth",doc,doc);


  new FXLabel(group2,"Matrix",NULL,LAYOUT_CENTER_X);
  new FXHorizontalSeparator(group2,SEPARATOR_GROOVE|LAYOUT_FILL_X);
  FXMatrix* matrix=new FXMatrix(group2,2,MATRIX_BY_COLUMNS|LAYOUT_FILL_X);

  new FXLabel(matrix,"Alpha:",NULL,JUSTIFY_RIGHT|LAYOUT_FILL_X|LAYOUT_CENTER_Y);
  new FXTextField(matrix,2,NULL,0,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN);
  new FXLabel(matrix,"Beta:",NULL,JUSTIFY_RIGHT|LAYOUT_FILL_X|LAYOUT_CENTER_Y);
  new FXTextField(matrix,2,NULL,0,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN);
  new FXLabel(matrix,"Gamma:",NULL,JUSTIFY_RIGHT|LAYOUT_FILL_X|LAYOUT_CENTER_Y);
  new FXTextField(matrix,2,NULL,0,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN);

  new FXCheckButton(group2,"Continuous Tracking\tSplitter continuously tracks split changes",this,ID_TRACKING);

  new FXLabel(group3,"Quite a Stretch",NULL,LAYOUT_CENTER_X);
  new FXHorizontalSeparator(group3,SEPARATOR_GROOVE|LAYOUT_FILL_X);
  FXMatrix* mat=new FXMatrix(group3,3,LAYOUT_FILL_X|LAYOUT_FILL_Y);

  new FXButton(mat,"One\nStretch the row\nStretch in Y\nStretch in X\tThe possibilities are endless..",NULL,NULL,0,FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_ROW);
  new FXButton(mat,"Two\nStretch in X\tThe possibilities are endless..",NULL,NULL,0,FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_X);
  new FXButton(mat,"Three\nStretch the row\nStretch in Y\nStretch in X\tThe possibilities are endless..",NULL,NULL,0,FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_ROW);

  new FXButton(mat,"Four\nStretch the column\nStretch the row\nStretch in Y\nStretch in X\tThe possibilities are endless..",NULL,NULL,0,FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN);
  new FXButton(mat,"Five\nStretch the column\nStretch in Y\nStretch in X\tThe possibilities are endless..",NULL,NULL,0,FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_COLUMN);
  new FXButton(mat,"Six\nStretch the column\nStretch the row\nStretch in Y\nStretch in X\tThe possibilities are endless..",NULL,NULL,0,FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);

  new FXButton(mat,"Seven\nStretch the column\nStretch the row\nCenter in Y\nCenter in X\tThe possibilities are endless..",NULL,NULL,0,FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_CENTER_X|LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN);
  new FXButton(mat,"Eight\nStretch the column\tThe possibilities are endless..",NULL,NULL,0,FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_COLUMN);
  new FXButton(mat,"Nine\nStretch the column\nStretch the row\nStretch in Y\tThe possibilities are endless..",NULL,NULL,0,FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT|LAYOUT_FILL_Y|LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN);

  // Make a tool tip
  new FXToolTip(getApp(),0);
  }


// Clean up
SplitterWindow::~SplitterWindow(){
  delete filemenu;
  delete modemenu;
  delete folder_open;
  delete folder_closed;
  delete doc;
  delete bigdoc;
  }


long SplitterWindow::onCmdReverse(FXObject*,FXSelector,void*){
  splitter->setSplitterStyle(splitter->getSplitterStyle()|SPLITTER_REVERSED);
  return 1;
  }

long SplitterWindow::onUpdReverse(FXObject* sender,FXSelector,void*){
  sender->handle(this,(splitter->getSplitterStyle()&SPLITTER_REVERSED)?FXSEL(SEL_COMMAND,ID_CHECK):FXSEL(SEL_COMMAND,ID_UNCHECK),NULL);
  return 1;
  }

long SplitterWindow::onCmdNormal(FXObject*,FXSelector,void*){
  splitter->setSplitterStyle(splitter->getSplitterStyle()&~SPLITTER_REVERSED);
  return 1;
  }

long SplitterWindow::onUpdNormal(FXObject* sender,FXSelector,void*){
  sender->handle(this,(splitter->getSplitterStyle()&SPLITTER_REVERSED)?FXSEL(SEL_COMMAND,ID_UNCHECK):FXSEL(SEL_COMMAND,ID_CHECK),NULL);
  return 1;
  }

long SplitterWindow::onCmdHorizontal(FXObject*,FXSelector,void*){
  splitter->setSplitterStyle(splitter->getSplitterStyle()&~SPLITTER_VERTICAL);
  return 1;
  }

long SplitterWindow::onUpdHorizontal(FXObject* sender,FXSelector,void*){
  sender->handle(this,(splitter->getSplitterStyle()&SPLITTER_VERTICAL)?FXSEL(SEL_COMMAND,ID_UNCHECK):FXSEL(SEL_COMMAND,ID_CHECK),NULL);
  return 1;
  }
long SplitterWindow::onCmdVectical(FXObject*,FXSelector,void*){
  splitter->setSplitterStyle(splitter->getSplitterStyle()|SPLITTER_VERTICAL);
  return 1;
  }

long SplitterWindow::onUpdVectical(FXObject* sender,FXSelector,void*){
  sender->handle(this,(splitter->getSplitterStyle()&SPLITTER_VERTICAL)?FXSEL(SEL_COMMAND,ID_CHECK):FXSEL(SEL_COMMAND,ID_UNCHECK),NULL);
  return 1;
  }

long SplitterWindow::onCmdTracking(FXObject*,FXSelector,void*){
  splitter->setSplitterStyle(splitter->getSplitterStyle()^SPLITTER_TRACKING);
  return 1;
  }

long SplitterWindow::onUpdTracking(FXObject* sender,FXSelector,void*){
  if(splitter->getSplitterStyle()&SPLITTER_TRACKING){
    sender->handle(this,FXSEL(SEL_COMMAND,ID_CHECK),NULL);
    }
  else{
    sender->handle(this,FXSEL(SEL_COMMAND,ID_UNCHECK),NULL);
    }
  return 1;
  }



// Start
void SplitterWindow::create(){
  FXMainWindow::create();
  show(PLACEMENT_SCREEN);
  }


/*******************************************************************************/


// Start the whole thing
int main(int argc,char *argv[]){
  FXApp  application("Splitter","FoxTest");
  application.init(argc,argv);
  SplitterWindow* mainwindow=new SplitterWindow(&application);
  application.create();
  mainwindow->show();
  return application.run();
  }


