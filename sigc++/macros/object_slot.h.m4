dnl 
dnl Object Slot Templates
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
  SigC::slot() (obj)
  -----------------------
  slot() can be applied to a object method to form a Slot with a 
  profile equivelent to the method.  At the same time an instance
  of that object must be specified.  The object must be derived
  from SigC::Object.

  Sample usage:

    struct A: public SigC::Object
      {
       void foo(int,int);
      } a;

    Slot2<void,int,int> s=slot(a,&A::foo);

*/


#include <sigc++/slot.h>
#include <sigc++/scope.h>

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif

dnl
dnl OBJECT_SLOT
dnl
define([OBJECT_SLOT],[dnl
/****************************************************************
***** Object Slot NUM($1)
****************************************************************/
OBJECT_SLOT_IMPL(R,[$1])

#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
OBJECT_SLOT_IMPL(void,[$1])
#endif
#endif

template <LIST(class R,1,ARG_CLASS($1),[$1],[class O,class O2],1)>
inline __SLOT__(R,[$1])
  slot(O* &obj,R (O2::*func)(ARG_TYPE($1)))
  {
    return [ObjectSlot]NUM($1)_<LIST(R,1,ARG_TYPE($1),[$1],O2,1)>
             ::create(obj,func);
  }

template <LIST(class R,1,ARG_CLASS($1),[$1],[class O,class O2],1)>
inline __SLOT__(R,[$1])
  slot(O* const &obj,R (O2::*func)(ARG_TYPE($1)))
  {
    return [ObjectSlot]NUM($1)_<LIST(R,1,ARG_TYPE($1),[$1],O2,1)>
             ::create(obj,func);
  }

template <LIST(class R,1,ARG_CLASS($1),[$1],[class O,class O2],1)>
inline __SLOT__(R,[$1])
  slot(O &obj,R (O2::*func)(ARG_TYPE($1)))
  {
    return [ObjectSlot]NUM($1)_<LIST(R,1,ARG_TYPE($1),[$1],O2,1)>
             ::create(&obj,func);
  }


])dnl

dnl
dnl OBJECT_SLOT_IMPL(R,[P1,P2,...])
dnl
define([OBJECT_SLOT_IMPL],[dnl
LINE(]__line__[)dnl
ifelse($1,void,[dnl
template <LIST(ARG_CLASS($2),[$2],class Obj,1)>
struct [ObjectSlot]NUM($2)_<LIST(void,1,ARG_TYPE($2),[$2],Obj,1)>
],[dnl
template <LIST(class R,1,ARG_CLASS($2),[$2],class Obj,1)>
struct [ObjectSlot]NUM($2)_
])dnl
  {TYPEDEF_RTYPE($1,RType)
   typedef $1     (Obj::*InFunc)(ARG_TYPE($2));
   typedef RType (Obj::*Func)(ARG_TYPE($2));
   typedef __SLOT__($1,[$2]) SlotType;
   typedef CallDataObj3<typename SlotType::Func,Obj,Func> CallData;

   static RType callback(LIST(void* d,1,ARG_BOTH($2),[$2]))
     {
      CallData* data=(CallData*)d;
      ifelse($1,void,,return) ((data->obj)->*(data->func))(ARG_NAME($2));
     }

   static SlotData* create(Obj* obj,InFunc func)
     {
      if (!obj) return 0;
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=obj;
      data.func=(Func)func;
      obj->register_data(tmp->receiver());
      return tmp;
     }
  };

])dnl

OBJECT_SLOT(ARGS(P,0))
OBJECT_SLOT(ARGS(P,1))
OBJECT_SLOT(ARGS(P,2))
OBJECT_SLOT(ARGS(P,3))

#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif

#endif
