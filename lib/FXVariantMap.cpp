/********************************************************************************
*                                                                               *
*                              V a r i a n t - M a p                            *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998,2013 by Jeroen van der Zijp.   All Rights Reserved.        *
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
#include "FXString.h"
#include "FXElement.h"
#include "FXException.h"
#include "FXVariant.h"
#include "FXVariantMap.h"


/*
  Notes:
  - We store the hash key, so that 99.999% of the time we can compare hash numbers;
    only when hash numbers match do we need to compare keys. With a pretty decent hash 
    function, the number of calls to strcmp() should be roughly the same as the number 
    of successful lookups.
    
  - When entry is removed, its key and data are cleared, but its hash value remains the
    same; in other words, voided slots have empty key but non-zero hash value, and free slots
    have empty key AND zero hash value.
    
  - Invariants:

      1 Always at least one table entry marked as "free" i.e. key==empty and hash==0.

      2 Table grows when number of free slots becomes less than or equal to 1+N/4.

      3 Table shrinks when used number of slots becomes less than or equal to N/4.

  - Minimum table size is 1 (one free slot). Thus, for a table to have one element its
    size must be at least  2.
    
  - Empty table is represented by magic EMPTY table value.  This is compile-time constant
    data and will never change.
    
  - NULL keys or empty-string keys are not allowed.
  
  - Similar to FXDictionary; reimplemented to support FXVariant as payload.
*/

#define EMPTY     ((Entry*)(__variantmap__empty__+3))
#define NOMEMORY  ((Entry*)(((FXival*)NULL)+3))
#define BSHIFT    5

using namespace FX;

/*******************************************************************************/

