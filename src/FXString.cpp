/********************************************************************************
*                                                                               *
*                           S t r i n g   O b j e c t                           *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997,2005 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXString.cpp,v 1.130 2005/01/16 16:06:07 fox Exp $                       *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXHash.h"
#include "FXStream.h"
#include "FXString.h"


/*
  Notes:
  - The special pointer-value null represents an empty "" string.
  - Strings are never NULL:- this speeds things up a lot.
  - Whenever an empty string "" would result, we try to substitute that with null,
    rather than keep a buffer.
  - In the new representation, '\0' is allowed as a character everywhere; but there
    is always an (uncounted) '\0' at the end.
  - The length preceeds the text in the buffer.
*/


// The string buffer is always rounded to a multiple of ROUNDVAL
// which must be 2^n.  Thus, small size changes will not result in any
// actual resizing of the buffer except when ROUNDVAL is exceeded.
#define ROUNDVAL    16

// Round up to nearest ROUNDVAL
#define ROUNDUP(n)  (((n)+ROUNDVAL-1)&-ROUNDVAL)

// This will come in handy
#define EMPTY       ((FXchar*)&emptystring[1])

using namespace FX;

/*******************************************************************************/

namespace FX {


// Empty string
static const FXint emptystring[2]={0,0};



// Special NULL string
const FXchar FXString::null[4]={0,0,0,0};


// Numbers for hexadecimal
const FXchar FXString::hex[17]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f',0};
const FXchar FXString::HEX[17]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F',0};


// Change the length of the string to len
void FXString::length(FXint len){
  if(*(((FXint*)str)-1)!=len){
    if(0<len){
      if(str==EMPTY)
        str=sizeof(FXint)+(FXchar*)malloc(ROUNDUP(1+len)+sizeof(FXint));
      else
        str=sizeof(FXint)+(FXchar*)realloc(str-sizeof(FXint),ROUNDUP(1+len)+sizeof(FXint));
      str[len]=0;
      *(((FXint*)str)-1)=len;
      }
    else if(str!=EMPTY){
      free(str-sizeof(FXint));
      str=EMPTY;
      }
    }
  }


// Simple construct
FXString::FXString():str(EMPTY){
  }


// Copy construct
FXString::FXString(const FXString& s):str(EMPTY){
  register FXint len=s.length();
  if(0<len){
    length(len);
    memcpy(str,s.str,len);
    }
  }


// Construct and init
FXString::FXString(const FXchar* s):str(EMPTY){
  if(s && s[0]){
    register FXint len=strlen(s);
    length(len);
    memcpy(str,s,len);
    }
  }


// Construct and init with substring
FXString::FXString(const FXchar* s,FXint n):str(EMPTY){
  if(s && 0<n){
    length(n);
    memcpy(str,s,n);
    }
  }


// Construct and fill with constant
FXString::FXString(FXchar c,FXint n):str(EMPTY){
  if(0<n){
    length(n);
    memset(str,c,n);
    }
  }


// Construct string from two parts
FXString::FXString(const FXchar* s1,const FXchar* s2):str(EMPTY){
  register FXint len1=0,len2=0,len;
  if(s1 && s1[0]){ len1=strlen(s1); }
  if(s2 && s2[0]){ len2=strlen(s2); }
  len=len1+len2;
  str=EMPTY;
  if(len){
    length(len);
    memcpy(str,s1,len1);
    memcpy(&str[len1],s2,len2);
    }
  }


// Return partition of string separated by delimiter delim
FXString FXString::section(FXchar delim,FXint start,FXint num) const {
  register FXint len=length(),s,e;
  s=0;
  if(0<start){
    while(s<len){
      ++s;
      if(str[s-1]==delim && --start==0) break;
      }
    }
  e=s;
  if(0<num){
    while(e<len){
      if(str[e]==delim && --num==0) break;
      ++e;
      }
    }
  return FXString(&str[s],e-s);
  }


// Return partition of string separated by delimiters in delim
FXString FXString::section(const FXchar* delim,FXint n,FXint start,FXint num) const {
  register FXint len=length(),s,e,i;
  register FXchar c;
  s=0;
  if(0<start){
    while(s<len){
      c=str[s++];
      i=n;
      while(--i>=0){
        if(delim[i]==c){
          if(--start==0) goto a;
          break;
          }
        }
      }
    }
a:e=s;
  if(0<num){
    while(e<len){
      c=str[e];
      i=n;
      while(--i>=0){
        if(delim[i]==c){
          if(--num==0) goto b;
          break;
          }
        }
      ++e;
      }
    }
b:return FXString(&str[s],e-s);
  }


// Return partition of string separated by delimiters in delim
FXString FXString::section(const FXchar* delim,FXint start,FXint num) const {
  return section(delim,strlen(delim),start,num);
  }


// Return partition of string separated by delimiters in delim
FXString FXString::section(const FXString& delim,FXint start,FXint num) const {
  return section(delim.text(),delim.length(),start,num);
  }


// Assignment
FXString& FXString::operator=(const FXString& s){
  if(str!=s.str){
    register FXint len=s.length();
    if(0<len){
      length(len);
      memmove(str,s.str,len);
      }
    else{
      length(0);
      }
    }
  return *this;
  }


// Assign a string
FXString& FXString::operator=(const FXchar* s){
  if(str!=s){
    if(s && s[0]){
      register FXint len=strlen(s);
      length(len);
      memmove(str,s,len);
      }
    else{
      length(0);
      }
    }
  return *this;
  }


// Concatenate two FXStrings
FXString operator+(const FXString& s1,const FXString& s2){
  return FXString(s1.str,s2.str);
  }


// Concatenate FXString and string
FXString operator+(const FXString& s1,const FXchar* s2){
  return FXString(s1.str,s2);
  }


// Concatenate string and FXString
FXString operator+(const FXchar* s1,const FXString& s2){
  return FXString(s1,s2.str);
  }


// Concatenate FXString and character
FXString operator+(const FXString& s,FXchar c){
  FXchar string[2];
  string[0]=c;
  string[1]=0;
  return FXString(s.str,string);
  }


// Concatenate character and FXString
FXString operator+(FXchar c,const FXString& s){
  FXchar string[2];
  string[0]=c;
  string[1]=0;
  return FXString(string,s.str);
  }


// Fill with a constant
FXString& FXString::fill(FXchar c,FXint n){
  length(n);
  memset(str,c,n);
  return *this;
  }


// Fill up to current length
FXString& FXString::fill(FXchar c){
  memset(str,c,length());
  return *this;
  }


// Assign input character to this string
FXString& FXString::assign(FXchar c){
  length(1);
  str[0]=c;
  return *this;
  }


// Assign input n characters c to this string
FXString& FXString::assign(FXchar c,FXint n){
  length(n);
  memset(str,c,n);
  return *this;
  }


// Assign first n characters of input string to this string
FXString& FXString::assign(const FXchar* s,FXint n){
  if(s && 0<n){
    length(n);
    memmove(str,s,n);
    }
  else{
    length(0);
    }
  return *this;
  }


// Assign input string to this string
FXString& FXString::assign(const FXString& s){
  assign(s.str,s.length());
  return *this;
  }


// Assign input string to this string
FXString& FXString::assign(const FXchar* s){
  if(s && s[0]){
    register FXint n=strlen(s);
    length(n);
    memmove(str,s,n);
    }
  else{
    length(0);
    }
  return *this;
  }


// Insert character at position
FXString& FXString::insert(FXint pos,FXchar c){
  register FXint len=length();
  length(len+1);
  if(pos<=0){
    memmove(&str[1],str,len);
    str[0]=c;
    }
  else if(pos>=len){
    str[len]=c;
    }
  else{
    memmove(&str[pos+1],&str[pos],len-pos);
    str[pos]=c;
    }
  return *this;
  }


// Insert n characters c at specified position
FXString& FXString::insert(FXint pos,FXchar c,FXint n){
  if(0<n){
    register FXint len=length();
    length(len+n);
    if(pos<=0){
      memmove(&str[n],str,len);
      memset(str,c,n);
      }
    else if(pos>=len){
      memset(&str[len],c,n);
      }
    else{
      memmove(&str[pos+n],&str[pos],len-pos);
      memset(&str[pos],c,n);
      }
    }
  return *this;
  }



// Insert string at position
FXString& FXString::insert(FXint pos,const FXchar* s,FXint n){
  if(s && 0<n){
    register FXint len=length();
    length(len+n);
    if(pos<=0){
      memmove(&str[n],str,len);
      memcpy(str,s,n);
      }
    else if(pos>=len){
      memcpy(&str[len],s,n);
      }
    else{
      memmove(&str[pos+n],&str[pos],len-pos);
      memcpy(&str[pos],s,n);
      }
    }
  return *this;
  }


// Insert string at position
FXString& FXString::insert(FXint pos,const FXString& s){
  insert(pos,s.str,s.length());
  return *this;
  }


// Insert string at position
FXString& FXString::insert(FXint pos,const FXchar* s){
  if(s && s[0]){
    register FXint len=length();
    register FXint n=strlen(s);
    length(len+n);
    if(pos<=0){
      memmove(&str[n],str,len);
      memcpy(str,s,n);
      }
    else if(pos>=len){
      memcpy(&str[len],s,n);
      }
    else{
      memmove(&str[pos+n],&str[pos],len-pos);
      memcpy(&str[pos],s,n);
      }
    }
  return *this;
  }


// Add character to the end
FXString& FXString::append(FXchar c){
  register FXint len=length();
  length(len+1);
  str[len]=c;
  return *this;
  }


// Append input n characters c to this string
FXString& FXString::append(FXchar c,FXint n){
  if(0<n){
    register FXint len=length();
    length(len+n);
    memset(&str[len],c,n);
    }
  return *this;
  }


// Add string to the end
FXString& FXString::append(const FXchar* s,FXint n){
  if(s && 0<n){
    register FXint len=length();
    length(len+n);
    memcpy(&str[len],s,n);
    }
  return *this;
  }


// Add string to the end
FXString& FXString::append(const FXString& s){
  append(s.str,s.length());
  return *this;
  }


// Add string to the end
FXString& FXString::append(const FXchar* s){
  if(s && s[0]){
    register FXint len=length();
    register FXint n=strlen(s);
    length(len+n);
    memcpy(&str[len],s,n);
    }
  return *this;
  }


// Append FXString
FXString& FXString::operator+=(const FXString& s){
  append(s.str,s.length());
  return *this;
  }


// Append string
FXString& FXString::operator+=(const FXchar* s){
  append(s);
  return *this;
  }


// Append character
FXString& FXString::operator+=(FXchar c){
  append(c);
  return *this;
  }


// Prepend character
FXString& FXString::prepend(FXchar c){
  register FXint len=length();
  length(len+1);
  memmove(&str[1],str,len);
  str[0]=c;
  return *this;
  }


// Prepend string
FXString& FXString::prepend(const FXchar* s,FXint n){
  if(s && 0<n){
    register FXint len=length();
    length(len+n);
    memmove(&str[n],str,len);
    memcpy(str,s,n);
    }
  return *this;
  }


// Prepend string with n characters c
FXString& FXString::prepend(FXchar c,FXint n){
  if(0<n){
    register FXint len=length();
    length(len+n);
    memmove(&str[n],str,len);
    memset(str,c,n);
    }
  return *this;
  }


// Prepend string
FXString& FXString::prepend(const FXString& s){
  prepend(s.str,s.length());
  return *this;
  }


// Prepend string
FXString& FXString::prepend(const FXchar* s){
  if(s && s[0]){
    register FXint len=length();
    register FXint n=strlen(s);
    length(len+n);
    memmove(&str[n],str,len);
    memcpy(str,s,n);
    }
  return *this;
  }


// Replace character in string
FXString& FXString::replace(FXint pos,FXchar c){
  register FXint len=length();
  if(pos<0){
    length(len+1);
    memmove(&str[1],str,len);
    str[0]=c;
    }
  else if(pos>=len){
    length(len+1);
    str[len]=c;
    }
  else{
    str[pos]=c;
    }
  return *this;
  }


// Replace the m characters at pos with n characters c
FXString& FXString::replace(FXint pos,FXint m,FXchar c,FXint n){
  register FXint len=length();
  if(pos+m<=0){
    if(0<n){
      length(len+n);
      memmove(&str[pos+n],str,len);
      memset(str,c,n);
      }
    }
  else if(len<=pos){
    if(0<n){
      length(len+n);
      memset(&str[len],c,n);
      }
    }
  else{
    if(pos<0){m+=pos;pos=0;}
    if(pos+m>len){m=len-pos;}
    if(m<n){
      length(len-m+n);
      memmove(&str[pos+n],&str[pos+m],len-pos-m);
      }
    else if(m>n){
      memmove(&str[pos+n],&str[pos+m],len-pos-m);
      length(len-m+n);
      }
    if(0<n){
      memset(&str[pos],c,n);
      }
    }
  return *this;
  }


// Replace part of string
FXString& FXString::replace(FXint pos,FXint m,const FXchar* s,FXint n){
  register FXint len=length();
  if(pos+m<=0){
    if(0<n){
      length(len+n);
      memmove(&str[pos+n],str,len);
      memcpy(str,s,n);
      }
    }
  else if(len<=pos){
    if(0<n){
      length(len+n);
      memcpy(&str[len],s,n);
      }
    }
  else{
    if(pos<0){m+=pos;pos=0;}
    if(pos+m>len){m=len-pos;}
    if(m<n){
      length(len-m+n);
      memmove(&str[pos+n],&str[pos+m],len-pos-m);
      }
    else if(m>n){
      memmove(&str[pos+n],&str[pos+m],len-pos-m);
      length(len-m+n);
      }
    if(0<n){
      memcpy(&str[pos],s,n);
      }
    }
  return *this;
  }


// Replace part of string
FXString& FXString::replace(FXint pos,FXint m,const FXString& s){
  replace(pos,m,s.str,s.length());
  return *this;
  }


// Replace part of string
FXString& FXString::replace(FXint pos,FXint m,const FXchar* s){
  replace(pos,m,s,strlen(s));
  return *this;
  }


// Remove section from buffer
FXString& FXString::remove(FXint pos,FXint n){
  if(0<n){
    register FXint len=length();
    if(pos<len && pos+n>0){
      if(pos<0){n+=pos;pos=0;}
      if(pos+n>len){n=len-pos;}
      memmove(&str[pos],&str[pos+n],len-n-pos);
      length(len-n);
      }
    }
  return *this;
  }


// Return number of occurrences of ch in string
FXint FXString::contains(FXchar ch){
  register FXint len=length();
  register FXint c=ch;
  register FXint m=0;
  register FXint i=0;
  while(i<len){
    if(str[i]==c){
      m++;
      }
    i++;
    }
  return m;
  }


// Return number of occurrences of string sub in string
FXint FXString::contains(const FXchar* sub,FXint n){
  register FXint len=length()-n;
  register FXint m=0;
  register FXint i=0;
  while(i<=len){
    if(compare(&str[i],sub,n)==0){
      m++;
      }
    i++;
    }
  return m;
  }


// Return number of occurrences of string sub in string
FXint FXString::contains(const FXchar* sub){
  return contains(sub,strlen(sub));
  }


// Return number of occurrences of string sub in string
FXint FXString::contains(const FXString& sub){
  return contains(sub.text(),sub.length());
  }


// Substitute one character by another
FXString& FXString::substitute(FXchar org,FXchar sub,FXbool all){
  register FXint len=length();
  register FXint c=org;
  register FXint s=sub;
  register FXint i=0;
  while(i<len){
    if(str[i]==c){
      str[i]=s;
      if(!all) break;
      }
    i++;
    }
  return *this;
  }


// Substitute one string by another
FXString& FXString::substitute(const FXchar* org,FXint olen,const FXchar* rep,FXint rlen,FXbool all){
  if(0<olen){
    register FXint pos=0;
    while(pos<=length()-olen){
      if(compare(&str[pos],org,olen)==0){
        replace(pos,olen,rep,rlen);
        if(!all) break;
        pos+=rlen;
        continue;
        }
      pos++;
      }
    }
  return *this;
  }


// Substitute one string by another
FXString& FXString::substitute(const FXchar* org,const FXchar* rep,FXbool all){
  return substitute(org,strlen(org),rep,strlen(rep),all);
  }


// Substitute one string by another
FXString& FXString::substitute(const FXString& org,const FXString& rep,FXbool all){
  return substitute(org.text(),org.length(),rep.text(),rep.length(),all);
  }


// Simplify whitespace in string
FXString& FXString::simplify(){
  if(str!=EMPTY){
    register FXint s=0;
    register FXint d=0;
    register FXint e=length();
    while(s<e && isspace((FXuchar)str[s])) s++;
    while(1){
      while(s<e && !isspace((FXuchar)str[s])) str[d++]=str[s++];
      while(s<e && isspace((FXuchar)str[s])) s++;
      if(s>=e) break;
      str[d++]=' ';
      }
    length(d);
    }
  return *this;
  }


// Remove leading and trailing whitespace
FXString& FXString::trim(){
  if(str!=EMPTY){
    register FXint s=0;
    register FXint e=length();
    while(0<e && isspace((FXuchar)str[e-1])) e--;
    while(s<e && isspace((FXuchar)str[s])) s++;
    memmove(str,&str[s],e-s);
    length(e-s);
    }
  return *this;
  }


// Remove leading whitespace
FXString& FXString::trimBegin(){
  if(str!=EMPTY){
    register FXint s=0;
    register FXint e=length();
    while(s<e && isspace((FXuchar)str[s])) s++;
    memmove(str,&str[s],e-s);
    length(e-s);
    }
  return *this;
  }


// Remove trailing whitespace
FXString& FXString::trimEnd(){
  if(str!=EMPTY){
    register FXint e=length();
    while(0<e && isspace((FXuchar)str[e-1])) e--;
    length(e);
    }
  return *this;
  }


// Truncate string
FXString& FXString::trunc(FXint pos){
  register FXint len=length();
  if(pos>len) pos=len;
  length(pos);
  return *this;
  }


// Clean string
FXString& FXString::clear(){
  length(0);
  return *this;
  }


// Get leftmost part
FXString FXString::left(FXint n) const {
  if(0<n){
    register FXint len=length();
    if(n>len) n=len;
    return FXString(str,n);
    }
  return FXString::null;
  }


// Get rightmost part
FXString FXString::right(FXint n) const {
  if(0<n){
    register FXint len=length();
    if(n>len) n=len;
    return FXString(str+len-n,n);
    }
  return FXString::null;
  }


// Get some part in the middle
FXString FXString::mid(FXint pos,FXint n) const {
  if(0<n){
    register FXint len=length();
    if(pos<len && pos+n>0){
      if(pos<0){n+=pos;pos=0;}
      if(pos+n>len){n=len-pos;}
      return FXString(str+pos,n);
      }
    }
  return FXString::null;
  }


// Return all characters before the nth occurrence of ch, searching forward
FXString FXString::before(FXchar c,FXint n) const {
  register FXint len=length();
  register FXint p=0;
  if(0<n){
    while(p<len){
      if(str[p]==c && --n==0) break;
      p++;
      }
    }
  return FXString(str,p);
  }


// Return all characters before the nth occurrence of ch, searching backward
FXString FXString::rbefore(FXchar c,FXint n) const {
  register FXint p=length();
  if(0<n){
    while(0<p){
      p--;
      if(str[p]==c && --n==0) break;
      }
    }
  return FXString(str,p);
  }


// Return all characters after the nth occurrence of ch, searching forward
FXString FXString::after(FXchar c,FXint n) const {
  register FXint len=length();
  register FXint p=0;
  if(0<n){
    while(p<len){
      p++;
      if(str[p-1]==c && --n==0) break;
      }
    }
  return FXString(&str[p],len-p);
  }


// Return all characters after the nth occurrence of ch, searching backward
FXString FXString::rafter(FXchar c,FXint n) const {
  register FXint len=length();
  register FXint p=len;
  if(0<n){
    while(0<p){
      if(str[p-1]==c && --n==0) break;
      p--;
      }
    }
  return FXString(&str[p],len-p);
  }


// Convert to lower case
FXString& FXString::lower(){
  register FXint len=length();
  for(register FXint i=0; i<len; i++){
    str[i]=tolower((FXuchar)str[i]);
    }
  return *this;
  }


// Convert to upper case
FXString& FXString::upper(){
  register FXint len=length();
  for(register FXint i=0; i<len; i++){
    str[i]=toupper((FXuchar)str[i]);
    }
  return *this;
  }


// Compare strings
FXint compare(const FXchar* s1,const FXchar* s2){
  register const FXuchar *p1=(const FXuchar *)s1;
  register const FXuchar *p2=(const FXuchar *)s2;
  register FXint c1,c2;
  do{
    c1=*p1++;
    c2=*p2++;
    }
  while(c1 && (c1==c2));
  return c1-c2;
  }

FXint compare(const FXchar* s1,const FXString& s2){
  return compare(s1,s2.str);
  }

FXint compare(const FXString& s1,const FXchar* s2){
  return compare(s1.str,s2);
  }

FXint compare(const FXString& s1,const FXString& s2){
  return compare(s1.str,s2.str);
  }


// Compare strings up to n
FXint compare(const FXchar* s1,const FXchar* s2,FXint n){
  register const FXuchar *p1=(const FXuchar *)s1;
  register const FXuchar *p2=(const FXuchar *)s2;
  register FXint c1,c2;
  if(0<n){
    do{
      c1=*p1++;
      c2=*p2++;
      }
    while(--n && c1 && (c1==c2));
    return c1-c2;
    }
  return 0;
  }

FXint compare(const FXchar* s1,const FXString& s2,FXint n){
  return compare(s1,s2.str,n);
  }

FXint compare(const FXString& s1,const FXchar* s2,FXint n){
  return compare(s1.str,s2,n);
  }

FXint compare(const FXString& s1,const FXString& s2,FXint n){
  return compare(s1.str,s2.str,n);
  }


// Compare strings case insensitive
FXint comparecase(const FXchar* s1,const FXchar* s2){
  register const FXuchar *p1=(const FXuchar *)s1;
  register const FXuchar *p2=(const FXuchar *)s2;
  register FXint c1,c2;
  do{
    c1=tolower(*p1++);
    c2=tolower(*p2++);
    }
  while(c1 && (c1==c2));
  return c1-c2;
  }

FXint comparecase(const FXchar* s1,const FXString& s2){
  return comparecase(s1,s2.str);
  }

FXint comparecase(const FXString& s1,const FXchar* s2){
  return comparecase(s1.str,s2);
  }

FXint comparecase(const FXString& s1,const FXString& s2){
  return comparecase(s1.str,s2.str);
  }


// Compare strings case insensitive up to n
FXint comparecase(const FXchar* s1,const FXchar* s2,FXint n){
  register const FXuchar *p1=(const FXuchar *)s1;
  register const FXuchar *p2=(const FXuchar *)s2;
  register FXint c1,c2;
  if(0<n){
    do{
      c1=tolower(*p1++);
      c2=tolower(*p2++);
      }
    while(--n && c1 && (c1==c2));
    return c1-c2;
    }
  return 0;
  }

FXint comparecase(const FXchar* s1,const FXString& s2,FXint n){
  return comparecase(s1,s2.str,n);
  }

FXint comparecase(const FXString& s1,const FXchar* s2,FXint n){
  return comparecase(s1.str,s2,n);
  }

FXint comparecase(const FXString& s1,const FXString& s2,FXint n){
  return comparecase(s1.str,s2.str,n);
  }

/// Comparison operators
FXbool operator==(const FXString& s1,const FXString& s2){
  return compare(s1.str,s2.str)==0;
  }

FXbool operator==(const FXString& s1,const FXchar* s2){
  return compare(s1.str,s2)==0;
  }

FXbool operator==(const FXchar* s1,const FXString& s2){
  return compare(s1,s2.str)==0;
  }

FXbool operator!=(const FXString& s1,const FXString& s2){
  return compare(s1.str,s2.str)!=0;
  }

FXbool operator!=(const FXString& s1,const FXchar* s2){
  return compare(s1.str,s2)!=0;
  }

FXbool operator!=(const FXchar* s1,const FXString& s2){
  return compare(s1,s2.str)!=0;
  }

FXbool operator<(const FXString& s1,const FXString& s2){
  return compare(s1.str,s2.str)<0;
  }

FXbool operator<(const FXString& s1,const FXchar* s2){
  return compare(s1.str,s2)<0;
  }

FXbool operator<(const FXchar* s1,const FXString& s2){
  return compare(s1,s2.str)<0;
  }

FXbool operator<=(const FXString& s1,const FXString& s2){
  return compare(s1.str,s2.str)<=0;
  }

FXbool operator<=(const FXString& s1,const FXchar* s2){
  return compare(s1.str,s2)<=0;
  }

FXbool operator<=(const FXchar* s1,const FXString& s2){
  return compare(s1,s2.str)<=0;
  }

FXbool operator>(const FXString& s1,const FXString& s2){
  return compare(s1.str,s2.str)>0;
  }

FXbool operator>(const FXString& s1,const FXchar* s2){
  return compare(s1.str,s2)>0;
  }

FXbool operator>(const FXchar* s1,const FXString& s2){
  return compare(s1,s2.str)>0;
  }

FXbool operator>=(const FXString& s1,const FXString& s2){
  return compare(s1.str,s2.str)>=0;
  }

FXbool operator>=(const FXString& s1,const FXchar* s2){
  return compare(s1.str,s2)>=0;
  }

FXbool operator>=(const FXchar* s1,const FXString& s2){
  return compare(s1,s2.str)>=0;
  }


// Find n-th occurrence of character, searching forward; return position or -1
FXint FXString::find(FXchar c,FXint pos,FXint n) const {
  register FXint len=length();
  register FXint p=pos;
  register FXint cc=c;
  if(p<0) p=0;
  if(n<=0) return p;
  while(p<len){
    if(str[p]==cc){ if(--n==0) return p; }
    ++p;
    }
  return -1;
  }


// Find n-th occurrence of character, searching backward; return position or -1
FXint FXString::rfind(FXchar c,FXint pos,FXint n) const {
  register FXint len=length();
  register FXint p=pos;
  register FXint cc=c;
  if(p>=len) p=len-1;
  if(n<=0) return p;
  while(0<=p){
    if(str[p]==cc){ if(--n==0) return p; }
    --p;
    }
  return -1;
  }


// Find a character, searching forward; return position or -1
FXint FXString::find(FXchar c,FXint pos) const {
  register FXint len=length();
  register FXint p=pos;
  register FXint cc=c;
  if(p<0) p=0;
  while(p<len){ if(str[p]==cc){ return p; } ++p; }
  return -1;
  }


// Find a character, searching backward; return position or -1
FXint FXString::rfind(FXchar c,FXint pos) const {
  register FXint len=length();
  register FXint p=pos;
  register FXint cc=c;
  if(p>=len) p=len-1;
  while(0<=p){ if(str[p]==cc){ return p; } --p; }
  return -1;
  }


// Find a substring of length n, searching forward; return position or -1
FXint FXString::find(const FXchar* substr,FXint n,FXint pos) const {
  register FXint len=length();
  if(0<=pos && 0<n && n<=len){
    register FXint c=substr[0];
    len=len-n+1;
    while(pos<len){
      if(str[pos]==c){
        if(!compare(str+pos,substr,n)){
          return pos;
          }
        }
      pos++;
      }
    }
  return -1;
  }


// Find a substring, searching forward; return position or -1
FXint FXString::find(const FXchar* substr,FXint pos) const {
  return find(substr,strlen(substr),pos);
  }


// Find a substring, searching forward; return position or -1
FXint FXString::find(const FXString& substr,FXint pos) const {
  return find(substr.text(),substr.length(),pos);
  }


// Find a substring of length n, searching backward; return position or -1
FXint FXString::rfind(const FXchar* substr,FXint n,FXint pos) const {
  register FXint len=length();
  if(0<=pos && 0<n && n<=len){
    register FXint c=substr[0];
    len-=n;
    if(pos>len) pos=len;
    while(0<=pos){
      if(str[pos]==c){
        if(!compare(str+pos,substr,n)){
          return pos;
          }
        }
      pos--;
      }
    }
  return -1;
  }


// Find a substring, searching backward; return position or -1
FXint FXString::rfind(const FXchar* substr,FXint pos) const {
  return rfind(substr,strlen(substr),pos);
  }


// Find a substring, searching backward; return position or -1
FXint FXString::rfind(const FXString& substr,FXint pos) const {
  return rfind(substr.text(),substr.length(),pos);
  }


// Find first character in the set of size n, starting from pos; return position or -1
FXint FXString::find_first_of(const FXchar* set,FXint n,FXint pos) const {
  register FXint len=length();
  register FXint p=pos;
  if(p<0) p=0;
  while(p<len){
    register FXint c=str[p];
    register FXint i=n;
    while(--i>=0){ if(set[i]==c) return p; }
    p++;
    }
  return -1;
  }


// Find first character in the set, starting from pos; return position or -1
FXint FXString::find_first_of(const FXchar* set,FXint pos) const {
  return find_first_of(set,strlen(set),pos);
  }


// Find first character in the set, starting from pos; return position or -1
FXint FXString::find_first_of(const FXString& set,FXint pos) const {
  return find_first_of(set.text(),set.length(),pos);
  }


// Find first character, starting from pos; return position or -1
FXint FXString::find_first_of(FXchar c,FXint pos) const {
  register FXint len=length();
  register FXint p=pos;
  register FXint cc=c;
  if(p<0) p=0;
  while(p<len){ if(str[p]==cc){ return p; } p++; }
  return -1;
  }


// Find last character in the set of size n, starting from pos; return position or -1
FXint FXString::find_last_of(const FXchar* set,FXint n,FXint pos) const {
  register FXint len=length();
  register FXint p=pos;
  if(p>=len) p=len-1;
  while(0<=p){
    register FXint c=str[p];
    register FXint i=n;
    while(--i>=0){ if(set[i]==c) return p; }
    p--;
    }
  return -1;
  }


// Find last character in the set, starting from pos; return position or -1
FXint FXString::find_last_of(const FXchar* set,FXint pos) const {
  return find_last_of(set,strlen(set),pos);
  }


// Find last character in the set, starting from pos; return position or -1
FXint FXString::find_last_of(const FXString& set,FXint pos) const {
  return find_last_of(set.text(),set.length(),pos);
  }


// Find last character, starting from pos; return position or -1
FXint FXString::find_last_of(FXchar c,FXint pos) const {
  register FXint len=length();
  register FXint p=pos;
  register FXint cc=c;
  if(p>=len) p=len-1;
  while(0<=p){ if(str[p]==cc){ return p; } p--; }
  return -1;
  }



// Find first character NOT in the set of size n, starting from pos; return position or -1
FXint FXString::find_first_not_of(const FXchar* set,FXint n,FXint pos) const {
  register FXint len=length();
  register FXint p=pos;
  if(p<0) p=0;
  while(p<len){
    register FXint c=str[p];
    register FXint i=n;
    while(--i>=0){ if(set[i]==c) goto x; }
    return p;
x:  p++;
    }
  return -1;
  }


// Find first character NOT in the set, starting from pos; return position or -1
FXint FXString::find_first_not_of(const FXchar* set,FXint pos) const {
  return find_first_not_of(set,strlen(set),pos);
  }


// Find first character NOT in the set, starting from pos; return position or -1
FXint FXString::find_first_not_of(const FXString& set,FXint pos) const {
  return find_first_not_of(set.text(),set.length(),pos);
  }


// Find first character NOT equal to c, starting from pos; return position or -1
FXint FXString::find_first_not_of(FXchar c,FXint pos) const {
  register FXint len=length();
  register FXint p=pos;
  register FXint cc=c;
  if(p<0) p=0;
  while(p<len){ if(str[p]!=cc){ return p; } p++; }
  return -1;
  }



// Find last character NOT in the set of size n, starting from pos; return position or -1
FXint FXString::find_last_not_of(const FXchar* set,FXint n,FXint pos) const {
  register FXint len=length();
  register FXint p=pos;
  if(p>=len) p=len-1;
  while(0<=p){
    register FXint c=str[p];
    register FXint i=n;
    while(--i>=0){ if(set[i]==c) goto x; }
    return p;
x:  p--;
    }
  return -1;
  }

// Find last character NOT in the set, starting from pos; return position or -1
FXint FXString::find_last_not_of(const FXchar* set,FXint pos) const {
  return find_last_not_of(set,strlen(set),pos);
  }

// Find last character NOT in the set, starting from pos; return position or -1
FXint FXString::find_last_not_of(const FXString& set,FXint pos) const {
  return find_last_not_of(set.text(),set.length(),pos);
  }


// Find last character NOT equal to c, starting from pos; return position or -1
FXint FXString::find_last_not_of(FXchar c,FXint pos) const {
  register FXint len=length();
  register FXint p=pos;
  register FXint cc=c;
  if(p>=len) p=len-1;
  while(0<=p){ if(str[p]!=cc){ return p; } p--; }
  return -1;
  }


// Find number of occurrences of character in string
FXint FXString::count(FXchar c) const {
  register FXint len=length();
  register FXint cc=c;
  register FXint n=0;
  for(register FXint i=0; i<len; i++){
    n+=(str[i]==cc);
    }
  return n;
  }


// Get hash value
FXuint FXString::hash() const {
  register FXint len=length();
  register FXuint h=0;
  for(register FXint i=0; i<len; i++){  // This should be a very good hash function:- just 4 collisions
    h = ((h << 5) + h) ^ str[i];        // on the webster web2 dictionary of 234936 words, and no
    }                                   // collisions at all on the standard dict!
  return h;
  }


// Save
FXStream& operator<<(FXStream& store,const FXString& s){        // Note stream format incompatible with FOX 1.0
  FXint len=s.length();
  store << len;
  store.save(s.str,len);
  return store;
  }


// Load
FXStream& operator>>(FXStream& store,FXString& s){              // Note stream format incompatible with FOX 1.0
  FXint len;
  store >> len;
  s.length(len);
  store.load(s.str,len);
  return store;
  }


// Print formatted string a-la vprintf
FXString& FXString::vformat(const char* fmt,va_list args){
  register FXint len=0;
  if(fmt && *fmt){
    register FXint n=strlen(fmt);       // Result is longer than format string
#if defined(WIN32) || defined(HAVE_VSNPRINTF)
    n+=128;                             // Add a bit of slop
x:  length(n);
    len=vsnprintf(str,n+1,fmt,args);
    if(len<0){ n<<=1; goto x; }         // Some implementations return -1 if not enough room
    if(n<len){ n=len; goto x; }         // Others return how much space would be needed
#else
    n+=1024;                            // Add a lot of slop
    length(n);
    len=vsprintf(str,fmt,args);
#endif
    }
  length(len);
  return *this;
  }


// Print formatted string a-la printf
FXString& FXString::format(const char* fmt,...){
  va_list args;
  va_start(args,fmt);
  vformat(fmt,args);
  va_end(args);
  return *this;
  }


// Furnish our own version if we have to
#ifndef HAVE_VSSCANF
extern "C" int vsscanf(const char* str, const char* format, va_list arg_ptr);
#endif


// Scan
FXint FXString::vscan(const char* fmt,va_list args) const {
  return vsscanf((char*)str,fmt,args);          // Cast needed for HP-UX 11, which has wrong prototype for vsscanf
  }


FXint FXString::scan(const char* fmt,...) const {
  FXint result;
  va_list args;
  va_start(args,fmt);
  result=vscan(fmt,args);
  va_end(args);
  return result;
  }


// Format a string a-la vprintf
FXString FXStringVFormat(const FXchar* fmt,va_list args){
  FXString result;
  result.vformat(fmt,args);
  return result;
  }


// Format a string a-la printf
FXString FXStringFormat(const FXchar* fmt,...){
  FXString result;
  va_list args;
  va_start(args,fmt);
  result.vformat(fmt,args);
  va_end(args);
  return result;
  }


// Conversion of integer to string
FXString FXStringVal(FXlong num,FXint base){
  FXchar buf[66];
  register FXchar *p=buf+66;
  register FXulong nn=(FXulong)num;
  if(base<2 || base>16){ fxerror("FXStringVal: base out of range.\n"); }
  if(num<0){nn=(FXulong)(~num)+1;}
  do{
    *--p=FXString::HEX[nn%base];
    nn/=base;
    }
  while(nn);
  if(num<0) *--p='-';
  FXASSERT(buf<=p);
  return FXString(p,buf+66-p);
  }


// Conversion of unsigned long to string
FXString FXStringVal(FXulong num,FXint base){
  FXchar buf[66];
  register FXchar *p=buf+66;
  register FXulong nn=num;
  if(base<2 || base>16){ fxerror("FXStringVal: base out of range.\n"); }
  do{
    *--p=FXString::HEX[nn%base];
    nn/=base;
    }
  while(nn);
  FXASSERT(buf<=p);
  return FXString(p,buf+66-p);
  }


// Conversion of integer to string
FXString FXStringVal(FXint num,FXint base){
  FXchar buf[34];
  register FXchar *p=buf+34;
  register FXuint nn=(FXuint)num;
  if(base<2 || base>16){ fxerror("FXStringVal: base out of range.\n"); }
  if(num<0){nn=(FXuint)(~num)+1;}
  do{
    *--p=FXString::HEX[nn%base];
    nn/=base;
    }
  while(nn);
  if(num<0) *--p='-';
  FXASSERT(buf<=p);
  return FXString(p,buf+34-p);
  }


// Conversion of unsigned integer to string
FXString FXStringVal(FXuint num,FXint base){
  FXchar buf[34];
  register FXchar *p=buf+34;
  register FXuint nn=num;
  if(base<2 || base>16){ fxerror("FXStringVal: base out of range.\n"); }
  do{
    *--p=FXString::HEX[nn%base];
    nn/=base;
    }
  while(nn);
  FXASSERT(buf<=p);
  return FXString(p,buf+34-p);
  }


// Formatting for reals
static const char *const expo[]={"%.*f","%.*E","%.*G"};


// Conversion of float to string
FXString FXStringVal(FXfloat num,FXint prec,FXbool exp){
  return FXStringFormat(expo[exp],prec,num);
  }


// Conversion of double to string
FXString FXStringVal(FXdouble num,FXint prec,FXbool exp){
  return FXStringFormat(expo[exp],prec,num);
  }


#ifndef HAVE_STRTOLL
extern "C" FXlong strtoll(const char *nptr, char **endptr, int base);
#endif


#ifndef HAVE_STRTOULL
extern "C" FXulong strtoull(const char *nptr, char **endptr, int base);
#endif


// Conversion of string to integer
FXlong FXLongVal(const FXString& s,FXint base){
  return (FXlong)strtoll(s.str,NULL,base);
  }

// Conversion of string to unsigned integer
FXulong FXULongVal(const FXString& s,FXint base){
  return (FXulong)strtoull(s.str,NULL,base);
  }

// Conversion of string to integer
FXint FXIntVal(const FXString& s,FXint base){
  return (FXint)strtol(s.str,NULL,base);
  }


// Conversion of string to unsigned integer
FXuint FXUIntVal(const FXString& s,FXint base){
  return (FXuint)strtoul(s.str,NULL,base);
  }


// Conversion of string to float
FXfloat FXFloatVal(const FXString& s){
  return (FXfloat)strtod(s.str,NULL);
  }


// Conversion of string to double
FXdouble FXDoubleVal(const FXString& s){
  return strtod(s.str,NULL);
  }


// Escape special characters in a string
FXString escape(const FXString& s){
  register FXint len=s.length(),p,c;
  FXString result;
  for(p=0; p<len; p++){
    c=(FXuchar)s[p];
    switch(c){
      case '\n':
        result+="\\n";
        break;
      case '\r':
        result+="\\r";
        break;
      case '\b':
        result+="\\b";
        break;
      case '\v':
        result+="\\v";
        break;
      case '\a':
        result+="\\a";
        break;
      case '\f':
        result+="\\f";
        break;
      case '\t':
        result+="\\t";
        break;
      case '\\':
        result+="\\\\";
        break;
      case '"':
        result+="\\\"";
        break;
      case '\'':
        result+="\\\'";
        break;
      default:
        if(c<0x20 || 0x7f<c){
          result+="\\x";
          result+=FXString::HEX[c>>4];
          result+=FXString::HEX[c&15];
          }
        else{
          result+=c;
          }
        break;
      }
    }
  return result;
  }


// Unescape special characters in a string
FXString unescape(const FXString& s){
  register FXint len=s.length(),p,c,v;
  FXString result;
  for(p=0; p<len; p++){
    c=(FXuchar)s[p];
    if(c=='\\'){
      if(++p>=len) goto x;
      c=(FXuchar)s[p];
      switch(c){
        case 'n':
          result+='\n';
          break;
        case 'r':
          result+='\r';
          break;
        case 'b':
          result+='\b';
          break;
        case 'v':
          result+='\v';
          break;
        case 'a':
          result+='\a';
          break;
        case 'f':
          result+='\f';
          break;
        case 't':
          result+='\t';
          break;
        case '\\':
          result+='\\';
          break;
        case '"':
          result+='\"';
          break;
        case '\'':
          result+='\'';
          break;
        case 'x':               // Hex escape
          v='x';
          if(isxdigit((FXuchar)s[p+1])){
            c=(FXuchar)s[++p];
            v=('a'<=c) ? c-'a'+10 : ('A'<=c) ? c-'A'+10 : c-'0';
            if(isxdigit((FXuchar)s[p+1])){
              c=(FXuchar)s[++p];
              v=v * 16 + (('a'<=c) ? c-'a'+10 : ('A'<=c) ? c-'A'+10 : c-'0');
              }
            }
          result+=v;
          break;
        case '0':               // Octal escape
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
          v=s[p]-'0';
          if('0'<=s[p+1] && s[p+1]<='7'){
            v=(v<<3)+s[++p]-'0';
            if('0'<=s[p+1] && s[p+1]<='7'){
              v=(v<<3)+s[++p]-'0';
              }
            }
          result+=v;
          break;
        default:
          result+=c;
          break;
        }
      continue;
      }
    result+=c;
    }
x:return result;
  }


// Delete
FXString::~FXString(){
  length(0);
  }

}


