// -*- C++ -*-
/**
 * \file latexrunparams.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 *  \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LatexRunParams_H
#define LatexRunParams_H

struct LatexRunParams {
        enum FLAVOR {
                LATEX,
                PDFLATEX
        };

        LatexRunParams() : flavor(LATEX), nice(false),
			   moving_arg(false), free_spacing(false),
			   use_babel(false) {}

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

	/** moving_arg == true means that the environment in which the inset
	    is typeset is a moving argument. The inset should take care about
	    fragile commands by preceding the latex with \protect.
	*/
	bool moving_arg;

	/** free_spacing == true means that the inset is in a free-spacing
	    paragraph.
	*/
	bool free_spacing;
	
	/** This var is set by the return value from BufferParams::writeLaTeX
	 */
	bool use_babel;
};

#endif // LATEXRUNPARAMS_H
