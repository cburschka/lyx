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
#include "debug.h"

#include "support/convert.h"
#include "support/docstream.h"


namespace lyx {

// FIXME This file has nothing to do with TeX anymore

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

tex_accent_struct lyx_accent_table[] = {
	{TEX_NOACCENT,   0,      "",                LFUN_NOACTION},
	{TEX_ACUTE,      0x0301, "acute",           LFUN_ACCENT_ACUTE},
	{TEX_GRAVE,      0x0300, "grave",           LFUN_ACCENT_GRAVE},
	{TEX_MACRON,     0x0304, "macron",          LFUN_ACCENT_MACRON},
	{TEX_TILDE,      0x0303, "tilde",           LFUN_ACCENT_TILDE},
	{TEX_UNDERBAR,   0x0320, "underbar",        LFUN_ACCENT_UNDERBAR},
	{TEX_CEDILLA,    0x0327, "cedilla",         LFUN_ACCENT_CEDILLA},
	{TEX_UNDERDOT,   0x0323, "underdot",        LFUN_ACCENT_UNDERDOT},
	{TEX_CIRCUMFLEX, 0x0302, "circumflex",      LFUN_ACCENT_CIRCUMFLEX},
	{TEX_CIRCLE,     0x030a, "circle",          LFUN_ACCENT_CIRCLE},
	{TEX_TIE,        0x0361, "tie",             LFUN_ACCENT_TIE},
	{TEX_BREVE,      0x0306, "breve",           LFUN_ACCENT_BREVE},
	{TEX_CARON,      0x030c, "caron",           LFUN_ACCENT_CARON},
//	{TEX_SPECIAL_CARON, 0x030c, "ooo",          LFUN_ACCENT_SPECIAL_CARON},
	// Don't fix this typo for compatibility reasons!
	{TEX_HUNGUML,    0x030b, "hugarian_umlaut", LFUN_ACCENT_HUNGARIAN_UMLAUT},
	{TEX_UMLAUT,     0x0308, "umlaut",          LFUN_ACCENT_UMLAUT},
	{TEX_DOT,        0x0307, "dot",             LFUN_ACCENT_DOT},
	{TEX_OGONEK,     0x0328, "ogonek",          LFUN_ACCENT_OGONEK}
};


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
	if (s.empty())
		return docstring(1, lyx_accent_table[accent].ucs4);

	odocstringstream os;
	os.put(s[0]);
	os.put(lyx_accent_table[accent].ucs4);
	if (s.length() > 1) {
		if (accent != TEX_TIE || s.length() > 2)
			lyxerr << "Warning: Too many characters given for accent "
			       << lyx_accent_table[accent].name << '.' << std::endl;
		os << s.substr(1);
	}
	// FIXME: We should normalize the result to precomposed form
	return os.str();
}


docstring const DoAccent(char_type c, tex_accent accent)
{
	return DoAccent(docstring(1, c), accent);
}


} // namespace lyx
