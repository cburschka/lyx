/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *          Copyright 1998 The LyX Team.
 *
 *======================================================*/

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetert.h"
#include "gettext.h"
#include "lyxfont.h"
#include "language.h"
#include "buffer.h"
#include "BufferView.h"
#include "frontends/LyXView.h"
#include "lyxtext.h"
#include "debug.h"
#include "lyxtextclasslist.h"
#include "lyxrow.h"

#include "insets/insettext.h"

#include "frontends/Dialogs.h"
#include "frontends/Alert.h"

#include "support/LOstream.h"


using std::ostream;
using std::min;
using std::endl;

using lyx::pos_type;


void InsetERT::init()
{
	setButtonLabel();
	labelfont = LyXFont(LyXFont::ALL_SANE);
	labelfont.decSize();
	labelfont.decSize();
	labelfont.setColor(LColor::latex);
	setInsetName("ERT");
}


InsetERT::InsetERT(BufferParams const & bp, bool collapsed)
	: InsetCollapsable(bp, collapsed)
{
	if (collapsed)
		status_ = Collapsed;
	else
		status_ = Open;
	init();
}


InsetERT::InsetERT(InsetERT const & in, bool same_id)
	: InsetCollapsable(in, same_id), status_(in.status_)
{
	init();
}


Inset * InsetERT::clone(Buffer const &, bool same_id) const
{
	return new InsetERT(*const_cast<InsetERT *>(this), same_id);
}


InsetERT::InsetERT(BufferParams const & bp,
		   Language const * l, string const & contents, bool collapsed)
	: InsetCollapsable(bp, collapsed)
{
	if (collapsed)
		status_ = Collapsed;
	else
		status_ = Open;

	LyXFont font(LyXFont::ALL_INHERIT, l);
#ifdef SET_HARD_FONT
	font.setFamily(LyXFont::TYPEWRITER_FAMILY);
	font.setColor(LColor::latex);
#endif

	string::const_iterator cit = contents.begin();
	string::const_iterator end = contents.end();
	pos_type pos = 0;
	for (; cit != end; ++cit) {
		inset.paragraph()->insertChar(pos++, *cit, font);
	}
	// the init has to be after the initialization of the paragraph
	// because of the label settings (draw_label for ert insets).
	init();
}


InsetERT::~InsetERT()
{
	hideDialog();
}


void InsetERT::read(Buffer const * buf, LyXLex & lex)
{
	bool token_found = false;
	if (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token == "status") {
			lex.next();
			string const tmp_token = lex.getString();

			if (tmp_token == "Inlined") {
				status(0, Inlined);
			} else if (tmp_token == "Collapsed") {
				status(0, Collapsed);
			} else {
				// leave this as default!
				status(0, Open);
			}

			token_found = true;
		} else {
			lyxerr << "InsetERT::Read: Missing 'status'-tag!"
				   << endl;
			// take countermeasures
			lex.pushToken(token);
		}
	}
#if 0
#warning this should be really short lived only for compatibility to
#warning files written 07/08/2001 so this has to go before 1.2.0! (Jug)
	if (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token == "collapsed") {
			lex.next();
			collapsed_ = lex.getBool();
		} else {
			// Take countermeasures
			lex.pushToken(token);
		}
	}
#endif
	inset.read(buf, lex);

#ifdef SET_HARD_FONT
#ifndef INHERIT_LANG
	LyXFont font(LyXFont::ALL_INHERIT, latex_language);
#else
	LyXFont font(LyXFont::ALL_INHERIT);
#endif
	font.setFamily(LyXFont::TYPEWRITER_FAMILY);
	font.setColor(LColor::latex);
	Paragraph * par = inset.paragraph();
	while (par) {
		pos_type siz = par->size();
		for (pos_type i = 0; i < siz; ++i) {
			par->setFont(i, font);
		}
		par = par->next();
	}
#endif

	if (!token_found) {
		if (collapsed_) {
			status(0, Collapsed);
		} else {
			status(0, Open);
		}
	}
	setButtonLabel();
}


