// -*- C++ -*-
/**
 * \file outputparams.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 *  \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef OUTPUTPARAMS_H
#define OUTPUTPARAMS_H

struct OutputParams {
        enum FLAVOR {
                LATEX,
                PDFLATEX
        };

        OutputParams() : flavor(LATEX), nice(false),
			   moving_arg(false), free_spacing(false),
			   use_babel(false), mixed_content(false),
			   linelen(0) {}

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

	/** Used for docbook to see if inside a region of mixed content.
	    In that case all the white spaces are significant and can not appear
	    at the begin or end.
	 */
	bool mixed_content;

	/** Line lenght to use with ascii export.
	 */
	int linelen;
};

#endif // LATEXRUNPARAMS_H
