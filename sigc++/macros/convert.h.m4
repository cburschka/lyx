dnl 
dnl  Convert Slot Templates
dnl 
dnl  Copyright (C) 1998-1999 Karl Nelson <kenelson@ece.ucdavis.edu>
dnl 
dnl  This library is free software; you can redistribute it and/or
dnl  modify it under the terms of the GNU Library General Public
dnl  License as published by the Free Software Foundation; either
dnl  version 2 of the License, or (at your option) any later version.
dnl 
dnl  This library is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl  Library General Public License for more details.
dnl 
dnl  You should have received a copy of the GNU Library General Public
dnl  License along with this library; if not, write to the Free
dnl  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
dnl 
// -*- c++ -*-
dnl Ignore the next line
/* This is a generated file, do not edit.  Generated from __file__ */
include(template.macros.m4)
#ifndef __header__
#define __header__

/*
  SigC::convert
  -------------
  convert() alters a Slot by assigning a conversion function 
  which can completely alter the parameter types of a slot. 

  Only convert functions for changing with same number of
  arguments is compiled by default.  See examples/custom_convert.h.m4 
  for details on how to build non standard ones.

  Sample usage:
    int my_string_to_char(Callback1<int,const char*> *d,const string &s)
    int f(const char*);
    string s=hello;


    Slot1<int,const string &>  s2=convert(slot(f),my_string_to_char);
    s2(s);  

*/
#include <sigc++/adaptor.h>

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif

dnl
dnl ADAPTOR_CONVERT_SLOT([P1..PN],[C0..CN])
dnl
define([ADAPTOR_CONVERT_SLOT],[dnl
/****************************************************************
***** Adaptor Convert Slot NUM($1)
****************************************************************/
ADAPTOR_CONVERT_SLOT_IMPL(R1,[$1],[$2])

#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
ADAPTOR_CONVERT_SLOT_IMPL(void,[$1],[$2])
#endif
#endif

template <LIST(class R1,1,ARG_CLASS($1),[$1],[
          class R2],1,ARG_CLASS($2),[$2])>
__SLOT__(R1,[$1])
  convert(const __SLOT__(R2,[$2]) &s,
          [R1 (*func)](LIST([Callback]NUM($2)PROT(<LIST(R2,1,ARG_TYPE($2),[$2])>)*,1,ARG_TYPE($1),[$1])))
  {return [AdaptorConvertSlot]NUM($1)[_]NUM($2)<LIST(R1,1,ARG_TYPE($1),[$1],[
           R2],1,ARG_TYPE($2),[$2])>::create(s.obj(),func);
  }

])dnl

dnl
dnl ADAPTOR_CONVERT_SLOT_IMPL(R,[P1..PN],[C0..CN])
dnl
define([ADAPTOR_CONVERT_SLOT_IMPL],[dnl
LINE(]__line__[)dnl
ifelse($1,void,[dnl
template <LIST(ARG_CLASS($2),[$2],[
          class R2],1,ARG_CLASS($3),[$3])>
struct [AdaptorConvertSlot]NUM($2)[_]NUM($3)
         <LIST(void,1,ARG_TYPE($2),[$2],[
          R2],1,ARG_TYPE($3),[$3])>
  : public AdaptorSlot_
],[dnl
template <LIST(class $1,1,ARG_CLASS($2),[$2],[
          class R2],1,ARG_CLASS($3),[$3])>
struct [AdaptorConvertSlot]NUM($2)[_]NUM($3): public AdaptorSlot_
])dnl
  {TYPEDEF_RTYPE($1,RType)
   typedef __SLOT__($1,[$2]) SlotType;
   typedef __SLOT__(R2,[$3]) InSlotType;
   typedef typename InSlotType::Callback Callback;
   typedef RType (*Func)  (Callback*,ARG_TYPE($2));
   typedef $1    (*InFunc)(Callback*,ARG_TYPE($2));
   typedef CallDataObj3<typename SlotType::Func,Callback,Func> CallData;

   static RType callback(LIST(void* d,1,ARG_BOTH($2),[$2]))
     {
      CallData* data=(CallData*)d;
      ifelse($1,void,,return) (data->func)(data->obj,ARG_NAME($2));
     }

   static SlotData* create(SlotData *s,InFunc func)
     {
      SlotData* tmp=(SlotData*)s;
      AdaptorNode *node=new AdaptorNode();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      data.obj=(Callback*)&(node->data_);
      return tmp;
     }
  };

])dnl

ADAPTOR_CONVERT_SLOT(ARGS(P,1),ARGS(Q,1))
ADAPTOR_CONVERT_SLOT(ARGS(P,2),ARGS(Q,2))
ADAPTOR_CONVERT_SLOT(ARGS(P,3),ARGS(Q,3))

#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif

#endif
