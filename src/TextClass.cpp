/**
 * \file TextClass.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author John Levon
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "TextClass.h"

#include "LayoutFile.h"
#include "Color.h"
#include "Counters.h"
#include "Floating.h"
#include "FloatList.h"
#include "Layout.h"
#include "Lexer.h"
#include "Font.h"

#include "frontends/alert.h"

#include "support/lassert.h"
#include "support/debug.h"
#include "support/ExceptionMessage.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/os.h"

#include <algorithm>
#include <fstream>
#include <sstream>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace {

class LayoutNamesEqual : public unary_function<Layout, bool> {
public:
	LayoutNamesEqual(docstring const & name)
		: name_(name)
	{}
	bool operator()(Layout const & c) const
	{
		return c.name() == name_;
	}
private:
	docstring name_;
};

int const FORMAT = 12;


bool layout2layout(FileName const & filename, FileName const & tempfile)
{
	FileName const script = libFileSearch("scripts", "layout2layout.py");
	if (script.empty()) {
		LYXERR0("Could not find layout conversion "
			  "script layout2layout.py.");
		return false;
	}

	ostringstream command;
	command << os::python() << ' ' << quoteName(script.toFilesystemEncoding())
		<< ' ' << quoteName(filename.toFilesystemEncoding())
		<< ' ' << quoteName(tempfile.toFilesystemEncoding());
	string const command_str = command.str();

	LYXERR(Debug::TCLASS, "Running `" << command_str << '\'');

	cmd_ret const ret = runCommand(command_str);
	if (ret.first != 0) {
		LYXERR0("Could not run layout conversion script layout2layout.py.");
		return false;
	}
	return true;
}


std::string translateRT(TextClass::ReadType rt) 
{
	switch (rt) {
	case TextClass::BASECLASS:
		return "textclass";
	case TextClass::MERGE:
		return "input file";
	case TextClass::MODULE:
		return "module file";
	case TextClass::VALIDATION:
		return "validation";
	}
	// shutup warning
	return string();
}

} // namespace anon


// This string should not be translated here, 
// because it is a layout identifier.
docstring const TextClass::plain_layout_ = from_ascii("Plain Layout");


InsetLayout DocumentClass::plain_insetlayout_;


/////////////////////////////////////////////////////////////////////////
//
// TextClass
//
/////////////////////////////////////////////////////////////////////////

TextClass::TextClass()
{
	outputType_ = LATEX;
	columns_ = 1;
	sides_ = OneSide;
	secnumdepth_ = 3;
	tocdepth_ = 3;
	pagestyle_ = "default";
	defaultfont_ = sane_font;
	opt_fontsize_ = "10|11|12";
	opt_pagestyle_ = "empty|plain|headings|fancy";
	titletype_ = TITLE_COMMAND_AFTER;
	titlename_ = "maketitle";
	loaded_ = false;
	_("Plain Layout"); // a hack to make this translatable
}


bool TextClass::readStyle(Lexer & lexrc, Layout & lay) const
{
	LYXERR(Debug::TCLASS, "Reading style " << to_utf8(lay.name()));
	if (!lay.read(lexrc, *this)) {
		LYXERR0("Error parsing style `" << to_utf8(lay.name()) << '\'');
		return false;
	}
	// Resolve fonts
	lay.resfont = lay.font;
	lay.resfont.realize(defaultfont_);
	lay.reslabelfont = lay.labelfont;
	lay.reslabelfont.realize(defaultfont_);
	return true; // no errors
}


enum TextClassTags {
	TC_OUTPUTTYPE = 1,
	TC_INPUT,
	TC_STYLE,
	TC_DEFAULTSTYLE,
	TC_INSETLAYOUT,
	TC_NOSTYLE,
	TC_COLUMNS,
	TC_SIDES,
	TC_PAGESTYLE,
	TC_DEFAULTFONT,
	TC_SECNUMDEPTH,
	TC_TOCDEPTH,
	TC_CLASSOPTIONS,
	TC_PREAMBLE,
	TC_PROVIDES,
	TC_REQUIRES,
	TC_LEFTMARGIN,
	TC_RIGHTMARGIN,
	TC_FLOAT,
	TC_COUNTER,
	TC_NOFLOAT,
	TC_TITLELATEXNAME,
	TC_TITLELATEXTYPE,
	TC_FORMAT,
	TC_ADDTOPREAMBLE,
	TC_DEFAULTMODULE,
	TC_PROVIDESMODULE,
	TC_EXCLUDESMODULE
};


namespace {

	LexerKeyword textClassTags[] = {
		{ "addtopreamble",   TC_ADDTOPREAMBLE },
		{ "classoptions",    TC_CLASSOPTIONS },
		{ "columns",         TC_COLUMNS },
		{ "counter",         TC_COUNTER },
		{ "defaultfont",     TC_DEFAULTFONT },
		{ "defaultmodule",   TC_DEFAULTMODULE },
		{ "defaultstyle",    TC_DEFAULTSTYLE },
		{ "excludesmodule",  TC_EXCLUDESMODULE },
		{ "float",           TC_FLOAT },
		{ "format",          TC_FORMAT },
		{ "input",           TC_INPUT },
		{ "insetlayout",     TC_INSETLAYOUT },
		{ "leftmargin",      TC_LEFTMARGIN },
		{ "nofloat",         TC_NOFLOAT },
		{ "nostyle",         TC_NOSTYLE },
		{ "outputtype",      TC_OUTPUTTYPE },
		{ "pagestyle",       TC_PAGESTYLE },
		{ "preamble",        TC_PREAMBLE },
		{ "provides",        TC_PROVIDES },
		{ "providesmodule",  TC_PROVIDESMODULE },
		{ "requires",        TC_REQUIRES },
		{ "rightmargin",     TC_RIGHTMARGIN },
		{ "secnumdepth",     TC_SECNUMDEPTH },
		{ "sides",           TC_SIDES },
		{ "style",           TC_STYLE },
		{ "titlelatexname",  TC_TITLELATEXNAME },
		{ "titlelatextype",  TC_TITLELATEXTYPE },
		{ "tocdepth",        TC_TOCDEPTH }
	};
	
} //namespace anon


bool TextClass::convertLayoutFormat(support::FileName const & filename, ReadType rt)
{
	LYXERR(Debug::TCLASS, "Converting layout file to " << FORMAT);
	FileName const tempfile = FileName::tempName("convert_layout");
	bool success = layout2layout(filename, tempfile);
	if (success)
		success = read(tempfile, rt);
	tempfile.removeFile();
	return success;
}

bool TextClass::read(FileName const & filename, ReadType rt)
{
	if (!filename.isReadableFile()) {
		lyxerr << "Cannot read layout file `" << filename << "'."
		       << endl;
		return false;
	}

	LYXERR(Debug::TCLASS, "Reading " + translateRT(rt) + ": " +
		to_utf8(makeDisplayPath(filename.absFilename())));

	// Define the plain layout used in table cells, ert, etc. Note that 
	// we do this before loading any layout file, so that classes can 
	// override features of this layout if they should choose to do so.
	if (rt == BASECLASS && !hasLayout(plain_layout_))
		layoutlist_.push_back(createBasicLayout(plain_layout_));

	Lexer lexrc(textClassTags);
	lexrc.setFile(filename);
	ReturnValues retval = read(lexrc, rt);
	
	LYXERR(Debug::TCLASS, "Finished reading " + translateRT(rt) + ": " +
			to_utf8(makeDisplayPath(filename.absFilename())));
	
	if (retval != FORMAT_MISMATCH) 
		return retval == OK;
	
	bool const worx = convertLayoutFormat(filename, rt);
	if (!worx) {
		LYXERR0 ("Unable to convert " << filename << 
			" to format " << FORMAT);
		return false;
	}
	return true;
}


bool TextClass::validate(std::string const & str)
{
	TextClass tc;
	return tc.read(str, VALIDATION);
}


bool TextClass::read(std::string const & str, ReadType rt) 
{
	Lexer lexrc(textClassTags);
	istringstream is(str);
	lexrc.setStream(is);
	ReturnValues retval = read(lexrc, rt);

	if (retval != FORMAT_MISMATCH) 
		return retval == OK;

	// write the layout string to a temporary file
	FileName const tempfile = FileName::tempName("TextClass_read");
	ofstream os(tempfile.toFilesystemEncoding().c_str());
	if (!os) {
		LYXERR0("Unable to create temporary file");
		return false;
	}
	os << str;
	os.close();

	// now try to convert it
	bool const worx = convertLayoutFormat(tempfile, rt);
	if (!worx) {
		LYXERR0("Unable to convert internal layout information to format " 
			<< FORMAT);
	}
	tempfile.removeFile();
	return worx;
}


// Reads a textclass structure from file.
TextClass::ReturnValues TextClass::read(Lexer & lexrc, ReadType rt) 
{
	bool error = !lexrc.isOK();

	// Format of files before the 'Format' tag was introduced
	int format = 1;

	// parsing
	while (lexrc.isOK() && !error) {
		int le = lexrc.lex();

		switch (le) {
		case Lexer::LEX_FEOF:
			continue;

		case Lexer::LEX_UNDEF:
			lexrc.printError("Unknown TextClass tag `$$Token'");
			error = true;
			continue;

		default:
			break;
		}

		switch (static_cast<TextClassTags>(le)) {

		case TC_FORMAT:
			if (lexrc.next())
				format = lexrc.getInteger();
			break;

		case TC_OUTPUTTYPE:   // output type definition
			readOutputType(lexrc);
			break;

		case TC_INPUT: // Include file
			if (lexrc.next()) {
				string const inc = lexrc.getString();
				FileName tmp = libFileSearch("layouts", inc,
							    "layout");

				if (tmp.empty()) {
					lexrc.printError("Could not find input file: " + inc);
					error = true;
				} else if (!read(tmp, MERGE)) {
					lexrc.printError("Error reading input"
							 "file: " + tmp.absFilename());
					error = true;
				}
			}
			break;

		case TC_DEFAULTSTYLE:
			if (lexrc.next()) {
				docstring const name = from_utf8(subst(lexrc.getString(),
							  '_', ' '));
				defaultlayout_ = name;
			}
			break;

		case TC_STYLE: {
			if (!lexrc.next()) {
				lexrc.printError("No name given for style: `$$Token'.");
				error = true;
				break;
			}
			docstring const name = from_utf8(subst(lexrc.getString(),
							'_', ' '));
			if (name.empty()) {
				string s = "Could not read name for style: `$$Token' "
					+ lexrc.getString() + " is probably not valid UTF-8!";
				lexrc.printError(s.c_str());
				Layout lay;
				// Since we couldn't read the name, we just scan the rest
				// of the style and discard it.
				error = !readStyle(lexrc, lay);
			} else if (hasLayout(name)) {
				Layout & lay = operator[](name);
				error = !readStyle(lexrc, lay);
			} else {
				Layout layout;
				layout.setName(name);
				error = !readStyle(lexrc, layout);
				if (!error)
					layoutlist_.push_back(layout);

				if (defaultlayout_.empty()) {
					// We do not have a default layout yet, so we choose
					// the first layout we encounter.
					defaultlayout_ = name;
				}
			}
			break;
		}

		case TC_NOSTYLE:
			if (lexrc.next()) {
				docstring const style = from_utf8(subst(lexrc.getString(),
						     '_', ' '));
				if (!deleteLayout(style))
					lyxerr << "Cannot delete style `"
					       << to_utf8(style) << '\'' << endl;
			}
			break;

		case TC_COLUMNS:
			if (lexrc.next())
				columns_ = lexrc.getInteger();
			break;

		case TC_SIDES:
			if (lexrc.next()) {
				switch (lexrc.getInteger()) {
				case 1: sides_ = OneSide; break;
				case 2: sides_ = TwoSides; break;
				default:
					lyxerr << "Impossible number of page"
						" sides, setting to one."
					       << endl;
					sides_ = OneSide;
					break;
				}
			}
			break;

		case TC_PAGESTYLE:
			lexrc.next();
			pagestyle_ = rtrim(lexrc.getString());
			break;

		case TC_DEFAULTFONT:
			defaultfont_ = lyxRead(lexrc);
			if (!defaultfont_.resolved()) {
				lexrc.printError("Warning: defaultfont should "
						 "be fully instantiated!");
				defaultfont_.realize(sane_font);
			}
			break;

		case TC_SECNUMDEPTH:
			lexrc.next();
			secnumdepth_ = lexrc.getInteger();
			break;

		case TC_TOCDEPTH:
			lexrc.next();
			tocdepth_ = lexrc.getInteger();
			break;

		// First step to support options
		case TC_CLASSOPTIONS:
			readClassOptions(lexrc);
			break;

		case TC_PREAMBLE:
			preamble_ = from_utf8(lexrc.getLongString("EndPreamble"));
			break;

		case TC_ADDTOPREAMBLE:
			preamble_ += from_utf8(lexrc.getLongString("EndPreamble"));
			break;

		case TC_PROVIDES: {
			lexrc.next();
			string const feature = lexrc.getString();
			lexrc.next();
			if (lexrc.getInteger())
				provides_.insert(feature);
			else
				provides_.erase(feature);
			break;
		}

		case TC_REQUIRES: {
			lexrc.eatLine();
			vector<string> const req 
				= getVectorFromString(lexrc.getString());
			requires_.insert(req.begin(), req.end());
			break;
		}

		case TC_DEFAULTMODULE: {
			lexrc.next();
			string const module = lexrc.getString();
			if (find(default_modules_.begin(), default_modules_.end(), module) == default_modules_.end())
				default_modules_.push_back(module);
			break;
		}

		case TC_PROVIDESMODULE: {
			lexrc.next();
			string const module = lexrc.getString();
			if (find(provided_modules_.begin(), provided_modules_.end(), module) == provided_modules_.end())
				provided_modules_.push_back(module);
			break;
		}

		case TC_EXCLUDESMODULE: {
			lexrc.next();
			string const module = lexrc.getString();
			// modules already have their own way to exclude other modules
			if (rt == MODULE) {
				LYXERR0("ExcludesModule tag cannot be used in a module!");
				break;
			}
			if (find(excluded_modules_.begin(), excluded_modules_.end(), module) == excluded_modules_.end())
				excluded_modules_.push_back(module);
			break;
		}

		case TC_LEFTMARGIN:	// left margin type
			if (lexrc.next())
				leftmargin_ = lexrc.getDocString();
			break;

		case TC_RIGHTMARGIN:	// right margin type
			if (lexrc.next())
				rightmargin_ = lexrc.getDocString();
			break;

		case TC_INSETLAYOUT: {
			if (!lexrc.next()) {
				lexrc.printError("No name given for InsetLayout: `$$Token'.");
				error = true;
				break;
			}
			docstring const name = subst(lexrc.getDocString(), '_', ' ');
			if (name.empty()) {
				string s = "Could not read name for InsetLayout: `$$Token' "
					+ lexrc.getString() + " is probably not valid UTF-8!";
				lexrc.printError(s.c_str());
				InsetLayout il;
				// Since we couldn't read the name, we just scan the rest
				// of the style and discard it.
				il.read(lexrc, *this);
				error = true;
			} else if (hasInsetLayout(name)) {
				InsetLayout & il = insetlayoutlist_[name];
				error = !il.read(lexrc, *this);
			} else {
				InsetLayout il;
				il.setName(name);
				error = !il.read(lexrc, *this);
				if (!error)
					insetlayoutlist_[name] = il;
			}
			break;
		}

		case TC_FLOAT:
			readFloat(lexrc);
			break;

		case TC_COUNTER:
			if (lexrc.next()) {
				docstring const name = lexrc.getDocString();
				if (name.empty()) {
					string s = "Could not read name for counter: `$$Token' "
							+ lexrc.getString() + " is probably not valid UTF-8!";
					lexrc.printError(s.c_str());
					Counter c;
					// Since we couldn't read the name, we just scan the rest
					// and discard it.
					c.read(lexrc);
				} else
					error = !counters_.read(lexrc, name);
			}
			else {
				lexrc.printError("No name given for style: `$$Token'.");
				error = true;
			}
			break;

		case TC_TITLELATEXTYPE:
			readTitleType(lexrc);
			break;

		case TC_TITLELATEXNAME:
			if (lexrc.next())
				titlename_ = lexrc.getString();
			break;

		case TC_NOFLOAT:
			if (lexrc.next()) {
				string const nofloat = lexrc.getString();
				floatlist_.erase(nofloat);
			}
			break;
		} // end of switch

		//Note that this is triggered the first time through the loop unless
		//we hit a format tag.
		if (format != FORMAT)
			break;
	}

	if (format != FORMAT)
		return FORMAT_MISMATCH;

	if (rt != BASECLASS) 
		return (error ? ERROR : OK);

	if (defaultlayout_.empty()) {
		LYXERR0("Error: Textclass '" << name_
						<< "' is missing a defaultstyle.");
		error = true;
	}
		
	// Try to erase "stdinsets" from the provides_ set. 
	// The
	//   Provides stdinsets 1
	// declaration simply tells us that the standard insets have been
	// defined. (It's found in stdinsets.inc but could also be used in
	// user-defined files.) There isn't really any such package. So we
	// might as well go ahead and erase it.
	// If we do not succeed, then it was not there, which means that
	// the textclass did not provide the definitions of the standard
	// insets. So we need to try to load them.
	int erased = provides_.erase("stdinsets");
	if (!erased) {
		FileName tmp = libFileSearch("layouts", "stdinsets.inc");

		if (tmp.empty()) {
			throw ExceptionMessage(WarningException, _("Missing File"),
				_("Could not find stdinsets.inc! This may lead to data loss!"));
			error = true;
		} else if (!read(tmp, MERGE)) {
			throw ExceptionMessage(WarningException, _("Corrupt File"),
				_("Could not read stdinsets.inc! This may lead to data loss!"));
			error = true;
		}
	}

	min_toclevel_ = Layout::NOT_IN_TOC;
	max_toclevel_ = Layout::NOT_IN_TOC;
	const_iterator lit = begin();
	const_iterator len = end();
	for (; lit != len; ++lit) {
		int const toclevel = lit->toclevel;
		if (toclevel != Layout::NOT_IN_TOC) {
			if (min_toclevel_ == Layout::NOT_IN_TOC)
				min_toclevel_ = toclevel;
			else
				min_toclevel_ = min(min_toclevel_, toclevel);
			max_toclevel_ = max(max_toclevel_, toclevel);
		}
	}
	LYXERR(Debug::TCLASS, "Minimum TocLevel is " << min_toclevel_
		<< ", maximum is " << max_toclevel_);

	return (error ? ERROR : OK);
}


void TextClass::readTitleType(Lexer & lexrc)
{
	LexerKeyword titleTypeTags[] = {
		{ "commandafter", TITLE_COMMAND_AFTER },
		{ "environment",  TITLE_ENVIRONMENT }
	};

	PushPopHelper pph(lexrc, titleTypeTags);

	int le = lexrc.lex();
	switch (le) {
	case Lexer::LEX_UNDEF:
		lexrc.printError("Unknown output type `$$Token'");
		break;
	case TITLE_COMMAND_AFTER:
	case TITLE_ENVIRONMENT:
		titletype_ = static_cast<TitleLatexType>(le);
		break;
	default:
		LYXERR0("Unhandled value " << le << " in TextClass::readTitleType.");
		break;
	}
}


void TextClass::readOutputType(Lexer & lexrc)
{
	LexerKeyword outputTypeTags[] = {
		{ "docbook",  DOCBOOK },
		{ "latex",    LATEX },
		{ "literate", LITERATE }
	};

	PushPopHelper pph(lexrc, outputTypeTags);

	int le = lexrc.lex();
	switch (le) {
	case Lexer::LEX_UNDEF:
		lexrc.printError("Unknown output type `$$Token'");
		return;
	case LATEX:
	case DOCBOOK:
	case LITERATE:
		outputType_ = static_cast<OutputType>(le);
		break;
	default:
		LYXERR0("Unhandled value " << le);
		break;
	}
}


void TextClass::readClassOptions(Lexer & lexrc)
{
	enum {
		CO_FONTSIZE = 1,
		CO_PAGESTYLE,
		CO_OTHER,
		CO_HEADER,
		CO_END
	};

	LexerKeyword classOptionsTags[] = {
		{"end",       CO_END },
		{"fontsize",  CO_FONTSIZE },
		{"header",    CO_HEADER },
		{"other",     CO_OTHER },
		{"pagestyle", CO_PAGESTYLE }
	};

	lexrc.pushTable(classOptionsTags);
	bool getout = false;
	while (!getout && lexrc.isOK()) {
		int le = lexrc.lex();
		switch (le) {
		case Lexer::LEX_UNDEF:
			lexrc.printError("Unknown ClassOption tag `$$Token'");
			continue;
		default: break;
		}
		switch (le) {
		case CO_FONTSIZE:
			lexrc.next();
			opt_fontsize_ = rtrim(lexrc.getString());
			break;
		case CO_PAGESTYLE:
			lexrc.next();
			opt_pagestyle_ = rtrim(lexrc.getString());
			break;
		case CO_OTHER:
			lexrc.next();
			options_ = lexrc.getString();
			break;
		case CO_HEADER:
			lexrc.next();
			class_header_ = subst(lexrc.getString(), "&quot;", "\"");
			break;
		case CO_END:
			getout = true;
			break;
		}
	}
	lexrc.popTable();
}


void TextClass::readFloat(Lexer & lexrc)
{
	enum {
		FT_TYPE = 1,
		FT_NAME,
		FT_PLACEMENT,
		FT_EXT,
		FT_WITHIN,
		FT_STYLE,
		FT_LISTNAME,
		FT_BUILTIN,
		FT_END
	};

	LexerKeyword floatTags[] = {
		{ "end", FT_END },
		{ "extension", FT_EXT },
		{ "guiname", FT_NAME },
		{ "latexbuiltin", FT_BUILTIN },
		{ "listname", FT_LISTNAME },
		{ "numberwithin", FT_WITHIN },
		{ "placement", FT_PLACEMENT },
		{ "style", FT_STYLE },
		{ "type", FT_TYPE }
	};

	lexrc.pushTable(floatTags);

	string type;
	string placement;
	string ext;
	string within;
	string style;
	string name;
	string listName;
	bool builtin = false;

	bool getout = false;
	while (!getout && lexrc.isOK()) {
		int le = lexrc.lex();
		switch (le) {
		case Lexer::LEX_UNDEF:
			lexrc.printError("Unknown float tag `$$Token'");
			continue;
		default: break;
		}
		switch (le) {
		case FT_TYPE:
			lexrc.next();
			type = lexrc.getString();
			if (floatlist_.typeExist(type)) {
				Floating const & fl = floatlist_.getType(type);
				placement = fl.placement();
				ext = fl.ext();
				within = fl.within();
				style = fl.style();
				name = fl.name();
				listName = fl.listName();
				builtin = fl.builtin();
			} 
			break;
		case FT_NAME:
			lexrc.next();
			name = lexrc.getString();
			break;
		case FT_PLACEMENT:
			lexrc.next();
			placement = lexrc.getString();
			break;
		case FT_EXT:
			lexrc.next();
			ext = lexrc.getString();
			break;
		case FT_WITHIN:
			lexrc.next();
			within = lexrc.getString();
			if (within == "none")
				within.erase();
			break;
		case FT_STYLE:
			lexrc.next();
			style = lexrc.getString();
			break;
		case FT_LISTNAME:
			lexrc.next();
			listName = lexrc.getString();
			break;
		case FT_BUILTIN:
			lexrc.next();
			builtin = lexrc.getBool();
			break;
		case FT_END:
			getout = true;
			break;
		}
	}

	// Here if have a full float if getout == true
	if (getout) {
		Floating fl(type, placement, ext, within,
			    style, name, listName, builtin);
		floatlist_.newFloat(fl);
		// each float has its own counter
		counters_.newCounter(from_ascii(type), from_ascii(within),
				      docstring(), docstring());
		// also define sub-float counters
		docstring const subtype = "sub-" + from_ascii(type);
		counters_.newCounter(subtype, from_ascii(type),
				      "\\alph{" + subtype + "}", docstring());
	}

	lexrc.popTable();
}


bool TextClass::hasLayout(docstring const & n) const
{
	docstring const name = n.empty() ? defaultLayoutName() : n;

	return find_if(layoutlist_.begin(), layoutlist_.end(),
		       LayoutNamesEqual(name))
		!= layoutlist_.end();
}


bool TextClass::hasInsetLayout(docstring const & n) const
{
	if (n.empty()) 
		return false;
	InsetLayouts::const_iterator it = insetlayoutlist_.begin();
	InsetLayouts::const_iterator en = insetlayoutlist_.end();
	for (; it != en; ++it)
		if (n == it->first)
			return true;
	return false;
}


Layout const & TextClass::operator[](docstring const & name) const
{
	LASSERT(!name.empty(), /**/);

	const_iterator it = 
		find_if(begin(), end(), LayoutNamesEqual(name));

	if (it == end()) {
		lyxerr << "We failed to find the layout '" << to_utf8(name)
		       << "' in the layout list. You MUST investigate!"
		       << endl;
		for (const_iterator cit = begin(); cit != end(); ++cit)
			lyxerr  << " " << to_utf8(cit->name()) << endl;

		// we require the name to exist
		LASSERT(false, /**/);
	}

	return *it;
}


