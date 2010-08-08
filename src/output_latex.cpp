/**
 * \file output_latex.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "output_latex.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "Encoding.h"
#include "InsetList.h"
#include "Language.h"
#include "Layout.h"
#include "LyXRC.h"
#include "OutputParams.h"
#include "Paragraph.h"
#include "ParagraphParameters.h"
#include "TextClass.h"
#include "TexRow.h"
#include "VSpace.h"

#include "insets/InsetBibitem.h"
#include "insets/InsetArgument.h"

#include "support/lassert.h"
#include "support/debug.h"
#include "support/lstrings.h"

#include <boost/next_prior.hpp>
#include <list>

using namespace std;
using namespace lyx::support;


namespace lyx {

namespace {

enum OpenEncoding {
		none,
		inputenc,
		CJK
};

static int open_encoding_ = none;
static int cjk_inherited_ = 0;
Language const * prev_env_language_ = 0;


ParagraphList::const_iterator
TeXEnvironment(Buffer const & buf,
	       Text const & text,
	       ParagraphList::const_iterator pit,
	       odocstream & os, TexRow & texrow,
	       OutputParams const & runparams);


ParagraphList::const_iterator
TeXDeeper(Buffer const & buf,
	  Text const & text,
	  ParagraphList::const_iterator pit,
	  odocstream & os, TexRow & texrow,
	  OutputParams const & runparams)
{
	LYXERR(Debug::LATEX, "TeXDeeper...     " << &*pit);
	ParagraphList::const_iterator par = pit;

	ParagraphList const & paragraphs = text.paragraphs();

	bool const force_plain_layout = text.inset().forcePlainLayout();
	while (par != paragraphs.end() &&
					par->params().depth() == pit->params().depth()) {
		// FIXME This test should not be necessary.
		// We should perhaps issue an error if it is.
		Layout const & style = force_plain_layout
			? buf.params().documentClass().plainLayout() : par->layout();
		if (style.isEnvironment()) {
			par = TeXEnvironment(buf, text, par,
					     os, texrow, runparams);
		} else {
			par = TeXOnePar(buf, text, par,
					     os, texrow, runparams);
		}
	}
	LYXERR(Debug::LATEX, "TeXDeeper...done ");

	return par;
}


ParagraphList::const_iterator
TeXEnvironment(Buffer const & buf,
	       Text const & text,
	       ParagraphList::const_iterator pit,
	       odocstream & os, TexRow & texrow,
	       OutputParams const & runparams)
{
	LYXERR(Debug::LATEX, "TeXEnvironment...     " << &*pit);

	BufferParams const & bparams = buf.params();

	// FIXME This test should not be necessary.
	// We should perhaps issue an error if it is.
	Layout const & style = text.inset().forcePlainLayout() ?
		bparams.documentClass().plainLayout() : pit->layout();

	ParagraphList const & paragraphs = text.paragraphs();
	ParagraphList::const_iterator const priorpit =
		pit == paragraphs.begin() ? pit : boost::prior(pit);

	bool const use_prev_env_language = prev_env_language_ != 0
			&& priorpit->layout().isEnvironment()
			&& (priorpit->getDepth() > pit->getDepth()
			    || (priorpit->getDepth() == pit->getDepth()
				&& priorpit->layout() != pit->layout()));

	Encoding const * const prev_encoding = runparams.encoding;
	Language const * const par_language = pit->getParLanguage(bparams);
	Language const * const doc_language = bparams.language;
	Language const * const prev_par_language =
		(pit != paragraphs.begin())
		? (use_prev_env_language ? prev_env_language_
					 : priorpit->getParLanguage(bparams))
		: doc_language;
	if (par_language->babel() != prev_par_language->babel()) {

		if (!lyxrc.language_command_end.empty() &&
		    prev_par_language->babel() != doc_language->babel() &&
		    !prev_par_language->babel().empty()) {
			os << from_ascii(subst(
				lyxrc.language_command_end,
				"$$lang",
				prev_par_language->babel()))
			   // the '%' is necessary to prevent unwanted whitespace
			   << "%\n";
			texrow.newline();
		}

		if ((lyxrc.language_command_end.empty() ||
		     par_language->babel() != doc_language->babel()) &&
		    !par_language->babel().empty()) {
			os << from_ascii(subst(
				lyxrc.language_command_begin,
				"$$lang",
				par_language->babel()))
			   // the '%' is necessary to prevent unwanted whitespace
			   << "%\n";
			texrow.newline();
		}
	}

	bool leftindent_open = false;
	if (!pit->params().leftIndent().zero()) {
		os << "\\begin{LyXParagraphLeftIndent}{"
		   << from_ascii(pit->params().leftIndent().asLatexString())
		   << "}\n";
		texrow.newline();
		leftindent_open = true;
	}

	if (style.isEnvironment()) {
		os << "\\begin{" << from_ascii(style.latexname()) << '}';
		if (style.optargs != 0 || style.reqargs != 0) {
			int ret = latexArgInsets(*pit, os, runparams, style.reqargs, style.optargs);
			while (ret > 0) {
				texrow.newline();
				--ret;
			}
		}
		if (style.latextype == LATEX_LIST_ENVIRONMENT) {
			os << '{'
			   << pit->params().labelWidthString()
			   << "}\n";
		} else if (style.labeltype == LABEL_BIBLIO) {
			if (pit->params().labelWidthString().empty())
				os << '{' << bibitemWidest(buf, runparams) << "}\n";
			else
				os << '{'
				  << pit->params().labelWidthString()
				  << "}\n";
		} else
			os << from_ascii(style.latexparam()) << '\n';
		texrow.newline();
	}

	// in multilingual environments, the CJK tags have to be nested properly
	bool cjk_nested = false;
	if (par_language->encoding()->package() == Encoding::CJK &&
	    open_encoding_ != CJK && pit->isMultiLingual(bparams)) {
		if (prev_par_language->encoding()->package() == Encoding::CJK)
			os << "\\begin{CJK}{" << from_ascii(par_language->encoding()->latexName())
			   << "}{" << from_ascii(bparams.fontsCJK) << "}%\n";
		open_encoding_ = CJK;
		cjk_nested = true;
		texrow.newline();
	}

	ParagraphList::const_iterator par = pit;
	do {
		par = TeXOnePar(buf, text, par, os, texrow, runparams);

		if (par == paragraphs.end()) {
			// Make sure that the last paragraph is
			// correctly terminated (because TeXOnePar does
			// not add a \n in this case)
			os << '\n';
			texrow.newline();
		} else if (par->params().depth() > pit->params().depth()) {
			if (par->layout().isParagraph()) {
			  // Thinko!
			  // How to handle this? (Lgb)
			  //&& !suffixIs(os, "\n\n")

				// There should be at least one '\n' already
				// but we need there to be two for Standard
				// paragraphs that are depth-increment'ed to be
				// output correctly.  However, tables can
				// also be paragraphs so don't adjust them.
				// ARRae
				// Thinkee:
				// Will it ever harm to have one '\n' too
				// many? i.e. that we sometimes will have
				// three in a row. (Lgb)
				os << '\n';
				texrow.newline();
			}
			par = TeXDeeper(buf, text, par, os, texrow,
					runparams);
		}
	} while (par != paragraphs.end()
		 && par->layout() == pit->layout()
		 && par->params().depth() == pit->params().depth()
		 && par->params().leftIndent() == pit->params().leftIndent());

	if (open_encoding_ == CJK && cjk_nested) {
		// We need to close the encoding even if it does not change
		// to do correct environment nesting
		os << "\\end{CJK}\n";
		texrow.newline();
		open_encoding_ = none;
	}

	if (style.isEnvironment()) {
		os << "\\end{" << from_ascii(style.latexname()) << "}\n";
		texrow.newline();
		prev_env_language_ = par_language;
		if (runparams.encoding != prev_encoding) {
			runparams.encoding = prev_encoding;
			if (!bparams.useXetex)
				os << setEncoding(prev_encoding->iconvName());
		}
	}

	if (leftindent_open) {
		os << "\\end{LyXParagraphLeftIndent}\n";
		texrow.newline();
		prev_env_language_ = par_language;
		if (runparams.encoding != prev_encoding) {
			runparams.encoding = prev_encoding;
			if (!bparams.useXetex)
				os << setEncoding(prev_encoding->iconvName());
		}
	}

	if (par != paragraphs.end())
		LYXERR(Debug::LATEX, "TeXEnvironment...done " << &*par);

	return par;
}

} // namespace anon


int latexArgInsets(Paragraph const & par, odocstream & os,
	OutputParams const & runparams, unsigned int reqargs,
	unsigned int optargs)
{
	unsigned int totalargs = reqargs + optargs;
	list<InsetArgument const *> ilist;

	InsetList::const_iterator it = par.insetList().begin();
	InsetList::const_iterator end = par.insetList().end();
	for (; it != end; ++it) {
		if (it->inset->lyxCode() == ARG_CODE) {
			if (ilist.size() >= totalargs) {
				LYXERR0("WARNING: Found extra argument inset.");
				continue;
			}
			InsetArgument const * ins =
				static_cast<InsetArgument const *>(it->inset);
			ilist.push_back(ins);
		}
	}

	if (!reqargs && ilist.size() == 0)
		return 0;

	int lines = 0;
	bool const have_optional_args = ilist.size() > reqargs;
	if (have_optional_args) {
		unsigned int todo = ilist.size() - reqargs;
		for (unsigned int i = 0; i < todo; ++i) {
			InsetArgument const * ins = ilist.front();
			ilist.pop_front();
			lines += ins->latexArgument(os, runparams, true);
		}
	}

	// we should now have no more insets than there are required
	// arguments.
	LASSERT(ilist.size() <= reqargs, /* */);
	if (!reqargs)
		return lines;

	for (unsigned int i = 0; i < reqargs; ++i) {
		if (ilist.empty())
			// a required argument wasn't given, so we output {}
			os << "{}";
		else {
			InsetArgument const * ins = ilist.front();
			ilist.pop_front();
			lines += ins->latexArgument(os, runparams, false);
		}
	}
	return lines;
}


