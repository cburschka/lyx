// -*- C++ -*-
/**
 * \file InsetMath.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_INSET_H
#define MATH_INSET_H

#include "MathData.h"

#include "insets/Inset.h"


namespace lyx {

enum HullType {
	hullNone,
	hullSimple,
	hullEquation,
	hullEqnArray,
	hullAlign,
	hullAlignAt,
	hullXAlignAt,
	hullXXAlignAt,
	hullFlAlign,
	hullMultline,
	hullGather,
	hullRegexp,
	hullUnknown
};

HullType hullType(docstring const & name);
docstring hullName(HullType type);

/**

Abstract base class for all math objects.  A math insets is for use of the
math editor only, it isn't a general LyX inset. It's used to represent all
the math objects.

Math insets do not know their parents, a cursor position or things
like that. They are dumb objects that are contained in other math insets
(InsetMathNests, in fact) thus forming a tree. The root of this tree is
always a InsetMathHull, which provides an interface to the Outer World by
inclusion in the "real LyX insets" FormulaInset and FormulaMacroInset.

*/

class OutputParams;
class InsetMathArray;
class InsetMathAMSArray;
class InsetMathBrace;
class InsetMathChar;
class InsetMathDelim;
class InsetMathFracBase;
class InsetMathFrac;
class InsetMathFont;
class InsetMathGrid;
class InsetMathHull;
class InsetMathMatrix;
class InsetMathNest;
class InsetMathScript;
class InsetMathString;
class InsetMathSpace;
class InsetMathSpecialChar;
class InsetMathSymbol;
class InsetMathUnknown;

class InsetMathRef;

class HtmlStream;
class NormalStream;
class OctaveStream;
class MapleStream;
class MaximaStream;
class MathematicaStream;
class MathStream;
class WriteStream;

class MathMacroTemplate;
class MathMacro;
class Cursor;
class TextPainter;
class TextMetricsInfo;
class ReplaceData;

/// Type of unique identifiers for math insets (used in TexRow)
typedef void const * uid_type;


class InsetMath : public Inset {
public:
	///
	InsetMath(Buffer * buf = 0) : Inset(buf) {}
	/// identification as math inset
	InsetMath * asInsetMath() { return this; }
	/// identification as math inset
	InsetMath const * asInsetMath() const { return this; }
	/// this is overridden in math text insets (i.e. mbox)
	bool inMathed() const { return true; }
	///
	virtual docstring name() const;

	/// this is overridden by specific insets
	virtual mode_type currentMode() const { return MATH_MODE; }

	/// the ascent of the inset above the baseline
	/// compute the size of the object for text based drawing
	virtual void metricsT(TextMetricsInfo const & mi, Dimension & dim) const;
	/// draw the object as text
	virtual void drawT(TextPainter &, int x, int y) const;

	/// return cell given its number
	virtual MathData & cell(idx_type);
	/// return cell given its number
	virtual MathData const & cell(idx_type) const;

