/*
 * Core externaliztion utilities
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
 * Id: objio.h 1.20 Fri, 05 May 2000 18:57:58 +0100 jop 
 */

#ifndef __XTL_OBJIO
#define __XTL_OBJIO

#include <xtl/config.h>

#include <typeinfo>
#include <stdexcept>
#include <string>

template <class Buffer>
class generic_format {
private:
	Buffer& buffer;
protected:
	void* require(int n)	{return buffer.require(n);}
	void unrequire(int n)	{buffer.unrequire(n);}
	void* desire(int n)	{return buffer.desire(n);}
	void undesire(int n)	{buffer.undesire(n);}

	void read(char* data, int size)		{buffer.read(data, size);}
	void write(char const* data, int size)	{buffer.write(data, size);}
public:
 	generic_format(Buffer& buf):buffer(buf) {}

	void input_start() {}

	void input_start_composite() {}
	void input_end_composite() {}

	void input_start_vector() {}
	void input_end_vector() {}

	virtual void output_start() {}

	void output_start_composite() {}
	void output_end_composite() {}

	void output_start_vector() {}
	void output_end_vector() {}

	void reset_buffer() {
		buffer.rewind();
		output_start();
	}
};

/*
		void input_start_array(int& n) {simple(n);}
		bool input_end_array(int& n) {return n--<=0;}

		void output_start_array(int n) {simple(n);}
		void output_end_array() {}
 */

template <class Buffer>
class raw_format: public generic_format<Buffer> {
 public:
 	typedef Buffer buffer;

 	raw_format(Buffer& buf):generic_format<Buffer>(buf) {}
	
	template <class T>
	inline void input_simple(T& data)
		{data=*((T*)require(sizeof(T)));}

	void input_start_array(int& n) {input_simple(n);}
	bool input_end_array(int& n) {return !(n-->0);}

	void input_chars(char* data, int size) {input_raw(data, size);}

	void input_raw(char* data, int size) {
		int i;
		for(i=0;i<(size>>8)-1;i++,data+=256)
			memcpy(data, require(256), 256);
		int res=size-(i<<8);
		memcpy(data, require(res), res);
	}

	template <class T>
	inline void output_simple(T const& data)
		{*((T*)desire(sizeof(T)))=data;}

	void output_start_array(int n) {output_simple(n);}
	void output_end_array() {}

	void output_chars(char const* data, int size) {output_raw(data, size);}

	void output_raw(char const* data, int size) {
		int i;
		for(i=0;i<(size>>8)-1;i++,data+=256)
			memcpy(desire(256), data, 256);
		int res=size-(i<<8);
		memcpy(desire(res), data, res);
	}
};

#ifdef XTL_CONFIG_CHOICE_MACROS
#include <xtl/macros.h>
#endif

#ifdef XTL_CONFIG_COMPOSITE_BUG
#define content(d) composite(d)
#else
#define content(d) simple(d)
#define array_s array
#define vector_s vector
#endif

class no_refs;

#define def_simple_input(type) \
	inline obj_input& simple(type& data) { \
		format.input_simple(data); \
		return *this; \
	}

template <class Format, class References=no_refs>
class obj_input {
 private:
	Format& format;
 	References refs;

 public:
 	obj_input(Format& buf):format(buf)
		{format.input_start();}

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

	inline obj_input& cstring(char*& data) {
		int size;
		format.input_start_array(size);
		data=new char[size+1];
		format.input_chars(data, size);
		data[size]=0;
		format.input_end_array(size);
		return *this;
	}

	inline obj_input& cstring(char* data, int max) {
		int size;
		format.input_start_array(size);
		format.input_chars(data, size>max?max:size);
		data[size>max?max:size]=0;
		format.input_end_array(size);
		return *this;
	}

	inline obj_input& simple(std::string& data) {
		int size;
		format.input_start_array(size);
		char* tmp=new char[size];
		format.input_chars(tmp, size);
		data=std::string(tmp, size);
		// FIXME: This causes a crash. Investigate...
		//delete[] tmp;
		format.input_end_array(size);
		return *this;
	}

	template <class T>
	inline obj_input& vector(T data[], int size) {
		format.input_start_vector();
		for(int i=0;i<size;i++)
			content(data[i]);
		format.input_end_vector();
		return *this;
	}

