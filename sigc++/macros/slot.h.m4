dnl 
dnl Abstract Slot templates
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

/*

This file just gives the basic definition of Slots. 

Callback# is the 4 byte data necessary for representing all
callback types.

CallData is a specific interpretation of the Callback data.

Slot_ is a pimple on SlotData containing an Object for
holding its referencees, a Dependency that removes the slot
when its caller or receiver die, and a Callback.

Slot is a handle to a Slot_.

*/

#include <sigc++/sigc++config.h>
#include <sigc++/type.h>
#include <sigc++/object.h>
#include <sigc++/handle.h>

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif

// Base node for a polymorphic list of "extra" data needed
// by various slots. 
struct LIBSIGC_API SlotNode
  {
   void *next_;
   SlotNode();
   virtual ~SlotNode()=0;
  };

struct LIBSIGC_API SlotIterator_
  {
	typedef SlotNode NodeType;
   typedef SlotIterator_ Iterator;
   NodeType *node_;

   NodeType* node()             {return node_;}
   const NodeType* node() const {return node_;}

   NodeType& operator*()
     {return *node_;
     }
   const NodeType& operator*() const
     {return *node_;
     }

   bool operator==(const Iterator& i) const
     {return node_==i.node_;
     }
   bool operator!=(const Iterator& i) const
     {return node_!=i.node_;
     }

   Iterator& operator++()
     {
      if (node_)
        node_=(NodeType*)node_->next_;
      return *this;
     }

   Iterator operator++(int)
     {Iterator tmp=*this;
      ++*this;
      return tmp;
     }

   Iterator& operator= (const Iterator& i)
     {
      node_=i.node_;
      return *this;
     }

   SlotIterator_():node_(0) {}
   SlotIterator_(NodeType *node):node_(node) {}
  };

// This is a list for storing internal data for slots
struct LIBSIGC_API SlotList_
  {
   typedef SlotNode NodeType;
   typedef SlotIterator_ Iterator;
   NodeType* head_;

   Iterator begin()             {return ((NodeType*)head_);}
   Iterator end()               {return Iterator();}
   const Iterator begin() const {return ((NodeType*)head_);}
   const Iterator end()   const {return Iterator();}

   // this is best used at the begining of list.
   Iterator insert_direct(Iterator pos,NodeType *n);

   void clear();
   bool empty() const {return head_==0;}

   SlotList_():head_(0)
     {}
   ~SlotList_()
     {clear();}

   private:
     SlotList_(const SlotList_&);
  };


struct SlotData;

// SlotDependent is an internal of SlotData used to unreference the
// Slot when either the sender or receiver have gone away
struct LIBSIGC_API SlotDependent:public ScopeNode
  {
   struct LIBSIGC_API Dep: public ScopeNode
     {
      SlotData *parent;
      virtual void erase();
      Dep() {}
      virtual ~Dep();
     } dep;

   ScopeNode* receiver() {return &dep;}
   ScopeNode* sender()   {return this;}
   SlotData*  parent()   {return dep.parent;}

   bool connected()
     {return (next_!=this);}

   virtual void erase();

   void set_parent(SlotData *s)
     {dep.parent=s;}

   SlotDependent(SlotData &s)
     {dep.parent=&s;}

   SlotDependent()
     {}

   virtual ~SlotDependent();
  };

// common data to all callbacks.  
struct Callback_
  {
   // callback function
   void* (*func_)(void*);

   struct O;
   struct C1
     {
      void* (*f1)(void*);
     };
   struct C2
     {
      O* o;
      void (O::*v)(void);
     };

   // Object pointer or function pointer
   union {C1 a1; C2 a2;};
  };

// All slots have the same base 
struct LIBSIGC_API SlotData:public ObjectScoped
  {
   typedef SlotList_ List;

   SlotDependent dep_;

   ScopeNode* receiver() {return dep_.receiver();}
   ScopeNode* sender()   {return dep_.sender();}

   // Called from signals to tell slot object it is connected
   // invalidates list and sets weak reference
   void connect();

   List list_;
   Callback_ data_;

   Callback_& callback() {return data_;}

   SlotData()
     {dep_.set_parent(this);}
   virtual ~SlotData();
  };


