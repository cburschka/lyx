/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995-2002 the LyX Team.
 *           
 * \author Baruch Even
 * \author Herbert Voss <voss@lyx.org>
 * ====================================================== */

/*
Known BUGS:
    
    * If the image is from the clipart, and the document is moved to another
       directory, the user is screwed. Need a way to handle it.
       This amounts to a problem of when to use relative or absolute file paths
       We should probably use what the user asks to use... but when he chooses
       by the file dialog we normally get an absolute path and this may not be 
       what the user meant.
       [Note that browseRelFile in helper_funcs.* provides a file name
        which is relative if it is at reference path (here puffer path)
        level or below, and an absolute path if the file name is not a
        `natural' relative file name. In any case,
            MakeAbsPath(filename, buf->filePath())
        is guaranteed to provide the correct absolute path. This is what is
        done know for include insets. Feel free to ask me -- JMarc
	14/01/2002]
	
	* If we are trying to create a file in a read-only directory and there
		are graphics that need converting, the converting will fail because
		it is done in-place, into the same directory as the original image.
		This needs to be fixed in the src/converter.C file
		[ This is presumed to be fixed, needs testing.]

	* We do not dither or resize the image in a WYSIWYM way, we load it at
		its original size and color, resizing is done in the final output,
		but not in the LyX window.

	* EPS figures are not fully detected, they may have a lot of possible
		suffixes so we need to read the file and detect if it's EPS or not.
		[Implemented, need testing]
		
TODO Before initial production release:
    * Replace insetfig everywhere
        * Search for comments of the form
            // INSET_GRAPHICS: remove this when InsetFig is thrown.
          And act upon them. Make sure not to remove InsetFig code for the 
		  1.2.0 release, only afterwards, after deployment shows InsetGraphics
		  to be ok.
        * What advanced features the users want to do?
            Implement them in a non latex dependent way, but a logical way.
            LyX should translate it to latex or any other fitting format.
    * Add a way to roll the image file into the file format.
    * When loading, if the image is not found in the expected place, try
       to find it in the clipart, or in the same directory with the image.
    * Keep a tab on the image file, if it changes, update the lyx view.
	* The image choosing dialog could show thumbnails of the image formats
	  it knows of, thus selection based on the image instead of based on
	  filename.
	* Add support for the 'picins' package.
	* Add support for the 'picinpar' package.
	* Improve support for 'subfigure' - Allow to set the various options
		that are possible.
 */

/* NOTES:
 * Fileformat:
 * Current version is 1 (inset file format version), when changing it
 * it should be changed in the Write() function when writing in one place
 * and when reading one should change the version check and the error message.
 * The filename is kept in  the lyx file in a relative way, so as to allow
 * moving the document file and its images with no problem.
 * 
 *
 * Conversions:
 *   Postscript output means EPS figures.
 *
 *   PDF output is best done with PDF figures if it's a direct conversion
 *   or PNG figures otherwise.
 *   	Image format
 *   	from        to
 *   	EPS         epstopdf
 *   	JPG/PNG     direct
 *   	PDF         direct
 *   	others      PNG
 */

#include <config.h> 

#ifdef __GNUG__
#pragma implementation
#endif 

#include "insets/insetgraphics.h"
#include "insets/insetgraphicsParams.h"
#include "graphics/GraphicsCache.h"
#include "graphics/GraphicsCacheItem.h"

#include "frontends/Dialogs.h"
#include "LyXView.h"
#include "buffer.h"
#include "BufferView.h"
#include "converter.h"
#include "frontends/support/LyXImage.h"
#include "Painter.h"
#include "lyx_gui_misc.h"
#include "support/FileInfo.h"
#include "support/filetools.h"
#include "frontends/controllers/helper_funcs.h"
#include "support/lyxlib.h"
#include "lyxtext.h"
#include "lyxrc.h"
#include "font.h" // For the lyxfont class.
#include "fstream" // for ifstream in isEPS
#include <algorithm> // For the std::max
#include "support/lyxmanip.h"
#include "debug.h"
#include "gettext.h"

extern string system_tempdir;

using std::ifstream;
using std::ostream;
using std::endl;

///////////////////////////////////////////////////////////////////////////
int VersionNumber = 1;
///////////////////////////////////////////////////////////////////////////

// This function is a utility function
// ... that should be with ChangeExtension ...
inline
string const RemoveExtension(string const & filename)
{
	return ChangeExtension(filename, string());
}


// Initialize only those variables that do not have a constructor.
InsetGraphics::InsetGraphics()
	: cacheHandle(0), imageLoaded(false)
{}


