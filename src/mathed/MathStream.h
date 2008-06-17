// -*- C++ -*-
/**
 * \file MathStream.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_MATHMLSTREAM_H
#define MATH_MATHMLSTREAM_H

#include "support/strfwd.h"

#include "InsetMath.h"
// FIXME: Move to individual insets
#include "MetricsInfo.h"


namespace lyx {

class MathData;
class InsetMath;
class MathAtom;

//
// LaTeX/LyX
//

class WriteStream {
public:
	///
	WriteStream(odocstream & os, bool fragile, bool latex, bool dryrun);
	///
	explicit WriteStream(odocstream & os);
	///
	~WriteStream();
	///
	int line() const { return line_; }
	///
	bool fragile() const { return fragile_; }
	///
	bool latex() const { return latex_; }
	///
	bool dryrun() const { return dryrun_; }
	///
	odocstream & os() { return os_; }
	///
	bool & firstitem() { return firstitem_; }
	///
	void addlines(unsigned int);
	/// writes space if next thing is isalpha()
	void pendingSpace(bool how);
	/// writes space if next thing is isalpha()
	bool pendingSpace() const { return pendingspace_; }
	/// tell whether to write the closing brace of \ensuremath
	void pendingBrace(bool brace);
	/// tell whether to write the closing brace of \ensuremath
	bool pendingBrace() const { return pendingbrace_; }
	/// tell whether we are in text mode or not when producing latex code
	void textMode(bool textmode);
	/// tell whether we are in text mode or not when producing latex code
	bool textMode() const { return textmode_; }
private:
	///
	odocstream & os_;
	/// do we have to write \\protect sometimes
	bool fragile_;
	/// are we at the beginning of an MathData?
	bool firstitem_;
	/// are we writing to .tex?
	int latex_;
	/// is it for preview?
	bool dryrun_;
	/// do we have a space pending?
	bool pendingspace_;
	/// do we have a brace pending?
	bool pendingbrace_;
	/// are we in text mode when producing latex code?
	bool textmode_;
	///
	int line_;
};

///
WriteStream & operator<<(WriteStream &, MathAtom const &);
///
WriteStream & operator<<(WriteStream &, MathData const &);
///
WriteStream & operator<<(WriteStream &, docstring const &);
///
WriteStream & operator<<(WriteStream &, char const * const);
///
WriteStream & operator<<(WriteStream &, char);
///
WriteStream & operator<<(WriteStream &, int);
///
WriteStream & operator<<(WriteStream &, unsigned int);

/// ensure math mode, possibly by opening \ensuremath
bool ensureMath(WriteStream & os, bool needs_math_mode = true, bool macro = false);

/// ensure the requested mode, possibly by closing \ensuremath
bool ensureMode(WriteStream & os, InsetMath::mode_type mode);


/**
 * MathEnsurer - utility class for ensuring math mode
 *
 * A local variable of this type can be used to either ensure math mode
 * or delay the writing of a pending brace when outputting LaTeX.
 *
 * Example 1:
 *
 *      MathEnsurer ensurer(os);
 *
 * If not already in math mode, inserts an \ensuremath command followed
 * by an open brace. This brace will be automatically closed when exiting
 * math mode. Math mode is automatically exited when writing something
 * that doesn't explicitly require math mode.
 *
 * Example 2:
 *
 *      MathEnsurer ensurer(os, false);
 *
 * Simply suspend writing a closing brace until the end of ensurer's scope.
 *
 * Example 3:
 *
 *      MathEnsurer ensurer(os, needs_math_mode, true);
 *
 * The third parameter is set to true only for a user defined macro, which
 * needs special handling. When it is a MathMacro, the needs_math_mode
 * parameter is true and the behavior is as in Example 1. When the
 * needs_math_mode parameter is false (not a MathMacro) and the macro
 * was entered in a text box and we are in math mode, the mode is reset
 * to text. This is because the macro was probably designed for text mode
 * (given that it was entered in text mode and we have no way to tell the
 * contrary).
 */
class MathEnsurer
{
public:
	///
	explicit MathEnsurer(WriteStream & os, bool needs_math_mode = true, bool macro = false)
		: os_(os), brace_(ensureMath(os, needs_math_mode, macro)) {}
	///
	~MathEnsurer() { os_.pendingBrace(brace_); }
private:
	///
	WriteStream & os_;
	///
	bool brace_;
};