	/// identifies certain types of insets
	virtual InsetMathAMSArray       * asAMSArrayInset()       { return 0; }
	virtual InsetMathAMSArray const * asAMSArrayInset() const { return 0; }
	virtual InsetMathArray          * asArrayInset()          { return 0; }
	virtual InsetMathArray const    * asArrayInset() const    { return 0; }
	virtual InsetMathBrace          * asBraceInset()          { return 0; }
	virtual InsetMathBrace const    * asBraceInset() const    { return 0; }
	virtual InsetMathChar const     * asCharInset() const     { return 0; }
	virtual InsetMathDelim          * asDelimInset()          { return 0; }
	virtual InsetMathDelim const    * asDelimInset() const    { return 0; }
	virtual InsetMathFracBase       * asFracBaseInset()       { return 0; }
	virtual InsetMathFracBase const * asFracBaseInset() const { return 0; }
	virtual InsetMathFrac           * asFracInset()           { return 0; }
	virtual InsetMathFrac const     * asFracInset() const     { return 0; }
	virtual InsetMathFont           * asFontInset()           { return 0; }
	virtual InsetMathFont const     * asFontInset() const     { return 0; }
	virtual InsetMathGrid           * asGridInset()           { return 0; }
	virtual InsetMathGrid const     * asGridInset() const     { return 0; }
	virtual InsetMathHull           * asHullInset()           { return 0; }
	virtual InsetMathHull const     * asHullInset() const     { return 0; }
	virtual MathMacro               * asMacro()               { return 0; }
	virtual MathMacro const         * asMacro() const         { return 0; }
	virtual MathMacroTemplate       * asMacroTemplate()       { return 0; }
	virtual MathMacroTemplate const * asMacroTemplate() const { return 0; }
	virtual InsetMathMatrix const   * asMatrixInset() const   { return 0; }
	virtual InsetMathNest           * asNestInset()           { return 0; }
	virtual InsetMathNest const     * asNestInset() const     { return 0; }
	virtual InsetMathScript         * asScriptInset()         { return 0; }
	virtual InsetMathScript const   * asScriptInset() const   { return 0; }
	virtual InsetMathSpace          * asSpaceInset()          { return 0; }
	virtual InsetMathSpace const    * asSpaceInset() const    { return 0; }
	virtual InsetMathString         * asStringInset()         { return 0; }
	virtual InsetMathString const   * asStringInset() const   { return 0; }
	virtual InsetMathSymbol const   * asSymbolInset() const   { return 0; }
	virtual InsetMathUnknown        * asUnknownInset()        { return 0; }
	virtual InsetMathUnknown const  * asUnknownInset() const  { return 0; }
	virtual InsetMathRef            * asRefInset()            { return 0; }
	virtual InsetMathSpecialChar const * asSpecialCharInset() const { return 0; }

	/// identifies things that can get scripts
	virtual bool isScriptable() const { return false; }
	/// identifies a binary operators (used for spacing)
	virtual bool isMathBin() const { return false; }
	/// identifies relational operators (used for spacing and splitting equations)
	virtual bool isMathRel() const { return false; }
	/// identifies punctuation (used for spacing)
	virtual bool isMathPunct() const { return false; }
	/// will this get written as a single block in {..}
	virtual bool extraBraces() const { return false; }

	/// return the content as char if the inset is able to do so
	virtual char_type getChar() const { return 0; }
	/// identifies things that can get \limits or \nolimits
	virtual bool takesLimits() const { return false; }

	/// replace things by other things
	virtual void replace(ReplaceData &) {}
	/// do we contain a given subsequence?
	virtual bool contains(MathData const &) const { return false; }
	/// access to the lock (only nest array have one)
	virtual bool lock() const { return false; }
	/// access to the lock (only nest array have one)
	virtual void lock(bool) {}

	// Indicate that we do not want to hide the normal version of
	// write(). This is to shut off a clang warning.
	using Inset::write;
	/// write LaTeX and LyX code
	virtual void write(WriteStream & os) const;
	/// write normalized content
	virtual void normalize(NormalStream &) const;
	/// write content as something readable by Maple
	virtual void maple(MapleStream &) const;
	/// write content as something readable by Maxima
	virtual void maxima(MaximaStream &) const;
	/// write content as something readable by Mathematica
	virtual void mathematica(MathematicaStream &) const;
	/// write content as MathML
	virtual void mathmlize(MathStream &) const;
	/// write content as HTML, best we can.
	/// the idea for this, and some of the details, come from
	/// eLyXer, written by Alex Fernandez. no code is borrowed. rather,
	/// we try to mimic how eLyXer outputs some math.
	virtual void htmlize(HtmlStream &) const;
	/// write content as something readable by Octave
	virtual void octave(OctaveStream &) const;

	/// plain text output in ucs4 encoding
	int plaintext(odocstringstream &, OutputParams const &, size_t) const;

	/// dump content to stderr for debugging
	virtual void dump() const;

	/// LyXInset stuff
	virtual bool numberedType() const { return false; }
	/// hull type
	virtual HullType getType() const;
	/// change type
	virtual void mutate(HullType /*newtype*/) {}

	/// math stuff usually isn't allowed in text mode
	virtual bool allowedIn(mode_type mode) const { return mode == MATH_MODE; }

	/// superscript kerning
	virtual int kerning(BufferView const *) const { return 0; }
	///
	bool isInToc() const { return true; }
	///
	InsetCode lyxCode() const { return MATH_CODE; }
	///
	uid_type id() const { return this; }
};

///
std::ostream & operator<<(std::ostream &, MathAtom const &);
///
odocstream & operator<<(odocstream &, MathAtom const &);

// initialize math
void initMath();


} // namespace lyx

#endif
