// -*- C++ -*-

#ifndef TEX_STRINGS
#define TEX_STRINGS

// this file should perhaps be split into even smaler parts

///
extern char const * string_paragraph_separation[];
///
extern char const * string_quotes_language[];
///
extern char const * string_papersize[];
///
extern char const * string_paperpackages[];
///
extern char const * string_orientation[];
///
extern char const * string_footnotekinds[];
///
extern char const * string_align[];

// The following was moved from tex-defs.h to here, because tex-defs.h is
// used all over. As it happens, that meant that these strings were included 
// 27 times in the object file. (Asger)

///
//extern char const * tex_babel[];

///
extern char const * tex_graphics[];

///
extern char const * tex_fonts[];

#endif
