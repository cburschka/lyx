dnl 
dnl Adaptor Rettype Templates
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
  SigC::rettype
  -------------
  rettype() alters a Slot by changing the return type.

  Only allowed conversions or conversions to void can properly
  be implemented.  The type must always be specified as a
  template parameter. 

  Simple Sample usage:

    int f(int);

    Slot1<void,int>   s1=rettype<void>(slot(&f)); 
    Slot1<float,int>  s2=rettype<float>(slot(&f)); 

*/

#include <sigc++/adaptor.h>

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif

define([FORMAT_ARG_CBASSIGN],[node->LOWER([$1])_=LOWER([$1]);])
define([FORMAT_ARG_CBNAME],[node->LOWER([$1])_])
define([FORMAT_ARG_CBBIND],[[$1] LOWER([$1])_;])

define([ARG_CBASSIGN],[PROT(ARG_LOOP([FORMAT_ARG_CBASSIGN],[[
      ]],$*))])
define([ARG_CBNAME],[PROT(ARG_LOOP([FORMAT_ARG_CBNAME],[[,]],$*))])
define([ARG_CBBIND],[PROT(ARG_LOOP([FORMAT_ARG_CBBIND],[[
      ]],$*))])

dnl
dnl ADAPTOR_RETTYPE_SLOT([P1..PN],[C0..CN])
dnl
define([ADAPTOR_RETTYPE_SLOT],[dnl
/****************************************************************
***** Adaptor Rettype Slot NUM($1)
****************************************************************/
ADAPTOR_RETTYPE_SLOT_IMPL(R1,[$1],[$2])

#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
ADAPTOR_RETTYPE_SLOT_IMPL(void,[$1],[$2])
#endif
#endif

LINE(]__line__[)dnl

template <LIST(class R1,1,class R2,1,ARG_CLASS($1),[$1])>
__SLOT__(R1,[$1])
  rettype(const __SLOT__(R2,[$1]) &s)
  {return [AdaptorRettypeSlot]NUM($1)_<LIST(R1,1,[
           ]ARG_TYPE($1),[$1],R2,1)>::create(s.obj());
  }
])dnl

dnl
dnl ADAPTOR_RETTYPE_SLOT_IMPL(R,[P1..PN],[C0..CN])
dnl
define([ADAPTOR_RETTYPE_SLOT_IMPL],[dnl
LINE(]__line__[)dnl
ifelse($1,void,[dnl
template <LIST([
   ]ARG_CLASS($2),[$2],[
   ]class R2,1)>
struct [AdaptorRettypeSlot]NUM($2)[_]
<LIST(void,1,[
   ]ARG_TYPE($2),[$2],[
   ]R2,1)>
],[dnl
template <LIST(class R1,1,[
   ]ARG_CLASS($2),[$2],[
   ]class R2,1)>
struct [AdaptorRettypeSlot]NUM($2)[_]
])dnl
  : public AdaptorSlot_
  {TYPEDEF_RTYPE($1,RType)
   typedef __SLOT__($1,[$2]) SlotType;
   typedef __SLOT__(R2,[$2]) InSlotType;
   typedef AdaptorNode Node;
   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(LIST(void* d,1,ARG_BOTH($2),[$2]))
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
ifelse($1,void,[dnl
      ((typename InSlotType::Callback&)(node->data_))(ARG_NAME($2));
],[dnl
      return RType(((typename InSlotType::Callback&)(node->data_))(ARG_NAME($2)));
])dnl
     }
   static SlotData* create(SlotData *s)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };
])dnl

ADAPTOR_RETTYPE_SLOT(ARGS(P,0))
ADAPTOR_RETTYPE_SLOT(ARGS(P,1))
ADAPTOR_RETTYPE_SLOT(ARGS(P,2))
ADAPTOR_RETTYPE_SLOT(ARGS(P,3))

#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif

#endif
