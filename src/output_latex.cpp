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
#include "Font.h"
#include "InsetList.h"
#include "Language.h"
#include "Layout.h"
#include "LyXRC.h"
#include "OutputParams.h"
#include "Paragraph.h"
#include "ParagraphParameters.h"
#include "TextClass.h"
#include "TexRow.h"

#include "insets/InsetBibitem.h"
#include "insets/InsetArgument.h"

#include "support/lassert.h"
#include "support/convert.h"
#include "support/debug.h"
#include "support/lstrings.h"
#include "support/textutils.h"

#include <QThreadStorage>

#include <algorithm>
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


struct OutputState
{
	OutputState() : open_encoding_(none), cjk_inherited_(0),
		        prev_env_language_(0)
	{
	}
	int open_encoding_;
	int cjk_inherited_;
	Language const * prev_env_language_;
};


OutputState * getOutputState()
{
	// FIXME An instance of OutputState should be kept around for each export
	//       instead of using local thread storage
	static QThreadStorage<OutputState *> outputstate;
	if (!outputstate.hasLocalData())
		outputstate.setLocalData(new OutputState);
	return outputstate.localData();
}


string const getPolyglossiaEnvName(Language const * lang)
{
	string result = lang->polyglossia();
	if (result == "arabic")
		// exceptional spelling; see polyglossia docs.
		result = "Arabic";
	return result;
}


struct TeXEnvironmentData
{
	bool cjk_nested;
	Layout const * style;
	Language const * par_language;
	Encoding const * prev_encoding;
	bool leftindent_open;
};


