// -*- C++ -*-
/**
 * \file InsetLayout.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetLayout.h"

#include "ColorSet.h"
#include "Layout.h"
#include "Lexer.h"
#include "TextClass.h"

#include "support/debug.h"
#include "support/lstrings.h"
#include "support/textutils.h"

#include <vector>

using std::string;
using std::set;
using std::vector;

using namespace lyx::support;

namespace lyx {

InsetLayout::InsetLayout() :
	name_(from_ascii("undefined")), lyxtype_(STANDARD),
	labelstring_(from_ascii("UNDEFINED")), contentaslabel_(false),
	decoration_(DEFAULT), latextype_(NOLATEXTYPE), font_(inherit_font),
	labelfont_(sane_font), bgcolor_(Color_error),
	fixedwidthpreambleencoding_(false), htmlforcecss_ (false),
	htmlisblock_(true), multipar_(true), custompars_(true),
	forceplain_(false), passthru_(false), parbreakisnewline_(false),
	freespacing_(false), keepempty_(false), forceltr_(false),
	forceownlines_(false), needprotect_(false), intoc_(false),
	spellcheck_(true), resetsfont_(false), display_(true),
	forcelocalfontswitch_(false), add_to_toc_(false), is_toc_caption_(false)
{
	labelfont_.setColor(Color_error);
}


InsetLayout::InsetDecoration translateDecoration(std::string const & str)
{
	if (compare_ascii_no_case(str, "classic") == 0)
		return InsetLayout::CLASSIC;
	if (compare_ascii_no_case(str, "minimalistic") == 0)
		return InsetLayout::MINIMALISTIC;
	if (compare_ascii_no_case(str, "conglomerate") == 0)
		return InsetLayout::CONGLOMERATE;
	return InsetLayout::DEFAULT;
}

namespace {

InsetLayout::InsetLaTeXType translateLaTeXType(std::string const & str)
{
	if (compare_ascii_no_case(str, "command") == 0)
		return InsetLayout::COMMAND;
	if (compare_ascii_no_case(str, "environment") == 0)
		return InsetLayout::ENVIRONMENT;
	if (compare_ascii_no_case(str, "none") == 0)
		return InsetLayout::NOLATEXTYPE;
	return InsetLayout::ILT_ERROR;
}

} // namespace anon


bool InsetLayout::read(Lexer & lex, TextClass const & tclass)
{
	enum {
		IL_ADDTOTOC,
		IL_ARGUMENT,
		IL_BABELPREAMBLE,
		IL_BGCOLOR,
		IL_CONTENTASLABEL,
		IL_COPYSTYLE,
		IL_COUNTER,
		IL_CUSTOMPARS,
		IL_DECORATION,
		IL_DISPLAY,
		IL_FIXEDWIDTH_PREAMBLE_ENCODING,
		IL_FONT,
		IL_FORCE_LOCAL_FONT_SWITCH,
		IL_FORCELTR,
		IL_FORCEOWNLINES,
		IL_FORCEPLAIN,
		IL_FREESPACING,
		IL_HTMLTAG,
		IL_HTMLATTR,
		IL_HTMLFORCECSS,
		IL_HTMLINNERTAG,
		IL_HTMLINNERATTR,
		IL_HTMLISBLOCK,
		IL_HTMLLABEL,
		IL_HTMLSTYLE,
		IL_HTMLPREAMBLE,
		IL_INTOC,
		IL_ISTOCCAPTION,
		IL_LABELFONT,
		IL_LABELSTRING,
		IL_LANGPREAMBLE,
		IL_LATEXNAME,
		IL_LATEXPARAM,
		IL_LATEXTYPE,
		IL_LEFTDELIM,
		IL_LYXTYPE,
		IL_OBSOLETEDBY,
		IL_KEEPEMPTY,
		IL_MULTIPAR,
		IL_NEEDPROTECT,
		IL_PASSTHRU,
		IL_PASSTHRU_CHARS,
		IL_PARBREAKISNEWLINE,
		IL_PREAMBLE,
		IL_REQUIRES,
		IL_RIGHTDELIM,
		IL_REFPREFIX,
		IL_RESETARGS,
		IL_RESETSFONT,
		IL_SPELLCHECK,
		IL_END
	};


	LexerKeyword elementTags[] = {
		{ "addtotoc", IL_ADDTOTOC },
		{ "argument", IL_ARGUMENT },
		{ "babelpreamble", IL_BABELPREAMBLE },
		{ "bgcolor", IL_BGCOLOR },
		{ "contentaslabel", IL_CONTENTASLABEL },
		{ "copystyle", IL_COPYSTYLE },
		{ "counter", IL_COUNTER},
		{ "custompars", IL_CUSTOMPARS },
		{ "decoration", IL_DECORATION },
		{ "display", IL_DISPLAY },
		{ "end", IL_END },
		{ "fixedwidthpreambleencoding", IL_FIXEDWIDTH_PREAMBLE_ENCODING },
		{ "font", IL_FONT },
		{ "forcelocalfontswitch", IL_FORCE_LOCAL_FONT_SWITCH },
		{ "forceltr", IL_FORCELTR },
		{ "forceownlines", IL_FORCEOWNLINES },
		{ "forceplain", IL_FORCEPLAIN },
		{ "freespacing", IL_FREESPACING },
		{ "htmlattr", IL_HTMLATTR },
		{ "htmlforcecss", IL_HTMLFORCECSS },
		{ "htmlinnerattr", IL_HTMLINNERATTR},
		{ "htmlinnertag", IL_HTMLINNERTAG},
		{ "htmlisblock", IL_HTMLISBLOCK},
		{ "htmllabel", IL_HTMLLABEL },
		{ "htmlpreamble", IL_HTMLPREAMBLE },
		{ "htmlstyle", IL_HTMLSTYLE },
		{ "htmltag", IL_HTMLTAG },
		{ "intoc", IL_INTOC },
		{ "istoccaption", IL_ISTOCCAPTION },
		{ "keepempty", IL_KEEPEMPTY },
		{ "labelfont", IL_LABELFONT },
		{ "labelstring", IL_LABELSTRING },
		{ "langpreamble", IL_LANGPREAMBLE },
		{ "latexname", IL_LATEXNAME },
		{ "latexparam", IL_LATEXPARAM },
		{ "latextype", IL_LATEXTYPE },
		{ "leftdelim", IL_LEFTDELIM },
		{ "lyxtype", IL_LYXTYPE },
		{ "multipar", IL_MULTIPAR },
		{ "needprotect", IL_NEEDPROTECT },
		{ "obsoletedby", IL_OBSOLETEDBY },
		{ "parbreakisnewline", IL_PARBREAKISNEWLINE },
		{ "passthru", IL_PASSTHRU },
		{ "passthruchars", IL_PASSTHRU_CHARS },
		{ "preamble", IL_PREAMBLE },
		{ "refprefix", IL_REFPREFIX },
		{ "requires", IL_REQUIRES },
		{ "resetargs", IL_RESETARGS },
		{ "resetsfont", IL_RESETSFONT },
		{ "rightdelim", IL_RIGHTDELIM },
		{ "spellcheck", IL_SPELLCHECK }
	};

	lex.pushTable(elementTags);

	labelfont_ = inherit_font;
	bgcolor_ = Color_none;
	bool getout = false;
	// whether we've read the CustomPars or ForcePlain tag
	// for issuing a warning in case MultiPars comes later
	bool readCustomOrPlain = false;

	string tmp;
	while (!getout && lex.isOK()) {
		int le = lex.lex();
		switch (le) {
		case Lexer::LEX_UNDEF:
			lex.printError("Unknown InsetLayout tag");
			continue;
		default:
			break;
		}
		switch (le) {
		// FIXME
		// Perhaps a more elegant way to deal with the next two would be the
		// way this sort of thing is handled in Layout::read(), namely, by
		// using the Lexer.
		case IL_LYXTYPE: {
			// make sure that we have the right sort of name.
			if (name_ != from_ascii("undefined")
			    && name_.substr(0,5) != from_ascii("Flex:")) {
				LYXERR0("Flex insets must have names of the form `Flex:<name>'.\n"
				        "This one has the name `" << to_utf8(name_) << "'\n"
				        "Ignoring LyXType declaration.");
				break;
			}
			string lt;
			lex >> lt;
			lyxtype_ = translateLyXType(lt);
			if (lyxtype_  == NOLYXTYPE)
				LYXERR0("Unknown LyXType `" << lt << "'.");
			if (lyxtype_ == CHARSTYLE) {
				// by default, charstyles force the plain layout
				multipar_ = false;
				forceplain_ = true;
			}
			break;
		}
		case IL_LATEXTYPE:  {
			string lt;
			lex >> lt;
			latextype_ = translateLaTeXType(lt);
			if (latextype_  == ILT_ERROR)
				LYXERR0("Unknown LaTeXType `" << lt << "'.");
			break;
		}
		case IL_LABELSTRING:
			lex >> labelstring_;
			break;
		case IL_DECORATION:
			lex >> tmp;
			decoration_ = translateDecoration(tmp);
			break;
		case IL_LATEXNAME:
			lex >> latexname_;
			break;
		case IL_LATEXPARAM:
			lex >> tmp;
			latexparam_ = subst(tmp, "&quot;", "\"");
			break;
		case IL_LEFTDELIM:
			lex >> leftdelim_;
			leftdelim_ = subst(leftdelim_, from_ascii("<br/>"),
						    from_ascii("\n"));
			break;
		case IL_FIXEDWIDTH_PREAMBLE_ENCODING:
			lex >> fixedwidthpreambleencoding_;
			break;
		case IL_FORCE_LOCAL_FONT_SWITCH:
			lex >> forcelocalfontswitch_;
			break;
		case IL_RIGHTDELIM:
			lex >> rightdelim_;
			rightdelim_ = subst(rightdelim_, from_ascii("<br/>"),
						     from_ascii("\n"));
			break;
		case IL_LABELFONT:
			labelfont_ = lyxRead(lex, inherit_font);
			break;
		case IL_FORCELTR:
			lex >> forceltr_;
			break;
		case IL_FORCEOWNLINES:
			lex >> forceownlines_;
			break;
		case IL_INTOC:
			lex >> intoc_;
			break;
		case IL_MULTIPAR:
			lex >> multipar_;
			// the defaults for these depend upon multipar_
			if (readCustomOrPlain)
				LYXERR0("Warning: Read MultiPar after CustomPars or ForcePlain. "
				        "Previous value may be overwritten!");
			readCustomOrPlain = false;
			custompars_ = multipar_;
			forceplain_ = !multipar_;
			break;
		case IL_COUNTER:
			lex >> counter_;
			break;
		case IL_CUSTOMPARS:
			lex >> custompars_;
			readCustomOrPlain = true;
			break;
		case IL_FORCEPLAIN:
			lex >> forceplain_;
			readCustomOrPlain = true;
			break;
		case IL_PASSTHRU:
			lex >> passthru_;
			break;
		case IL_PASSTHRU_CHARS:
			lex >> passthru_chars_;
			break;
		case IL_PARBREAKISNEWLINE:
			lex >> parbreakisnewline_;
			break;
		case IL_KEEPEMPTY:
			lex >> keepempty_;
			break;
		case IL_FREESPACING:
			lex >> freespacing_;
			break;
		case IL_NEEDPROTECT:
			lex >> needprotect_;
			break;
		case IL_CONTENTASLABEL:
			lex >> contentaslabel_;
			break;
		case IL_COPYSTYLE: {
			// initialize with a known style
			docstring style;
			lex >> style;
			style = subst(style, '_', ' ');

			// We don't want to apply the algorithm in DocumentClass::insetLayout()
			// here. So we do it the long way.
			TextClass::InsetLayouts::const_iterator it =
					tclass.insetLayouts().find(style);
			if (it != tclass.insetLayouts().end()) {
				docstring const tmpname = name_;
				this->operator=(it->second);
				name_ = tmpname;
			} else {
				LYXERR0("Cannot copy unknown InsetLayout `"
					<< style << "' to InsetLayout `"
					<< name() << "'\n"
					<< "All InsetLayouts so far:");
				TextClass::InsetLayouts::const_iterator lit =
						tclass.insetLayouts().begin();
				TextClass::InsetLayouts::const_iterator len =
						tclass.insetLayouts().end();
				for (; lit != len; ++lit)
					lyxerr << lit->second.name() << "\n";
			}
			break;
		}
		case IL_OBSOLETEDBY: {
			docstring style;
			lex >> style;
			style = subst(style, '_', ' ');

			// We don't want to apply the algorithm in DocumentClass::insetLayout()
			// here. So we do it the long way.
			TextClass::InsetLayouts::const_iterator it =
					tclass.insetLayouts().find(style);
			if (it != tclass.insetLayouts().end()) {
				docstring const tmpname = name_;
				this->operator=(it->second);
				name_ = tmpname;
				if (obsoleted_by().empty())
					obsoleted_by_ = style;
			} else {
				LYXERR0("Cannot replace InsetLayout `"
					<< name()
					<< "' with unknown InsetLayout `"
					<< style << "'\n"
					<< "All InsetLayouts so far:");
				TextClass::InsetLayouts::const_iterator lit =
						tclass.insetLayouts().begin();
				TextClass::InsetLayouts::const_iterator len =
						tclass.insetLayouts().end();
				for (; lit != len; ++lit)
					lyxerr << lit->second.name() << "\n";
			}
			break;
		}

		case IL_FONT: {
			font_ = lyxRead(lex, inherit_font);
			// If you want to define labelfont, you need to do so after
			// font is defined.
			labelfont_ = font_;
			break;
		}
		case IL_RESETARGS:
			bool reset;
			lex >> reset;
			if (reset) {
				latexargs_.clear();
				postcommandargs_.clear();
			}
			break;
		case IL_ARGUMENT:
			readArgument(lex);
			break;
		case IL_BGCOLOR:
			lex >> tmp;
			bgcolor_ = lcolor.getFromLyXName(tmp);
			break;
		case IL_PREAMBLE:
			preamble_ = from_utf8(lex.getLongString("EndPreamble"));
			break;
		case IL_BABELPREAMBLE:
			babelpreamble_ = from_utf8(lex.getLongString("EndBabelPreamble"));
			break;
		case IL_LANGPREAMBLE:
			langpreamble_ = from_utf8(lex.getLongString("EndLangPreamble"));
			break;
		case IL_REFPREFIX:
			lex >> refprefix_;
			break;
		case IL_HTMLTAG:
			lex >> htmltag_;
			break;
		case IL_HTMLATTR:
			lex >> htmlattr_;
			break;
		case IL_HTMLFORCECSS:
			lex >> htmlforcecss_;
			break;
		case IL_HTMLINNERTAG:
			lex >> htmlinnertag_;
			break;
		case IL_HTMLINNERATTR:
			lex >> htmlinnerattr_;
			break;
		case IL_HTMLLABEL:
			lex >> htmllabel_;
			break;
		case IL_HTMLISBLOCK:
			lex >> htmlisblock_;
			break;
		case IL_HTMLSTYLE:
			htmlstyle_ = from_utf8(lex.getLongString("EndHTMLStyle"));
			break;
		case IL_HTMLPREAMBLE:
			htmlpreamble_ = from_utf8(lex.getLongString("EndPreamble"));
			break;
		case IL_REQUIRES: {
			lex.eatLine();
			vector<string> const req
				= getVectorFromString(lex.getString(true));
			requires_.insert(req.begin(), req.end());
			break;
		}
		case IL_SPELLCHECK:
			lex >> spellcheck_;
			break;
		case IL_RESETSFONT:
			lex >> resetsfont_;
			break;
		case IL_DISPLAY:
			lex >> display_;
			break;
		case IL_ADDTOTOC:
			lex >> toc_type_;
			add_to_toc_ = !toc_type_.empty();
			break;
		case IL_ISTOCCAPTION:
			lex >> is_toc_caption_;
			break;
		case IL_END:
			getout = true;
			break;
		}
	}

	// Here add element to list if getout == true
	if (!getout)
		return false;

	// The label font is generally used as-is without
	// any realization against a given context.
	labelfont_.realize(sane_font);

	lex.popTable();
	return true;
}


InsetLayout::InsetLyXType translateLyXType(std::string const & str)
{
	if (compare_ascii_no_case(str, "charstyle") == 0)
		return InsetLayout::CHARSTYLE;
	if (compare_ascii_no_case(str, "custom") == 0)
		return InsetLayout::CUSTOM;
	if (compare_ascii_no_case(str, "element") == 0)
		return InsetLayout::ELEMENT;
	if (compare_ascii_no_case(str, "end") == 0)
		return InsetLayout::END;
	if (compare_ascii_no_case(str, "standard") == 0)
		return InsetLayout::STANDARD;
	return InsetLayout::NOLYXTYPE;
}


string const & InsetLayout::htmltag() const
{
	if (htmltag_.empty())
		htmltag_ = multipar_ ? "div" : "span";
	return htmltag_;
}


string const & InsetLayout::htmlattr() const
{
	if (htmlattr_.empty())
		htmlattr_ = "class=\"" + defaultCSSClass() + "\"";
	return htmlattr_;
}


string const & InsetLayout::htmlinnerattr() const
{
	if (htmlinnerattr_.empty())
		htmlinnerattr_ = "class=\"" + defaultCSSClass() + "_inner\"";
	return htmlinnerattr_;
}


string InsetLayout::defaultCSSClass() const
{
	if (!defaultcssclass_.empty())
		return defaultcssclass_;
	string d;
	string n = to_utf8(name());
	string::const_iterator it = n.begin();
	string::const_iterator en = n.end();
	for (; it != en; ++it) {
		if (!isAlphaASCII(*it))
			d += "_";
		else if (isLower(*it))
			d += *it;
		else
			d += lowercase(*it);
	}
	// are there other characters we need to remove?
	defaultcssclass_ = d;
	return defaultcssclass_;
}


void InsetLayout::makeDefaultCSS() const
{
	if (!htmldefaultstyle_.empty())
		return;
	docstring const mainfontCSS = font_.asCSS();
	if (!mainfontCSS.empty())
		htmldefaultstyle_ =
				from_ascii(htmltag() + "." + defaultCSSClass() + " {\n") +
				mainfontCSS + from_ascii("\n}\n");
}


docstring InsetLayout::htmlstyle() const
{
	if (!htmlstyle_.empty() && !htmlforcecss_)
		return htmlstyle_;
	if (htmldefaultstyle_.empty())
		makeDefaultCSS();
	docstring retval = htmldefaultstyle_;
	if (!htmlstyle_.empty())
		retval += '\n' + htmlstyle_ + '\n';
	return retval;
}

void InsetLayout::readArgument(Lexer & lex)
{
	Layout::latexarg arg;
	arg.mandatory = false;
	arg.autoinsert = false;
	arg.insertcotext = false;
	bool error = false;
	bool finished = false;
	arg.font = inherit_font;
	arg.labelfont = inherit_font;
	arg.is_toc_caption = false;
	string nr;
	lex >> nr;
	bool const postcmd = prefixIs(nr, "post:");
	while (!finished && lex.isOK() && !error) {
		lex.next();
		string const tok = ascii_lowercase(lex.getString());

		if (tok.empty()) {
			continue;
		} else if (tok == "endargument") {
			finished = true;
		} else if (tok == "labelstring") {
			lex.next();
			arg.labelstring = lex.getDocString();
		} else if (tok == "menustring") {
			lex.next();
			arg.menustring = lex.getDocString();
		} else if (tok == "mandatory") {
			lex.next();
			arg.mandatory = lex.getBool();
		} else if (tok == "autoinsert") {
			lex.next();
			arg.autoinsert = lex.getBool();
		} else if (tok == "insertcotext") {
			lex.next();
			arg.insertcotext = lex.getBool();
		} else if (tok == "leftdelim") {
			lex.next();
			arg.ldelim = lex.getDocString();
			arg.ldelim = subst(arg.ldelim,
						    from_ascii("<br/>"), from_ascii("\n"));
		} else if (tok == "rightdelim") {
			lex.next();
			arg.rdelim = lex.getDocString();
			arg.rdelim = subst(arg.rdelim,
						    from_ascii("<br/>"), from_ascii("\n"));
		} else if (tok == "defaultarg") {
			lex.next();
			arg.defaultarg = lex.getDocString();
		} else if (tok == "presetarg") {
			lex.next();
			arg.presetarg = lex.getDocString();
		} else if (tok == "tooltip") {
			lex.next();
			arg.tooltip = lex.getDocString();
		} else if (tok == "requires") {
			lex.next();
			arg.requires = lex.getString();
		} else if (tok == "decoration") {
			lex.next();
			arg.decoration = lex.getString();
		} else if (tok == "font") {
			arg.font = lyxRead(lex, arg.font);
		} else if (tok == "labelfont") {
			arg.labelfont = lyxRead(lex, arg.labelfont);
		} else if (tok == "passthruchars") {
			lex.next();
			arg.pass_thru_chars = lex.getDocString();
		} else if (tok == "istoccaption") {
			lex.next();
			arg.is_toc_caption = lex.getBool();
		} else {
			lex.printError("Unknown tag");
			error = true;
		}
	}
	if (arg.labelstring.empty())
		LYXERR0("Incomplete Argument definition!");
	else if (postcmd)
		postcommandargs_[nr] = arg;
	else
		latexargs_[nr] = arg;
}


Layout::LaTeXArgMap InsetLayout::args() const
{
	Layout::LaTeXArgMap args = latexargs_;
	if (!postcommandargs_.empty())
		args.insert(postcommandargs_.begin(), postcommandargs_.end());
	return args;
}


unsigned int InsetLayout::optArgs() const
{
	unsigned int nr = 0;
	Layout::LaTeXArgMap const args = InsetLayout::args();
	Layout::LaTeXArgMap::const_iterator it = args.begin();
	for (; it != args.end(); ++it) {
		if (!(*it).second.mandatory)
			++nr;
	}
	return nr;
}


unsigned int InsetLayout::requiredArgs() const
{
	unsigned int nr = 0;
	Layout::LaTeXArgMap const args = InsetLayout::args();
	Layout::LaTeXArgMap::const_iterator it = args.begin();
	for (; it != args.end(); ++it) {
		if ((*it).second.mandatory)
			++nr;
	}
	return nr;
}


} //namespace lyx