InsetGraphics::InsetGraphics(InsetGraphics const & ig, bool same_id)
	: Inset(), SigC::Object()
	, cacheHandle(ig.cacheHandle)
	, imageLoaded(ig.imageLoaded)
{
	setParams(ig.getParams());
	if (same_id)
		id_ = ig.id_;
}


InsetGraphics::~InsetGraphics()
{
	// Emits the hide signal to the dialog connected (if any)
	hideDialog();
}


string const
InsetGraphics::statusMessage() const
{
	string msg;
	if (cacheHandle.get()) {
		switch (cacheHandle->getImageStatus()) {
		case GraphicsCacheItem::UnknownError:
			msg = _("Unknown Error");
			break;
		case GraphicsCacheItem::Loading:
			msg = _("Loading...");
			break;
		case GraphicsCacheItem::ErrorReading:
			msg = _("Error reading");
			break;
		case GraphicsCacheItem::Converting:
			msg = _("Converting Image");
			break;
		case GraphicsCacheItem::ErrorConverting:
			msg = _("Error converting");
			break;
		case GraphicsCacheItem::Loaded:
			// No message to write.
			break;
		}
	}
	return msg;
}


int InsetGraphics::ascent(BufferView *, LyXFont const &) const
{
	LyXImage * pixmap = 0;
	if (cacheHandle.get() && (pixmap = cacheHandle->getImage()))
		return pixmap->getHeight();
	else
		return 50;
}


int InsetGraphics::descent(BufferView *, LyXFont const &) const
{
	// this is not true if viewport is used and clip is not.
	return 0;
}


int InsetGraphics::width(BufferView *, LyXFont const & font) const
{
	LyXImage * pixmap = 0;
	
	if (cacheHandle.get() && (pixmap = cacheHandle->getImage()))
		return pixmap->getWidth();
	else {
		int font_width = 0;

		LyXFont msgFont(font);
		msgFont.setFamily(LyXFont::SANS_FAMILY);

		string const justname = OnlyFilename (params.filename);
		if (!justname.empty()) {
			msgFont.setSize(LyXFont::SIZE_FOOTNOTE);
			font_width = lyxfont::width(justname, msgFont);
		}

		string const msg = statusMessage();
		if (!msg.empty()) {
			msgFont.setSize(LyXFont::SIZE_TINY);
			int const msg_width = lyxfont::width(msg, msgFont);
			font_width = std::max(font_width, msg_width);
		}
		
		return std::max(50, font_width + 15);
	}
}


void InsetGraphics::draw(BufferView * bv, LyXFont const & font,
                         int baseline, float & x, bool) const
{
	Painter & paint = bv->painter();

	int ldescent = descent(bv, font);
	int lascent = ascent(bv, font);
	int lwidth = width(bv, font);

	// Make sure x is updated upon exit from this routine
	int old_x = int(x);
	x += lwidth;

	// This will draw the graphics. If the graphics has not been loaded yet,
	// we draw just a rectangle.
	if (imageLoaded) {

		paint.image(old_x + 2, baseline - lascent,
		            lwidth - 4, lascent + ldescent,
		            cacheHandle->getImage());
	} else {
		
		// Get the image status, default to unknown error.
		GraphicsCacheItem::ImageStatus status = GraphicsCacheItem::UnknownError;
		if (lyxrc.display_graphics != "no" && lyxrc.use_gui
		    && params.display != InsetGraphicsParams::NONE &&
		    cacheHandle.get())
			status = cacheHandle->getImageStatus();
		
		// Check if the image is now ready.
		if (status == GraphicsCacheItem::Loaded) {
			imageLoaded = true;

			// Tell BufferView we need to be updated!
			bv->text->status(bv, LyXText::CHANGED_IN_DRAW);
			return;
		}

		paint.rectangle(old_x + 2, baseline - lascent,
		                lwidth - 4,
		                lascent + ldescent);

		// Print the file name.
		LyXFont msgFont(font);
		msgFont.setFamily(LyXFont::SANS_FAMILY);

		string const justname = OnlyFilename (params.filename);
		if (!justname.empty()) {
			msgFont.setSize(LyXFont::SIZE_FOOTNOTE);
			paint.text(old_x + 8, 
				   baseline - lyxfont::maxAscent(msgFont) - 4,
				   justname, msgFont);
		}

		// Print the message.
		string const msg = statusMessage();
		if (!msg.empty()) {
			msgFont.setSize(LyXFont::SIZE_TINY);
			paint.text(old_x + 8, baseline - 4, msg, msgFont);
		}
	}
}


