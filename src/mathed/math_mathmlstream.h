#ifndef MATH_MATHMLSTREAM_H
#define MATH_MATHMLSTREAM_H


// Please keep all four streams in one file until the interface has
// settled.


#include <iosfwd>

class MathArray;
class MathInset;


//
//  MathML
//

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

class MathMLStream {
public:
	///
	explicit MathMLStream(std::ostream & os);
	///
	void cr();
	///
	std::ostream & os() { return os_; }
	///
	int & line() { return line_; }
	///
	int & tab() { return tab_; }
private:
	///
	std::ostream & os_;
	///
	int tab_;
	///
	int line_;
	///
	char lastchar_;
};

///
MathMLStream & operator<<(MathMLStream &, MathInset const *);
///
MathMLStream & operator<<(MathMLStream &, MathArray const &);
///
MathMLStream & operator<<(MathMLStream &, char const *);
///
MathMLStream & operator<<(MathMLStream &, char);
///
MathMLStream & operator<<(MathMLStream &, MTag const &);
///
MathMLStream & operator<<(MathMLStream &, ETag const &);



//
// Debugging
//

class NormalStream {
public:
	///
	explicit NormalStream(std::ostream & os) : os_(os) {}
	///
	std::ostream & os() { return os_; }
private:
	///
	std::ostream & os_;
};

///
NormalStream & operator<<(NormalStream &, MathInset const *);
///
NormalStream & operator<<(NormalStream &, MathArray const &);
///
NormalStream & operator<<(NormalStream &, char const *);
///
NormalStream & operator<<(NormalStream &, char);




//
// Maple
//


class MapleStream {
public:
	///
	explicit MapleStream(std::ostream & os) : os_(os) {}
	///
	std::ostream & os() { return os_; }
private:
	///
	std::ostream & os_;
};


///
MapleStream & operator<<(MapleStream &, MathInset const *);
///
MapleStream & operator<<(MapleStream &, MathArray const &);
///
MapleStream & operator<<(MapleStream &, char const *);
///
MapleStream & operator<<(MapleStream &, char);
///
MapleStream & operator<<(MapleStream &, int);


//
// Octave
//


class OctaveStream {
public:
	///
	explicit OctaveStream(std::ostream & os) : os_(os) {}
	///
	std::ostream & os() { return os_; }
private:
	///
	std::ostream & os_;
};

///
OctaveStream & operator<<(OctaveStream &, MathInset const *);
///
OctaveStream & operator<<(OctaveStream &, MathArray const &);
///
OctaveStream & operator<<(OctaveStream &, char const *);
///
OctaveStream & operator<<(OctaveStream &, char);



//
// LaTeX/LyX
//

class WriteStream {
public:
	///
	WriteStream(std::ostream & os, bool fragile);
	///
	explicit WriteStream(std::ostream & os_);
	/// yes... the references will be removed some day...
	int & line() { return line_; }
	///
	bool fragile() const { return fragile_; }
	///
	std::ostream & os() { return os_; }
	///
	bool & firstitem() { return firstitem_; }
private:
	///
	std::ostream & os_;
	///
	bool fragile_;
	/// are we at the beginning of an MathArray?
	bool firstitem_;
	///
	int line_;
};

///
WriteStream & operator<<(WriteStream &, MathInset const *);
///
WriteStream & operator<<(WriteStream &, MathArray const &);
///
WriteStream & operator<<(WriteStream &, char const *);
///
WriteStream & operator<<(WriteStream &, char);
///
WriteStream & operator<<(WriteStream &, int);
///
WriteStream & operator<<(WriteStream &, unsigned int);

#endif
