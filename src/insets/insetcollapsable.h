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
#include "lyxfont.h"
#include "funcrequest.h" // for adjustCommand
#include "LColor.h"
#include "box.h"

#include <boost/weak_ptr.hpp>

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
	void read(Buffer const *, LyXLex &);
	///
	void write(Buffer const *, std::ostream &) const;
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
	virtual bool insetAllowed(InsetOld::Code code) const {
		return inset.insetAllowed(code);
	}
	///
	bool isTextInset() const { return true; }
	///
	void insetUnlock(BufferView *);
	///
	bool needFullRow() const { return isOpen(); }
	///
	bool lockInsetInInset(BufferView *, UpdatableInset *);
	///
	bool unlockInsetInInset(BufferView *, UpdatableInset *,
				bool lr = false);
	///
	int insetInInsetY() const;
	///
	RESULT localDispatch(FuncRequest const &);
	///
	int latex(Buffer const *, std::ostream &,
		  LatexRunParams const &) const;
	///
	int ascii(Buffer const *, std::ostream &, int) const;
	///
	int linuxdoc(Buffer const *, std::ostream &) const;
	///
	int docbook(Buffer const *, std::ostream &, bool mixcont) const;
	///
	void validate(LaTeXFeatures & features) const;
	/// FIXME, document
	void getCursorPos(BufferView *, int & x, int & y) const;
	/// Get the absolute document x,y of the cursor
	virtual void getCursor(BufferView &, int &, int &) const;
	///
	void fitInsetCursor(BufferView * bv) const {
		inset.fitInsetCursor(bv);
	}
	UpdatableInset * getLockingInset() const;
	///
	UpdatableInset * getFirstLockingInsetOfType(InsetOld::Code);
	///
	void setFont(BufferView *, LyXFont const &, bool toggleall = false,
		 bool selectall = false);
	///
	void setLabel(string const & l) const;
	///
	void setLabelFont(LyXFont & f) { labelfont = f; }
#if 0
	///
	void setAutoCollapse(bool f) { autocollapse = f; }
#endif
	///
	LyXText * getLyXText(BufferView const *, bool const recursive) const;
	///
	void deleteLyXText(BufferView *, bool recursive=true) const;
	///
	void getLabelList(std::vector<string> &) const;
	///
	int scroll(bool recursive=true) const;
	///
	void scroll(BufferView *bv, float sx) const {
		UpdatableInset::scroll(bv, sx);
	}
	///
	void scroll(BufferView *bv, int offset) const {
		UpdatableInset::scroll(bv, offset);
	}
	///
	InsetOld * getInsetFromID(int id) const;
	///
	ParagraphList * getParagraphs(int) const;
	///
	LyXCursor const & cursor(BufferView *) const;
	///
	bool isOpen() const { return !collapsed_; }
	///
	void open(BufferView *);
	///
	void close(BufferView *) const;
	///
	bool allowSpellcheck() const { return inset.allowSpellcheck(); }
	///
	WordLangTuple const
	selectNextWordToSpellcheck(BufferView *, float &) const;
	///
	void selectSelectedWord(BufferView * bv) {
		inset.selectSelectedWord(bv);
	}

	void markErased();

	bool nextChange(BufferView * bv, lyx::pos_type & length);

	///
	bool searchForward(BufferView * bv, string const & str,
			   bool = true, bool = false);
	bool searchBackward(BufferView * bv, string const & str,
			    bool = true, bool = false);

	///
	void addPreview(lyx::graphics::PreviewLoader &) const;

protected:
	///
	virtual void cache(BufferView *) const;
	///
	virtual BufferView * view() const;

	///
	void dimension_collapsed(Dimension &) const;
	///
	int height_collapsed() const;
	///
	void draw_collapsed(PainterInfo & pi, int x, int y) const;
	///
	int getMaxTextWidth(Painter & pain, UpdatableInset const *) const;

	///
	mutable bool collapsed_;
	///
	LColor::color framecolor;
	///
	LyXFont labelfont;
public:
	///
	mutable InsetText inset;
protected:
	///
	mutable Box button_dim;
	///
	mutable int topx;
	mutable int topbaseline;

private:
	///
	void lfunMouseRelease(FuncRequest const &);
	///
	FuncRequest adjustCommand(FuncRequest const &);

	///
	mutable string label;
#if 0
	///
	bool autocollapse;
#endif
	///
	bool in_update;
	///
	mutable bool first_after_edit;
	///
	mutable boost::weak_ptr<BufferView> view_;
};

#endif
