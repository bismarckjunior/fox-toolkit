/********************************************************************************
*                                                                               *
*          S i n g l e - P r e c i s i o n   C o m p l e x   N u m b e r        *
*                                                                               *
*********************************************************************************
* Copyright (C) 2006,2014 by Jeroen van der Zijp.   All Rights Reserved.        *
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
#include "FXStream.h"
#include "FXObject.h"
#include "FXComplexf.h"


using namespace FX;


/*******************************************************************************/

namespace FX {


// Complex square root
FXComplexf csqrt(const FXComplexf& c){
  register FXfloat mag=c.modulus();
  register FXfloat rr=sqrt((mag+c.re)*0.5f);
  register FXfloat ii=sqrt((mag-c.re)*0.5f);
#if defined(WIN32)
  return FXComplexf(rr,c.im<0.0?-ii:ii);
#else
  return FXComplexf(rr,copysignf(ii,c.im));
#endif
  }


// Complex sine
FXComplexf csin(const FXComplexf& c){
  return FXComplexf(sinf(c.re)*coshf(c.im),cosf(c.re)*sinhf(c.im));
  }


// Complex cosine
FXComplexf ccos(const FXComplexf& c){
  return FXComplexf(cosf(c.re)*coshf(c.im),-sinf(c.re)*sinhf(c.im));
  }


// Complex tangent
FXComplexf ctan(const FXComplexf& c){
  FXComplexf em=exponent(FXComplexf(c.im,-c.re));
  FXComplexf ep=exponent(FXComplexf(-c.im,c.re));
  FXComplexf t=(em-ep)/(em+ep);
  return FXComplexf(-t.im,t.re);
  }


// Complex hyperbolic sine
FXComplexf csinh(const FXComplexf& c){
  return FXComplexf(cosf(c.im)*sinhf(c.re),sinf(c.im)*coshf(c.re));
  }


// Complex hyperbolic cosine
FXComplexf ccosh(const FXComplexf& c){
  return FXComplexf(cosf(c.im)*coshf(c.re),sinf(c.im)*sinhf(c.re));
  }


// Complex hyperbolic tangent
FXComplexf ctanh(const FXComplexf& c){
  return csinh(c)/ccosh(c);
  }


FXStream& operator<<(FXStream& store,const FXComplexf& c){
  store << c.re << c.im;
  return store;
  }


FXStream& operator>>(FXStream& store,FXComplexf& c){
  store >> c.re >> c.im;
  return store;
  }

}