void InsetGraphics::edit(BufferView *bv, int, int, unsigned int)
{
	bv->owner()->getDialogs()->showGraphics(this);
}


void InsetGraphics::edit(BufferView * bv, bool)
{
	edit(bv, 0, 0, 0);
}


Inset::EDITABLE InsetGraphics::editable() const
{
	return IS_EDITABLE;
}


void InsetGraphics::write(Buffer const * buf, ostream & os) const
{
	os << "Graphics FormatVersion " << VersionNumber << '\n';
	params.Write(buf, os);
}


void InsetGraphics::read(Buffer const * buf, LyXLex & lex)
{
	string const token = lex.getString();

	if (token == "Graphics")
		readInsetGraphics(buf, lex);
	else if (token == "Figure") // Compatibility reading of FigInset figures.
		readFigInset(buf, lex);
	else
		lyxerr[Debug::INFO] << "Not a Graphics or Figure inset!\n";

	updateInset();
}

void InsetGraphics::readInsetGraphics(Buffer const * buf, LyXLex & lex)
{
	bool finished = false;

	while (lex.isOK() && !finished) {
		lex.next();

		string const token = lex.getString();
		lyxerr[Debug::INFO] << "Token: '" << token << '\'' 
				    << std::endl;

		if (token.empty()) {
			continue;
		} else if (token == "\\end_inset") {
			finished = true;
		} else if (token == "FormatVersion") {
			lex.next();
			int version = lex.getInteger();
			if (version > VersionNumber)
				lyxerr
				<< "This document was created with a newer Graphics widget"
				", You should use a newer version of LyX to read this"
				" file."
				<< std::endl;
			// TODO: Possibly open up a dialog?
		}
		else {
			if (! params.Read(buf, lex, token))
				lyxerr << "Unknown token, " << token << ", skipping." 
					<< std::endl;
		}
	}
}


void InsetGraphics::readFigInset(Buffer const * buf, LyXLex & lex)
{
	std::vector<string> const oldUnits =
		getVectorFromString("pt,cm,in,p%,c%");
	bool finished = false;
	
	while (lex.isOK() && !finished) {
		lex.next();

		string const token = lex.getString();
		lyxerr[Debug::INFO] << "Token: " << token << endl;
		
		if (token.empty())
			continue;
		else if (token == "\\end_inset") {
			finished = true;
		} else if (token == "file") {
			if (lex.next()) {
				string const name = lex.getString();
				string const path = buf->filePath();
				params.filename = MakeAbsPath(name, path);
			}
		} else if (token == "extra") {
			if (lex.next());
			// kept for backwards compability. Delete in 0.13.x
		} else if (token == "subcaption") {
			if (lex.eatLine())
				params.subcaptionText = lex.getString();
			params.subcaption = true;
		} else if (token == "label") {
			if (lex.next());
			// kept for backwards compability. Delete in 0.13.x
		} else if (token == "angle") {
			if (lex.next())
				params.rotateAngle = lex.getFloat();
		} else if (token == "size") {
			if (lex.next())
				params.lyxwidth = LyXLength(lex.getString()+"pt");
			if (lex.next())
				params.lyxheight = LyXLength(lex.getString()+"pt");
		} else if (token == "flags") {
			InsetGraphicsParams::DisplayType tmp = InsetGraphicsParams::COLOR;
			if (lex.next())
				switch (lex.getInteger()) {
				case 1: tmp = InsetGraphicsParams::MONOCHROME; break;
				case 2: tmp = InsetGraphicsParams::GRAYSCALE; break;
				}
			params.display = tmp;
		} else if (token == "subfigure") {
			params.subcaption = true;
		} else if (token == "width") {
		    if (lex.next()) {
			int i = lex.getInteger();
			if (lex.next()) {
			    if (i == 5) {
				params.scale = lex.getInteger();
				params.size_type = InsetGraphicsParams::SCALE;
			    } else {
				params.width = LyXLength(lex.getString()+oldUnits[i]);
				params.size_type = InsetGraphicsParams::WH;
			    }
			}
		    }
		} else if (token == "height") {
		    if (lex.next()) {
			int i = lex.getInteger();
			if (lex.next()) {
			    params.height = LyXLength(lex.getString()+oldUnits[i]);
			    params.size_type = InsetGraphicsParams::WH;
			}
		    }
		}
	}
}

