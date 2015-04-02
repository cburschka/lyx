// -*- C++ -*-
/**
 * \file MathMacro.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_MACRO_H
#define MATH_MACRO_H

#include "InsetMathNest.h"
#include "MacroTable.h"
#include "MathData.h"

#include <map>

namespace lyx {

/// This class contains the data for a macro.
class MathMacro : public InsetMathNest {
public:
	/// A macro can be built from an existing template
	MathMacro(Buffer * buf, docstring const & name);
	///
	MathMacro(MathMacro const &);
	///
	MathMacro & operator=(MathMacro const &);
	///
	~MathMacro();
	///
	virtual MathMacro * asMacro() { return this; }
	///
	virtual MathMacro const * asMacro() const { return this; }
	///
	void draw(PainterInfo & pi, int x, int y) const;
	/// draw selection background
	void drawSelection(PainterInfo & pi, int x, int y) const;
	/// draw decorations.
	void drawDecoration(PainterInfo & pi, int x, int y) const
	{ drawMarkers2(pi, x, y); }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	int kerning(BufferView const * bv) const;
	/// get cursor position
	void cursorPos(BufferView const & bv, CursorSlice const & sl,
		bool boundary, int & x, int & y) const;
	///
	void edit(Cursor & cur, bool front, EntryDirection entry_from);
	///
	Inset * editXY(Cursor & cur, int x, int y);

	/// target pos when we enter the inset while moving forward
	bool idxFirst(Cursor &) const;
	/// target pos when we enter the inset while moving backwards
	bool idxLast(Cursor &) const;

	///
	virtual bool notifyCursorLeaves(Cursor const & old, Cursor & cur);

	/// Remove cell (starting from 0)
	void removeArgument(pos_type pos);
	/// Insert empty cell (starting from 0)
	void insertArgument(pos_type pos);

	///
	void validate(LaTeXFeatures &) const;

	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream & os) const;
	///
	void maple(MapleStream &) const;
	///
	void maxima(MaximaStream &) const;
	///
	void mathematica(MathematicaStream &) const;
	///
	void mathmlize(MathStream &) const;
	///
	void htmlize(HtmlStream &) const;
	///
	void octave(OctaveStream &) const;
	///
	void infoize(odocstream &) const;
	///
	void infoize2(odocstream &) const;

	/// fold the macro in the next metrics call
	void fold(Cursor & cur);
	/// unfold the macro in the next metrics call
	void unfold(Cursor & cur);
	/// will it be folded or unfolded in the next metric call?
	bool folded() const;

	enum DisplayMode {
		DISPLAY_INIT,
		DISPLAY_INTERACTIVE_INIT,
		DISPLAY_UNFOLDED,
		DISPLAY_NORMAL
	};

	///
	DisplayMode displayMode() const;

	///
	bool extraBraces() const;

	///
	docstring name() const;
	///
	docstring macroName() const;
	///
	bool validName() const;
	///
	size_t arity() const;

	///
	size_t optionals() const;
	///
	void setOptionals(int n);
	/// Return the maximal number of arguments the macro is greedy for.
	size_t appetite() const;
	///
	InsetCode lyxCode() const { return MATH_MACRO_CODE; }

protected:
	friend class MathData;
	friend class ArgumentProxy;
	friend class Cursor;

	/// update the display mode (should only be called after detaching arguments)
	void setDisplayMode(DisplayMode mode, int appetite = -1);
	/// compute the next display mode
	DisplayMode computeDisplayMode() const;
	/// update macro definition
	void updateMacro(MacroContext const & mc);
	/// check if macro definition changed, argument changed etc. and adapt
	void updateRepresentation(Cursor * cur, MacroContext const & mc, UpdateType);
	/// empty macro, put arguments into args, possibly strip arity-attachedArgsNum_ empty ones.
	/// Includes the optional arguments.
	void detachArguments(std::vector<MathData> & args, bool strip);
	/// attach arguments (maybe less than arity at the end of an MathData),
	/// including the optional ones (even if it can be empty here)
	void attachArguments(std::vector<MathData> const & args, size_t arity, int optionals);
	///
	MacroData const * macro();
	///
	bool editMetrics(BufferView const * bv) const;

private:
	///
	virtual Inset * clone() const;
	///
	bool editMode(BufferView const * bv) const;

	///
	class Private;
	///
	Private * d;
	/// update lock to avoid loops
	class UpdateLocker;
	friend class UpdateLocker;

public:
	///
	bool completionSupported(Cursor const &) const;
	///
	bool inlineCompletionSupported(Cursor const & cur) const;
	///
	bool automaticInlineCompletion() const;
	///
	bool automaticPopupCompletion() const;
	///
	CompletionList const * createCompletionList(Cursor const & cur) const;
	///
	docstring completionPrefix(Cursor const & cur) const;
	///
	bool insertCompletion(Cursor & cur, docstring const & s, bool finished);
	///
	void completionPosAndDim(Cursor const &, int & x, int & y, Dimension & dim) const;
};

} // namespace lyx
#endif
