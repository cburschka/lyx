dnl 
dnl  Bind Slot Templates
dnl 
dnl  Copyright (C) 1998-1999 Karl Nelson <kenelson@ece.ucdavis.edu>
dnl  Copyright (C) 2000 Carl Nygard <cnygard@bellatlantic.net>
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
/* This was also shamelessly copied, hacked, munched, and carefully 
 * tweaked from KNelson's original bind.h.m4
 * CJN 3.22.00
 */
include(template.macros.m4)
#ifndef __header__
#define __header__

/*
  SigC::retbind
  -------------
  retbind() alters a Slot by fixing the return value to certain values

  Return value fixing ignores any slot return value.  The slot is
  destroyed in the process and a new one is created, so references
  holding onto the slot will no longer be valid.

  Typecasting may be necessary to match arguments between the
  slot and the binding return value.  Types must be an exact match.
  To insure the proper type, the type can be explicitly specified
  on template instantation.

  Simple Sample usage:

    void f(int,int);
    Slot2<void,int,int> s1=slot(f);

    Slot1<int,int,int>  s2=retbind(s1,1);  // s1 is invalid
    cout << "s2: " << s2(2,1) << endl;

  Type specified usage:
    
    struct A {};
    struct B :public A {};
    B* b;
    Slot1<void> s1;

    Slot0<A*> s2=retbind<A*>(s1,b);  // B* must be told to match A*
     
*/

#include <sigc++/adaptor.h>
#include <sigc++/scope.h>

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif

dnl
dnl ADAPTOR_RETBIND_SLOT([P1..PN])
dnl
define([ADAPTOR_RETBIND_SLOT],[dnl
/****************************************************************
***** Adaptor RetBind Slot NUM($1) arguments
****************************************************************/
ADAPTOR_RETBIND_SLOT_IMPL(R,[$1])

#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
ADAPTOR_RETBIND_SLOT_IMPL(void,[$1])
#endif
#endif

template <LIST(class Ret,1,[
    ]class R,1,[
    ]ARG_CLASS($1),[$1])>
__SLOT__(Ret,[$1])
  retbind(const [Slot]NUM($1)<LIST(R,1,ARG_TYPE($1),[$1])> &s,
       Ret ret)
  {return [AdaptorRetBindSlot]NUM($1)<LIST(Ret,1,R,1,[
           ]ARG_TYPE($1),[$1])>::create(s.data(),ret);
  }

])dnl


dnl
dnl ADAPTOR_RETBIND_SLOT_IMPL(R,[P1..PN])
dnl
define([ADAPTOR_RETBIND_SLOT_IMPL],[dnl
LINE(]__line__[)dnl
ifelse($1,void,[dnl
template <LIST(class Ret,1,ARG_CLASS($2),[$2])>
struct [AdaptorRetBindSlot]NUM($2)
   <LIST(Ret,1,void,1,[
   ]ARG_TYPE($2),[$2])> : public AdaptorSlot_
],[dnl
template <LIST(class Ret,1,class $1,1,[
   ]ARG_CLASS($2),[$2])>
struct [AdaptorRetBindSlot]NUM($2): public AdaptorSlot_
])dnl
  {TYPEDEF_RTYPE(Ret,RType)
   typedef __SLOT__(Ret,[$2]) SlotType;
   typedef __SLOT__($1,[$2]) InSlotType;

   struct Node:public AdaptorNode
     {
	   Ret ret_;
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(LIST(void* d,1,ARG_BOTH($2),[$2]))
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      ((typename InSlotType::Callback&)(node->data_))(LIST(ARG_NAME($2),[$2]));
      return node->ret_;
     }
   static SlotData* create(SlotData *s,Ret ret)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      node->ret_ = ret;
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };
])dnl

ADAPTOR_RETBIND_SLOT(ARGS(P,0))
ADAPTOR_RETBIND_SLOT(ARGS(P,1))
ADAPTOR_RETBIND_SLOT(ARGS(P,2))
ADAPTOR_RETBIND_SLOT(ARGS(P,3))

#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif

#endif