string const InsetGraphics::createLatexOptions() const
{
	// Calculate the options part of the command, we must do it to a string
	// stream since we might have a trailing comma that we would like to remove
	// before writing it to the output stream.
	ostringstream options;
	if (!params.bb.empty())
	    options << "bb=" << strip(params.bb) << ',';
	if (params.draft)
	    options << "%\n  draft,";
	if (params.clip)
	    options << "%\n  clip,";
	if (params.size_type == InsetGraphicsParams::WH) {
	    if (!params.width.zero())
		options << "%\n  width=" << params.width.asLatexString() << ',';
	    if (!params.height.zero())
		options << "%\n  height=" << params.height.asLatexString() << ',';
	} else if (params.size_type == InsetGraphicsParams::SCALE) {
	    if (params.scale > 0)
		options << "%\n  scale=" << double(params.scale)/100.0 << ',';
	}
	if (params.keepAspectRatio)
	    options << "%\n  keepaspectratio,";
	// Make sure it's not very close to zero, a float can be effectively
	// zero but not exactly zero.
	if (!lyx::float_equal(params.rotateAngle, 0, 0.001)) {
	    options << "%\n  angle=" << params.rotateAngle << ',';
	    if (!params.rotateOrigin.empty()) {
		options << "%\n  origin=";
		options << params.rotateOrigin[0];
		if (contains(params.rotateOrigin,"Top"))
		    options << 't';
		else if (contains(params.rotateOrigin,"Bottom"))
		    options << 'b';
		else if (contains(params.rotateOrigin,"Baseline"))
		    options << 'B';
		options << ',';
	    }
	}
	if (!params.special.empty())
	    options << params.special << ',';
	string opts = options.str().c_str();
	opts = strip(opts, ',');
	return opts;
}

namespace {

enum FileType {
	EPS,
	PNG,
	JPEG,
	GIF,
	PDF,
	UNKNOWN
};

bool isEPS(string const & filename)
{
	if (filename.empty() || !IsFileReadable(filename)) return false;

	ifstream ifs(filename.c_str());

	if (!ifs) return false;	// Couldn't open file...

	bool is_eps = false; // Have we recognized the file as EPS?
	string to_find = "%!PS-Adobe-"; // The string we use to recognize
	int const max_attempts = 500; // Maximum strings to read to attempt recognition
	int count = 0; // Counter of attempts.
	string str;
	for (; count < max_attempts; ++count) {
		if (ifs.eof()) {
			lyxerr[Debug::INFO] << "InsetGraphics (isEPS)"
				" End of file reached and it wasn't found to be EPS!" << endl;
			break;
		}

		ifs >> str;
		if (str.find(to_find)) {
			is_eps = true;
			break;
		}
	}

	return is_eps;
}

enum FileType classifyFileType(string const & filename, string const & suffix)
{
	if (suffix == "png")
		return PNG;
	else if (suffix == "jpg" || suffix == "jpeg")
		return JPEG;
	else if (suffix == "gif")
		return GIF;
	else if (suffix == "pdf")
		return PDF;
	else if (isEPS(filename))
		return EPS;

	return UNKNOWN;
}

string decideOutputImageFormat(string const & suffix, enum FileType type)
{
	// lyxrc.pdf_mode means:
	// Are we creating a PDF or a PS file?
	// (Should actually mean, are we using latex or pdflatex).	
	if (lyxrc.pdf_mode) {
		if (type == EPS || type == EPS || type == PDF)
			return "pdf";
		else if (type == JPEG)
			return suffix;
		else
			return "png";
	}

	// If it's postscript, we always do eps.
	// There are many suffixes that are actually EPS (ask Garst for example)
	// so we detect if it's an EPS by looking in the file, if it is, we return
	// the same suffix of the file so it won't be converted.
	if (type == EPS)
		return suffix;
	
	return "eps";
}

} // Anon. namespace

string const InsetGraphics::prepareFile(Buffer const *buf) const
{
	// do_convert = Do we need to convert the file?
	// nice = Do we create a nice version?
	//        This is used when exporting the latex file only.
	// 
	// if (!do_convert)
	//   return original filename
	// 
	// if (!nice)
	//   convert_place = temp directory
	//   return new filename in temp directory
	// else
	//   convert_place = original file directory
	//   return original filename without the extension
	//
	
	// Get the extension (format) of the original file.
	string const extension = GetExtension(params.filename);
	FileType type = classifyFileType(params.filename, extension);
	
	// Are we creating a PDF or a PS file?
	// (Should actually mean, are we usind latex or pdflatex).
	string const image_target = decideOutputImageFormat(extension, type);

	if (extension == image_target)
		return params.filename;

	string outfile;
	if (!buf->niceFile) {
		string const temp = AddName(buf->tmppath, params.filename);
		outfile = RemoveExtension(temp);
		
		//lyxerr << "buf::tmppath = " << buf->tmppath << "\n";
		//lyxerr << "filename = " << params.filename << "\n";
		//lyxerr << "temp = " << temp << "\n";
		//lyxerr << "outfile = " << outfile << endl;
	} else {
		string const path = buf->filePath();
		string const relname = MakeRelPath(params.filename, path);
		outfile = RemoveExtension(relname);
	}
	converters.convert(buf, params.filename, outfile, extension, image_target);
	return outfile;
}


