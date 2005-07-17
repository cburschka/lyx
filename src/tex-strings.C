/**
 * \file tex-strings.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "tex-strings.h"


// this file should perhaps be split into even smaller parts

char const * const string_paragraph_separation[] = {
	"indent", "skip", ""
};


char const * const string_quotes_language[] = {
	"english", "swedish", "german", "polish", "french", "danish", ""
};


char const * const string_papersize[] = {
	"default", "custom", "letterpaper", "executivepaper", "legalpaper",
	"a3paper", "a4paper", "a5paper", "b3paper", "b4paper", "b5paper", ""
};


char const * const string_orientation[] = {
	"portrait", "landscape", ""
};


char const * const string_footnotekinds[] = {
	"footnote", "margin", "fig", "tab", "alg", "wide-fig", "wide-tab", ""
};


char const * const string_align[] = {
	"block", "left", "right", "center", ""
};


// The following was moved from tex-defs.h to here, because tex-defs.h is
// used all over. As it happens, that meant that these strings were included
// 27 times in the object file. (Asger)

char const * const tex_graphics[] = {"default", "dvips", "dvitops", "emtex",
		      "ln", "oztex", "textures", "none", ""};


char const * const tex_fonts[] = {"default", "ae", "pslatex", "times", "palatino",
			    "helvet", "avant", "newcent", "bookman", ""};
