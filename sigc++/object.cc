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
#include <sigc++/object.h>

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif // SIGC_CXX_NAMESPACES

int sigc_major_version=SIGC_MAJOR_VERSION;
int sigc_minor_version=SIGC_MINOR_VERSION;
int sigc_micro_version=SIGC_MICRO_VERSION;

ObjectReferenced::ObjectReferenced():
  obj_count_(0),
  obj_dynamic_(0),obj_owned_(1),obj_floating_(1),obj_transfer_(0),
  obj_invalid_(0),obj_destroyed_(0),obj_weak_(0)
  {}

ObjectScoped::ObjectScoped():list_()
  {}

ObjectReferenced::~ObjectReferenced()
  {}

ObjectScoped::~ObjectScoped()
  {
   // we can't be destroyed again.
   obj_destroyed_=1;

   // trash the list.
   invalid(1);
  }

void ObjectReferenced::reference()
  {
   // if we exceed the int limit,  we should unset dynamic_
   if (!(++obj_count_))
     obj_dynamic_=0;
  }

void ObjectReferenced::unreference()
  {
   if (obj_count_
       && (!--obj_count_)
       && obj_dynamic_
       && !obj_floating_
       && !obj_destroyed_
      )
      { 
        obj_destroyed_=1; 
        delete this; 
      }
  }

void ObjectScoped::register_data(ScopeNode *data)
  {
   list_.insert_direct(list_.end(),data);
  }

void ObjectScoped::register_scope(Scope *scope,const Scope *parent)
  {
   if (!scope) return;

   // check for invalid in progress
   if (obj_invalid_)
     return;

   // reregistering a scope
   if (scope->obj_==this) 
     {
      if (obj_transfer_&&(ScopeNode*)parent==list_.begin().node())
        {list_.swap_elements(const_cast<Scope*>(parent),scope);    
         obj_transfer_=0;
        }
      return;
     }

   if (obj_transfer_&&(ScopeNode*)parent==list_.begin().node())
     {
      list_.insert_direct(list_.begin(),scope);
      obj_transfer_=0;
     }
   else
     list_.insert_direct(list_.end(),scope);

   // complete connection
   scope->obj_=this;
   scope->on_connect();
  }

void ObjectScoped::unregister_scope(Scope *scope)
  {
   if (!scope) return;

   // Check for loss of ownership
   if (obj_owned_&&(ScopeNode*)scope==list_.begin().node())
     obj_owned_=0;

   list_.erase(scope);
   scope->obj_=0;
  }

void ObjectScoped::set_weak()
  {
    if (obj_weak_) return;
    obj_weak_=1;
    reference();
  }

struct Invalid_
  {
    ObjectReferenced* r_;
    Invalid_(ObjectReferenced& r): r_(&r) 
      {
        r_->obj_invalid_=1;
        r_->reference();
      }
    ~Invalid_() 
      {
        r_->obj_invalid_=0;
        r_->unreference();
      }
  };

void ObjectScoped::invalid(bool level)
  {
   if (!level&&(obj_invalid_||!obj_dynamic_))
     return;

   List_::Iterator current=list_.begin();
   List_::Iterator next=current;
   if (current==list_.end()&&!obj_weak_)
     return;

   Invalid_ r(*this);

   if (obj_weak_) 
     {
       obj_weak_=0;
       unreference();
     }

   while (current!=(list_.end()))
     {
      ++next;
      (*current).disconnect(level);
      current=next;
     }
  }

Object::~Object()
  {}

void ObjectReferenced::set_dynamic() {obj_dynamic_=1;}


#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif // SIGC_CXX_NAMESPACES
