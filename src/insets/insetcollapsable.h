// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 2000-2001 The LyX Team.
 *
 *======================================================
 */


#ifndef INSETCOLLAPSABLE_H
#define INSETCOLLAPSABLE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "inset.h"
#include "insettext.h"
#include "lyxfont.h"
#include "LColor.h"

class Painter;
class LyXText;
class Paragraph;
class LyXCursor;

/** A colapsable text inset
  
*/
class InsetCollapsable : public UpdatableInset {
public:
	///
	enum UpdateCodes {
		NONE = 0,
		FULL
	};
	///
	static int const TEXT_TO_TOP_OFFSET = 2;
	///
	static int const TEXT_TO_BOTTOM_OFFSET = 2;
	///
	InsetCollapsable();
	///
	void read(Buffer const *, LyXLex &);
	///
	void write(Buffer const *, std::ostream &) const;
	///
	int ascent(BufferView *, LyXFont const &) const;
	///
	int descent(BufferView *, LyXFont const &) const;
	///
	int width(BufferView *, LyXFont const & f) const;
	///
	void draw(BufferView *, const LyXFont &, int , float &, bool) const;
	///
	void update(BufferView *, LyXFont const &, bool =false); 
	///
	void edit(BufferView *, int, int, unsigned int);
	///
	EDITABLE editable() const;
	///
	bool insertInset(BufferView *, Inset * inset);
	///
	bool isTextInset() const { return true; }
	///
	bool doClearArea() const;
	///
	void insetUnlock(BufferView *);
	///
	bool needFullRow() const { return !collapsed; }
	///
	bool lockInsetInInset(BufferView *, UpdatableInset *);
	///
	bool unlockInsetInInset(BufferView *, UpdatableInset *,
				bool lr = false);
	///
	bool updateInsetInInset(BufferView *, Inset *);
	///
	unsigned int insetInInsetY();
	///
	void insetButtonRelease(BufferView *, int, int, int);
	///
	void insetButtonPress(BufferView *, int, int, int);
	///
	void insetMotionNotify(BufferView *, int, int, int);
	///
	void insetKeyPress(XKeyEvent *);
	///
	UpdatableInset::RESULT localDispatch(BufferView *, kb_action,
					     string const &);
	///
	int latex(Buffer const *, std::ostream &,
		  bool fragile, bool free_spc) const;
	///
	int ascii(Buffer const *, std::ostream &, int) const { return 0; }
	///
	int linuxdoc(Buffer const *, std::ostream &) const { return 0; }
	///
	int docBook(Buffer const *, std::ostream &) const { return 0; }
	///
	void validate(LaTeXFeatures & features) const;
	///
	void getCursorPos(BufferView *, int & x, int & y) const;
	///
	void toggleInsetCursor(BufferView *);
	///
	UpdatableInset * getLockingInset();
	///
	UpdatableInset * getFirstLockingInsetOfType(Inset::Code);
	///
	void setFont(BufferView *, LyXFont const &, bool toggleall = false,
                 bool selectall = false);
	///
	void setLabel(string const & l) { label = l; }
	///
	void setLabelFont(LyXFont & f) { labelfont = f; }
	///
	void setAutoCollapse(bool f) { autocollapse = f; }
	///
	int getMaxWidth(BufferView *, UpdatableInset const *) const;
	///
	LyXText * getLyXText(BufferView const *, bool const recursive) const;
	///
	void deleteLyXText(BufferView *, bool recursive=true) const;
	///
	void resizeLyXText(BufferView *, bool force = false) const;
	///
	std::vector<string> const getLabelList() const;
	///
	bool nodraw() const;
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
	Paragraph * getParFromID(int id) const;
	///
	Inset * getInsetFromID(int id) const;
	///
	Paragraph * firstParagraph() const;
	///
	LyXCursor const & cursor(BufferView *) const;

protected:
	///
	int ascent_collapsed(Painter &, LyXFont const &) const;
	///
	int descent_collapsed(Painter &, LyXFont const &) const;
	///
	int width_collapsed(Painter &, LyXFont const & f) const;
	///
	void draw_collapsed(Painter & pain, const LyXFont &, int , float &) const;
	///
	int getMaxTextWidth(Painter & pain, UpdatableInset const *) const;
	
	///
	bool collapsed;
	///
	LColor::color framecolor;
	///
	LyXFont labelfont;
public:
	///
	InsetText inset;
protected:
	///
	mutable int button_length;
	///
	mutable int button_top_y;
	///
	mutable int button_bottom_y;
private:
	///
	string label;
	///
	bool autocollapse;
	///
	int widthCollapsed;
	///
	mutable int oldWidth;
	///
	mutable int topx;
	mutable int topbaseline;
	mutable UpdateCodes need_update;
};

#endif
