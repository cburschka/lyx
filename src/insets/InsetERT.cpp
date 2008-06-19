/**
 * \file InsetERT.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetERT.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Cursor.h"
#include "debug.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "Language.h"
#include "Color.h"
#include "LyXAction.h"
#include "Lexer.h"
#include "TextClass.h"
#include "MetricsInfo.h"
#include "ParagraphParameters.h"
#include "Paragraph.h"

#include "frontends/alert.h"

#include <sstream>


namespace lyx {

using support::token;

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
	Font font(Font::ALL_SANE);
	font.decSize();
	font.decSize();
	font.setColor(Color::latex);
	setLabelFont(font);
	text_.current_font.setLanguage(latex_language);
	text_.real_current_font.setLanguage(latex_language);
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


auto_ptr<Inset> InsetERT::doClone() const
{
	return auto_ptr<Inset>(new InsetERT(*this));
}


#if 0
InsetERT::InsetERT(BufferParams const & bp,
		   Language const *, string const & contents, CollapseStatus status)
	: InsetCollapsable(bp, status)
{
	Font font(Font::ALL_INHERIT, latex_language);
	paragraphs().begin()->insert(0, contents, font);

	// the init has to be after the initialization of the paragraph
	// because of the label settings (draw_label for ert insets).
	init();
}
#endif


InsetERT::~InsetERT()
{
	InsetERTMailer(*this).hideDialog();
}


void InsetERT::write(Buffer const & buf, ostream & os) const
{
	os << "ERT" << "\n";
	InsetCollapsable::write(buf, os);
}


void InsetERT::read(Buffer const & buf, Lexer & lex)
{
	InsetCollapsable::read(buf, lex);

	// Force default font
	// This avoids paragraphs in buffer language that would have a
	// foreign language after a document langauge change, and it ensures
	// that all new text in ERT gets the "latex" language, since new text
	// inherits the language from the last position of the existing text.
	// As a side effect this makes us also robust against bugs in LyX
	// that might lead to font changes in ERT in .lyx files.
	Font font(Font::ALL_INHERIT, latex_language);
	ParagraphList::iterator par = paragraphs().begin();
	ParagraphList::iterator const end = paragraphs().end();
	while (par != end) {
		pos_type siz = par->size();
		for (pos_type i = 0; i <= siz; ++i) {
			par->setFont(i, font);
		}
		++par;
	}
}


docstring const InsetERT::editMessage() const
{
	return _("Opened ERT Inset");
}


int InsetERT::latex(Buffer const &, odocstream & os,
		    OutputParams const &) const
{
	ParagraphList::const_iterator par = paragraphs().begin();
	ParagraphList::const_iterator end = paragraphs().end();

	int lines = 0;
	while (par != end) {
		pos_type siz = par->size();
		for (pos_type i = 0; i < siz; ++i) {
			// ignore all struck out text
			if (par->isDeleted(i))
				continue;

			os.put(par->getChar(i));
		}
		++par;
		if (par != end) {
			os << "\n";
			++lines;
		}
	}

	return lines;
}


int InsetERT::plaintext(Buffer const &, odocstream &,
			OutputParams const &) const
{
	return 0; // do not output TeX code
}


int InsetERT::docbook(Buffer const &, odocstream & os,
		      OutputParams const &) const
{
	ParagraphList::const_iterator par = paragraphs().begin();
	ParagraphList::const_iterator end = paragraphs().end();

	int lines = 0;
	while (par != end) {
		pos_type siz = par->size();
		for (pos_type i = 0; i < siz; ++i)
			os.put(par->getChar(i));
		++par;
		if (par != end) {
			os << "\n";
			++lines;
		}
	}

	return lines;
}


void InsetERT::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	//lyxerr << "\nInsetERT::doDispatch (begin): cmd: " << cmd << endl;
	switch (cmd.action) {

	case LFUN_QUOTE_INSERT: {
		// We need to bypass the fancy quotes in Text
		FuncRequest f(LFUN_SELF_INSERT, "\"");
		dispatch(cur, f);
		break;
	}
	case LFUN_INSET_MODIFY: {
		InsetCollapsable::CollapseStatus st;
		InsetERTMailer::string2params(to_utf8(cmd.argument()), st);
		setStatus(cur, st);
		break;
	}
	case LFUN_PASTE:
	case LFUN_CLIPBOARD_PASTE:
	case LFUN_PRIMARY_SELECTION_PASTE: {
		InsetCollapsable::doDispatch(cur, cmd);

		// Since we can only store plain text, we must reset all
		// attributes.
		// FIXME: Change only the pasted paragraphs

		BufferParams const & bp = cur.buffer().params();
		Layout_ptr const layout =
			bp.getTextClass().defaultLayout();
		Font font = layout->font;
		// ERT contents has always latex_language
		font.setLanguage(latex_language);
		ParagraphList::iterator const end = paragraphs().end();
		for (ParagraphList::iterator par = paragraphs().begin();
		     par != end; ++par) {
			// in case par had a manual label
			par->setBeginOfBody();
			pos_type const siz = par->size();
			for (pos_type i = 0; i < siz; ++i) {
				par->setFont(i, font);
			}
			par->params().clear();
		}
		break;
	}
	default:
		// Force any new text to latex_language
		// FIXME: This should only be necessary in init(), but
		// new paragraphs that are created by pressing enter at the
		// start of an existing paragraph get the buffer language
		// and not latex_language, so we take this brute force
		// approach.
		text_.current_font.setLanguage(latex_language);
		text_.real_current_font.setLanguage(latex_language);

		InsetCollapsable::doDispatch(cur, cmd);
		break;
	}
}


bool InsetERT::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action) {
		// suppress these
		case LFUN_ACCENT_ACUTE:
		case LFUN_ACCENT_BREVE:
		case LFUN_ACCENT_CARON:
		case LFUN_ACCENT_CEDILLA:
		case LFUN_ACCENT_CIRCLE:
		case LFUN_ACCENT_CIRCUMFLEX:
		case LFUN_ACCENT_DOT:
		case LFUN_ACCENT_GRAVE:
		case LFUN_ACCENT_HUNGARIAN_UMLAUT:
		case LFUN_ACCENT_MACRON:
		case LFUN_ACCENT_OGONEK:
		case LFUN_ACCENT_SPECIAL_CARON:
		case LFUN_ACCENT_TIE:
		case LFUN_ACCENT_TILDE:
		case LFUN_ACCENT_UMLAUT:
		case LFUN_ACCENT_UNDERBAR:
		case LFUN_ACCENT_UNDERDOT:
		case LFUN_APPENDIX:
		case LFUN_BREAK_LINE:
		case LFUN_CAPTION_INSERT:
		case LFUN_DEPTH_DECREMENT:
		case LFUN_DEPTH_INCREMENT:
		case LFUN_DOTS_INSERT:
		case LFUN_END_OF_SENTENCE_PERIOD_INSERT:
		case LFUN_ENVIRONMENT_INSERT:
		case LFUN_ERT_INSERT:
		case LFUN_FILE_INSERT:
		case LFUN_FLOAT_INSERT:
		case LFUN_FLOAT_WIDE_INSERT:
		case LFUN_WRAP_INSERT:
		case LFUN_FONT_BOLD:
		case LFUN_FONT_CODE:
		case LFUN_FONT_DEFAULT:
		case LFUN_FONT_EMPH:
		case LFUN_FONT_FREE_APPLY:
		case LFUN_FONT_FREE_UPDATE:
		case LFUN_FONT_NOUN:
		case LFUN_FONT_ROMAN:
		case LFUN_FONT_SANS:
		case LFUN_FONT_FRAK:
		case LFUN_FONT_ITAL:
		case LFUN_FONT_SIZE:
		case LFUN_FONT_STATE:
		case LFUN_FONT_UNDERLINE:
		case LFUN_FOOTNOTE_INSERT:
		case LFUN_HFILL_INSERT:
		case LFUN_HTML_INSERT:
		case LFUN_HYPHENATION_POINT_INSERT:
		case LFUN_LIGATURE_BREAK_INSERT:
		case LFUN_INDEX_INSERT:
		case LFUN_INDEX_PRINT:
		case LFUN_LABEL_INSERT:
		case LFUN_OPTIONAL_INSERT:
		case LFUN_BIBITEM_INSERT:
		case LFUN_LINE_INSERT:
		case LFUN_PAGEBREAK_INSERT:
		case LFUN_CLEARPAGE_INSERT:
		case LFUN_CLEARDOUBLEPAGE_INSERT:
		case LFUN_LANGUAGE:
		case LFUN_LAYOUT:
		case LFUN_LAYOUT_PARAGRAPH:
		case LFUN_LAYOUT_TABULAR:
		case LFUN_MARGINALNOTE_INSERT:
		case LFUN_MATH_DISPLAY:
		case LFUN_MATH_INSERT:
		case LFUN_MATH_MATRIX:
		case LFUN_MATH_MODE:
		case LFUN_MENU_OPEN:
		case LFUN_MENU_SEPARATOR_INSERT:
		case LFUN_BRANCH_INSERT:
		case LFUN_CHARSTYLE_INSERT:
		case LFUN_NOTE_INSERT:
		case LFUN_BOX_INSERT:
		case LFUN_NOTE_NEXT:
		case LFUN_PARAGRAPH_SPACING:
		case LFUN_LABEL_GOTO:
		case LFUN_REFERENCE_NEXT:
		case LFUN_SPACE_INSERT:
		case LFUN_SERVER_GOTO_FILE_ROW:
		case LFUN_SERVER_NOTIFY:
		case LFUN_SERVER_SET_XY:
		case LFUN_TABULAR_INSERT:
		case LFUN_TOC_INSERT:
		case LFUN_URL_INSERT:
		case LFUN_FLOAT_LIST:
		case LFUN_INSET_INSERT:
		case LFUN_PARAGRAPH_PARAMS:
		case LFUN_PARAGRAPH_PARAMS_APPLY:
		case LFUN_PARAGRAPH_UPDATE:
		case LFUN_NOMENCL_INSERT:
		case LFUN_NOMENCL_PRINT:
		case LFUN_NOACTION:
			status.enabled(false);
			return true;

		case LFUN_QUOTE_INSERT:
		case LFUN_INSET_MODIFY:
		case LFUN_PASTE:
		case LFUN_CLIPBOARD_PASTE:
		case LFUN_PRIMARY_SELECTION_PASTE:
			status.enabled(true);
			return true;

		// this one is difficult to get right. As a half-baked
		// solution, we consider only the first action of the sequence
		case LFUN_COMMAND_SEQUENCE: {
			// argument contains ';'-terminated commands
			string const firstcmd = token(to_utf8(cmd.argument()), ';', 0);
			FuncRequest func(lyxaction.lookupFunc(firstcmd));
			func.origin = cmd.origin;
			return getStatus(cur, func, status);
		}

		default:
			return InsetCollapsable::getStatus(cur, cmd, status);
	}
}


void InsetERT::setButtonLabel()
{
	// FIXME UNICODE
	setLabel(isOpen() ?  _("ERT") : getNewLabel(_("ERT")));
}


bool InsetERT::insetAllowed(Inset::Code /* code */) const
{
	return false;
}