int InsetGraphics::latex(Buffer const *buf, ostream & os,
			 bool /*fragile*/, bool/*fs*/) const
{
	// If there is no file specified, just output a message about it in
	// the latex output.
	if (params.filename.empty()) {
		os  << "\\fbox{\\rule[-0.5in]{0pt}{1in}"
			<< _("empty figure path") << "}\n";
		return 1; // One end of line marker added to the stream.
	}
	// Keep count of newlines that we issued.
	int newlines = 0;
	// This variables collect all the latex code that should be before and
	// after the actual includegraphics command.
	string before;
	string after;
	// Do we want subcaptions?
	if (params.subcaption) {
		before += "\\subfigure[" + params.subcaptionText + "]{";
		after = '}' + after;
	}
	// We never use the starred form, we use the "clip" option instead.
	os << before << "\\includegraphics";
	// Write the options if there are any.
	string const opts = createLatexOptions();
	if (!opts.empty()) {
		os << "[%\n  " << opts << ']';
	}
	// Make the filename relative to the lyx file
	// and remove the extension so the LaTeX will use whatever is
	// appropriate (when there are several versions in different formats)
	string const filename = prepareFile(buf);
	os << '{' << filename << '}' << after;
	// Return how many newlines we issued.
	return newlines;
}


int InsetGraphics::ascii(Buffer const *, ostream &, int) const
{
	// No graphics in ascii output. Possible to use gifscii to convert
	// images to ascii approximation.
	
	// 1. Convert file to ascii using gifscii
	// 2. Read ascii output file and add it to the output stream.
	
	return 0;
}


int InsetGraphics::linuxdoc(Buffer const *, ostream &) const
{
	// No graphics in LinuxDoc output. Should check how/what to add.
	return 0;
}


// For explanation on inserting graphics into DocBook checkout:
// http://linuxdoc.org/LDP/LDP-Author-Guide/inserting-pictures.html
// See also the docbook guide at http://www.docbook.org/
int InsetGraphics::docbook(Buffer const * buf, ostream & os) const
{
	// Change the path to be relative to the main file.
	string const buffer_dir = buf->filePath();
	string filename = RemoveExtension(
		MakeRelPath(params.filename, buffer_dir));

	if (suffixIs(filename, ".eps"))
		filename.erase(filename.length() - 4);

	// In DocBook v5.0, the graphic tag will be eliminated from DocBook, will 
	// need to switch to MediaObject. However, for now this is sufficient and 
	// easier to use.
	os << "<graphic fileref=\"" << filename << "\"></graphic>";
	return 0;
}


void InsetGraphics::validate(LaTeXFeatures & features) const
{
	// If we have no image, we should not require anything.
	if (params.filename.empty())
		return ;

	features.require("graphicx");

	if (params.subcaption)
		features.require("subfigure");
}


// Update the inset after parameters changed (read from file or changed in
// dialog.
void InsetGraphics::updateInset() const
{
	GraphicsCache & gc = GraphicsCache::getInstance();
	boost::shared_ptr<GraphicsCacheItem> temp(0);

	// We do it this way so that in the face of some error, we will still
	// be in a valid state.
	if (!params.filename.empty() && lyxrc.use_gui
	    && lyxrc.display_graphics != "no" 
	    && params.display != InsetGraphicsParams::NONE) {
		temp = gc.addFile(params.filename);
	}

	// Mark the image as unloaded so that it gets updated.
	imageLoaded = false;

	cacheHandle = temp;
}


bool InsetGraphics::setParams(InsetGraphicsParams const & p)
{
	// If nothing is changed, just return and say so.
	if (params == p)
		return false;

	// Copy the new parameters.
	params = p;

	// Update the inset with the new parameters.
	updateInset();

	// We have changed data, report it.
	return true;
}


InsetGraphicsParams InsetGraphics::getParams() const
{
	return params;
}


Inset * InsetGraphics::clone(Buffer const &, bool same_id) const
{
	return new InsetGraphics(*this, same_id);
}