void InsetERT::write(Buffer const * buf, ostream & os) const
{
	string st;

	switch (status_) {
	case Open:
		st = "Open";
		break;
	case Collapsed:
		st = "Collapsed";
		break;
	case Inlined:
		st = "Inlined";
		break;
	}

	os << getInsetName() << "\n"
	   << "status "<< st << "\n";

	//inset.writeParagraphData(buf, os);
	string const layout(textclasslist[buf->params.textclass].defaultLayoutName());
	Paragraph * par = inset.paragraph();
	while (par) {
		os << "\n\\layout " << layout << "\n";
		pos_type siz = par->size();
		for (pos_type i = 0; i < siz; ++i) {
			Paragraph::value_type c = par->getChar(i);
			switch (c) {
			case Paragraph::META_INSET:
			case Paragraph::META_HFILL:
				lyxerr << "Element is not allowed in insertERT"
				       << endl;
			case Paragraph::META_NEWLINE:
				os << "\n\\newline \n";
				break;
			case '\\':
				os << "\n\\backslash \n";
				break;
			default:
				os << c;
				break;
			}
		}
		par = par->next();
	}
}


string const InsetERT::editMessage() const
{
	return _("Opened ERT Inset");
}


bool InsetERT::insertInset(BufferView *, Inset *)
{
	return false;
}


void InsetERT::setFont(BufferView *, LyXFont const &, bool, bool selectall)
{
	// if selectall is activated then the fontchange was an outside general
	// fontchange and this messages is not needed
	if (!selectall)
		Alert::alert(_("Impossible Operation!"),
			   _("Not permitted to change font-types inside ERT-insets!"),
			   _("Sorry."));
}


void InsetERT::updateStatus(BufferView * bv, bool swap) const
{
	if (status_ != Inlined) {
		if (collapsed_) {
			status(bv, swap ? Open : Collapsed);
		} else {
			status(bv, swap ? Collapsed : Open);
		}
	}
}

void InsetERT::edit(BufferView * bv, int x, int y, mouse_button::state button)
{
	if (button == mouse_button::button3)
		return;

	if (status_ == Inlined) {
		if (!bv->lockInset(this))
			return;
		inset.edit(bv, x, y, button);
	} else {
		InsetCollapsable::edit(bv, x, y, button);
	}
	set_latex_font(bv);
	updateStatus(bv);
}


Inset::EDITABLE InsetERT::editable() const
{
	if (status_ == Collapsed)
		return IS_EDITABLE;
	return HIGHLY_EDITABLE;
}


void InsetERT::edit(BufferView * bv, bool front)
{
	InsetCollapsable::edit(bv, front);
	updateStatus(0);
	set_latex_font(bv);
}




void InsetERT::insetButtonPress(BufferView * bv,
	int x, int y, mouse_button::state button)
{
	if (status_ == Inlined) {
		inset.insetButtonPress(bv, x, y, button);
	} else {
		InsetCollapsable::insetButtonPress(bv, x, y, button);
	}
}


bool InsetERT::insetButtonRelease(BufferView * bv, int x, int y, 
	mouse_button::state button)
{
	if (button == mouse_button::button3) {
		showInsetDialog(bv);
		return true;
	}

	if (status_ != Inlined && (x >= 0)  && (x < button_length) &&
	    (y >= button_top_y) &&  (y <= button_bottom_y)) {
		updateStatus(bv, true);
	} else {
		LyXFont font(LyXFont::ALL_SANE);
		int yy = ascent(bv, font) + y - inset.ascent(bv, font);

		// inlined is special - the text appears above
		// button_bottom_y
		if (status_ == Inlined) {
			inset.insetButtonRelease(bv, x, yy, button);
		} else if (!collapsed_ && (y > button_bottom_y)) {
			yy -= (ascent_collapsed() + descent_collapsed());
			inset.insetButtonRelease(bv, x, yy, button);
		}
	}
	return false;
}


void InsetERT::insetMotionNotify(BufferView * bv,
	int x, int y, mouse_button::state state)
{
	if (status_ == Inlined) {
		inset.insetMotionNotify(bv, x, y, state);
	} else {
		InsetCollapsable::insetMotionNotify(bv, x, y, state);
	}
}


