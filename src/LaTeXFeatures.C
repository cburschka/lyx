// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 the LyX Team.
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

extern LyXRC * lyxrc;

LaTeXFeatures::LaTeXFeatures(int n)
	: layout(n, false)
{
	// packages
	color = false;
	graphics = false;
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


string LaTeXFeatures::getPackages(BufferParams const & params)
{
	string packages;
	LyXTextClass const & tclass = textclasslist.TextClass(params.textclass);

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
		    && params.language != "french") // french provides
						    // \index !
			packages += "\\usepackage{makeidx}\n";
		packages += "\\makeindex\n";
	}

	// graphics.sty
	if (graphics && params.graphicsDriver != "none") {
		if (params.graphicsDriver == "default")
			packages += "\\usepackage{graphics}\n";
		else
			packages += "\\usepackage[" 
				+ params.graphicsDriver + "]{graphics}\n";
	}

	//verbatim.sty
	if (verbatim)
		packages += "\\usepackage{verbatim}\n";

	if (algorithm) {
		packages += "\\usepackage{algorithm}\n";
	}

	// setspace.sty
	if ((params.spacing.getSpace() != Spacing::Single)
	    || setspace) {
		packages += "\\usepackage{setspace}\n";
	}
	switch (params.spacing.getSpace()) {
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
		//char value[30];
		//sprintf(value, "%.2f", params.spacing.getValue());
#ifdef HAVE_SSTREAM
		ostringstream value;
#else
		char val[30];
		ostrstream value(val, 30);
		
#endif
		value << params.spacing.getValue(); // setw?
#ifdef HAVE_SSTREAM
		packages += string("\\setstretch{") 
			  + value.str().c_str() + "}\n";
#else
		value << '\0';
		packages += string("\\setstretch{") 
			  + value.str() + "}\n";
#endif
		break;
	}

	//longtable.sty
	if (longtable)
		packages += "\\usepackage{longtable}\n";

	//rotating.sty
	if (rotating)
		packages += "\\usepackage{rotating}\n";

	// amssymb.sty
	if (amssymb)
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
	
	return packages;
}


string LaTeXFeatures::getMacros(BufferParams const & /* params */)
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

	return macros;
}


string LaTeXFeatures::getTClassPreamble(BufferParams const & params)
{
	// the text class specific preamble 
	LyXTextClass const & tclass = textclasslist.TextClass(params.textclass);
	string tcpreamble = tclass.preamble();

	for (unsigned int i = 0; i < tclass.numLayouts(); ++i) {
		if (layout[i]) {
			tcpreamble += tclass[i].preamble();
		}
	}

	return tcpreamble;
}	


void LaTeXFeatures::showStruct(BufferParams const & params) {
	lyxerr << "LyX needs the following commands when LaTeXing:"
	// packs
	       << "\n***** Packages:" << getPackages(params)
	       << "\n***** Macros:" << getMacros(params)
	       << "\n***** Textclass stuff:" << getTClassPreamble(params)
	       << "\n***** done." << endl;
}