// FIXME: this should be anonymous
ParagraphList::const_iterator TeXOnePar(Buffer const & buf,
	  Text const & text,
	  ParagraphList::const_iterator const pit,
	  odocstream & os, TexRow & texrow,
	  OutputParams const & runparams_in,
	  string const & everypar,
	  int start_pos, int end_pos)
{
	LYXERR(Debug::LATEX, "TeXOnePar...     " << &*pit << " '"
		<< everypar << "'");

	BufferParams const & bparams = buf.params();
	// FIXME This check should not really be needed.
	// Perhaps we should issue an error if it is.
	Layout const style = text.inset().forcePlainLayout() ?
		bparams.documentClass().plainLayout() : pit->layout();

	ParagraphList const & paragraphs = text.paragraphs();
	ParagraphList::const_iterator const priorpit = 
		pit == paragraphs.begin() ? pit : boost::prior(pit);
	ParagraphList::const_iterator const nextpit = 
		pit == paragraphs.end() ? pit : boost::next(pit);

	if (style.inpreamble)
		return nextpit;

	OutputParams runparams = runparams_in;
	runparams.isLastPar = nextpit == paragraphs.end();

	bool const maintext = text.isMainText();
	// we are at the beginning of an inset and CJK is already open;
	// we count inheritation levels to get the inset nesting right.
	if (pit == paragraphs.begin() && !maintext
	    && (cjk_inherited_ > 0 || open_encoding_ == CJK)) {
		cjk_inherited_ += 1;
		open_encoding_ = none;
	}

	if (runparams.pass_thru) {
		int const dist = distance(paragraphs.begin(), pit);
		Font const outerfont = text.outerFont(dist);

		// No newline if only one paragraph in this lyxtext
		if (dist > 0) {
			os << '\n';
			texrow.newline();
		}

		pit->latex(bparams, outerfont, os, texrow,
		           runparams, start_pos, end_pos);
		return nextpit;
	}

	// This paragraph's language
	Language const * const par_language = pit->getParLanguage(bparams);
	// The document's language
	Language const * const doc_language = bparams.language;
	// The language that was in effect when the environment this paragraph is
	// inside of was opened
	Language const * const outer_language =
		(runparams.local_font != 0) ?
			runparams.local_font->language() : doc_language;

	// The previous language that was in effect is the language of the
	// previous paragraph, unless the previous paragraph is inside an
	// environment with nesting depth greater than (or equal to, but with
	// a different layout) the current one. If there is no previous
	// paragraph, the previous language is the outer language.
	bool const use_prev_env_language = prev_env_language_ != 0
			&& priorpit->layout().isEnvironment()
			&& (priorpit->getDepth() > pit->getDepth()
			    || (priorpit->getDepth() == pit->getDepth()
				&& priorpit->layout() != pit->layout()));
	Language const * const prev_language =
		(pit != paragraphs.begin())
		? (use_prev_env_language ? prev_env_language_
					 : priorpit->getParLanguage(bparams))
		: outer_language;

	if (par_language->babel() != prev_language->babel()
	    // check if we already put language command in TeXEnvironment()
	    && !(style.isEnvironment()
		 && (pit == paragraphs.begin() ||
		     (priorpit->layout() != pit->layout() &&
		      priorpit->getDepth() <= pit->getDepth())
		     || priorpit->getDepth() < pit->getDepth())))
	{
		if (!lyxrc.language_command_end.empty() &&
		    prev_language->babel() != outer_language->babel() &&
		    !prev_language->babel().empty())
		{
			os << from_ascii(subst(lyxrc.language_command_end,
				"$$lang",
				prev_language->babel()))
			   // the '%' is necessary to prevent unwanted whitespace
			   << "%\n";
			texrow.newline();
		}

		// We need to open a new language if we couldn't close the previous
		// one (because there's no language_command_end); and even if we closed
		// the previous one, if the current language is different than the
		// outer_language (which is currently in effect once the previous one
		// is closed).
		if ((lyxrc.language_command_end.empty() ||
		     par_language->babel() != outer_language->babel()) &&
		    !par_language->babel().empty()) {
			// If we're inside an inset, and that inset is within an \L or \R
			// (or equivalents), then within the inset, too, any opposite
			// language paragraph should appear within an \L or \R (in addition
			// to, outside of, the normal language switch commands).
			// This behavior is not correct for ArabTeX, though.
			if (	// not for ArabTeX
					(par_language->lang() != "arabic_arabtex" &&
					 outer_language->lang() != "arabic_arabtex") &&
					// are we in an inset?
					runparams.local_font != 0 &&
					// is the inset within an \L or \R?
					//
					// FIXME: currently, we don't check this; this means that
					// we'll have unnnecessary \L and \R commands, but that
					// doesn't seem to hurt (though latex will complain)
					//
					// is this paragraph in the opposite direction?
					runparams.local_font->isRightToLeft() !=
						par_language->rightToLeft()
				) {
				// FIXME: I don't have a working copy of the Arabi package, so
				// I'm not sure if the farsi and arabic_arabi stuff is correct
				// or not...
				if (par_language->lang() == "farsi")
					os << "\\textFR{";
				else if (outer_language->lang() == "farsi")
					os << "\\textLR{";
				else if (par_language->lang() == "arabic_arabi")
					os << "\\textAR{";
				else if (outer_language->lang() == "arabic_arabi")
					os << "\\textLR{";
				// remaining RTL languages currently is hebrew
				else if (par_language->rightToLeft())
					os << "\\R{";
				else
					os << "\\L{";
			}
			// With CJK, the CJK tag has to be closed first (see below)
			if (runparams.encoding->package() != Encoding::CJK) {
				os << from_ascii(subst(
					lyxrc.language_command_begin,
					"$$lang",
					par_language->babel()))
				   // the '%' is necessary to prevent unwanted whitespace
				   << "%\n";
				texrow.newline();
			}
		}
	}

	// Switch file encoding if necessary; no need to do this for "default"
	// encoding, since this only affects the position of the outputted
	// \inputencoding command; the encoding switch will occur when necessary
	if (bparams.inputenc == "auto" &&
	    runparams.encoding->package() != Encoding::none) {
		// Look ahead for future encoding changes.
		// We try to output them at the beginning of the paragraph,
		// since the \inputencoding command is not allowed e.g. in
		// sections. For this reason we only set runparams.moving_arg
		// after checking for the encoding change, otherwise the
		// change would be always avoided by switchEncoding().
		for (pos_type i = 0; i < pit->size(); ++i) {
			char_type const c = pit->getChar(i);
			Encoding const * const encoding =
				pit->getFontSettings(bparams, i).language()->encoding();
			if (encoding->package() != Encoding::CJK &&
			    runparams.encoding->package() == Encoding::inputenc &&
			    c < 0x80)
				continue;
			if (pit->isInset(i))
				break;
			// All characters before c are in the ASCII range, and
			// c is non-ASCII (but no inset), so change the
			// encoding to that required by the language of c.
			// With CJK, only add switch if we have CJK content at the beginning
			// of the paragraph
			if (encoding->package() != Encoding::CJK || i == 0) {
				pair<bool, int> enc_switch = switchEncoding(os, bparams, runparams,
					*encoding);
				// the following is necessary after a CJK environment in a multilingual
				// context (nesting issue).
				if (par_language->encoding()->package() == Encoding::CJK &&
				    open_encoding_ != CJK && cjk_inherited_ == 0) {
					os << "\\begin{CJK}{" << from_ascii(par_language->encoding()->latexName())
					   << "}{" << from_ascii(bparams.fontsCJK) << "}%\n";
					open_encoding_ = CJK;
					texrow.newline();
				}
				if (encoding->package() != Encoding::none && enc_switch.first) {
					if (enc_switch.second > 0) {
						// the '%' is necessary to prevent unwanted whitespace
						os << "%\n";
						texrow.newline();
					}
					// With CJK, the CJK tag had to be closed first (see above)
					if (runparams.encoding->package() == Encoding::CJK) {
						os << from_ascii(subst(
							lyxrc.language_command_begin,
							"$$lang",
							par_language->babel()))
						// the '%' is necessary to prevent unwanted whitespace
						<< "%\n";
						texrow.newline();
					}
					runparams.encoding = encoding;
				}
				break;
			}
		}
	}

	runparams.moving_arg |= style.needprotect;
	Encoding const * const prev_encoding = runparams.encoding;

	bool const useSetSpace = bparams.documentClass().provides("SetSpace");
	if (pit->allowParagraphCustomization()) {
		if (pit->params().startOfAppendix()) {
			os << "\\appendix\n";
			texrow.newline();
		}

		if (!pit->params().spacing().isDefault()
			&& (pit == paragraphs.begin()
			    || !priorpit->hasSameLayout(*pit)))
		{
			os << from_ascii(pit->params().spacing().writeEnvirBegin(useSetSpace))
			    << '\n';
			texrow.newline();
		}

		if (style.isCommand()) {
			os << '\n';
			texrow.newline();
		}
	}

	switch (style.latextype) {
	case LATEX_COMMAND:
		os << '\\' << from_ascii(style.latexname());

		// Separate handling of optional argument inset.
		if (style.optargs != 0 || style.reqargs != 0) {
			int ret = latexArgInsets(*pit, os, runparams, style.reqargs, style.optargs);
			while (ret > 0) {
				texrow.newline();
				--ret;
			}
		}
		else
			os << from_ascii(style.latexparam());
		break;
	case LATEX_ITEM_ENVIRONMENT:
	case LATEX_LIST_ENVIRONMENT:
		os << "\\item ";
		break;
	case LATEX_BIB_ENVIRONMENT:
		// ignore this, the inset will write itself
		break;
	default:
		break;
	}

	Font const outerfont = text.outerFont(distance(paragraphs.begin(), pit));

	// FIXME UNICODE
	os << from_utf8(everypar);
	pit->latex(bparams, outerfont, os, texrow,
						 runparams, start_pos, end_pos);

	// Make sure that \\par is done with the font of the last
	// character if this has another size as the default.
	// This is necessary because LaTeX (and LyX on the screen)
	// calculates the space between the baselines according
	// to this font. (Matthias)
	//
	// Is this really needed ? (Dekel)
	// We do not need to use to change the font for the last paragraph
	// or for a command.

	Font const font = pit->empty()
		 ? pit->getLayoutFont(bparams, outerfont)
		 : pit->getFont(bparams, pit->size() - 1, outerfont);

	bool const is_command = style.isCommand();

	if (style.resfont.size() != font.fontInfo().size()
	    && nextpit != paragraphs.end()
	    && !is_command) {
		os << '{';
		os << "\\" << from_ascii(font.latexSize()) << " \\par}";
	} else if (is_command) {
		os << '}';
		if (runparams.encoding != prev_encoding) {
			runparams.encoding = prev_encoding;
			if (!bparams.useXetex)
				os << setEncoding(prev_encoding->iconvName());
		}
	}

	bool pending_newline = false;
	switch (style.latextype) {
	case LATEX_ITEM_ENVIRONMENT:
	case LATEX_LIST_ENVIRONMENT:
		if (nextpit != paragraphs.end()
		    && (pit->params().depth() < nextpit->params().depth()))
			pending_newline = true;
		break;
	case LATEX_ENVIRONMENT: {
		// if its the last paragraph of the current environment
		// skip it otherwise fall through
		if (nextpit != paragraphs.end() && 
		    (nextpit->layout() != pit->layout()
		     || nextpit->params().depth() != pit->params().depth()))
			break;
	}

	// fall through possible
	default:
		// we don't need it for the last paragraph!!!
		if (nextpit != paragraphs.end())
			pending_newline = true;
	}

	if (pit->allowParagraphCustomization()) {
		if (!pit->params().spacing().isDefault()
			&& (nextpit == paragraphs.end() || !nextpit->hasSameLayout(*pit)))
		{
			if (pending_newline) {
				os << '\n';
				texrow.newline();
			}
			os << from_ascii(pit->params().spacing().writeEnvirEnd(useSetSpace));
			pending_newline = true;
		}
	}

	// Closing the language is needed for the last paragraph; it is also
	// needed if we're within an \L or \R that we may have opened above (not
	// necessarily in this paragraph) and are about to close.
	bool closing_rtl_ltr_environment =
		// not for ArabTeX
		(par_language->lang() != "arabic_arabtex" &&
		 outer_language->lang() != "arabic_arabtex") &&
		// have we opened and \L or \R environment?
		runparams.local_font != 0 &&
		runparams.local_font->isRightToLeft() != par_language->rightToLeft() &&
		// are we about to close the language?
		((nextpit != paragraphs.end() &&
		  par_language->babel() !=
		    (nextpit->getParLanguage(bparams))->babel()) ||
		  (nextpit == paragraphs.end() &&
		    par_language->babel() != outer_language->babel()));

	if (closing_rtl_ltr_environment || (nextpit == paragraphs.end()
	    && par_language->babel() != outer_language->babel())) {
		// Since \selectlanguage write the language to the aux file,
		// we need to reset the language at the end of footnote or
		// float.

		if (pending_newline) {
			os << '\n';
			texrow.newline();
		}
		// when the paragraph uses CJK, the language has to be closed earlier
		if (font.language()->encoding()->package() != Encoding::CJK) {
			if (lyxrc.language_command_end.empty()) {
				// If this is a child, we should restore the
				// master language after the last paragraph.
				Language const * const current_language =
					(nextpit == paragraphs.end()
					&& runparams.master_language)
						? runparams.master_language
						: outer_language;
				if (!current_language->babel().empty()) {
					os << from_ascii(subst(
						lyxrc.language_command_begin,
						"$$lang",
						current_language->babel()));
					pending_newline = true;
				}
			} else if (!par_language->babel().empty()) {
				os << from_ascii(subst(
					lyxrc.language_command_end,
					"$$lang",
					par_language->babel()));
				pending_newline = true;
			}
		}
	}
	if (closing_rtl_ltr_environment)
		os << "}";

	if (pending_newline) {
		os << '\n';
		texrow.newline();
	}

	// if this is a CJK-paragraph and the next isn't, close CJK
	// also if the next paragraph is a multilingual environment (because of nesting)
	if (nextpit != paragraphs.end() && open_encoding_ == CJK &&
	    (nextpit->getParLanguage(bparams)->encoding()->package() != Encoding::CJK ||
	     (nextpit->layout().isEnvironment() && nextpit->isMultiLingual(bparams)))
	     // inbetween environments, CJK has to be closed later (nesting!)
	     && (!style.isEnvironment() || !nextpit->layout().isEnvironment())) {
		os << "\\end{CJK}\n";
		open_encoding_ = none;
	}

	// If this is the last paragraph, close the CJK environment
	// if necessary. If it's an environment, we'll have to \end that first.
	if (nextpit == paragraphs.end() && !style.isEnvironment()) {
		switch (open_encoding_) {
			case CJK: {
				// do nothing at the end of child documents
				if (maintext && buf.masterBuffer() != &buf)
					break;
				// end of main text
				if (maintext) {
					os << '\n';
					texrow.newline();
					os << "\\end{CJK}\n";
					texrow.newline();
				// end of an inset
				} else
					os << "\\end{CJK}";
				open_encoding_ = none;
				break;
			}
			case inputenc: {
				os << "\\egroup";
				open_encoding_ = none;
				break;
			}
			case none:
			default:
				// do nothing
				break;
		}
	}

	// If this is the last paragraph, and a local_font was set upon entering
	// the inset, and we're using "auto" or "default" encoding, the encoding
	// should be set back to that local_font's encoding.
	// However, do not change the encoding when XeTeX is used.
	if (nextpit == paragraphs.end() && runparams_in.local_font != 0
	    && runparams_in.encoding != runparams_in.local_font->language()->encoding()
	    && (bparams.inputenc == "auto" || bparams.inputenc == "default")
	    && (!bparams.useXetex)) {
		runparams_in.encoding = runparams_in.local_font->language()->encoding();
		os << setEncoding(runparams_in.encoding->iconvName());
	}
	// Otherwise, the current encoding should be set for the next paragraph.
	else
		runparams_in.encoding = runparams.encoding;


	// we don't need a newline for the last paragraph!!!
	// Note from JMarc: we will re-add a \n explicitly in
	// TeXEnvironment, because it is needed in this case
	if (nextpit != paragraphs.end()) {
		Layout const & next_layout = nextpit->layout();
		if (style == next_layout
		    // no blank lines before environments!
		    || !next_layout.isEnvironment()
		    // unless there's a depth change
		    // FIXME What we really want to do here is put every \begin and \end
		    // tag on a new line (which was not the case with nested environments).
		    // But in the present state of play, we don't have access to the
		    // information whether the current TeX row is empty or not.
		    // For some ideas about how to fix this, see this thread:
		    // http://www.mail-archive.com/lyx-devel@lists.lyx.org/msg145787.html
		    || nextpit->params().depth() != pit->params().depth()) {
			os << '\n';
			texrow.newline();
		}
	}

	if (nextpit != paragraphs.end())
		LYXERR(Debug::LATEX, "TeXOnePar...done " << &*nextpit);

	return nextpit;
}


