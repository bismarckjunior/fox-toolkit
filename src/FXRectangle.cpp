/********************************************************************************
*                                                                               *
*                          R e c t a n g l e    C l a s s                       *
*                                                                               *
*********************************************************************************
* Copyright (C) 1994,2004 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXRectangle.cpp,v 1.11 2004/01/18 21:25:49 fox Exp $                     *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxpriv.h"
#include "FXStream.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"

using namespace FX;

/*******************************************************************************/

namespace FX {

// Fast inlines
static inline FXshort _max(FXshort a,FXshort b){ return a>b?a:b; }
static inline FXshort _min(FXshort a,FXshort b){ return a<b?a:b; }


// Grow by amount
FXRectangle& FXRectangle::grow(FXshort margin){
  x-=margin;
  y-=margin;
  w+=(margin+margin);
  h+=(margin+margin);
  return *this;
  }


FXRectangle& FXRectangle::grow(FXshort hormargin,FXshort vermargin){
  x-=hormargin;
  y-=vermargin;
  w+=(hormargin+hormargin);
  h+=(vermargin+vermargin);
  return *this;
  }


FXRectangle& FXRectangle::grow(FXshort leftmargin,FXshort rightmargin,FXshort topmargin,FXshort bottommargin){
  x-=leftmargin;
  y-=topmargin;
  w+=(leftmargin+rightmargin);
  h+=(topmargin+bottommargin);
  return *this;
  }


// Shrink by amount
FXRectangle& FXRectangle::shrink(FXshort margin){
  x+=margin;
  y+=margin;
  w-=(margin+margin);
  h-=(margin+margin);
  return *this;
  }


FXRectangle& FXRectangle::shrink(FXshort hormargin,FXshort vermargin){
  x+=hormargin;
  y+=vermargin;
  w-=(hormargin+hormargin);
  h-=(vermargin+vermargin);
  return *this;
  }


FXRectangle& FXRectangle::shrink(FXshort leftmargin,FXshort rightmargin,FXshort topmargin,FXshort bottommargin){
  x+=leftmargin;
  y+=topmargin;
  w-=(leftmargin+rightmargin);
  h-=(topmargin+bottommargin);
  return *this;
  }


// Union with rectangle
FXRectangle& FXRectangle::operator+=(const FXRectangle &r){
  w=_max(x+w,r.x+r.w); x=_min(x,r.x); w-=x;
  h=_max(y+h,r.y+r.h); y=_min(y,r.y); h-=y;
  return *this;
  }


// Intersection with rectangle
FXRectangle& FXRectangle::operator*=(const FXRectangle &r){
  w=_min(x+w,r.x+r.w); x=_max(x,r.x); w-=x;
  h=_min(y+h,r.y+r.h); y=_max(y,r.y); h-=y;
  return *this;
  }


// Union between rectangles
FXRectangle operator+(const FXRectangle& p,const FXRectangle& q){
  register FXshort xx=_min(p.x,q.x);
  register FXshort ww=_max(p.x+p.w,q.x+q.w)-xx;
  register FXshort yy=_min(p.y,q.y);
  register FXshort hh=_max(p.y+p.h,q.y+q.h)-yy;
  return FXRectangle(xx,yy,ww,hh);
  }


// Intersection between rectangles
FXRectangle operator*(const FXRectangle& p,const FXRectangle& q){
  register FXshort xx=_max(p.x,q.x);
  register FXshort ww=_min(p.x+p.w,q.x+q.w)-xx;
  register FXshort yy=_max(p.y,q.y);
  register FXshort hh=_min(p.y+p.h,q.y+q.h)-yy;
  return FXRectangle(xx,yy,ww,hh);
  }



// Save object to a stream
FXStream& operator<<(FXStream& store,const FXRectangle& r){
  store << r.x << r.y << r.w << r.h;
  return store;
  }


// Load object from a stream
FXStream& operator>>(FXStream& store,FXRectangle& r){
  store >> r.x >> r.y >> r.w >> r.h;
  return store;
  }

}