int InsetERT::latex(Buffer const *, ostream & os, bool /*fragile*/,
		    bool /*free_spc*/) const
{
	Paragraph * par = inset.paragraph();
	int lines = 0;
	while (par) {
		pos_type siz = par->size();
		for (pos_type i = 0; i < siz; ++i) {
			Paragraph::value_type c = par->getChar(i);
			switch (c) {
			case Paragraph::META_NEWLINE:
				os << '\n';
				++lines;
				break;
			default:
				os << c;
				break;
			}
		}
		par = par->next();
		if (par) {
			os << "\n\n";
			lines += 2;
		}
	}

	return lines;
}


int InsetERT::ascii(Buffer const *,
		    ostream &, int /*linelen*/) const
{
	return 0;
}


int InsetERT::linuxdoc(Buffer const *, ostream & os) const
{
	Paragraph * par = inset.paragraph();
	int lines = 0;
	while (par) {
		pos_type siz = par->size();
		for (pos_type i = 0; i < siz; ++i) {
			Paragraph::value_type c = par->getChar(i);
			switch (c) {
			case Paragraph::META_NEWLINE:
				os << '\n';
				++lines;
				break;
			default:
				os << c;
				break;
			}
		}
		par = par->next();
		if (par) {
			os << "\n";
			lines ++;
		}
	}

	return lines;
}


int InsetERT::docbook(Buffer const *, ostream & os, bool) const
{
	Paragraph * par = inset.paragraph();
	int lines = 0;
	while (par) {
		pos_type siz = par->size();
		for (pos_type i = 0; i < siz; ++i) {
			Paragraph::value_type c = par->getChar(i);
			switch (c) {
			case Paragraph::META_NEWLINE:
				os << '\n';
				++lines;
				break;
			default:
				os << c;
				break;
			}
		}
		par = par->next();
		if (par) {
			os << "\n";
			lines ++;
		}
	}

	return lines;
}


UpdatableInset::RESULT
InsetERT::localDispatch(BufferView * bv, kb_action action, string const & arg)
{
	UpdatableInset::RESULT result = DISPATCHED_NOUPDATE;

	if (!inset.paragraph()->size()) {
		set_latex_font(bv);
	}

	switch (action) {
	case LFUN_LAYOUT:
		bv->owner()->setLayout(inset.paragraph()->layout());
		break;
	default:
		result = InsetCollapsable::localDispatch(bv, action, arg);
	}
	switch (action) {
	case LFUN_BREAKPARAGRAPH:
	case LFUN_BREAKPARAGRAPHKEEPLAYOUT:
	case LFUN_BACKSPACE:
	case LFUN_BACKSPACE_SKIP:
	case LFUN_DELETE:
	case LFUN_DELETE_SKIP:
	case LFUN_DELETE_LINE_FORWARD:
	case LFUN_CUT:
		set_latex_font(bv);
		break;

	default:
		break;
	}
	return result;
}


string const InsetERT::get_new_label() const
{
	string la;
	pos_type const max_length = 15;
	pos_type const p_siz = inset.paragraph()->size();
	pos_type const n = min(max_length, p_siz);
	int i = 0;
	int j = 0;
	for(; i < n && j < p_siz; ++j) {
		if (inset.paragraph()->isInset(j))
			continue;
		la += inset.paragraph()->getChar(j);
		++i;
	}
	if (inset.paragraph()->next() || (i > 0 && j < p_siz)) {
		la += "...";
	}
	if (la.empty()) {
		la = _("ERT");
	}
	return la;
}


void InsetERT::setButtonLabel() const
{
	if (status_ == Collapsed) {
		setLabel(get_new_label());
	} else {
		setLabel(_("ERT"));
	}
}


bool InsetERT::checkInsertChar(LyXFont & /* font */)
{
#ifdef SET_HARD_FONT
#ifndef INHERIT_LANG
	LyXFont f(LyXFont::ALL_INHERIT, latex_language);
#else
	LyXFont f(LyXFont::ALL_INHERIT);
#endif
	font = f;
	font.setFamily(LyXFont::TYPEWRITER_FAMILY);
	font.setColor(LColor::latex);
#endif
	return true;
}