static TeXEnvironmentData prepareEnvironment(Buffer const & buf,
					Text const & text,
					ParagraphList::const_iterator pit,
					otexstream & os,
					OutputParams const & runparams)
{
	TeXEnvironmentData data;

	BufferParams const & bparams = buf.params();

	// FIXME This test should not be necessary.
	// We should perhaps issue an error if it is.
	Layout const & style = text.inset().forcePlainLayout() ?
		bparams.documentClass().plainLayout() : pit->layout();

	ParagraphList const & paragraphs = text.paragraphs();
	ParagraphList::const_iterator const priorpit =
		pit == paragraphs.begin() ? pit : boost::prior(pit);

	OutputState * state = getOutputState();
	bool const use_prev_env_language = state->prev_env_language_ != 0
			&& priorpit->layout().isEnvironment()
			&& (priorpit->getDepth() > pit->getDepth()
			    || (priorpit->getDepth() == pit->getDepth()
				&& priorpit->layout() != pit->layout()));

	data.prev_encoding = runparams.encoding;
	data.par_language = pit->getParLanguage(bparams);
	Language const * const doc_language = bparams.language;
	Language const * const prev_par_language =
		(pit != paragraphs.begin())
		? (use_prev_env_language ? state->prev_env_language_
					 : priorpit->getParLanguage(bparams))
		: doc_language;

	bool const use_polyglossia = runparams.use_polyglossia;
	string const par_lang = use_polyglossia ?
		getPolyglossiaEnvName(data.par_language) : data.par_language->babel();
	string const prev_par_lang = use_polyglossia ?
		getPolyglossiaEnvName(prev_par_language) : prev_par_language->babel();
	string const doc_lang = use_polyglossia ?
		getPolyglossiaEnvName(doc_language) : doc_language->babel();
	string const lang_begin_command = use_polyglossia ?
		"\\begin{$$lang}" : lyxrc.language_command_begin;
	string const lang_end_command = use_polyglossia ?
		"\\end{$$lang}" : lyxrc.language_command_end;

	if (par_lang != prev_par_lang) {
		if (!lang_end_command.empty() &&
		    prev_par_lang != doc_lang &&
		    !prev_par_lang.empty()) {
			os << from_ascii(subst(
				lang_end_command,
				"$$lang",
				prev_par_lang))
			  // the '%' is necessary to prevent unwanted whitespace
			  << "%\n";
		}

		if ((lang_end_command.empty() ||
		    par_lang != doc_lang) &&
		    !par_lang.empty()) {
			os << from_ascii(subst(
				lang_begin_command,
				"$$lang",
				par_lang));
			if (use_polyglossia
			    && !data.par_language->polyglossiaOpts().empty())
					os << "["
					   << from_ascii(data.par_language->polyglossiaOpts())
					   << "]";
			  // the '%' is necessary to prevent unwanted whitespace
			os << "%\n";
		}
	}

	data.leftindent_open = false;
	if (!pit->params().leftIndent().zero()) {
		os << "\\begin{LyXParagraphLeftIndent}{"
		   << from_ascii(pit->params().leftIndent().asLatexString())
		   << "}\n";
		data.leftindent_open = true;
	}

	if (style.isEnvironment()) {
		os << "\\begin{" << from_ascii(style.latexname()) << '}';
		if (!style.latexargs().empty()) {
			OutputParams rp = runparams;
			rp.local_font = &pit->getFirstFontSettings(bparams);
			latexArgInsets(paragraphs, pit, os, rp, style.latexargs());
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
	}
	data.style = &style;

	// in multilingual environments, the CJK tags have to be nested properly
	data.cjk_nested = false;
	if (data.par_language->encoding()->package() == Encoding::CJK &&
	    state->open_encoding_ != CJK && pit->isMultiLingual(bparams)) {
		if (prev_par_language->encoding()->package() == Encoding::CJK)
			os << "\\begin{CJK}{" << from_ascii(data.par_language->encoding()->latexName())
			   << "}{" << from_ascii(bparams.fonts_cjk) << "}%\n";
		state->open_encoding_ = CJK;
		data.cjk_nested = true;
	}
	return data;
}


static void finishEnvironment(otexstream & os, OutputParams const & runparams,
			      TeXEnvironmentData const & data)
{
	OutputState * state = getOutputState();
	if (state->open_encoding_ == CJK && data.cjk_nested) {
		// We need to close the encoding even if it does not change
		// to do correct environment nesting
		os << "\\end{CJK}\n";
		state->open_encoding_ = none;
	}

	if (data.style->isEnvironment()) {
		os << breakln
		   << "\\end{" << from_ascii(data.style->latexname()) << "}\n";
		state->prev_env_language_ = data.par_language;
		if (runparams.encoding != data.prev_encoding) {
			runparams.encoding = data.prev_encoding;
			if (!runparams.isFullUnicode())
				os << setEncoding(data.prev_encoding->iconvName());
		}
	}

	if (data.leftindent_open) {
		os << breakln << "\\end{LyXParagraphLeftIndent}\n";
		state->prev_env_language_ = data.par_language;
		if (runparams.encoding != data.prev_encoding) {
			runparams.encoding = data.prev_encoding;
			if (!runparams.isFullUnicode())
				os << setEncoding(data.prev_encoding->iconvName());
		}
	}

	// Check whether we should output a blank line after the environment
	if (!data.style->nextnoindent)
		os << '\n';
}


void TeXEnvironment(Buffer const & buf, Text const & text,
		    OutputParams const & runparams,
		    pit_type & pit, otexstream & os)
{
	ParagraphList const & paragraphs = text.paragraphs();
	ParagraphList::const_iterator par = paragraphs.constIterator(pit);
	LYXERR(Debug::LATEX, "TeXEnvironment for paragraph " << pit);

	Layout const & current_layout = par->layout();
	depth_type const current_depth = par->params().depth();
	Length const & current_left_indent = par->params().leftIndent();

	// This is for debugging purpose at the end.
	pit_type const par_begin = pit;
	for (; pit < runparams.par_end; ++pit) {
		ParagraphList::const_iterator par = paragraphs.constIterator(pit);

		// check first if this is an higher depth paragraph.
		bool go_out = (par->params().depth() < current_depth);
		if (par->params().depth() == current_depth) {
			// This environment is finished.
			go_out |= (par->layout() != current_layout);
			go_out |= (par->params().leftIndent() != current_left_indent);
		}
		if (go_out) {
			// nothing to do here, restore pit and go out.
			pit--;
			break;
		}

		if (par->layout() == current_layout
			&& par->params().depth() == current_depth
			&& par->params().leftIndent() == current_left_indent) {
			// We are still in the same environment so TeXOnePar and continue;
			TeXOnePar(buf, text, pit, os, runparams);
			continue;
		}

		// We are now in a deeper environment.
		// Either par->layout() != current_layout
		// Or     par->params().depth() > current_depth
		// Or     par->params().leftIndent() != current_left_indent)

		// FIXME This test should not be necessary.
		// We should perhaps issue an error if it is.
		bool const force_plain_layout = text.inset().forcePlainLayout();
		Layout const & style = force_plain_layout
			? buf.params().documentClass().plainLayout()
			: par->layout();

		if (!style.isEnvironment()) {
			// This is a standard paragraph, no need to call TeXEnvironment.
			TeXOnePar(buf, text, pit, os, runparams);
			continue;
		}

		// This is a new environment.
		TeXEnvironmentData const data =
			prepareEnvironment(buf, text, par, os, runparams);
		// Recursive call to TeXEnvironment!
		TeXEnvironment(buf, text, runparams, pit, os);
		finishEnvironment(os, runparams, data);
	}

	if (pit != runparams.par_end)
		LYXERR(Debug::LATEX, "TeXEnvironment for paragraph " << par_begin << " done.");
}


void getArgInsets(otexstream & os, OutputParams const & runparams, Layout::LaTeXArgMap const & latexargs,
		  map<int, lyx::InsetArgument const *> ilist, vector<string> required, string const & prefix)
{
	unsigned int const argnr = latexargs.size();
	if (argnr == 0)
		return;

	// Default and preset args are always output, so if they require
	// other arguments, consider this.
	Layout::LaTeXArgMap::const_iterator lit = latexargs.begin();
	Layout::LaTeXArgMap::const_iterator const lend = latexargs.end();
	for (; lit != lend; ++lit) {
		Layout::latexarg arg = (*lit).second;
		if ((!arg.presetarg.empty() || !arg.defaultarg.empty()) && !arg.requires.empty()) {
				vector<string> req = getVectorFromString(arg.requires);
				required.insert(required.end(), req.begin(), req.end());
			}
	}

	for (unsigned int i = 1; i <= argnr; ++i) {
		map<int, InsetArgument const *>::const_iterator lit = ilist.find(i);
		bool inserted = false;
		if (lit != ilist.end()) {
			InsetArgument const * ins = (*lit).second;
			if (ins) {
				Layout::LaTeXArgMap::const_iterator const lait =
						latexargs.find(ins->name());
				if (lait != latexargs.end()) {
					Layout::latexarg arg = (*lait).second;
					docstring ldelim = arg.mandatory ?
							from_ascii("{") : from_ascii("[");
					docstring rdelim = arg.mandatory ?
							from_ascii("}") : from_ascii("]");
					if (!arg.ldelim.empty())
						ldelim = arg.ldelim;
					if (!arg.rdelim.empty())
						rdelim = arg.rdelim;
					ins->latexArgument(os, runparams, ldelim, rdelim, arg.presetarg);
					inserted = true;
				}
			}
		}
		if (!inserted) {
			Layout::LaTeXArgMap::const_iterator lait = latexargs.begin();
			Layout::LaTeXArgMap::const_iterator const laend = latexargs.end();
			for (; lait != laend; ++lait) {
				string const name = prefix + convert<string>(i);
				if ((*lait).first == name) {
					Layout::latexarg arg = (*lait).second;
					docstring preset = arg.presetarg;
					if (!arg.defaultarg.empty()) {
						if (!preset.empty())
							preset += ",";
						preset += arg.defaultarg;
					}
					if (arg.mandatory) {
						docstring ldelim = arg.ldelim.empty() ?
								from_ascii("{") : arg.ldelim;
						docstring rdelim = arg.rdelim.empty() ?
								from_ascii("}") : arg.rdelim;
						os << ldelim << preset << rdelim;
					} else if (!preset.empty()) {
						docstring ldelim = arg.ldelim.empty() ?
								from_ascii("[") : arg.ldelim;
						docstring rdelim = arg.rdelim.empty() ?
								from_ascii("]") : arg.rdelim;
						os << ldelim << preset << rdelim;
					} else if (find(required.begin(), required.end(),
						   (*lait).first) != required.end()) {
						docstring ldelim = arg.ldelim.empty() ?
								from_ascii("[") : arg.ldelim;
						docstring rdelim = arg.rdelim.empty() ?
								from_ascii("]") : arg.rdelim;
						os << ldelim << rdelim;
					} else
						break;
				}
			}
		}
	}
}


} // namespace anon


