#ifndef MATH_MATHMLSTREAM_H
#define MATH_MATHMLSTREAM_H

#include <iosfwd>
#include "support/LOstream.h"

struct MathArray;
struct MathInset;
struct Buffer;

struct MTag {
	///
	MTag(char const * const tag) : tag_(tag) {}
	///
	char const * const tag_;
};

struct ETag {
	///
	ETag(char const * const tag) : tag_(tag) {}
	///
	char const * const tag_;
};

struct MathMLStream {
	///
	explicit MathMLStream(std::ostream & os);
	///
	MathMLStream & operator<<(MathInset const *);
	///
	MathMLStream & operator<<(MathArray const &);
	///
	MathMLStream & operator<<(char const *);
	///
	MathMLStream & operator<<(char);
	///
	MathMLStream & operator<<(MTag const &);
	///
	MathMLStream & operator<<(ETag const &);
	///
	void cr();

	///
	std::ostream & os_;
	///
	int tab_;
	///
	int line_;
	///
	char lastchar_;
};


struct NormalStream {
	///
	explicit NormalStream(std::ostream & os) : os_(os) {}
	///
	std::ostream & os_;
	///
	NormalStream & operator<<(MathInset const *);
	///
	NormalStream & operator<<(MathArray const &);
	///
	NormalStream & operator<<(char const *);
	///
	NormalStream & operator<<(char);
};


struct MapleStream {
	///
	explicit MapleStream(std::ostream & os) : os_(os) {}
	///
	std::ostream & os_;
	///
	MapleStream & operator<<(MathInset const *);
	///
	MapleStream & operator<<(MathArray const &);
	///
	MapleStream & operator<<(char const *);
	///
	MapleStream & operator<<(char);
	///
	MapleStream & operator<<(int);
};


struct OctaveStream {
	///
	explicit OctaveStream(std::ostream & os) : os_(os) {}
	///
	std::ostream & os_;
	///
	OctaveStream & operator<<(MathInset const *);
	///
	OctaveStream & operator<<(MathArray const &);
	///
	OctaveStream & operator<<(char const *);
	///
	OctaveStream & operator<<(char);
};


struct WriteStream {
	///
	WriteStream(Buffer const * buffer_, std::ostream & os_, bool fragile_);
	///
	explicit WriteStream(std::ostream & os_);

	///
	WriteStream & operator<<(MathInset const *);
	///
	WriteStream & operator<<(MathArray const &);
	///
	WriteStream & operator<<(char const *);
	///
	WriteStream & operator<<(char);
	///
	WriteStream & operator<<(int);
	///
	WriteStream & operator<<(unsigned int);

	///
	Buffer const * buffer;
	///
	std::ostream & os;
	///
	bool fragile;
	/// are we at the beginning of an MathArray?
	bool firstitem;
	///
	int line_;
};


#endif