	template <class T>
	inline obj_input& array(T*& data, int& size) {
		format.input_start_array(size);
		data=new T[size];
		for(int j=size,i=0;!format.input_end_array(j);i++)
			content(data[i]);
		return *this;
	}

#ifdef XTL_CONFIG_COMPOSITE_BUG
	template <class T>
	inline obj_input& vector_s(T data[], int size) {
		format.input_start_vector();
		for(int i=0;i<size;i++)
			simple(data[i]);
		format.input_end_vector();
		return *this;
	}

	template <class T>
	inline obj_input& array_s(T*& data, int& size) {
		format.input_start_array(size);
		data=new T[size];
		for(int j=size,i=0;!format.input_end_array(j);i++)
			simple(data[i]);
		return *this;
	}
#endif

	inline obj_input& opaque(char data[], int size) {
		format.input_start_vector();
		format.input_raw(data, size);
		format.input_end_vector();
		return *this;
	}

	inline obj_input& bytes(char*& data, int& size) {
		int s=size;
		format.input_start_array(size);
		data=new char[size];
		format.input_raw(data, size);
		format.input_end_array(s);
		return *this;
	}

	// This is for compilers which do not support specialization
	// If possible, use simple(T&) below.
	template <class T>
	inline obj_input& composite(T& data) {
		format.input_start_composite();
		::composite(*this, data);
		format.input_end_composite();
		return *this;
	}

#ifndef XTL_CONFIG_COMPOSITE_BUG
	template <class T>
	inline obj_input& simple(T& data) {
		format.input_start_composite();
		::composite(*this, data);
		format.input_end_composite();
		return *this;
	}
#endif	

	template <class T>
	inline obj_input& reference(T*& data) {
		refs.reference(*this, data);
		return *this;
	}

	template <class T>
	inline obj_input& pointer(T*& data) {
		bool exists;
		simple(exists);
		if (exists) 
			reference(data);
		return *this;
	}

	template <class T>
	inline obj_input& container(T& data) {
		int j=0;
		format.input_start_array(j);
		while(!format.input_end_array(j)) {
			typename T::value_type v;
			content(v);
			data.insert(data.end(), v);
		}
		return *this;
	}

#ifdef XTL_CONFIG_CHOICE_MACROS
	decl_ich_method(2)
	decl_ich_method(3)
	decl_ich_method(4)
	decl_ich_method(5)

	decl_iobj_method(2)
	decl_iobj_method(3)
	decl_iobj_method(4)
	decl_iobj_method(5)
#endif
};

#define def_simple_output(type) \
	inline obj_output& simple(type const& data) { \
		format.output_simple(data); \
		return *this; \
	}

template <class Format, class References=no_refs>
class obj_output {
 private:
 	Format& format;
 	References refs;

 public:
 	obj_output(Format& buf):format(buf)
		{format.output_start();}

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

	inline obj_output& cstring(char const* data) {
		int size=strlen(data);
		format.output_start_array(size);
		format.output_chars(data, size);
		format.output_end_array();
		return *this;
	}

	inline obj_output& cstring(char const* data, int max) {
		int size=strlen(data);
		format.output_start_array(size);
		format.output_chars(data, size);
		format.output_end_array();
		return *this;
	}

	inline obj_output& simple(const std::string& data) {
		int size=data.size();
		format.output_start_array(size);
		format.output_chars(data.data(), size);
		format.output_end_array();
		return *this;
	}

	template <class T>
	inline obj_output& vector(const T data[], int size) {
		format.output_start_vector();
		for(int i=0;i<size;i++)
			content(data[i]);
		format.output_end_vector();
		return *this;
	}

	template <class T>
	inline obj_output& array(T const* data, int size) {
		format.output_start_array(size);
		for(int i=0;i<size;i++)
			content(data[i]);
		format.output_end_array();
		return *this;
	}
#ifdef XTL_CONFIG_COMPOSITE_BUG
	template <class T>
	inline obj_output& vector_s(const T data[], int size) {
		format.output_start_vector();
		for(int i=0;i<size;i++)
			simple(data[i]);
		format.output_end_vector();
		return *this;
	}

	template <class T>
	inline obj_output& array_s(T const* data, int size) {
		format.output_start_array(size);
		for(int i=0;i<size;i++)
			simple(data[i]);
		format.output_end_array();
		return *this;
	}
#endif

	inline obj_output& opaque(const char data[], int size) {
		format.output_start_vector();
		format.output_raw(data, size);
		format.output_end_vector();
		return *this;
	}

	inline obj_output& bytes(char const* data, int size) {
		format.output_start_array(size);
		format.output_raw(data, size);
		format.output_end_array();
		return *this;
	}

