// -*- C++ -*-

#ifndef MATH_ATOM_H
#define MATH_ATOM_H

#include <config.h>
#include <iosfwd>

#ifdef __GNUG__
#pragma interface
#endif

#include "math_defs.h"

/** 
The 'atom' is the major blob in math typesetting.  And 'atom' consists
of a nucleus, an optional superscript, and an optional subscript.

Exactly where the subscript and superscript are drawn depends on the
size, and type, of the nucleus they are attached to.  

Jules
*/

class LaTeXFeatures;
class MathCharInset;
class MathScriptInset;
class MathInset;
class MathMacro;
class MathArray;
class Painter;

class MathAtom {
public: 
	///
	MathAtom();
	///
	MathAtom(MathAtom const &);
	///
	explicit MathAtom(MathInset * p);
	///
	MathAtom(MathInset * p, MathScriptInset * up, MathScriptInset * down);
	/// 
	virtual ~MathAtom(); 
	///
	void operator=(MathAtom const &);
	///
	void swap(MathAtom &);

	/// draw the object, sets xo_ and yo_ cached values 
	virtual void draw(Painter &, int x, int y) const;
	/// reproduce itself
	void metrics(MathStyles st) const;
	/// 
	int ascent() const;
	///
	int descent() const;
	///
	int width() const;
	///
	int height() const;

	///
	int xo() const;
	///
	int yo() const;
	///
	void xo(int tx) const;
	///
	void yo(int ty) const;
	///

	///
	void getXY(int & x, int & y) const;
	///
	bool covers(int x, int y) const;

	///
	void dump() const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	void handleFont(MathTextCodes) {}

	/// make sure superscript is available
	MathScriptInset * ensure(bool up);
	/// delete subscript array if empty
	void removeEmptyScripts();
	/// can we add a super- or subscript?
	virtual bool allows(bool up) const { return script_[up] == 0; }
	/// can we add a super- or subscript?
	virtual bool allowsLimits() const { return true; }
	/// set limits
	void limits(int lim) { limits_ = lim; }
	/// 
	int limits() const { return limits_; }
	///
	bool hasLimits() const;
	/// returns superscript
	MathScriptInset * up() const;
	/// returns subscript
	MathScriptInset * down() const;
	/// returns superscript
	MathScriptInset * & up();
	/// returns subscript
	MathScriptInset * & down();
	///
	MathInset * nucleus() const { return nucleus_; }
	///
	void substitute(const MathMacro &);
	///
	void write(std::ostream &, bool) const;
	///
	void writeNormal(std::ostream &) const;

protected:
	/// possible subscript (index 0) and superscript (index 1)
	MathScriptInset * script_[2]; 
	///
	MathInset * nucleus_;
	///
	int limits_;

private:
	/// the following are used for positioning the cursor with the mouse
	/// cached cursor start position in pixels from the document left
	mutable int xo_;
	/// cached cursor start position in pixels from the document top
	mutable int yo_;

	/// raw copy
	void copy(MathAtom const & p);
	/// raw destruction
	void done();

	/// returns y offset for superscript
	int dy0() const;
	/// returns y offset for subscript
	int dy1() const;
	/// returns x offset for superscript
	int dx0() const;
	/// returns x offset for subscript
	int dx1() const;
	/// returns x offset for main part
	int dxx() const;
	/// returns width of nucleus if any
	int nwid() const;
	/// returns ascent of nucleus if any
	int nasc() const;
	/// returns descent of nucleus if any
	int ndes() const;
};

std::ostream & operator<<(std::ostream &, MathAtom const &);

#endif
