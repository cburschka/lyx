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
#include "debug.h"
#include "lyxtext.h"
#include "font.h"
#include "lyxlex.h"

#include "insets/insettext.h"

#include "support/LOstream.h"
#include "support/lstrings.h"

using std::vector;
using std::ostream;
using std::endl;
using std::max;


class LyXText;


InsetCollapsable::InsetCollapsable(BufferParams const & bp, bool collapsed)
	: UpdatableInset(), collapsed_(collapsed), inset(bp),
	  button_length(0), button_top_y(0), button_bottom_y(0),
	  need_update(NONE), label("Label"),
#if 0
	autocollapse(false),
#endif
	  oldWidth(0), in_update(false)
{
	inset.setOwner(this);
	inset.setAutoBreakRows(true);
	inset.setDrawFrame(0, InsetText::ALWAYS);
	inset.setFrameColor(0, LColor::collapsableframe);
	setInsetName("Collapsable");
}


InsetCollapsable::InsetCollapsable(InsetCollapsable const & in, bool same_id)
	: UpdatableInset(in, same_id), collapsed_(in.collapsed_),
	  framecolor(in.framecolor), labelfont(in.labelfont), inset(in.inset),
	  button_length(0), button_top_y(0), button_bottom_y(0),
	  need_update(NONE), label(in.label),
#if 0
	  autocollapse(in.autocollapse),
#endif
	  oldWidth(0), in_update(false)
{
	inset.init(&(in.inset), same_id);
	inset.setOwner(this);
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
	if (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token == "collapsed") {
			lex.next();
			collapsed_ = lex.getBool();
		} else {
			lyxerr << "InsetCollapsable::Read: Missing collapsed!"
			       << endl;
			// Take countermeasures
			lex.pushToken(token);
		}
	}
	inset.read(buf, lex);
}


int InsetCollapsable::ascent_collapsed() const
{
	int width = 0;
	int ascent = 0;
	int descent = 0;
	lyxfont::buttonText(label, labelfont, width, ascent, descent);
	return ascent;
}


int InsetCollapsable::descent_collapsed() const
{
	int width = 0;
	int ascent = 0;
	int descent = 0;
	lyxfont::buttonText(label, labelfont, width, ascent, descent);
	return descent;
}


