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

using std::endl;

LaTeXFeatures::LaTeXFeatures(BufferParams const & p, LyXTextClass::size_type n)
	: layout(n, false), params(p)
{
	// packages
	array = false;
	color = false;
	graphics = false; // INSET_GRAPHICS: remove this when InsetFig is thrown.
	graphicx = false;
	setspace = false;
	makeidx = false;
	verbatim = false;
	longtable = false;
	//algorithm = false;
	rotating = false;
	amssymb = false;
	latexsym = false;
	pifont = false;
	subfigure = false;
	floatflt = false;
	url = false;
	varioref = false;
	prettyref = false;
	chess = false;
	natbib = false;
	floats = false;
	
	// commands
	lyx = false;
	lyxline = false;
	noun = false;
	lyxarrow = false;

	// quotes
	quotesinglbase = false;
	quotedblbase = false;
	guilsinglleft = false;
	guilsinglright = false;
	guillemotleft = false;
	guillemotright = false;

	// Math mode
	amsstyle = false;
	binom = false;
	boldsymbol = false;
    
	// special features
	ParagraphIndent = false;
	NeedLyXFootnoteCode = false;
	NeedLyXMinipageIndent = false;
}


void LaTeXFeatures::require(string const & name)
{
	if (name == "array") {
		array = true;
	} else if (name == "color") {
		color = true;
	} else if (name == "graphics") {
		graphicx = true;
		graphics = true;// INSET_GRAPHICS: remove this when InsetFig is thrown.
	} else if (name == "setspace") {
		setspace = true;
	} else if (name == "makeidx") {
		makeidx = true;
	} else if (name == "verbatim") {
		verbatim = true;
	} else if (name == "longtable") {
		longtable = true;
	//} else if (name == "algorithm") {
	//algorithm = true;
	} else if (name == "rotating") {
		rotating = true;
	} else if (name == "amssymb") {
		amssymb = true;
	} else if (name == "latexsym") {
		latexsym = true;
	} else if (name == "pifont") {
		pifont = true;
	} else if (name == "subfigure") {
		subfigure = true;
	} else if (name == "floatflt") {
		floatflt = true;
	} else if (name == "url") {
		url = true;
	} else if (name == "varioref") {
		varioref = true;
	} else if (name == "prettyref") {
		prettyref = true;
	} else if (name == "chess") {
		chess = true;
	} else if (name == "amsstyle") {
		amsstyle = true;
	} else if (name == "boldsymbol") {
		boldsymbol = true;
	} else if (name == "binom") {
		binom = true;
	} else if (name == "natbib") {
		natbib = true;
	} else if (name == "float") {
		floats = true;
	}
}


