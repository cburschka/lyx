#include <config.h>
#include "tex-strings.h"

// this file should perhaps be split into even smaller parts

char const * string_paragraph_separation[3] = {
        "indent", "skip", ""
};


char const * string_quotes_language[7] = {
        "english", "swedish", "german", "polish", "french", "danish", ""
};      


char const * string_papersize[12] = {
	"Default", "Custom", "letterpaper", "executivepaper", "legalpaper", 
	"a3paper", "a4paper", "a5paper", "b3paper", "b4paper", "b5paper", ""
};


char const * string_paperpackages[4] = {
	"a4", "a4wide", "widemarginsa4", ""
};


char const * string_orientation[3] = {
	"portrait", "landscape", ""
};


char const * string_footnotekinds[8] = {
        "footnote", "margin", "fig", "tab", "alg", "wide-fig", "wide-tab", ""
};


char const * string_align[5] = {
        "block", "left", "right", "center", ""
};


// The following was moved from tex-defs.h to here, because tex-defs.h is
// used all over. As it happens, that meant that these strings were included 
// 27 times in the object file. (Asger)

///
char const * tex_babel[] = {"default", "afrikaans", "american",
			   "austrian", "bahasa", "brazil", "breton",
			   "catalan", "croatian", "czech", "danish", "dutch", 
			   "english", "esperanto", "estonian",
			   "finnish", "francais", "french", "frenchb",
			   "galician",
			   "german", "greek", "hebrew", "hungarian", "irish",
			   "italian", "lsorbian", "magyar", "norsk",
			   "polish", "portuges", "romanian",
			   "russian", "scottish",
			   "spanish", "slovak", "slovene", "swedish",
			   "turkish", "usorbian", "welsh",
			   ""};


char const * tex_graphics[] = {"default", "dvips", "dvitops", "emtex",
		      "ln", "oztex", "textures", "none", ""};


char const * tex_fonts[] = {"default", "times", "palatino", "helvet", "avant",
		     "newcent", "bookman", ""};