//int InsetCollapsable::width_collapsed(Painter & pain) const
int InsetCollapsable::width_collapsed() const
{
	int width;
	int ascent;
	int descent;
	lyxfont::buttonText(label, labelfont, width, ascent, descent);
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


void InsetCollapsable::draw_collapsed(Painter & pain,
				      int baseline, float & x) const
{
	pain.buttonText(int(x) + TEXT_TO_INSET_OFFSET,
			baseline, label, labelfont);
	x += width_collapsed();
}


void InsetCollapsable::draw(BufferView * bv, LyXFont const & f,
			    int baseline, float & x, bool cleared) const
{
	if (need_update != NONE) {
		const_cast<InsetText *>(&inset)->update(bv, f, true);
		bv->text->status(bv, LyXText::CHANGED_IN_DRAW);
		need_update = NONE;
		return;
	}
	if (nodraw())
		return;

	Painter & pain = bv->painter();

	button_length = width_collapsed();
	button_top_y = -ascent(bv, f);
	button_bottom_y = -ascent(bv, f) + ascent_collapsed() +
		descent_collapsed();

	if (!isOpen()) {
		draw_collapsed(pain, baseline, x);
		return;
	}

	float old_x = x;

	if (!owner())
		x += static_cast<float>(scroll());

	if (!cleared && (inset.need_update == InsetText::FULL ||
			 inset.need_update == InsetText::INIT ||
			 top_x != int(x) ||
			 top_baseline != baseline))
	{
		// we don't need anymore to clear here we just have to tell
		// the underlying LyXText that it should do the RowClear!
		inset.setUpdateStatus(bv, InsetText::FULL);
		bv->text->status(bv, LyXText::CHANGED_IN_DRAW);
		return;
	}

	top_x = int(x);
	topx_set = true;
	top_baseline = baseline;

	int const bl = baseline - ascent(bv, f) + ascent_collapsed();

	draw_collapsed(pain, bl, old_x);
	inset.draw(bv, f,
			   bl + descent_collapsed() + inset.ascent(bv, f),
			   x, cleared);
	if (x < (top_x + button_length + TEXT_TO_INSET_OFFSET))
		x = top_x + button_length + TEXT_TO_INSET_OFFSET;
}


void InsetCollapsable::edit(BufferView * bv, int xp, int yp,
			    unsigned int button)
{
	UpdatableInset::edit(bv, xp, yp, button);

	if (collapsed_) {
		collapsed_ = false;
		// set this only here as it should be recollapsed only if
		// it was already collapsed!
		first_after_edit = true;
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
	first_after_edit = true;
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


void InsetCollapsable::insetButtonPress(BufferView * bv,
					int x, int y, int button)
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


bool InsetCollapsable::insetButtonRelease(BufferView * bv,
					  int x, int y, int button)
{
	bool ret = false;
	if ((button != 3) && (x >= 0)  && (x < button_length) &&
	    (y >= button_top_y) &&  (y <= button_bottom_y))
	{
		if (collapsed_) {
			collapsed_ = false;
// should not be called on inset open!
//			inset.insetButtonRelease(bv, 0, 0, button);
			inset.setUpdateStatus(bv, InsetText::FULL);
			bv->updateInset(this, false);
		} else {
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
		ret = inset.insetButtonRelease(bv, x, yy, button);
	}
	if ((button == 3) && !ret) {
		return showInsetDialog(bv);
	}
	return false;
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


int InsetCollapsable::ascii(Buffer const * buf, ostream & os, int ll) const
{
	return inset.ascii(buf, os, ll);
}


int InsetCollapsable::linuxdoc(Buffer const * buf, ostream & os) const
{
	return inset.linuxdoc(buf, os);
}


int InsetCollapsable::docbook(Buffer const * buf, ostream & os) const
{
	return inset.docbook(buf, os);
}

#if 0
int InsetCollapsable::getMaxWidth(BufferView * bv,
				  UpdatableInset const * in) const
{
#if 0
	int const w = UpdatableInset::getMaxWidth(bv, in);

	if (w < 0) {
		// What does a negative max width signify? (Lgb)
		// Use the max width of the draw-area (Jug)
		return w;
	}
	// should be at least 30 pixels !!!
	return max(30, w - width_collapsed());
#else
	return UpdatableInset::getMaxWidth(bv, in);
#endif
}
#endif


void InsetCollapsable::update(BufferView * bv, LyXFont const & font,
			      bool reinit)
{
	if (in_update) {
		if (reinit && owner()) {
			owner()->update(bv, font, true);
		}
		return;
	}
	in_update = true;
	inset.update(bv, font, reinit);
	if (reinit && owner()) {
		owner()->update(bv, font, true);
	}
	in_update = false;
}


UpdatableInset::RESULT
InsetCollapsable::localDispatch(BufferView * bv, kb_action action,
				string const & arg)
{
	UpdatableInset::RESULT result = inset.localDispatch(bv, action, arg);
	if (result >= FINISHED)
		bv->unlockInset(this);
	first_after_edit = false;
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
	if (in == this)
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


vector<string> const InsetCollapsable::getLabelList() const
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
	lyxerr[Debug::INFO] << "Looking for paragraph " << id << endl;
	return inset.getParFromID(id);
}


Paragraph * InsetCollapsable::firstParagraph() const
{
	return inset.firstParagraph();
}


Paragraph * InsetCollapsable::getFirstParagraph(int i) const
{
	return inset.getFirstParagraph(i);
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


void InsetCollapsable::open(BufferView * bv)
{
	if (!collapsed_) return;

	collapsed_ = false;
	bv->updateInset(this, false);
}


void InsetCollapsable::close(BufferView * bv) const
{
	if (collapsed_)
		return;

	collapsed_ = true;
	bv->updateInset(const_cast<InsetCollapsable *>(this), false);
}


void InsetCollapsable::setLabel(string const & l) const
{
	label = l;
}


bool InsetCollapsable::searchForward(BufferView * bv, string const & str,
				     bool cs, bool mw)
{
	bool found = inset.searchForward(bv, str, cs, mw);
	if (first_after_edit && !found)
		close(bv);
	first_after_edit = false;
	return found;
}


bool InsetCollapsable::searchBackward(BufferView * bv, string const & str,
				      bool cs, bool mw)
{
	bool found = inset.searchBackward(bv, str, cs, mw);
	if (first_after_edit && !found)
		close(bv);
	first_after_edit = false;
	return found;
}


string const InsetCollapsable::selectNextWordToSpellcheck(BufferView * bv,
					      float & value) const
{
	string const str = inset.selectNextWordToSpellcheck(bv, value);
	if (first_after_edit && str.empty())
		close(bv);
	first_after_edit = false;
	return str;
}
