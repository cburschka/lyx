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

      Note that browseRelFile in helper_funcs.* provides a file name
      which is relative if it is at reference path (here puffer path)
      level or below, and an absolute path if the file name is not a
      `natural' relative file name. In any case,
	      MakeAbsPath(filename, buf->filePath())
      is guaranteed to provide the correct absolute path. This is what is
      done know for include insets. Feel free to ask me -- JMarc
      14/01/2002

TODO Before initial production release:

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
 *	Image format
 *	from        to
 *	EPS         epstopdf
 *	PS          ps2pdf
 *	JPG/PNG     direct
 *	PDF         direct
 *	others      PNG
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insets/insetgraphics.h"
#include "insets/insetgraphicsParams.h"

#include "graphics/GraphicsCache.h"
#include "graphics/GraphicsImage.h"

#include "LyXView.h"
#include "lyxtext.h"
#include "buffer.h"
#include "BufferView.h"
#include "converter.h"
#include "Painter.h"
#include "lyxrc.h"
#include "font.h"    // For the lyxfont class.
#include "debug.h"
#include "gettext.h"
#include "LaTeXFeatures.h"

#include "frontends/Dialogs.h"
#include "frontends/Alert.h"
#include "frontends/controllers/helper_funcs.h" // getVectorFromString

#include "support/LAssert.h"
#include "support/filetools.h"
#include "support/lyxalgo.h" // lyx::count
#include "support/path.h"

#include <algorithm> // For the std::max

extern string system_tempdir;

using std::ostream;
using std::endl;

///////////////////////////////////////////////////////////////////////////
int const VersionNumber = 1;
///////////////////////////////////////////////////////////////////////////

namespace {

// This function is a utility function
// ... that should be with ChangeExtension ...
inline
string const RemoveExtension(string const & filename)
{
	return ChangeExtension(filename, string());
}

} // namespace anon


namespace {

string const unique_id()
{
	static unsigned int seed = 1000;

	ostringstream ost;
	ost << "graph" << ++seed;

	// Needed if we use lyxstring.
	return ost.str().c_str();
}

} // namespace anon


InsetGraphics::InsetGraphics()
	: graphic_label(unique_id()),
	  cached_status_(grfx::ErrorUnknown), cache_filled_(false), old_asc(0)

{}


InsetGraphics::InsetGraphics(InsetGraphics const & ig,
			     string const & filepath,
			     bool same_id)
	: Inset(ig, same_id),
	  SigC::Object(),
	  graphic_label(unique_id()),
	  cached_status_(grfx::ErrorUnknown), cache_filled_(false), old_asc(0)
{
	setParams(ig.params(), filepath);
}


InsetGraphics::~InsetGraphics()
{
	cached_image_.reset(0);
	grfx::GCache & gc = grfx::GCache::get();
	gc.remove(*this);

	// Emits the hide signal to the dialog connected (if any)
	hideDialog();
}


string const InsetGraphics::statusMessage() const
{
	string msg;

	switch (cached_status_) {
	case grfx::WaitingToLoad:
		msg = _("Waiting for draw request to start loading...");
		break;
	case grfx::Loading:
		msg = _("Loading...");
		break;
	case grfx::Converting:
		msg = _("Converting to loadable format...");
		break;
	case grfx::ScalingEtc:
		msg = _("Loaded. Scaling etc...");
		break;
	case grfx::ErrorNoFile:
		msg = _("No file found!");
		break;
	case grfx::ErrorLoading:
		msg = _("Error loading file into memory");
		break;
	case grfx::ErrorConverting:
		msg = _("Error converting to loadable format");
		break;
	case grfx::ErrorScalingEtc:
		msg = _("Error scaling etc");
		break;
	case grfx::ErrorUnknown:
		msg = _("No image");
		break;
	case grfx::Loaded:
		msg = _("Loaded but not displaying");
		break;
	}

	return msg;
}


void InsetGraphics::setCache() const
{
	if (cache_filled_)
		return;

	grfx::GCache & gc = grfx::GCache::get();
	cached_status_ = gc.status(*this);
	cached_image_  = gc.image(*this);
}


bool InsetGraphics::drawImage() const
{
	setCache();
	Pixmap const pixmap =
		(cached_status_ == grfx::Loaded && cached_image_.get() != 0) ?
		cached_image_->getPixmap() : 0;

	return pixmap != 0;
}


int InsetGraphics::ascent(BufferView *, LyXFont const &) const
{
	old_asc = 50;
	if (drawImage())
		old_asc = cached_image_->getHeight();
	return old_asc;
}


int InsetGraphics::descent(BufferView *, LyXFont const &) const
{
	return 0;
}


int InsetGraphics::width(BufferView *, LyXFont const & font) const
{
	if (drawImage())
		return cached_image_->getWidth();
	else {
		int font_width = 0;

		LyXFont msgFont(font);
		msgFont.setFamily(LyXFont::SANS_FAMILY);

		string const justname = OnlyFilename (params().filename);
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
	int oasc = old_asc;
	grfx::ImageStatus old_status_ = cached_status_;

	int ldescent = descent(bv, font);
	int lascent  = ascent(bv, font);
	int lwidth   = width(bv, font);

	// we may have changed while someone other was drawing us so better
	// to not draw anything as we surely call to redraw ourself soon.
	// This is not a nice thing to do and should be fixed properly somehow.
	// But I still don't know the best way to go. So let's do this like this
	// for now (Jug 20020311)
	if (lascent != oasc) {
		return;
	}

	// Make sure now that x is updated upon exit from this routine
	int old_x = int(x);
	x += lwidth;

	// Initiate the loading of the graphics file
	if (cached_status_ == grfx::WaitingToLoad) {
		grfx::GCache & gc = grfx::GCache::get();
		gc.startLoading(*this);
	}

	// This will draw the graphics. If the graphics has not been loaded yet,
	// we draw just a rectangle.
	Painter & paint = bv->painter();

	if (drawImage()) {
		paint.image(old_x + 2, baseline - lascent,
			    lwidth - 4, lascent + ldescent,
			    *cached_image_.get());

	} else {

		paint.rectangle(old_x + 2, baseline - lascent,
				lwidth - 4,
				lascent + ldescent);

		// Print the file name.
		LyXFont msgFont(font);
		msgFont.setFamily(LyXFont::SANS_FAMILY);
		string const justname = OnlyFilename (params().filename);
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

	// the status message may mean we changed size, so indicate
	// we need a row redraw
	if (old_status_ != grfx::ErrorUnknown && old_status_ != cached_status_) {
		bv->getLyXText()->status(bv, LyXText::CHANGED_IN_DRAW);
	}

	// Reset the cache, ready for the next draw request
	cached_status_ = grfx::ErrorUnknown;
	cached_image_.reset(0);
	cache_filled_ = false;
}


// Update the inset after parameters changed (read from file or changed in
// dialog. The grfx::GCache makes the decisions about whether or not to draw
// (interogates lyxrc, ascertains whether file exists etc)
void InsetGraphics::updateInset(string const & filepath) const
{
	grfx::GCache & gc = grfx::GCache::get();
	gc.update(*this, filepath);
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


void InsetGraphics::write(Buffer const *, ostream & os) const
{
	os << "Graphics FormatVersion " << VersionNumber << '\n';
	params().Write(os);
}


void InsetGraphics::read(Buffer const * buf, LyXLex & lex)
{
	string const token = lex.getString();

	if (token == "Graphics")
		readInsetGraphics(lex);
	else if (token == "Figure") // Compatibility reading of FigInset figures.
		readFigInset(lex);
	else
		lyxerr[Debug::GRAPHICS] << "Not a Graphics or Figure inset!\n";

	updateInset(buf->filePath());
}


void InsetGraphics::readInsetGraphics(LyXLex & lex)
{
	bool finished = false;

	while (lex.isOK() && !finished) {
		lex.next();

		string const token = lex.getString();
		lyxerr[Debug::GRAPHICS] << "Token: '" << token << '\''
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
			if (! params_.Read(lex, token))
				lyxerr << "Unknown token, " << token << ", skipping."
					<< std::endl;
		}
	}
}

// FormatVersion < 1.0  (LyX < 1.2)
void InsetGraphics::readFigInset(LyXLex & lex)
{
	std::vector<string> const oldUnits =
		getVectorFromString("pt,cm,in,p%,c%");
	bool finished = false;
	// set the display default
	if (lyxrc.display_graphics == "mono")
	    params_.display = InsetGraphicsParams::MONOCHROME;
	else if (lyxrc.display_graphics == "gray")
	    params_.display = InsetGraphicsParams::GRAYSCALE;
	else if (lyxrc.display_graphics == "color")
	    params_.display = InsetGraphicsParams::COLOR;
	else
	    params_.display = InsetGraphicsParams::NONE;
	while (lex.isOK() && !finished) {
		lex.next();

		string const token = lex.getString();
		lyxerr[Debug::GRAPHICS] << "Token: " << token << endl;

		if (token.empty())
			continue;
		else if (token == "\\end_inset") {
			finished = true;
		} else if (token == "file") {
			if (lex.next()) {
				params_.filename = lex.getString();
			}
		} else if (token == "extra") {
			if (lex.next());
			// kept for backwards compability. Delete in 0.13.x
		} else if (token == "subcaption") {
			if (lex.eatLine())
				params_.subcaptionText = lex.getString();
		} else if (token == "label") {
			if (lex.next());
			// kept for backwards compability. Delete in 0.13.x
		} else if (token == "angle") {
			if (lex.next()) {
				params_.rotate = true;
				params_.rotateAngle = lex.getFloat();
			}
		} else if (token == "size") {
			if (lex.next())
				params_.lyxwidth = LyXLength(lex.getString()+"pt");
			if (lex.next())
				params_.lyxheight = LyXLength(lex.getString()+"pt");
			params_.lyxsize_type = InsetGraphicsParams::WH;
		} else if (token == "flags") {
			if (lex.next())
				switch (lex.getInteger()) {
				case 1: params_.display = InsetGraphicsParams::MONOCHROME;
				    break;
				case 2: params_.display = InsetGraphicsParams::GRAYSCALE;
				    break;
				case 3: params_.display = InsetGraphicsParams::COLOR;
				    break;
				}
		} else if (token == "subfigure") {
			params_.subcaption = true;
		} else if (token == "width") {
		    if (lex.next()) {
			int i = lex.getInteger();
			if (lex.next()) {
			    if (i == 5) {
				params_.scale = lex.getInteger();
				params_.size_type = InsetGraphicsParams::SCALE;
			    } else {
				params_.width = LyXLength(lex.getString()+oldUnits[i]);
				params_.size_type = InsetGraphicsParams::WH;
			    }
			}
		    }
		} else if (token == "height") {
		    if (lex.next()) {
			int i = lex.getInteger();
			if (lex.next()) {
			    params_.height = LyXLength(lex.getString()+oldUnits[i]);
			    params_.size_type = InsetGraphicsParams::WH;
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
	if (!params().bb.empty())
	    options << "  bb=" << strip(params().bb) << ",\n";
	if (params().draft)
	    options << "  draft,\n";
	if (params().clip)
	    options << "  clip,\n";
	if (params().size_type == InsetGraphicsParams::WH) {
	    if (!params().width.zero())
		options << "  width=" << params().width.asLatexString() << ",\n";
	    if (!params().height.zero())
		options << "  height=" << params().height.asLatexString() << ",\n";
	} else if (params().size_type == InsetGraphicsParams::SCALE) {
	    if (params().scale > 0)
		options << "  scale=" << double(params().scale)/100.0 << ",\n";
	}
	if (params().keepAspectRatio)
	    options << "  keepaspectratio,\n";
	// Make sure it's not very close to zero, a float can be effectively
	// zero but not exactly zero.
	if (!lyx::float_equal(params().rotateAngle, 0, 0.001) && params().rotate) {
	    options << "  angle=" << params().rotateAngle << ",\n";
	    if (!params().rotateOrigin.empty()) {
		options << "  origin=" << params().rotateOrigin[0];
		if (contains(params().rotateOrigin,"Top"))
		    options << 't';
		else if (contains(params().rotateOrigin,"Bottom"))
		    options << 'b';
		else if (contains(params().rotateOrigin,"Baseline"))
		    options << 'B';
		options << ",\n";
	    }
	}
	if (!params().special.empty())
	    options << params().special << ",\n";
	string opts = options.str().c_str();
	return opts.substr(0,opts.size()-2);	// delete last ",\n"
}

namespace {
string findTargetFormat(string const & suffix)
{
	// lyxrc.pdf_mode means:
	// Are we creating a PDF or a PS file?
	// (Should actually mean, are we using latex or pdflatex).
	lyxerr[Debug::GRAPHICS] << "findTargetFormat: lyxrc.pdf_mode = "
			    << lyxrc.pdf_mode << std::endl;
	if (lyxrc.pdf_mode) {
		if (contains(suffix,"ps") || suffix == "pdf")
			return "pdf";
		else if (suffix == "jpg")
			return suffix;
		else
			return "png";
	}
	// If it's postscript, we always do eps.
	lyxerr[Debug::GRAPHICS] << "findTargetFormat: PostScript mode\n";
	if (suffix != "ps")
	    return "eps";
	else
	    return "ps";
}

} // Anon. namespace


string const InsetGraphics::prepareFile(Buffer const *buf) const
{
	// LaTeX can cope if the graphics file doesn't exist, so just return the
	// filename.
	string const orig_file = params().filename;
	string const orig_file_with_path =
		MakeAbsPath(orig_file, buf->filePath());

	if (!IsFileReadable(orig_file_with_path))
		return orig_file;

	// If the file is compressed and we have specified that it should not be
	// uncompressed, then just return its name and let LaTeX do the rest!
	bool const zipped = zippedFile(orig_file);
	if (zipped && params().noUnzip) {
		return orig_file;
	}

	// "nice" means that the buffer is exported to LaTeX format but not
	//        run through the LaTeX compiler.
	// if (nice)
	//     No conversion of the graphics file is needed.
	//     Return the original filename without any extension.
	if (buf->niceFile)
		return RemoveExtension(orig_file);

	// We're going to be running the exported buffer through the LaTeX
	// compiler, so must ensure that LaTeX can cope with the graphics
	// file format.

	// Perform all these manipulations on a temporary file if possible.
	// If we are not using a temp dir, then temp_file contains the
	// original file.
	string temp_file = MakeAbsPath(OnlyFilename(orig_file), buf->tmppath);

	lyxerr[Debug::GRAPHICS]
		<< "InsetGraphics::prepareFile. The temp file is: "
		<< temp_file << endl;

	// If we are using a temp dir, then copy the file into it.
	if (lyxrc.use_tempdir && !IsFileReadable(temp_file)) {
		bool const success = lyx::copy(orig_file_with_path, temp_file);
		lyxerr[Debug::GRAPHICS]
			<< "InsetGraphics::prepareFile. Copying from " 
			<< orig_file << " to " << temp_file
			<< (success ? " succeeded\n" : " failed\n");
		if (!success) {
			Alert::alert(_("Cannot copy file"), orig_file,
					_("into tempdir"));
			return orig_file;
		}
	}

	// Uncompress the file if necessary. If it has been uncompressed in
	// a previous call to prepareFile, do nothing.
        if (zipped) {
		// What we want to end up with:
		string const temp_file_unzipped =
			ChangeExtension(temp_file, string());

		if (!IsFileReadable(temp_file_unzipped)) {
			// unzipFile generates a random filename, so move this
			// file where we want it to go.
			string const tmp = unzipFile(temp_file);
			lyx::copy(tmp, temp_file_unzipped);
			lyx::unlink(tmp);

			lyxerr[Debug::GRAPHICS]
				<< "InsetGraphics::prepareFile. Unzipped to "
				<< temp_file_unzipped << endl;
		}

		// We have an uncompressed file where we expect it,
		// so rename temp_file and continue.
		temp_file = temp_file_unzipped;
	}

	// Ascertain the graphics format that LaTeX requires.
	string const from = getExtFromContents(temp_file);
	string const to   = findTargetFormat(from);

	// No conversion is needed. LaTeX can handle the graphics file as it is.
	// This is true even if the orig_file is compressed.
	if (from == to) {
		return orig_file;
	}

	string const outfile_base = RemoveExtension(temp_file);

	lyxerr[Debug::GRAPHICS]
		<< "InsetGraphics::prepareFile. The original file is "
		<< orig_file << ".\n"
		<< "A copy has been made and convert is to be called with:\n"
		<< "\tfile to convert = " << temp_file << '\n'
		<< "\toutfile_base = " << outfile_base << '\n'
		<< "\t from " << from << " to " << to << '\n';

	converters.convert(buf, temp_file, outfile_base, from, to);
	return RemoveExtension(temp_file);
}


int InsetGraphics::latex(Buffer const *buf, ostream & os,
			 bool /*fragile*/, bool/*fs*/) const
{
	// If there is no file specified or not existing,
	// just output a message about it in the latex output.
	lyxerr[Debug::GRAPHICS]
		<< "insetgraphics::latex: Filename = "
		<< params().filename << endl;

	// A missing (e)ps-extension is no problem for LaTeX, so
	// we have to test three different cases
	string const file_(MakeAbsPath(params().filename, buf->filePath()));
	bool const file_exists = 
		!file_.empty() && 
		(IsFileReadable(file_) ||		// original
		 IsFileReadable(file_ + ".eps") || 	// original.eps
		 IsFileReadable(file_ + ".ps"));	// original.ps
	string const message = file_exists ? 
		string() : string("bb = 0 0 200 100, draft, type=eps]");
	// if !message.empty() than there was no existing file
	// "filename(.(e)ps)" found. In this case LaTeX
	// draws only a rectangle with the above bb and the
	// not found filename in it.
	lyxerr[Debug::GRAPHICS]
		<< "InsetGraphics::latex. Message = \"" << message << '\"' << endl;

	// These variables collect all the latex code that should be before and
	// after the actual includegraphics command.
	string before;
	string after;
	// Do we want subcaptions?
	if (params().subcaption) {
		before += "\\subfigure[" + params().subcaptionText + "]{";
		after = '}';
	}
	// We never use the starred form, we use the "clip" option instead.
	before += "\\includegraphics";

	// Write the options if there are any.
	string const opts = createLatexOptions();
	lyxerr[Debug::GRAPHICS]
		<< "InsetGraphics::latex. Opts = " << opts << endl;

	if (!opts.empty() && !message.empty())
		before += ("[" + opts + ',' + message);
	else if (!message.empty())
		before += ('[' + message);
	else if (!opts.empty())
		before += ("[" + opts + ']');

	lyxerr[Debug::GRAPHICS]
		<< "InsetGraphics::latex. Before = " << before
		<< "\nafter = " << after << endl;

	// Make the filename relative to the lyx file
	// and remove the extension so the LaTeX will use whatever is
	// appropriate (when there are several versions in different formats)
	string const latex_str = message.empty() ?
		(before + '{' + prepareFile(buf) + '}' + after) :
		(before + '{' + params().filename + " not found!}" + after);
	os << latex_str;

	// Return how many newlines we issued.
	int const newlines =
		int(lyx::count(latex_str.begin(), latex_str.end(),'\n') + 1);

	return newlines;
}


int InsetGraphics::ascii(Buffer const *, ostream & os, int) const
{
	// No graphics in ascii output. Possible to use gifscii to convert
	// images to ascii approximation.
	// 1. Convert file to ascii using gifscii
	// 2. Read ascii output file and add it to the output stream.
	// at least we send the filename
	os << '<' << _("Graphic file:") << params().filename << ">\n";
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
int InsetGraphics::docbook(Buffer const *, ostream & os) const
{
	// In DocBook v5.0, the graphic tag will be eliminated from DocBook, will
	// need to switch to MediaObject. However, for now this is sufficient and
	// easier to use.
	os << "<graphic fileref=\"&" << graphic_label << ";\">";
	return 0;
}


void InsetGraphics::validate(LaTeXFeatures & features) const
{
	// If we have no image, we should not require anything.
	if (params().filename.empty())
		return ;

	features.includeFile(graphic_label, RemoveExtension(params().filename));

	features.require("graphicx");

	if (params().subcaption)
		features.require("subfigure");
}


bool InsetGraphics::setParams(InsetGraphicsParams const & p,
			      string const & filepath)
{
	// If nothing is changed, just return and say so.
	if (params() == p && !p.filename.empty()) {
		return false;
	}

	// Copy the new parameters.
	params_ = p;

	// Update the inset with the new parameters.
	updateInset(filepath);

	// We have changed data, report it.
	return true;
}


InsetGraphicsParams const & InsetGraphics::params() const
{
	return params_;
}


Inset * InsetGraphics::clone(Buffer const & buffer, bool same_id) const
{
	return new InsetGraphics(*this, buffer.filePath(), same_id);
}
