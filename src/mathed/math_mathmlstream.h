#ifndef MATH_MATH_MLSTREAM
#define MATH_MATH_MLSTREAM

#include <iosfwd>


struct MathMLStream {
	///
	explicit MathMLStream(std::ostream & os) : os_(os) {}
	///
	std::ostream & os_;
	///
	MathMLStream & operator<<(MathInset const *);
	///
	MathMLStream & operator<<(MathArray const &);
	///
	MathMLStream & operator<<(char const *);
	///
	MathMLStream & operator<<(char);
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

#endif
