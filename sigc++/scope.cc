// -*- c++ -*-
/* 
 * Copyright 1999 Karl Nelson <kenelson@ece.ucdavis.edu>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <sigc++/scope.h>
#include <sigc++/object.h>

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif //SIGC_CXX_NAMESPACES

/*********************************************************/

Reference::~Reference()       {if (obj_) obj_->unreference();}
void Reference::set_sink()    {if (obj_) obj_->set_sink();}

void Reference::init(ObjectReferenced *obj)
  {
    obj_=obj;
    cache_=0;
    if (obj_)
      {
        obj_->register_ref(this);
        obj_->reference();
      }
  }

void Reference::set(ObjectReferenced *obj,void* cache,bool is_ptr)
  {
    if (obj_==obj) 
      return;
    if (obj_) obj_->unreference();
    obj_=obj;
    cache_=cache;
    if (obj_)
      {
        obj_->register_ref(this);
        obj_->reference();
        if (is_ptr) obj_->set_sink();
      }
  }

/*********************************************************/
// object interaction properties
void Scope::reference()   {if (obj_) obj_->reference();}
void Scope::unreference() {if (obj_) obj_->unreference();}
void Scope::set_sink()    {if (obj_) obj_->set_sink();}

void Scope::invalid()
  {
    if (obj_) 
      object()->invalid();
  }

void Scope::register_scope(ObjectScoped *obj)    
  {
    if (obj) 
      obj->register_scope(this);
  }

void Scope::register_scope(const Scope *parent)    
  {
    if (parent&&parent->obj_) 
      parent->object()->register_scope(this,parent);
  }

void Scope::unregister_scope()  
  {
    if (obj_) 
      object()->unregister_scope(this);
  }


/*********************************************************/
// object interaction properties
void Scope::set(ObjectScoped *obj,void* cache,bool)
  {
    if (obj_==obj) 
      return;
    unregister_scope();
    register_scope(obj);
    cache_=(obj_?cache:0); 
  }  

void ScopeNode::disconnect(bool dummy)
  {}

void Scope::erase()
  {
    obj_=0;
    cache_=0;
  }

Scope::~Scope()
  {}

/*********************************************************/

// These are here because they are virtual

void Scopes::Uncounted::disconnect(bool l)
  {
    unregister_scope();
  }

Scopes::Uncounted::~Uncounted()
  {
    disconnect();
  }

void Scopes::Extend::set(ObjectScoped *obj,void* cache,bool is_ptr)
  {
    Scope::set(obj,cache,is_ptr);
    if (is_ptr) set_sink();
  }

void Scopes::Extend::on_connect()
  {
    reference();
  }

void Scopes::Extend::erase()
  {
    unreference();
    Scope::erase();
  }

void Scopes::Extend::disconnect(bool l)
  {
    unregister_scope();
  }

Scopes::Extend::~Extend()
  {
    disconnect();
  }

void Scopes::Limit::set(ObjectScoped *obj,void* cache,bool is_ptr)
  {
    Scope::set(obj,cache,is_ptr);
    if (is_ptr) set_sink();
  }

void Scopes::Limit::on_connect()
  {
    reference();
  }

void Scopes::Limit::erase()
  {
    unreference();
    Scope::erase();
  }

void Scopes::Limit::disconnect(bool l)
  {
    invalid();
    unregister_scope();
  }

Scopes::Limit::~Limit()
  {
    disconnect();
  }

/*****************************************************************/

inline void splice_node(ScopeNode *n1,ScopeNode *n2)
  {
    n1->next_=n2;
    n2->prev_=n1;
  }

inline void unlink_node(ScopeNode *n)
  {
    if (n->next_==n) return;
    splice_node(n->prev_,n->next_);
    n->prev_=n->next_=n;
  }

void ScopeNode::remove_self() {unlink_node(this);}
ScopeNode::~ScopeNode() {unlink_node(this);}
void ScopeNode::erase() {}

DataNode::~DataNode()  {}
void DataNode::erase() {delete this;}



ScopeIterator_ ScopeList::insert_direct(Iterator pos,NodeType *tmp)
  {
   if (!tmp) return tmp;
   NodeType *after=pos.node();
   NodeType *before=(NodeType*)(after->prev_);

   splice_node(before,tmp);
   splice_node(tmp,after);
   return tmp;
  }
  
ScopeIterator_ ScopeList::erase(Iterator pos)
  {
   NodeType* loc=pos.node();
   Iterator tmp=pos;
   if (!loc) return tmp;
   ++tmp;  
 
   unlink_node(loc);
   loc->erase();
   return tmp;
  }

void ScopeList::swap_elements(Iterator p1,Iterator p2)
  {
   NodeType *loc1=p1.node();
   NodeType *loc2=p2.node();

   if (!loc1||!loc2) return;

   NodeType *before1=(NodeType*)loc1->prev_;
   NodeType *after1=(NodeType*)loc1->next_;
   NodeType *before2=(NodeType*)loc2->prev_;
   NodeType *after2=(NodeType*)loc2->next_;

   if (before1==loc1)
     {
      // loc1 is a stray?
      if (before2==loc2) return;
      splice_node(loc2,loc2);
      splice_node(before2,loc1);
      splice_node(loc1,after2);
      return; 
     }
   if (before2==loc2)
     {
      // loc2 is a stray?
      splice_node(loc1,loc1);
      splice_node(before1,loc2);
      splice_node(loc2,after1);
      return; 
     }
   if (loc1==before2) 
     {// loc1 is right before loc2
      splice_node(before1,loc2);
      splice_node(loc2,loc1);
      splice_node(loc1,after2);
      return;
     }
   if (loc2==before1)
     {// loc2 is right before loc1
      splice_node(before2,loc1);
      splice_node(loc1,loc2);
      splice_node(loc2,after1);
      return;
     }

   splice_node(before1,loc2);
   splice_node(loc2,after1);
   splice_node(before2,loc1);
   splice_node(loc1,after2);
  }

#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif 
