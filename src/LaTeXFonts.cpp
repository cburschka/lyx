/**
 * \file LaTeXFonts.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "LaTeXFonts.h"

#include "LaTeXFeatures.h"
#include "Lexer.h"

#include "frontends/alert.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"


using namespace std;
using namespace lyx::support;


namespace lyx {

LaTeXFonts latexfonts;


bool LaTeXFont::available(bool ot1) const
{
	return ot1 ? available_ot1_ : available_;
}


bool LaTeXFont::providesOSF(bool ot1) const
{
	if (!osfpackage_.empty())
		return LaTeXFeatures::isAvailable(to_ascii(osfpackage_));

	if (ot1 && !ot1package_.empty() && ot1package_ != "none")
		return false;

	if (!package_.empty() && !LaTeXFeatures::isAvailable(to_ascii(package_)))
		return false;

	return (!osfoption_.empty() || !osfscoption_.empty());
}


bool LaTeXFont::providesSC(bool ot1) const
{
	if (ot1 && !ot1package_.empty() && ot1package_ != "none")
		return false;

	if (!package_.empty() && !LaTeXFeatures::isAvailable(to_ascii(package_)))
		return false;

	return (!scoption_.empty() || !osfscoption_.empty());
}


bool LaTeXFont::providesScale(bool ot1) const
{
	if (ot1 && !ot1package_.empty() && ot1package_ != "none")
		return false;

	if (!package_.empty() && !LaTeXFeatures::isAvailable(to_ascii(package_)))
		return false;

	return (!scaleoption_.empty());
}


string const LaTeXFont::getAvailablePackage(bool dryrun, bool ot1, bool complete)
{
	if (ot1 && !ot1package_.empty()) {
		if (ot1package_ != "none"
		    && (LaTeXFeatures::isAvailable(to_ascii(ot1package_)) || dryrun))
			return to_ascii(ot1package_);
		if (!dryrun && ot1package_ != "none")
			frontend::Alert::warning(_("Font not available"),
					bformat(_("The LaTeX package `%1$s' needed for the font `%2$s'\n"
						  "is not available on your system. LyX will fall back to the default font."),
						ot1package_, guiname_), true);
		return string();
	}
	if (complete && !completepackage_.empty()) {
		if (LaTeXFeatures::isAvailable(to_ascii(completepackage_)) || dryrun)
			return to_ascii(completepackage_);
	}
	if (!package_.empty()) {
		if (!requires_.empty() && LaTeXFeatures::isAvailable(to_ascii(requires_)))
			return to_ascii(package_);
		if (LaTeXFeatures::isAvailable(to_ascii(package_)))
			return to_ascii(package_);
		else if (!altpackages_.empty()) {
			for (size_t i = 0; i < altpackages_.size(); ++i) {
				if (LaTeXFeatures::isAvailable(to_ascii(altpackages_[i])))
					return to_ascii(altpackages_[i]);
			}
		}
		// Output unavailable packages in source preview
		if (dryrun)
			return to_ascii(package_);
		docstring const req = requires_.empty() ? package_ : requires_;
			frontend::Alert::warning(_("Font not available"),
					bformat(_("The LaTeX package `%1$s' needed for the font `%2$s'\n"
						  "is not available on your system. LyX will fall back to the default font."),
						req, guiname_), true);
	}
	return string();
}


string const LaTeXFont::getPackageOptions(bool const & ot1, bool const & sc,
					  bool const & osf, int const & scale)
{
	if (ot1 && !ot1package_.empty())
		return string();

	ostringstream os;
	if (sc && osf && providesOSF() && providesSC()) {
		if (!osfscoption_.empty())
			os << to_ascii(osfscoption_);
		else
			os << to_ascii(osfoption_) << ',' << to_ascii(scoption_);
	} else if (osf && providesOSF())
		os << to_ascii(osfoption_);
	else if (sc && providesSC())
		os << to_ascii(scoption_);
	if (scale != 100 && !scaleoption_.empty()) {
		if (!os.str().empty())
			os << ',';
		os << subst(to_ascii(scaleoption_), "$$val",
			    convert<std::string>(float(scale) / 100));
	}
	return os.str();
}


bool LaTeXFont::readFont(Lexer & lex)
{
	enum LaTeXFontTags {
		LF_ALT_PACKAGES = 1,
		LF_COMPLETE_PACKAGE,
		LF_END,
		LF_FAMILY,
		LF_GUINAME,
		LF_OSFOPTION,
		LF_OSFPACKAGE,
		LF_OSFSCOPTION,
		LF_OT1_PACKAGE,
		LF_PACKAGE,
		LF_REQUIRES,
		LF_SCALEOPTION,
		LF_SCOPTION,
		LF_SWITCHDEFAULT
	};

	// Keep these sorted alphabetically!
	LexerKeyword latexFontTags[] = {
		{ "altpackages",          LF_ALT_PACKAGES },
		{ "completepackage",      LF_COMPLETE_PACKAGE },
		{ "endfont",              LF_END },
		{ "family",               LF_FAMILY },
		{ "guiname",              LF_GUINAME },
		{ "osfoption",            LF_OSFOPTION },
		{ "osfpackage",           LF_OSFPACKAGE },
		{ "osfscoption",          LF_OSFSCOPTION },
		{ "ot1package",           LF_OT1_PACKAGE },
		{ "package",              LF_PACKAGE },
		{ "requires",             LF_REQUIRES },
		{ "scaleoption",          LF_SCALEOPTION },
		{ "scoption",             LF_SCOPTION },
		{ "switchdefault",        LF_SWITCHDEFAULT }
	};

	bool error = false;
	bool finished = false;
	lex.pushTable(latexFontTags);
	// parse style section
	while (!finished && lex.isOK() && !error) {
		int le = lex.lex();
		// See comment in LyXRC.cpp.
		switch (le) {
		case Lexer::LEX_FEOF:
			continue;

		case Lexer::LEX_UNDEF: // parse error
			lex.printError("Unknown LaTeXFont tag `$$Token'");
			error = true;
			continue;

		default: 
			break;
		}
		switch (static_cast<LaTeXFontTags>(le)) {
		case LF_END: // end of structure
			finished = true;
			break;
		case LF_ALT_PACKAGES: {
			docstring altp;
			lex >> altp;
			altpackages_ = getVectorFromString(altp);
			break;
		}
		case LF_COMPLETE_PACKAGE:
			lex >> completepackage_;
			break;
		case LF_FAMILY:
			lex >> family_;
			break;
		case LF_GUINAME:
			lex >> guiname_;
			break;
		case LF_OSFOPTION:
			lex >> osfoption_;
			break;
		case LF_OSFPACKAGE:
			lex >> osfpackage_;
			break;
		case LF_OSFSCOPTION:
			lex >> osfscoption_;
			break;
		case LF_OT1_PACKAGE:
			lex >> ot1package_;
			break;
		case LF_PACKAGE:
			lex >> package_;
			break;
		case LF_REQUIRES:
			lex >> requires_;
			break;
		case LF_SCALEOPTION:
			lex >> scaleoption_;
			break;
		case LF_SCOPTION:
			lex >> scoption_;
			break;
		case LF_SWITCHDEFAULT:
			lex >> switchdefault_;
			break;
		}
	}
	if (!finished) {
		lex.printError("No End tag found for LaTeXFont tag `$$Token'");
		return false;
	}
	lex.popTable();
	return finished && !error;
}


bool LaTeXFont::read(Lexer & lex)
{
	switchdefault_ = 0;

	if (!lex.next()) {
		lex.printError("No name given for LaTeX font: `$$Token'.");
		return false;
	}

	name_ = lex.getDocString();
	LYXERR(Debug::INFO, "Reading LaTeX font " << name_);
	if (!readFont(lex)) {
		LYXERR0("Error parsing LaTeX font `" << name_ << '\'');
		return false;
	}

	bool available = true;
	if (!requires_.empty())
		available = LaTeXFeatures::isAvailable(to_ascii(requires_));
	else if (!package_.empty()) {
		available = LaTeXFeatures::isAvailable(to_ascii(package_));
		if (!available && !altpackages_.empty()) {
			for (size_t i = 0; i < altpackages_.size(); ++i) {
				available = LaTeXFeatures::isAvailable(to_ascii(altpackages_[i]));
				if (available)
					break;
			}
		}
	}
	available_ = available;

	if (!ot1package_.empty() && ot1package_ != "none")
		available_ot1_ = LaTeXFeatures::isAvailable(to_ascii(ot1package_));
	else
		available_ot1_ = available;

	return true;
}


void LaTeXFonts::readLaTeXFonts()
{
	// Read latexfonts file
	FileName filename = libFileSearch(string(), "latexfonts");
	if (filename.empty()) {
		LYXERR0("Error: latexfonts file not found!");
		return;
	}
	Lexer lex;
	lex.setFile(filename);
	lex.setContext("LaTeXFeatures::readLaTeXFonts");
	while (lex.isOK()) {
		int le = lex.lex();
		switch (le) {
		case Lexer::LEX_FEOF:
			continue;

		default:
			break;
		}
		if (lex.getString() != "Font") {
			lex.printError("Unknown LaTeXFont tag `$$Token'");
			continue;
		}
		LaTeXFont f;
		f.read(lex);
		if (!lex)
			break;

		texfontmap_[f.name()] = f;
	}
}


LaTeXFonts::TexFontMap LaTeXFonts::getLaTeXFonts()
{
	if (texfontmap_.empty())
		readLaTeXFonts();
	return texfontmap_;
}


LaTeXFont LaTeXFonts::getLaTeXFont(docstring const & name)
{
	if (texfontmap_.empty())
		readLaTeXFonts();
	return texfontmap_[name];
}


} // namespace lyx