typedef Scopes::Extend SlotExtend;
#ifdef LIBSIGC_MSC
#pragma warning(disable: 4231)
LIBSIGC_TMPL template class LIBSIGC_API Handle<SlotData,SlotExtend>;
#endif
class LIBSIGC_API Connection:protected Handle<SlotData,SlotExtend>
  {
   typedef Handle<SlotData,SlotExtend> Base;
   public:
     // hides virtual method
     void disconnect() {if (obj()) obj()->invalid();}
     bool connected() {return Base::connected ();}

     Connection():Base() {}
     Connection(SlotData *s):Base(s) {}
     Connection(const Connection& s):Base(s) {}
  };

// possible casts of Callback
template <class C,class F>
struct CallDataFunc
  {
   C callback;
   F func;
  };

template <class C,class O>
struct CallDataObj2
  {
   C callback;
   O *obj;
  };

template <class C,class O,class F>
struct CallDataObj3
  { 
   C   callback;
   O*  obj;
   F   func;
  };

// from Abstract_Slots we build abstract slots
// with various lengths of arguments
//   A slot is not concrete til it has a call

dnl
dnl SLOT([P1,P2,...])
dnl
define([SLOT],[dnl
/****************************************************************
***** Abstract Slot NUM($1)
****************************************************************/
SLOT_IMPL(R,[$1])

#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
SLOT_IMPL(void,[$1])
#endif
#endif

])dnl  end SLOT

dnl
dnl SLOT_IMPL(R,[P1,P2,...])
dnl
define([SLOT_IMPL],[dnl
LINE(]__line__[)dnl

ifelse($1,void,[dnl
template <ARG_CLASS($2)>
struct [Callback]NUM($2)<LIST(void,1,ARG_TYPE($2),[$2])>:public Callback_
  {
   typedef void RType;
   typedef RType (*Func)(LIST([void*],1,ARG_TYPE($2),[$2]));
   inline RType call(ARG_REF($2))
     {((Func)(func_))(LIST([(void*)this],1,ARG_NAME($2),[$2]));}
   inline RType operator()(ARG_REF($2))
     {((Func)(func_))(LIST([(void*)this],1,ARG_NAME($2),[$2]));}
  };
],[dnl
template <LIST(class R,1,ARG_CLASS($2),[$2])>
struct [Callback]NUM($2):public Callback_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef Trait<R>::type RType;
#endif
   typedef RType (*Func)(LIST([void*],1,ARG_TYPE($2),[$2]));
   inline RType call(ARG_REF($2))
     {return ((Func)(func_))(LIST([(void*)this],1,ARG_NAME($2),[$2]));}
   inline RType operator()(ARG_REF($2))
     {return ((Func)(func_))(LIST([(void*)this],1,ARG_NAME($2),[$2]));}
  };
])dnl

ifelse($1,void,[dnl
template <ARG_CLASS($2)>
class __SLOT__(void,[$2])
],[dnl
template <LIST(class R,1,ARG_CLASS($2),[$2])>
class [Slot]NUM($2)
])dnl
   :public Handle<SlotData,SlotExtend>
  {
   public:
     typedef Handle<SlotData,SlotExtend> Base;
     typedef [Callback]NUM($2)<LIST($1,1,ARG_TYPE($2),[$2])> Callback;
     typedef ifelse([$1$2],void,,typename) Callback::RType RType;
     typedef RType (*Func)(LIST([void*],1,ARG_TYPE($2),[$2]));

     SlotData* data()     const {return (SlotData*)(scope_.object());}

     [Slot]NUM($2)() {}
     [Slot]NUM($2)(SlotData *s):Base(s)    {}
     [Slot]NUM($2)(const [Slot]NUM($2)& s):Base(s.obj()) {}

     inline RType call(ARG_REF($2)) 
        {
         if (connected()) 
ifelse($1,void,[
           ((Callback&)(data()->callback())).call(ARG_NAME($2));
],[dnl
           return ((Callback&)(data()->callback())).call(ARG_NAME($2));
         return RType();
])dnl
        }
     inline RType operator()(ARG_REF($2)) 
        {
         if (connected()) 
ifelse($1,void,[
           ((Callback&)(data()->callback())).call(ARG_NAME($2));
],[dnl
           return ((Callback&)(data()->callback())).call(ARG_NAME($2));
         return RType();
])dnl
        }
  };
])dnl  end SLOT_IMPL

SLOT(ARGS(P,0))
SLOT(ARGS(P,1))
SLOT(ARGS(P,2))
SLOT(ARGS(P,3))
SLOT(ARGS(P,4))

#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif

#endif // __header__
