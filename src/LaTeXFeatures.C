// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 the LyX Team.
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
#include "FloatList.h"

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
	algorithm = false;
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
	LyXParagraphIndent = false;
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
	} else if (name == "algorithm") {
		algorithm = true;
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
	}
}


string const LaTeXFeatures::getPackages()
{
	string packages;
	LyXTextClass const & tclass =
		textclasslist.TextClass(params.textclass);

	// array-package
	if (array)
		packages += "\\usepackage{array}\n";

	// color.sty
	if (color) {
		if (params.graphicsDriver == "default")
			packages += "\\usepackage{color}\n";
		else
			packages += "\\usepackage[" 
				+ params.graphicsDriver + "]{color}\n";
	}
		
	// makeidx.sty
	if (makeidx) {
		if (! tclass.provides(LyXTextClass::makeidx)
		    && params.language->babel() != "french") // french provides
						             // \index !
			packages += "\\usepackage{makeidx}\n";
		packages += "\\makeindex\n";
	}

	// graphicx.sty
	if (graphicx && params.graphicsDriver != "none") {
		if (params.graphicsDriver == "default")
			packages += "\\usepackage{graphicx}\n";
		else
			packages += "\\usepackage[" 
				+ params.graphicsDriver + "]{graphicx}\n";
	}

	// INSET_GRAPHICS: remove this when InsetFig is thrown.
	// graphics.sty
	if (graphics && params.graphicsDriver != "none") {
		if (params.graphicsDriver == "default")
			packages += "\\usepackage{graphics}\n";
		else
			packages += "\\usepackage[" 
				+ params.graphicsDriver + "]{graphics}\n";
	}

	// verbatim.sty
	if (verbatim)
		packages += "\\usepackage{verbatim}\n";

	if (algorithm) {
		packages += "\\usepackage{algorithm}\n";
	}

	// lyxchess.sty
	if (chess) {
		packages += "\\usepackage{lyxchess}\n";
	}

	// setspace.sty
	if ((params.spacing.getSpace() != Spacing::Single
	     && !params.spacing.isDefault())
	    || setspace) {
		packages += "\\usepackage{setspace}\n";
	}
	switch (params.spacing.getSpace()) {
	case Spacing::Default:
	case Spacing::Single:
		// we dont use setspace.sty so dont print anything
		//packages += "\\singlespacing\n";
		break;
	case Spacing::Onehalf:
		packages += "\\onehalfspacing\n";
		break;
	case Spacing::Double:
		packages += "\\doublespacing\n";
		break;
	case Spacing::Other:
		std::ostringstream value;
		value << params.spacing.getValue(); // setw?
		packages += string("\\setstretch{") 
			  + value.str().c_str() + "}\n";
		break;
	}

	//longtable.sty
	if (longtable)
		packages += "\\usepackage{longtable}\n";

	//rotating.sty
	if (rotating)
		packages += "\\usepackage{rotating}\n";

	// amssymb.sty
	if (amssymb || params.use_amsmath)
		packages += "\\usepackage{amssymb}\n";

	// latexsym.sty
	if (latexsym)
		packages += "\\usepackage{latexsym}\n";

	// pifont.sty
	if (pifont)
		packages += "\\usepackage{pifont}\n";

	// subfigure.sty
	if (subfigure)
		packages += "\\usepackage{subfigure}\n";

	// floatflt.sty
	if (floatflt)
		packages += "\\usepackage{floatflt}\n";

	// url.sty
	if (url && ! tclass.provides(LyXTextClass::url))
		packages += "\\IfFileExists{url.sty}{\\usepackage{url}}\n"
			    "                      {\\newcommand{\\url}{\\texttt}}\n";

	// varioref.sty
	if (varioref)
		packages += "\\usepackage{varioref}\n";

	// prettyref.sty
	if (prettyref)
		packages += "\\usepackage{prettyref}\n";

	// float.sty
	// We only need float.sty if we use non builtin floats. This includes
	// modified table and figure floats. (Lgb)
	if (!usedFloats.empty()) {
		bool use_float = false;
		UsedFloats::const_iterator beg = usedFloats.begin();
		UsedFloats::const_iterator end = usedFloats.end();
		for (; beg != end; ++beg) {
			Floating const & fl = floatList.getType((*beg));
			if (!fl.type().empty() && !fl.builtin()) {
				use_float = true;
				break;
			}
		}
		if (use_float)
			packages += "\\usepackage{float}\n";
	}
	
	packages += externalPreambles;

	return packages;
}


