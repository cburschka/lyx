/*
 * Run-time polymorphic streams for XTL
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
 * Id: vobjio.h 1.12 Fri, 05 May 2000 18:57:58 +0100 jop 
 */

#ifndef __XTL_VOBJIO
#define __XTL_VOBJIO

class v_buffer {
 public:
	virtual void read(char* ptr, int size)=0;
	virtual void write(char const* ptr, int size)=0;

	virtual void* require(int size)=0;
	virtual void flush()=0;
};

template <class Buffer>
class v_buffer_wrap: public v_buffer {
 protected:
	Buffer buf;

 public:
	virtual void read(char* ptr, int size) {buf.read(ptr, size);}
	virtual void write(char const* ptr, int size) {buf.write(ptr, size);}

	virtual void* require(int size) {return buf.require(size);}
	virtual void flush() {buf.flush();}
};

class v_mem_buffer: public v_buffer {
 private:
	mem_buffer buf;

 public:
	v_mem_buffer(void *b, int size):buf(b, size) {}

	virtual void read(char* ptr, int size) {buf.read(ptr, size);}
	virtual void write(char const* ptr, int size) {buf.write(ptr, size);}

	virtual void* require(int size) {return buf.require(size);}
	virtual void flush() {buf.flush();}
};

#define def_simple_input(type) \
	virtual void input_simple(type& data)=0;

#define def_simple_output(type) \
	virtual void output_simple(const type& data)=0;

class v_format {
 public:
	def_simple_input(bool)
	def_simple_input(char)
	def_simple_input(unsigned char)
	def_simple_input(short)
	def_simple_input(unsigned short)
	def_simple_input(int)
	def_simple_input(unsigned int)
	def_simple_input(long)
	def_simple_input(unsigned long)
	def_simple_input(longlong)
	def_simple_input(unsignedlonglong)
	def_simple_input(float)
	def_simple_input(double)
	
	virtual void input_start()=0;

	virtual void input_start_composite()=0;
	virtual void input_end_composite()=0;

	virtual void input_start_vector()=0;
	virtual void input_end_vector()=0;

	virtual void input_start_array(int& n)=0;
	virtual bool input_end_array(int& n)=0;

	virtual void input_chars(char* data, int size)=0;
	virtual void input_raw(char* data, int size)=0;

	def_simple_output(bool)
	def_simple_output(char)
	def_simple_output(unsigned char)
	def_simple_output(short)
	def_simple_output(unsigned short)
	def_simple_output(int)
	def_simple_output(unsigned int)
	def_simple_output(long)
	def_simple_output(unsigned long)
	def_simple_output(longlong)
	def_simple_output(unsignedlonglong)
	def_simple_output(float)
	def_simple_output(double)
	
	virtual void output_start()=0;

	virtual void output_start_composite()=0;
	virtual void output_end_composite()=0;

	virtual void output_start_vector()=0;
	virtual void output_end_vector()=0;

	virtual void output_start_array(int n)=0;
	virtual void output_end_array()=0;

	virtual void output_chars(char const* data, int size)=0;
	virtual void output_raw(char const* data, int size)=0;

	virtual void reset_buffer()=0;
};

#undef def_simple_input
#undef def_simple_output

#define def_simple_input(type) \
	virtual void input_simple(type& data) { \
		rep.input_simple(data); \
	} 

#define def_simple_output(type) \
	virtual void output_simple(const type& data) { \
		rep.output_simple(data); \
	} 

template <class Format>
class v_format_wrap: public v_format {
 protected:
	Format rep;

 public:
 	typedef typename Format::buffer buffer;

	v_format_wrap(buffer& buf):rep(buf) {}

	def_simple_input(bool);
	def_simple_input(char);
	def_simple_input(unsigned char);
	def_simple_input(short);
	def_simple_input(unsigned short);
	def_simple_input(int);
	def_simple_input(unsigned int);
	def_simple_input(long);
	def_simple_input(unsigned long);
	def_simple_input(longlong);
	def_simple_input(unsignedlonglong);
	def_simple_input(float);
	def_simple_input(double);

	virtual void input_start() {rep.input_start();}

	virtual void input_start_composite() {rep.input_start_composite();}
	virtual void input_end_composite() {rep.input_end_composite();}

	virtual void input_start_vector() {rep.input_start_vector();}
	virtual void input_end_vector() {rep.input_end_vector();}

	virtual void input_start_array(int& n) {rep.input_start_array(n);}
	virtual bool input_end_array(int& n) {return rep.input_end_array(n);}

	virtual void input_chars(char* data, int size) {rep.input_chars(data, size);}
	virtual void input_raw(char* data, int size) {rep.input_raw(data, size);}

	def_simple_output(bool);
	def_simple_output(char);
	def_simple_output(unsigned char);
	def_simple_output(short);
	def_simple_output(unsigned short);
	def_simple_output(int);
	def_simple_output(unsigned int);
	def_simple_output(long);
	def_simple_output(unsigned long);
	def_simple_output(longlong);
	def_simple_output(unsignedlonglong);
	def_simple_output(float);
	def_simple_output(double);
	
	virtual void output_start() {rep.output_start();}

	virtual void output_start_composite() {rep.output_start_composite();}
	virtual void output_end_composite() {rep.output_end_composite();}

	virtual void output_start_vector() {rep.output_start_vector();}
	virtual void output_end_vector() {rep.output_end_vector();}

	virtual void output_start_array(int n) {rep.output_start_array(n);}
	virtual void output_end_array() {rep.output_end_array();}

	virtual void output_chars(char const* data, int size) {rep.output_chars(data, size);}
	virtual void output_raw(char const* data, int size) {rep.output_raw(data, size);}

	virtual void reset_buffer() { rep.reset_buffer(); }
};

#undef def_simple_input
#undef def_simple_output

#endif

