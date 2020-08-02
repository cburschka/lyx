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
#include "CiteEnginesList.h"
#include "Color.h"
#include "Counters.h"
#include "Floating.h"
#include "FloatList.h"
#include "Layout.h"
#include "Lexer.h"
#include "Font.h"
#include "ModuleList.h"

#include "frontends/alert.h"

#include "support/lassert.h"
#include "support/debug.h"
#include "support/ExceptionMessage.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/TempFile.h"

#include <algorithm>
#include <fstream>
#include <sstream>

#ifdef ERROR
#undef ERROR
#endif

using namespace std;
using namespace lyx::support;

namespace lyx {

// Keep the changes documented in the Customization manual.
//
// If you change this format, then you MUST also make sure that
// your changes do not invalidate the hardcoded layout file in
// LayoutFile.cpp. Additions will never do so, but syntax changes
// could. See LayoutFileList::addEmptyClass() and, especially, the
// definition of the layoutpost string.
// You should also run the development/tools/updatelayouts.py script,
// to update the format of all of our layout files.
//
int const LAYOUT_FORMAT = 83; // tcuvelier: DocBookWrapperMergeWithPrevious.


// Layout format for the current lyx file format. Controls which format is
// targeted by Local Layout > Convert. In master, equal to LAYOUT_FORMAT.
int const LYXFILE_LAYOUT_FORMAT = LAYOUT_FORMAT;


namespace {

bool layout2layout(FileName const & filename, FileName const & tempfile,
                   int const format = LAYOUT_FORMAT)
{
	FileName const script = libFileSearch("scripts", "layout2layout.py");
	if (script.empty()) {
		LYXERR0("Could not find layout conversion "
		        "script layout2layout.py.");
		return false;
	}

	ostringstream command;
	command << os::python() << ' ' << quoteName(script.toFilesystemEncoding())
	        << " -t " << format
	        << ' ' << quoteName(filename.toFilesystemEncoding())
	        << ' ' << quoteName(tempfile.toFilesystemEncoding());
	string const command_str = command.str();

	LYXERR(Debug::TCLASS, "Running `" << command_str << '\'');

	cmd_ret const ret = runCommand(command_str);
	if (ret.first != 0) {
		if (format == LAYOUT_FORMAT)
			LYXERR0("Conversion of layout with layout2layout.py has failed.");
		return false;
	}
	return true;
}


string translateReadType(TextClass::ReadType rt)
{
	switch (rt) {
	case TextClass::BASECLASS:
		return "textclass";
	case TextClass::MERGE:
		return "input file";
	case TextClass::MODULE:
		return "module file";
	case TextClass::CITE_ENGINE:
		return "cite engine";
	case TextClass::VALIDATION:
		return "validation";
	}
	// shutup warning
	return string();
}

} // namespace


// This string should not be translated here,
// because it is a layout identifier.
docstring const TextClass::plain_layout_ = from_ascii(N_("Plain Layout"));


/////////////////////////////////////////////////////////////////////////
//
// TextClass
//
/////////////////////////////////////////////////////////////////////////

TextClass::TextClass()
	: loaded_(false), tex_class_avail_(false),
	  opt_enginetype_("authoryear|numerical"), opt_fontsize_("10|11|12"),
	  opt_pagesize_("default|a4|a5|b5|letter|legal|executive"),
	  opt_pagestyle_("empty|plain|headings|fancy"), fontsize_format_("$$spt"), pagesize_("default"),
	  pagesize_format_("$$spaper"), pagestyle_("default"), tablestyle_("default"),
	  docbookroot_("article"), docbookforceabstract_(false),
	  columns_(1), sides_(OneSide), secnumdepth_(3), tocdepth_(3), outputType_(LATEX),
	  outputFormat_("latex"), has_output_format_(false), defaultfont_(sane_font), 
	  titletype_(TITLE_COMMAND_AFTER), titlename_("maketitle"),
	  min_toclevel_(0), max_toclevel_(0), maxcitenames_(2),
	  cite_full_author_list_(true), bibintoc_(false) {
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
	TC_OUTPUTFORMAT,
	TC_INPUT,
	TC_STYLE,
	TC_MODIFYSTYLE,
	TC_PROVIDESTYLE,
	TC_DEFAULTSTYLE,
	TC_INSETLAYOUT,
	TC_NOINSETLAYOUT,
	TC_NOSTYLE,
	TC_COLUMNS,
	TC_SIDES,
	TC_PAGESTYLE,
	TC_PAGESIZE,
	TC_DEFAULTFONT,
	TC_SECNUMDEPTH,
	TC_TOCDEPTH,
	TC_CLASSOPTIONS,
	TC_PREAMBLE,
	TC_HTMLPREAMBLE,
	TC_HTMLSTYLES,
	TC_PROVIDES,
	TC_REQUIRES,
	TC_PKGOPTS,
	TC_LEFTMARGIN,
	TC_RIGHTMARGIN,
	TC_FLOAT,
	TC_COUNTER,
	TC_NOCOUNTER,
	TC_IFCOUNTER,
	TC_NOFLOAT,
	TC_TITLELATEXNAME,
	TC_TITLELATEXTYPE,
	TC_FORMAT,
	TC_ADDTOPREAMBLE,
	TC_ADDTOHTMLPREAMBLE,
	TC_ADDTOHTMLSTYLES,
	TC_DEFAULTMODULE,
	TC_PROVIDESMODULE,
	TC_EXCLUDESMODULE,
	TC_HTMLTOCSECTION,
	TC_CITEENGINE,
	TC_ADDTOCITEENGINE,
	TC_CITEENGINETYPE,
	TC_CITEFORMAT,
	TC_CITEFRAMEWORK,
	TC_MAXCITENAMES,
	TC_DEFAULTBIBLIO,
	TC_FULLAUTHORLIST,
	TC_OUTLINERNAME,
	TC_TABLESTYLE,
	TC_BIBINTOC,
	TC_DOCBOOKROOT,
	TC_DOCBOOKFORCEABSTRACT
};


namespace {

LexerKeyword textClassTags[] = {
	{ "addtociteengine",   TC_ADDTOCITEENGINE },
	{ "addtohtmlpreamble", TC_ADDTOHTMLPREAMBLE },
	{ "addtohtmlstyles",   TC_ADDTOHTMLSTYLES },
	{ "addtopreamble",     TC_ADDTOPREAMBLE },
	{ "bibintoc",          TC_BIBINTOC },
	{ "citeengine",        TC_CITEENGINE },
	{ "citeenginetype",    TC_CITEENGINETYPE },
	{ "citeformat",        TC_CITEFORMAT },
	{ "citeframework",     TC_CITEFRAMEWORK },
	{ "classoptions",      TC_CLASSOPTIONS },
	{ "columns",           TC_COLUMNS },
	{ "counter",           TC_COUNTER },
	{ "defaultbiblio",     TC_DEFAULTBIBLIO },
	{ "defaultfont",       TC_DEFAULTFONT },
	{ "defaultmodule",     TC_DEFAULTMODULE },
	{ "defaultstyle",      TC_DEFAULTSTYLE },
	{ "docbookforceabstract", TC_DOCBOOKFORCEABSTRACT },
	{ "docbookroot",       TC_DOCBOOKROOT },
	{ "excludesmodule",    TC_EXCLUDESMODULE },
	{ "float",             TC_FLOAT },
	{ "format",            TC_FORMAT },
	{ "fullauthorlist",    TC_FULLAUTHORLIST },
	{ "htmlpreamble",      TC_HTMLPREAMBLE },
	{ "htmlstyles",        TC_HTMLSTYLES },
	{ "htmltocsection",    TC_HTMLTOCSECTION },
	{ "ifcounter",         TC_IFCOUNTER },
	{ "input",             TC_INPUT },
	{ "insetlayout",       TC_INSETLAYOUT },
	{ "leftmargin",        TC_LEFTMARGIN },
	{ "maxcitenames",      TC_MAXCITENAMES },
	{ "modifystyle",       TC_MODIFYSTYLE },
	{ "nocounter",         TC_NOCOUNTER },
	{ "nofloat",           TC_NOFLOAT },
	{ "noinsetlayout",     TC_NOINSETLAYOUT },
	{ "nostyle",           TC_NOSTYLE },
	{ "outlinername",      TC_OUTLINERNAME },
	{ "outputformat",      TC_OUTPUTFORMAT },
	{ "outputtype",        TC_OUTPUTTYPE },
	{ "packageoptions",    TC_PKGOPTS },
	{ "pagesize",          TC_PAGESIZE },
	{ "pagestyle",         TC_PAGESTYLE },
	{ "preamble",          TC_PREAMBLE },
	{ "provides",          TC_PROVIDES },
	{ "providesmodule",    TC_PROVIDESMODULE },
	{ "providestyle",      TC_PROVIDESTYLE },
	{ "requires",          TC_REQUIRES },
	{ "rightmargin",       TC_RIGHTMARGIN },
	{ "secnumdepth",       TC_SECNUMDEPTH },
	{ "sides",             TC_SIDES },
	{ "style",             TC_STYLE },
	{ "tablestyle",        TC_TABLESTYLE },
	{ "titlelatexname",    TC_TITLELATEXNAME },
	{ "titlelatextype",    TC_TITLELATEXTYPE },
	{ "tocdepth",          TC_TOCDEPTH }
};

} // namespace


bool TextClass::convertLayoutFormat(support::FileName const & filename, ReadType rt)
{
	LYXERR(Debug::TCLASS, "Converting layout file to " << LAYOUT_FORMAT);
	TempFile tmp("convertXXXXXX.layout");
	FileName const tempfile = tmp.name();
	bool success = layout2layout(filename, tempfile);
	if (success)
		success = readWithoutConv(tempfile, rt) == OK;
	return success;
}


std::string TextClass::convert(std::string const & str)
{
	TempFile tmp1("localXXXXXX.layout");
	FileName const fn = tmp1.name();
	ofstream os(fn.toFilesystemEncoding().c_str());
	os << str;
	os.close();
	TempFile tmp2("convert_localXXXXXX.layout");
	FileName const tempfile = tmp2.name();
	bool success = layout2layout(fn, tempfile, LYXFILE_LAYOUT_FORMAT);
	if (!success)
		return "";
	ifstream is(tempfile.toFilesystemEncoding().c_str());
	string ret;
	string tmp;
	while (!is.eof()) {
		getline(is, tmp);
		ret += tmp + '\n';
	}
	is.close();
	return ret;
}


TextClass::ReturnValues TextClass::readWithoutConv(FileName const & filename, ReadType rt)
{
	if (!filename.isReadableFile()) {
		lyxerr << "Cannot read layout file `" << filename << "'."
		       << endl;
		return ERROR;
	}

	LYXERR(Debug::TCLASS, "Reading " + translateReadType(rt) + ": " +
		to_utf8(makeDisplayPath(filename.absFileName())));

	// Define the plain layout used in table cells, ert, etc. Note that
	// we do this before loading any layout file, so that classes can
	// override features of this layout if they should choose to do so.
	if (rt == BASECLASS && !hasLayout(plain_layout_))
		layoutlist_.push_back(createBasicLayout(plain_layout_));

	Lexer lexrc(textClassTags);
	lexrc.setFile(filename);
	ReturnValues retval = read(lexrc, rt);

	LYXERR(Debug::TCLASS, "Finished reading " + translateReadType(rt) + ": " +
			to_utf8(makeDisplayPath(filename.absFileName())));

	return retval;
}


bool TextClass::read(FileName const & filename, ReadType rt)
{
	ReturnValues const retval = readWithoutConv(filename, rt);
	if (retval != FORMAT_MISMATCH)
		return retval == OK;

	bool const worx = convertLayoutFormat(filename, rt);
	if (!worx)
		LYXERR0 ("Unable to convert " << filename <<
			" to format " << LAYOUT_FORMAT);
	return worx;
}


TextClass::ReturnValues TextClass::validate(std::string const & str)
{
	TextClass tc;
	return tc.read(str, VALIDATION);
}


TextClass::ReturnValues TextClass::read(std::string const & str, ReadType rt)
{
	Lexer lexrc(textClassTags);
	istringstream is(str);
	lexrc.setStream(is);
	ReturnValues retval = read(lexrc, rt);

	if (retval != FORMAT_MISMATCH)
		return retval;

	// write the layout string to a temporary file
	TempFile tmp("TextClass_read");
	FileName const tempfile = tmp.name();
	ofstream os(tempfile.toFilesystemEncoding().c_str());
	if (!os) {
		LYXERR0("Unable to create temporary file");
		return ERROR;
	}
	os << str;
	os.close();

	// now try to convert it to LAYOUT_FORMAT
	if (!convertLayoutFormat(tempfile, rt)) {
		LYXERR0("Unable to convert internal layout information to format "
			<< LAYOUT_FORMAT);
		return ERROR;
	}

	return OK_OLDFORMAT;
}


// Reads a textclass structure from file.
TextClass::ReturnValues TextClass::read(Lexer & lexrc, ReadType rt)
{
	if (!lexrc.isOK())
		return ERROR;

	// The first usable line should be
	// Format LAYOUT_FORMAT
	if (lexrc.lex() != TC_FORMAT || !lexrc.next()
	    || lexrc.getInteger() != LAYOUT_FORMAT)
		return FORMAT_MISMATCH;

	// parsing
	bool error = false;
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

		// used below to track whether we are in an IfStyle or IfCounter tag.
		bool modifystyle  = false;
		bool providestyle = false;
		bool ifcounter    = false;

		switch (static_cast<TextClassTags>(le)) {

		case TC_FORMAT:
			lexrc.next();
			lexrc.printError("Duplicate Format directive");
			break;

		case TC_OUTPUTFORMAT:
			if (lexrc.next()) {
				outputFormat_ = lexrc.getString();
				has_output_format_ = true;
			}
			break;

		case TC_OUTPUTTYPE:
			readOutputType(lexrc);
			switch(outputType_) {
			case LATEX:
				outputFormat_ = "latex";
				break;
			case LITERATE:
				outputFormat_ = "literate";
				break;
			}
			break;

		case TC_INPUT: // Include file
			if (lexrc.next()) {
				FileName tmp;
				string const inc = lexrc.getString();
				if (!path().empty() && (prefixIs(inc, "./") ||
							prefixIs(inc, "../")))
					tmp = fileSearch(path(), inc, "layout");
				else
					tmp = libFileSearch("layouts", inc,
							    "layout");

				if (tmp.empty()) {
					lexrc.printError("Could not find input file: " + inc);
					error = true;
				} else if (!read(tmp, MERGE)) {
					lexrc.printError("Error reading input file: " + tmp.absFileName());
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

		case TC_MODIFYSTYLE:
			modifystyle = true;
		// fall through
		case TC_PROVIDESTYLE:
			// if modifystyle is true, then we got here by falling through
			// so we are not in an ProvideStyle block
			if (!modifystyle)
				providestyle = true;
		// fall through
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
				lexrc.printError(s);
				Layout lay;
				// Since we couldn't read the name, we just scan the rest
				// of the style and discard it.
				error = !readStyle(lexrc, lay);
				break;
			}

			bool const have_layout = hasLayout(name);

			// If the layout already exists, then we want to add it to
			// the existing layout, as long as we are not in an ProvideStyle
			// block.
			if (have_layout && !providestyle) {
				Layout & lay = operator[](name);
				error = !readStyle(lexrc, lay);
			}
			// If the layout does not exist, then we want to create a new
			// one, but not if we are in a ModifyStyle block.
			else if (!have_layout && !modifystyle) {
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
			// There are two ways to get here:
			//  (i)  The layout exists but we are in an ProvideStyle block
			//  (ii) The layout doesn't exist, but we are in an ModifyStyle
			//       block.
			// Either way, we just scan the rest and discard it
			else {
				Layout lay;
				// signal to coverity that we do not care about the result
				(void)readStyle(lexrc, lay);
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

		case TC_NOINSETLAYOUT:
			if (lexrc.next()) {
				docstring const style = from_utf8(subst(lexrc.getString(),
								 '_', ' '));
				if (!deleteInsetLayout(style))
					LYXERR0("Style `" << style << "' cannot be removed\n"
						"because it was not found!");
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

		case TC_PAGESIZE:
			lexrc.next();
			pagesize_ = rtrim(lexrc.getString());
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
			preamble_ = lexrc.getLongString(from_ascii("EndPreamble"));
			break;

		case TC_HTMLPREAMBLE:
			htmlpreamble_ = lexrc.getLongString(from_ascii("EndPreamble"));
			break;

		case TC_HTMLSTYLES:
			htmlstyles_ = lexrc.getLongString(from_ascii("EndStyles"));
			break;

		case TC_HTMLTOCSECTION:
			html_toc_section_ = from_utf8(trim(lexrc.getString()));
			break;

		case TC_ADDTOPREAMBLE:
			preamble_ += lexrc.getLongString(from_ascii("EndPreamble"));
			break;

		case TC_ADDTOHTMLPREAMBLE:
			htmlpreamble_ += lexrc.getLongString(from_ascii("EndPreamble"));
			break;

		case TC_ADDTOHTMLSTYLES:
			htmlstyles_ += lexrc.getLongString(from_ascii("EndStyles"));
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
			required_.insert(req.begin(), req.end());
			break;
		}

		case TC_PKGOPTS : {
			lexrc.next();
			string const pkg = lexrc.getString();
			lexrc.next();
			string const options = lexrc.getString();
			package_options_[pkg] = options;
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
			bool const validating = (rt == VALIDATION);
			if (name.empty()) {
				string s = "Could not read name for InsetLayout: `$$Token' "
					+ lexrc.getString() + " is probably not valid UTF-8!";
				lexrc.printError(s);
				InsetLayout il;
				// Since we couldn't read the name, we just scan the rest
				// of the style and discard it.
				il.read(lexrc, *this);
				// Let's try to continue rather than abort, unless we're validating
				// in which case we want to report the error
				if (validating)
					error = true;
			} else if (hasInsetLayout(name)) {
				InsetLayout & il = insetlayoutlist_[name];
				error = !il.read(lexrc, *this, validating);
			} else {
				InsetLayout il;
				il.setName(name);
				error = !il.read(lexrc, *this, validating);
				if (!error)
					insetlayoutlist_[name] = il;
			}
			break;
		}

		case TC_FLOAT:
			error = !readFloat(lexrc);
			break;

		case TC_CITEENGINE:
			error = !readCiteEngine(lexrc, rt);
			break;

		case TC_ADDTOCITEENGINE:
			error = !readCiteEngine(lexrc, rt, true);
			break;

		case TC_CITEENGINETYPE:
			if (lexrc.next())
				opt_enginetype_ = rtrim(lexrc.getString());
			break;

		case TC_CITEFORMAT:
			error = !readCiteFormat(lexrc, rt);
			break;

		case TC_CITEFRAMEWORK:
			lexrc.next();
			citeframework_ = rtrim(lexrc.getString());
			break;

		case TC_MAXCITENAMES:
			lexrc.next();
			maxcitenames_ = size_t(lexrc.getInteger());
			break;

		case TC_DEFAULTBIBLIO:
			if (lexrc.next()) {
				vector<string> const dbs =
					getVectorFromString(rtrim(lexrc.getString()), "|");
				for (auto const & dbase : dbs) {
					if (!contains(dbase, ':')) {
						vector<string> const enginetypes =
							getVectorFromString(opt_enginetype_, "|");
						for (string const & s: enginetypes)
							cite_default_biblio_style_[s] = dbase;
					} else {
						string eng;
						string const db = split(dbase, eng, ':');
						cite_default_biblio_style_[eng] = db;
					}
				}
			}
			break;

		case TC_BIBINTOC:
			if (lexrc.next())
				bibintoc_ = lexrc.getBool();
			break;

		case TC_FULLAUTHORLIST:
			if (lexrc.next())
				cite_full_author_list_ &= lexrc.getBool();
			break;

		case TC_NOCOUNTER:
			if (lexrc.next()) {
				docstring const cnt = lexrc.getDocString();
				if (!counters_.remove(cnt))
					LYXERR0("Unable to remove counter: " + to_utf8(cnt));
			}
			break;

		case TC_IFCOUNTER:
			ifcounter = true;
			// fall through
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
					error = !counters_.read(lexrc, name, !ifcounter);
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

		case TC_OUTLINERNAME:
			error = !readOutlinerName(lexrc);
			break;

        case TC_TABLESTYLE:
			lexrc.next();
			tablestyle_ = rtrim(lexrc.getString());
			break;

		case TC_DOCBOOKROOT:
			if (lexrc.next())
				docbookroot_ = lexrc.getString();
			break;

		case TC_DOCBOOKFORCEABSTRACT:
			if (lexrc.next())
				docbookforceabstract_ = lexrc.getBool();
			break;
		} // end of switch
	}

	// at present, we abort if we encounter an error,
	// so there is no point continuing.
	if (error)
		return ERROR;

	if (rt != BASECLASS)
		return OK;

	if (defaultlayout_.empty()) {
		LYXERR0("Error: Textclass '" << name_
						<< "' is missing a defaultstyle.");
		return ERROR;
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
	size_type const erased = provides_.erase("stdinsets");
	if (!erased) {
		FileName tmp = libFileSearch("layouts", "stdinsets.inc");

		if (tmp.empty()) {
			frontend::Alert::warning(_("Missing File"),
				_("Could not find stdinsets.inc! This may lead to data loss!"));
			error = true;
		} else if (!read(tmp, MERGE)) {
			frontend::Alert::warning(_("Corrupt File"),
				_("Could not read stdinsets.inc! This may lead to data loss!"));
			error = true;
		}
	}

	min_toclevel_ = Layout::NOT_IN_TOC;
	max_toclevel_ = Layout::NOT_IN_TOC;
	for (auto const & lay : *this) {
		int const toclevel = lay.toclevel;
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
		CO_FONTSIZE_FORMAT,
		CO_PAGESIZE,
		CO_PAGESIZE_FORMAT,
		CO_PAGESTYLE,
		CO_OTHER,
		CO_END
	};

	LexerKeyword classOptionsTags[] = {
		{"end",       CO_END },
		{"fontsize",  CO_FONTSIZE },
		{"fontsizeformat", CO_FONTSIZE_FORMAT },
		{"other",     CO_OTHER },
		{"pagesize",  CO_PAGESIZE },
		{"pagesizeformat", CO_PAGESIZE_FORMAT },
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
		default:
			break;
		}
		switch (le) {
		case CO_FONTSIZE:
			lexrc.next();
			opt_fontsize_ = rtrim(lexrc.getString());
			break;
		case CO_FONTSIZE_FORMAT:
			lexrc.next();
			fontsize_format_ = rtrim(lexrc.getString());
			break;
		case CO_PAGESIZE:
			lexrc.next();
			opt_pagesize_ = rtrim(lexrc.getString());
			break;
		case CO_PAGESIZE_FORMAT:
			lexrc.next();
			pagesize_format_ = rtrim(lexrc.getString());
			break;
		case CO_PAGESTYLE:
			lexrc.next();
			opt_pagestyle_ = rtrim(lexrc.getString());
			break;
		case CO_OTHER:
			lexrc.next();
			if (options_.empty())
				options_ = lexrc.getString();
			else
				options_ += ',' + lexrc.getString();
			break;
		case CO_END:
			getout = true;
			break;
		}
	}
	lexrc.popTable();
}


vector<CitationStyle> const & TextClass::getCiteStyles(
	CiteEngineType const & type) const
{
	static vector<CitationStyle> empty;
	map<CiteEngineType, vector<CitationStyle> >::const_iterator it = cite_styles_.find(type);
	if (it == cite_styles_.end())
		return empty;
	return it->second;
}


bool TextClass::readCiteEngine(Lexer & lexrc, ReadType rt, bool const add)
{
	int const type = readCiteEngineType(lexrc);
	bool authoryear = (type & ENGINE_TYPE_AUTHORYEAR);
	bool numerical = (type & ENGINE_TYPE_NUMERICAL);
	bool defce = (type & ENGINE_TYPE_DEFAULT);

	if (rt == CITE_ENGINE) {
		// The cite engines are not supposed to overwrite
		// CiteStyle defined by the class or a module.
		if (authoryear)
			authoryear = getCiteStyles(ENGINE_TYPE_AUTHORYEAR).empty();
		if (numerical)
			numerical = getCiteStyles(ENGINE_TYPE_NUMERICAL).empty();
		if (defce)
			defce = getCiteStyles(ENGINE_TYPE_DEFAULT).empty();
	}

	if (rt != CITE_ENGINE && !add) {
		// Reset if we defined CiteStyle
		// from the class or a module
		if (authoryear)
			cite_styles_[ENGINE_TYPE_AUTHORYEAR].clear();
		if (numerical)
			cite_styles_[ENGINE_TYPE_NUMERICAL].clear();
		if (defce)
			cite_styles_[ENGINE_TYPE_DEFAULT].clear();
	}

	string def;
	bool getout = false;
	while (!getout && lexrc.isOK()) {
		lexrc.eatLine();
		def = lexrc.getString();
		def = subst(def, " ", "");
		def = subst(def, "\t", "");
		if (compare_ascii_no_case(def, "end") == 0) {
			getout = true;
			continue;
		}
		CitationStyle cs;
		char ichar = def[0];
		if (ichar == '#')
			continue;
		if (isUpperCase(ichar)) {
			cs.forceUpperCase = true;
			def[0] = lowercase(ichar);
		}

		/** For portability reasons (between different
		 *  cite engines such as natbib and biblatex),
		 *  we distinguish between:
		 *  1. The LyX name as output in the LyX file
		 *  2. Possible aliases that might fall back to
		 *     the given LyX name in the current engine
		 *  3. The actual LaTeX command that is output
		 *  (2) and (3) are optional.
		 *  Also, the GUI string for the starred version can
		 *  be changed
		 *  The syntax is:
		 *  LyXName|alias,nextalias*<!stardesc!stardesctooltip>[][]=latexcmd
		 */
		enum ScanMode {
			LyXName,
			Alias,
			LaTeXCmd,
			StarDesc
		};

		ScanMode mode = LyXName;
		ScanMode oldmode = LyXName;
		string lyx_cmd;
		string alias;
		string latex_cmd;
		string stardesc;
		size_t const n = def.size();
		for (size_t i = 0; i != n; ++i) {
			ichar = def[i];
			if (ichar == '|')
				mode = Alias;
			else if (ichar == '=')
				mode = LaTeXCmd;
			else if (ichar == '<') {
				oldmode = mode;
				mode = StarDesc;
			} else if (ichar == '>')
				mode = oldmode;
			else if (mode == LaTeXCmd)
				latex_cmd += ichar;
			else if (mode == StarDesc)
				stardesc += ichar;
			else if (ichar == '$')
				cs.hasQualifiedList = true;
			else if (ichar == '*')
				cs.hasStarredVersion = true;
			else if (ichar == '[' && cs.textAfter)
				cs.textBefore = true;
			else if (ichar == '[')
				cs.textAfter = true;
			else if (ichar != ']') {
				if (mode == Alias)
					alias += ichar;
				else
					lyx_cmd += ichar;
			}
		}
		cs.name = lyx_cmd;
		cs.cmd = latex_cmd.empty() ? lyx_cmd : latex_cmd;
		if (!alias.empty()) {
			vector<string> const aliases = getVectorFromString(alias);
			for (string const & s: aliases)
				cite_command_aliases_[s] = lyx_cmd;
		}
		vector<string> const stardescs = getVectorFromString(stardesc, "!");
		int size = int(stardesc.size());
		if (size > 0)
			cs.stardesc = stardescs[0];
		if (size > 1)
			cs.startooltip = stardescs[1];
		if (add) {
			if (authoryear)
				class_cite_styles_[ENGINE_TYPE_AUTHORYEAR].push_back(cs);
			if (numerical)
				class_cite_styles_[ENGINE_TYPE_NUMERICAL].push_back(cs);
			if (defce)
				class_cite_styles_[ENGINE_TYPE_DEFAULT].push_back(cs);
		} else {
			if (authoryear)
				cite_styles_[ENGINE_TYPE_AUTHORYEAR].push_back(cs);
			if (numerical)
				cite_styles_[ENGINE_TYPE_NUMERICAL].push_back(cs);
			if (defce)
				cite_styles_[ENGINE_TYPE_DEFAULT].push_back(cs);
		}
	}
	// If we do AddToCiteEngine, do not apply yet,
	// except if we have already a style to add something to
	bool apply_ay = !add;
	bool apply_num = !add;
	bool apply_def = !add;
	if (add) {
		if (type & ENGINE_TYPE_AUTHORYEAR)
			apply_ay = !getCiteStyles(ENGINE_TYPE_AUTHORYEAR).empty();
		if (type & ENGINE_TYPE_NUMERICAL)
			apply_num = !getCiteStyles(ENGINE_TYPE_NUMERICAL).empty();
		if (type & ENGINE_TYPE_DEFAULT)
			apply_def = !getCiteStyles(ENGINE_TYPE_DEFAULT).empty();
	}

	// Add the styles from AddToCiteEngine to the class' styles
	// (but only if they are not yet defined)
	for (auto const & cis : class_cite_styles_) {
		// Only consider the current CiteEngineType
		if (!(type & cis.first))
			continue;
		for (auto const & ciss : cis.second) {
			bool defined = false;
			// Check if the style "name" is already def'ed
			for (auto const & av : getCiteStyles(cis.first))
				if (av.name == ciss.name)
					defined = true;
			if (!defined) {
				if (cis.first == ENGINE_TYPE_AUTHORYEAR && apply_ay)
					cite_styles_[ENGINE_TYPE_AUTHORYEAR].push_back(ciss);
				else if (cis.first == ENGINE_TYPE_NUMERICAL && apply_num)
					cite_styles_[ENGINE_TYPE_NUMERICAL].push_back(ciss);
				else if (cis.first == ENGINE_TYPE_DEFAULT && apply_def)
					cite_styles_[ENGINE_TYPE_DEFAULT].push_back(ciss);
			}
		}
	}
	if (type & ENGINE_TYPE_AUTHORYEAR && apply_ay)
		class_cite_styles_[ENGINE_TYPE_AUTHORYEAR].clear();
	if (type & ENGINE_TYPE_NUMERICAL && apply_num)
		class_cite_styles_[ENGINE_TYPE_NUMERICAL].clear();
	if (type & ENGINE_TYPE_DEFAULT && apply_def)
		class_cite_styles_[ENGINE_TYPE_DEFAULT].clear();
	return getout;
}


int TextClass::readCiteEngineType(Lexer & lexrc) const
{
	static_assert(ENGINE_TYPE_DEFAULT ==
				  (ENGINE_TYPE_AUTHORYEAR | ENGINE_TYPE_NUMERICAL),
				  "Incorrect default engine type");
	if (!lexrc.next()) {
		lexrc.printError("No cite engine type given for token: `$$Token'.");
		return ENGINE_TYPE_DEFAULT;
	}
	string const type = rtrim(lexrc.getString());
	if (compare_ascii_no_case(type, "authoryear") == 0)
		return ENGINE_TYPE_AUTHORYEAR;
	else if (compare_ascii_no_case(type, "numerical") == 0)
		return ENGINE_TYPE_NUMERICAL;
	else if (compare_ascii_no_case(type, "default") != 0) {
		string const s = "Unknown cite engine type `" + type
			+ "' given for token: `$$Token',";
		lexrc.printError(s);
	}
	return ENGINE_TYPE_DEFAULT;
}


bool TextClass::readCiteFormat(Lexer & lexrc, ReadType rt)
{
	int const type = readCiteEngineType(lexrc);
	string etype;
	string definition;
	// Cite engine definitions do not overwrite existing
	// definitions from the class or a module
	bool const overwrite = rt != CITE_ENGINE;
	while (lexrc.isOK()) {
		lexrc.next();
		etype = lexrc.getString();
		if (compare_ascii_no_case(etype, "end") == 0)
			break;
		if (!lexrc.isOK())
			return false;
		lexrc.eatLine();
		definition = lexrc.getString();
		char initchar = etype[0];
		if (initchar == '#')
			continue;
		if (initchar == '!' || initchar == '_' || prefixIs(etype, "B_")) {
			bool defined = false;
			bool aydefined = false;
			bool numdefined = false;
			// Check if the macro is already def'ed
			for (auto const & cm : cite_macros_) {
				if (!(type & cm.first))
					continue;
				if (cm.second.find(etype) != cm.second.end()) {
					if (type == cm.first)
						// defined as default or specific type
						defined = true;
					if (cm.first == ENGINE_TYPE_AUTHORYEAR)
						// defined for author-year
						aydefined = true;
					else if (cm.first == ENGINE_TYPE_NUMERICAL)
						// defined for numerical
						numdefined = true;
				}
			}
			if (!defined || overwrite) {
				if (type & ENGINE_TYPE_AUTHORYEAR && (type != ENGINE_TYPE_DEFAULT || !aydefined))
					cite_macros_[ENGINE_TYPE_AUTHORYEAR][etype] = definition;
				if (type & ENGINE_TYPE_NUMERICAL && (type != ENGINE_TYPE_DEFAULT || !numdefined))
					cite_macros_[ENGINE_TYPE_NUMERICAL][etype] = definition;
				if (type == ENGINE_TYPE_DEFAULT)
					cite_macros_[ENGINE_TYPE_DEFAULT][etype] = definition;
			}
		} else {
			bool defined = false;
			bool aydefined = false;
			bool numdefined = false;
			// Check if the format is already def'ed
			for (auto const & cm : cite_formats_) {
				if (!(type & cm.first))
					continue;
				if (cm.second.find(etype) != cm.second.end()) {
					if (type == cm.first)
						// defined as default or specific type
						defined = true;
					if (cm.first == ENGINE_TYPE_AUTHORYEAR)
						// defined for author-year
						aydefined = true;
					else if (cm.first == ENGINE_TYPE_NUMERICAL)
						// defined for numerical
						numdefined = true;
				}
			}
			if (!defined || overwrite){
				if (type & ENGINE_TYPE_AUTHORYEAR && (type != ENGINE_TYPE_DEFAULT || !aydefined))
					cite_formats_[ENGINE_TYPE_AUTHORYEAR][etype] = definition;
				if (type & ENGINE_TYPE_NUMERICAL && (type != ENGINE_TYPE_DEFAULT || !numdefined))
					cite_formats_[ENGINE_TYPE_NUMERICAL][etype] = definition;
				if (type == ENGINE_TYPE_DEFAULT)
					cite_formats_[ENGINE_TYPE_DEFAULT][etype] = definition;
			}
		}
	}
	return true;
}


bool TextClass::readFloat(Lexer & lexrc)
{
	enum {
		FT_TYPE = 1,
		FT_NAME,
		FT_PLACEMENT,
		FT_EXT,
		FT_WITHIN,
		FT_STYLE,
		FT_LISTNAME,
		FT_USESFLOAT,
		FT_PREDEFINED,
		FT_HTMLSTYLE,
		FT_HTMLATTR,
		FT_HTMLTAG,
		FT_DOCBOOKATTR,
		FT_DOCBOOKTAG,
		FT_LISTCOMMAND,
		FT_REFPREFIX,
		FT_ALLOWED_PLACEMENT,
		FT_ALLOWS_SIDEWAYS,
		FT_ALLOWS_WIDE,
		FT_REQUIRES,
		FT_END
	};

	LexerKeyword floatTags[] = {
		{ "allowedplacement", FT_ALLOWED_PLACEMENT },
		{ "allowssideways", FT_ALLOWS_SIDEWAYS },
		{ "allowswide", FT_ALLOWS_WIDE },
		{ "docbookattr", FT_DOCBOOKATTR },
		{ "docbooktag", FT_DOCBOOKTAG },
		{ "end", FT_END },
		{ "extension", FT_EXT },
		{ "guiname", FT_NAME },
		{ "htmlattr", FT_HTMLATTR },
		{ "htmlstyle", FT_HTMLSTYLE },
		{ "htmltag", FT_HTMLTAG },
		{ "ispredefined", FT_PREDEFINED },
		{ "listcommand", FT_LISTCOMMAND },
		{ "listname", FT_LISTNAME },
		{ "numberwithin", FT_WITHIN },
		{ "placement", FT_PLACEMENT },
		{ "refprefix", FT_REFPREFIX },
		{ "requires", FT_REQUIRES },
		{ "style", FT_STYLE },
		{ "type", FT_TYPE },
		{ "usesfloatpkg", FT_USESFLOAT }
	};

	lexrc.pushTable(floatTags);

	string ext;
	string htmlattr;
	docstring htmlstyle;
	string htmltag;
	string docbookattr;
	string docbooktag;
	string listname;
	string listcommand;
	string name;
	string placement;
	string allowed_placement = "!htbpH";
	string refprefix;
	string style;
	string type;
	string within;
	string required;
	bool usesfloat = true;
	bool ispredefined = false;
	bool allowswide = true;
	bool allowssideways = true;

	bool getout = false;
	while (!getout && lexrc.isOK()) {
		int le = lexrc.lex();
		switch (le) {
		case Lexer::LEX_UNDEF:
			lexrc.printError("Unknown float tag `$$Token'");
			continue;
		default:
			break;
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
				listname = fl.listName();
				usesfloat = fl.usesFloatPkg();
				ispredefined = fl.isPredefined();
				listcommand = fl.listCommand();
				refprefix = fl.refPrefix();
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
		case FT_ALLOWED_PLACEMENT:
			lexrc.next();
			allowed_placement = lexrc.getString();
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
		case FT_LISTCOMMAND:
			lexrc.next();
			listcommand = lexrc.getString();
			break;
		case FT_REFPREFIX:
			lexrc.next();
			refprefix = lexrc.getString();
			break;
		case FT_LISTNAME:
			lexrc.next();
			listname = lexrc.getString();
			break;
		case FT_USESFLOAT:
			lexrc.next();
			usesfloat = lexrc.getBool();
			break;
		case FT_REQUIRES:
			lexrc.next();
			required = lexrc.getString();
			break;
		case FT_PREDEFINED:
			lexrc.next();
			ispredefined = lexrc.getBool();
			break;
		case FT_ALLOWS_SIDEWAYS:
			lexrc.next();
			allowssideways = lexrc.getBool();
			break;
		case FT_ALLOWS_WIDE:
			lexrc.next();
			allowswide = lexrc.getBool();
			break;
		case FT_HTMLATTR:
			lexrc.next();
			htmlattr = lexrc.getString();
			break;
		case FT_HTMLSTYLE:
			lexrc.next();
			htmlstyle = lexrc.getLongString(from_ascii("EndHTMLStyle"));
			break;
		case FT_HTMLTAG:
			lexrc.next();
			htmltag = lexrc.getString();
			break;
		case FT_DOCBOOKATTR:
			lexrc.next();
			docbookattr = lexrc.getString();
			break;
		case FT_DOCBOOKTAG:
			lexrc.next();
			docbooktag = lexrc.getString();
			break;
		case FT_END:
			getout = true;
			break;
		}
	}

	lexrc.popTable();

	// Here we have a full float if getout == true
	if (getout) {
		if (!usesfloat && listcommand.empty()) {
			// if this float uses the same auxfile as an existing one,
			// there is no need for it to provide a list command.
			bool found_ext = false;
			for (auto const & f : floatlist_) {
				if (f.second.ext() == ext) {
					found_ext = true;
					break;
				}
			}
			if (!found_ext)
				LYXERR0("The layout does not provide a list command " <<
			          "for the float `" << type << "'. LyX will " <<
			          "not be able to produce a float list.");
		}
		Floating fl(type, placement, ext, within, style, name,
			listname, listcommand, refprefix, allowed_placement,
			htmltag, htmlattr, htmlstyle, docbooktag, docbookattr,
			required, usesfloat, ispredefined, allowswide,
			allowssideways);
		floatlist_.newFloat(fl);
		// each float has its own counter
		counters_.newCounter(from_ascii(type), from_ascii(within),
				docstring(), docstring(),
				bformat(_("%1$s (Float)"), _(name)));
		// also define sub-float counters
		docstring const subtype = "sub-" + from_ascii(type);
		counters_.newCounter(subtype, from_ascii(type),
				"\\alph{" + subtype + "}", docstring(),
				 bformat(_("Sub-%1$s (Float)"), _(name)));
	}
	return getout;
}


bool TextClass::readOutlinerName(Lexer & lexrc)
{
	std::string type;
	docstring name;
	if (lexrc.next())
		type = lexrc.getString();
	else {
		lexrc.printError("No type given for OutlinerName: `$$Token'.");
		return false;
	}
	if (lexrc.next())
		name = lexrc.getDocString();
	else {
		lexrc.printError("No name given for OutlinerName: `$$Token'.");
		return false;
	}
	outliner_names_[type] = name;
    return true;
}


string const & TextClass::prerequisites(string const & sep) const
{
	if (contains(prerequisites_, ',')) {
		vector<string> const pres = getVectorFromString(prerequisites_);
		prerequisites_ = getStringFromVector(pres, sep);
	}
	return prerequisites_;
}


bool TextClass::hasLayout(docstring const & n) const
{
	docstring const name = n.empty() ? defaultLayoutName() : n;
	return getLayout(name) != nullptr;
}


bool TextClass::hasInsetLayout(docstring const & n) const
{
	if (n.empty())
		return false;
	InsetLayouts::const_iterator it = insetlayoutlist_.find(n);
	return it != insetlayoutlist_.end();
}


Layout const & TextClass::operator[](docstring const & name) const
{
	LATTEST(!name.empty());

	Layout const * c = getLayout(name);
	if (!c) {
		LYXERR0("We failed to find the layout '" << name
		       << "' in the layout list. You MUST investigate!");
		for (auto const & lay : *this)
			lyxerr  << " " << to_utf8(lay.name()) << endl;

		// We require the name to exist
		static const Layout dummy;
		LASSERT(false, return dummy);
	}

	return *c;
}


Layout & TextClass::operator[](docstring const & name)
{
	LATTEST(!name.empty());
	// Safe to continue, given what we do below.

	Layout * c = getLayout(name);
	if (!c) {
		LYXERR0("We failed to find the layout '" << to_utf8(name)
		       << "' in the layout list. You MUST investigate!");
		for (auto const & lay : *this)
			LYXERR0(" " << to_utf8(lay.name()));

		// we require the name to exist
		LATTEST(false);
		// we are here only in release mode
		layoutlist_.push_back(createBasicLayout(name, true));
		c = getLayout(name);
	}

	return *c;
}


bool TextClass::deleteLayout(docstring const & name)
{
	if (name == defaultLayoutName() || name == plainLayoutName())
		return false;

	LayoutList::iterator it =
		remove_if(layoutlist_.begin(), layoutlist_.end(),
			[name](const Layout &c) { return c.name() == name; });

	LayoutList::iterator const end = layoutlist_.end();
	bool const ret = (it != end);
	layoutlist_.erase(it, end);
	return ret;
}


bool TextClass::deleteInsetLayout(docstring const & name)
{
	return insetlayoutlist_.erase(name);
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
		       << to_utf8(makeDisplayPath(layout_file.absFileName()))
		       << "'\n(Check `" << name_
		       << "')\nCheck your installation and "
		          "try Options/Reconfigure..."
		       << endl;
	}

	return loaded_;
}


Layout const * TextClass::getLayout(docstring const & name) const
{
	LayoutList::const_iterator cit =
		find_if(begin(), end(),
			[name](const Layout &c) { return c.name() == name; });
	if (cit == layoutlist_.end())
		return nullptr;

	return &(*cit);
}


Layout * TextClass::getLayout(docstring const & name)
{
	LayoutList::iterator it =
		find_if(layoutlist_.begin(), layoutlist_.end(),
			[name](const Layout &c) { return c.name() == name; });
	if (it == layoutlist_.end())
		return nullptr;

	return &(*it);
}


bool DocumentClass::addLayoutIfNeeded(docstring const & n) const
{
	if (hasLayout(n))
		return false;

	layoutlist_.push_back(createBasicLayout(n, true));
	return true;
}


string DocumentClass::forcedLayouts() const
{
	ostringstream os;
	bool first = true;
	for (auto const & lay : *this) {
		if (lay.forcelocal > 0) {
			if (first) {
				os << "Format " << LAYOUT_FORMAT << '\n';
				first = false;
			}
			lay.write(os);
		}
	}
	return os.str();
}


InsetLayout const & DocumentClass::insetLayout(docstring const & name) const
{
	// FIXME The fix for the InsetLayout part of 4812 would be here:
	// Add the InsetLayout to the document class if it is not found.
	docstring n = name;
	InsetLayouts::const_iterator cen = insetlayoutlist_.end();
	while (!n.empty()) {
		InsetLayouts::const_iterator cit = insetlayoutlist_.lower_bound(n);
		if (cit != cen && cit->first == n) {
			if (cit->second.obsoleted_by().empty())
				return cit->second;
			n = cit->second.obsoleted_by();
			return insetLayout(n);
		}
		// If we have a generic prefix (e.g., "Note:"),
		// try if this one alone is found.
		size_t i = n.find(':');
		if (i == string::npos)
			break;
		n = n.substr(0, i);
	}
	// Layout "name" not found.
	return plainInsetLayout();
}


InsetLayout const & DocumentClass::plainInsetLayout() {
	static const InsetLayout plain_insetlayout_;
	return plain_insetlayout_;
}


docstring const & TextClass::defaultLayoutName() const
{
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
	static Layout * defaultLayout = nullptr;

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
		LATTEST(false);
	};
	return *defaultLayout;
}


DocumentClassPtr getDocumentClass(
		LayoutFile const & baseClass, LayoutModuleList const & modlist,
		string const & cengine, bool const clone)
{
	DocumentClassPtr doc_class =
	    DocumentClassPtr(new DocumentClass(baseClass));
	for (auto const & mod : modlist) {
		LyXModule * lm = theModuleList[mod];
		if (!lm) {
			docstring const msg =
						bformat(_("The module %1$s has been requested by\n"
						"this document but has not been found in the list of\n"
						"available modules. If you recently installed it, you\n"
						"probably need to reconfigure LyX.\n"), from_utf8(mod));
			if (!clone)
				frontend::Alert::warning(_("Module not available"), msg);
			continue;
		}
		if (!lm->isAvailable() && !clone) {
			docstring const prereqs = from_utf8(getStringFromVector(lm->prerequisites(), "\n\t"));
			docstring const msg =
				bformat(_("The module %1$s requires a package that is not\n"
					"available in your LaTeX installation, or a converter that\n"
					"you have not installed. LaTeX output may not be possible.\n"
					"Missing prerequisites:\n"
						"\t%2$s\n"
					"See section 3.1.2.3 (Modules) of the User's Guide for more information."),
				from_utf8(mod), prereqs);
			frontend::Alert::warning(_("Package not available"), msg, true);
		}
		FileName layout_file = libFileSearch("layouts", lm->getFilename());
		if (!doc_class->read(layout_file, TextClass::MODULE)) {
			docstring const msg =
						bformat(_("Error reading module %1$s\n"), from_utf8(mod));
			frontend::Alert::warning(_("Read Error"), msg);
		}
	}

	if (cengine.empty())
		return doc_class;

	LyXCiteEngine * ce = theCiteEnginesList[cengine];
	if (!ce) {
		docstring const msg =
					bformat(_("The cite engine %1$s has been requested by\n"
					"this document but has not been found in the list of\n"
					"available engines. If you recently installed it, you\n"
					"probably need to reconfigure LyX.\n"), from_utf8(cengine));
		if (!clone)
			frontend::Alert::warning(_("Cite Engine not available"), msg);
	} else if (!ce->isAvailable() && !clone) {
		docstring const prereqs = from_utf8(getStringFromVector(ce->prerequisites(), "\n\t"));
		docstring const msg =
			bformat(_("The cite engine %1$s requires a package that is not\n"
				"available in your LaTeX installation, or a converter that\n"
				"you have not installed. LaTeX output may not be possible.\n"
				"Missing prerequisites:\n"
					"\t%2$s\n"
				"See section 3.1.2.3 (Modules) of the User's Guide for more information."),
			from_utf8(cengine), prereqs);
		frontend::Alert::warning(_("Package not available"), msg, true);
	} else {
		FileName layout_file = libFileSearch("citeengines", ce->getFilename());
		if (!doc_class->read(layout_file, TextClass::CITE_ENGINE)) {
			docstring const msg =
						bformat(_("Error reading cite engine %1$s\n"), from_utf8(cengine));
			frontend::Alert::warning(_("Read Error"), msg);
		}
	}

	return doc_class;
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
	for (auto const & l : layoutlist_)
		if (l.latexname() == lay)
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


Layout const & DocumentClass::getTOCLayout() const
{
	// we're going to look for the layout with the minimum toclevel
	int minlevel = 1000;
	Layout const * lay = nullptr;
	for (auto const & l : *this) {
		int const level = l.toclevel;
		// we don't want Part or unnumbered sections
		if (level == Layout::NOT_IN_TOC || level < 0
			|| level >= minlevel || l.counter.empty())
			continue;
		lay = &l;
		minlevel = level;
	}
	if (lay)
		return *lay;
	// hmm. that is very odd, so we'll do our best.
	return operator[](defaultLayoutName());
}


Layout const & DocumentClass::htmlTOCLayout() const
{
	if (html_toc_section_.empty())
		html_toc_section_ = getTOCLayout().name();
	return operator[](html_toc_section_);
}


string const DocumentClass::getCiteFormat(CiteEngineType const & type,
	string const & entry, bool const punct, string const & fallback) const
{
	string default_format = "{%fullnames:author%[[%fullnames:author%, ]][[{%fullnames:editor%[[%fullnames:editor%, ed., ]]}]]}"
				"\"%title%\"{%journal%[[, {!<i>!}%journal%{!</i>!}]][[{%publisher%[[, %publisher%]]"
				"[[{%institution%[[, %institution%]]}]]}]]}{%year%[[ (%year%)]]}{%pages%[[, %pages%]]}";
	if (punct)
		default_format += ".";

	map<CiteEngineType, map<string, string> >::const_iterator itype = cite_formats_.find(type);
	if (itype == cite_formats_.end())
		return default_format;
	map<string, string>::const_iterator it = itype->second.find(entry);
	if (it == itype->second.end() && !fallback.empty())
		it = itype->second.find(fallback);
	if (it == itype->second.end())
		return default_format;
	if (punct)
		return it->second + ".";
	return it->second;
}


string const & DocumentClass::getCiteMacro(CiteEngineType const & type,
	string const & macro) const
{
	static string empty;
	map<CiteEngineType, map<string, string> >::const_iterator itype = cite_macros_.find(type);
	if (itype == cite_macros_.end())
		return empty;
	map<string, string>::const_iterator it = itype->second.find(macro);
	if (it == itype->second.end())
		return empty;
	return it->second;
}


vector<string> const DocumentClass::citeCommands(
	CiteEngineType const & type) const
{
	vector<CitationStyle> const styles = citeStyles(type);
	vector<string> cmds;
	for (auto const & cs : styles)
		cmds.push_back(cs.name);

	return cmds;
}


vector<CitationStyle> const & DocumentClass::citeStyles(
	CiteEngineType const & type) const
{
	static vector<CitationStyle> empty;
	map<CiteEngineType, vector<CitationStyle> >::const_iterator it = cite_styles_.find(type);
	if (it == cite_styles_.end())
		return empty;
	return it->second;
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