Layout & TextClass::operator[](docstring const & name)
{
	LASSERT(!name.empty(), /**/);

	iterator it = find_if(begin(), end(), LayoutNamesEqual(name));

	if (it == end()) {
		LYXERR0("We failed to find the layout '" << to_utf8(name)
		       << "' in the layout list. You MUST investigate!");
		for (const_iterator cit = begin(); cit != end(); ++cit)
			LYXERR0(" " << to_utf8(cit->name()));

		// we require the name to exist
		LASSERT(false, /**/);
	}

	return *it;
}


bool TextClass::deleteLayout(docstring const & name)
{
	if (name == defaultLayoutName() || name == plainLayoutName())
		return false;

	LayoutList::iterator it =
		remove_if(layoutlist_.begin(), layoutlist_.end(),
			  LayoutNamesEqual(name));

	LayoutList::iterator end = layoutlist_.end();
	bool const ret = (it != end);
	layoutlist_.erase(it, end);
	return ret;
}


// Load textclass info if not loaded yet
bool TextClass::load(string const & path) const
{
	if (loaded_)
		return true;

	// Read style-file, provided path is searched before the system ones
	// If path is a file, it is loaded directly.
	FileName layout_file(path);
	if (!path.empty() && !layout_file.isReadableFile())
		layout_file = FileName(addName(path, name_ + ".layout"));
	if (layout_file.empty() || !layout_file.exists())
		layout_file = libFileSearch("layouts", name_, "layout");
	loaded_ = const_cast<TextClass*>(this)->read(layout_file);

	if (!loaded_) {
		lyxerr << "Error reading `"
		       << to_utf8(makeDisplayPath(layout_file.absFilename()))
		       << "'\n(Check `" << name_
		       << "')\nCheck your installation and "
			"try Options/Reconfigure..." << endl;
	}

	return loaded_;
}


