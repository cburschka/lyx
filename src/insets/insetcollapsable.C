/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *          Copyright 1998-2001 The LyX Team.
 *
 * ======================================================
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetcollapsable.h"
#include "gettext.h"
#include "lyxfont.h"
#include "BufferView.h"
#include "Painter.h"
#include "insets/insettext.h"
#include "support/LOstream.h"
#include "support/lstrings.h"
#include "debug.h"
#include "lyxtext.h"
#include "font.h"

class LyXText;

using std::ostream;
using std::endl;
using std::max;


InsetCollapsable::InsetCollapsable(bool collapsed)
	: UpdatableInset(), collapsed_(collapsed), 
	  button_length(0), button_top_y(0), button_bottom_y(0),
	  label("Label"),
#if 0
	draw_label(label),
#endif
#if 0
	autocollapse(false),
#endif
	  oldWidth(0), need_update(FULL),
	  inlined(false)
#if 0
	, change_label_with_text(false)
#endif
{
	inset.setOwner(this);
	inset.setAutoBreakRows(true);
	inset.setDrawFrame(0, InsetText::ALWAYS);
	inset.setFrameColor(0, LColor::collapsableframe);
	setInsetName("Collapsable");
}


InsetCollapsable::InsetCollapsable(InsetCollapsable const & in, bool same_id)
	: UpdatableInset(in, same_id), collapsed_(in.collapsed_), 
	  framecolor(in.framecolor), labelfont(in.labelfont),
	  button_length(0), button_top_y(0), button_bottom_y(0),
	  label(in.label),
#if 0
	draw_label(label),
#endif
#if 0
	autocollapse(in.autocollapse),
#endif
	  oldWidth(0), need_update(FULL),
	  inlined(in.inlined)
#if 0
	, change_label_with_text(in.change_label_with_text)
#endif
{
	inset.init(&(in.inset), same_id);
	inset.setOwner(this);
}


Inset * InsetCollapsable::clone(Buffer const &, bool same_id) const
{
	return new InsetCollapsable(*const_cast<InsetCollapsable *>(this),
								same_id);
}


bool InsetCollapsable::insertInset(BufferView * bv, Inset * in)
{
	if (!insetAllowed(in->lyxCode())) {
		lyxerr << "InsetCollapsable::InsertInset: "
			"Unable to insert inset." << endl;
		return false;
	}
	return inset.insertInset(bv, in);
}


void InsetCollapsable::write(Buffer const * buf, ostream & os) const
{
	os << "collapsed " << tostr(collapsed_) << "\n";
	inset.writeParagraphData(buf, os);
}



void InsetCollapsable::read(Buffer const * buf, LyXLex & lex)
{
	if (lex.IsOK()) {
		lex.next();
		string const token = lex.GetString();
		if (token == "collapsed") {
			lex.next();
			collapsed_ = lex.GetBool();
		} else {
			lyxerr << "InsetCollapsable::Read: Missing collapsed!"
			       << endl;
		}
	}
	inset.read(buf, lex);
#if 0
	if (collapsed_ && change_label_with_text) {
		draw_label = get_new_label();
	} else {
		draw_label = label;
	}
#endif
}


//int InsetCollapsable::ascent_collapsed(Painter & pain) const
int InsetCollapsable::ascent_collapsed() const
{
	int width = 0;
	int ascent = 0;
	int descent = 0;
#if 0
	pain.buttonText(0, 0, draw_label, labelfont, false, 
			width, ascent, descent);
#else
	lyxfont::buttonText(label, labelfont, width, ascent, descent);
#endif
	return ascent;
}


//int InsetCollapsable::descent_collapsed(Painter & pain) const
int InsetCollapsable::descent_collapsed() const
{
	int width = 0;
	int ascent = 0;
	int descent = 0;
#if 0
	pain.buttonText(0, 0, draw_label, labelfont, false, 
			width, ascent, descent);
#else
	lyxfont::buttonText(label, labelfont, width, ascent, descent);
#endif
	return descent;
}


//int InsetCollapsable::width_collapsed(Painter & pain) const
int InsetCollapsable::width_collapsed() const
{
	int width;
	int ascent;
	int descent;
#if 0
	pain.buttonText(TEXT_TO_INSET_OFFSET, 0, draw_label, labelfont, false,
			width, ascent, descent);
#else
	lyxfont::buttonText(label, labelfont, width, ascent, descent);
#endif
	return width + (2*TEXT_TO_INSET_OFFSET);
}


