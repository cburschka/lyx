/* Automatic buffer driver for XTL
 *
 * Copyright (C) 2000 Allan Rae, allan.rae@mailbox.uq.edu.au
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
 * Id: autobuf.h 1.4 Fri, 05 May 2000 18:57:58 +0100 jop 
 */

#ifndef __XTL_AUTOBUF
#define __XTL_AUTOBUF

#include <xtl/config.h>
#include <xtl/objio.h>

/** Based on mem_buffer.  It automatically increases it's capacity when being
    written too.  It still throws an exception if there is insufficient data
    when reading.  As a result the input and output routines memory handling
    had to be separated (mem_buffer uses the same code for both) into required
    memory (input) and desired memory (output). ARRae 20000423
    
    Possible improvements:
    auto_mem_buffer(char *, size_t)  same constructor as mem_buffer
    reserve(size_t)                  make it bigger if need be.
    
    Rewrite XTL to use an iterator interface; at least at the buffer level.
*/
class auto_mem_buffer {
public:
	class buffer_overflow_error: public std::overflow_error {
	public:
	 	int bytes_left;
		int bytes_needed;

		buffer_overflow_error(int left, int needed):
			std::overflow_error("XTL auto_mem_buffer overflow"),
			     bytes_left(left),bytes_needed(needed) {}
	};

	auto_mem_buffer() : buffer(0), pos(0), lim(32) {
		buffer = new char[lim];
	}

	auto_mem_buffer(size_t size) : buffer(0), pos(0), lim(4) {
		// the smallest useful size is probably 8 bytes (4*2)
		do {
			lim *= 2;
		} while (lim < size);
		buffer = new char[lim];
	}

// I haven't figured out why yet but this code causes a segfault upon
// destruction of the copy after the destruction of the original.
// HOWEVER, this only occurs in LyX, it doesn't occur in my test program
// which is almost identical to the way LyX operates.  The major differences
// between LyX and the test program are the presense of libsigc++ and xforms.
// The contents of the mem buffer are used to build up an xforms dialog.
// ARRae 20000423
//
// 	auto_mem_buffer(auto_mem_buffer const & o)
// 		: buffer(0), pos(o.pos), lim(o.lim) {
// 		buffer = new char[lim];
// 		memcpy(buffer, o.buffer, (pos > 0) ? pos : lim);
// 	}

 	~auto_mem_buffer() {
		delete[] buffer;
	}

	inline void read(char* ptr, int size) {
		memcpy(ptr, require(size), size);
	}

	inline void write(char const* ptr, int size) {
		memcpy(desire(size), ptr, size);
	}

	inline void* require(int size) {
		size_t aux = pos;
		if ((pos += size) > lim)
			throw buffer_overflow_error(lim - aux, size);
		return buffer + aux;
	}

	inline void* desire(int size) {
		size_t const aux = pos;
		if ((pos += size) > lim) {
			do {
				lim *= 2;
			} while (lim < pos);
			char * tmp = new char[lim];
			memcpy(tmp, buffer, aux);
			delete[] buffer;
			buffer = tmp;
		}
		return buffer + aux;
	}

	inline void flush()
		{}

	inline void unrequire(int n)
		{ pos -= n; }

	inline void undesire(int n)
		{ pos -= n; }

	inline void rewind()
		{ pos = 0; }

	inline int size()
		{ return pos; }

	inline char* data()
		{ return buffer; }

	template <class Format>
	inline void composite(obj_input<Format>& stream) {
		int size, idx;
		stream.array(buffer, size).simple(idx);
		pos = idx;
		lim = size;
		count = 1;
	}
	
	template <class Format>
	inline void composite(obj_output<Format>& stream) {
		stream.array(buffer, lim).simple(pos);
	}
private:
	auto_mem_buffer(auto_mem_buffer const &);

	char * buffer;
	size_t pos, lim;	
};

#endif