string const LaTeXFeatures::getMacros()
{
	string macros;

	// always include this
	if (true || lyx) 
		macros += lyx_def + '\n';

	if (lyxline) 
		macros += lyxline_def + '\n';

	if (noun) {
		macros += noun_def + '\n';
	}

	if (lyxarrow) {
		macros += lyxarrow_def + '\n';
	}

	// quotes. 
	if (quotesinglbase)
		macros += quotesinglbase_def + '\n';
	if (quotedblbase)
		macros += quotedblbase_def + '\n';
	if (guilsinglleft)
		macros += guilsinglleft_def + '\n';
	if (guilsinglright)
		macros += guilsinglright_def + '\n';
	if (guillemotleft)
		macros += guillemotleft_def + '\n';
	if (guillemotright)
		macros += guillemotright_def + '\n';
    
        // Math mode    
	if (boldsymbol && !amsstyle)
		macros += boldsymbol_def + '\n';
	if (binom && !amsstyle)
		macros += binom_def + '\n';

	// other
        if (NeedLyXMinipageIndent) 
		macros += minipageindent_def;
        if (LyXParagraphIndent) 
		macros += paragraphindent_def;
        if (NeedLyXFootnoteCode) 
		macros += floatingfootnote_def;

	// floats
	// Here we will output the code to create the needed float styles.
	// We will try to do this as minimal as possible.
	// \floatstyle{ruled}
	// \newfloat{algorithm}{htbp}{loa}
	// \floatname{algorithm}{Algorithm}
	UsedFloats::const_iterator beg = usedFloats.begin();
	UsedFloats::const_iterator end = usedFloats.end();
	for (; beg != end; ++beg) {
		Floating const & fl = floatList.getType((*beg));
		
		// We have to special case "table" and "figure"
		if ((fl.type() == "tabular" && !fl.builtin()) ||
		    (fl.type() == "figure" && !fl.builtin())) {
			// Output code to modify "table" or "figure"
			// but only if builtin == false
			
		} else {
			// The other non builtin floats.
		}
	}

	for (LanguageList::const_iterator cit = UsedLanguages.begin();
	     cit != UsedLanguages.end(); ++cit)
		if (!(*cit)->latex_options().empty())
			macros += (*cit)->latex_options() + '\n';
	if (!params.language->latex_options().empty())
		macros += params.language->latex_options() + '\n';

	return macros;
}


string const LaTeXFeatures::getTClassPreamble()
{
	// the text class specific preamble 
	LyXTextClass const & tclass =
		textclasslist.TextClass(params.textclass);
	string tcpreamble = tclass.preamble();

	for (unsigned int i = 0; i < tclass.numLayouts(); ++i) {
		if (layout[i]) {
			tcpreamble += tclass[i].preamble();
		}
	}

	return tcpreamble;
}	


string const LaTeXFeatures::getIncludedFiles(string const fname) const
{
	string sgmlpreamble;
	string basename = BasePath(fname);

	FileMap::const_iterator end = IncludedFiles.end();
	for (FileMap::const_iterator fi = IncludedFiles.begin();
	     fi != end; ++fi)
	  sgmlpreamble += "\n<!ENTITY " + fi->first
			+ (IsSGMLFilename(fi->second) ? " SYSTEM \"" : " \"" )
			+ MakeRelPath(fi->second,basename) + "\">";

	return sgmlpreamble;
}


void LaTeXFeatures::showStruct() {
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
