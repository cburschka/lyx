// -*- C++ -*-
/**
 * \file InsetMathMacro.h
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
class InsetMathMacro : public InsetMathNest {
public:
	/// A macro can be built from an existing template
	InsetMathMacro(Buffer * buf, docstring const & name);
	///
	InsetMathMacro(InsetMathMacro const &);
	///
	InsetMathMacro & operator=(InsetMathMacro const &);
	///
	~InsetMathMacro();
	///
	virtual InsetMathMacro * asMacro() { return this; }
	///
	virtual InsetMathMacro const * asMacro() const { return this; }
	///
	marker_type marker(BufferView const *) const;
	/// If the macro is in normal edit mode, dissolve its contents in
	/// the row. Otherwise, just insert the inset.
	bool addToMathRow(MathRow &, MetricsInfo & mi) const;
	///
	void beforeMetrics() const;
	///
	void afterMetrics() const;
	///
	void beforeDraw(PainterInfo const &) const;
	///
	void afterDraw(PainterInfo const &) const;

	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	/// was the macro in edit mode when computing metrics?
	bool editMetrics(BufferView const * bv) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
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
	mode_type currentMode() const;

	/// Assumes that macros are up-to-date
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
	/// FIXME: Often dangling.
	MacroData const * macro() const;
	///
	docstring macroName() const;
	/// Level of nesting in macros (including this one)
	int nesting() const;
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
	/// This is not used for display; however whether it is mathrel determines
	/// how to split equations intelligently.
	MathClass mathClass() const; //override

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
	void updateRepresentation(Cursor * cur, MacroContext const & mc,
	                          UpdateType, int nesting);
	/// empty macro, put arguments into args, possibly strip arity-attachedArgsNum_ empty ones.
	/// Includes the optional arguments.
	void detachArguments(std::vector<MathData> & args, bool strip);
	/// attach arguments (maybe less than arity at the end of an MathData),
	/// including the optional ones (even if it can be empty here)
	void attachArguments(std::vector<MathData> const & args, size_t arity, int optionals);

private:
	/// This function is needed for now because of two shortfalls of the current
	/// implementation: the macro() pointer is often dangling, in which case we
	/// fall back to a backup copy, and the macro is not known at inset
	/// creation, in which case we fall back to the global macro with this name.
	MacroData const * macroBackup() const;
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
