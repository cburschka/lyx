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

#include "Color.h"
#include "Counters.h"
#include "Floating.h"
#include "FloatList.h"
#include "Layout.h"
#include "Lexer.h"
#include "Font.h"

#include "frontends/alert.h"

#include "support/debug.h"
#include "support/ExceptionMessage.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/os.h"

#include <sstream>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace {

class LayoutNamesEqual : public unary_function<LayoutPtr, bool> {
public:
	LayoutNamesEqual(docstring const & name)
		: name_(name)
	{}
	bool operator()(LayoutPtr const & c) const
	{
		return c->name() == name_;
	}
private:
	docstring name_;
};


int const FORMAT = 6;


bool layout2layout(FileName const & filename, FileName const & tempfile)
{
	FileName const script = libFileSearch("scripts", "layout2layout.py");
	if (script.empty()) {
		lyxerr << "Could not find layout conversion "
			  "script layout2layout.py." << endl;
		return false;
	}

	ostringstream command;
	command << os::python() << ' ' << quoteName(script.toFilesystemEncoding())
		<< ' ' << quoteName(filename.toFilesystemEncoding())
		<< ' ' << quoteName(tempfile.toFilesystemEncoding());
	string const command_str = command.str();

	LYXERR(Debug::TCLASS, "Running `" << command_str << '\'');

	cmd_ret const ret =
		runCommand(command_str);
	if (ret.first != 0) {
		lyxerr << "Could not run layout conversion "
			  "script layout2layout.py." << endl;
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
	}
	// shutup warning
	return string();
}

} // namespace anon


TextClass::TextClass(string const & fn, string const & cln,
			   string const & desc, bool texClassAvail )
	: name_(fn), latexname_(cln), description_(desc),
	  floatlist_(new FloatList), counters_(new Counters),
	  texClassAvail_(texClassAvail)
{
	modular_ = false;
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
	// a hack to make this available for translation
	// i'm sure there must be a better way (rgh)
	_("PlainLayout");
}


docstring const TextClass::emptylayout_ = from_ascii("PlainLayout");


InsetLayout TextClass::empty_insetlayout_;


bool TextClass::isTeXClassAvailable() const
{
	return texClassAvail_;
}


bool TextClass::readStyle(Lexer & lexrc, Layout & lay)
{
	LYXERR(Debug::TCLASS, "Reading style " << to_utf8(lay.name()));
	if (!lay.read(lexrc, *this)) {
		lyxerr << "Error parsing style `" << to_utf8(lay.name()) << '\'' << endl;
		return false;
	}
	// Resolve fonts
	lay.resfont = lay.font;
	lay.resfont.realize(defaultfont());
	lay.reslabelfont = lay.labelfont;
	lay.reslabelfont.realize(defaultfont());
	return true; // no errors
}


enum TextClassTags {
	TC_OUTPUTTYPE = 1,
	TC_INPUT,
	TC_STYLE,
	TC_DEFAULTSTYLE,
	TC_INSETLAYOUT,
	TC_ENVIRONMENT,
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
	TC_FORMAT
};


