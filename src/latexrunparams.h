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

/** The latex that we export depends occasionally on what is to
    compile the file.
*/
struct LatexRunParams {
        enum FLAVOR {
                LATEX,
                PDFLATEX
        };

        LatexRunParams() : flavor(LATEX) {}
	//, nice(false), fragile(false) {}

        FLAVOR flavor;
//         bool nice;
//         bool fragile;
};

// enum LatexFlavor {
// 	LATEX_FLAVOR,
// 	PDFLATEX_FLAVOR
// };

#endif // LatexRunParams_H
