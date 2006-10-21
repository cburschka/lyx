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

namespace lyx {

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
extern char const * const tex_fonts_roman[];
///
extern char const * tex_fonts_roman_gui[];
///
extern char const * const tex_fonts_sans[];
///
extern char const * tex_fonts_sans_gui[];
///
extern char const * const tex_fonts_monospaced[];
///
extern char const * tex_fonts_monospaced_gui[];


} // namespace lyx

#endif