/**
 * ModeSpecifier - utility class for specifying a given mode (math or text)
 *
 * A local variable of this type can be used to specify that a command or
 * environment works in a given mode. For example, \mbox works in text
 * mode, but \boxed works in math mode. Note that no mode changing commands
 * are needed, but we have to track the current mode, hence this class.
 *
 * Example:
 *
 *      ModeSpecifier specifier(os, TEXT_MODE);
 *
 * Sets the current mode to text mode.
 *
 * At the end of specifier's scope the mode is reset to its previous value.
 */
class ModeSpecifier
{
public:
	///
	explicit ModeSpecifier(WriteStream & os, InsetMath::mode_type mode)
		: os_(os), textmode_(ensureMode(os, mode)) {}
	///
	~ModeSpecifier() { os_.textMode(textmode_); }
private:
	///
	WriteStream & os_;
	///
	bool textmode_;
};



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

class MathStream {
public:
	///
	explicit MathStream(odocstream & os);
	///
	void cr();
	///
	odocstream & os() { return os_; }
	///
	int line() const { return line_; }
	///
	int & tab() { return tab_; }
	///
	friend MathStream & operator<<(MathStream &, char const *);
private:
	///
	odocstream & os_;
	///
	int tab_;
	///
	int line_;
	///
	char lastchar_;
};

///
MathStream & operator<<(MathStream &, MathAtom const &);
///
MathStream & operator<<(MathStream &, MathData const &);
///
MathStream & operator<<(MathStream &, docstring const &);
///
MathStream & operator<<(MathStream &, char const *);
///
MathStream & operator<<(MathStream &, char);
///
MathStream & operator<<(MathStream &, MTag const &);
///
MathStream & operator<<(MathStream &, ETag const &);



//
// Debugging
//

class NormalStream {
public:
	///
	explicit NormalStream(odocstream & os) : os_(os) {}
	///
	odocstream & os() { return os_; }
private:
	///
	odocstream & os_;
};

///
NormalStream & operator<<(NormalStream &, MathAtom const &);
///
NormalStream & operator<<(NormalStream &, MathData const &);
///
NormalStream & operator<<(NormalStream &, docstring const &);
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
	explicit MapleStream(odocstream & os) : os_(os) {}
	///
	odocstream & os() { return os_; }
private:
	///
	odocstream & os_;
};


///
MapleStream & operator<<(MapleStream &, MathAtom const &);
///
MapleStream & operator<<(MapleStream &, MathData const &);
///
MapleStream & operator<<(MapleStream &, docstring const &);
///
MapleStream & operator<<(MapleStream &, char_type);
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
	explicit MaximaStream(odocstream & os) : os_(os) {}
	///
	odocstream & os() { return os_; }
private:
	///
	odocstream & os_;
};


///
MaximaStream & operator<<(MaximaStream &, MathAtom const &);
///
MaximaStream & operator<<(MaximaStream &, MathData const &);
///
MaximaStream & operator<<(MaximaStream &, docstring const &);
///
MaximaStream & operator<<(MaximaStream &, char_type);
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
	explicit MathematicaStream(odocstream & os) : os_(os) {}
	///
	odocstream & os() { return os_; }
private:
	///
	odocstream & os_;
};


///
MathematicaStream & operator<<(MathematicaStream &, MathAtom const &);
///
MathematicaStream & operator<<(MathematicaStream &, MathData const &);
///
MathematicaStream & operator<<(MathematicaStream &, docstring const &);
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
	explicit OctaveStream(odocstream & os) : os_(os) {}
	///
	odocstream & os() { return os_; }
private:
	///
	odocstream & os_;
};

///
OctaveStream & operator<<(OctaveStream &, MathAtom const &);
///
OctaveStream & operator<<(OctaveStream &, MathData const &);
///
OctaveStream & operator<<(OctaveStream &, docstring const &);
///
OctaveStream & operator<<(OctaveStream &, char_type);
///
OctaveStream & operator<<(OctaveStream &, char const *);
///
OctaveStream & operator<<(OctaveStream &, char);
///
OctaveStream & operator<<(OctaveStream &, int);

} // namespace lyx

#endif
