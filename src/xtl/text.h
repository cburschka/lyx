/*
 * Text format driver for XTL
 *
 * Copyright (C) 1998-2000 Jose' Orlando Pereira, jop@di.uminho.pt
 * Copyright (C) 2000 Angus Leeming, a.leeming@ic.ac.uk
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
 * Id: text.h 1.14 Fri, 05 May 2000 18:57:58 +0100 jop 
 */

#ifndef __XTL_TEXT
#define __XTL_TEXT

#include <strstream>

#define def_simple_output(type) \
	void output_simple(type const& data) { \
		space(); \
		std::ostrstream os((char*)desire(20), 20); \
		os << data; \
		undesire(20-os.pcount()); \
 	}

// Required to compile "long long" with DEC cxx -std strict_ansi
#define def_simple_output_ll(type) \
	void output_simple(type const& data) { \
		space(); \
		std::ostrstream os((char*)desire(20), 20); \
		os << static_cast<long>( data ); \
		undesire(20-os.pcount()); \
 	}


template <class Buffer>
class text_format: public generic_format<Buffer> {
#if 0
	// Not yet...
 private:
	void gchar(int c)	{}
 public:
	void input_start_composite() {gchar('{');}
	void input_end_composite() {gchar('}');}

	void input_start_vector() {gchar('[');}
	void input_end_vector() {gchar(']');}

	void input_start_array(int& n) {gchar('<');}
	bool input_end_array(int& n) {gchar('>');}

 	void input_simple(int& data)
		{data=69;return *this;}
#endif

 private:
 	bool need;

 	void space() {if (need) pchar(' ');need=true;}
	void pchar(int c)	{*(char*)desire(1)=c;}
 public:
 	typedef Buffer buffer;

	text_format(Buffer& buf):generic_format<Buffer>(buf),need(false) {}

	void output_start_composite() {space();pchar('{');need=false;}
	void output_end_composite() {pchar('}');need=true;}

	void output_start_vector() {space();pchar('[');need=false;}
	void output_end_vector() {pchar(']');need=true;}

	void output_start_array(int n) {space();pchar('<');need=false;}
	void output_end_array() {pchar('>');need=true;}

	def_simple_output(bool)
	def_simple_output(char)
	def_simple_output(unsigned char)
	def_simple_output(short)
	def_simple_output(unsigned short)
	def_simple_output(int)
	def_simple_output(unsigned int)
	def_simple_output(long)
	def_simple_output(unsigned long)
	def_simple_output_ll(longlong)
	def_simple_output_ll(unsignedlonglong)
	def_simple_output(float)
	def_simple_output(double)

	void output_chars(char const* data, int size) {
		output_simple('"');
		write(data, size);
		need=false;
		output_simple('"');
	}

	void output_raw(char const* data, int size) {
		output_chars(data, size);
	}
};

#undef def_simple_output
#undef def_simple_output_ll

#endif
