/**
 * \file insetcollapsable.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetcollapsable.h"

#include "buffer.h"
#include "BufferView.h"
#include "debug.h"
#include "lyxlex.h"
#include "funcrequest.h"
#include "metricsinfo.h"
#include "paragraph.h"
#include "WordLangTuple.h"

#include "frontends/font_metrics.h"
#include "frontends/Painter.h"
#include "frontends/LyXView.h"

#include "support/LAssert.h"

using namespace lyx::support;
using namespace lyx::graphics;

using std::endl;
using std::max;
using std::vector;
using std::ostream;


InsetCollapsable::InsetCollapsable(BufferParams const & bp, bool collapsed)
	: UpdatableInset(), collapsed_(collapsed), inset(bp),
	  button_dim(0, 0, 0, 0), label("Label"),
#if 0
	autocollapse(false),
#endif
	  in_update(false), first_after_edit(false)
{
	inset.setOwner(this);
	inset.setAutoBreakRows(true);
	inset.setDrawFrame(InsetText::ALWAYS);
	inset.setFrameColor(LColor::collapsableframe);
	setInsetName("Collapsable");
}


InsetCollapsable::InsetCollapsable(InsetCollapsable const & in)
	: UpdatableInset(in), collapsed_(in.collapsed_),
	  framecolor(in.framecolor), labelfont(in.labelfont), inset(in.inset),
	  button_dim(0, 0, 0, 0), label(in.label),
#if 0
	  autocollapse(in.autocollapse),
#endif
	  in_update(false), first_after_edit(false)
{
	inset.init(&(in.inset));
	inset.setOwner(this);
}


bool InsetCollapsable::insertInset(BufferView * bv, InsetOld * in)
{
	if (!insetAllowed(in->lyxCode())) {
		lyxerr << "InsetCollapsable::InsertInset: "
			"Unable to insert inset." << endl;
		return false;
	}
	return inset.insertInset(bv, in);
}


void InsetCollapsable::write(Buffer const & buf, ostream & os) const
{
	os << "collapsed " << (collapsed_ ? "true" : "false") << "\n";
	inset.writeParagraphData(buf, os);
}


void InsetCollapsable::read(Buffer const & buf, LyXLex & lex)
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
}


int InsetCollapsable::height_collapsed() const
{
	Dimension dim;
	font_metrics::buttonText(label, labelfont, dim.wid, dim.asc, dim.des);
	return dim.asc + dim.des;
}


void InsetCollapsable::metrics(MetricsInfo & mi, Dimension & dim) const
{
	//lyxerr << "InsetCollapsable::metrics:  width: " << mi.base.textwidth << endl;
	dimension_collapsed(dim);
	if (!collapsed_) {
		Dimension insetdim;
		inset.metrics(mi, insetdim);
		dim.des += insetdim.height() + TEXT_TO_BOTTOM_OFFSET;
		dim.wid = max(dim.wid, insetdim.wid);
	}
	dim_ = dim;
}


void InsetCollapsable::draw_collapsed(PainterInfo & pi, int x, int y) const
{
	pi.pain.buttonText(x, y, label, labelfont);
}


void InsetCollapsable::draw(PainterInfo & pi, int x, int y, bool inlined) const
{
	Assert(pi.base.bv);
	cache(pi.base.bv);

	Dimension dim_collapsed;
	dimension_collapsed(dim_collapsed);

	int const aa  = ascent();
	button_dim.x1 = 0;
	button_dim.x2 = dim_collapsed.width();
	button_dim.y1 = -aa;
	button_dim.y2 = -aa + dim_collapsed.height();

	if (!isOpen()) {
		draw_collapsed(pi, x, y);
		return;
	}

	int old_x = x;

	if (!owner())
		x += scroll();

	top_x = x;
	top_baseline = y;

	int const bl = y - aa + dim_collapsed.ascent();

	if (inlined) {
		inset.draw(pi, x, y);
	} else {
		draw_collapsed(pi, old_x, bl);
		inset.draw(pi, x, bl + dim_collapsed.descent() + inset.ascent());
	}
}


void InsetCollapsable::draw(PainterInfo & pi, int x, int y) const
{
	// by default, we are not inlined-drawing
	draw(pi, x, y, false);
}


InsetOld::EDITABLE InsetCollapsable::editable() const
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
	FuncRequest cmd1 = cmd;
	cmd1.y = ascent() + cmd.y - (height_collapsed() + inset.ascent());
	return cmd1;
}


void InsetCollapsable::lfunMouseRelease(FuncRequest const & cmd)
{
	bool ret = false;
	BufferView * bv = cmd.view();

	if (collapsed_ && cmd.button() != mouse_button::button3) {
		collapsed_ = false;
		bv->updateInset(this);
		bv->buffer()->markDirty();
		return;
	}

	if (cmd.button() != mouse_button::button3 && hitButton(cmd)) {
		if (collapsed_) {
			collapsed_ = false;
		} else {
			collapsed_ = true;
			bv->unlockInset(this);
		}
		bv->updateInset(this);
		bv->buffer()->markDirty();
	} else if (!collapsed_ && cmd.y > button_dim.y2) {
		ret = inset.localDispatch(adjustCommand(cmd)) == DISPATCHED;
	}
	if (cmd.button() == mouse_button::button3 && !ret)
		showInsetDialog(bv);
}


int InsetCollapsable::latex(Buffer const & buf, ostream & os,
			    LatexRunParams const & runparams) const
{
	return inset.latex(buf, os, runparams);
}


int InsetCollapsable::ascii(Buffer const & buf, ostream & os, int ll) const
{
	return inset.ascii(buf, os, ll);
}


int InsetCollapsable::linuxdoc(Buffer const & buf, ostream & os) const
{
	return inset.linuxdoc(buf, os);
}


int InsetCollapsable::docbook(Buffer const & buf, ostream & os, bool mixcont) const
{
	return inset.docbook(buf, os, mixcont);
}


bool InsetCollapsable::hitButton(FuncRequest const & cmd) const
{
	return button_dim.contains(cmd.x, cmd.y);
}


InsetOld::RESULT InsetCollapsable::localDispatch(FuncRequest const & cmd)
{
	//lyxerr << "InsetCollapsable::localDispatch: "
	//	<< cmd.action << " '" << cmd.argument << "'\n";
	BufferView * bv = cmd.view();
	switch (cmd.action) {
		case LFUN_INSET_EDIT: {
			if (!cmd.argument.empty()) {
				UpdatableInset::localDispatch(cmd);
				if (collapsed_) {
					lyxerr << "branch collapsed_" << endl;
					collapsed_ = false;
					if (bv->lockInset(this)) {
						bv->updateInset(this);
						bv->buffer()->markDirty();
						inset.localDispatch(cmd);
						first_after_edit = true;
					}
				} else {
					lyxerr << "branch not collapsed_" << endl;
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
				if (cmd.y <= button_dim.y2) {
					cmd1.y = 0;
				} else {
					cmd1.y = ascent() + cmd.y - (height_collapsed() + inset.ascent());
				}
				inset.localDispatch(cmd);
			}
			return DISPATCHED;
		}

		case LFUN_MOUSE_PRESS:
			if (!collapsed_ && cmd.y > button_dim.y2)
				inset.localDispatch(adjustCommand(cmd));
			return DISPATCHED;

		case LFUN_MOUSE_MOTION:
			if (!collapsed_ && cmd.y > button_dim.y2)
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
	if (&inset == in)
		return const_cast<InsetCollapsable *>(this);
	return in;
}


UpdatableInset * InsetCollapsable::getFirstLockingInsetOfType(InsetOld::Code c)
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


void InsetCollapsable::getLabelList(std::vector<string> & list) const
{
	inset.getLabelList(list);
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


InsetOld * InsetCollapsable::getInsetFromID(int id_arg) const
{
	if (id_arg == id())
		return const_cast<InsetCollapsable *>(this);
	return inset.getInsetFromID(id_arg);
}


void InsetCollapsable::open(BufferView * bv)
{
	if (!collapsed_)
		return;

	collapsed_ = false;
	bv->updateInset(this);
}


void InsetCollapsable::close(BufferView * bv) const
{
	if (collapsed_)
		return;

	collapsed_ = true;
	bv->updateInset(this);
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


void InsetCollapsable::addPreview(PreviewLoader & loader) const
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
