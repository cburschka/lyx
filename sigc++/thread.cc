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
#define SIGC_THREAD_IMPL
#include <sigc++/thread.h>

#ifdef SIGC_PTHREADS
#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
namespace Threads
{
#endif

Mutex::Mutex(const MutexAttr attr)
  {
    pthread_mutex_init(&mutex_,attr.impl_);
  }

// (needs work)
Mutex::~Mutex()
  {destroy();}

int Mutex::lock()    {return pthread_mutex_lock(&mutex_);}
int Mutex::trylock() {return pthread_mutex_trylock(&mutex_);}
int Mutex::unlock()  {return pthread_mutex_unlock(&mutex_);}
int Mutex::destroy() {return pthread_mutex_destroy(&mutex_);}


Condition::Condition(const CondAttr &attr)
  {
   pthread_cond_init(&cond_,attr.impl_);
  }

Condition::~Condition()
  {destroy();}

int Condition::signal()       {return pthread_cond_signal(&cond_);}
int Condition::broadcast()    {return pthread_cond_broadcast(&cond_);}
int Condition::wait(Mutex &m) {return pthread_cond_wait(&cond_,m);}
int Condition::wait(Mutex &m,struct timespec* spec)
  {return pthread_cond_timedwait(&cond_,m,spec);}
int Condition::destroy()      {return pthread_cond_destroy(&cond_);}


void Semaphore::up()
  {
   access_.lock();
   value_++;
   access_.unlock();
   sig_.signal();
  }

void Semaphore::down()
  {
   access_.lock();
   while (value_<1)
     {sig_.wait(access_);}
   value_--;
   access_.unlock();
  }

Semaphore::Semaphore(int value):value_(value) {}
Semaphore::~Semaphore() {}



void* Thread::call_main_(void* obj)
  {
    Thread *thread=(Thread*)obj;
    return thread->main(thread->arg_);
  }

Thread::Thread(const ThreadAttr &attr):attr_(attr) {}
Thread::~Thread() {}

int Thread::detach()
#ifdef SIGC_PTHREAD_DCE
  {return pthread_detach(&thread_);}
#else
  {return pthread_detach(thread_);}
#endif

int Thread::start(void* arg)
  {
   arg_=arg;
   Thread *t=this;
   return pthread_create(&thread_,attr_.impl_,call_main_,t);
  }

void* Private_::get()
  {
    void* value;
#ifdef SIGC_PTHREAD_DCE
    pthread_getspecific(key_,(pthread_addr_t*)(&value));
#else
    value=(void*)(pthread_getspecific(key_));
#endif
    return value;
  }

void Private_::set(void *value)
  {
    pthread_setspecific(key_,value);
  }

void Private_::create(void (*dtor)(void*))
  {
#ifdef SIGC_PTHREAD_DCE
    pthread_keycreate(&key_,dtor);
#else
    pthread_key_create(&key_,dtor);
#endif
  }

void Private_::destroy()
  {
#ifndef SIGC_PTHREAD_DCE
    pthread_key_delete(key_);
#endif
  }

Private<int>::operator int&() 
{
	int * value = static_cast<int*>(get());
	if (!value)
		set(static_cast<void*>(value = new int(0)));
	return *(value);
}

#ifdef SIGC_PTHREAD_DCE
MutexAttr Mutex::Default={pthread_mutexattr_default};
CondAttr Condition::Default={pthread_condattr_default};
ThreadAttr Thread::Default={pthread_attr_default};
#else
MutexAttr Mutex::Default={0};
CondAttr Condition::Default={0};
ThreadAttr Thread::Default={0};
#endif


#ifdef SIGC_CXX_NAMESPACES
};
};
#endif

#endif
