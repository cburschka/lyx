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
#include <sigc++/basic_signal.h>

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif //SIGC_CXX_NAMESPACES

Signal_::Impl::Impl():incoming_(),outgoing_() {}
Signal_::Impl::~Impl() {}


Signal_::Signal_()
  :impl(0) 
  {}

Signal_::~Signal_()
  {
    delete impl;
  }

bool Signal_::empty() const
  {
    return (!impl||impl->outgoing_.empty());
  }

void Signal_::clear()
  {
    if (impl)
      impl->outgoing_.clear();
  }

SlotData* Signal_::out_connect(SlotData *sd)
  {
   // we now reference the object
   sd->connect();

   // insert in list
   if (!impl) impl=new Impl();
   impl->outgoing_.insert_direct(impl->outgoing_.begin(),sd->sender());
   return sd;
  }

SlotData* Signal_::in_connect()
  {
   SlotData* sd=manage(new SlotData());
   if (!impl) impl=new Impl();
   impl->incoming_.insert_direct(impl->incoming_.end(),sd->receiver());
   return sd;
  }

#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif 