string const LaTeXFeatures::getPackages() const
{
	ostringstream packages;
	LyXTextClass const & tclass =
		textclasslist.TextClass(params.textclass);

	// array-package
	if (array)
		packages << "\\usepackage{array}\n";

	// color.sty
	if (color) {
		if (params.graphicsDriver == "default")
			packages << "\\usepackage{color}\n";
		else
			packages << "\\usepackage[" 
				 << params.graphicsDriver
				 << "]{color}\n";
	}
		
	// makeidx.sty
	if (makeidx) {
		if (! tclass.provides(LyXTextClass::makeidx))
			packages << "\\usepackage{makeidx}\n";
		packages << "\\makeindex\n";
	}

	// graphicx.sty
	if (graphicx && params.graphicsDriver != "none") {
		if (params.graphicsDriver == "default")
			packages << "\\usepackage{graphicx}\n";
		else
			packages << "\\usepackage[" 
				 << params.graphicsDriver
				 << "]{graphicx}\n";
	}

	// INSET_GRAPHICS: remove this when InsetFig is thrown.
	// graphics.sty
	if (graphics && params.graphicsDriver != "none") {
		if (params.graphicsDriver == "default")
			packages << "\\usepackage{graphics}\n";
		else
			packages << "\\usepackage[" 
				 << params.graphicsDriver
				 << "]{graphics}\n";
	}

	// verbatim.sty
	if (verbatim)
		packages << "\\usepackage{verbatim}\n";

	//if (algorithm) {
	//	packages << "\\usepackage{algorithm}\n";
	//}

	// lyxskak.sty --- newer chess support based on skak.sty
	if (chess) {
		packages << "\\usepackage[ps,mover]{lyxskak}\n";
	}

	// setspace.sty
	if ((params.spacing.getSpace() != Spacing::Single
	     && !params.spacing.isDefault())
	    || setspace) {
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

	//longtable.sty
	if (longtable)
		packages << "\\usepackage{longtable}\n";

	//rotating.sty
	if (rotating)
		packages << "\\usepackage{rotating}\n";

	// amssymb.sty
	if (amssymb || params.use_amsmath)
		packages << "\\usepackage{amssymb}\n";

	// latexsym.sty
	if (latexsym)
		packages << "\\usepackage{latexsym}\n";

	// pifont.sty
	if (pifont)
		packages << "\\usepackage{pifont}\n";

	// subfigure.sty
	if (subfigure)
		packages << "\\usepackage{subfigure}\n";

	// floatflt.sty
	if (floatflt)
		packages << "\\usepackage{floatflt}\n";

	// url.sty
	if (url && ! tclass.provides(LyXTextClass::url))
		packages << "\\IfFileExists{url.sty}{\\usepackage{url}}\n"
			    "                      {\\newcommand{\\url}{\\texttt}}\n";

	// varioref.sty
	if (varioref)
		packages << "\\usepackage{varioref}\n";

	// prettyref.sty
	if (prettyref)
		packages << "\\usepackage{prettyref}\n";

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
				const_cast<LaTeXFeatures *>(this)->floats = true;
				break;
			}
		}
	}
	if (floats) {
		packages << "\\usepackage{float}\n";
	}
	
	// natbib.sty
	if (natbib) {
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
	if (true || lyx) 
		macros << lyx_def << '\n';

	if (lyxline) 
		macros << lyxline_def << '\n';

	if (noun) {
		macros << noun_def << '\n';
	}

	if (lyxarrow) {
		macros << lyxarrow_def << '\n';
	}

	// quotes. 
	if (quotesinglbase)
		macros << quotesinglbase_def << '\n';
	if (quotedblbase)
		macros << quotedblbase_def << '\n';
	if (guilsinglleft)
		macros << guilsinglleft_def << '\n';
	if (guilsinglright)
		macros << guilsinglright_def << '\n';
	if (guillemotleft)
		macros << guillemotleft_def << '\n';
	if (guillemotright)
		macros << guillemotright_def << '\n';
    
        // Math mode    
	if (boldsymbol && !amsstyle)
		macros << boldsymbol_def << '\n';
	if (binom && !amsstyle)
		macros << binom_def << '\n';

	// other
        if (NeedLyXMinipageIndent) 
		macros << minipageindent_def;
        if (ParagraphIndent) 
		macros << paragraphindent_def;
        if (NeedLyXFootnoteCode) 
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
	LyXTextClass const & tclass =
		textclasslist.TextClass(params.textclass);
	ostringstream tcpreamble;
	
	tcpreamble << tclass.preamble();

	for (unsigned int i = 0; i < tclass.numLayouts(); ++i) {
		if (layout[i]) {
			tcpreamble << tclass[i].preamble();
		}
	}

	return tcpreamble.str().c_str();
}	


string const LaTeXFeatures::getLyXSGMLEntities() const
{
	// Definition of entities used in the document that are LyX related.
	ostringstream entities;

	if (lyxarrow) {
		entities << "<!ENTITY lyxarrow \"-&gt;\">"
			 << '\n';
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
			// outputted so that the same flotastyle will not be
			// used several times. when the same style is still in
			// effect. (Lgb)
		}
	}
}
