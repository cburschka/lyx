/**
 * \file insetert.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */
#include <config.h>

#include "insetert.h"
#include "insettext.h"

#include "buffer.h"
#include "BufferView.h"
#include "debug.h"
#include "funcrequest.h"
#include "gettext.h"
#include "language.h"
#include "lyxfont.h"
#include "lyxlex.h"
#include "lyxrow.h"
#include "lyxtext.h"
#include "WordLangTuple.h"

#include "frontends/Alert.h"
#include "frontends/Dialogs.h"
#include "frontends/LyXView.h"

#include "support/LOstream.h"
#include "support/LAssert.h"
#include "support/tostr.h"

using namespace lyx::support;

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


InsetERT::InsetERT(InsetERT const & in)
	: InsetCollapsable(in), status_(in.status_)
{
	init();
}


InsetBase * InsetERT::clone() const
{
	return new InsetERT(*this);
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
		inset.paragraphs.begin()->insertChar(pos++, *cit, font);
	}
	// the init has to be after the initialization of the paragraph
	// because of the label settings (draw_label for ert insets).
	init();
}


InsetERT::~InsetERT()
{
	InsetERTMailer mailer(*this);
	mailer.hideDialog();
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
	LyXFont font(LyXFont::ALL_INHERIT, latex_language);
	font.setFamily(LyXFont::TYPEWRITER_FAMILY);
	font.setColor(LColor::latex);

	ParagraphList::iterator pit = inset.paragraphs.begin();
	ParagraphList::iterator pend = inset.paragraphs.end();
	for (; pit != pend; ++pit) {
		pos_type siz = pit->size();
		for (pos_type i = 0; i < siz; ++i) {
			pit->setFont(i, font);
		}
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
	string const layout(buf->params.getLyXTextClass().defaultLayoutName());
	ParagraphList::iterator par = inset.paragraphs.begin();
	ParagraphList::iterator end = inset.paragraphs.end();
	for (; par != end; ++par) {
		os << "\n\\layout " << layout << "\n";
		pos_type siz = par->size();
		for (pos_type i = 0; i < siz; ++i) {
			Paragraph::value_type c = par->getChar(i);
			switch (c) {
			case Paragraph::META_INSET:
				if (par->getInset(i)->lyxCode() != Inset::NEWLINE_CODE) {
					lyxerr << "Element is not allowed in insertERT"
					       << endl;
				} else {
					par->getInset(i)->write(buf, os);
				}
				break;

			case '\\':
				os << "\n\\backslash \n";
				break;
			default:
				os << c;
				break;
			}
		}
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
#ifdef WITH_WARNINGS
#warning FIXME. More UI stupidity...
#endif
	// if selectall is activated then the fontchange was an outside general
	// fontchange and this messages is not needed
	if (!selectall)
		Alert::error(_("Cannot change font"),
			   _("You cannot change font settings inside TeX code."));
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


Inset::EDITABLE InsetERT::editable() const
{
	if (status_ == Collapsed)
		return IS_EDITABLE;
	return HIGHLY_EDITABLE;
}


void InsetERT::lfunMousePress(FuncRequest const & cmd)
{
	if (status_ == Inlined)
		inset.localDispatch(cmd);
	else
		InsetCollapsable::localDispatch(cmd);
}


bool InsetERT::lfunMouseRelease(FuncRequest const & cmd)
{
	BufferView * bv = cmd.view();

	if (cmd.button() == mouse_button::button3) {
		showInsetDialog(bv);
		return true;
	}

	if (status_ != Inlined && (cmd.x >= 0) && (cmd.x < button_length) &&
	    (cmd.y >= button_top_y) && (cmd.y <= button_bottom_y)) {
		updateStatus(bv, true);
	} else {
		LyXFont font(LyXFont::ALL_SANE);
		FuncRequest cmd1 = cmd;
		cmd1.y = ascent(bv, font) + cmd.y - inset.ascent(bv, font);

		// inlined is special - the text appears above
		// button_bottom_y
		if (status_ == Inlined)
			inset.localDispatch(cmd1);
		else if (!collapsed_ && (cmd.y > button_bottom_y)) {
			cmd1.y -= height_collapsed();
			inset.localDispatch(cmd1);
		}
	}
	return false;
}


void InsetERT::lfunMouseMotion(FuncRequest const & cmd)
{
	if (status_ == Inlined)
		inset.localDispatch(cmd);
	else
		InsetCollapsable::localDispatch(cmd);
}


int InsetERT::latex(Buffer const *, ostream & os,
		    LatexRunParams const &) const
{
	ParagraphList::iterator par = inset.paragraphs.begin();
	ParagraphList::iterator end = inset.paragraphs.end();

	int lines = 0;
	while (par != end) {
		pos_type siz = par->size();
		for (pos_type i = 0; i < siz; ++i) {
			// ignore all struck out text
			if (isDeletedText(*par, i))
				continue;

			if (par->isNewline(i)) {
				os << '\n';
				++lines;
			} else {
				os << par->getChar(i);
			}
		}
		++par;
		if (par != end) {
			os << "\n";
			++lines;
		}
	}

	return lines;
}


int InsetERT::ascii(Buffer const *, ostream &, int /*linelen*/) const
{
	return 0;
}


int InsetERT::linuxdoc(Buffer const *, ostream & os) const
{
	ParagraphList::iterator par = inset.paragraphs.begin();
	ParagraphList::iterator end = inset.paragraphs.end();

	int lines = 0;
	while (par != end) {
		pos_type siz = par->size();
		for (pos_type i = 0; i < siz; ++i) {
			if (par->isNewline(i)) {
				os << '\n';
				++lines;
			} else {
				os << par->getChar(i);
			}
		}
		++par;
		if (par != end) {
			os << "\n";
			lines ++;
		}
	}

	return lines;
}


int InsetERT::docbook(Buffer const *, ostream & os, bool) const
{
	ParagraphList::iterator par = inset.paragraphs.begin();
	ParagraphList::iterator end = inset.paragraphs.end();

	int lines = 0;
	while (par != end) {
		pos_type siz = par->size();
		for (pos_type i = 0; i < siz; ++i) {
			if (par->isNewline(i)) {
				os << '\n';
				++lines;
			} else {
				os << par->getChar(i);
			}
		}
		++par;
		if (par != end) {
			os << "\n";
			lines ++;
		}
	}

	return lines;
}


Inset::RESULT InsetERT::localDispatch(FuncRequest const & cmd)
{
	Inset::RESULT result = UNDISPATCHED;
	BufferView * bv = cmd.view();

	if (inset.paragraphs.begin()->empty()) {
		set_latex_font(bv);
	}

	switch (cmd.action) {

	case LFUN_INSET_EDIT:
		if (cmd.button() == mouse_button::button3)
			break;
		if (status_ == Inlined) {
			if (!bv->lockInset(this))
				break;
			result = inset.localDispatch(cmd);
		} else {
			result = InsetCollapsable::localDispatch(cmd);
		}
		set_latex_font(bv);
		updateStatus(bv);
		break;

	case LFUN_INSET_MODIFY: {
		InsetERT::ERTStatus status_;
		InsetERTMailer::string2params(cmd.argument, status_);

		status(bv, status_);

		/* FIXME: I refuse to believe we have to live
		 * with ugliness like this ! Note that this
		 * rebreak *is* needed. Consider a change from
		 * Open (needfullrow) to Inlined (only the space
		 * taken by the text).
		 */
		inset.getLyXText(cmd.view())->fullRebreak();
		inset.update(cmd.view(), true);
		bv->updateInset(this);
		result = DISPATCHED;
	}
	break;

	case LFUN_MOUSE_PRESS:
		lfunMousePress(cmd);
		result = DISPATCHED;
		break;

	case LFUN_MOUSE_MOTION:
		lfunMouseMotion(cmd);
		result = DISPATCHED;
		break;

	case LFUN_MOUSE_RELEASE:
		lfunMouseRelease(cmd);
		result = DISPATCHED;
		break;

	case LFUN_LAYOUT:
		bv->owner()->setLayout(inset.paragraphs.begin()->layout()->name());
		result = DISPATCHED_NOUPDATE;
		break;

	default:
		result = InsetCollapsable::localDispatch(cmd);
	}

	switch (cmd.action) {
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
	pos_type const p_siz = inset.paragraphs.begin()->size();
	pos_type const n = min(max_length, p_siz);
	pos_type i = 0;
	pos_type j = 0;
	for(; i < n && j < p_siz; ++j) {
		if (inset.paragraphs.begin()->isInset(j))
			continue;
		la += inset.paragraphs.begin()->getChar(j);
		++i;
	}
	if (p_siz > 1 || (i > 0 && j < p_siz)) {
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
	LyXFont f(LyXFont::ALL_INHERIT, latex_language);
	font = f;
	font.setFamily(LyXFont::TYPEWRITER_FAMILY);
	font.setColor(LColor::latex);
#endif
	return true;
}


void InsetERT::metrics(MetricsInfo & mi, Dimension & dim) const
{
	if (inlined())
		inset.metrics(mi, dim);
	else
		InsetCollapsable::metrics(mi, dim);
}


void InsetERT::draw(PainterInfo & pi, int x, int y) const
{
	InsetCollapsable::draw(pi, x, y, inlined());
}


void InsetERT::set_latex_font(BufferView * /* bv */)
{
#ifdef SET_HARD_FONT
	LyXFont font(LyXFont::ALL_INHERIT, latex_language);

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
		switch (st) {
		case Inlined:
			if (bv)
				inset.setUpdateStatus(InsetText::INIT);
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
		if (bv) {
			bv->updateInset(const_cast<InsetERT *>(this));
			bv->buffer()->markDirty();
		}
	}
}


bool InsetERT::showInsetDialog(BufferView * bv) const
{
	InsetERTMailer mailer(const_cast<InsetERT &>(*this));
	mailer.showDialog(bv);
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


WordLangTuple const
InsetERT::selectNextWordToSpellcheck(BufferView * bv, float &) const
{
	bv->unlockInset(const_cast<InsetERT *>(this));
	return WordLangTuple();
}


void InsetERT::getDrawFont(LyXFont & font) const
{
	LyXFont f(LyXFont::ALL_INHERIT, latex_language);
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
	int rw = text->rows().begin()->width();
	if (!rw)
		rw = w;
	rw += 40;
	if (text->rows().size() == 1 && rw < w)
		return -1;
	return w;
}


void InsetERT::update(BufferView * bv, bool reinit)
{
	if (inset.need_update & InsetText::INIT ||
	    inset.need_update & InsetText::FULL) {
		setButtonLabel();
	}

	InsetCollapsable::update(bv, reinit);
}


string const InsetERTMailer::name_("ert");

InsetERTMailer::InsetERTMailer(InsetERT & inset)
	: inset_(inset)
{}


string const InsetERTMailer::inset2string() const
{
	return params2string(inset_.status());
}


void InsetERTMailer::string2params(string const & in,
				   InsetERT::ERTStatus & status)
{
	status = InsetERT::Collapsed;

	string name;
	string body = split(in, name, ' ');

	if (body.empty())
		return;

	status = static_cast<InsetERT::ERTStatus>(strToInt(body));
}


string const
InsetERTMailer::params2string(InsetERT::ERTStatus status)
{
	return name_ + ' ' + tostr(status);
}