void DocumentClass::addLayoutIfNeeded(docstring const & n) const
{
	if (!hasLayout(n))
		layoutlist_.push_back(createBasicLayout(n, true));
}


InsetLayout const & DocumentClass::insetLayout(docstring const & name) const 
{
	// FIXME The fix for the InsetLayout part of 4812 would be here:
	// Add the InsetLayout to the document class if it is not found.
	docstring n = name;
	InsetLayouts::const_iterator cen = insetlayoutlist_.end();
	while (!n.empty()) {
		InsetLayouts::const_iterator cit = insetlayoutlist_.lower_bound(n);
		if (cit != cen && cit->first == n)
			return cit->second;
		size_t i = n.find(':');
		if (i == string::npos)
			break;
		n = n.substr(0, i);
	}
	return plain_insetlayout_;
}


docstring const & TextClass::defaultLayoutName() const
{
	// This really should come from the actual layout... (Lgb)
	return defaultlayout_;
}


Layout const & TextClass::defaultLayout() const
{
	return operator[](defaultLayoutName());
}


bool TextClass::isDefaultLayout(Layout const & layout) const 
{
	return layout.name() == defaultLayoutName();
}


bool TextClass::isPlainLayout(Layout const & layout) const 
{
	return layout.name() == plainLayoutName();
}