int InsetCollapsable::ascent(BufferView * /*bv*/, LyXFont const &) const
{
	return ascent_collapsed();
}


int InsetCollapsable::descent(BufferView * bv, LyXFont const & font) const
{
	if (collapsed_) 
		return descent_collapsed();

	return descent_collapsed()
		+ inset.descent(bv, font)
		+ inset.ascent(bv, font)
		+ TEXT_TO_BOTTOM_OFFSET;
}


int InsetCollapsable::width(BufferView * bv, LyXFont const & font) const
{
	if (collapsed_) 
		return width_collapsed();

	int widthCollapsed = width_collapsed();

	return (inset.width(bv, font) > widthCollapsed) ?
		inset.width(bv, font) : widthCollapsed;
}


void InsetCollapsable::draw_collapsed(Painter & pain, int baseline, float & x) const
{
#if 0
	int width = 0;
	pain.buttonText(int(x) + TEXT_TO_INSET_OFFSET,
			baseline, draw_label, labelfont, true, width);
	x += width + TEXT_TO_INSET_OFFSET;
#else
	pain.buttonText(int(x) + TEXT_TO_INSET_OFFSET,
			baseline, label, labelfont);
	x += width_collapsed();
#endif
}


void InsetCollapsable::draw(BufferView * bv, LyXFont const & f, 
                            int baseline, float & x, bool cleared) const
{
	if (nodraw())
		return;

	Painter & pain = bv->painter();

	button_length = width_collapsed();
	button_top_y = -ascent(bv, f);
	button_bottom_y = -ascent(bv, f) + ascent_collapsed() +
		descent_collapsed();

	if (collapsed_) {
		draw_collapsed(pain, baseline, x);
		x += TEXT_TO_INSET_OFFSET;
		return;
	}

	float old_x = x;

#if 0
	UpdatableInset::draw(bv, f, baseline, x, cleared);
#else
	if (!owner())
		x += static_cast<float>(scroll());
#endif
	if (!cleared && (inset.need_update == InsetText::FULL ||
			 inset.need_update == InsetText::INIT ||
			 top_x != int(x) ||
			 top_baseline != baseline))
	{
#if 1
		// we don't need anymore to clear here we just have to tell
		// the underlying LyXText that it should do the RowClear!
		inset.setUpdateStatus(bv, InsetText::FULL);
		bv->text->status(bv, LyXText::CHANGED_IN_DRAW);
		return;
#else
		int w =  owner() ? width(bv, f) : pain.paperWidth();
		int h = ascent(bv, f) + descent(bv, f);
		int const tx = (needFullRow() && !owner()) ? 0 : int(x);
		int const ty = max(0, baseline - ascent(bv, f));

		if ((ty + h) > pain.paperHeight())
			h = pain.paperHeight();
		if ((top_x + w) > pain.paperWidth())
			w = pain.paperWidth();
		if (baseline < 0)
			h += (baseline - ascent(bv, f));
		pain.fillRectangle(tx, ty - 1, w, h + 2);
		cleared = true;
#endif
	}

	top_x = int(x);
	top_baseline = baseline;

	int const bl = baseline - ascent(bv, f) + ascent_collapsed();

	draw_collapsed(pain, bl, old_x);
	inset.draw(bv, f, 
		   bl + descent_collapsed() + inset.ascent(bv, f),
		   x, cleared);
	need_update = NONE;
}


void InsetCollapsable::edit(BufferView * bv, int xp, int yp,
                            unsigned int button)
{
	UpdatableInset::edit(bv, xp, yp, button);

	if (collapsed_) {
#if 0
		draw_label = label;
#endif
		collapsed_ = false;
		if (!bv->lockInset(this))
			return;
		bv->updateInset(this, false);
		inset.edit(bv);
	} else {
		if (!bv->lockInset(this))
			return;
		if (yp <= button_bottom_y) {
			inset.edit(bv);
		} else {
			LyXFont font(LyXFont::ALL_SANE);
			int yy = ascent(bv, font) + yp -
				(ascent_collapsed() +
				 descent_collapsed() +
				 inset.ascent(bv, font));
			inset.edit(bv, xp, yy, button);
		}
	}
}


void InsetCollapsable::edit(BufferView * bv, bool front)
{
	UpdatableInset::edit(bv, front);

	if (collapsed_) {
#if 0
		draw_label = label;
#endif
		collapsed_ = false;
		if (!bv->lockInset(this))
			return;
		inset.setUpdateStatus(bv, InsetText::FULL);
		bv->updateInset(this, false);
		inset.edit(bv, front);
	} else {
		if (!bv->lockInset(this))
			return;
		inset.edit(bv, front);
	}
}


