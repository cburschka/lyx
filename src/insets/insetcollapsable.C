/**
 * \file insetcollapsable.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "insetcollapsable.h"
#include "insettext.h"

#include "BufferView.h"
#include "debug.h"
#include "dimension.h"
#include "gettext.h"
#include "lyxfont.h"
#include "lyxlex.h"
#include "lyxtext.h"
#include "WordLangTuple.h"
#include "funcrequest.h"
#include "buffer.h"

#include "frontends/font_metrics.h"
#include "frontends/Painter.h"
#include "frontends/LyXView.h"

#include "support/LAssert.h"
#include "support/LOstream.h"

using std::vector;
using std::ostream;
using std::endl;
using std::max;


InsetCollapsable::InsetCollapsable(BufferParams const & bp, bool collapsed)
	: UpdatableInset(), collapsed_(collapsed), inset(bp),
	  button_length(0), button_top_y(0), button_bottom_y(0),
	  label("Label"),
#if 0
	autocollapse(false),
#endif
	  oldWidth(0), in_update(false), first_after_edit(false)
{
	inset.setOwner(this);
	inset.setAutoBreakRows(true);
	inset.setDrawFrame(0, InsetText::ALWAYS);
	inset.setFrameColor(0, LColor::collapsableframe);
	setInsetName("Collapsable");
}


InsetCollapsable::InsetCollapsable(InsetCollapsable const & in)
	: UpdatableInset(in), collapsed_(in.collapsed_),
	  framecolor(in.framecolor), labelfont(in.labelfont), inset(in.inset),
	  button_length(0), button_top_y(0), button_bottom_y(0),
	  label(in.label),
#if 0
	  autocollapse(in.autocollapse),
#endif
	  oldWidth(0), in_update(false), first_after_edit(false)
{
	inset.init(&(in.inset));
	inset.setOwner(this);
}


// InsetCollapsable::InsetCollapsable(InsetCollapsable const & in, bool same_id)
//	: UpdatableInset(in, same_id), collapsed_(in.collapsed_),
//	  framecolor(in.framecolor), labelfont(in.labelfont), inset(in.inset),
//	  button_length(0), button_top_y(0), button_bottom_y(0),
//	  label(in.label),
// #if 0
//	  autocollapse(in.autocollapse),
// #endif
//	  oldWidth(0), in_update(false), first_after_edit(false)
// {
//	inset.init(&(in.inset), same_id);
//	inset.setOwner(this);
// }


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
	os << "collapsed " << (collapsed_ ? "true" : "false") << "\n";
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


void InsetCollapsable::dimension_collapsed(Dimension & dim) const
{
	font_metrics::buttonText(label, labelfont, dim.wid, dim.asc, dim.des);
	dim.wid += 2 * TEXT_TO_INSET_OFFSET;
}


int InsetCollapsable::height_collapsed() const
{
	Dimension dim;
	font_metrics::buttonText(label, labelfont, dim.wid, dim.asc, dim.des);
	return dim.asc + dim.des;
}


void InsetCollapsable::dimension(BufferView * bv, LyXFont const & font,
	Dimension & dim) const
{
	dimension_collapsed(dim);
	if (collapsed_)
		return;
	Dimension insetdim;
	inset.dimension(bv, font, insetdim);
	dim.des += insetdim.height() + TEXT_TO_BOTTOM_OFFSET;
	dim.wid = max(dim.wid, insetdim.width());
}


void InsetCollapsable::draw_collapsed(Painter & pain,
				      int baseline, float & x) const
{
	pain.buttonText(int(x) + TEXT_TO_INSET_OFFSET,
			baseline, label, labelfont);
	Dimension dim;
	dimension_collapsed(dim);
	x += dim.wid;
}


void InsetCollapsable::draw(BufferView * bv, LyXFont const & f,
                            int baseline, float & x, bool inlined) const
{
	lyx::Assert(bv);
	cache(bv);

	if (nodraw())
		return;

	Dimension dim_collapsed;
	dimension_collapsed(dim_collapsed);

	Painter & pain = bv->painter();

	button_length   = dim_collapsed.width();
	button_top_y    = -ascent(bv, f);
	button_bottom_y = -ascent(bv, f) + dim_collapsed.height();

	if (!isOpen()) {
		draw_collapsed(pain, baseline, x);
		return;
	}

	float old_x = x;

	if (!owner())
		x += scroll();

	top_x = int(x);
	top_baseline = baseline;

	int const bl = baseline - ascent(bv, f) + dim_collapsed.ascent();

	if (inlined) {
		inset.draw(bv, f, baseline, x);
	} else {
		draw_collapsed(pain, bl, old_x);
		int const yy = bl + dim_collapsed.descent() + inset.ascent(bv, f);
		inset.draw(bv, f, yy, x);
		// contained inset may be shorter than the button
		if (x < top_x + button_length + TEXT_TO_INSET_OFFSET)
			x = top_x + button_length + TEXT_TO_INSET_OFFSET;
	}
}


void InsetCollapsable::draw(BufferView * bv, LyXFont const & f,
			    int baseline, float & x) const
{
	// by default, we are not inlined-drawing
	draw(bv, f, baseline, x, false);
}


Inset::EDITABLE InsetCollapsable::editable() const
{
	return collapsed_ ? IS_EDITABLE : HIGHLY_EDITABLE;
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
	bv->updateInset(this);
}


FuncRequest InsetCollapsable::adjustCommand(FuncRequest const & cmd)
{
	LyXFont font(LyXFont::ALL_SANE);
	FuncRequest cmd1 = cmd;
	cmd1.y = ascent(cmd.view(), font) + cmd.y -
	    (height_collapsed() + inset.ascent(cmd.view(), font));
	return cmd1;
}


void InsetCollapsable::lfunMouseRelease(FuncRequest const & cmd)
{
	bool ret = false;
	BufferView * bv = cmd.view();

	if (collapsed_ && cmd.button() != mouse_button::button3) {
		collapsed_ = false;
		inset.setUpdateStatus(bv, InsetText::FULL);
		bv->updateInset(this);
		bv->buffer()->markDirty();
		return;
	}

	if ((cmd.button() != mouse_button::button3) && (cmd.x < button_length) &&
	    (cmd.y >= button_top_y) && (cmd.y <= button_bottom_y))
	{
		if (collapsed_) {
			collapsed_ = false;
			inset.setUpdateStatus(bv, InsetText::FULL);
			bv->updateInset(this);
			bv->buffer()->markDirty();
		} else {
			collapsed_ = true;
			bv->unlockInset(this);
			bv->updateInset(this);
			bv->buffer()->markDirty();
		}
	} else if (!collapsed_ && (cmd.y > button_bottom_y)) {
		ret = (inset.localDispatch(adjustCommand(cmd)) == DISPATCHED);
	}
	if (cmd.button() == mouse_button::button3 && !ret)
		showInsetDialog(bv);
}


int InsetCollapsable::latex(Buffer const * buf, ostream & os,
			    LatexRunParams const & runparams) const
{
	return inset.latex(buf, os, runparams);
}


int InsetCollapsable::ascii(Buffer const * buf, ostream & os, int ll) const
{
	return inset.ascii(buf, os, ll);
}


int InsetCollapsable::linuxdoc(Buffer const * buf, ostream & os) const
{
	return inset.linuxdoc(buf, os);
}


int InsetCollapsable::docbook(Buffer const * buf, ostream & os, bool mixcont) const
{
	return inset.docbook(buf, os, mixcont);
}


void InsetCollapsable::update(BufferView * bv, bool reinit)
{
	if (in_update) {
		if (reinit && owner()) {
			owner()->update(bv, true);
		}
		return;
	}
	in_update = true;
	inset.update(bv, reinit);
	if (reinit && owner()) {
		owner()->update(bv, true);
	}
	in_update = false;
}


Inset::RESULT InsetCollapsable::localDispatch(FuncRequest const & cmd)
{
	//lyxerr << "InsetCollapsable::localDispatch: " << cmd.action << "\n";
	BufferView * bv = cmd.view();
	switch (cmd.action) {
		case LFUN_INSET_EDIT: {
			if (!cmd.argument.empty()) {
				UpdatableInset::localDispatch(cmd);
				if (collapsed_) {
					collapsed_ = false;
					if (bv->lockInset(this)) {
						inset.setUpdateStatus(bv, InsetText::FULL);
						bv->updateInset(this);
						bv->buffer()->markDirty();
						inset.localDispatch(cmd);
						first_after_edit = true;
					}
				} else {
					if (bv->lockInset(this))
						inset.localDispatch(cmd);
				}
				return DISPATCHED;
			}

#ifdef WITH_WARNINGS
#warning Fix this properly in BufferView_pimpl::workAreaButtonRelease
#endif
			if (cmd.button() == mouse_button::button3)
				return DISPATCHED;

			UpdatableInset::localDispatch(cmd);

			if (collapsed_) {
				collapsed_ = false;
				// set this only here as it should be recollapsed only if
				// it was already collapsed!
				first_after_edit = true;
				if (!bv->lockInset(this))
					return DISPATCHED;
				bv->updateInset(this);
				bv->buffer()->markDirty();
				inset.localDispatch(cmd);
			} else {
				FuncRequest cmd1 = cmd;
				if (!bv->lockInset(this))
					return DISPATCHED;
				if (cmd.y <= button_bottom_y) {
					cmd1.y = 0;
				} else {
					LyXFont font(LyXFont::ALL_SANE);
					cmd1.y = ascent(bv, font) + cmd.y -
						(height_collapsed() + inset.ascent(bv, font));
				}
				inset.localDispatch(cmd);
			}
			return DISPATCHED;
		}

		case LFUN_MOUSE_PRESS:
			if (!collapsed_ && cmd.y > button_bottom_y)
				inset.localDispatch(adjustCommand(cmd));
			return DISPATCHED;

		case LFUN_MOUSE_MOTION:
			if (!collapsed_ && cmd.y > button_bottom_y)
				inset.localDispatch(adjustCommand(cmd));
			return DISPATCHED;

		case LFUN_MOUSE_RELEASE:
			lfunMouseRelease(cmd);
			return DISPATCHED;

		default:
			UpdatableInset::RESULT result = inset.localDispatch(cmd);
			if (result >= FINISHED)
				bv->unlockInset(this);
			first_after_edit = false;
			return result;
	}
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


int InsetCollapsable::insetInInsetY() const
{
	return inset.insetInInsetY() - (top_baseline - inset.y());
}


void InsetCollapsable::validate(LaTeXFeatures & features) const
{
	inset.validate(features);
}


void InsetCollapsable::getCursor(BufferView & bv, int & x, int & y) const
{
	inset.getCursor(bv, x, y);
}


void InsetCollapsable::getCursorPos(BufferView * bv, int & x, int & y) const
{
	inset.getCursorPos(bv, x , y);
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


ParagraphList * InsetCollapsable::getParagraphs(int i) const
{
	return inset.getParagraphs(i);
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
	bv->updateInset(this);
}


void InsetCollapsable::close(BufferView * bv) const
{
	if (collapsed_)
		return;

	collapsed_ = true;
	bv->updateInset(const_cast<InsetCollapsable *>(this));
}


void InsetCollapsable::setLabel(string const & l) const
{
	label = l;
}


void InsetCollapsable::markErased()
{
	inset.markErased();
}


bool InsetCollapsable::nextChange(BufferView * bv, lyx::pos_type & length)
{
	bool found = inset.nextChange(bv, length);

	if (first_after_edit && !found)
		close(bv);
	else if (!found)
		first_after_edit = false;
	return found;
}


bool InsetCollapsable::searchForward(BufferView * bv, string const & str,
				     bool cs, bool mw)
{
	bool found = inset.searchForward(bv, str, cs, mw);
	if (first_after_edit && !found)
		close(bv);
	else if (!found)
		first_after_edit = false;
	return found;
}


bool InsetCollapsable::searchBackward(BufferView * bv, string const & str,
				      bool cs, bool mw)
{
	bool found = inset.searchBackward(bv, str, cs, mw);
	if (first_after_edit && !found)
		close(bv);
	else if (!found)
		first_after_edit = false;
	return found;
}


WordLangTuple const
InsetCollapsable::selectNextWordToSpellcheck(BufferView * bv, float & value) const
{
	WordLangTuple word = inset.selectNextWordToSpellcheck(bv, value);
	if (first_after_edit && word.word().empty())
		close(bv);
	first_after_edit = false;
	return word;
}


void InsetCollapsable::addPreview(grfx::PreviewLoader & loader) const
{
	inset.addPreview(loader);
}


void InsetCollapsable::cache(BufferView * bv) const
{
	view_ = bv->owner()->view();
}


BufferView * InsetCollapsable::view() const
{
	return view_.lock().get();
}
