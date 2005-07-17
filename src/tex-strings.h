// -*- C++ -*-
/**
 * \file tex-strings.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TEX_STRINGS
#define TEX_STRINGS

// this file should perhaps be split into even smaler parts

///
extern char const * const string_paragraph_separation[];
///
extern char const * const string_quotes_language[];
///
extern char const * const string_papersize[];
///
extern char const * const string_orientation[];
///
extern char const * const string_footnotekinds[];
///
extern char const * const string_align[];

// The following was moved from tex-defs.h to here, because tex-defs.h is
// used all over. As it happens, that meant that these strings were included
// 27 times in the object file. (Asger)

///
extern char const * const tex_graphics[];

///
extern char const * const tex_fonts[];

#endif