// LaTeX all paragraphs
void latexParagraphs(Buffer const & buf,
		     Text const & text,
		     odocstream & os,
		     TexRow & texrow,
		     OutputParams const & runparams,
		     string const & everypar)
{
	bool was_title = false;
	bool already_title = false;
	BufferParams const & bparams = buf.params();
	DocumentClass const & tclass = bparams.documentClass();
	ParagraphList const & paragraphs = text.paragraphs();
	ParagraphList::const_iterator par = paragraphs.begin();
	ParagraphList::const_iterator endpar = paragraphs.end();

	LASSERT(runparams.par_begin <= runparams.par_end, /**/);
	// if only part of the paragraphs will be outputed
	if (runparams.par_begin !=  runparams.par_end) {
		par = boost::next(paragraphs.begin(), runparams.par_begin);
		endpar = boost::next(paragraphs.begin(), runparams.par_end);
		// runparams will be passed to nested paragraphs, so
		// we have to reset the range parameters.
		const_cast<OutputParams&>(runparams).par_begin = 0;
		const_cast<OutputParams&>(runparams).par_end = 0;
	}

	bool const maintext = text.isMainText();
	bool const is_child = buf.masterBuffer() != &buf;

	// Open a CJK environment at the beginning of the main buffer
	// if the document's language is a CJK language
	// (but not in child documents)
	if (maintext && !is_child
	    && bparams.encoding().package() == Encoding::CJK) {
		os << "\\begin{CJK}{" << from_ascii(bparams.encoding().latexName())
		<< "}{" << from_ascii(bparams.fontsCJK) << "}%\n";
		texrow.newline();
		open_encoding_ = CJK;
	}
	// if "auto begin" is switched off, explicitly switch the
	// language on at start
	if (maintext && !lyxrc.language_auto_begin &&
	    !bparams.language->babel().empty()) {
		// FIXME UNICODE
		os << from_utf8(subst(lyxrc.language_command_begin,
					"$$lang",
					bparams.language->babel()))
			<< '\n';
	texrow.newline();
	}

	ParagraphList::const_iterator lastpar;
	// if only_body
	while (par != endpar) {
		lastpar = par;
		// FIXME This check should not be needed. We should
		// perhaps issue an error if it is.
		Layout const & layout = text.inset().forcePlainLayout() ?
				tclass.plainLayout() : par->layout();

		if (layout.intitle) {
			if (already_title) {
				lyxerr << "Error in latexParagraphs: You"
					" should not mix title layouts"
					" with normal ones." << endl;
			} else if (!was_title) {
				was_title = true;
				if (tclass.titletype() == TITLE_ENVIRONMENT) {
					os << "\\begin{"
							<< from_ascii(tclass.titlename())
							<< "}\n";
					texrow.newline();
				}
			}
		} else if (was_title && !already_title) {
			if (tclass.titletype() == TITLE_ENVIRONMENT) {
				os << "\\end{" << from_ascii(tclass.titlename())
						<< "}\n";
			}
			else {
				os << "\\" << from_ascii(tclass.titlename())
						<< "\n";
			}
			texrow.newline();
			already_title = true;
			was_title = false;
		}

		if (layout.isEnvironment() ||
					!par->params().leftIndent().zero()) {
			par = TeXEnvironment(buf, text, par, os,
								texrow, runparams);
		} else {
			par = TeXOnePar(buf, text, par, os, texrow,
					runparams, everypar);
		}
		if (distance(lastpar, par) >= distance(lastpar, endpar))
			break;
	}

	// It might be that we only have a title in this document
	if (was_title && !already_title) {
		if (tclass.titletype() == TITLE_ENVIRONMENT) {
			os << "\\end{" << from_ascii(tclass.titlename())
			    << "}\n";
		}
		else {
			os << "\\" << from_ascii(tclass.titlename())
			    << "\n";
				}
		texrow.newline();
	}

	// if "auto end" is switched off, explicitly close the language at the end
	// but only if the last par is in a babel language
	if (maintext && !lyxrc.language_auto_end && !bparams.language->babel().empty() &&
		lastpar->getParLanguage(bparams)->encoding()->package() != Encoding::CJK) {
		os << from_utf8(subst(lyxrc.language_command_end,
					"$$lang",
					bparams.language->babel()))
			<< '\n';
		texrow.newline();
	}

	// If the last paragraph is an environment, we'll have to close
	// CJK at the very end to do proper nesting.
	if (maintext && !is_child && open_encoding_ == CJK) {
		os << "\\end{CJK}\n";
		texrow.newline();
		open_encoding_ = none;
	}

	// reset inherited encoding
	if (cjk_inherited_ > 0) {
		cjk_inherited_ -= 1;
		if (cjk_inherited_ == 0)
			open_encoding_ = CJK;
	}
}


