/**
 * \file insetert.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetert.h"

#include "buffer.h"
#include "bufferparams.h"
#include "BufferView.h"
#include "debug.h"
#include "dispatchresult.h"
#include "funcrequest.h"
#include "gettext.h"
#include "language.h"
#include "LColor.h"
#include "lyxlex.h"
#include "metricsinfo.h"
#include "paragraph.h"

#include "frontends/Alert.h"
#include "frontends/LyXView.h"

#include "support/tostr.h"

using lyx::pos_type;

using lyx::support::split;
using lyx::support::strToInt;

using std::endl;
using std::min;
using std::string;
using std::auto_ptr;
using std::ostream;


void InsetERT::init()
{
	setButtonLabel();

	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	font.decSize();
	font.setColor(LColor::latex);
	setLabelFont(font);

	setInsetName("ERT");
}


InsetERT::InsetERT(BufferParams const & bp, bool collapsed)
	: InsetCollapsable(bp, collapsed)
{
	status_ = collapsed ? Collapsed : Open;
	init();
}


InsetERT::InsetERT(InsetERT const & in)
	: InsetCollapsable(in)
{
	init();
}


auto_ptr<InsetBase> InsetERT::clone() const
{
	return auto_ptr<InsetBase>(new InsetERT(*this));
}


InsetERT::InsetERT(BufferParams const & bp,
		   Language const * l, string const & contents, bool collapsed)
	: InsetCollapsable(bp, collapsed)
{
	status_ = collapsed ? Collapsed : Open;

	LyXFont font(LyXFont::ALL_INHERIT, l);
#ifdef SET_HARD_FONT
	font.setFamily(LyXFont::TYPEWRITER_FAMILY);
	font.setColor(LColor::latex);
#endif

	string::const_iterator cit = contents.begin();
	string::const_iterator end = contents.end();
	pos_type pos = 0;
	for (; cit != end; ++cit) {
		inset.paragraphs().begin()->insertChar(pos++, *cit, font);
	}
	// the init has to be after the initialization of the paragraph
	// because of the label settings (draw_label for ert insets).
	init();
}


InsetERT::~InsetERT()
{
	InsetERTMailer(*this).hideDialog();
}


void InsetERT::read(Buffer const & buf, LyXLex & lex)
{
	bool token_found = false;
	if (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token == "status") {
			lex.next();
			string const tmp_token = lex.getString();

			if (tmp_token == "Inlined") {
				status_ = Inlined;
			} else if (tmp_token == "Collapsed") {
				status_ = Collapsed;
			} else {
				// leave this as default!
				status_ = Open;
			}

			token_found = true;
		} else {
			lyxerr << "InsetERT::Read: Missing 'status'-tag!"
				   << endl;
			// take countermeasures
			lex.pushToken(token);
		}
	}
	inset.read(buf, lex);

#ifdef SET_HARD_FONT
	LyXFont font(LyXFont::ALL_INHERIT, latex_language);
	font.setFamily(LyXFont::TYPEWRITER_FAMILY);
	font.setColor(LColor::latex);

	ParagraphList::iterator pit = inset.paragraphs().begin();
	ParagraphList::iterator pend = inset.paragraphs().end();
	for (; pit != pend; ++pit) {
		pos_type siz = pit->size();
		for (pos_type i = 0; i < siz; ++i) {
			pit->setFont(i, font);
		}
	}
#endif

	if (!token_found) {
		if (isOpen())
			status_ = Open;
		else
			status_ = Collapsed;
	}
	setButtonLabel();
}


void InsetERT::write(Buffer const & buf, ostream & os) const
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

	os << getInsetName() << "\n" << "status "<< st << "\n";

	//inset.writeParagraphData(buf, os);
	string const layout(buf.params().getLyXTextClass().defaultLayoutName());
	ParagraphList::iterator par = inset.paragraphs().begin();
	ParagraphList::iterator end = inset.paragraphs().end();
	for (; par != end; ++par) {
		os << "\n\\begin_layout " << layout << "\n";
		pos_type siz = par->size();
		for (pos_type i = 0; i < siz; ++i) {
			Paragraph::value_type c = par->getChar(i);
			switch (c) {
			case Paragraph::META_INSET:
				if (par->getInset(i)->lyxCode() != InsetOld::NEWLINE_CODE) {
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
		os << "\n\\end_layout\n";
	}
}


string const InsetERT::editMessage() const
{
	return _("Opened ERT Inset");
}


bool InsetERT::insertInset(BufferView *, InsetOld *)
{
	return false;
}


void InsetERT::updateStatus(bool swap) const
{
	if (status_ != Inlined) {
		if (isOpen())
			status_ = swap ? Collapsed : Open;
		else
			status_ = swap ? Open : Collapsed;
		setButtonLabel();
	}
}


void InsetERT::lfunMousePress(FuncRequest const & cmd)
{
	if (status_ == Inlined)
		inset.dispatch(cmd);
	else {
		idx_type idx = 0;
		pos_type pos = 0;
		InsetCollapsable::priv_dispatch(cmd, idx, pos);
	}
}


bool InsetERT::lfunMouseRelease(FuncRequest const & cmd)
{
	BufferView * bv = cmd.view();

	if (cmd.button() == mouse_button::button3) {
		showInsetDialog(bv);
		return true;
	}

	if (status_ != Inlined && hitButton(cmd)) {
		updateStatus(true);
	} else {
		FuncRequest cmd1 = cmd;
#warning metrics?
		cmd1.y = ascent() + cmd.y - inset.ascent();

		// inlined is special - the text appears above
		if (status_ == Inlined)
			inset.dispatch(cmd1);
		else if (isOpen() && cmd.y > buttonDim().y2) {
			cmd1.y -= height_collapsed();
			inset.dispatch(cmd1);
		}
	}
	return false;
}


void InsetERT::lfunMouseMotion(FuncRequest const & cmd)
{
	if (status_ == Inlined)
		inset.dispatch(cmd);
	else {
		idx_type idx = 0;
		pos_type pos = 0;
		InsetCollapsable::priv_dispatch(cmd, idx, pos);
	}
}


int InsetERT::latex(Buffer const &, ostream & os,
		    OutputParams const &) const
{
	ParagraphList::iterator par = inset.paragraphs().begin();
	ParagraphList::iterator end = inset.paragraphs().end();

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


int InsetERT::plaintext(Buffer const &, ostream &,
		    OutputParams const & /*runparams*/) const
{
	return 0;
}


