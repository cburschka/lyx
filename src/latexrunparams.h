// -*- C++ -*-
/**
 * \file latexrunparams.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 *  \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef LatexRunParams_H
#define LatexRunParams_H

struct LatexRunParams {
        enum FLAVOR {
                LATEX,
                PDFLATEX
        };

        LatexRunParams() : flavor(LATEX), nice(false) {}

	/** The latex that we export depends occasionally on what is to
	    compile the file.
	*/
        FLAVOR flavor;
	/** Are we to write a 'nice' LaTeX file or not.
	    This esentially seems to mean whether InsetInclude, InsetGraphics
	    and InsetExternal should add the absolute path to any external
	    files or not.
	*/
	bool nice;
};

#endif // LatexRunParams_H