void latexArgInsets(Paragraph const & par, otexstream & os,
	OutputParams const & runparams, Layout::LaTeXArgMap const & latexargs, string const & prefix)
{
	map<int, InsetArgument const *> ilist;
	vector<string> required;

	InsetList::const_iterator it = par.insetList().begin();
	InsetList::const_iterator end = par.insetList().end();
	for (; it != end; ++it) {
		if (it->inset->lyxCode() == ARG_CODE) {
			InsetArgument const * ins =
				static_cast<InsetArgument const *>(it->inset);
			if (ins->name().empty())
				LYXERR0("Error: Unnamed argument inset!");
			else {
				string const name = prefix.empty() ? ins->name() : split(ins->name(), ':');
				unsigned int const nr = convert<unsigned int>(name);
				ilist[nr] = ins;
				Layout::LaTeXArgMap::const_iterator const lit =
						latexargs.find(ins->name());
				if (lit != latexargs.end()) {
					Layout::latexarg const & arg = (*lit).second;
					if (!arg.requires.empty()) {
						vector<string> req = getVectorFromString(arg.requires);
						required.insert(required.end(), req.begin(), req.end());
					}
				}
			}
		}
	}
	getArgInsets(os, runparams, latexargs, ilist, required, prefix);
}


void latexArgInsets(ParagraphList const & pars, ParagraphList::const_iterator pit,
	otexstream & os, OutputParams const & runparams, Layout::LaTeXArgMap const & latexargs,
	string const & prefix)
{
	map<int, InsetArgument const *> ilist;
	vector<string> required;

	depth_type const current_depth = pit->params().depth();
	Layout const current_layout = pit->layout();

	// get the first paragraph in sequence with this layout and depth
	pit_type offset = 0;
	while (true) {
		if (boost::prior(pit, offset) == pars.begin())
			break;
		ParagraphList::const_iterator priorpit = boost::prior(pit, offset + 1);
		if (priorpit->layout() == current_layout
		    && priorpit->params().depth() == current_depth)
			++offset;
		else
			break;
	}

	ParagraphList::const_iterator spit = boost::prior(pit, offset);

	for (; spit != pars.end(); ++spit) {
		if (spit->layout() != current_layout || spit->params().depth() < current_depth)
			break;
		if (spit->params().depth() > current_depth)
			continue;
		InsetList::const_iterator it = spit->insetList().begin();
		InsetList::const_iterator end = spit->insetList().end();
		for (; it != end; ++it) {
			if (it->inset->lyxCode() == ARG_CODE) {
				InsetArgument const * ins =
					static_cast<InsetArgument const *>(it->inset);
				if (ins->name().empty())
					LYXERR0("Error: Unnamed argument inset!");
				else {
					string const name = prefix.empty() ? ins->name() : split(ins->name(), ':');
					unsigned int const nr = convert<unsigned int>(name);
					if (ilist.find(nr) == ilist.end())
						ilist[nr] = ins;
					Layout::LaTeXArgMap::const_iterator const lit =
							latexargs.find(ins->name());
					if (lit != latexargs.end()) {
						Layout::latexarg const & arg = (*lit).second;
						if (!arg.requires.empty()) {
							vector<string> req = getVectorFromString(arg.requires);
							required.insert(required.end(), req.begin(), req.end());
						}
					}
				}
			}
		}
	}
	getArgInsets(os, runparams, latexargs, ilist, required, prefix);
}

