/**
 * \file tex-accent.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "tex-accent.h"
#include "support/convert.h"


namespace lyx {

/* the names used by TeX and XWindows for deadkeys/accents are not the same
   so here follows a table to clearify the differences. Please correct this
   if I got it wrong

   |------------------|------------------|------------------|--------------|
   |      TeX         |     XWindows     |   \bind/LFUN     | used by intl |
   |------------------|------------------|------------------|--------------|
   |    grave         |    grave         |LFUN_ACCENT_GRAVE        | grave
   |    acute         |    acute         |LFUN_ACCENT_ACUTE        | acute
   |    circumflex    |    circumflex    |LFUN_ACCENT_CIRCUMFLEX   | circumflex
   | umlaut/dieresis  |    diaeresis     |LFUN_ACCENT_UMLAUT       | umlaut
   |    tilde         |    tilde         |LFUN_ACCENT_TILDE        | tilde
   |    macron        |    maron         |LFUN_ACCENT_MACRON       | macron
   |    dot           |    abovedot      |LFUN_ACCENT_DOT          | dot
   |    cedilla       |    cedilla       |LFUN_ACCENT_CEDILLA      | cedilla
   |    underdot      |                  |LFUN_ACCENT_UNDERDOT     | underdot
   |    underbar      |                  |LFUN_ACCENT_UNDERBAR     | underbar
   |    hácek         |    caron         |LFUN_ACCENT_CARON        | caron
   |    breve         |    breve         |LFUN_ACCENT_BREVE        | breve
   |    tie           |                  |LFUN_ACCENT_TIE          | tie
   | Hungarian umlaut |    doubleacute   |LFUN_ACCENT_HUNGARIAN_UMLAUT  | hungarian umlaut
   |    circle        |    abovering     |LFUN_ACCENT_CIRCLE       | circle
   |                  |    ogonek        |                  |
   |                  |    iota          |                  |
   |                  |    voiced_sound  |                  |
   |                  | semivoiced_sound |                  |
   |                  |                  |LFUN_ACCENT_SPECIAL_CARON| special caron
   */
// I am not sure how some of the XWindows names coresponds to the TeX ones.

tex_accent_struct lyx_accent_table[18] = {
	{ static_cast<tex_accent>(0), "", /*"",*/ "", static_cast<kb_action>(0)},
	{TEX_ACUTE,      "\\'",  /*" AEIOUYaeiouySZszRLCNrlcn",*/ "acute",   LFUN_ACCENT_ACUTE},
	{TEX_GRAVE,      "\\`",  /*" aeiouAEIOU",*/           "grave",    LFUN_ACCENT_GRAVE},
	{TEX_MACRON,     "\\=",  /*" EeAIOUaiou",*/           "macron",    LFUN_ACCENT_MACRON},
	{TEX_TILDE,      "\\~",  /*" ANOanoIiUu",*/           "tilde",    LFUN_ACCENT_TILDE},
	{TEX_UNDERBAR,   "\\b", /*" ",*/                     "underbar", LFUN_ACCENT_UNDERBAR},
	{TEX_CEDILLA,    "\\c", /*" CcSsTtRLGrlgNKnk",*/     "cedilla",    LFUN_ACCENT_CEDILLA},
	{TEX_UNDERDOT,   "\\d", /*" ",*/                     "underdot", LFUN_ACCENT_UNDERDOT},
	{TEX_CIRCUMFLEX, "\\^",  /*" AEIOUaeiouHJhjCGScgs",*/ "circumflex",  LFUN_ACCENT_CIRCUMFLEX},
	{TEX_CIRCLE,     "\\r", /*" AaUu",*/                 "circle",  LFUN_ACCENT_CIRCLE},
	{TEX_TIE,        "\\t", /*" ",*/                     "tie",    LFUN_ACCENT_TIE},
	{TEX_BREVE,      "\\u", /*" AaGgUu",*/               "breve",    LFUN_ACCENT_BREVE},
	{TEX_CARON,      "\\v", /*" LSTZlstzCEDNRcednr",*/   "caron",    LFUN_ACCENT_CARON},
//  {TEX_SPECIAL_CARON, "\\q", "", "ooo",  LFUN_ACCENT_SPECIAL_CARON},
	{TEX_HUNGUML,    "\\H", /*" OUou",*/                 "hugarian_umlaut",    LFUN_ACCENT_HUNGARIAN_UMLAUT},
	{TEX_UMLAUT,     "\\\"", /*" AEIOUaeiouy",*/          "umlaut",    LFUN_ACCENT_UMLAUT},
	{TEX_DOT,        "\\.",  /*" ZzICGicgEe",*/           "dot",    LFUN_ACCENT_DOT},
	{TEX_OGONEK,     "\\k",  /*" AaEe",*/                 "ogonek",    LFUN_ACCENT_OGONEK},
	{ static_cast<tex_accent>(0), "", /*"",*/ "", static_cast<kb_action>(0)}};


tex_accent_struct get_accent(kb_action action)
{
	int i = 0;
	while (i <= TEX_MAX_ACCENT) {
		if (lyx_accent_table[i].action == action)
			return lyx_accent_table[i];
		++i;
	}
	struct tex_accent_struct temp = { static_cast<tex_accent>(0), 0,
					  0, static_cast<kb_action>(0)};
	return temp;
}


docstring const DoAccent(docstring const & s, tex_accent accent)
{
	docstring res;

	res += lyx_accent_table[accent].cmd;
	res += '{';
	if (s == "i" || s == "j") {
		res += '\\';
	}
	res += s;
	res += '}';
	return res;
}


docstring const DoAccent(char_type c, tex_accent accent)
{
	return DoAccent(docstring(1, c), accent);
}


} // namespace lyx