// Reads a textclass structure from file.
bool TextClass::read(FileName const & filename, ReadType rt)
{
	if (!filename.isReadableFile()) {
		lyxerr << "Cannot read layout file `" << filename << "'."
		       << endl;
		return false;
	}

	keyword_item textClassTags[] = {
		{ "classoptions",    TC_CLASSOPTIONS },
		{ "columns",         TC_COLUMNS },
		{ "counter",         TC_COUNTER },
		{ "defaultfont",     TC_DEFAULTFONT },
		{ "defaultstyle",    TC_DEFAULTSTYLE },
		{ "environment",     TC_ENVIRONMENT },
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
		{ "requires",        TC_REQUIRES },
		{ "rightmargin",     TC_RIGHTMARGIN },
		{ "secnumdepth",     TC_SECNUMDEPTH },
		{ "sides",           TC_SIDES },
		{ "style",           TC_STYLE },
		{ "titlelatexname",  TC_TITLELATEXNAME },
		{ "titlelatextype",  TC_TITLELATEXTYPE },
		{ "tocdepth",        TC_TOCDEPTH }
	};

	LYXERR(Debug::TCLASS, "Reading " + translateRT(rt) + ": " +
		to_utf8(makeDisplayPath(filename.absFilename())));

	// Define the `empty' layout used in table cells, ert, etc. Note that 
	// we do this before loading any layout file, so that classes can 
	// override features of this layout if they should choose to do so.
	if (rt == BASECLASS) {
		static char const * s = "Margin Static\n"
			"LatexType Paragraph\n"
			"LatexName dummy\n"
			"Align Block\n"
			"AlignPossible Left, Right, Center\n"
			"LabelType No_Label\n"
			"End";
		istringstream ss(s);
		Lexer lex(textClassTags, sizeof(textClassTags) / sizeof(textClassTags[0]));
		lex.setStream(ss);
		Layout lay;
		lay.setName(emptylayout_);
		if (!readStyle(lex, lay)) {
			// The only way this happens is because the hardcoded layout above
			// is wrong.
			BOOST_ASSERT(false);
		}
		layoutlist_.push_back(boost::shared_ptr<Layout>(new Layout(lay)));
	}

	Lexer lexrc(textClassTags,
		sizeof(textClassTags) / sizeof(textClassTags[0]));

	lexrc.setFile(filename);
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

		case TC_ENVIRONMENT:
		case TC_STYLE:
			if (lexrc.next()) {
				docstring const name = from_utf8(subst(lexrc.getString(),
						    '_', ' '));
				if (name.empty()) {
					string s = "Could not read name for style: `$$Token' "
						+ lexrc.getString() + " is probably not valid UTF-8!";
					lexrc.printError(s.c_str());
					Layout lay;
					//FIXME If we're just dropping this layout, do we really
					//care whether there's an error?? Or should we just set
					//error to true, since we couldn't even read the name?
					error = !readStyle(lexrc, lay);
				} else if (hasLayout(name)) {
					Layout * lay = operator[](name).get();
					error = !readStyle(lexrc, *lay);
				} else {
					Layout lay;
					lay.setName(name);
					if (le == TC_ENVIRONMENT)
						lay.is_environment = true;
					error = !readStyle(lexrc, lay);
					if (!error)
						layoutlist_.push_back(boost::shared_ptr<Layout>(new Layout(lay)));

					if (defaultlayout_.empty()) {
						// We do not have a default layout yet, so we choose
						// the first layout we encounter.
						defaultlayout_ = name;
					}
				}
			}
			else {
				//FIXME Should we also eat the style here? viz:
				//Layout lay;
				//readStyle(lexrc, lay);
				//as above...
				lexrc.printError("No name given for style: `$$Token'.");
				error = true;
			}
			break;

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

		case TC_LEFTMARGIN:	// left margin type
			if (lexrc.next())
				leftmargin_ = lexrc.getDocString();
			break;

		case TC_RIGHTMARGIN:	// right margin type
			if (lexrc.next())
				rightmargin_ = lexrc.getDocString();
			break;

		case TC_INSETLAYOUT:
			if (lexrc.next()) {
				InsetLayout il;
				if (il.read(lexrc)) {
					insetlayoutlist_[il.name()] = il;
				}
				// else there was an error, so forget it
			}
			break;

		case TC_FLOAT:
			readFloat(lexrc);
			break;

		case TC_COUNTER:
			readCounter(lexrc);
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
				floatlist_->erase(nofloat);
			}
			break;
		}

		//Note that this is triggered the first time through the loop unless
		//we hit a format tag.
		if (format != FORMAT)
			break;
	}

	if (format != FORMAT) {
		LYXERR(Debug::TCLASS, "Converting layout file from format "
				      << format << " to " << FORMAT);
		FileName const tempfile = FileName::tempName();
		error = !layout2layout(filename, tempfile);
		if (!error)
			error = read(tempfile, rt);
		tempfile.removeFile();
		return !error;
	}

	LYXERR(Debug::TCLASS, "Finished reading " + translateRT(rt) + ": " +
			to_utf8(makeDisplayPath(filename.absFilename())));

	if (rt != BASECLASS) 
		return !error;

	if (defaultlayout_.empty()) {
		lyxerr << "Error: Textclass '" << name_
						<< "' is missing a defaultstyle." << endl;
		error = true;
	}
		
	//Try to erase "stdinsets" from the provides_ set. 
	//The
	//  Provides stdinsets 1
	//declaration simply tells us that the standard insets have been
	//defined. (It's found in stdinsets.inc but could also be used in
	//user-defined files.) There isn't really any such package. So we
	//might as well go ahead and erase it.
	//If we do not succeed, then it was not there, which means that
	//the textclass did not provide the definitions of the standard
	//insets. So we need to try to load them.
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
	const_iterator cit = begin();
	const_iterator the_end = end();
	for ( ; cit != the_end ; ++cit) {
		int const toclevel = (*cit)->toclevel;
		if (toclevel != Layout::NOT_IN_TOC) {
			if (min_toclevel_ == Layout::NOT_IN_TOC)
				min_toclevel_ = toclevel;
			else
				min_toclevel_ = min(min_toclevel_,
							toclevel);
			max_toclevel_ = max(max_toclevel_,
							toclevel);
		}
	}
	LYXERR(Debug::TCLASS, "Minimum TocLevel is " << min_toclevel_
		<< ", maximum is " << max_toclevel_);

	return !error;
}


