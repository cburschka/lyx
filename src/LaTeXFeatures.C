/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 the LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation "LaTeXFeatures.h"
#endif

#include "LString.h"
#include "debug.h"
#include "lyx_sty.h"
#include "lyxrc.h"
#include "LaTeXFeatures.h"
#include "bufferparams.h"
#include "layout.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "FloatList.h"
#include "language.h"
#include "encoding.h"

using std::endl;
using std::set;

using lyx::layout_type;
using lyx::textclass_type;

LaTeXFeatures::LaTeXFeatures(BufferParams const & p, layout_type n)
	: layout(n, false), params(p)
{}


void LaTeXFeatures::require(string const & name)
{
	// INSET_GRAPHICS: remove this when InsetFig is thrown.
	if (name == "graphics") {
		features.push_back("graphicx");
		features.push_back("graphics");
	} else
		features.push_back(name);
}


void LaTeXFeatures::useLayout(std::vector<bool>::size_type const & idx)
{
	layout[idx] = true;
}


bool LaTeXFeatures::isRequired(string const & name) const
{
	FeaturesList::const_iterator i = std::find(features.begin(), 
						   features.end(),
						   name);
	return i != features.end();
}


void LaTeXFeatures::addExternalPreamble(string const & pream)
{
	externalPreambles += pream;
}


void LaTeXFeatures::useFloat(string const & name)
{
	usedFloats.insert(name);
}


void LaTeXFeatures::useLanguage(Language const * lang)
{
	UsedLanguages.insert(lang);
}


void LaTeXFeatures::includeFile(string const & key, string const & name) 
{
	IncludedFiles[key] = name;
}


bool LaTeXFeatures::hasLanguages() 
{
	return !UsedLanguages.empty();
}


string LaTeXFeatures::getLanguages() const 
{
	ostringstream languages;

	for (LanguageList::const_iterator cit =
		    UsedLanguages.begin();
	     cit != UsedLanguages.end(); 
	     ++cit)
		languages << (*cit)->babel() << ',';

	return languages.str().c_str();
}


set<string> LaTeXFeatures::getEncodingSet(string const & doc_encoding) 
{
	set<string> encodings;
	for (LanguageList::const_iterator it =
		     UsedLanguages.begin();
	     it != UsedLanguages.end(); ++it)
		if ((*it)->encoding()->LatexName() != doc_encoding)
			encodings.insert((*it)->encoding()->LatexName());
	return encodings;
}


string const LaTeXFeatures::getPackages() const
{
	ostringstream packages;
	LyXTextClass const & tclass =
		textclasslist.TextClass(params.textclass);


	/**
	 *  These are all the 'simple' includes.  i.e
	 *  packages which we just \usepackage{package}
	 **/

	// array-package
	if (isRequired("array"))
		packages << "\\usepackage{array}\n";

	// verbatim.sty
	if (isRequired("verbatim"))
		packages << "\\usepackage{verbatim}\n";

	//longtable.sty
	if (isRequired("longtable"))
		packages << "\\usepackage{longtable}\n";

	//rotating.sty
	if (isRequired("rotating"))
		packages << "\\usepackage{rotating}\n";


	// latexsym.sty
	if (isRequired("latexsym"))
		packages << "\\usepackage{latexsym}\n";

	// pifont.sty
	if (isRequired("pifont"))
		packages << "\\usepackage{pifont}\n";

	// subfigure.sty
	if (isRequired("subfigure"))
		packages << "\\usepackage{subfigure}\n";

	// floatflt.sty
	if (isRequired("floatflt"))
		packages << "\\usepackage{floatflt}\n";


	// varioref.sty
	if (isRequired("varioref"))
		packages << "\\usepackage{varioref}\n";

	// prettyref.sty
	if (isRequired("prettyref"))
		packages << "\\usepackage{prettyref}\n";

	/**
	 * The rest of these packages are somewhat more complicated
	 * than those above.
	 **/

	// color.sty
	if (isRequired("color")) {
		if (params.graphicsDriver == "default")
			packages << "\\usepackage{color}\n";
		else
			packages << "\\usepackage[" 
				 << params.graphicsDriver
				 << "]{color}\n";
	}
		
	// makeidx.sty
	if (isRequired("makeidx")) {
		if (! tclass.provides(LyXTextClass::makeidx))
			packages << "\\usepackage{makeidx}\n";
		packages << "\\makeindex\n";
	}

	// graphicx.sty
	if (isRequired("graphicx") && params.graphicsDriver != "none") {
		if (params.graphicsDriver == "default")
			packages << "\\usepackage{graphicx}\n";
		else
			packages << "\\usepackage[" 
				 << params.graphicsDriver
				 << "]{graphicx}\n";
	}

	// INSET_GRAPHICS: remove this when InsetFig is thrown.
	// graphics.sty
	if (isRequired("graphics") && params.graphicsDriver != "none") {
		if (params.graphicsDriver == "default")
			packages << "\\usepackage{graphics}\n";
		else
			packages << "\\usepackage[" 
				 << params.graphicsDriver
				 << "]{graphics}\n";
	}

	//if (algorithm) {
	//	packages << "\\usepackage{algorithm}\n";
	//}

	// lyxskak.sty --- newer chess support based on skak.sty
	if (isRequired("chess")) {
		packages << "\\usepackage[ps,mover]{lyxskak}\n";
	}

	// setspace.sty
	if ((params.spacing.getSpace() != Spacing::Single
	     && !params.spacing.isDefault())
	    || isRequired("setspace")) {
		packages << "\\usepackage{setspace}\n";
	}
	switch (params.spacing.getSpace()) {
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
			 << params.spacing.getValue() << "}\n";
		break;
	}

	// amssymb.sty
	if (isRequired("amssymb") || params.use_amsmath)
		packages << "\\usepackage{amssymb}\n";
	// url.sty
	if (isRequired("url") && ! tclass.provides(LyXTextClass::url))
		packages << "\\IfFileExists{url.sty}{\\usepackage{url}}\n"
			    "                      {\\newcommand{\\url}{\\texttt}}\n";

	// float.sty
	// We only need float.sty if we use non builtin floats, or if we
	// use the "H" modifier. This includes modified table and
	// figure floats. (Lgb)
	if (!usedFloats.empty()) {
		UsedFloats::const_iterator beg = usedFloats.begin();
		UsedFloats::const_iterator end = usedFloats.end();
		for (; beg != end; ++beg) {
			Floating const & fl = floatList.getType((*beg));
			if (!fl.type().empty() && !fl.builtin()) {
				const_cast<LaTeXFeatures *>(this)->require("floats");
				break;
			}
		}
	}
	if (isRequired("floats")) {
		packages << "\\usepackage{float}\n";
	}
	
	// natbib.sty
	if (isRequired("natbib")) {
		packages << "\\usepackage[";
		if (params.use_numerical_citations) {
			packages << "numbers";
		} else {
			packages << "authoryear";
		}
		packages << "]{natbib}\n";
	}
	
	packages << externalPreambles;

	return packages.str().c_str();
}