namespace {

// output the proper paragraph start according to latextype.
void parStartCommand(Paragraph const & par, otexstream & os,
		     OutputParams const & runparams, Layout const & style) 
{
	switch (style.latextype) {
	case LATEX_COMMAND:
		os << '\\' << from_ascii(style.latexname());

		// Command arguments
		if (!style.latexargs().empty())
			latexArgInsets(par, os, runparams, style.latexargs());
		os << from_ascii(style.latexparam());
		break;
	case LATEX_ITEM_ENVIRONMENT:
	case LATEX_LIST_ENVIRONMENT:
		os << "\\" + style.itemcommand();
		// Item arguments
		if (!style.itemargs().empty())
			latexArgInsets(par, os, runparams, style.itemargs(), "item:");
		os << " ";
		break;
	case LATEX_BIB_ENVIRONMENT:
		// ignore this, the inset will write itself
		break;
	default:
		break;
	}
}

} // namespace anon

// FIXME: this should be anonymous
void TeXOnePar(Buffer const & buf,
	       Text const & text,
	       pit_type pit,
	       otexstream & os,
	       OutputParams const & runparams_in,
	       string const & everypar,
	       int start_pos, int end_pos)
{
	BufferParams const & bparams = runparams_in.is_child
		? buf.masterParams() : buf.params();
	ParagraphList const & paragraphs = text.paragraphs();
	Paragraph const & par = paragraphs.at(pit);
	// FIXME This check should not really be needed.
	// Perhaps we should issue an error if it is.
	Layout const & style = text.inset().forcePlainLayout() ?
		bparams.documentClass().plainLayout() : par.layout();

	if (style.inpreamble)
		return;

	LYXERR(Debug::LATEX, "TeXOnePar for paragraph " << pit << " ptr " << &par << " '"
		<< everypar << "'");

	OutputParams runparams = runparams_in;
	runparams.isLastPar = (pit == pit_type(paragraphs.size() - 1));
	// We reinitialze par begin and end to be on the safe side
	// with embedded inset as we don't know if they set those
	// value correctly.
	runparams.par_begin = 0;
	runparams.par_end = 0;

	bool const maintext = text.isMainText();
	// we are at the beginning of an inset and CJK is already open;
	// we count inheritation levels to get the inset nesting right.
	OutputState * state = getOutputState();
	if (pit == 0 && !maintext
	    && (state->cjk_inherited_ > 0 || state->open_encoding_ == CJK)) {
		state->cjk_inherited_ += 1;
		state->open_encoding_ = none;
	}

	if (text.inset().isPassThru()) {
		Font const outerfont = text.outerFont(pit);

		// No newline before first paragraph in this lyxtext
		if (pit > 0) {
			os << '\n';
			if (!text.inset().getLayout().parbreakIsNewline())
				os << '\n';
		}

		par.latex(bparams, outerfont, os, runparams, start_pos, end_pos);
		return;
	}

	Paragraph const * nextpar = runparams.isLastPar
		? 0 : &paragraphs.at(pit + 1);

	if (style.pass_thru) {
		Font const outerfont = text.outerFont(pit);
		parStartCommand(par, os, runparams, style);

		par.latex(bparams, outerfont, os, runparams, start_pos, end_pos);

		// I did not create a parEndCommand for this minuscule
		// task because in the other user of parStartCommand
		// the code is different (JMarc)
		if (style.isCommand())
			os << "}\n";
		else
			os << '\n';
		if (!style.parbreak_is_newline) {
			os << '\n';
		} else if (nextpar && !style.isEnvironment()) {
			Layout const nextstyle = text.inset().forcePlainLayout()
				? bparams.documentClass().plainLayout()
				: nextpar->layout();
			if (nextstyle.name() != style.name())
				os << '\n';
		}

		return;
	}

	// This paragraph's language
	Language const * const par_language = par.getParLanguage(bparams);
	// The document's language
	Language const * const doc_language = bparams.language;
	// The language that was in effect when the environment this paragraph is
	// inside of was opened
	Language const * const outer_language =
		(runparams.local_font != 0) ?
			runparams.local_font->language() : doc_language;

	Paragraph const * priorpar = (pit == 0) ? 0 : &paragraphs.at(pit - 1);

	// The previous language that was in effect is the language of the
	// previous paragraph, unless the previous paragraph is inside an
	// environment with nesting depth greater than (or equal to, but with
	// a different layout) the current one. If there is no previous
	// paragraph, the previous language is the outer language.
	bool const use_prev_env_language = state->prev_env_language_ != 0
			&& priorpar
			&& priorpar->layout().isEnvironment()
			&& (priorpar->getDepth() > par.getDepth()
			    || (priorpar->getDepth() == par.getDepth()
				    && priorpar->layout() != par.layout()));
	Language const * const prev_language =
		(pit != 0)
		? (use_prev_env_language ? state->prev_env_language_
					 : priorpar->getParLanguage(bparams))
		: outer_language;


	bool const use_polyglossia = runparams.use_polyglossia;
	string const par_lang = use_polyglossia ?
		getPolyglossiaEnvName(par_language): par_language->babel();
	string const prev_lang = use_polyglossia ?
		getPolyglossiaEnvName(prev_language) : prev_language->babel();
	string const doc_lang = use_polyglossia ?
		getPolyglossiaEnvName(doc_language) : doc_language->babel();
	string const outer_lang = use_polyglossia ?
		getPolyglossiaEnvName(outer_language) : outer_language->babel();
	string lang_begin_command = use_polyglossia ?
		"\\begin{$$lang}" : lyxrc.language_command_begin;
	string lang_end_command = use_polyglossia ?
		"\\end{$$lang}" : lyxrc.language_command_end;
	// the '%' is necessary to prevent unwanted whitespace
	string lang_command_termination = "%\n";

	// In some insets (such as Arguments), we cannot use \selectlanguage
	bool const localswitch = !use_polyglossia
		&& text.inset().forceLocalFontSwitch();
	if (localswitch) {
		lang_begin_command = lyxrc.language_command_local;
		lang_end_command = "}";
		lang_command_termination.clear();
	}

	if (par_lang != prev_lang
		// check if we already put language command in TeXEnvironment()
		&& !(style.isEnvironment()
		     && (pit == 0 || (priorpar->layout() != par.layout()
			                  && priorpar->getDepth() <= par.getDepth())
		                  || priorpar->getDepth() < par.getDepth())))
	{
		if (!lang_end_command.empty() &&
		    prev_lang != outer_lang &&
		    !prev_lang.empty())
		{
			os << from_ascii(subst(lang_end_command,
				"$$lang",
				prev_lang))
			   << lang_command_termination;
		}

		// We need to open a new language if we couldn't close the previous
		// one (because there's no language_command_end); and even if we closed
		// the previous one, if the current language is different than the
		// outer_language (which is currently in effect once the previous one
		// is closed).
		if ((lang_end_command.empty() || par_lang != outer_lang)
			&& !par_lang.empty()) {
			// If we're inside an inset, and that inset is within an \L or \R
			// (or equivalents), then within the inset, too, any opposite
			// language paragraph should appear within an \L or \R (in addition
			// to, outside of, the normal language switch commands).
			// This behavior is not correct for ArabTeX, though.
			if (!use_polyglossia
			    // not for ArabTeX
				&& par_language->lang() != "arabic_arabtex"
				&& outer_language->lang() != "arabic_arabtex"
			    // are we in an inset?
			    && runparams.local_font != 0
			    // is the inset within an \L or \R?
			    //
			    // FIXME: currently, we don't check this; this means that
			    // we'll have unnnecessary \L and \R commands, but that
			    // doesn't seem to hurt (though latex will complain)
			    //
			    // is this paragraph in the opposite direction?
			    && runparams.local_font->isRightToLeft() != par_language->rightToLeft()) {
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
			if (runparams.encoding->package() != Encoding::CJK
			    && !par_lang.empty()) {
				os << from_ascii(subst(
					lang_begin_command,
					"$$lang",
					par_lang));
				if (use_polyglossia
				    && !par_language->polyglossiaOpts().empty())
						os << "["
						  << from_ascii(par_language->polyglossiaOpts())
						  << "]";
				os << lang_command_termination;
			}
		}
	}

	// Switch file encoding if necessary; no need to do this for "default"
	// encoding, since this only affects the position of the outputted
	// \inputencoding command; the encoding switch will occur when necessary
	if (bparams.inputenc == "auto"
		&& runparams.encoding->package() != Encoding::none) {
		// Look ahead for future encoding changes.
		// We try to output them at the beginning of the paragraph,
		// since the \inputencoding command is not allowed e.g. in
		// sections. For this reason we only set runparams.moving_arg
		// after checking for the encoding change, otherwise the
		// change would be always avoided by switchEncoding().
		for (pos_type i = 0; i < par.size(); ++i) {
			char_type const c = par.getChar(i);
			Encoding const * const encoding =
				par.getFontSettings(bparams, i).language()->encoding();
			if (encoding->package() != Encoding::CJK
				&& runparams.encoding->package() == Encoding::inputenc
				&& isASCII(c))
				continue;
			if (par.isInset(i))
				break;
			// All characters before c are in the ASCII range, and
			// c is non-ASCII (but no inset), so change the
			// encoding to that required by the language of c.
			// With CJK, only add switch if we have CJK content at the beginning
			// of the paragraph
			if (i != 0 && encoding->package() == Encoding::CJK)
				continue;

			pair<bool, int> enc_switch = switchEncoding(os.os(),
						bparams, runparams, *encoding);
			// the following is necessary after a CJK environment in a multilingual
			// context (nesting issue).
			if (par_language->encoding()->package() == Encoding::CJK
				&& state->open_encoding_ != CJK && state->cjk_inherited_ == 0) {
				os << "\\begin{CJK}{" << from_ascii(par_language->encoding()->latexName())
				   << "}{" << from_ascii(bparams.fonts_cjk) << "}%\n";
				state->open_encoding_ = CJK;
			}
			if (encoding->package() != Encoding::none && enc_switch.first) {
				if (enc_switch.second > 0) {
					// the '%' is necessary to prevent unwanted whitespace
					os << "%\n";
				}
				// With CJK, the CJK tag had to be closed first (see above)
				if (runparams.encoding->package() == Encoding::CJK
				    && !par_lang.empty()) {
					os << from_ascii(subst(
						lang_begin_command,
						"$$lang",
						par_lang))
					<< lang_command_termination;
				}
				runparams.encoding = encoding;
			}
			break;
		}
	}

	runparams.moving_arg |= style.needprotect;
	Encoding const * const prev_encoding = runparams.encoding;

	bool const useSetSpace = bparams.documentClass().provides("SetSpace");
	if (par.allowParagraphCustomization()) {
		if (par.params().startOfAppendix()) {
			os << "\n\\appendix\n";
		}

		if (!par.params().spacing().isDefault()
			&& (pit == 0 || !priorpar->hasSameLayout(par)))
		{
			os << from_ascii(par.params().spacing().writeEnvirBegin(useSetSpace))
			    << '\n';
		}

		if (style.isCommand()) {
			os << '\n';
		}
	}

	parStartCommand(par, os, runparams, style);
	Font const outerfont = text.outerFont(pit);

	// FIXME UNICODE
	os << from_utf8(everypar);
	par.latex(bparams, outerfont, os, runparams, start_pos, end_pos);

	// Make sure that \\par is done with the font of the last
	// character if this has another size as the default.
	// This is necessary because LaTeX (and LyX on the screen)
	// calculates the space between the baselines according
	// to this font. (Matthias)
	//
	// We must not change the font for the last paragraph
	// of non-multipar insets, tabular cells or commands,
	// since this produces unwanted whitespace.

	Font const font = par.empty()
		 ? par.getLayoutFont(bparams, outerfont)
		 : par.getFont(bparams, par.size() - 1, outerfont);

	bool const is_command = style.isCommand();

	if (style.resfont.size() != font.fontInfo().size()
	    && (nextpar || maintext
	        || (text.inset().getLayout().isMultiPar()
	            && text.inset().lyxCode() != CELL_CODE))
	    && !is_command) {
		os << '{';
		os << "\\" << from_ascii(font.latexSize()) << " \\par}";
	} else if (is_command) {
		os << '}';
		if (!style.postcommandargs().empty())
			latexArgInsets(par, os, runparams, style.postcommandargs(), "post:");
		if (runparams.encoding != prev_encoding) {
			runparams.encoding = prev_encoding;
			if (!runparams.isFullUnicode())
				os << setEncoding(prev_encoding->iconvName());
		}
	}

	bool pending_newline = false;
	bool unskip_newline = false;
	switch (style.latextype) {
	case LATEX_ITEM_ENVIRONMENT:
	case LATEX_LIST_ENVIRONMENT:
		if (nextpar && par.params().depth() < nextpar->params().depth())
			pending_newline = true;
		break;
	case LATEX_ENVIRONMENT: {
		// if its the last paragraph of the current environment
		// skip it otherwise fall through
		if (nextpar
			&& (nextpar->layout() != par.layout()
		        || nextpar->params().depth() != par.params().depth()))
			break;
	}

	// fall through possible
	default:
		// we don't need it for the last paragraph!!!
		if (nextpar)
			pending_newline = true;
	}

	if (par.allowParagraphCustomization()) {
		if (!par.params().spacing().isDefault()
			&& (runparams.isLastPar || !nextpar->hasSameLayout(par))) {
			if (pending_newline)
				os << '\n';

			string const endtag =
				par.params().spacing().writeEnvirEnd(useSetSpace);
			if (prefixIs(endtag, "\\end{"))
				os << breakln;

			os << from_ascii(endtag);
			pending_newline = true;
		}
	}

	// Closing the language is needed for the last paragraph; it is also
	// needed if we're within an \L or \R that we may have opened above (not
	// necessarily in this paragraph) and are about to close.
	bool closing_rtl_ltr_environment = !use_polyglossia
		// not for ArabTeX
		&& (par_language->lang() != "arabic_arabtex"
		    && outer_language->lang() != "arabic_arabtex")
		// have we opened an \L or \R environment?
		&& runparams.local_font != 0
		&& runparams.local_font->isRightToLeft() != par_language->rightToLeft()
		// are we about to close the language?
		&&((nextpar && par_language->babel() != (nextpar->getParLanguage(bparams))->babel())
		   || (runparams.isLastPar && par_language->babel() != outer_language->babel()));

	if (closing_rtl_ltr_environment
	    || (runparams.isLastPar
	        && ((!use_polyglossia && par_language->babel() != outer_language->babel())
		    || (use_polyglossia && par_language->polyglossia() != outer_language->polyglossia())))) {
		// Since \selectlanguage write the language to the aux file,
		// we need to reset the language at the end of footnote or
		// float.

		if (pending_newline)
			os << '\n';

		// when the paragraph uses CJK, the language has to be closed earlier
		if (font.language()->encoding()->package() != Encoding::CJK) {
			if (lang_end_command.empty()) {
				// If this is a child, we should restore the
				// master language after the last paragraph.
				Language const * const current_language =
					(runparams.isLastPar && runparams.master_language)
						? runparams.master_language
						: outer_language;
				string const current_lang = use_polyglossia
					? getPolyglossiaEnvName(current_language)
					: current_language->babel();
				if (!current_lang.empty()) {
					os << from_ascii(subst(
						lang_begin_command,
						"$$lang",
						current_lang));
					pending_newline = !localswitch;
					unskip_newline = !localswitch;
				}
			} else if (!par_lang.empty()) {
				os << from_ascii(subst(
					lang_end_command,
					"$$lang",
					par_lang));
				pending_newline = !localswitch;
				unskip_newline = !localswitch;
			}
		}
	}
	if (closing_rtl_ltr_environment)
		os << "}";

	bool const last_was_separator =
		par.size() > 0 && par.isEnvSeparator(par.size() - 1);

	if (pending_newline) {
		if (unskip_newline)
			// prevent unwanted whitespace
			os << '%';
		if (!os.afterParbreak() && !last_was_separator)
			os << '\n';
	}

	// if this is a CJK-paragraph and the next isn't, close CJK
	// also if the next paragraph is a multilingual environment (because of nesting)
	if (nextpar
		&& state->open_encoding_ == CJK
		&& (nextpar->getParLanguage(bparams)->encoding()->package() != Encoding::CJK
		   || (nextpar->layout().isEnvironment() && nextpar->isMultiLingual(bparams)))
		// inbetween environments, CJK has to be closed later (nesting!)
		&& (!style.isEnvironment() || !nextpar->layout().isEnvironment())) {
		os << "\\end{CJK}\n";
		state->open_encoding_ = none;
	}

	// If this is the last paragraph, close the CJK environment
	// if necessary. If it's an environment, we'll have to \end that first.
	if (runparams.isLastPar && !style.isEnvironment()) {
		switch (state->open_encoding_) {
			case CJK: {
				// do nothing at the end of child documents
				if (maintext && buf.masterBuffer() != &buf)
					break;
				// end of main text
				if (maintext) {
					os << "\n\\end{CJK}\n";
				// end of an inset
				} else
					os << "\\end{CJK}";
				state->open_encoding_ = none;
				break;
			}
			case inputenc: {
				os << "\\egroup";
				state->open_encoding_ = none;
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
	// However, do not change the encoding when a fully unicode aware backend
	// such as XeTeX is used.
	if (runparams.isLastPar && runparams_in.local_font != 0
	    && runparams_in.encoding != runparams_in.local_font->language()->encoding()
	    && (bparams.inputenc == "auto" || bparams.inputenc == "default")
	    && (!runparams.isFullUnicode())) {
		runparams_in.encoding = runparams_in.local_font->language()->encoding();
		os << setEncoding(runparams_in.encoding->iconvName());
	}
	// Otherwise, the current encoding should be set for the next paragraph.
	else
		runparams_in.encoding = runparams.encoding;


	// we don't need a newline for the last paragraph!!!
	// Note from JMarc: we will re-add a \n explicitly in
	// TeXEnvironment, because it is needed in this case
	if (nextpar && !os.afterParbreak() && !last_was_separator) {
		// Make sure to start a new line
		os << breakln;
		Layout const & next_layout = nextpar->layout();
		// A newline '\n' is always output before a command,
		// so avoid doubling it.
		if (!next_layout.isCommand()) {
			// Here we now try to avoid spurious empty lines by
			// outputting a paragraph break only if: (case 1) the
			// paragraph style allows parbreaks and no \begin, \end
			// or \item tags are going to follow (i.e., if the next
			// isn't the first or the current isn't the last
			// paragraph of an environment or itemize) and the
			// depth and alignment of the following paragraph is
			// unchanged, or (case 2) the following is a
			// non-environment paragraph whose depth is increased
			// but whose alignment is unchanged, or (case 3) the
			// paragraph is not an environment and the next one is a
			// non-itemize-like env at lower depth, or (case 4) the
			// paragraph is a command not followed by an environment
			// and the alignment of the current and next paragraph
			// is unchanged, or (case 5) the current alignment is
			// changed and a standard paragraph follows.
			DocumentClass const & tclass = bparams.documentClass();
			if ((style == next_layout
			     && !style.parbreak_is_newline
			     && style.latextype != LATEX_ITEM_ENVIRONMENT
			     && style.latextype != LATEX_LIST_ENVIRONMENT
			     && style.align == par.getAlign()
			     && nextpar->getDepth() == par.getDepth()
			     && nextpar->getAlign() == par.getAlign())
			    || (!next_layout.isEnvironment()
				&& nextpar->getDepth() > par.getDepth()
				&& nextpar->getAlign() == par.getAlign())
			    || (!style.isEnvironment()
				&& next_layout.latextype == LATEX_ENVIRONMENT
				&& nextpar->getDepth() < par.getDepth())
			    || (style.isCommand()
				&& !next_layout.isEnvironment()
				&& style.align == par.getAlign()
				&& next_layout.align == nextpar->getAlign())
			    || (style.align != par.getAlign()
				&& tclass.isDefaultLayout(next_layout))) {
				os << '\n';
			}
		}
	}

	LYXERR(Debug::LATEX, "TeXOnePar for paragraph " << pit << " done; ptr "
		<< &par << " next " << nextpar);

	return;
}


// LaTeX all paragraphs
void latexParagraphs(Buffer const & buf,
		     Text const & text,
		     otexstream & os,
		     OutputParams const & runparams,
		     string const & everypar)
{
	LASSERT(runparams.par_begin <= runparams.par_end,
		{ os << "% LaTeX Output Error\n"; return; } );

	BufferParams const & bparams = buf.params();

	bool const maintext = text.isMainText();
	bool const is_child = buf.masterBuffer() != &buf;

	// Open a CJK environment at the beginning of the main buffer
	// if the document's language is a CJK language
	// (but not in child documents)
	OutputState * state = getOutputState();
	if (maintext && !is_child
	    && bparams.encoding().package() == Encoding::CJK) {
		os << "\\begin{CJK}{" << from_ascii(bparams.encoding().latexName())
		<< "}{" << from_ascii(bparams.fonts_cjk) << "}%\n";
		state->open_encoding_ = CJK;
	}
	// if "auto begin" is switched off, explicitly switch the
	// language on at start
	string const mainlang = runparams.use_polyglossia
		? getPolyglossiaEnvName(bparams.language)
		: bparams.language->babel();
	string const lang_begin_command = runparams.use_polyglossia ?
		"\\begin{$$lang}" : lyxrc.language_command_begin;

	if (maintext && !lyxrc.language_auto_begin &&
	    !mainlang.empty()) {
		// FIXME UNICODE
		os << from_utf8(subst(lang_begin_command,
					"$$lang",
					mainlang));
		if (runparams.use_polyglossia
		    && !bparams.language->polyglossiaOpts().empty())
			os << "["
			    << from_ascii(bparams.language->polyglossiaOpts())
			    << "]";
		os << '\n';
	}

	ParagraphList const & paragraphs = text.paragraphs();

	if (runparams.par_begin == runparams.par_end) {
		// The full doc will be exported but it is easier to just rely on
		// runparams range parameters that will be passed TeXEnvironment.
		runparams.par_begin = 0;
		runparams.par_end = paragraphs.size();
	}

	pit_type pit = runparams.par_begin;
	// lastpit is for the language check after the loop.
	pit_type lastpit = pit;
	// variables used in the loop:
	bool was_title = false;
	bool already_title = false;
	DocumentClass const & tclass = bparams.documentClass();

	for (; pit < runparams.par_end; ++pit) {
		lastpit = pit;
		ParagraphList::const_iterator par = paragraphs.constIterator(pit);

		// FIXME This check should not be needed. We should
		// perhaps issue an error if it is.
		Layout const & layout = text.inset().forcePlainLayout() ?
				tclass.plainLayout() : par->layout();

		if (layout.intitle) {
			if (already_title) {
				LYXERR0("Error in latexParagraphs: You"
					" should not mix title layouts"
					" with normal ones.");
			} else if (!was_title) {
				was_title = true;
				if (tclass.titletype() == TITLE_ENVIRONMENT) {
					os << "\\begin{"
							<< from_ascii(tclass.titlename())
							<< "}\n";
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
			already_title = true;
			was_title = false;
		}


		if (!layout.isEnvironment() && par->params().leftIndent().zero()) {
			// This is a standard top level paragraph, TeX it and continue.
			TeXOnePar(buf, text, pit, os, runparams, everypar);
			continue;
		}
		
		TeXEnvironmentData const data =
			prepareEnvironment(buf, text, par, os, runparams);
		// pit can be changed in TeXEnvironment.
		TeXEnvironment(buf, text, runparams, pit, os);
		finishEnvironment(os, runparams, data);
	}

	if (pit == runparams.par_end) {
			// Make sure that the last paragraph is
			// correctly terminated (because TeXOnePar does
			// not add a \n in this case)
			//os << '\n';
	}

	// It might be that we only have a title in this document
	if (was_title && !already_title) {
		if (tclass.titletype() == TITLE_ENVIRONMENT) {
			os << "\\end{" << from_ascii(tclass.titlename())
			   << "}\n";
		} else {
			os << "\\" << from_ascii(tclass.titlename())
			   << "\n";
		}
	}

	// if "auto end" is switched off, explicitly close the language at the end
	// but only if the last par is in a babel language
	string const lang_end_command = runparams.use_polyglossia ?
		"\\end{$$lang}" : lyxrc.language_command_end;
	if (maintext && !lyxrc.language_auto_end && !mainlang.empty() &&
		paragraphs.at(lastpit).getParLanguage(bparams)->encoding()->package() != Encoding::CJK) {
		os << from_utf8(subst(lang_end_command,
					"$$lang",
					mainlang))
			<< '\n';
	}

	// If the last paragraph is an environment, we'll have to close
	// CJK at the very end to do proper nesting.
	if (maintext && !is_child && state->open_encoding_ == CJK) {
		os << "\\end{CJK}\n";
		state->open_encoding_ = none;
	}

	// reset inherited encoding
	if (state->cjk_inherited_ > 0) {
		state->cjk_inherited_ -= 1;
		if (state->cjk_inherited_ == 0)
			state->open_encoding_ = CJK;
	}
}


pair<bool, int> switchEncoding(odocstream & os, BufferParams const & bparams,
		   OutputParams const & runparams, Encoding const & newEnc,
		   bool force)
{
	Encoding const & oldEnc = *runparams.encoding;
	bool moving_arg = runparams.moving_arg;
	// If we switch from/to CJK, we need to switch anyway, despite custom inputenc
	bool const from_to_cjk = 
		(oldEnc.package() == Encoding::CJK && newEnc.package() != Encoding::CJK)
		|| (oldEnc.package() != Encoding::CJK && newEnc.package() == Encoding::CJK);
	if (!force && !from_to_cjk
	    && ((bparams.inputenc != "auto" && bparams.inputenc != "default") || moving_arg))
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
	OutputState * state = getOutputState();
	switch (newEnc.package()) {
		case Encoding::none:
		case Encoding::japanese:
			// shouldn't ever reach here, see above
			return make_pair(true, 0);
		case Encoding::inputenc: {
			int count = inputenc_arg.length();
			if (oldEnc.package() == Encoding::CJK &&
			    state->open_encoding_ == CJK) {
				os << "\\end{CJK}";
				state->open_encoding_ = none;
				count += 9;
			}
			else if (oldEnc.package() == Encoding::inputenc &&
				 state->open_encoding_ == inputenc) {
				os << "\\egroup";
				state->open_encoding_ = none;
				count += 7;
			}
			if (runparams.local_font != 0
			    && 	oldEnc.package() == Encoding::CJK) {
				// within insets, \inputenc switches need
				// to be embraced within \bgroup...\egroup;
				// else CJK fails.
				os << "\\bgroup";
				count += 7;
				state->open_encoding_ = inputenc;
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
			    state->open_encoding_ == CJK) {
				os << "\\end{CJK}";
				count += 9;
			}
			if (oldEnc.package() == Encoding::inputenc &&
			    state->open_encoding_ == inputenc) {
				os << "\\egroup";
				count += 7;
			}
			os << "\\begin{CJK}{" << inputenc_arg << "}{"
			   << from_ascii(bparams.fonts_cjk) << "}";
			state->open_encoding_ = CJK;
			return make_pair(true, count + 15);
		}
	}
	// Dead code to avoid a warning:
	return make_pair(true, 0);

}

} // namespace lyx
