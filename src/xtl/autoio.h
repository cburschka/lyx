/*
 * Automatic object hierarchy externalization for XTL
 *
 * Copyright (C) 1998-2000 Jose' Orlando Pereira, Universidade do Minho
 */
/* XTL - eXternalization Template Library - http://gsd.di.uminho.pt/~jop/xtl
 * Copyright (C) 1998-2000 Jose' Orlando Pereira, Universidade do Minho
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
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA
 *
 * Id: autoio.h 1.4 Fri, 05 May 2000 18:57:58 +0100 jop 
 */

#ifndef __XTL_AUTOIO
#define __XTL_AUTOIO

#include <xtl/objio.h>
#include <xtl/vobjio.h>
#include <map>
#include <stdexcept>

class reader;

typedef std::map<int, reader*> externalizable_index;

class externalizable_base;

class reader {
 private:
	int id;

 public:
 	reader(int i, std::map<int, reader*>& repo):id(i) {
		std::map<int, reader*>::const_iterator t=repo.find(id);
		if (t!=repo.end())
			throw std::logic_error("registering duplicate class id");
		repo.insert(std::make_pair(id, this));
	}

 	virtual externalizable_base* read(obj_input<v_format>& stream)=0;

	int classid() const {return id;}
};

template <class T>
class concrete_reader: public reader {
 public:
 	concrete_reader(int i, std::map<int, reader*>& repo):reader(i, repo) {}

 	virtual externalizable_base* read(obj_input<v_format>& stream) {
		T* ptr=new T;
		ptr->vcomposite(stream);
		return ptr;
	}
};

class externalizable_base {
 public:
 	int classid() const {return info().classid();}
 	virtual reader& info() const=0;
	
	virtual void vcomposite(obj_output<v_format>& stream) const=0;
	virtual void vcomposite(obj_input<v_format>& stream)=0;
};

class auto_obj_input: public obj_input<v_format> {
 private:
 	const std::map<int, reader*>& repo;

 public:
 	auto_obj_input(v_format& f, const std::map<int, reader*>& r):
		obj_input<v_format>(f), repo(r) {}

 	template <class T>
 	auto_obj_input& auto_object(T*& data) {
		int id=0;
		simple(id);
		std::map<int, reader*>::const_iterator t=repo.find(id);
		if (t==repo.end())
			throw std::logic_error("unknown class id");
		externalizable_base* ptr=(*t).second->read(*this);
		data=dynamic_cast<T*>(ptr);
		return *this;
	}
};

class auto_obj_output: public obj_output<v_format> {
 public:
 	auto_obj_output(v_format& f):obj_output<v_format>(f) {}

 	template <class T>
 	auto_obj_output& auto_object(T*& data) {
		const externalizable_base* ptr=data;
		int id=ptr->classid();
		simple(id);
		ptr->vcomposite(*this);
		return *this;
	}
};

#define decl_externalizable(name)\
public:\
	virtual reader& info() const {return myinfo;}\
	virtual void vcomposite(obj_output<v_format>& stream) const\
		{const_cast<name*>(this)->composite(stream);}\
	virtual void vcomposite(obj_input<v_format>& stream)\
		{composite(stream);}\
private:\
	static concrete_reader<name> myinfo

#define impl_externalizable(name, id, index)\
concrete_reader<name> name::myinfo(id, index)

#endif