int InsetERT::linuxdoc(Buffer const &, ostream & os,
		       OutputParams const &)const
{
	ParagraphList::iterator par = inset.paragraphs().begin();
	ParagraphList::iterator end = inset.paragraphs().end();

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


int InsetERT::docbook(Buffer const &, ostream & os,
		      OutputParams const &) const
{
	ParagraphList::iterator par = inset.paragraphs().begin();
	ParagraphList::iterator end = inset.paragraphs().end();

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


void InsetERT::edit(BufferView * bv, bool left)
{
	if (status_ == Inlined) {
		inset.edit(bv, left);
	} else {
		InsetCollapsable::edit(bv, left);
	}
	setLatexFont(bv);
	updateStatus();
}


DispatchResult
InsetERT::priv_dispatch(FuncRequest const & cmd, idx_type & idx, pos_type & pos)
{
	BufferView * bv = cmd.view();

	if (inset.paragraphs().begin()->empty())
		setLatexFont(bv);

	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetERTMailer::string2params(cmd.argument, status_);
		setButtonLabel();
		bv->update();
		return DispatchResult(true, true);
	}

	case LFUN_MOUSE_PRESS:
		lfunMousePress(cmd);
		return DispatchResult(true, true);

	case LFUN_MOUSE_MOTION:
		lfunMouseMotion(cmd);
		return DispatchResult(true, true);

	case LFUN_MOUSE_RELEASE:
		lfunMouseRelease(cmd);
		return DispatchResult(true, true);

	case LFUN_LAYOUT:
		bv->owner()->setLayout(inset.paragraphs().begin()->layout()->name());
		return DispatchResult(true);

	case LFUN_BREAKPARAGRAPH:
	case LFUN_BREAKPARAGRAPHKEEPLAYOUT:
	case LFUN_BACKSPACE:
	case LFUN_BACKSPACE_SKIP:
	case LFUN_DELETE:
	case LFUN_DELETE_SKIP:
	case LFUN_DELETE_LINE_FORWARD:
	case LFUN_CUT:
		setLatexFont(bv);
		return InsetCollapsable::priv_dispatch(cmd, idx, pos);

	default:
		return InsetCollapsable::priv_dispatch(cmd, idx, pos);
	}
}


void InsetERT::setButtonLabel() const
{
	setLabel(status_ == Collapsed ? getNewLabel(_("ERT")) : _("ERT"));
}


bool InsetERT::insetAllowed(InsetOld::Code code) const
{
	return code == InsetOld::NEWLINE_CODE;
}


void InsetERT::metrics(MetricsInfo & mi, Dimension & dim) const
{
	InsetCollapsable::metrics(mi, dim);
	dim_ = dim;
}


void InsetERT::draw(PainterInfo & pi, int x, int y) const
{
	InsetCollapsable::draw(pi, x, y);
}


void InsetERT::setLatexFont(BufferView * /*bv*/)
{
#ifdef SET_HARD_FONT
	LyXFont font(LyXFont::ALL_INHERIT, latex_language);
	font.setFamily(LyXFont::TYPEWRITER_FAMILY);
	font.setColor(LColor::latex);
	inset.text_.setFont(bv, font, false);
#endif
}


void InsetERT::setStatus(CollapseStatus st)
{
	status_ = st;
	setButtonLabel();
}


bool InsetERT::showInsetDialog(BufferView * bv) const
{
	InsetERTMailer(const_cast<InsetERT &>(*this)).showDialog(bv);
	return true;
}


void InsetERT::getDrawFont(LyXFont & font) const
{
	font = LyXFont(LyXFont::ALL_INHERIT, latex_language);
	font.setFamily(LyXFont::TYPEWRITER_FAMILY);
	font.setColor(LColor::latex);
}


string const InsetERTMailer::name_("ert");

InsetERTMailer::InsetERTMailer(InsetERT & inset)
	: inset_(inset)
{}


string const InsetERTMailer::inset2string(Buffer const &) const
{
	return params2string(inset_.status());
}


void InsetERTMailer::string2params(string const & in,
				   InsetCollapsable::InsetCollapsable::CollapseStatus & status)
{
	status = InsetCollapsable::Collapsed;

	string name;
	string body = split(in, name, ' ');

	if (body.empty())
		return;

	status = static_cast<InsetCollapsable::CollapseStatus>(strToInt(body));
}


string const
InsetERTMailer::params2string(InsetCollapsable::CollapseStatus status)
{
	return name_ + ' ' + tostr(status);
}
