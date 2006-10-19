// -*- C++ -*-
/**
 * \file MathMLStream.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_MATHMLSTREAM_H
#define MATH_MATHMLSTREAM_H


// Please keep all four streams in one file until the interface has
// settled.


#include "metricsinfo.h"

#include "support/docstream.h"

class MathArray;
class InsetMath;
class MathAtom;

//
// LaTeX/LyX
//

class WriteStream {
public:
	///
	WriteStream(lyx::odocstream & os, bool fragile, bool latex);
	///
	explicit WriteStream(lyx::odocstream & os);
	///
	~WriteStream();
	///
	int line() const { return line_; }
	///
	bool fragile() const { return fragile_; }
	///
	bool latex() const { return latex_; }
	///
	lyx::odocstream & os() { return os_; }
	///
	bool & firstitem() { return firstitem_; }
	///
	void addlines(unsigned int);
	/// writes space if next thing is isalpha()
	void pendingSpace(bool how);
	/// writes space if next thing is isalpha()
	bool pendingSpace() const { return pendingspace_; }
private:
	///
	lyx::odocstream & os_;
	/// do we have to write \\protect sometimes
	bool fragile_;
	/// are we at the beginning of an MathArray?
	bool firstitem_;
	/// are we writing to .tex?
	int latex_;
	/// do we have a space pending?
	bool pendingspace_;
	///
	int line_;
};

///
WriteStream & operator<<(WriteStream &, MathAtom const &);
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



//
//  MathML
//

class MTag {
public:
	///
	MTag(char const * const tag) : tag_(tag) {}
	///
	char const * const tag_;
};

class ETag {
public:
	///
	ETag(char const * const tag) : tag_(tag) {}
	///
	char const * const tag_;
};

class MathMLStream {
public:
	///
	explicit MathMLStream(lyx::odocstream & os);
	///
	void cr();
	///
	lyx::odocstream & os() { return os_; }
	///
	int line() const { return line_; }
	///
	int & tab() { return tab_; }
	///
	friend MathMLStream & operator<<(MathMLStream &, char const *);
private:
	///
	lyx::odocstream & os_;
	///
	int tab_;
	///
	int line_;
	///
	char lastchar_;
};

///
MathMLStream & operator<<(MathMLStream &, MathAtom const &);
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
	explicit NormalStream(lyx::odocstream & os) : os_(os) {}
	///
	lyx::odocstream & os() { return os_; }
private:
	///
	lyx::odocstream & os_;
};

///
NormalStream & operator<<(NormalStream &, MathAtom const &);
///
NormalStream & operator<<(NormalStream &, MathArray const &);
///
NormalStream & operator<<(NormalStream &, char const *);
///
NormalStream & operator<<(NormalStream &, char);
///
NormalStream & operator<<(NormalStream &, int);


//
// Maple
//


class MapleStream {
public:
	///
	explicit MapleStream(lyx::odocstream & os) : os_(os) {}
	///
	lyx::odocstream & os() { return os_; }
private:
	///
	lyx::odocstream & os_;
};


///
MapleStream & operator<<(MapleStream &, MathAtom const &);
///
MapleStream & operator<<(MapleStream &, MathArray const &);
///
MapleStream & operator<<(MapleStream &, char const *);
///
MapleStream & operator<<(MapleStream &, char);
///
MapleStream & operator<<(MapleStream &, int);


//
// Maxima
//


class MaximaStream {
public:
	///
	explicit MaximaStream(lyx::odocstream & os) : os_(os) {}
	///
	lyx::odocstream & os() { return os_; }
private:
	///
	lyx::odocstream & os_;
};


///
MaximaStream & operator<<(MaximaStream &, MathAtom const &);
///
MaximaStream & operator<<(MaximaStream &, MathArray const &);
///
MaximaStream & operator<<(MaximaStream &, char const *);
///
MaximaStream & operator<<(MaximaStream &, char);
///
MaximaStream & operator<<(MaximaStream &, int);


//
// Mathematica
//


class MathematicaStream {
public:
	///
	explicit MathematicaStream(lyx::odocstream & os) : os_(os) {}
	///
	lyx::odocstream & os() { return os_; }
private:
	///
	lyx::odocstream & os_;
};


///
MathematicaStream & operator<<(MathematicaStream &, MathAtom const &);
///
MathematicaStream & operator<<(MathematicaStream &, MathArray const &);
///
MathematicaStream & operator<<(MathematicaStream &, char const *);
///
MathematicaStream & operator<<(MathematicaStream &, char);
///
MathematicaStream & operator<<(MathematicaStream &, int);


//
// Octave
//


class OctaveStream {
public:
	///
	explicit OctaveStream(lyx::odocstream & os) : os_(os) {}
	///
	lyx::odocstream & os() { return os_; }
private:
	///
	lyx::odocstream & os_;
};

///
OctaveStream & operator<<(OctaveStream &, MathAtom const &);
///
OctaveStream & operator<<(OctaveStream &, MathArray const &);
///
OctaveStream & operator<<(OctaveStream &, char const *);
///
OctaveStream & operator<<(OctaveStream &, char);
///
OctaveStream & operator<<(OctaveStream &, int);



#endif