Layout TextClass::createBasicLayout(docstring const & name, bool unknown) const
{
	static Layout * defaultLayout = NULL;

	if (defaultLayout) {
		defaultLayout->setUnknown(unknown);
		defaultLayout->setName(name);
		return *defaultLayout;
	}

	static char const * s = "Margin Static\n"
			"LatexType Paragraph\n"
			"LatexName dummy\n"
			"Align Block\n"
			"AlignPossible Left, Right, Center\n"
			"LabelType No_Label\n"
			"End";
	istringstream ss(s);
	Lexer lex(textClassTags);
	lex.setStream(ss);
	defaultLayout = new Layout;
	defaultLayout->setUnknown(unknown);
	defaultLayout->setName(name);
	if (!readStyle(lex, *defaultLayout)) {
		// The only way this happens is because the hardcoded layout above
		// is wrong.
		LASSERT(false, /**/);
	};
	return *defaultLayout;
}

/////////////////////////////////////////////////////////////////////////
//
// DocumentClassBundle
//
/////////////////////////////////////////////////////////////////////////

DocumentClassBundle::~DocumentClassBundle()
{
	for (size_t i = 0; i != documentClasses_.size(); ++i)
		delete documentClasses_[i];
	documentClasses_.clear();
}

DocumentClass & DocumentClassBundle::newClass(LayoutFile const & baseClass)
{
	DocumentClass * dc = new DocumentClass(baseClass);
	documentClasses_.push_back(dc);
	return *documentClasses_.back();
}


DocumentClassBundle & DocumentClassBundle::get()
{
	static DocumentClassBundle singleton; 
	return singleton; 
}


/////////////////////////////////////////////////////////////////////////
//
// DocumentClass
//
/////////////////////////////////////////////////////////////////////////

DocumentClass::DocumentClass(LayoutFile const & tc)
	: TextClass(tc)
{}


bool DocumentClass::hasLaTeXLayout(std::string const & lay) const
{
	LayoutList::const_iterator it  = layoutlist_.begin();
	LayoutList::const_iterator end = layoutlist_.end();
	for (; it != end; ++it)
		if (it->latexname() == lay)
			return true;
	return false;
}


bool DocumentClass::provides(string const & p) const
{
	return provides_.find(p) != provides_.end();
}


bool DocumentClass::hasTocLevels() const
{
	return min_toclevel_ != Layout::NOT_IN_TOC;
}


/////////////////////////////////////////////////////////////////////////
//
// PageSides
//
/////////////////////////////////////////////////////////////////////////

ostream & operator<<(ostream & os, PageSides p)
{
	switch (p) {
	case OneSide:
		os << '1';
		break;
	case TwoSides:
		os << '2';
		break;
	}
	return os;
}


} // namespace lyx
