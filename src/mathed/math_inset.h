// -*- C++ -*-
/**
 * \file math_inset.h
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

#include "insets/insetbase.h"

#include <string>

/**

Abstract base class for all math objects.  A math insets is for use of the
math editor only, it isn't a general LyX inset. It's used to represent all
the math objects.

Math insets do not know there parents, a cursor position or things
like that. The are dumb object that are contained in other math insets
(MathNestInsets, in fact) thus forming a tree. The root of this tree is
always a MathHullInset, which provides an interface to the Outer World by
inclusion in the "real LyX insets" FormulaInset and FormulaMacroInset.

*/

class OutputParams;
class MathArrayInset;
class MathAMSArrayInset;
class MathCharInset;
class MathDelimInset;
class MathFracInset;
class MathFontInset;
class MathGridInset;
class MathHullInset;
class MathMatrixInset;
class MathNestInset;
class MathParboxInset;
class MathScriptInset;
class MathStringInset;
class MathSpaceInset;
class MathSymbolInset;
class MathUnknownInset;

class RefInset;

class MathArray;
class MathAtom;

class NormalStream;
class OctaveStream;
class MapleStream;
class MaximaStream;
class MathematicaStream;
class MathMLStream;
class WriteStream;
class InfoStream;

class MathMacroTemplate;
class MathMacro;
class MathPosFinder;
class Dimension;
class LCursor;
class TextPainter;
class TextMetricsInfo;
class ReplaceData;


class MathInset : public InsetBase {
public:
	/// identification as math inset
	MathInset * asMathInset() { return this; }
	/// this is overridden in math text insets (i.e. mbox)
	bool inMathed() const { return true; }

	/// substitutes macro arguments if necessary
	virtual void substitute(MathMacro const & macro);
	/// the ascent of the inset above the baseline
	/// compute the size of the object for text based drawing
	virtual void metricsT(TextMetricsInfo const & mi, Dimension & dim) const;
	/// draw the object as text
	virtual void drawT(TextPainter &, int x, int y) const;

	/// return cell given its number
	virtual MathArray & cell(idx_type);
	/// return cell given its number
	virtual MathArray const & cell(idx_type) const;

	/// identifies certain types of insets
	virtual MathAMSArrayInset       * asAMSArrayInset()       { return 0; }
	virtual MathAMSArrayInset const * asAMSArrayInset() const { return 0; }
	virtual MathArrayInset          * asArrayInset()          { return 0; }
	virtual MathArrayInset const    * asArrayInset() const    { return 0; }
	virtual MathCharInset const     * asCharInset() const     { return 0; }
	virtual MathDelimInset          * asDelimInset()          { return 0; }
	virtual MathDelimInset const    * asDelimInset() const    { return 0; }
	virtual MathFracInset           * asFracInset()           { return 0; }
	virtual MathFracInset const     * asFracInset() const     { return 0; }
	virtual MathFontInset           * asFontInset()           { return 0; }
	virtual MathFontInset const     * asFontInset() const     { return 0; }
	virtual MathGridInset           * asGridInset()           { return 0; }
	virtual MathGridInset const     * asGridInset() const     { return 0; }
	virtual MathHullInset           * asHullInset()           { return 0; }
	virtual MathHullInset const     * asHullInset() const     { return 0; }
	virtual MathMacroTemplate       * asMacroTemplate()       { return 0; }
	virtual MathMacroTemplate const * asMacroTemplate() const { return 0; }
	virtual MathMatrixInset const   * asMatrixInset() const   { return 0; }
	virtual MathNestInset           * asNestInset()           { return 0; }
	virtual MathNestInset const     * asNestInset() const     { return 0; }
	virtual MathParboxInset         * asParboxInset()         { return 0; }
	virtual MathScriptInset         * asScriptInset()         { return 0; }
	virtual MathScriptInset const   * asScriptInset() const   { return 0; }
	virtual MathSpaceInset          * asSpaceInset()          { return 0; }
	virtual MathSpaceInset const    * asSpaceInset() const    { return 0; }
	virtual MathStringInset         * asStringInset()         { return 0; }
	virtual MathStringInset const   * asStringInset() const   { return 0; }
	virtual MathSymbolInset const   * asSymbolInset() const   { return 0; }
	virtual MathUnknownInset        * asUnknownInset()        { return 0; }
	virtual MathUnknownInset const  * asUnknownInset() const  { return 0; }
	virtual RefInset                * asRefInset()            { return 0; }

	/// identifies things that can get scripts
	virtual bool isScriptable() const { return false; }
	/// is the a relational operator (used for splitting equations)
	virtual bool isRelOp() const { return false; }
	/// will this get written as a single block in {..}
	virtual bool extraBraces() const { return false; }

	/// return the content as char if the inset is able to do so
	virtual char getChar() const { return 0; }
	/// identifies things that can get \limits or \nolimits
	virtual bool takesLimits() const { return false; }

	/// char char code if possible
	virtual void handleFont(std::string const &) {}
	/// replace things by other things
	virtual void replace(ReplaceData &) {}
	/// do we contain a given subsequence?
	virtual bool contains(MathArray const &) const { return false; }
	/// access to the lock (only nest array have one)
	virtual bool lock() const { return false; }
	/// access to the lock (only nest array have one)
	virtual void lock(bool) {}

	/// write LaTeX and Lyx code
	virtual void write(WriteStream & os) const;
	/// write normalized content
	virtual void normalize(NormalStream &) const;
	/// write content as something readable by Maple
	virtual void maple(MapleStream &) const;
	/// write content as something readable by Maxima
	virtual void maxima(MaximaStream &) const;
	/// write content as something readable by Mathematica
	virtual void mathematica(MathematicaStream &) const;
	/// write content as something resembling MathML
	virtual void mathmlize(MathMLStream &) const;
	/// write content as something readable by Octave
	virtual void octave(OctaveStream &) const;

	/// dump content to stderr for debugging
	virtual void dump() const;

	/// LyXInset stuff
	virtual bool numberedType() const { return false; }
	/// hull type
	virtual std::string const & getType() const;
	/// change type
	virtual void mutate(std::string const &) {}
	/// usually the latex name
	virtual std::string name() const;
};

std::ostream & operator<<(std::ostream &, MathAtom const &);

// initialize math
void initMath();

#endif