pair<bool, int> switchEncoding(odocstream & os, BufferParams const & bparams,
		   OutputParams const & runparams, Encoding const & newEnc,
		   bool force)
{
	Encoding const & oldEnc = *runparams.encoding;
	bool moving_arg = runparams.moving_arg;
	if (!force && ((bparams.inputenc != "auto" && bparams.inputenc != "default")
		|| moving_arg))
		return make_pair(false, 0);

	// Do nothing if the encoding is unchanged.
	if (oldEnc.name() == newEnc.name())
		return make_pair(false, 0);

	// FIXME We ignore encoding switches from/to encodings that do
	// neither support the inputenc package nor the CJK package here.
	// This does of course only work in special cases (e.g. switch from
	// tis620-0 to latin1, but the text in latin1 contains ASCII only),
	// but it is the best we can do
	if (oldEnc.package() == Encoding::none
		|| newEnc.package() == Encoding::none)
		return make_pair(false, 0);

	LYXERR(Debug::LATEX, "Changing LaTeX encoding from "
		<< oldEnc.name() << " to " << newEnc.name());
	os << setEncoding(newEnc.iconvName());
	if (bparams.inputenc == "default")
		return make_pair(true, 0);

	docstring const inputenc_arg(from_ascii(newEnc.latexName()));
	switch (newEnc.package()) {
		case Encoding::none:
		case Encoding::japanese:
			// shouldn't ever reach here, see above
			return make_pair(true, 0);
		case Encoding::inputenc: {
			int count = inputenc_arg.length();
			if (oldEnc.package() == Encoding::CJK &&
			    open_encoding_ == CJK) {
				os << "\\end{CJK}";
				open_encoding_ = none;
				count += 9;
			}
			else if (oldEnc.package() == Encoding::inputenc &&
				 open_encoding_ == inputenc) {
				os << "\\egroup";
				open_encoding_ = none;
				count += 7;
			}
			if (runparams.local_font != 0
			    && 	oldEnc.package() == Encoding::CJK) {
				// within insets, \inputenc switches need
				// to be embraced within \bgroup...\egroup;
				// else CJK fails.
				os << "\\bgroup";
				count += 7;
				open_encoding_ = inputenc;
			}
			// with the japanese option, inputenc is omitted.
			if (runparams.use_japanese)
				return make_pair(true, count);
			os << "\\inputencoding{" << inputenc_arg << '}';
			return make_pair(true, count + 16);
		}
		case Encoding::CJK: {
			int count = inputenc_arg.length();
			if (oldEnc.package() == Encoding::CJK &&
			    open_encoding_ == CJK) {
				os << "\\end{CJK}";
				count += 9;
			}
			if (oldEnc.package() == Encoding::inputenc &&
			    open_encoding_ == inputenc) {
				os << "\\egroup";
				count += 7;
			}
			os << "\\begin{CJK}{" << inputenc_arg << "}{"
			   << from_ascii(bparams.fontsCJK) << "}";
			open_encoding_ = CJK;
			return make_pair(true, count + 15);
		}
	}
	// Dead code to avoid a warning:
	return make_pair(true, 0);

}

} // namespace lyx
