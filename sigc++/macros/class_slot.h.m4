dnl 
dnl Class Slot Templates
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
  SigC::slot_class() (class)
  -----------------------
  slot_class() can be applied to a class method to form a Slot with a
  profile equivelent to the method.  At the same time an instance
  of that class must be specified.  This is an unsafe interface.

  This does NOT require that the class be derived from SigC::Object.
  However, the object should be static with regards to the signal system.
  (allocated within the global scope.)  If it is not and a connected
  slot is call it will result in a seg fault.  If the object must
  be destroyed before the connected slots, all connections must
  be disconnected by hand.

  Sample usage:

    struct A
      {
       void foo(int,int);
      } a;

    Slot2<void,int,int> s=slot_class(a,&A::foo);

*/


#include <sigc++/object_slot.h>

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif

dnl
dnl CLASS_SLOT
dnl
define([CLASS_SLOT],[dnl
/****************************************************************
***** Class Slot NUM($1)
****************************************************************/
template <LIST(class R,1,ARG_CLASS($1),[$1],class Obj,1)>
struct [ClassSlot]NUM($1)_:public [ObjectSlot]NUM($1)_<LIST(R,1,ARG_TYPE($1),[$1],Obj,1)>
  {
   typedef [ObjectSlot]NUM($1)_<LIST(R,1,ARG_TYPE($1),[$1],Obj,1)> Base;
   typedef typename Base::InFunc InFunc;

   static SlotData* create(Obj* obj,InFunc func)
     {
      if (!obj) return 0;
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=obj;
      data.func=(Func)func;
      return tmp;
     }
  };

template <LIST(class R,1,ARG_CLASS($1),[$1],class Obj,1)>
__SLOT__(R,[$1])
  slot_class(Obj &obj,R (Obj::*func)(ARG_TYPE($1)))
  {return [ClassSlot]NUM($1)_<LIST(R,1,ARG_TYPE($1),[$1],Obj,1)>::create(&obj,func);
  }

template <LIST(class R,1,ARG_CLASS($1),[$1],class Obj,1)>
__SLOT__(R,[$1])
  slot_class(Obj *obj,R (Obj::*func)(ARG_TYPE($1)))
  {return [ClassSlot]NUM($1)_<LIST(R,1,ARG_TYPE($1),[$1],Obj,1)>::create(obj,func);
  }

])dnl

CLASS_SLOT(ARGS(P,0))
CLASS_SLOT(ARGS(P,1))
CLASS_SLOT(ARGS(P,2))

#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif

#endif