	// This is for compilers which do not support specialization
	// If possible, use simple(T&) below.
	template <class T>
	inline obj_output& composite(T const& data) {
		format.output_start_composite();
		::composite(*this, const_cast<T&>(data));
		format.output_end_composite();
		return *this;
	}

#ifndef XTL_CONFIG_COMPOSITE_BUG
	template <class T>
	inline obj_output& simple(T const& data) {
		format.output_start_composite();
		::composite(*this, const_cast<T&>(data));
		format.output_end_composite();
		return *this;
	}
#endif

	template <class T>
	inline obj_output& reference(T const* data) {
		refs.reference(*this, data);
		return *this;
	}

	template <class T>
	inline obj_output& pointer(T const* data) {
		bool exists=data!=NULL;
		simple(exists);
		if (exists)
			reference(data);
		return *this;
	}

	template <class T>
	inline obj_output& container(T const& data) {
		int j=data.size();
		format.output_start_array(j);
		for(typename T::const_iterator i=data.begin();
			i!=data.end();
			i++)
			content(*i);
		format.output_end_array();
		return *this;
	}

#ifdef XTL_CONFIG_CHOICE_MACROS
	decl_och_method(2)
	decl_och_method(3)
	decl_och_method(4)
	decl_och_method(5)

	decl_oobj_method(2)
	decl_oobj_method(3)
	decl_oobj_method(4)
	decl_oobj_method(5)

#define option(t) (t*)NULL

#endif
};

#undef def_simple_input
#undef def_simple_output

template <class Stream, class T>
inline void composite(Stream& stream, T& data) {
	data.composite(stream);
}

class no_refs {
 public:
 	template <class Format, class T>
	void reference(obj_input<Format>& stream, T*& data) {
		delete data;
		data=new T;
		stream.content(*data);
	}

 	template <class Format, class T>
	void reference(obj_output<Format>& stream, T const* data) {
		stream.content(*data);
	}
};

// mem_buffer

class mem_buffer {
 private:
 	char *buffer, *ptr, *lim;

 public:
	class buffer_overflow_error: public std::overflow_error {
	 public:
	 	int bytes_left;
		int bytes_needed;

		buffer_overflow_error(int left, int needed):
		std::overflow_error("XTL mem_buffer overflow"),
			bytes_left(left),bytes_needed(needed) {}
	};

 	mem_buffer(void *buf, int size):
		buffer((char*)buf),ptr((char*)buf),lim(((char*)buf)+size) {}

	inline void read(char* ptr, int size)
		{memcpy(ptr, require(size), size);}
	inline void write(char const* ptr, int size)
		{memcpy(desire(size), ptr, size);}

	inline void* require(int size) {
		char* aux=ptr;
		if ((ptr+=size)>lim)
			throw buffer_overflow_error(lim-aux,size);
		return aux;
	}
	inline void* desire(int size)
		{return require(size);}
	inline void flush()
		{}

	inline void unrequire(int n)
		{ptr-=n;}
	inline void undesire(int n)
		{unrequire(n);}
	inline void rewind()
		{ptr=buffer;}
	inline int size()
		{return ptr-buffer;}
	inline char* data()
		{return buffer;}

	template <class Format>
	inline void composite(obj_input<Format>& stream) {
		int size, idx;
		stream.array(buffer, size).simple(idx);
		ptr=buffer+idx;
		lim=buffer+size;
	}

	template <class Format>
	inline void composite(obj_output<Format>& stream) {
		stream.array(buffer, lim-buffer).simple((int)ptr-buffer);
	}

};

#ifdef XTL_CONFIG_SIMPLE_CONTAINERS

#include <utility>
#include <vector>
#include <list>
#include <map>

template <class Stream, class T1, class T2>
inline void composite(Stream& stream, std::pair<const T1,T2>& data) {
	stream.simple(const_cast<T1&>(data.first));
	stream.simple(data.second);
}

template <class Stream, class T1, class T2>
inline void composite(Stream& stream, std::pair<T1,T2>& data) {
	stream.simple(data.first);
	stream.simple(data.second);
}

template <class Stream, class T>
inline void composite(Stream& stream, std::list<T>& data) {
	stream.container(data);
}

template <class Stream, class T>
inline void composite(Stream& stream, std::vector<T>& data) {
	stream.container(data);
}

template <class Stream, class T1, class T2>
inline void composite(Stream& stream, std::map<T1, T2>& data) {
	stream.container(data);
}

#endif

#endif