int InsetERT::ascent(BufferView * bv, LyXFont const & font) const
{
	if (!inlined())
		return InsetCollapsable::ascent(bv, font);

	return inset.ascent(bv, font);
}


int InsetERT::descent(BufferView * bv, LyXFont const & font) const
{
	if (!inlined())
		return InsetCollapsable::descent(bv, font);

	return inset.descent(bv, font);
}


int InsetERT::width(BufferView * bv, LyXFont const & font) const
{
	if (!inlined())
		return InsetCollapsable::width(bv, font);

	return inset.width(bv, font);
}


void InsetERT::draw(BufferView * bv, LyXFont const & f,
		    int baseline, float & x, bool cleared) const
{
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

	if (inlined()) {
		inset.draw(bv, f, baseline, x, cleared);
	} else {
		draw_collapsed(pain, bl, old_x);
		inset.draw(bv, f,
				   bl + descent_collapsed() + inset.ascent(bv, f),
				   x, cleared);
	}
	need_update = NONE;
}


void InsetERT::set_latex_font(BufferView * /* bv */)
{
#ifdef SET_HARD_FONT
#ifndef INHERIT_LANG
	LyXFont font(LyXFont::ALL_INHERIT, latex_language);
#else
	LyXFont font(LyXFont::ALL_INHERIT);
#endif

	font.setFamily(LyXFont::TYPEWRITER_FAMILY);
	font.setColor(LColor::latex);

	inset.getLyXText(bv)->setFont(bv, font, false);
#endif
}


// attention this function can be called with bv == 0
void InsetERT::status(BufferView * bv, ERTStatus const st) const
{
	if (st != status_) {
		status_ = st;
		need_update = FULL;
		switch (st) {
		case Inlined:
			if (bv)
				inset.setUpdateStatus(bv, InsetText::INIT);
			break;
		case Open:
			collapsed_ = false;
			setButtonLabel();
			break;
		case Collapsed:
			collapsed_ = true;
			setButtonLabel();
			if (bv)
				bv->unlockInset(const_cast<InsetERT *>(this));
			break;
		}
		if (bv)
			bv->updateInset(const_cast<InsetERT *>(this), false);
	}
}


bool InsetERT::showInsetDialog(BufferView * bv) const
{
	bv->owner()->getDialogs()->showERT(const_cast<InsetERT *>(this));
	return true;
}


void InsetERT::open(BufferView * bv)
{
	if (!collapsed_)
		return;
	status(bv, Open);
}


void InsetERT::close(BufferView * bv) const
{
	if (status_ == Collapsed || status_ == Inlined)
		return;

	status(bv, Collapsed);
}


string const InsetERT::selectNextWordToSpellcheck(BufferView * bv,
						  float &) const
{
	bv->unlockInset(const_cast<InsetERT *>(this));
	return string();
}


void InsetERT::getDrawFont(LyXFont & font) const
{
#ifndef INHERIT_LANG
	LyXFont f(LyXFont::ALL_INHERIT, latex_language);
#else
	LyXFont f(LyXFont::ALL_INHERIT);
#endif
	font = f;
	font.setFamily(LyXFont::TYPEWRITER_FAMILY);
	font.setColor(LColor::latex);
}


int InsetERT::getMaxWidth(BufferView * bv, UpdatableInset const * in) const
{
	int w = InsetCollapsable::getMaxWidth(bv, in);
	if (status_ != Inlined || w < 0)
		return w;
	LyXText * text = inset.getLyXText(bv);
	int rw = text->firstRow()->width();
	if (!rw)
		rw = w;
	rw += 40;
	if (!text->firstRow()->next() && rw < w)
		return -1;
	return w;
}


void InsetERT::update(BufferView * bv, LyXFont const & font,
                      bool reinit)
{
	if (inset.need_update & InsetText::INIT ||
		inset.need_update & InsetText::FULL)
	{
		setButtonLabel();
	}
	InsetCollapsable::update(bv, font, reinit);
}