namespace FX {


// Empty dictionary table value
extern const FXint __string__empty__[];
extern const FXival __variantmap__empty__[];
const FXival __variantmap__empty__[8]={1,0,1,(FXival)(__string__empty__+1),0,0,0,0};


// Construct empty dictionary
FXVariantMap::FXVariantMap():table(EMPTY){
  FXASSERT(sizeof(FXVariantMap)==sizeof(FXptr));
  FXASSERT(sizeof(Entry)<=sizeof(FXival)*4);
  }


// Construct from another map
FXVariantMap::FXVariantMap(const FXVariantMap& other):table(EMPTY){
  FXASSERT(sizeof(FXVariantMap)==sizeof(FXptr));
  FXASSERT(sizeof(Entry)<=sizeof(FXival)*4);
  if(no(other.no())){
    copyElms(table,other.table,no());
    free(other.free());
    used(other.used());
    }
  }


// Adjust the size of the table
FXbool FXVariantMap::no(FXival n){
  register FXival m=no();
  if(__likely(m!=n)){
    register Entry *elbat;

    // Release old table
    if(1<m){
      destructElms(table,m);
      ::free(((FXival*)table)-3);
      table=EMPTY;
      }

    // Allocate new table
    if(1<n){
      if((elbat=(Entry*)(((FXival*)::calloc(sizeof(FXival)*3+sizeof(Entry)*n,1))+3))==NOMEMORY) return false;
      ((FXival*)elbat)[-3]=n;
      ((FXival*)elbat)[-2]=0;
      ((FXival*)elbat)[-1]=n;
      constructElms(elbat,n);
      table=elbat;
      }
    }
  return true;
  }


// Resize the table to the given size, keeping contents
FXbool FXVariantMap::resize(FXival n){
  FXVariantMap elbat;
  FXASSERT((n&(n-1))==0);       // Power of 2
  FXASSERT((n-used())>0);       // At least one free slot
  if(elbat.no(n)){
    if(1<elbat.no() && 1<no()){
      register FXuval p,b,h,x;
      register FXival i;
      for(i=0; i<no(); ++i){                  // Hash existing entries into new table
        p=b=h=table[i].hash;
        if(!table[i].key.empty()){
          while(elbat.table[x=p&(n-1)].hash){ // Locate slot
            p=(p<<2)+p+b+1;
            b>>=BSHIFT;
            }
          elbat.table[x].key.adopt(table[i].key);   // Steal string from old table
          elbat.table[x].data.adopt(table[i].data); // Steal data from old table
          elbat.table[x].hash=h;                    // And copy the hash value
          }
        }
      elbat.free(n-used());     // All non-empty slots now free
      elbat.used(used());       // Used slots not changed
      }
    adopt(elbat);
    return true;
    }
  return false;
  }


// Assignment operator
FXVariantMap& FXVariantMap::operator=(const FXVariantMap& other){
  if(table!=other.table && no(other.no())){
    copyElms(table,other.table,no());
    free(other.free());
    used(other.used());
    }
  return *this;
  }


// Adopt array from another
FXVariantMap& FXVariantMap::adopt(FXVariantMap& other){
  if(table!=other.table && no(1)){
    table=other.table;
    other.table=EMPTY;
    }
  return *this;
  }


// Find slot index for key; return -1 if not found
FXival FXVariantMap::find(const FXchar* ky) const {
  register FXuval p,b,x,h;
  if(__unlikely(!ky || !*ky)){ throw FXRangeException("FXVariantMap::find: null or empty key\n"); }
  p=b=h=FXString::hash(ky);
  FXASSERT(h);
  while(table[x=p&(no()-1)].hash){
    if(table[x].hash==h && table[x].key==ky) return x;
    p=(p<<2)+p+b+1;
    b>>=BSHIFT;
    }
  return -1;
  }


// Return reference to variant assocated with key
FXVariant& FXVariantMap::at(const FXchar* ky){
  register FXuval p,b,h,x;
  if(__unlikely(!ky || !*ky)){ throw FXRangeException("FXVariantMap::at: null or empty key\n"); }
  p=b=h=FXString::hash(ky);
  FXASSERT(h);
  while(table[x=p&(no()-1)].hash){
    if(table[x].hash==h && table[x].key==ky) goto x;   // Return existing slot
    p=(p<<2)+p+b+1;
    b>>=BSHIFT;
    }
  if(__unlikely(free()<=1+(no()>>2)) && __unlikely(!resize(no()<<1))){ throw FXMemoryException("FXVariantMap::at: out of memory\n"); }
  p=b=h;
  while(table[x=p&(no()-1)].hash){
    if(table[x].key.empty()) goto y;                    // Return voided slot
    p=(p<<2)+p+b+1;
    b>>=BSHIFT;
    }
  free(free()-1);                                       // Put into empty slot
y:used(used()+1);
  table[x].key=ky;
  table[x].hash=h;
x:return table[x].data;
  }


// Return constant reference to variant assocated with key
const FXVariant& FXVariantMap::at(const FXchar* ky) const {
  register FXuval p,b,x,h;
  if(__unlikely(!ky || !*ky)){ throw FXRangeException("FXVariantMap::at: null or empty key\n"); }
  p=b=h=FXString::hash(ky);
  FXASSERT(h);
  while(table[x=p&(no()-1)].hash){
    if(table[x].hash==h && table[x].key==ky) goto x;   // Return existing slot
    p=(p<<2)+p+b+1;
    b>>=BSHIFT;
    }
x:return table[x].data;                                 // If not found we stopped at a free slot
  }


// Remove entry from table
void FXVariantMap::remove(const FXchar* ky){
  register FXuval p,b,h,x;
  if(__unlikely(!ky || !*ky)){ throw FXRangeException("FXVariantMap::remove: null or empty key\n"); }
  p=b=h=FXString::hash(ky);
  FXASSERT(h);
  while(table[x=p&(no()-1)].hash!=h || table[x].key!=ky){
    if(!table[x].hash) return;
    p=(p<<2)+p+b+1;
    b>>=BSHIFT;
    }
  table[x].key.clear();         // Void the slot (not empty!)
  table[x].data.clear();        // Clear the variant
  used(used()-1);
  if(__unlikely(used()<=(no()>>2))) resize(no()>>1);
  }


// Compare all non-empty entries
FXbool FXVariantMap::operator==(const FXVariantMap& other) const {
  register FXival i,j;
  if(table!=other.table){
    for(i=0; i<no(); ++i){
      if(key(i).empty()) continue;
      if((j=other.find(key(i)))<0) return false;
      if(data(i)!=other.data(j)) return false;
      }
    }
  return true;
  }


// Clear the table
void FXVariantMap::clear(){
  no(1);
  }


// Destroy table
FXVariantMap::~FXVariantMap(){
  clear();
  }

}
