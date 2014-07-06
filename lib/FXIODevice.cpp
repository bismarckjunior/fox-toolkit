/********************************************************************************
*                                                                               *
*                        I / O   D e v i c e   C l a s s                        *
*                                                                               *
*********************************************************************************
* Copyright (C) 2005,2014 by Jeroen van der Zijp.   All Rights Reserved.        *
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
#include "fxascii.h"
#include "FXArray.h"
#include "FXHash.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXIO.h"
#include "FXIODevice.h"



/*
  Notes:

  - An abstract class for low-level IO.
*/

// Bad handle value
#ifdef WIN32
#define BadHandle INVALID_HANDLE_VALUE
#else
#define BadHandle -1
#endif

using namespace FX;

/*******************************************************************************/

namespace FX {



// Construct
FXIODevice::FXIODevice():device(BadHandle){
  }


// Construct with given handle and mode
FXIODevice::FXIODevice(FXInputHandle h,FXuint m):device(BadHandle){
  attach(h,m);
  }


// Open device with access mode m and handle h
FXbool FXIODevice::open(FXInputHandle h,FXuint m){
  device=h;
  access=m;
  pointer=0L;
  return true;
  }


// Return true if open
FXbool FXIODevice::isOpen() const {
  return device!=BadHandle;
  }


// Return true if serial access only
FXbool FXIODevice::isSerial() const {
  return true;
  }


// Attach existing file handle
void FXIODevice::attach(FXInputHandle h,FXuint m){
  close();
  device=h;
  access=(m|OwnHandle);
  pointer=0L;
  }


// Detach existing file handle
void FXIODevice::detach(){
  device=BadHandle;
  access=NoAccess;
  pointer=0L;
  }


// Get position
FXlong FXIODevice::position() const {
  return pointer;
  }


// Move to position
FXlong FXIODevice::position(FXlong,FXuint){
  return -1;
  }


// Read block
FXival FXIODevice::readBlock(void* ptr,FXival count){
  FXival nread=-1;
  if(__likely(device!=BadHandle) && __likely(access&ReadOnly)){
#if defined(WIN32)
    DWORD nr;
    if(::ReadFile(device,ptr,(DWORD)count,&nr,NULL)!=0){
      nread=(FXival)nr;
      }
    pointer+=nread;
#else
    do{
      nread=::read(device,ptr,count);
      }
    while(nread<0 && errno==EINTR);
    pointer+=nread;
#endif
    }
  return nread;
  }


// Write block
FXival FXIODevice::writeBlock(const void* ptr,FXival count){
  FXival nwritten=-1;
  if(__likely(device!=BadHandle) && __likely(access&WriteOnly)){
#if defined(WIN32)
    DWORD nw;
    if(::WriteFile(device,ptr,(DWORD)count,&nw,NULL)!=0){
      nwritten=(FXival)nw;
      }
    pointer+=nwritten;
#else
    do{
      nwritten=::write(device,ptr,count);
      }
    while(nwritten<0 && errno==EINTR);
    pointer+=nwritten;
#endif
    }
  return nwritten;
  }


// Truncate file
FXlong FXIODevice::truncate(FXlong){
  return -1;
  }


// Synchronize disk with cached data
FXbool FXIODevice::flush(){
  return false;
  }


// Test if we're at the end; -1 if error
FXint FXIODevice::eof(){
  return -1;
  }


// Return file size
FXlong FXIODevice::size(){
  return -1;
  }


// Close file
FXbool FXIODevice::close(){
  if(__likely(device!=BadHandle)){
    if(access&OwnHandle){
#if defined(WIN32)
      if(::CloseHandle(device)!=0){
        device=BadHandle;
        access=NoAccess;
        pointer=0L;
        return true;
        }
#else
      if(::close(device)==0){
        device=BadHandle;
        access=NoAccess;
        pointer=0L;
        return true;
        }
#endif
      }
    device=BadHandle;
    access=NoAccess;
    pointer=0L;
    }
  return false;
  }


// Destroy
FXIODevice::~FXIODevice(){
  close();
  }


}
