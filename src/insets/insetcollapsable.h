// -*- C++ -*-
/**
 * \file insetcollapsable.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETCOLLAPSABLE_H
#define INSETCOLLAPSABLE_H

#include "inset.h"
#include "insettext.h"

#include "box.h"
#include "lyxfont.h"

class Painter;
class LyXText;
class Paragraph;
class LyXCursor;

/** A collapsable text inset

*/
class InsetCollapsable : public UpdatableInset {
public:
	///
	static int const TEXT_TO_TOP_OFFSET = 2;
	///
	static int const TEXT_TO_BOTTOM_OFFSET = 2;
	/// inset is initially collapsed if bool = true
	InsetCollapsable(BufferParams const &, bool collapsed = false);
	///
	InsetCollapsable(InsetCollapsable const & in);
	///
	void read(Buffer const &, LyXLex &);
	///
	void write(Buffer const &, std::ostream &) const;
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	/// draw, either inlined (no button) or collapsed/open
	void draw(PainterInfo & pi, int x, int y, bool inlined) const;
	///
	bool hitButton(FuncRequest const &) const;
	///
	EDITABLE editable() const;
	///
	bool insertInset(BufferView *, InsetOld * inset);
	///
	virtual bool insetAllowed(InsetOld::Code code) const;
	///
	bool isTextInset() const;
	///
	void insetUnlock(BufferView *);
	///
	bool lockInsetInInset(BufferView *, UpdatableInset *);
	///
	bool unlockInsetInInset(BufferView *, UpdatableInset *,
				bool lr = false);
	///
	int insetInInsetY() const;
	///
	int latex(Buffer const &, std::ostream &,
		  LatexRunParams const &) const;
	///
	int ascii(Buffer const &, std::ostream &,
		  LatexRunParams const &) const;
	///
	int linuxdoc(Buffer const &, std::ostream &,
		     LatexRunParams const &) const;
	///
	int docbook(Buffer const &, std::ostream &,
		    LatexRunParams const & runparams) const;
	///
	void validate(LaTeXFeatures & features) const;
	/// FIXME, document
	void getCursorPos(BufferView *, int & x, int & y) const;
	/// Get the absolute document x,y of the cursor
	virtual void getCursor(BufferView &, int &, int &) const;
	///
	void fitInsetCursor(BufferView * bv) const;
	///
	UpdatableInset * getLockingInset() const;
	///
	UpdatableInset * getFirstLockingInsetOfType(InsetOld::Code);
	///
	void setFont(BufferView *, LyXFont const &, bool toggleall = false,
		 bool selectall = false);
	///
	void setLabel(std::string const & l) const;
	///
	void setLabelFont(LyXFont & f);
#if 0
	///
	void setAutoCollapse(bool f);
#endif
	///
	LyXText * getLyXText(BufferView const *, bool const recursive) const;
	///
	void deleteLyXText(BufferView *, bool recursive=true) const;
	/// Appends \c list with all labels found within this inset.
	void getLabelList(Buffer const &, std::vector<std::string> & list) const;
	///
	int scroll(bool recursive=true) const;
	///
	void scroll(BufferView *bv, float sx) const;
	///
	void scroll(BufferView *bv, int offset) const;
	///
	InsetOld * getInsetFromID(int id) const;
	///
	ParagraphList * getParagraphs(int) const;
	///
	int numParagraphs() const;
	///
	LyXText * getText(int) const;
	///
	LyXCursor const & cursor(BufferView *) const;
	///
	virtual bool display() const { return isOpen(); }
	///
	bool isOpen() const;
	///
	void open(BufferView *);
	///
	void close(BufferView *) const;
	///
	bool allowSpellcheck() const;
	///
	WordLangTuple const
	selectNextWordToSpellcheck(BufferView *, float &) const;
	///
	void selectSelectedWord(BufferView *);

	void markErased();

	bool nextChange(BufferView * bv, lyx::pos_type & length);

	///
	bool searchForward(BufferView * bv, std::string const & str,
			   bool = true, bool = false);
	bool searchBackward(BufferView * bv, std::string const & str,
			    bool = true, bool = false);

	///
	void addPreview(lyx::graphics::PreviewLoader &) const;

protected:
	///
	virtual
	DispatchResult
	priv_dispatch(FuncRequest const &, idx_type &, pos_type &);
	///
	void dimension_collapsed(Dimension &) const;
	///
	int height_collapsed() const;
	///
	void draw_collapsed(PainterInfo & pi, int x, int y) const;
	///
	int getMaxTextWidth(Painter & pain, UpdatableInset const *) const;
	/// Should be non-const...
	void setCollapsed(bool) const;
	///
	Box const & buttonDim() const;

private:
	///
	void lfunMouseRelease(FuncRequest const &);
	///
	FuncRequest adjustCommand(FuncRequest const &);
	///
	void edit(BufferView *, int index);

public:
	///
	mutable InsetText inset;
private:
	///
	mutable bool collapsed_;
	///
	LyXFont labelfont_;
	///
	mutable Box button_dim;
	///
	mutable int topx;
	mutable int topbaseline;

	///
	mutable std::string label;
#if 0
	///
	bool autocollapse;
#endif
	///
	bool in_update;
	///
	mutable bool first_after_edit;
};


inline
bool InsetCollapsable::insetAllowed(InsetOld::Code code) const
{
	return inset.insetAllowed(code);
}


inline
bool InsetCollapsable::isTextInset() const
{
	return true;
}


inline
void InsetCollapsable::fitInsetCursor(BufferView * bv) const
{
	inset.fitInsetCursor(bv);
}

inline
void InsetCollapsable::setLabelFont(LyXFont & f)
{
	labelfont_ = f;
}

#if 0
inline
void InsetCollapsable::setAutoCollapse(bool f)
{
	autocollapse = f;
}
#endif

inline
void InsetCollapsable::scroll(BufferView *bv, float sx) const
{
	UpdatableInset::scroll(bv, sx);
}


inline
void InsetCollapsable::scroll(BufferView *bv, int offset) const
{
	UpdatableInset::scroll(bv, offset);
}


inline
bool InsetCollapsable::isOpen() const
{
	return !collapsed_;
}


inline
bool InsetCollapsable::allowSpellcheck() const
{
	return inset.allowSpellcheck();
}


inline
void InsetCollapsable::selectSelectedWord(BufferView * bv)
{
	inset.selectSelectedWord(bv);
}


inline
Box const & InsetCollapsable::buttonDim() const
{
	return button_dim;
}

#endif
