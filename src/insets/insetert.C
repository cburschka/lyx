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

#include "support/std_sstream.h"

using lyx::pos_type;

using std::endl;
using std::min;

using std::auto_ptr;
using std::istringstream;
using std::ostream;
using std::ostringstream;
using std::string;


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


InsetERT::InsetERT(BufferParams const & bp, CollapseStatus status)
	: InsetCollapsable(bp, status)
{
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
		   Language const * l, string const & contents, CollapseStatus status)
	: InsetCollapsable(bp, status)
{
	LyXFont font(LyXFont::ALL_INHERIT, l);
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


void InsetERT::write(Buffer const & buf, ostream & os) const
{
	os << "ERT" << "\n";
	InsetCollapsable::write(buf, os);
}


string const InsetERT::editMessage() const
{
	return _("Opened ERT Inset");
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


DispatchResult InsetERT::priv_dispatch(LCursor & cur, FuncRequest const & cmd)
{
	lyxerr << "\nInsetERT::priv_dispatch (begin): cmd: " << cmd << endl;
	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetCollapsable::CollapseStatus st;
		InsetERTMailer::string2params(cmd.argument, st);
		setStatus(st);
		return DispatchResult(true, true);
	}

	case LFUN_LAYOUT:
	case LFUN_BOLD:
	case LFUN_CODE:
	case LFUN_DEFAULT:
	case LFUN_EMPH:
	case LFUN_FREEFONT_APPLY:
	case LFUN_FREEFONT_UPDATE:
	case LFUN_NOUN:
	case LFUN_ROMAN:
	case LFUN_SANS:
	case LFUN_FRAK:
	case LFUN_ITAL:
	case LFUN_FONT_SIZE:
	case LFUN_FONT_STATE:
	case LFUN_UNDERLINE:
		return DispatchResult(true);

	default:
		return InsetCollapsable::priv_dispatch(cur, cmd);
	}
}


void InsetERT::setButtonLabel()
{
	setLabel(status() == Collapsed ? getNewLabel(_("ERT")) : _("ERT"));
}


bool InsetERT::insetAllowed(InsetOld::Code code) const
{
	return code == InsetOld::NEWLINE_CODE;
}


void InsetERT::metrics(MetricsInfo & mi, Dimension & dim) const
{
	LyXFont tmpfont = mi.base.font;
	getDrawFont(mi.base.font);
	InsetCollapsable::metrics(mi, dim);
	mi.base.font = tmpfont;
	dim_ = dim;
}


void InsetERT::draw(PainterInfo & pi, int x, int y) const
{
	LyXFont tmpfont = pi.base.font;
	getDrawFont(pi.base.font);
	InsetCollapsable::draw(pi, x, y);
	pi.base.font = tmpfont;
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
				   InsetCollapsable::CollapseStatus & status)
{
	status = InsetCollapsable::Collapsed;
	if (in.empty())
		return;

	istringstream data(in);
	LyXLex lex(0,0);
	lex.setStream(data);

	string name;
	lex >> name;
	if (name != name_)
		return print_mailer_error("InsetERTMailer", in, 1, name_);

	int s;
	lex >> s;
	if (lex)
		status = static_cast<InsetCollapsable::CollapseStatus>(s);
}


string const
InsetERTMailer::params2string(InsetCollapsable::CollapseStatus status)
{
	ostringstream data;
	data << name_ << ' ' << status;
	return data.str();
}
