// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef TEX_ACCENT
#define TEX_ACCENT

#ifdef __GNUG__
#pragma interface
#endif

#include "commandtags.h"

/* the names used by TeX and XWindows for deadkeys/accents are not the same
   so here follows a table to clearify the differences. Please correct this
   if I got it wrong

   |------------------|------------------|------------------|--------------|
   |      TeX         |     XWindows     |   \bind/LFUN     | used by intl |
   |------------------|------------------|------------------|--------------|
   |    grave         |    grave         |LFUN_GRAVE        | grave
   |    acute         |    acute         |LFUN_ACUTE        | acute
   |    circumflex    |    circumflex    |LFUN_CIRCUMFLEX   | circumflex
   | umlaut/dieresis  |    diaeresis     |LFUN_UMLAUT       | umlaut
   |    tilde         |    tilde         |LFUN_TILDE        | tilde
   |    macron        |    maron         |LFUN_MACRON       | macron
   |    dot           |    abovedot      |LFUN_DOT          | dot
   |    cedilla       |    cedilla       |LFUN_CEDILLA      | cedilla
   |    underdot      |                  |LFUN_UNDERDOT     | underdot
   |    underbar      |                  |LFUN_UNDERBAR     | underbar
   |    hácek         |    caron         |LFUN_CARON        | caron
   |    breve         |    breve         |LFUN_BREVE        | breve
   |    tie           |                  |LFUN_TIE          | tie
   | Hungarian umlaut |    doubleacute   |LFUN_HUNG_UMLAUT  | hungarian umlaut
   |    circle        |    abovering     |LFUN_CIRCLE       | circle
   |                  |    ogonek        |                  |
   |                  |    iota          |                  |
   |                  |    voiced_sound  |                  |
   |                  | semivoiced_sound |                  |
   |                  |                  |LFUN_SPECIAL_CARON| special caron
   */
// I am not sure how some of the XWindows names coresponds to the TeX ones.

///
enum tex_accent {
	///
	TEX_NOACCENT = 0,
	///
	TEX_ACUTE,
	///
	TEX_GRAVE,
	///
	TEX_MACRON,
	///
	TEX_TILDE,
	///
	TEX_UNDERBAR,
	///
	TEX_CEDILLA,
	///
	TEX_UNDERDOT,
	///
	TEX_CIRCUMFLEX,
	///
	TEX_CIRCLE,
	///
	TEX_TIE,
	///
	TEX_BREVE,
	///
	TEX_CARON,
//  TEX_SPECIAL_CARON,
	///
	TEX_HUNGUML,
	///
	TEX_UMLAUT,
	///
	TEX_DOT,
	///
	TEX_OGONEK,
	///
	TEX_MAX_ACCENT= TEX_OGONEK
};


/** the native is so we know what characters TeX can write in a decent way
 it should be up to the user if _all_ characters should be allowed to
 be accented. Lgb. */
struct tex_accent_struct {
	///
	tex_accent accent;
	///
	char const * cmd;
	///
	char const * native;
	///
	char const * name;
	///
	kb_action action;
};

///
extern tex_accent_struct lyx_accent_table[];
        
#endif
