/**
 * \file InsetERT.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetERT.h"

#include "Cursor.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "InsetLayout.h"
#include "Language.h"
#include "Lexer.h"
#include "xml.h"
#include "ParagraphParameters.h"
#include "Paragraph.h"
#include "output_docbook.h"

#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/TempFile.h"

#include <sstream>
#include <regex>
#include <iostream>

using namespace std;
using namespace lyx::support;

namespace lyx {

InsetERT::InsetERT(Buffer * buf, CollapseStatus status)
	: InsetCollapsible(buf)
{
	status_ = status;
}


InsetERT::InsetERT(InsetERT const & old)
	: InsetCollapsible(old)
{}


void InsetERT::write(ostream & os) const
{
	os << "ERT" << "\n";
	InsetCollapsible::write(os);
}


int InsetERT::plaintext(odocstringstream & os,
        OutputParams const & rp, size_t max_length) const
{
	if (!rp.inIndexEntry)
		// do not output TeX code
		return 0;

	ParagraphList::const_iterator par = paragraphs().begin();
	ParagraphList::const_iterator end = paragraphs().end();

	while (par != end && os.str().size() <= max_length) {
		pos_type siz = par->size();
		for (pos_type i = 0; i < siz; ++i) {
			char_type const c = par->getChar(i);
			// output the active characters
			switch (c) {
			case '|':
			case '!':
			case '@':
				os.put(c);
				break;
			default:
				break;
			}
		}
		++par;
	}
	return 0;
}


static const std::map<docstring, docstring> raw_latex_encoding_to_unicode_xml{
		// Punctuation.
		{from_ascii("\\textquotesingle"), from_ascii("'")},
		{from_ascii("!`"), from_ascii("&#161;")}, // inverted exclamation mark
		{from_ascii("?`"), from_ascii("&#191;")}, // inverted interrogation mark

		// Logos.
		{from_ascii("\\LaTeX"), from_ascii("LaTeX")},
		{from_ascii("\\LaTeXe"), from_ascii("LaTeX2&#x03b5;")}, // LaTeX 2 epsilon
		{from_ascii("\\LyX"), from_ascii("LyX")},
		{from_ascii("\\TeX"), from_ascii("TeX")},

		// Accentuated letters (alphabetical order of description, ligatures after accents).
		// Missing letters are only indicated for vowels (only few consonants have diacritics).
		// Only symbols that can be expressed as one Unicode characters are present; symbols with more than one
		// diacritic are also omitted.
		{from_ascii("\\`{a}"), from_ascii("&#225;")}, // a acute
		{from_ascii("\\` a"), from_ascii("&#225;")},
		{from_ascii("\\`{A}"), from_ascii("&#193;")}, // A acute
		{from_ascii("\\` A"), from_ascii("&#193;")},
		{from_ascii("\\u{a}"), from_ascii("&#259;")}, // a breve
		{from_ascii("\\u a"), from_ascii("&#259;")},
		{from_ascii("\\u{A}"), from_ascii("&#258;")}, // A breve
		{from_ascii("\\u A"), from_ascii("&#258;")},
		{from_ascii("\\v{a}"), from_ascii("&#462;")}, // a caron
		{from_ascii("\\v a"), from_ascii("&#462;")},
		{from_ascii("\\v{A}"), from_ascii("&#461;")}, // A caron
		{from_ascii("\\v A"), from_ascii("&#461;")},
		// No a cedilla.
		{from_ascii("\\^{a}"), from_ascii("&#226;")}, // a circumflex
		{from_ascii("\\^ a"), from_ascii("&#226;")},
		{from_ascii("\\^{A}"), from_ascii("&#194;")}, // A circumflex
		{from_ascii("\\^ A"), from_ascii("&#194;")},
		{from_ascii("\\\"{a}"), from_ascii("&#228;")}, // a diaeresis
		{from_ascii("\\\" a"), from_ascii("&#228;")},
		{from_ascii("\\\"{A}"), from_ascii("&#196;")}, // A diaeresis
		{from_ascii("\\\" A"), from_ascii("&#196;")},
		// No a double acute.
		{from_ascii("\\`{a}"), from_ascii("&#224;")}, // a grave
		{from_ascii("\\` a"), from_ascii("&#224;")},
		{from_ascii("\\`{A}"), from_ascii("&#192;")}, // A grave
		{from_ascii("\\` A"), from_ascii("&#192;")},
		{from_ascii("\\~{a}"), from_ascii("&#227;")}, // a tilde
		{from_ascii("\\~ a"), from_ascii("&#227;")},
		{from_ascii("\\~{A}"), from_ascii("&#195;")}, // A tilde
		{from_ascii("\\~ A"), from_ascii("&#195;")},
		{from_ascii("\\aa"), from_ascii("&#229;")}, // a ring
		{from_ascii("\\r{a}"), from_ascii("&#229;")},
		{from_ascii("\\r a"), from_ascii("&#229;")},
		{from_ascii("\\AA"), from_ascii("&#197;")}, // A ring
		{from_ascii("\\r{A}"), from_ascii("&#197;")},
		{from_ascii("\\r A"), from_ascii("&#197;")},
		{from_ascii("\\ae"), from_ascii("&#230;")}, // ae ligature
		{from_ascii("\\AE"), from_ascii("&#198;")}, // AE ligature
		{from_ascii("\\v{c}"), from_ascii("&#269;")}, // c caron
		{from_ascii("\\v c"), from_ascii("&#269;")},
		{from_ascii("\\v{C}"), from_ascii("&#268;")}, // C caron
		{from_ascii("\\v C"), from_ascii("&#268;")},
		{from_ascii("\\c{c}"), from_ascii("&#199;")}, // c cedilla
		{from_ascii("\\c c"), from_ascii("&#199;")},
		{from_ascii("\\c{C}"), from_ascii("&#231;")}, // C cedilla
		{from_ascii("\\c C"), from_ascii("&#231;")},
		{from_ascii("\\v{d}"), from_ascii("&#271;")}, // d caron
		{from_ascii("\\v d"), from_ascii("&#271;")},
		{from_ascii("\\v{D}"), from_ascii("&#270;")}, // D caron
		{from_ascii("\\v D"), from_ascii("&#270;")},
		{from_ascii("\\`{e}"), from_ascii("&#233;")}, // e acute
		{from_ascii("\\` e"), from_ascii("&#233;")},
		{from_ascii("\\`{E}"), from_ascii("&#201;")}, // E acute
		{from_ascii("\\` E"), from_ascii("&#201;")},
		{from_ascii("\\u{e}"), from_ascii("&#277;")}, // e breve
		{from_ascii("\\u e"), from_ascii("&#277;")},
		{from_ascii("\\u{E}"), from_ascii("&#276;")}, // E breve
		{from_ascii("\\u E"), from_ascii("&#276;")},
		{from_ascii("\\v{e}"), from_ascii("&#283;")}, // e caron
		{from_ascii("\\v e"), from_ascii("&#283;")},
		{from_ascii("\\v{E}"), from_ascii("&#282;")}, // E caron
		{from_ascii("\\v E"), from_ascii("&#282;")},
		{from_ascii("\\c{e}"), from_ascii("&#553;")}, // e cedilla
		{from_ascii("\\c e"), from_ascii("&#553;")},
		{from_ascii("\\c{E}"), from_ascii("&#552;")}, // E cedilla
		{from_ascii("\\c E"), from_ascii("&#552;")},
		{from_ascii("\\^{e}"), from_ascii("&#234;")}, // e circumflex
		{from_ascii("\\^ e"), from_ascii("&#234;")},
		{from_ascii("\\^{E}"), from_ascii("&#202;")}, // E circumflex
		{from_ascii("\\^ E"), from_ascii("&#202;")},
		{from_ascii("\\\"{e}"), from_ascii("&#235;")}, // e diaeresis
		{from_ascii("\\\" e"), from_ascii("&#235;")},
		{from_ascii("\\\"{E}"), from_ascii("&#203;")}, // E diaeresis
		{from_ascii("\\\" E"), from_ascii("&#203;")},
		// No e double acute.
		{from_ascii("\\`{e}"), from_ascii("&#232;")}, // e grave
		{from_ascii("\\` e"), from_ascii("&#232;")},
		{from_ascii("\\`{E}"), from_ascii("&#200;")}, // E grave
		{from_ascii("\\` E"), from_ascii("&#200;")},
		{from_ascii("\\~{e}"), from_ascii("&#7869;")}, // e tilde
		{from_ascii("\\~ e"), from_ascii("&#7869;")},
		{from_ascii("\\~{E}"), from_ascii("&#7868;")}, // E tilde
		{from_ascii("\\~ E"), from_ascii("&#7868;")},
		// No e ring.
		{from_ascii("\\u{g}"), from_ascii("&#287;")}, // g breve
		{from_ascii("\\u g"), from_ascii("&#287;")},
		{from_ascii("\\u{G}"), from_ascii("&#286;")}, // G breve
		{from_ascii("\\u G"), from_ascii("&#286;")},
		{from_ascii("\\v{g}"), from_ascii("&#487;")}, // g caron
		{from_ascii("\\v g"), from_ascii("&#487;")},
		{from_ascii("\\v{G}"), from_ascii("&#486;")}, // G caron
		{from_ascii("\\v G"), from_ascii("&#486;")},
		{from_ascii("\\c{g}"), from_ascii("&#291;")}, // g cedilla
		{from_ascii("\\c g"), from_ascii("&#291;")},
		{from_ascii("\\c{G}"), from_ascii("&#290;")}, // G cedilla
		{from_ascii("\\c G"), from_ascii("&#290;")},
		{from_ascii("\\i"), from_ascii("&#305;")}, // i dotless
		{from_ascii("\\`{i}"), from_ascii("&#237;")}, // i acute
		{from_ascii("\\` i"), from_ascii("&#237;")},
		{from_ascii("\\`{I}"), from_ascii("&#205;")}, // I acute
		{from_ascii("\\` I"), from_ascii("&#205;")},
		{from_ascii("\\u{i}"), from_ascii("&#301;")}, // i breve
		{from_ascii("\\u i"), from_ascii("&#301;")},
		{from_ascii("\\u{I}"), from_ascii("&#300;")}, // I breve
		{from_ascii("\\u I"), from_ascii("&#300;")},
		{from_ascii("\\v{i}"), from_ascii("&#464;")}, // i caron
		{from_ascii("\\v i"), from_ascii("&#464;")},
		{from_ascii("\\v{I}"), from_ascii("&#463;")}, // I caron
		{from_ascii("\\v I"), from_ascii("&#463;")},
		// No i cedilla.
		{from_ascii("\\^{i}"), from_ascii("&#238;")}, // i circumflex
		{from_ascii("\\^ i"), from_ascii("&#238;")},
		{from_ascii("\\^{I}"), from_ascii("&#206;")}, // I circumflex
		{from_ascii("\\^ I"), from_ascii("&#239;")},
		{from_ascii("\\\"{i}"), from_ascii("&#239;")}, // i diaeresis
		{from_ascii("\\\" i"), from_ascii("&#235;")},
		{from_ascii("\\\"{I}"), from_ascii("&#235;")}, // I diaeresis
		{from_ascii("\\\" I"), from_ascii("&#207;")},
		// No i double acute.
		{from_ascii("\\`{i}"), from_ascii("&#236;")}, // i grave
		{from_ascii("\\` i"), from_ascii("&#236;")},
		{from_ascii("\\`{I}"), from_ascii("&#204;")}, // I grave
		{from_ascii("\\` I"), from_ascii("&#204;")},
		{from_ascii("\\~{i}"), from_ascii("&#297;")}, // i tilde
		{from_ascii("\\~ i"), from_ascii("&#297;")},
		{from_ascii("\\~{I}"), from_ascii("&#296;")}, // I tilde
		{from_ascii("\\~ I"), from_ascii("&#296;")},
		// No i ring.
		{from_ascii("\\j"), from_ascii("&#567;")}, // j dotless
		{from_ascii("\\v{k}"), from_ascii("&#488;")}, // k caron
		{from_ascii("\\v k"), from_ascii("&#488;")},
		{from_ascii("\\v{K}"), from_ascii("&#489;")}, // K caron
		{from_ascii("\\v K"), from_ascii("&#489;")},
		{from_ascii("\\c{k}"), from_ascii("&#311;")}, // k cedilla
		{from_ascii("\\c k"), from_ascii("&#311;")},
		{from_ascii("\\c{K}"), from_ascii("&#310;")}, // K cedilla
		{from_ascii("\\c K"), from_ascii("&#310;")},
		{from_ascii("\\v{l}"), from_ascii("&#318;")}, // l caron
		{from_ascii("\\v l"), from_ascii("&#318;")},
		{from_ascii("\\v{L}"), from_ascii("&#317;")}, // L caron
		{from_ascii("\\v L"), from_ascii("&#317;")},
		{from_ascii("\\c{l}"), from_ascii("&#316;")}, // l cedilla
		{from_ascii("\\c l"), from_ascii("&#316;")},
		{from_ascii("\\c{L}"), from_ascii("&#315;")}, // L cedilla
		{from_ascii("\\c L"), from_ascii("&#315;")},
		{from_ascii("\\l"), from_ascii("&#322;")}, // l stroke
		{from_ascii("\\L"), from_ascii("&#321;")}, // L stroke
		{from_ascii("\\v{n}"), from_ascii("&#328;")}, // n caron
		{from_ascii("\\v n"), from_ascii("&#328;")},
		{from_ascii("\\v{N}"), from_ascii("&#327;")}, // N caron
		{from_ascii("\\v N"), from_ascii("&#327;")},
		{from_ascii("\\c{n}"), from_ascii("&#326;")}, // n cedilla
		{from_ascii("\\c n"), from_ascii("&#326;")},
		{from_ascii("\\c{N}"), from_ascii("&#325;")}, // N cedilla
		{from_ascii("\\c N"), from_ascii("&#325;")},
		{from_ascii("\\~{n}"), from_ascii("&#241;")}, // n tilde
		{from_ascii("\\~ n"), from_ascii("&#241;")},
		{from_ascii("\\~{N}"), from_ascii("&#209;")}, // N tilde
		{from_ascii("\\~ N"), from_ascii("&#209;")},
		{from_ascii("\\`{o}"), from_ascii("&#243;")}, // o acute
		{from_ascii("\\` o"), from_ascii("&#243;")},
		{from_ascii("\\`{O}"), from_ascii("&#211;")}, // O acute
		{from_ascii("\\` O"), from_ascii("&#211;")},
		{from_ascii("\\u{o}"), from_ascii("&#335;")}, // o breve
		{from_ascii("\\u o"), from_ascii("&#335;")},
		{from_ascii("\\u{O}"), from_ascii("&#334;")}, // O breve
		{from_ascii("\\u O"), from_ascii("&#334;")},
		{from_ascii("\\v{o}"), from_ascii("&#466;")}, // o caron
		{from_ascii("\\v o"), from_ascii("&#466;")},
		{from_ascii("\\v{O}"), from_ascii("&#465;")}, // O caron
		{from_ascii("\\v O"), from_ascii("&#465;")},
		// No o cedilla.
		{from_ascii("\\^{o}"), from_ascii("&#244;")}, // o circumflex
		{from_ascii("\\^ o"), from_ascii("&#244;")},
		{from_ascii("\\^{O}"), from_ascii("&#212;")}, // O circumflex
		{from_ascii("\\^ O"), from_ascii("&#212;")},
		{from_ascii("\\\"{o}"), from_ascii("&#246;")}, // o diaeresis
		{from_ascii("\\\" o"), from_ascii("&#246;")},
		{from_ascii("\\\"{O}"), from_ascii("&#214;")}, // O diaeresis
		{from_ascii("\\\" O"), from_ascii("&#214;")},
		{from_ascii("\\H{o}"), from_ascii("&#337;")}, // o double acute
		{from_ascii("\\H o"), from_ascii("&#337;")},
		{from_ascii("\\H{O}"), from_ascii("&#336;")}, // O double acute
		{from_ascii("\\H O"), from_ascii("&#336;")},
		{from_ascii("\\`{o}"), from_ascii("&#242;")}, // o grave
		{from_ascii("\\` o"), from_ascii("&#242;")},
		{from_ascii("\\`{O}"), from_ascii("&#210;")}, // O grave
		{from_ascii("\\` O"), from_ascii("&#210;")},
		{from_ascii("\\o"), from_ascii("&#248;")}, // o stroke
		{from_ascii("\\O"), from_ascii("&#216;")}, // O stroke
		{from_ascii("\\~{o}"), from_ascii("&#245;")}, // o tilde
		{from_ascii("\\~ o"), from_ascii("&#245;")},
		{from_ascii("\\~{O}"), from_ascii("&#213;")}, // O tilde
		{from_ascii("\\~ O"), from_ascii("&#213;")},
		// No o ring.
		{from_ascii("\\oe"), from_ascii("&#339;")}, // oe ligature
		{from_ascii("\\OE"), from_ascii("&#338;")}, // OE ligature
		{from_ascii("\\v{r}"), from_ascii("&#345;")}, // r caron
		{from_ascii("\\v r"), from_ascii("&#345;")},
		{from_ascii("\\v{R}"), from_ascii("&#344;")}, // R caron
		{from_ascii("\\v R"), from_ascii("&#344;")},
		{from_ascii("\\c{r}"), from_ascii("&#343;")}, // r cedilla
		{from_ascii("\\c r"), from_ascii("&#343;")},
		{from_ascii("\\c{R}"), from_ascii("&#342;")}, // R cedilla
		{from_ascii("\\c R"), from_ascii("&#342;")},
		{from_ascii("\\v{s}"), from_ascii("&#353;")}, // s caron
		{from_ascii("\\v s"), from_ascii("&#353;")},
		{from_ascii("\\v{S}"), from_ascii("&#352;")}, // S caron
		{from_ascii("\\v S"), from_ascii("&#352;")},
		{from_ascii("\\c{s}"), from_ascii("&#351;")}, // s cedilla
		{from_ascii("\\c s"), from_ascii("&#351;")},
		{from_ascii("\\c{S}"), from_ascii("&#350;")}, // S cedilla
		{from_ascii("\\c S"), from_ascii("&#350;")},
		{from_ascii("\\v{t}"), from_ascii("&#357;")}, // t caron
		{from_ascii("\\v t"), from_ascii("&#357;")},
		{from_ascii("\\v{T}"), from_ascii("&#356;")}, // T caron
		{from_ascii("\\v T"), from_ascii("&#356;")},
		{from_ascii("\\c{t}"), from_ascii("&#355;")}, // t cedilla
		{from_ascii("\\c t"), from_ascii("&#355;")},
		{from_ascii("\\c{T}"), from_ascii("&#354;")}, // T cedilla
		{from_ascii("\\c T"), from_ascii("&#354;")},
		{from_ascii("\\`{u}"), from_ascii("&#250;")}, // u acute
		{from_ascii("\\` u"), from_ascii("&#250;")},
		{from_ascii("\\`{U}"), from_ascii("&#218;")}, // U acute
		{from_ascii("\\` U"), from_ascii("&#218;")},
		{from_ascii("\\u{u}"), from_ascii("&#365;")}, // u breve
		{from_ascii("\\u u"), from_ascii("&#365;")},
		{from_ascii("\\u{U}"), from_ascii("&#364;")}, // U breve
		{from_ascii("\\u U"), from_ascii("&#364;")},
		{from_ascii("\\v{u}"), from_ascii("&#468;")}, // u caron
		{from_ascii("\\v u"), from_ascii("&#468;")},
		{from_ascii("\\v{U}"), from_ascii("&#467;")}, // U caron
		{from_ascii("\\v U"), from_ascii("&#467;")},
		// No u cedilla.
		{from_ascii("\\^{u}"), from_ascii("&#251;")}, // u circumflex
		{from_ascii("\\^ u"), from_ascii("&#251;")},
		{from_ascii("\\^{U}"), from_ascii("&#219;")}, // U circumflex
		{from_ascii("\\^ U"), from_ascii("&#219;")},
		{from_ascii("\\\"{u}"), from_ascii("&#252;")}, // u diaeresis
		{from_ascii("\\\" u"), from_ascii("&#252;")},
		{from_ascii("\\\"{U}"), from_ascii("&#220;")}, // U diaeresis
		{from_ascii("\\\" U"), from_ascii("&#220;")},
		{from_ascii("\\H{u}"), from_ascii("&#369;")}, // u double acute
		{from_ascii("\\H u"), from_ascii("&#369;")},
		{from_ascii("\\H{U}"), from_ascii("&#368;")}, // U double acute
		{from_ascii("\\H U"), from_ascii("&#368;")},
		{from_ascii("\\`{u}"), from_ascii("&#249;")}, // u grave
		{from_ascii("\\` u"), from_ascii("&#249;")},
		{from_ascii("\\`{U}"), from_ascii("&#217;")}, // U grave
		{from_ascii("\\` U"), from_ascii("&#217;")},
		{from_ascii("\\~{u}"), from_ascii("&#361;")}, // u tilde
		{from_ascii("\\~ u"), from_ascii("&#361;")},
		{from_ascii("\\~{U}"), from_ascii("&#360;")}, // U tilde
		{from_ascii("\\~ U"), from_ascii("&#360;")},
		{from_ascii("\\r{u}"), from_ascii("&#229;")}, // u ring
		{from_ascii("\\r u"), from_ascii("&#367;")},
		{from_ascii("\\r{U}"), from_ascii("&#367;")}, // U ring
		{from_ascii("\\r U"), from_ascii("&#366;")},
		{from_ascii("\\`{y}"), from_ascii("&#253;")}, // y acute
		{from_ascii("\\` y"), from_ascii("&#253;")},
		{from_ascii("\\`{Y}"), from_ascii("&#221;")}, // Y acute
		{from_ascii("\\` Y"), from_ascii("&#221;")},
		{from_ascii("\\v{z}"), from_ascii("&#382;")}, // z caron
		{from_ascii("\\v z"), from_ascii("&#382;")},
		{from_ascii("\\v{Z}"), from_ascii("&#381;")}, // Z caron
		{from_ascii("\\v Z"), from_ascii("&#381;")},
		// No y breve.
		// No y cedilla.
		{from_ascii("\\^{y}"), from_ascii("&#375;")}, // y circumflex
		{from_ascii("\\^ y"), from_ascii("&#375;")},
		{from_ascii("\\^{Y}"), from_ascii("&#374;")}, // Y circumflex
		{from_ascii("\\^ Y"), from_ascii("&#374;")},
		{from_ascii("\\\"{y}"), from_ascii("&#255;")}, // y diaeresis
		{from_ascii("\\\" y"), from_ascii("&#255;")},
		{from_ascii("\\\"{Y}"), from_ascii("&#376;")}, // Y diaeresis
		{from_ascii("\\\" Y"), from_ascii("&#376;")},
		// No y double acute.
		{from_ascii("\\`{y}"), from_ascii("&#7923;")}, // y grave
		{from_ascii("\\` y"), from_ascii("&#7923;")},
		{from_ascii("\\`{Y}"), from_ascii("&#7922;")}, // Y grave
		{from_ascii("\\` Y"), from_ascii("&#7922;")},
		{from_ascii("\\~{y}"), from_ascii("&#7929;")}, // y tilde
		{from_ascii("\\~ y"), from_ascii("&#7929;")},
		{from_ascii("\\~{Y}"), from_ascii("&#7928;")}, // Y tilde
		{from_ascii("\\~ Y"), from_ascii("&#7928;")},
		// No y ring.
};


void InsetERT::docbook(XMLStream & xs, OutputParams const & runparams) const
{
	auto const begin = paragraphs().begin();
	auto par = begin;
	auto const end = paragraphs().end();

	odocstringstream os; // No need for XML handling here.

	// Recreate the logic of makeParagraph in output_docbook.cpp, but much simplified: never open <para>
	// in an ERT, use simple line breaks.
	// New line after each paragraph of the ERT, save the last one.
	while (true) { // For each paragraph in the ERT...
        std::vector<docstring> pars_prepend;
        std::vector<docstring> pars;
        std::vector<docstring> pars_append;
        tie(pars_prepend, pars, pars_append) = par->simpleDocBookOnePar(buffer(), runparams, text().outerFont(distance(begin, par)), 0, false, true);

        for (docstring const & parXML : pars_prepend)
            xs << XMLStream::ESCAPE_NONE << parXML;
		auto p = pars.begin();
		while (true) { // For each line of this ERT paragraph...
			os << *p;
			++p;
			if (p != pars.end())
				os << "\n";
			else
				break;
		}
        for (docstring const & parXML : pars_append)
            xs << XMLStream::ESCAPE_NONE << parXML;

		++par;
		if (par != end)
			os << "\n";
		else
			break;
	}

//	// Implement the special case of \and: split the current item.
//	if (os.str() == "\\and" || os.str() == "\\and ") {
//		auto lay = getLayout();
//	}

	// Try to recognise some commands to have a nicer DocBook output.
	bool output_as_comment = true;

	// First step: some commands have a direct mapping to DocBook, mostly because the mapping is simply text or
	// an XML entity.
	{
		docstring os_trimmed = trim(os.str());

		auto command_raw_translation = raw_latex_encoding_to_unicode_xml.find(os_trimmed);
		if (command_raw_translation != raw_latex_encoding_to_unicode_xml.end()) {
			xs << command_raw_translation->second;
			output_as_comment = false;
		} else {
			// If the trimmed ERT ends with {}, try a mapping without it.
			auto os_braces = os_trimmed.find(from_ascii("{}"));

			if (os_braces != lyx::docstring::npos) {
				auto key = os_trimmed.substr(0, os_braces);
				auto command_braces_translation = raw_latex_encoding_to_unicode_xml.find(key);

				if (command_braces_translation != raw_latex_encoding_to_unicode_xml.end()) {
					xs << command_braces_translation->second;
					output_as_comment = false;
				}
			}
		}
	}

	// Second step: the command \string can be ignored. If that's the only command in the ERT, then done.
	// There may be several occurrences. (\string is 7 characters long.)
	if (os.str().length() >= 7) {
		docstring os_str = os.str();

		while (os_str.length() >= 7) {
			auto os_text = os_str.find(from_ascii("\\string"));

			if (os_text != lyx::docstring::npos && !std::isalpha(static_cast<int>(os_str[os_text + 7]))) {
				os_str = os_str.substr(0, os_text) + os_str.substr(os_text + 7, os_str.length());

				if (os_str.find('\\') == std::string::npos) {
					xs << os_str;
					output_as_comment = false;
					break;
				}
			} else {
				break;
			}
		}
	}

	// Otherwise, output the ERT as a comment with the appropriate escaping if the command is not recognised.
	if (output_as_comment) {
		xs << XMLStream::ESCAPE_NONE << "<!-- ";
		xs << XMLStream::ESCAPE_COMMENTS << os.str();
		xs << XMLStream::ESCAPE_NONE << " -->";
	}
}


void InsetERT::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {
	case LFUN_INSET_MODIFY:
		if (cmd.getArg(0) == "ert") {
			cur.recordUndoInset(this);
			setStatus(cur, string2params(to_utf8(cmd.argument())));
			break;
		}
		//fall-through
	default:
		InsetCollapsible::doDispatch(cur, cmd);
		break;
	}

}


bool InsetERT::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action()) {
	case LFUN_INSET_INSERT:
		status.setEnabled(false);
		return true;
	case LFUN_INSET_MODIFY:
		if (cmd.getArg(0) == "ert") {
			status.setEnabled(true);
			return true;
		}
		//fall through

	default:
		return InsetCollapsible::getStatus(cur, cmd, status);
	}
}



docstring const InsetERT::buttonLabel(BufferView const & bv) const
{
	// U+1F512 LOCK
	docstring const locked = tempfile_ ? docstring(1, 0x1F512) : docstring();
	if (decoration() == InsetDecoration::CLASSIC)
		return locked + (isOpen(bv) ? _("ERT") : getNewLabel(_("ERT")));
	return locked + getNewLabel(_("ERT"));
}


InsetCollapsible::CollapseStatus InsetERT::string2params(string const & in)
{
	if (in.empty())
		return Collapsed;
	istringstream data(in);
	Lexer lex;
	lex.setStream(data);
	lex.setContext("InsetERT::string2params");
	lex >> "ert";
	int s;
	lex >> s;
	return static_cast<CollapseStatus>(s);
}


string InsetERT::params2string(CollapseStatus status)
{
	ostringstream data;
	data << "ert" << ' ' << status;
	return data.str();
}


docstring InsetERT::xhtml(XMLStream &, OutputParams const &) const
{
	return docstring();
}

} // namespace lyx
