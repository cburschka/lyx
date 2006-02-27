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
#include "cursor.h"
#include "debug.h"
#include "dispatchresult.h"
#include "funcrequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "language.h"
#include "LColor.h"
#include "LyXAction.h"
#include "lyxlex.h"
#include "lyxtextclass.h"
#include "metricsinfo.h"
#include "ParagraphParameters.h"
#include "paragraph.h"

#include "frontends/Alert.h"
#include "frontends/LyXView.h"

#include <sstream>

using lyx::pos_type;
using lyx::support::token;

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
	text_.current_font.setLanguage(latex_language);
	text_.real_current_font.setLanguage(latex_language);

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


auto_ptr<InsetBase> InsetERT::doClone() const
{
	return auto_ptr<InsetBase>(new InsetERT(*this));
}


#if 0
InsetERT::InsetERT(BufferParams const & bp,
		   Language const *, string const & contents, CollapseStatus status)
	: InsetCollapsable(bp, status)
{
	LyXFont font(LyXFont::ALL_INHERIT, latex_language);
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


void InsetERT::read(Buffer const & buf, LyXLex & lex)
{
	InsetCollapsable::read(buf, lex);

	// Force default font
	// This avoids paragraphs in buffer language that would have a
	// foreign language after a document langauge change, and it ensures
	// that all new text in ERT gets the "latex" language, since new text
	// inherits the language from the last position of the existing text.
	// As a side effect this makes us also robust against bugs in LyX
	// that might lead to font changes in ERT in .lyx files.
	LyXFont font(LyXFont::ALL_INHERIT, latex_language);
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


string const InsetERT::editMessage() const
{
	return _("Opened ERT Inset");
}


int InsetERT::latex(Buffer const &, ostream & os,
		    OutputParams const &) const
{
	ParagraphList::const_iterator par = paragraphs().begin();
	ParagraphList::const_iterator end = paragraphs().end();

	int lines = 0;
	while (par != end) {
		pos_type siz = par->size();
		for (pos_type i = 0; i < siz; ++i) {
			// ignore all struck out text
			if (isDeletedText(*par, i))
				continue;

			os << par->getChar(i);
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
		       OutputParams const &) const
{
	ParagraphList::const_iterator par = paragraphs().begin();
	ParagraphList::const_iterator end = paragraphs().end();

	int lines = 0;
	while (par != end) {
		pos_type siz = par->size();
		for (pos_type i = 0; i < siz; ++i)
			os << par->getChar(i);
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
	ParagraphList::const_iterator par = paragraphs().begin();
	ParagraphList::const_iterator end = paragraphs().end();

	int lines = 0;
	while (par != end) {
		pos_type siz = par->size();
		for (pos_type i = 0; i < siz; ++i)
			os << par->getChar(i);
		++par;
		if (par != end) {
			os << "\n";
			++lines;
		}
	}

	return lines;
}


void InsetERT::doDispatch(LCursor & cur, FuncRequest & cmd)
{
	//lyxerr << "\nInsetERT::doDispatch (begin): cmd: " << cmd << endl;
	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetCollapsable::CollapseStatus st;
		InsetERTMailer::string2params(cmd.argument, st);
		setStatus(cur, st);
		break;
	}
	case LFUN_PASTE:
	case LFUN_PASTESELECTION: {
		InsetCollapsable::doDispatch(cur, cmd);

		// Since we can only store plain text, we must reset all
		// attributes.
		// FIXME: Change only the pasted paragraphs

		BufferParams const & bp = cur.buffer().params();
		LyXLayout_ptr const layout =
			bp.getLyXTextClass().defaultLayout();
		LyXFont font = layout->font;
		// ERT contents has always latex_language
		font.setLanguage(latex_language);
		ParagraphList::iterator const end = paragraphs().end();
		for (ParagraphList::iterator par = paragraphs().begin();
		     par != end; ++par) {
			par->layout(layout);
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


bool InsetERT::getStatus(LCursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action) {
		// suppress these
		case LFUN_ACUTE:
		case LFUN_BREVE:
		case LFUN_CARON:
		case LFUN_CEDILLA:
		case LFUN_CIRCLE:
		case LFUN_CIRCUMFLEX:
		case LFUN_DOT:
		case LFUN_GRAVE:
		case LFUN_HUNG_UMLAUT:
		case LFUN_MACRON:
		case LFUN_OGONEK:
		case LFUN_SPECIAL_CARON:
		case LFUN_TIE:
		case LFUN_TILDE:
		case LFUN_UMLAUT:
		case LFUN_UNDERBAR:
		case LFUN_UNDERDOT:
		case LFUN_APPENDIX:
		case LFUN_BREAKLINE:
		case LFUN_INSET_CAPTION:
		case LFUN_DEPTH_MIN:
		case LFUN_DEPTH_PLUS:
		case LFUN_LDOTS:
		case LFUN_END_OF_SENTENCE:
		case LFUN_ENVIRONMENT_INSERT:
		case LFUN_INSET_ERT:
		case LFUN_FILE_INSERT:
		case LFUN_INSET_FLOAT:
		case LFUN_INSET_WIDE_FLOAT:
		case LFUN_INSET_WRAP:
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
		case LFUN_INSET_FOOTNOTE:
		case LFUN_HFILL:
		case LFUN_HTMLURL:
		case LFUN_HYPHENATION:
		case LFUN_LIGATURE_BREAK:
		case LFUN_INDEX_INSERT:
		case LFUN_INDEX_PRINT:
		case LFUN_INSERT_LABEL:
		case LFUN_INSET_OPTARG:
		case LFUN_INSERT_BIBITEM:
		case LFUN_INSERT_LINE:
		case LFUN_INSERT_PAGEBREAK:
		case LFUN_LANGUAGE:
		case LFUN_LAYOUT:
		case LFUN_LAYOUT_PARAGRAPH:
		case LFUN_LAYOUT_TABULAR:
		case LFUN_INSET_MARGINAL:
		case LFUN_MATH_DISPLAY:
		case LFUN_INSERT_MATH:
		case LFUN_INSERT_MATRIX:
		case LFUN_MATH_MODE:
		case LFUN_MENU_OPEN_BY_NAME:
		case LFUN_MENU_SEPARATOR:
		case LFUN_INSERT_BRANCH:
		case LFUN_INSERT_CHARSTYLE:
		case LFUN_INSERT_NOTE:
		case LFUN_INSERT_BOX:
		case LFUN_GOTONOTE:
		case LFUN_PARAGRAPH_SPACING:
		case LFUN_QUOTE:
		case LFUN_LABEL_GOTO:
		case LFUN_REFERENCE_GOTO:
		case LFUN_SPACE_INSERT:
		case LFUN_GOTOFILEROW:
		case LFUN_NOTIFY:
		case LFUN_SETXY:
		case LFUN_TABULAR_INSERT:
		case LFUN_TOC_INSERT:
		case LFUN_URL:
		case LFUN_FLOAT_LIST:
		case LFUN_INSET_INSERT:
		case LFUN_PARAGRAPH_APPLY:
		case LFUN_PARAGRAPH_UPDATE:
		case LFUN_NOACTION:
			status.enabled(false);
			return true;

		case LFUN_INSET_MODIFY:
		case LFUN_PASTE:
		case LFUN_PASTESELECTION:
			status.enabled(true);
			return true;

		// this one is difficult to get right. As a half-baked
		// solution, we consider only the first action of the sequence
		case LFUN_SEQUENCE: {
			// argument contains ';'-terminated commands
			string const firstcmd = token(cmd.argument, ';', 0);
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
	setLabel(isOpen() ?  _("ERT") : getNewLabel(_("ERT")));
}


bool InsetERT::insetAllowed(InsetBase::Code /* code */) const
{
	return false;
}


void InsetERT::metrics(MetricsInfo & mi, Dimension & dim) const
{
	LyXFont tmpfont = mi.base.font;
	getDrawFont(mi.base.font);
	mi.base.font.realize(tmpfont);
	InsetCollapsable::metrics(mi, dim);
	mi.base.font = tmpfont;
	dim_ = dim;
}


void InsetERT::draw(PainterInfo & pi, int x, int y) const
{
	LyXFont tmpfont = pi.base.font;
	getDrawFont(pi.base.font);
	pi.base.font.realize(tmpfont);
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