void TextClass::readTitleType(Lexer & lexrc)
{
	keyword_item titleTypeTags[] = {
		{ "commandafter", TITLE_COMMAND_AFTER },
		{ "environment", TITLE_ENVIRONMENT }
	};

	PushPopHelper pph(lexrc, titleTypeTags, TITLE_ENVIRONMENT);

	int le = lexrc.lex();
	switch (le) {
	case Lexer::LEX_UNDEF:
		lexrc.printError("Unknown output type `$$Token'");
		return;
	case TITLE_COMMAND_AFTER:
	case TITLE_ENVIRONMENT:
		titletype_ = static_cast<TitleLatexType>(le);
		break;
	default:
		lyxerr << "Unhandled value " << le
		       << " in TextClass::readTitleType." << endl;

		break;
	}
}


void TextClass::readOutputType(Lexer & lexrc)
{
	keyword_item outputTypeTags[] = {
		{ "docbook", DOCBOOK },
		{ "latex", LATEX },
		{ "literate", LITERATE }
	};

	PushPopHelper pph(lexrc, outputTypeTags, LITERATE);

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
		lyxerr << "Unhandled value " << le
		       << " in TextClass::readOutputType." << endl;

		break;
	}
}


enum ClassOptionsTags {
	CO_FONTSIZE = 1,
	CO_PAGESTYLE,
	CO_OTHER,
	CO_HEADER,
	CO_END
};


