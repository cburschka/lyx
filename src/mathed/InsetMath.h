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

#include "MathClass.h"

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

class Cursor;
class OutputParams;
class MetricsInfo;

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
class InsetMathSubstack;
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

class MathData;
class InsetMathMacroTemplate;
class InsetMathMacro;
class MathRow;
class TextPainter;
class TextMetricsInfo;
class ReplaceData;

/// Type of unique identifiers for math insets (used in TexRow)
typedef void const * uid_type;


class InsetMath : public Inset {
public:
	///
	explicit InsetMath(Buffer * buf = 0) : Inset(buf) {}
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

	// The possible marker types for math insets
	enum marker_type { NO_MARKER, MARKER2, MARKER, BOX_MARKER };
	/// this is overridden by insets with specific edit marker type
	virtual marker_type marker(BufferView const *) const;

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
	virtual InsetMathAMSArray       * asAMSArrayInset()       { return nullptr; }
	virtual InsetMathAMSArray const * asAMSArrayInset() const { return nullptr; }
	virtual InsetMathArray          * asArrayInset()          { return nullptr; }
	virtual InsetMathArray const    * asArrayInset() const    { return nullptr; }
	virtual InsetMathBrace          * asBraceInset()          { return nullptr; }
	virtual InsetMathBrace const    * asBraceInset() const    { return nullptr; }
	virtual InsetMathChar const     * asCharInset() const     { return nullptr; }
	virtual InsetMathDelim          * asDelimInset()          { return nullptr; }
	virtual InsetMathDelim const    * asDelimInset() const    { return nullptr; }
	virtual InsetMathFracBase       * asFracBaseInset()       { return nullptr; }
	virtual InsetMathFracBase const * asFracBaseInset() const { return nullptr; }
	virtual InsetMathFrac           * asFracInset()           { return nullptr; }
	virtual InsetMathFrac const     * asFracInset() const     { return nullptr; }
	virtual InsetMathFont           * asFontInset()           { return nullptr; }
	virtual InsetMathFont const     * asFontInset() const     { return nullptr; }
	virtual InsetMathGrid           * asGridInset()           { return nullptr; }
	virtual InsetMathGrid const     * asGridInset() const     { return nullptr; }
	virtual InsetMathHull           * asHullInset()           { return nullptr; }
	virtual InsetMathHull const     * asHullInset() const     { return nullptr; }
	virtual InsetMathMacro               * asMacro()               { return nullptr; }
	virtual InsetMathMacro const         * asMacro() const         { return nullptr; }
	virtual InsetMathMacroTemplate       * asMacroTemplate()       { return nullptr; }
	virtual InsetMathMacroTemplate const * asMacroTemplate() const { return nullptr; }
	virtual InsetMathMatrix const   * asMatrixInset() const   { return nullptr; }
	virtual InsetMathNest           * asNestInset()           { return nullptr; }
	virtual InsetMathNest const     * asNestInset() const     { return nullptr; }
	virtual InsetMathScript         * asScriptInset()         { return nullptr; }
	virtual InsetMathScript const   * asScriptInset() const   { return nullptr; }
	virtual InsetMathSpace          * asSpaceInset()          { return nullptr; }
	virtual InsetMathSpace const    * asSpaceInset() const    { return nullptr; }
	virtual InsetMathString         * asStringInset()         { return nullptr; }
	virtual InsetMathString const   * asStringInset() const   { return nullptr; }
	virtual InsetMathSubstack       * asSubstackInset()       { return nullptr; }
	virtual InsetMathSubstack const * asSubstackInset() const { return nullptr; }
	virtual InsetMathSymbol const   * asSymbolInset() const   { return nullptr; }
	virtual InsetMathUnknown        * asUnknownInset()        { return nullptr; }
	virtual InsetMathUnknown const  * asUnknownInset() const  { return nullptr; }
	virtual InsetMathRef            * asRefInset()            { return nullptr; }
	virtual InsetMathSpecialChar const * asSpecialCharInset() const { return nullptr; }

	/// The class of the math object (used primarily for spacing)
	virtual MathClass mathClass() const;
	/// Add this inset to a math row. Return true if contents got added
	virtual bool addToMathRow(MathRow &, MetricsInfo & mi) const;
	/// Hook that is run before metrics computation starts
	virtual void beforeMetrics() const {}
	/// Hook that is run after metrics computation
	virtual void afterMetrics() const {}
	/// Hook that is run before actual drawing
	virtual void beforeDraw(PainterInfo const &) const {}
	/// Hook that is run after drawing
	virtual void afterDraw(PainterInfo const &) const {}

	/// identifies things that can get scripts
	virtual bool isScriptable() const { return false; }
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

	/// Italic correction as described in InsetMathScript.h
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