Inset::EDITABLE InsetCollapsable::editable() const
{
	if (collapsed_)
		return IS_EDITABLE;
	return HIGHLY_EDITABLE;
}


void InsetCollapsable::insetUnlock(BufferView * bv)
{
#if 0
	if (autocollapse) {
		if (change_label_with_text) {
			draw_label = get_new_label();
		} else {
			draw_label = label;
		}
		collapsed_ = true;
	}
#endif
	inset.insetUnlock(bv);
	if (scroll())
		scroll(bv, 0.0F);
	bv->updateInset(this, false);
}


void InsetCollapsable::insetButtonPress(BufferView * bv, int x, int y,
					int button)
{
	if (!collapsed_ && (y > button_bottom_y)) {
		LyXFont font(LyXFont::ALL_SANE);
		int yy = ascent(bv, font) + y -
		    (ascent_collapsed() +
		     descent_collapsed() +
		     inset.ascent(bv, font));
		inset.insetButtonPress(bv, x, yy, button);
	}
}


void InsetCollapsable::insetButtonRelease(BufferView * bv,
					  int x, int y, int button)
{
	if ((x >= 0)  && (x < button_length) &&
	    (y >= button_top_y) &&  (y <= button_bottom_y)) {
		if (collapsed_) {
#if 0
			draw_label = label;
#endif
			collapsed_ = false;
			inset.insetButtonRelease(bv, 0, 0, button);
			inset.setUpdateStatus(bv, InsetText::FULL);
			bv->updateInset(this, false);
		} else {
#if 0
			if (change_label_with_text) {
				draw_label = get_new_label();
			} else {
				draw_label = label;
			}
#endif
			collapsed_ = true;
			bv->unlockInset(this);
			bv->updateInset(this, false);
		}
	} else if (!collapsed_ && (y > button_bottom_y)) {
		LyXFont font(LyXFont::ALL_SANE);
		int yy = ascent(bv, font) + y -
		    (ascent_collapsed() +
		     descent_collapsed() +
		     inset.ascent(bv, font));
		inset.insetButtonRelease(bv, x, yy, button);
	}
}


void InsetCollapsable::insetMotionNotify(BufferView * bv,
					 int x, int y, int state)
{
	if (y > button_bottom_y) {
		LyXFont font(LyXFont::ALL_SANE);
		int yy = ascent(bv, font) + y -
		    (ascent_collapsed() +
		     descent_collapsed() +
		     inset.ascent(bv, font));
		inset.insetMotionNotify(bv, x, yy, state);
	}
}


void InsetCollapsable::insetKeyPress(XKeyEvent * xke)
{
	inset.insetKeyPress(xke);
}


int InsetCollapsable::latex(Buffer const * buf, ostream & os,
			    bool fragile, bool free_spc) const
{
	return inset.latex(buf, os, fragile, free_spc);
}


int InsetCollapsable::getMaxWidth(BufferView * bv,
				  UpdatableInset const * inset) const
{
	int const w = UpdatableInset::getMaxWidth(bv, inset);

	if (w < 0) {
		// What does a negative max width signify? (Lgb)
		// Use the max width of the draw-area (Jug)
		return w;
	}
	// should be at least 30 pixels !!!
	return max(30, w - width_collapsed());
}


void InsetCollapsable::update(BufferView * bv, LyXFont const & font,
			      bool reinit)
{
	inset.update(bv, font, reinit);
}


UpdatableInset::RESULT
InsetCollapsable::localDispatch(BufferView * bv, kb_action action,
				string const & arg)
{
	UpdatableInset::RESULT result = inset.localDispatch(bv, action, arg);
	if (result == FINISHED)
		bv->unlockInset(this);
	return result;
}


bool InsetCollapsable::lockInsetInInset(BufferView * bv, UpdatableInset * in)
{
	if (&inset == in)
		return true;
	return inset.lockInsetInInset(bv, in);
}


bool InsetCollapsable::unlockInsetInInset(BufferView * bv, UpdatableInset * in,
					  bool lr)
{
	if (&inset == in) {
		bv->unlockInset(this);
		return true;
	}
	return inset.unlockInsetInInset(bv, in, lr);
}


bool InsetCollapsable::updateInsetInInset(BufferView * bv, Inset *in)
{
	if (&inset == in)
		return true;
	return inset.updateInsetInInset(bv, in);
}


unsigned int InsetCollapsable::insetInInsetY()
{
	return inset.insetInInsetY() - (top_baseline - inset.y());
}


