dnl 
dnl  Bind Slot Templates
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
  SigC::bind
  -------------
  bind() alters a Slot by fixing arguments to certain values.

  Argument fixing starts from the last argument.  The slot is
  destroyed in the process and a new one is created, so references
  holding onto the slot will no longer be valid.

  Up to two arguments can be bound at a time with the default
  header.

  Simple Sample usage:

    void f(int,int);
    Slot2<void,int,int> s1=slot(f);

    Slot1<void,int>  s2=bind(s1,1);  // s1 is invalid
    s2(2);  // call f with arguments 2,1 

  Multibinding usage:

    void f(int,int);
    Slot2<void,int,int> s1=slot(f);
 
    Slot0<void>  s2=bind(s1,1,2);  // s1 is invalid
    s2();  // call f with arguments 1,2 

  Type specified usage:
    
    struct A {};
    struct B :public A {};
    B* b;
    Slot0<void, A*> s1;

    Slot0<void> s2=bind(s1, b);  // B* converted to A*
     
*/

#include <sigc++/adaptor.h>
#include <sigc++/scope.h>

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
dnl ADAPTOR_BIND_SLOT([P1..PN],[C0..CM],[A0..AM])
dnl
define([ADAPTOR_BIND_SLOT],[dnl
/****************************************************************
***** Adaptor Bind Slot NUM($1) arguments, NUM($2) hidden arguments
****************************************************************/
ADAPTOR_BIND_SLOT_IMPL(R,[$1],[$2])

#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
ADAPTOR_BIND_SLOT_IMPL(void,[$1],[$2])
#endif
#endif

template <LIST(ARG_CLASS($3),[$3],[
    ]class R,1,[
    ]ARG_CLASS($1),[$1],[
    ]ARG_CLASS($2),[$2])>
inline
__SLOT__(R,[$1])
  bind(const [Slot]eval(NUM($1)+NUM($2))<LIST(R,1,ARG_TYPE($1),[$1],ARG_TYPE($2),[$2])> &s,
       ARG_BOTH($3))
  {return [AdaptorBindSlot]NUM($1)[_]NUM($2)<LIST(R,1,[
           ]ARG_TYPE($1),[$1],[
           ]ARG_TYPE($2),[$2])>::create(s.data(),ARG_NAME($3));
  }

])dnl


dnl
dnl ADAPTOR_BIND_SLOT_IMPL(R,[P1..PN],[C0..CN])
dnl
define([ADAPTOR_BIND_SLOT_IMPL],[dnl
LINE(]__line__[)dnl
ifelse($1,void,[dnl
template <LIST(ARG_CLASS($2),[$2],[
   ]ARG_CLASS($3),[$3])>
struct [AdaptorBindSlot]NUM($2)[_]NUM($3)
   <LIST(void,1,ARG_TYPE($2),[$2],[
    ]ARG_TYPE($3),[$3])> : public AdaptorSlot_
],[dnl
template <LIST(class R,1,[
   ]ARG_CLASS($2),[$2],[
   ]ARG_CLASS($3),[$3])>
struct [AdaptorBindSlot]NUM($2)[_]NUM($3): public AdaptorSlot_
])dnl
  {TYPEDEF_RTYPE($1,RType)
   typedef __SLOT__($1,[$2]) SlotType;
   typedef __SLOT__($1,[$2],[$3]) InSlotType;

   struct Node:public AdaptorNode
     {
      ARG_CBBIND($3)

      Node(ARG_BOTH($3))
	: ARG_CBINIT($3)
 	{ }
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(LIST(void* d,1,ARG_BOTH($2),[$2]))
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      ifelse($1,void,,return) ((typename InSlotType::Callback&)(node->data_))(LIST(ARG_NAME($2),[$2],[
                                     ]ARG_CBNAME($3),[$3]));
     }
   static SlotData* create(SlotData *s,ARG_BOTH($3))
     {
      Node *node=new Node(ARG_NAME($3));
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };
])dnl

ADAPTOR_BIND_SLOT(ARGS(P,0),ARGS(C,1),ARGS(A,1))
ADAPTOR_BIND_SLOT(ARGS(P,1),ARGS(C,1),ARGS(A,1))
ADAPTOR_BIND_SLOT(ARGS(P,2),ARGS(C,1),ARGS(A,1))

ADAPTOR_BIND_SLOT(ARGS(P,0),ARGS(C,2),ARGS(A,2))
ADAPTOR_BIND_SLOT(ARGS(P,1),ARGS(C,2),ARGS(A,2))
ADAPTOR_BIND_SLOT(ARGS(P,2),ARGS(C,2),ARGS(A,2))

#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif

#endif
