/*
 * GIOP format driver for XTL
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
 * Id: giop.h 1.14 Fri, 12 May 2000 17:21:18 +0100 jop 
 */

#ifndef __XTL_GIOP
#define __XTL_GIOP

// These should be static const fields of GIOP_format, but some
// compilers prefer them like this.
#if   (__BYTE_ORDER == __LITTLE_ENDIAN)
#	define machineEndianess 1
#elif (__BYTE_ORDER == __BIG_ENDIAN)
#	define machineEndianess 0
#endif

// Data is written to the buffer with the byte order of the local machine.
// When reading a buffer (which may come from a different machine), the
// ordering must be accounted for.
// These must be global due to a joint g++/glibc/i386 "feature".
inline void _xtl_n2hs(char const in[2], short* out, bool _bufferEndianess) {
	if( _bufferEndianess == machineEndianess )
		*out=*reinterpret_cast<short const*>( in );
	else
		*out=bswap_16(*reinterpret_cast<unsigned short const*>( in ));
}

inline void _xtl_n2hl(char const in[4], int* out, bool _bufferEndianess) {
	if( _bufferEndianess == machineEndianess )
		*out=*reinterpret_cast<int const*>( in );
	else
		*out=bswap_32(*reinterpret_cast<unsigned int const*>( in ));
}

inline void _xtl_n2hh(char const in[8], longlong* out, bool _bufferEndianess) {
	if( _bufferEndianess == machineEndianess )
		*out=*reinterpret_cast<longlong const*>( in );
	else
		*out=bswap_64(*reinterpret_cast<unsignedlonglong const*>( in ));
}

template <class Buffer>
class GIOP_format: public generic_format<Buffer> {
 private:
	unsigned char bufferEndianess;
	int align;

	// Add padding so that data (of size n) is aligned on
	// n byte boundaries
	int padding( int n ) {
		int pad = ((align % n) ? (n - (align % n)) : 0);
		align += n + pad;
		return pad;
	}

	char* req_align( int n ) {
		int pad = padding( n );
		return ( reinterpret_cast<char*>( require(n+pad) ) + pad );
	}

	char* des_align( int n ) {
		int pad = padding( n );
		return ( reinterpret_cast<char*>( desire(n+pad) ) + pad );
	}

	inline void h2ns( short const* in, char out[2] )
		{ *reinterpret_cast<short*>( out ) = *in; }

	inline void h2nl( int const* in, char out[4] )
		{ *reinterpret_cast<int*>( out ) = *in; }

	inline void h2nh( longlong const* in, char out[8] )
		{ *reinterpret_cast<longlong*>( out ) = *in; }

 public:
 	typedef Buffer buffer;

	GIOP_format(Buffer& buf):
		generic_format<Buffer>(buf),
		bufferEndianess(machineEndianess),
		align(0) {}

	// Allows data output on one machine to be read on another with
	// (possibly) different byte ordering
	void input_start()
		{ input_simple(bufferEndianess); }
		
	void input_start_array(int& n)
		{ input_simple(n); } 
	bool input_end_array(int& n)
		{ return n--<=0; }

	void input_simple( bool& data )
		{ data=!!*req_align(1); }
 	void input_simple( char& data )
		{ data=*req_align(1); }
 	void input_simple( unsigned char& data )
		{ data=*req_align(1); }
 	void input_simple( short& data )
		{ _xtl_n2hs(req_align(2), &data, bufferEndianess); }
 	void input_simple( unsigned short& data )
		{ _xtl_n2hs(req_align(2), reinterpret_cast<short*>( &data ), bufferEndianess); }
 	void input_simple( int& data )
		{ _xtl_n2hl(req_align(4), &data, bufferEndianess); }
 	void input_simple( unsigned int& data )
		{ _xtl_n2hl(req_align(4), reinterpret_cast<int*>( &data ), bufferEndianess); }
	// long is stored as longlong to allow communication
	// between 32 and 64 bit machines
 	void input_simple( long& data )
		{ _xtl_n2hh(req_align(8), reinterpret_cast<longlong*>( &data ), bufferEndianess); }
 	void input_simple( unsigned long& data )
		{ _xtl_n2hh(req_align(8), reinterpret_cast<longlong*>( &data ), bufferEndianess); }
 	void input_simple( longlong& data )
		{ _xtl_n2hh(req_align(8), &data, bufferEndianess); }
 	void input_simple( unsignedlonglong& data )
		{ _xtl_n2hh(req_align(8), reinterpret_cast<longlong*>( &data ), bufferEndianess); }
 	void input_simple( float& data )
		{_xtl_n2hl(req_align(4), reinterpret_cast<int*>( &data ), bufferEndianess); }
 	void input_simple(double& data)
		{_xtl_n2hh(req_align(8), reinterpret_cast<longlong*>( &data ), bufferEndianess); }

	void input_chars(char* data, int size) {
		input_raw(data, size);
	}

	void input_raw(char* data, int size) {
		int i;
		for(i=0;i<(size>>8)-1;i++,data+=256)
			memcpy(data, require(256), 256);
		int res=size-(i<<8);
		memcpy(data, require(res), res);
		align+=res;
	}

 	void output_start() {
		align = 0;
		output_simple(bufferEndianess);
	}

	void output_start_array(int n) {output_simple(n);}
	void output_end_array() {}
	
 	void output_simple( bool const& data )
		{ *des_align(1)=data?1:0; }
 	void output_simple( char const& data )
		{ *des_align(1)=data; }
 	void output_simple( unsigned char const& data )
		{ *des_align(1)=data; }
 	void output_simple( short const& data )
		{ h2ns(&data, des_align(2)); }
 	void output_simple( unsigned short const& data )
		{ h2ns(reinterpret_cast<short const*>( &data ), des_align(2)); }
 	void output_simple( int const& data )
		{ h2nl(&data, des_align(4)); }
 	void output_simple( unsigned int const& data )
		{ h2nl(reinterpret_cast<int const*>( &data ), des_align(4)); }
 	void output_simple( long const& data )
		{ h2nh(reinterpret_cast<longlong const*>( &data ), des_align(8)); }
 	void output_simple( unsigned long const& data )
		{ h2nh(reinterpret_cast<longlong const*>( &data ), des_align(8)); }
 	void output_simple( longlong const& data )
		{ h2nh(&data, des_align(8)); }
 	void output_simple( unsignedlonglong const& data )
		{ h2nh(reinterpret_cast<longlong const*>( &data ), des_align(8)); }
 	void output_simple( float const& data )
		{ h2nl(reinterpret_cast<int const*>( &data ), des_align(4)); }
 	void output_simple( double const& data )
		{ h2nh(reinterpret_cast<longlong const*>( &data ), des_align(8)); }

	void output_chars(char const* data, int size) {
		output_raw(data, size);
	}

	void output_raw(char const* data, int size) {
		int i;
		for(i=0;i<(size>>8)-1;i++,data+=256)
			memcpy(desire(256), data, 256);
		int res=size-(i<<8);
		memcpy(desire(res), data, res);
		align+=res;
	}
};

#endif // __XTL_GIOP
