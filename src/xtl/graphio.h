/*
 * Automatic object graph externalization for XTL
 *
 * Copyright (C) 1998-2000 Jose' Orlando Pereira, jop@di.uminho.pt
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
 * Id: graphio.h 1.8 Fri, 05 May 2000 18:57:58 +0100 jop 
 */

#ifndef __XTL_GRAPHIO
#define __XTL_GRAPHIO

#include <map>

class graph_refs {
 private:
 	int label;

	typedef std::map<void*,int> trans_type;
	trans_type trans;

 public:
 	graph_refs(): label(1) {}

	template <class Format, class Refs, class T>
	void reference(obj_input<Format, Refs>& stream, T*& ptr) {
		int x;
		stream.simple(x);
		if (x==0) {
			ptr=new T;
			trans.insert(pair<void*,int>(label, (void*)ptr));
			label++;
			stream.content(*ptr);
		} else 
			ptr=(T*)trans[x];
	}

	template <class Format, class Refs, class T>
	void reference(obj_output<Format, Refs>& stream, T const* ptr) {
		trans_type::iterator i=trans.find((void*)ptr);
		if (i!=trans.end())
			stream.simple((*i).second);
		else {
			stream.simple(0);
			trans.insert(std::pair<void*,int>((void*)ptr, label));
			int tmp=label;
			label++;
			stream.content(*ptr);
		}
	}
};

#endif

