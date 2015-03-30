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
	DisplayMode displayMode() const { return displayMode_; }

	///
	bool extraBraces() const { return displayMode_ == DISPLAY_NORMAL && arity() > 0; }

	///
	docstring name() const;
	///
	bool validName() const;
	///
	size_t arity() const {
		if (displayMode_ == DISPLAY_NORMAL )
			return cells_.size();
		else
			return 0;
	}

	///
	size_t optionals() const { return optionals_; }
	///
	void setOptionals(int n) {
		if (n <= int(nargs()))
			optionals_ = n;
	}
	/// Return the maximal number of arguments the macro is greedy for.
	size_t appetite() const { return appetite_; }
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
	MacroData const * macro() { return macro_; }
	///
	bool editMetrics(BufferView const * bv) const;

private:
	///
	virtual Inset * clone() const;
	///
	bool editMode(BufferView const * bv) const;

	/// name of macro
	docstring name_;
	/// current display mode
	DisplayMode displayMode_;
	/// expanded macro with ArgumentProxies
	MathData expanded_;
	/// macro definition with #1,#2,.. insets
	MathData definition_;
	/// number of arguments that were really attached
	size_t attachedArgsNum_;
	/// optional argument attached? (only in DISPLAY_NORMAL mode)
	size_t optionals_;
	/// fold mode to be set in next metrics call?
	bool nextFoldMode_;
	/// if macro_ == true, then here is a copy of the macro
	/// don't use it for locking
	MacroData macroBackup_;
	/// if macroNotFound_ == false, then here is a reference to the macro
	/// this might invalidate after metrics was called
	MacroData const * macro_;
	///
	mutable std::map<BufferView const *, bool> editing_;
	///
	std::string requires_;
	/// update macro representation
	bool needsUpdate_;
	/// update lock to avoid loops
	class UpdateLocker;
	friend class UpdateLocker;
	bool isUpdating_;
	/// maximal number of arguments the macro is greedy for
	size_t appetite_;

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