string const LaTeXFeatures::getMacros() const
{
	ostringstream macros;

	// always include this
	if (true || isRequired("lyx")) 
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
	if (isRequired("boldsymbol") && !isRequired("amsstyle"))
		macros << boldsymbol_def << '\n';
	if (isRequired("binom") && !isRequired("amsstyle"))
		macros << binom_def << '\n';

	// other
	if (isRequired("NeedLyXMinipageIndent"))
		macros << minipageindent_def;
	if (isRequired("ParagraphIndent")) 
		macros << paragraphindent_def;
	if (isRequired("NeedLyXFootnoteCode")) 
		macros << floatingfootnote_def;

	// floats
	getFloatDefinitions(macros);

	for (LanguageList::const_iterator cit = UsedLanguages.begin();
	     cit != UsedLanguages.end(); ++cit)
		if (!(*cit)->latex_options().empty())
			macros << (*cit)->latex_options() << '\n';
	if (!params.language->latex_options().empty())
		macros << params.language->latex_options() << '\n';

	return macros.str().c_str();
}


string const LaTeXFeatures::getTClassPreamble() const
{
	// the text class specific preamble 
	LyXTextClass const & tclass = textclasslist.TextClass(params.textclass);
	ostringstream tcpreamble;
	
	tcpreamble << tclass.preamble();

	for (layout_type i = 0; i < tclass.numLayouts(); ++i) {
		if (layout[i]) {
			tcpreamble  << tclass[i].preamble();
		}
	}

	return tcpreamble.str().c_str();
}	


string const LaTeXFeatures::getLyXSGMLEntities() const
{
	// Definition of entities used in the document that are LyX related.
	ostringstream entities;

	if (isRequired("lyxarrow")) {
		entities << "<!ENTITY lyxarrow \"-&gt;\">" << '\n';
	}

	return entities.str().c_str();
}


string const LaTeXFeatures::getIncludedFiles(string const & fname) const
{
	ostringstream sgmlpreamble;
	string const basename = OnlyPath(fname);

	FileMap::const_iterator end = IncludedFiles.end();
	for (FileMap::const_iterator fi = IncludedFiles.begin();
	     fi != end; ++fi)
		sgmlpreamble << "\n<!ENTITY " << fi->first
			     << (IsSGMLFilename(fi->second) ? " SYSTEM \"" : " \"" )
			     << MakeRelPath(fi->second, basename) << "\">";

	return sgmlpreamble.str().c_str();
}


void LaTeXFeatures::showStruct() const {
	lyxerr << "LyX needs the following commands when LaTeXing:"
	       << "\n***** Packages:" << getPackages()
	       << "\n***** Macros:" << getMacros()
	       << "\n***** Textclass stuff:" << getTClassPreamble()
	       << "\n***** done." << endl;
}


BufferParams const & LaTeXFeatures::bufferParams() const
{
	return params;
}

void LaTeXFeatures::getFloatDefinitions(std::ostream & os) const
{
	// Here we will output the code to create the needed float styles.
	// We will try to do this as minimal as possible.
	// \floatstyle{ruled}
	// \newfloat{algorithm}{htbp}{loa}
	// \floatname{algorithm}{Algorithm}
	UsedFloats::const_iterator cit = usedFloats.begin();
	UsedFloats::const_iterator end = usedFloats.end();
	// ostringstream floats;
	for (; cit != end; ++cit) {
		Floating const & fl = floatList.getType((*cit));
		
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
			   << "}{" << ext << "}";
			if (!within.empty())
				os << "[" << within << "]";
			os << "\n"
			   << "\\floatname{" << type << "}{"
			   << name << "}\n";
			
			// What missing here is to code to minimalize the code
			// outputted so that the same floatstyle will not be
			// used several times, when the same style is still in
			// effect. (Lgb)
		}
	}
}