void InsetCollapsable::validate(LaTeXFeatures & features) const
{
	inset.validate(features);
}


void InsetCollapsable::getCursorPos(BufferView * bv, int & x, int & y) const
{
	inset.getCursorPos(bv, x , y);
}


void InsetCollapsable::toggleInsetCursor(BufferView * bv)
{
	inset.toggleInsetCursor(bv);
}


void InsetCollapsable::showInsetCursor(BufferView * bv, bool show)
{
	inset.showInsetCursor(bv, show);
}


void InsetCollapsable::hideInsetCursor(BufferView * bv)
{
	inset.hideInsetCursor(bv);
}


UpdatableInset * InsetCollapsable::getLockingInset() const
{
	UpdatableInset * in = inset.getLockingInset();
	if (const_cast<InsetText *>(&inset) == in)
		return const_cast<InsetCollapsable *>(this);
	return in;
}


UpdatableInset * InsetCollapsable::getFirstLockingInsetOfType(Inset::Code c)
{
	if (c == lyxCode())
		return this;
	return inset.getFirstLockingInsetOfType(c);
}


void InsetCollapsable::setFont(BufferView * bv, LyXFont const & font,
                               bool toggleall, bool selectall)
{
	inset.setFont(bv, font, toggleall, selectall);
}


bool InsetCollapsable::doClearArea() const
{
	return inset.doClearArea();
}


LyXText * InsetCollapsable::getLyXText(BufferView const * bv,
				       bool const recursive) const
{
	return inset.getLyXText(bv, recursive);
}


void InsetCollapsable::deleteLyXText(BufferView * bv, bool recursive) const
{
	inset.deleteLyXText(bv, recursive);
}


void InsetCollapsable::resizeLyXText(BufferView * bv, bool force) const
{
	inset.resizeLyXText(bv, force);
	LyXFont font(LyXFont::ALL_SANE);
	oldWidth = width(bv, font);
}


std::vector<string> const InsetCollapsable::getLabelList() const
{
	return inset.getLabelList();
}


bool InsetCollapsable::nodraw() const
{
	return inset.nodraw();
}

 
int InsetCollapsable::scroll(bool recursive) const
{
	int sx = UpdatableInset::scroll(false);

	if (recursive)
		sx += inset.scroll(recursive);

	return sx;
}


Paragraph * InsetCollapsable::getParFromID(int id) const
{
	return inset.getParFromID(id);
}


Paragraph * InsetCollapsable::firstParagraph() const
{
	return inset.firstParagraph();
}


LyXCursor const & InsetCollapsable::cursor(BufferView * bv) const
{
	return inset.cursor(bv);
}


Inset * InsetCollapsable::getInsetFromID(int id_arg) const
{
	if (id_arg == id())
		return const_cast<InsetCollapsable *>(this);
	return inset.getInsetFromID(id_arg);
}


#if 0
void InsetCollapsable::open(BufferView * bv, bool flag)
{
	if (flag == !collapsed_)
		return;
	collapsed_ = !flag;
#if 0
	if (collapsed_ && change_label_with_text) {
		draw_label = get_new_label();
	} else {
		draw_label = label;
	}
#endif
	bv->updateInset(this, false);
}
#else
void InsetCollapsable::open(BufferView * bv)
{
	if (!collapsed_) return;
	
	collapsed_ = false;
	bv->updateInset(this, false);
}


void InsetCollapsable::close(BufferView * bv)
{
	if (collapsed_) return;
	
	collapsed_ = true;
	bv->updateInset(this, false);
}
#endif


#if 0
void InsetCollapsable::setLabel(string const & l, bool flag)
{
	label = l;
	change_label_with_text = flag;
	if (collapsed_ && change_label_with_text) {
		draw_label = get_new_label();
	} else {
		draw_label = label;
	}
}
#else
void InsetCollapsable::setLabel(string const & l)
{
	label = l;
}
#endif


#if 0
string const InsetCollapsable::get_new_label() const
{
	string la;
	Paragraph::size_type const max_length = 15;

	int n = std::min(max_length, inset.paragraph()->size());
	int i = 0;
	int j = 0;
	for(; i < n && j < inset.paragraph()->size(); ++j) {
		if (inset.paragraph()->isInset(j))
			continue;
		la += inset.paragraph()->getChar(j);
		++i;
	}
	if ((i > 0) && (j < inset.paragraph()->size()))
		la += "...";
	if (la.empty())
		la = label;
	return la;
}
#endif
