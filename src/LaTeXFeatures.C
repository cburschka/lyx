/**
 * \file LaTeXFeatures.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author José Matos
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Jürgen Vigna
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "LaTeXFeatures.h"

#include "bufferparams.h"
#include "debug.h"
#include "encoding.h"
#include "Floating.h"
#include "FloatList.h"
#include "language.h"
#include "lyx_sty.h"
#include "lyxrc.h"

#include "support/filetools.h"

#include "support/std_sstream.h"

using lyx::support::IsSGMLFilename;
using lyx::support::MakeRelPath;
using lyx::support::OnlyPath;

using std::endl;
using std::find;
using std::string;
using std::list;
using std::ostream;
using std::ostringstream;
using std::set;

namespace biblio = lyx::biblio;


LaTeXFeatures::LaTeXFeatures(Buffer const & b, BufferParams const & p, bool n)
	: buffer_(&b), params_(p), nice_(n)
{}


bool LaTeXFeatures::useBabel() const
{
	return lyxrc.language_use_babel ||
		bufferParams().language->lang() != lyxrc.default_language ||
		this->hasLanguages();
}


void LaTeXFeatures::require(string const & name)
{
	if (isRequired(name))
		return;

	features_.push_back(name);
}


void LaTeXFeatures::useLayout(string const & layoutname)
{
	// Some code to avoid loops in dependency definition
	static int level = 0;
	const int maxlevel = 30;
	if (level > maxlevel) {
		lyxerr << "LaTeXFeatures::useLayout: maximum level of "
		       << "recursion attained by layout "
		       << layoutname << endl;
		return;
	}

	LyXTextClass const & tclass = params_.getLyXTextClass();
	if (tclass.hasLayout(layoutname)) {
		// Is this layout already in usedLayouts?
		list<string>::const_iterator cit = usedLayouts_.begin();
		list<string>::const_iterator end = usedLayouts_.end();
		for (; cit != end; ++cit) {
			if (layoutname == *cit)
				return;
		}

		LyXLayout_ptr const & lyt = tclass[layoutname];
		if (!lyt->depends_on().empty()) {
			++level;
			useLayout(lyt->depends_on());
			--level;
		}
		usedLayouts_.push_back(layoutname);
	} else {
		lyxerr << "LaTeXFeatures::useLayout: layout `"
		       << layoutname << "' does not exist in this class"
		       << endl;
	}

	--level;
}


bool LaTeXFeatures::isRequired(string const & name) const
{
	return find(features_.begin(), features_.end(), name) != features_.end();
}


void LaTeXFeatures::addExternalPreamble(string const & preamble)
{
	FeaturesList::const_iterator begin = preamble_snippets_.begin();
	FeaturesList::const_iterator end   = preamble_snippets_.end();
	if (find(begin, end, preamble) == end)
		preamble_snippets_.push_back(preamble);
}


void LaTeXFeatures::useFloat(string const & name)
{
	usedFloats_.insert(name);
	// We only need float.sty if we use non builtin floats, or if we
	// use the "H" modifier. This includes modified table and
	// figure floats. (Lgb)
	Floating const & fl = params_.getLyXTextClass().floats().getType(name);
	if (!fl.type().empty() && !fl.builtin()) {
		require("float");
	}
}


void LaTeXFeatures::useLanguage(Language const * lang)
{
	UsedLanguages_.insert(lang);
}


void LaTeXFeatures::includeFile(string const & key, string const & name)
{
	IncludedFiles_[key] = name;
}


bool LaTeXFeatures::hasLanguages() const
{
	return !UsedLanguages_.empty();
}


string LaTeXFeatures::getLanguages() const
{
	ostringstream languages;

	for (LanguageList::const_iterator cit =
		    UsedLanguages_.begin();
	     cit != UsedLanguages_.end();
	     ++cit)
		languages << (*cit)->babel() << ',';

	return languages.str();
}


set<string> LaTeXFeatures::getEncodingSet(string const & doc_encoding) const
{
	set<string> encodings;
	LanguageList::const_iterator it  = UsedLanguages_.begin();
	LanguageList::const_iterator end = UsedLanguages_.end();
	for (; it != end; ++it)
		if ((*it)->encoding()->LatexName() != doc_encoding)
			encodings.insert((*it)->encoding()->LatexName());
	return encodings;
}

namespace {

char const * simplefeatures[] = {
	"array",
	"verbatim",
	"longtable",
	"rotating",
	"latexsym",
	"pifont",
	"subfigure",
	"floatflt",
	"varioref",
	"prettyref",
	"float",
	"wasy",
	"dvipost",
	"fancybox",
	"calc",
	"jurabib"
};

int const nb_simplefeatures = sizeof(simplefeatures) / sizeof(char const *);

}


string const LaTeXFeatures::getPackages() const
{
	ostringstream packages;
	LyXTextClass const & tclass = params_.getLyXTextClass();

	//
	//  These are all the 'simple' includes.  i.e
	//  packages which we just \usepackage{package}
	//
	for (int i = 0; i < nb_simplefeatures; ++i) {
		if (isRequired(simplefeatures[i]))
			packages << "\\usepackage{"
				 << simplefeatures[i] << "}\n";
	}

	//
	// The rest of these packages are somewhat more complicated
	// than those above.
	//

	if (isRequired("amsmath")
	    && !tclass.provides(LyXTextClass::amsmath)
	    && params_.use_amsmath != BufferParams::AMS_OFF) {
		packages << "\\usepackage{amsmath}\n";
	}

	// color.sty
	if (isRequired("color")) {
		if (params_.graphicsDriver == "default")
			packages << "\\usepackage[usenames]{color}\n";
		else
			packages << "\\usepackage["
				 << params_.graphicsDriver
				 << ",usenames"
				 << "]{color}\n";
	}

	// makeidx.sty
	if (isRequired("makeidx")) {
		if (! tclass.provides(LyXTextClass::makeidx))
			packages << "\\usepackage{makeidx}\n";
		packages << "\\makeindex\n";
	}

	// graphicx.sty
	if (isRequired("graphicx") && params_.graphicsDriver != "none") {
		if (params_.graphicsDriver == "default")
			packages << "\\usepackage{graphicx}\n";
		else
			packages << "\\usepackage["
				 << params_.graphicsDriver
				 << "]{graphicx}\n";
	}

	//if (algorithm) {
	//	packages << "\\usepackage{algorithm}\n";
	//}

	// lyxskak.sty --- newer chess support based on skak.sty
	if (isRequired("chess")) {
		packages << "\\usepackage[ps,mover]{lyxskak}\n";
	}

	// setspace.sty
	if ((params_.spacing().getSpace() != Spacing::Single
	     && !params_.spacing().isDefault())
	    || isRequired("setspace")) {
		packages << "\\usepackage{setspace}\n";
	}
	switch (params_.spacing().getSpace()) {
	case Spacing::Default:
	case Spacing::Single:
		// we dont use setspace.sty so dont print anything
		//packages += "\\singlespacing\n";
		break;
	case Spacing::Onehalf:
		packages << "\\onehalfspacing\n";
		break;
	case Spacing::Double:
		packages << "\\doublespacing\n";
		break;
	case Spacing::Other:
		packages << "\\setstretch{"
			 << params_.spacing().getValue() << "}\n";
		break;
	}

	// amssymb.sty
	if (isRequired("amssymb") || params_.use_amsmath == BufferParams::AMS_ON)
		packages << "\\usepackage{amssymb}\n";
	// url.sty
	if (isRequired("url") && ! tclass.provides(LyXTextClass::url))
		packages << "\\IfFileExists{url.sty}{\\usepackage{url}}\n"
			    "                      {\\newcommand{\\url}{\\texttt}}\n";

	// float.sty
	// natbib.sty
	if (isRequired("natbib") && ! tclass.provides(LyXTextClass::natbib)) {
		packages << "\\usepackage[";
		if (params_.cite_engine == biblio::ENGINE_NATBIB_NUMERICAL) {
			packages << "numbers";
		} else {
			packages << "authoryear";
		}
		packages << "]{natbib}\n";
	}

	// bibtopic -- the dot provides the aux file naming which
	// LyX can detect.
	if (isRequired("bibtopic")) {
		packages << "\\usepackage[dot]{bibtopic}\n";
	}

	return packages.str();
}


string const LaTeXFeatures::getMacros() const
{
	ostringstream macros;

	if (!preamble_snippets_.empty())
		macros << '\n';
	FeaturesList::const_iterator pit  = preamble_snippets_.begin();
	FeaturesList::const_iterator pend = preamble_snippets_.end();
	for (; pit != pend; ++pit) {
		macros << *pit << '\n';
	}

	if (isRequired("LyX"))
		macros << lyx_def << '\n';

	if (isRequired("lyxline"))
		macros << lyxline_def << '\n';

	if (isRequired("noun"))
		macros << noun_def << '\n';

	if (isRequired("lyxarrow"))
		macros << lyxarrow_def << '\n';

	// quotes.
	if (isRequired("quotesinglbase"))
		macros << quotesinglbase_def << '\n';
	if (isRequired("quotedblbase"))
		macros << quotedblbase_def << '\n';
	if (isRequired("guilsinglleft"))
		macros << guilsinglleft_def << '\n';
	if (isRequired("guilsinglright"))
		macros << guilsinglright_def << '\n';
	if (isRequired("guillemotleft"))
		macros << guillemotleft_def << '\n';
	if (isRequired("guillemotright"))
		macros << guillemotright_def << '\n';

	// Math mode
	if (isRequired("boldsymbol") && !isRequired("amsmath"))
		macros << boldsymbol_def << '\n';
	if (isRequired("binom") && !isRequired("amsmath"))
		macros << binom_def << '\n';
	if (isRequired("mathcircumflex"))
		macros << mathcircumflex_def << '\n';

	// other
	if (isRequired("ParagraphLeftIndent"))
		macros << paragraphleftindent_def;
	if (isRequired("NeedLyXFootnoteCode"))
		macros << floatingfootnote_def;

	// some problems with tex->html converters
	if (isRequired("NeedTabularnewline"))
		macros << tabularnewline_def;

	// greyedout environment (note inset)
	if (isRequired("lyxgreyedout"))
		macros << lyxgreyedout_def;

	if (isRequired("lyxdot"))
		macros << lyxdot_def << '\n';

	// floats
	getFloatDefinitions(macros);

	return macros.str();
}


string const LaTeXFeatures::getBabelOptions() const
{
	ostringstream tmp;

	LanguageList::const_iterator it  = UsedLanguages_.begin();
	LanguageList::const_iterator end =  UsedLanguages_.end();
	for (; it != end; ++it)
		if (!(*it)->latex_options().empty())
			tmp << (*it)->latex_options() << '\n';
	if (!params_.language->latex_options().empty())
		tmp << params_.language->latex_options() << '\n';

	return tmp.str();
}


string const LaTeXFeatures::getTClassPreamble() const
{
	// the text class specific preamble
	LyXTextClass const & tclass = params_.getLyXTextClass();
	ostringstream tcpreamble;

	tcpreamble << tclass.preamble();

	list<string>::const_iterator cit = usedLayouts_.begin();
	list<string>::const_iterator end = usedLayouts_.end();
	for (; cit != end; ++cit) {
		tcpreamble << tclass[*cit]->preamble();
	}

	CharStyles::iterator cs = tclass.charstyles().begin();
	CharStyles::iterator csend = tclass.charstyles().end();
	for (; cs != csend; ++cs) {
		if (isRequired(cs->name))
			tcpreamble << cs->preamble;
	}

	return tcpreamble.str();
}


string const LaTeXFeatures::getLyXSGMLEntities() const
{
	// Definition of entities used in the document that are LyX related.
	ostringstream entities;

	if (isRequired("lyxarrow")) {
		entities << "<!ENTITY lyxarrow \"-&gt;\">" << '\n';
	}

	return entities.str();
}


string const LaTeXFeatures::getIncludedFiles(string const & fname) const
{
	ostringstream sgmlpreamble;
	string const basename = OnlyPath(fname);

	FileMap::const_iterator end = IncludedFiles_.end();
	for (FileMap::const_iterator fi = IncludedFiles_.begin();
	     fi != end; ++fi)
		sgmlpreamble << "\n<!ENTITY " << fi->first
			     << (IsSGMLFilename(fi->second) ? " SYSTEM \"" : " \"")
			     << MakeRelPath(fi->second, basename) << "\">";

	return sgmlpreamble.str();
}


void LaTeXFeatures::showStruct() const {
	lyxerr << "LyX needs the following commands when LaTeXing:"
	       << "\n***** Packages:" << getPackages()
	       << "\n***** Macros:" << getMacros()
	       << "\n***** Textclass stuff:" << getTClassPreamble()
	       << "\n***** done." << endl;
}


Buffer const & LaTeXFeatures::buffer() const
{
	return *buffer_;
}


void LaTeXFeatures::setBuffer(Buffer const & buffer)
{
	buffer_ = &buffer;
}


BufferParams const & LaTeXFeatures::bufferParams() const
{
	return params_;
}


void LaTeXFeatures::getFloatDefinitions(ostream & os) const
{
	FloatList const & floats = params_.getLyXTextClass().floats();

	// Here we will output the code to create the needed float styles.
	// We will try to do this as minimal as possible.
	// \floatstyle{ruled}
	// \newfloat{algorithm}{htbp}{loa}
	// \floatname{algorithm}{Algorithm}
	UsedFloats::const_iterator cit = usedFloats_.begin();
	UsedFloats::const_iterator end = usedFloats_.end();
	// ostringstream floats;
	for (; cit != end; ++cit) {
		Floating const & fl = floats.getType((*cit));

		// For builtin floats we do nothing.
		if (fl.builtin()) continue;

		// We have to special case "table" and "figure"
		if (fl.type() == "tabular" || fl.type() == "figure") {
			// Output code to modify "table" or "figure"
			// but only if builtin == false
			// and that have to be true at this point in the
			// function.
			string const type = fl.type();
			string const placement = fl.placement();
			string const style = fl.style();
			if (!style.empty()) {
				os << "\\floatstyle{" << style << "}\n"
				   << "\\restylefloat{" << type << "}\n";
			}
			if (!placement.empty()) {
				os << "\\floatplacement{" << type << "}{"
				   << placement << "}\n";
			}
		} else {
			// The other non builtin floats.

			string const type = fl.type();
			string const placement = fl.placement();
			string const ext = fl.ext();
			string const within = fl.within();
			string const style = fl.style();
			string const name = fl.name();
			os << "\\floatstyle{" << style << "}\n"
			   << "\\newfloat{" << type << "}{" << placement
			   << "}{" << ext << '}';
			if (!within.empty())
				os << '[' << within << ']';
			os << '\n'
			   << "\\floatname{" << type << "}{"
			   << name << "}\n";

			// What missing here is to code to minimalize the code
			// output so that the same floatstyle will not be
			// used several times, when the same style is still in
			// effect. (Lgb)
		}
	}
}