bool InsetERT::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Font tmpfont = mi.base.font;
	getDrawFont(mi.base.font,
		mi.base.bv->buffer()->params().getFont());
	mi.base.font.realize(tmpfont);
	InsetCollapsable::metrics(mi, dim);
	mi.base.font = tmpfont;
	bool const changed = dim_ != dim;
	dim_ = dim;
	return changed;
}


void InsetERT::draw(PainterInfo & pi, int x, int y) const
{
	Font tmpfont = pi.base.font;
	getDrawFont(pi.base.font,
		pi.base.bv->buffer()->params().getFont());
	pi.base.font.realize(tmpfont);
	InsetCollapsable::draw(pi, x, y);
	pi.base.font = tmpfont;
}


bool InsetERT::showInsetDialog(BufferView * bv) const
{
	InsetERTMailer(const_cast<InsetERT &>(*this)).showDialog(bv);
	return true;
}


void InsetERT::getDrawFont(Font & font, Font bfont) const
{
	font = Font(Font::ALL_INHERIT, latex_language);
	font.setFamily(Font::TYPEWRITER_FAMILY);
	font.setColor(Color::latex);
	// use sensible size (e.g. in headings)
	if (status() != InsetCollapsable::Inlined)
		font.setSize(bfont.size());
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
	Lexer lex(0,0);
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


} // namespace lyx
