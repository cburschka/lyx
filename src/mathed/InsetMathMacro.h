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
	InsetMathMacro * asMacro() override { return this; }
	///
	InsetMathMacro const * asMacro() const override { return this; }
	///
	marker_type marker(BufferView const *) const override;
	/// If the macro is in normal edit mode, dissolve its contents in
	/// the row. Otherwise, just insert the inset.
	bool addToMathRow(MathRow &, MetricsInfo & mi) const override;

	/// Whether the inset allows \(no)limits
	bool allowsLimitsChange() const;
	/// The default limits value
	Limits defaultLimits(bool display) const override;
	/// whether the inset has limit-like sub/superscript
	Limits limits() const override;
	/// sets types of sub/superscripts
	void limits(Limits lim) override;

	///
	void beforeMetrics() const override;
	///
	void afterMetrics() const override;
	///
	void beforeDraw(PainterInfo const &) const override;
	///
	void afterDraw(PainterInfo const &) const override;

	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	/// was the macro in edit mode when computing metrics?
	bool editMetrics(BufferView const * bv) const;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	int kerning(BufferView const * bv) const override;
	/// get cursor position
	void cursorPos(BufferView const & bv, CursorSlice const & sl,
		bool boundary, int & x, int & y) const override;
	///
	void edit(Cursor & cur, bool front, EntryDirection entry_from) override;
	///
	Inset * editXY(Cursor & cur, int x, int y) override;

	/// target pos when we enter the inset while moving forward
	bool idxFirst(Cursor &) const override;
	/// target pos when we enter the inset while moving backwards
	bool idxLast(Cursor &) const override;

	///
	bool notifyCursorLeaves(Cursor const & old, Cursor & cur) override;

	/// Remove cell (starting from 0)
	void removeArgument(pos_type pos);
	/// Insert empty cell (starting from 0)
	void insertArgument(pos_type pos);

	///
	void validate(LaTeXFeatures &) const override;
	///
	mode_type currentMode() const override;

	/// Assumes that macros are up-to-date
	void write(WriteStream & os) const override;
	///
	void normalize(NormalStream & os) const override;
	///
	void maple(MapleStream &) const override;
	///
	void maxima(MaximaStream &) const override;
	///
	void mathematica(MathematicaStream &) const override;
	///
	void mathmlize(MathStream &) const override;
	///
	void htmlize(HtmlStream &) const override;
	///
	void octave(OctaveStream &) const override;
	///
	void infoize(odocstream &) const override;
	///
	void infoize2(odocstream &) const override;

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
	bool extraBraces() const override;

	///
	docstring name() const override;
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
	InsetCode lyxCode() const override { return MATH_MACRO_CODE; }
	/// This is not used for display; however whether it is mathrel determines
	/// how to split equations intelligently.
	MathClass mathClass() const override;
	/// Override so as to set Buffer for definition_ member, too.
	void setBuffer(Buffer &) override;

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
	Inset * clone() const override;
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
	bool completionSupported(Cursor const &) const override;
	///
	bool inlineCompletionSupported(Cursor const & cur) const override;
	///
	bool automaticInlineCompletion() const override;
	///
	bool automaticPopupCompletion() const override;
	///
	CompletionList const * createCompletionList(Cursor const & cur) const override;
	///
	docstring completionPrefix(Cursor const & cur) const override;
	///
	bool insertCompletion(Cursor & cur, docstring const & s, bool finished) override;
	///
	void completionPosAndDim(Cursor const &, int & x, int & y, Dimension & dim) const override;
};

} // namespace lyx
#endif
