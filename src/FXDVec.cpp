/********************************************************************************
*                                                                               *
*                 D o u b l e - V e c t o r   O p e r a t i o n s               *
*                                                                               *
*********************************************************************************
* Copyright (C) 1994,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXDVec.cpp,v 1.5 2002/01/18 22:42:59 jeroen Exp $                        *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXObject.h"
#include "FXDVec.h"

FXDVec::FXDVec(FXColor color){
  v[0]=0.003921568627*FXREDVAL(color);
  v[1]=0.003921568627*FXGREENVAL(color);
  v[2]=0.003921568627*FXBLUEVAL(color);
  }


FXDVec& FXDVec::operator=(FXColor color){
  v[0]=0.003921568627*FXREDVAL(color);
  v[1]=0.003921568627*FXGREENVAL(color);
  v[2]=0.003921568627*FXBLUEVAL(color);
  return *this;
  }


FXDVec::operator FXColor() const {
  return FXRGB((v[0]*255.0),(v[1]*255.0),(v[2]*255.0));
  }


FXdouble len(const FXDVec& a){
  return sqrt(a.v[0]*a.v[0]+a.v[1]*a.v[1]+a.v[2]*a.v[2]);
  }


FXDVec normalize(const FXDVec& a){
  register double n=1.0/sqrt(a.v[0]*a.v[0]+a.v[1]*a.v[1]+a.v[2]*a.v[2]);
  return FXDVec(n*a.v[0],n*a.v[1],n*a.v[2]);
  }


FXDVec lo(const FXDVec& a,const FXDVec& b){
  return FXDVec(FXMIN(a.v[0],b.v[0]),FXMIN(a.v[1],b.v[1]),FXMIN(a.v[2],b.v[2]));
  }


FXDVec hi(const FXDVec& a,const FXDVec& b){
  return FXDVec(FXMAX(a.v[0],b.v[0]),FXMAX(a.v[1],b.v[1]),FXMAX(a.v[2],b.v[2]));
  }

FXStream& operator<<(FXStream& store,const FXDVec& v){
  store << v[0] << v[1] << v[2];
  return store;
  }

FXStream& operator>>(FXStream& store,FXDVec& v){
  store >> v[0] >> v[1] >> v[2];
  return store;
  }


