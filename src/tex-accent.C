/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <cstdlib>
#include "tex-accent.h"
#include "commandtags.h"
#include "LString.h"
#include "support/lstrings.h"

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

//static
tex_accent_struct lyx_accent_table[18] = {
	{ static_cast<tex_accent>(0), "", "", "", static_cast<kb_action>(0)},
	{TEX_ACUTE,      "\\'",  " AEIOUYaeiouySZszRLCNrlcn", "acute",   LFUN_ACUTE},
	{TEX_GRAVE,      "\\`",  " aeiouAEIOU",           "grave",    LFUN_GRAVE},
	{TEX_MACRON,     "\\=",  " EeAIOUaiou",           "macron",    LFUN_MACRON},
	{TEX_TILDE,      "\\~",  " ANOanoIiUu",           "tilde",    LFUN_TILDE},
	{TEX_UNDERBAR,   "\\b", " ",                     "underbar", LFUN_UNDERBAR},
	{TEX_CEDILLA,    "\\c", " CcSsTtRLGrlgNKnk",     "cedilla",    LFUN_CEDILLA},
	{TEX_UNDERDOT,   "\\d", " ",                     "underdot", LFUN_UNDERDOT},
	{TEX_CIRCUMFLEX, "\\^",  " AEIOUaeiouHJhjCGScgs", "circumflex",  LFUN_CIRCUMFLEX},
	{TEX_CIRCLE,     "\\r", " AaUu",                 "circle",  LFUN_CIRCLE},
	{TEX_TIE,        "\\t", " ",                     "tie",    LFUN_TIE},
	{TEX_BREVE,      "\\u", " AaGgUu",               "breve",    LFUN_BREVE},
	{TEX_CARON,      "\\v", " LSTZlstzCEDNRcednr",   "caron",    LFUN_CARON},
//  {TEX_SPECIAL_CARON, "\\q", "", "ooo",  LFUN_SPECIAL_CARON},
	{TEX_HUNGUML,    "\\H", " OUou",                 "hugarian_umlaut",    LFUN_HUNG_UMLAUT},
	{TEX_UMLAUT,     "\\\"", " AEIOUaeiouy",          "umlaut",    LFUN_UMLAUT},
	{TEX_DOT,        "\\.",  " ZzICGicgEe",           "dot",    LFUN_DOT},
	{TEX_OGONEK,     "\\k",  " AaEe",                 "ogonek",    LFUN_OGONEK},
	{ static_cast<tex_accent>(0), "", "", "", static_cast<kb_action>(0)}};
        

tex_accent_struct get_accent(kb_action action)
{
	int i = 0;
	while (i <= TEX_MAX_ACCENT){
		if (lyx_accent_table[i].action == action)
			return lyx_accent_table[i];
		++i;
	}
	struct tex_accent_struct temp = { static_cast<tex_accent>(0), 0, 0,
					  0, static_cast<kb_action>(0)};
	return temp;
}

string DoAccent(string const & s, tex_accent accent)
{
	string res;
	
	res += lyx_accent_table[accent].cmd;
	res += '{';
	if (s == "i" || s == "j") {
		res += '\\';
	}
	res += s;
	res += '}';
	return res;
}

string DoAccent(char c, tex_accent accent)
{
	return DoAccent(tostr(c), accent);
}