void TextClass::readClassOptions(Lexer & lexrc)
{
	keyword_item classOptionsTags[] = {
		{"end", CO_END },
		{"fontsize", CO_FONTSIZE },
		{"header", CO_HEADER },
		{"other", CO_OTHER },
		{"pagestyle", CO_PAGESTYLE }
	};

	lexrc.pushTable(classOptionsTags, CO_END);
	bool getout = false;
	while (!getout && lexrc.isOK()) {
		int le = lexrc.lex();
		switch (le) {
		case Lexer::LEX_UNDEF:
			lexrc.printError("Unknown ClassOption tag `$$Token'");
			continue;
		default: break;
		}
		switch (static_cast<ClassOptionsTags>(le)) {
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


enum FloatTags {
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


void TextClass::readFloat(Lexer & lexrc)
{
	keyword_item floatTags[] = {
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

	lexrc.pushTable(floatTags, FT_END);

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
		switch (static_cast<FloatTags>(le)) {
		case FT_TYPE:
			lexrc.next();
			type = lexrc.getString();
			if (floatlist_->typeExist(type)) {
				Floating const & fl = floatlist_->getType(type);
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
		floatlist_->newFloat(fl);
		// each float has its own counter
		counters_->newCounter(from_ascii(type), from_ascii(within), 
				      docstring(), docstring());
	}

	lexrc.popTable();
}


enum CounterTags {
	CT_NAME = 1,
	CT_WITHIN,
	CT_LABELSTRING,
	CT_LABELSTRING_APPENDIX,
	CT_END
};


void TextClass::readCounter(Lexer & lexrc)
{
	keyword_item counterTags[] = {
		{ "end", CT_END },
		{ "labelstring", CT_LABELSTRING },
		{ "labelstringappendix", CT_LABELSTRING_APPENDIX },
		{ "name", CT_NAME },
		{ "within", CT_WITHIN }
	};

	lexrc.pushTable(counterTags, CT_END);

	docstring name;
	docstring within;
	docstring labelstring;
	docstring labelstring_appendix;

	bool getout = false;
	while (!getout && lexrc.isOK()) {
		int le = lexrc.lex();
		switch (le) {
		case Lexer::LEX_UNDEF:
			lexrc.printError("Unknown counter tag `$$Token'");
			continue;
		default: break;
		}
		switch (static_cast<CounterTags>(le)) {
		case CT_NAME:
			lexrc.next();
			name = lexrc.getDocString();
			if (counters_->hasCounter(name))
				LYXERR(Debug::TCLASS, "Reading existing counter " << to_utf8(name));
			else
				LYXERR(Debug::TCLASS, "Reading new counter " << to_utf8(name));
			break;
		case CT_WITHIN:
			lexrc.next();
			within = lexrc.getDocString();
			if (within == "none")
				within.erase();
			break;
		case CT_LABELSTRING:
			lexrc.next();
			labelstring = lexrc.getDocString();
			labelstring_appendix = labelstring;
			break;
		case CT_LABELSTRING_APPENDIX:
			lexrc.next();
			labelstring_appendix = lexrc.getDocString();
			break;
		case CT_END:
			getout = true;
			break;
		}
	}

	// Here if have a full counter if getout == true
	if (getout)
		counters_->newCounter(name, within, 
				      labelstring, labelstring_appendix);

	lexrc.popTable();
}


FontInfo const & TextClass::defaultfont() const
{
	return defaultfont_;
}


docstring const & TextClass::leftmargin() const
{
	return leftmargin_;
}


docstring const & TextClass::rightmargin() const
{
	return rightmargin_;
}


bool TextClass::hasLayout(docstring const & n) const
{
	docstring const name = n.empty() ? defaultLayoutName() : n;

	return find_if(layoutlist_.begin(), layoutlist_.end(),
		       LayoutNamesEqual(name))
		!= layoutlist_.end();
}



LayoutPtr const & TextClass::operator[](docstring const & name) const
{
	BOOST_ASSERT(!name.empty());

	LayoutList::const_iterator cit =
		find_if(layoutlist_.begin(),
			layoutlist_.end(),
			LayoutNamesEqual(name));

	if (cit == layoutlist_.end()) {
		lyxerr << "We failed to find the layout '" << to_utf8(name)
		       << "' in the layout list. You MUST investigate!"
		       << endl;
		for (LayoutList::const_iterator it = layoutlist_.begin();
			 it != layoutlist_.end(); ++it)
			lyxerr  << " " << to_utf8(it->get()->name()) << endl;

		// we require the name to exist
		BOOST_ASSERT(false);
	}

	return *cit;
}


bool TextClass::deleteLayout(docstring const & name)
{
	if (name == defaultLayoutName() || name == emptyLayoutName())
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
	FileName layout_file;
	if (!path.empty())
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


FloatList & TextClass::floats()
{
	return *floatlist_.get();
}


FloatList const & TextClass::floats() const
{
	return *floatlist_.get();
}


Counters & TextClass::counters() const
{
	return *counters_.get();
}


// Return the layout object of an inset given by name. If the name
// is not found as such, the part after the ':' is stripped off, and
// searched again. In this way, an error fallback can be provided:
// An erroneous 'CharStyle:badname' (e.g., after a documentclass switch)
// will invoke the layout object defined by name = 'CharStyle'.
// If that doesn't work either, an empty object returns (shouldn't
// happen).  -- Idea JMarc, comment MV
InsetLayout const & TextClass::insetlayout(docstring const & name) const 
{
	docstring n = name;
	while (!n.empty()) {
		if (insetlayoutlist_.count(n) > 0)
			return insetlayoutlist_[n];
		docstring::size_type i = n.find(':');
		if (i == string::npos)
			break;
		n = n.substr(0,i);
	}
	return empty_insetlayout_;
}


docstring const & TextClass::defaultLayoutName() const
{
	// This really should come from the actual layout... (Lgb)
	return defaultlayout_;
}


LayoutPtr const & TextClass::defaultLayout() const
{
	return operator[](defaultLayoutName());
}


string const & TextClass::name() const
{
	return name_;
}


string const & TextClass::latexname() const
{
	const_cast<TextClass*>(this)->load();
	return latexname_;
}


string const & TextClass::description() const
{
	return description_;
}


string const & TextClass::opt_fontsize() const
{
	return opt_fontsize_;
}


string const & TextClass::opt_pagestyle() const
{
	return opt_pagestyle_;
}


string const & TextClass::options() const
{
	return options_;
}


string const & TextClass::class_header() const
{
	return class_header_;
}


string const & TextClass::pagestyle() const
{
	return pagestyle_;
}


docstring const & TextClass::preamble() const
{
	return preamble_;
}


PageSides TextClass::sides() const
{
	return sides_;
}


int TextClass::secnumdepth() const
{
	return secnumdepth_;
}


int TextClass::tocdepth() const
{
	return tocdepth_;
}


OutputType TextClass::outputType() const
{
	return outputType_;
}


bool TextClass::provides(string const & p) const
{
	return provides_.find(p) != provides_.end();
}


unsigned int TextClass::columns() const
{
	return columns_;
}


TitleLatexType TextClass::titletype() const
{
	return titletype_;
}


string const & TextClass::titlename() const
{
	return titlename_;
}


int TextClass::size() const
{
	return layoutlist_.size();
}


int TextClass::min_toclevel() const
{
	return min_toclevel_;
}


int TextClass::max_toclevel() const
{
	return max_toclevel_;
}


bool TextClass::hasTocLevels() const
{
	return min_toclevel_ != Layout::NOT_IN_TOC;
}


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
