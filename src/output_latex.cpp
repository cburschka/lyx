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
#include "debug.h"
#include "Encoding.h"
#include "InsetList.h"
#include "Language.h"
#include "Layout.h"
#include "LyXRC.h"
#include "OutputParams.h"
#include "Paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "TextClass.h"
#include "TexRow.h"
#include "VSpace.h"

#include "insets/InsetBibitem.h"
#include "insets/InsetOptArg.h"

#include "support/lstrings.h"

#include <boost/next_prior.hpp>

namespace lyx {

using support::subst;

using std::endl;
using std::string;
using std::pair;
using std::make_pair;


namespace {

ParagraphList::const_iterator
TeXEnvironment(Buffer const & buf,
	       ParagraphList const & paragraphs,
	       ParagraphList::const_iterator pit,
	       odocstream & os, TexRow & texrow,
	       OutputParams const & runparams);

ParagraphList::const_iterator
TeXOnePar(Buffer const & buf,
	  ParagraphList const & paragraphs,
	  ParagraphList::const_iterator pit,
	  odocstream & os, TexRow & texrow,
	  OutputParams const & runparams,
	  string const & everypar = string());


ParagraphList::const_iterator
TeXDeeper(Buffer const & buf,
	  ParagraphList const & paragraphs,
	  ParagraphList::const_iterator pit,
	  odocstream & os, TexRow & texrow,
	  OutputParams const & runparams)
{
	LYXERR(Debug::LATEX, "TeXDeeper...     " << &*pit);
	ParagraphList::const_iterator par = pit;

	while (par != paragraphs.end() &&
		     par->params().depth() == pit->params().depth()) {
		if (par->layout()->isEnvironment()) {
			par = TeXEnvironment(buf, paragraphs, par,
					     os, texrow, runparams);
		} else {
			par = TeXOnePar(buf, paragraphs, par,
					     os, texrow, runparams);
		}
	}
	LYXERR(Debug::LATEX, "TeXDeeper...done ");

	return par;
}


ParagraphList::const_iterator
TeXEnvironment(Buffer const & buf,
	       ParagraphList const & paragraphs,
	       ParagraphList::const_iterator pit,
	       odocstream & os, TexRow & texrow,
	       OutputParams const & runparams)
{
	LYXERR(Debug::LATEX, "TeXEnvironment...     " << &*pit);

	BufferParams const & bparams = buf.params();

	LayoutPtr const & style = pit->layout();

	Language const * const par_language = pit->getParLanguage(bparams);
	Language const * const doc_language = bparams.language;
	Language const * const prev_par_language =
		(pit != paragraphs.begin())
		? boost::prior(pit)->getParLanguage(bparams)
		: doc_language;
	if (par_language->babel() != prev_par_language->babel()) {

		if (!lyxrc.language_command_end.empty() &&
		    prev_par_language->babel() != doc_language->babel() &&
		    !prev_par_language->babel().empty()) {
			os << from_ascii(subst(
				lyxrc.language_command_end,
				"$$lang",
				prev_par_language->babel()))
			   << '\n';
			texrow.newline();
		}

		if ((lyxrc.language_command_end.empty() ||
		     par_language->babel() != doc_language->babel()) &&
		    !par_language->babel().empty()) {
			os << from_ascii(subst(
				lyxrc.language_command_begin,
				"$$lang",
				par_language->babel()))
			   << '\n';
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

	if (style->isEnvironment()) {
		os << "\\begin{" << from_ascii(style->latexname()) << '}';
		if (style->optionalargs > 0) {
			int ret = latexOptArgInsets(buf, *pit, os, runparams,
						    style->optionalargs);
			while (ret > 0) {
				texrow.newline();
				--ret;
			}
		}
		if (style->latextype == LATEX_LIST_ENVIRONMENT) {
			os << '{'
			   << pit->params().labelWidthString()
			   << "}\n";
		} else if (style->labeltype == LABEL_BIBLIO) {
			// ale970405
			os << '{' << bibitemWidest(buf) << "}\n";
		} else
			os << from_ascii(style->latexparam()) << '\n';
		texrow.newline();
	}
	ParagraphList::const_iterator par = pit;
	do {
		par = TeXOnePar(buf, paragraphs, par, os, texrow, runparams);

		if (par == paragraphs.end()) {
			// Make sure that the last paragraph is
			// correctly terminated (because TeXOnePar does
			// not add a \n in this case)
			os << '\n';
			texrow.newline();
		} else if (par->params().depth() > pit->params().depth()) {
			if (par->layout()->isParagraph()) {
			  // Thinko!
			  // How to handle this? (Lgb)
			  //&& !suffixIs(os, "\n\n")
				  //) {

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
			par = TeXDeeper(buf, paragraphs, par, os, texrow,
					runparams);
		}
	} while (par != paragraphs.end()
		 && par->layout() == pit->layout()
		 && par->params().depth() == pit->params().depth()
		 && par->params().leftIndent() == pit->params().leftIndent());

	if (style->isEnvironment()) {
		os << "\\end{" << from_ascii(style->latexname()) << "}\n";
		texrow.newline();
	}

	if (leftindent_open) {
		os << "\\end{LyXParagraphLeftIndent}\n";
		texrow.newline();
	}

	if (par != paragraphs.end())
		LYXERR(Debug::LATEX, "TeXEnvironment...done " << &*par);

	return par;
}

}


int latexOptArgInsets(Buffer const & buf, Paragraph const & par,
		      odocstream & os, OutputParams const & runparams, int number)
{
	int lines = 0;

	InsetList::const_iterator it = par.insetList().begin();
	InsetList::const_iterator end = par.insetList().end();
	for (; it != end && number > 0 ; ++it) {
		if (it->inset->lyxCode() == OPTARG_CODE) {
			InsetOptArg * ins =
				static_cast<InsetOptArg *>(it->inset);
			lines += ins->latexOptional(buf, os, runparams);
			--number;
		}
	}
	return lines;
}


namespace {

ParagraphList::const_iterator
TeXOnePar(Buffer const & buf,
	  ParagraphList const & paragraphs,
	  ParagraphList::const_iterator pit,
	  odocstream & os, TexRow & texrow,
	  OutputParams const & runparams_in,
	  string const & everypar)
{
	LYXERR(Debug::LATEX, "TeXOnePar...     " << &*pit << " '"
		<< everypar << "'");
	BufferParams const & bparams = buf.params();
	LayoutPtr style;

	if (runparams_in.verbatim) {
		int const dist = std::distance(paragraphs.begin(), pit);
		Font const outerfont = outerFont(dist, paragraphs);

		// No newline if only one paragraph in this lyxtext
		if (dist > 0) {
			os << '\n';
			texrow.newline();
		}

		/*bool need_par = */ pit->latex(buf, bparams, outerfont,
			os, texrow, runparams_in);

		return ++pit;
	}

	// In an inset with unlimited length (all in one row),
	// force layout to default
	if (!pit->forceDefaultParagraphs())
		style = pit->layout();
	else
		style = bparams.getTextClass().defaultLayout();

	OutputParams runparams = runparams_in;
	runparams.moving_arg |= style->needprotect;

	// This paragraph's language
	Language const * const par_language = pit->getParLanguage(bparams);
	// The document's language
	Language const * const doc_language = bparams.language;
	// The language that was in effect when the environemnt this paragraph is 
	// inside of was opened
	Language const * const outer_language = 
		(runparams.local_font != 0) ?
			runparams.local_font->language() : doc_language;
	// The previous language that was in effect is either the language of
	// the previous paragraph, if there is one, or else the outer language
	// if there is no previous paragraph
	Language const * const prev_language =
		(pit != paragraphs.begin()) ?
			boost::prior(pit)->getParLanguage(bparams) : outer_language;

	if (par_language->babel() != prev_language->babel()
	    // check if we already put language command in TeXEnvironment()
	    && !(style->isEnvironment()
		 && (pit == paragraphs.begin() ||
		     (boost::prior(pit)->layout() != pit->layout() &&
		      boost::prior(pit)->getDepth() <= pit->getDepth())
		     || boost::prior(pit)->getDepth() < pit->getDepth())))
	{
		if (!lyxrc.language_command_end.empty() &&
		    prev_language->babel() != outer_language->babel() &&
		    !prev_language->babel().empty())
		{
			os << from_ascii(subst(lyxrc.language_command_end,
				"$$lang",
				prev_language->babel()))
			   << '\n';
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
			os << from_ascii(subst(
				lyxrc.language_command_begin,
				"$$lang",
				par_language->babel()))
			   << '\n';
			texrow.newline();
		}
	}

	// Switch file encoding if necessary; no need to do this for "default"
	// encoding, since this only affects the position of the outputted
	// \inputencoding command; the encoding switch will occur when necessary
	if (bparams.inputenc == "auto" &&
	    runparams.encoding->package() == Encoding::inputenc) {
		// Look ahead for future encoding changes.
		// We try to output them at the beginning of the paragraph,
		// since the \inputencoding command is not allowed e.g. in
		// sections.
		for (pos_type i = 0; i < pit->size(); ++i) {
			char_type const c = pit->getChar(i);
			if (c < 0x80)
				continue;
			if (pit->isInset(i))
				break;
			// All characters before c are in the ASCII range, and
			// c is non-ASCII (but no inset), so change the
			// encoding to that required by the language of c.
			Encoding const * const encoding =
				pit->getFontSettings(bparams, i).language()->encoding();
			pair<bool, int> enc_switch = switchEncoding(os, bparams, false,
					*(runparams.encoding), *encoding);
			if (encoding->package() == Encoding::inputenc && enc_switch.first) {
				runparams.encoding = encoding;
				if (enc_switch.second > 0) {
					os << '\n';
					texrow.newline();
				}
			}
			break;
		}
	}

	// In an inset with unlimited length (all in one row),
	// don't allow any special options in the paragraph
	if (!pit->forceDefaultParagraphs()) {
		if (pit->params().startOfAppendix()) {
			os << "\\appendix\n";
			texrow.newline();
		}

		if (!pit->params().spacing().isDefault()
			&& (pit == paragraphs.begin()
			    || !boost::prior(pit)->hasSameLayout(*pit)))
		{
			os << from_ascii(pit->params().spacing().writeEnvirBegin())
			    << '\n';
			texrow.newline();
		}

		if (style->isCommand()) {
			os << '\n';
			texrow.newline();
		}
	}

	switch (style->latextype) {
	case LATEX_COMMAND:
		os << '\\' << from_ascii(style->latexname());

		// Separate handling of optional argument inset.
		if (style->optionalargs > 0) {
			int ret = latexOptArgInsets(buf, *pit, os, runparams,
						    style->optionalargs);
			while (ret > 0) {
				texrow.newline();
				--ret;
			}
		}
		else
			os << from_ascii(style->latexparam());
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

	Font const outerfont =
		outerFont(std::distance(paragraphs.begin(), pit),
			  paragraphs);

	// FIXME UNICODE
	os << from_utf8(everypar);
	bool need_par = pit->latex(buf, bparams, outerfont,
					     os, texrow, runparams);

	// Make sure that \\par is done with the font of the last
	// character if this has another size as the default.
	// This is necessary because LaTeX (and LyX on the screen)
	// calculates the space between the baselines according
	// to this font. (Matthias)
	//
	// Is this really needed ? (Dekel)
	// We do not need to use to change the font for the last paragraph
	// or for a command.

	Font const font =
		(pit->empty()
		 ? pit->getLayoutFont(bparams, outerfont)
		 : pit->getFont(bparams, pit->size() - 1, outerfont));

	bool is_command = style->isCommand();

	if (style->resfont.size() != font.fontInfo().size()
	    && boost::next(pit) != paragraphs.end()
	    && !is_command) {
		if (!need_par)
			os << '{';
		os << "\\" << from_ascii(font.latexSize()) << " \\par}";
	} else if (need_par) {
		os << "\\par}";
	} else if (is_command)
		os << '}';

	bool pending_newline = false;
	switch (style->latextype) {
	case LATEX_ITEM_ENVIRONMENT:
	case LATEX_LIST_ENVIRONMENT:
		if (boost::next(pit) != paragraphs.end()
		    && (pit->params().depth() < boost::next(pit)->params().depth()))
			pending_newline = true;
		break;
	case LATEX_ENVIRONMENT: {
		// if its the last paragraph of the current environment
		// skip it otherwise fall through
		ParagraphList::const_iterator next = boost::next(pit);

		if (next != paragraphs.end()
		    && (next->layout() != pit->layout()
			|| next->params().depth() != pit->params().depth()))
			break;
	}

		// fall through possible
	default:
		// we don't need it for the last paragraph!!!
		if (boost::next(pit) != paragraphs.end())
			pending_newline = true;
	}

	if (!pit->forceDefaultParagraphs()) {
		if (!pit->params().spacing().isDefault()
			&& (boost::next(pit) == paragraphs.end()
			    || !boost::next(pit)->hasSameLayout(*pit)))
		{
			if (pending_newline) {
				os << '\n';
				texrow.newline();
			}
			os << from_ascii(pit->params().spacing().writeEnvirEnd());
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
		((boost::next(pit) != paragraphs.end() &&
		  par_language->babel() != 
		  	(boost::next(pit)->getParLanguage(bparams))->babel()) ||
		 (boost::next(pit) == paragraphs.end() &&
		  par_language->babel() != outer_language->babel()));

	if (closing_rtl_ltr_environment || (boost::next(pit) == paragraphs.end()
	    && par_language->babel() != outer_language->babel())) {
		// Since \selectlanguage write the language to the aux file,
		// we need to reset the language at the end of footnote or
		// float.

		if (pending_newline) {
			os << '\n';
			texrow.newline();
		}
		if (lyxrc.language_command_end.empty()) {
			if (!prev_language->babel().empty()) {
				os << from_ascii(subst(
					lyxrc.language_command_begin,
					"$$lang",
					prev_language->babel()));
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
	if (closing_rtl_ltr_environment)
		os << "}";

	if (pending_newline) {
		os << '\n';
		texrow.newline();
	}

	// If this is the last paragraph, and a local_font was set upon entering
	// the inset, the encoding should be set back to that local_font's 
	// encoding. We don't use switchEncoding(), because no explicit encoding
	// switch command is needed, since latex will automatically revert to it
	// when this inset closes.
	// This switch is only necessary if we're using "auto" or "default" 
	// encoding. 
	if (boost::next(pit) == paragraphs.end() && runparams_in.local_font != 0) {
		runparams_in.encoding = runparams_in.local_font->language()->encoding();
		if (bparams.inputenc == "auto" || bparams.inputenc == "default")
			os << setEncoding(runparams_in.encoding->iconvName());

	}
	// Otherwise, the current encoding should be set for the next paragraph.
	else
		runparams_in.encoding = runparams.encoding;


	// we don't need it for the last paragraph!!!
	// Note from JMarc: we will re-add a \n explicitely in
	// TeXEnvironment, because it is needed in this case
	if (boost::next(pit) != paragraphs.end()) {
		os << '\n';
		texrow.newline();
	}

	if (boost::next(pit) != paragraphs.end())
		LYXERR(Debug::LATEX, "TeXOnePar...done " << &*boost::next(pit));

	return ++pit;
}

} // anon namespace


// LaTeX all paragraphs
void latexParagraphs(Buffer const & buf,
		     ParagraphList const & paragraphs,
		     odocstream & os,
		     TexRow & texrow,
		     OutputParams const & runparams,
		     string const & everypar)
{
	bool was_title = false;
	bool already_title = false;
	TextClass const & tclass = buf.params().getTextClass();
	ParagraphList::const_iterator par = paragraphs.begin();
	ParagraphList::const_iterator endpar = paragraphs.end();

	BOOST_ASSERT(runparams.par_begin <= runparams.par_end);
	// if only part of the paragraphs will be outputed
	if (runparams.par_begin !=  runparams.par_end) {
		par = boost::next(paragraphs.begin(), runparams.par_begin);
		endpar = boost::next(paragraphs.begin(), runparams.par_end);
		// runparams will be passed to nested paragraphs, so
		// we have to reset the range parameters.
		const_cast<OutputParams&>(runparams).par_begin = 0;
		const_cast<OutputParams&>(runparams).par_end = 0;
	}

	// if only_body
	while (par != endpar) {
		ParagraphList::const_iterator lastpar = par;
		// well we have to check if we are in an inset with unlimited
		// length (all in one row) if that is true then we don't allow
		// any special options in the paragraph and also we don't allow
		// any environment other than the default layout of the
		// text class to be valid!
		if (!par->forceDefaultParagraphs()) {
			LayoutPtr const & layout = par->layout();

			if (layout->intitle) {
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

			if (layout->is_environment) {
				par = TeXOnePar(buf, paragraphs, par, os, texrow,
						runparams, everypar);
			} else if (layout->isEnvironment() ||
				   !par->params().leftIndent().zero()) {
				par = TeXEnvironment(buf, paragraphs, par, os,
						     texrow, runparams);
			} else {
				par = TeXOnePar(buf, paragraphs, par, os, texrow,
						runparams, everypar);
			}
		} else {
			par = TeXOnePar(buf, paragraphs, par, os, texrow,
					runparams, everypar);
		}
		if (std::distance(lastpar, par) >= std::distance(lastpar, endpar))
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
}


pair<bool, int> switchEncoding(odocstream & os, BufferParams const & bparams,
		   bool moving_arg, Encoding const & oldEnc,
		   Encoding const & newEnc)
{
	if ((bparams.inputenc != "auto" && bparams.inputenc != "default")
		|| moving_arg)
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

	docstring const inputenc = from_ascii(newEnc.latexName());
	switch (newEnc.package()) {
		case Encoding::none:
			// shouldn't ever reach here, see above
			return make_pair(true, 0);
		case Encoding::inputenc: {
			int count = inputenc.length();
			if (oldEnc.package() == Encoding::CJK) {
				os << "\\end{CJK}";
				count += 9;
			}
			os << "\\inputencoding{" << inputenc << '}';
			return make_pair(true, count + 16);
		}
		case Encoding::CJK: {
			int count = inputenc.length();
			if (oldEnc.package() == Encoding::CJK) {
				os << "\\end{CJK}";
				count += 9;
			}
			os << "\\begin{CJK}{" << inputenc << "}{}";
			return make_pair(true, count + 15);
		}
	}
	// Dead code to avoid a warning:
	return make_pair(true, 0);
}

} // namespace lyx
