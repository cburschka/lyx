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
#include "gettext.h"


namespace lyx {


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
		      "ln", "oztex", "textures", "none", ""
};


char const * const tex_fonts_roman[] = {"default", "cmr", "lmodern", "ae", "times", "palatino",
			    "charter", "newcent", "bookman", "utopia", "beraserif", "ccfonts", "chancery", ""
};


char const * tex_fonts_roman_gui[] = { N_("Default"), N_("Computer Modern Roman"), N_("Latin Modern Roman"),
			    N_("AE (Almost European)"), N_("Times Roman"), N_("Palatino"), N_("Bitstream Charter"),
			    N_("New Century Schoolbook"), N_("Bookman"), N_("Utopia"),  N_("Bera Serif"),
			    N_("Concrete Roman"), N_("Zapf Chancery"), ""
};


char const * const tex_fonts_sans[] = {"default", "cmss", "lmss", "helvet", "avant", "berasans", "cmbr", ""
};


char const * tex_fonts_sans_gui[] = { N_("Default"), N_("Computer Modern Sans"), N_("Latin Modern Sans"), 
			    N_("Helvetica"), N_("Avant Garde"), N_("Bera Sans"), N_("CM Bright"), ""
};


char const * const tex_fonts_monospaced[] = {"default", "cmtt", "lmtt", "courier", "beramono", 
			    "luximono", "cmtl", ""
};


char const * tex_fonts_monospaced_gui[] = { N_("Default"), N_("Computer Modern Typewriter"),
			    N_("Latin Modern Typewriter"), N_("Courier"), N_("Bera Mono"), N_("LuxiMono"),
			    N_("CM Typewriter Light"), ""
};


} // namespace lyx
