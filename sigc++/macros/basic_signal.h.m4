dnl 
dnl Basic Signal Templates
dnl 
dnl  Copyright 1999 Karl Nelson <kenelson@ece.ucdavis.edu>
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
#include <sigc++/marshal.h>
#include <sigc++/slot.h>

QT_FIREWALL

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif

/****************************************************************
*****  Signals (build by macros)
****************************************************************/

// common part to all signals
class LIBSIGC_API Signal_
  {
    private:
      Signal_(const Signal_&);

    protected:
      typedef ScopeList List;
   
      struct LIBSIGC_API Impl 
        { 
          typedef ScopeList List;
          List incoming_;
          List outgoing_;
          Impl();
          ~Impl();
        };

      Impl *impl;

      SlotData* in_connect();
      SlotData* out_connect(SlotData *s);

      Signal_();
      virtual ~Signal_();

    public:
      bool empty() const;
      void clear();
  };

dnl
dnl BASIC_SIGNAL([P1, P2, ...])
dnl
define([BASIC_SIGNAL],
[/****************************************************************
*****  Signal NUM($1)
****************************************************************/
LINE(]__line__[)dnl

template <LIST(class R,1,ARG_CLASS($1),[$1],[typename Marsh=class Marshal<R> ],1)>
  class [Signal]NUM($1):public Signal_
  {
   public:
     typedef __SLOT__(R,[$1])                       InSlotType;
     typedef __SLOT__(typename Marsh::OutType,[$1]) OutSlotType;

   private:
#ifdef SIGC_CXX_PARTIAL_SPEC
     typedef typename Marsh::OutType SType;
     typedef R RType;
#else
     typedef Trait<typename Marsh::OutType>::type SType;
     typedef Trait<R>::type RType;
#endif
     typedef typename InSlotType::Callback Callback;
     typedef [Signal]NUM($1)<LIST(R,1,ARG_TYPE($1),[$1],Marsh,1)> Self;
     typedef CallDataObj2<typename OutSlotType::Func,Self> CallData;

     static SType callback(LIST(void* d,1,ARG_BOTH($1),[$1]))
       {
        CallData* data=(CallData*)d;
        return data->obj->emit(ARG_NAME($1));
       }

   public:
     OutSlotType slot()
       {
        SlotData* tmp=in_connect();
        CallData &data=reinterpret_cast<CallData&>(tmp->data_);
        data.callback=(typename OutSlotType::Func)callback;
        data.obj=this;
        return tmp;
       }

     Connection connect(const InSlotType &s)
       {
        return out_connect(s.data());
       }

     SType emit(ARG_REF($1));
     SType operator()(ARG_REF($1))
       {return emit(ARG_NAME($1));}

     [Signal]NUM($1)() {}
     [Signal]NUM($1)(const InSlotType &s) {connect(s);}
     ~[Signal]NUM($1)() {}
  };


// emit
template <LIST(class R,1,ARG_CLASS($1),[$1],class Marsh,1)>
typename  [Signal]NUM($1)<LIST(R,1,ARG_TYPE($1),[$1],Marsh,1)>::SType [Signal]NUM($1)<LIST(R,1,ARG_TYPE($1),[$1],Marsh,1)>::
  emit(ARG_REF($1))
  {
   if (!impl||impl->outgoing_.empty()) return Marsh::default_value();
   List &out=impl->outgoing_; 
   Marsh rc;
   SlotData *data;
   List::Iterator i=out.begin();
   while (i!=out.end())
     {
      data=((SlotDependent*)(i.node()))->parent();
      ++i;
      Callback& s=(Callback&)(data->callback());
      if (rc.marshal(s.call(ARG_NAME($1)))) return rc.value();
     } 
   return rc.value();
  }

#ifdef SIGC_CXX_PARTIAL_SPEC
template <LIST(ARG_CLASS($1),[$1],[class Marsh],1)>
class [Signal]NUM($1)<LIST(void,1,ARG_TYPE($1),[$1],Marsh,1)>
  :public Signal_
  {
   public:
     typedef __SLOT__(void,[$1]) InSlotType;
     typedef __SLOT__(void,[$1]) OutSlotType;
   private:
     typedef ifelse([$1],,,[typename ])InSlotType::Callback Callback;
     typedef [Signal]NUM($1)<LIST(void,1,ARG_TYPE($1),[$1],Marsh,1)> Self;
     typedef CallDataObj2<ifelse([$1],,,[typename ])OutSlotType::Func,Self> CallData;

     static void callback(LIST(void* d,1,ARG_BOTH($1),[$1]))
       {
        CallData* data=(CallData*)d;
        data->obj->emit(ARG_NAME($1));
       }

   public:
     OutSlotType slot()
       {
        SlotData* tmp=in_connect();
        CallData& data=reinterpret_cast<CallData&>(tmp->data_);
        data.callback=callback;
        data.obj=this;
        return tmp;
       }

     Connection connect(const InSlotType &s)
       {
        return out_connect(s.data());
       }

     void emit(ARG_REF($1));
     void operator()(ARG_REF($1))
       {emit(ARG_NAME($1));}

     [Signal]NUM($1)() {}
     [Signal]NUM($1)(const InSlotType &s) {connect(s);}
     ~[Signal]NUM($1)() {}
  };


// emit
template <LIST(ARG_CLASS($1),[$1],class Marsh,1)>
void [Signal]NUM($1)<LIST(void,1,ARG_TYPE($1),[$1],Marsh,1)>::
  emit(ARG_REF($1))
  {
   if (!impl||impl->outgoing_.empty()) return;
   List &out=impl->outgoing_; 
   SlotData *data;
   List::Iterator i=out.begin();
   while (i!=out.end())
     {
      data=((SlotDependent*)(i.node()))->parent();
      ++i;
      Callback& s=(Callback&)(data->callback());
      s.call(ARG_NAME($1));
     }
  }

#endif

])dnl

BASIC_SIGNAL(ARGS(P,0))
BASIC_SIGNAL(ARGS(P,1))
BASIC_SIGNAL(ARGS(P,2))
BASIC_SIGNAL(ARGS(P,3))

#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif

END_QT_FIREWALL

#endif // __header__

