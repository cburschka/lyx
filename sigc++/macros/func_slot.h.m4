dnl 
dnl Function slot templates
dnl 
dnl  Copyright (C) 1998 Karl Nelson <kenelson@ece.ucdavis.edu>
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
#include <sigc++/slot.h>

/*
  SigC::slot() (function)
  -----------------------
  slot() can be applied to a function to form a Slot with a 
  profile equivelent to the function.  To avoid warns be
  sure to pass the address of the function.

  Sample usage:

    void foo(int,int);

    Slot2<void,int,int> s=slot(&foo);

*/

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif

// From which we build specific Slots and a set of
// functions for creating a slot of this type

dnl
dnl FUNCTION_SLOT([P1,P2,...])
dnl
define([FUNCTION_SLOT],[dnl
/****************************************************************
***** Function Slot NUM($1)
****************************************************************/
FUNCTION_SLOT_IMPL(R,[$1])

#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
FUNCTION_SLOT_IMPL(void,[$1])
#endif
#endif

template <LIST(class R,1,ARG_CLASS($1),[$1])>
inline __SLOT__(R,[$1])
  slot(R (*func)(ARG_TYPE($1)))
  {
   return [FuncSlot]NUM($1)_<LIST(R,1,ARG_TYPE($1),[$1])>::create(func);
  }

])dnl

dnl
dnl FUNCTION_SLOT_IMPL(R,[P1,P2,...])
dnl
define([FUNCTION_SLOT_IMPL],[dnl
LINE(]__line__[)dnl
ifelse($1,void,[dnl
template <ARG_CLASS($2)>
struct [FuncSlot]NUM($2)_<LIST(void,1,ARG_TYPE($2),[$2])>
],[dnl
template <LIST(class R,1,ARG_CLASS($2),[$2])>
struct [FuncSlot]NUM($2)_
])dnl
  {TYPEDEF_RTYPE($1,RType)
   typedef $1     (*InFunc)(ARG_TYPE($2));
   typedef RType (*Func)(ARG_TYPE($2));
   typedef __SLOT__($1,[$2]) SlotType;

ifelse($1,void,[dnl
   typedef CallDataFunc<IF([$2],[typename ])SlotType::Func,Func> CallData;
],[dnl
   typedef CallDataFunc<typename SlotType::Func,Func> CallData;
])dnl

   static RType callback(LIST(void* data,1,ARG_BOTH($2),[$2]))
     {
      ifelse($1,void,,return) (((CallData*)data)->func)(ARG_NAME($2));
     }

   static SlotData* create(InFunc func)
     {
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      return tmp;
     }
  };
])dnl

FUNCTION_SLOT(ARGS(P,0))
FUNCTION_SLOT(ARGS(P,1))
FUNCTION_SLOT(ARGS(P,2))

#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif

#endif // __header__
