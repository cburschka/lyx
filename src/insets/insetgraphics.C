/**
 * \file insetgraphics.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

/*
TODO

    * What advanced features the users want to do?
      Implement them in a non latex dependent way, but a logical way.
      LyX should translate it to latex or any other fitting format.
    * Add a way to roll the image file into the file format.
    * When loading, if the image is not found in the expected place, try
      to find it in the clipart, or in the same directory with the image.
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

#include "insets/insetgraphics.h"
#include "insets/render_graphic.h"

#include "buffer.h"
#include "BufferView.h"
#include "converter.h"
#include "cursor.h"
#include "debug.h"
#include "dispatchresult.h"
#include "exporter.h"
#include "format.h"
#include "funcrequest.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "lyx_main.h"
#include "lyxlength.h"
#include "lyxlex.h"
#include "metricsinfo.h"
#include "mover.h"
#include "outputparams.h"
#include "sgml.h"

#include "frontends/Alert.h"
#include "frontends/LyXView.h"

#include "support/filetools.h"
#include "support/lyxalgo.h" // lyx::count
#include "support/lyxlib.h" // lyx::sum
#include "support/lstrings.h"
#include "support/os.h"
#include "support/systemcall.h"

#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>

#include <sstream>

namespace support = lyx::support;

using lyx::support::AbsolutePath;
using lyx::support::bformat;
using lyx::support::ChangeExtension;
using lyx::support::compare_timestamps;
using lyx::support::contains;
using lyx::support::FileName;
using lyx::support::float_equal;
using lyx::support::GetExtension;
using lyx::support::IsFileReadable;
using lyx::support::LibFileSearch;
using lyx::support::OnlyFilename;
using lyx::support::rtrim;
using lyx::support::strToDbl;
using lyx::support::subst;
using lyx::support::Systemcall;
using lyx::support::unzipFile;
using lyx::support::unzippedFileName;

namespace os = lyx::support::os;

using std::endl;
using std::string;
using std::auto_ptr;
using std::istringstream;
using std::ostream;
using std::ostringstream;


namespace {

// This function is a utility function
// ... that should be with ChangeExtension ...
inline
string const RemoveExtension(string const & filename)
{
	return ChangeExtension(filename, string());
}


string findTargetFormat(string const & format, OutputParams const & runparams)
{
	// Are we using latex or pdflatex?
	if (runparams.flavor == OutputParams::PDFLATEX) {
		lyxerr[Debug::GRAPHICS] << "findTargetFormat: PDF mode" << endl;
		// Convert postscript to pdf
		if (format == "eps" || format == "ps")
			return "pdf";
		// pdflatex can use jpeg, png and pdf directly
		if (format == "jpg" || format == "pdf")
			return format;
		// Convert everything else to png
		return "png";
	}
	// If it's postscript, we always do eps.
	lyxerr[Debug::GRAPHICS] << "findTargetFormat: PostScript mode" << endl;
	if (format != "ps")
		// any other than ps is changed to eps
		return "eps";
	// let ps untouched
	return format;
}

} // namespace anon


InsetGraphics::InsetGraphics()
	: graphic_label(sgml::uniqueID("graph")),
	  graphic_(new RenderGraphic(this))
{}


InsetGraphics::InsetGraphics(InsetGraphics const & ig)
	: InsetOld(ig),
	  boost::signals::trackable(),
	  graphic_label(sgml::uniqueID("graph")),
	  graphic_(new RenderGraphic(*ig.graphic_, this))
{
	setParams(ig.params());
}


auto_ptr<InsetBase> InsetGraphics::doClone() const
{
	return auto_ptr<InsetBase>(new InsetGraphics(*this));
}


InsetGraphics::~InsetGraphics()
{
	InsetGraphicsMailer(*this).hideDialog();
}


void InsetGraphics::doDispatch(LCursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {
	case LFUN_GRAPHICS_EDIT: {
		Buffer const & buffer = *cur.bv().buffer();
		InsetGraphicsParams p;
		InsetGraphicsMailer::string2params(cmd.argument, buffer, p);
		editGraphics(p, buffer);
		break;
	}

	case LFUN_INSET_MODIFY: {
		Buffer const & buffer = cur.buffer();
		InsetGraphicsParams p;
		InsetGraphicsMailer::string2params(cmd.argument, buffer, p);
		if (!p.filename.empty()) {
			setParams(p);
			cur.bv().update();
		}
		break;
	}

	case LFUN_INSET_DIALOG_UPDATE:
		InsetGraphicsMailer(*this).updateDialog(&cur.bv());
		break;

	case LFUN_MOUSE_RELEASE:
		InsetGraphicsMailer(*this).showDialog(&cur.bv());
		break;

	default:
		InsetBase::doDispatch(cur, cmd);
		break;
	}
}


void InsetGraphics::edit(LCursor & cur, bool)
{
	InsetGraphicsMailer(*this).showDialog(&cur.bv());
}


void InsetGraphics::metrics(MetricsInfo & mi, Dimension & dim) const
{
	graphic_->metrics(mi, dim);
	dim_ = dim;
}


void InsetGraphics::draw(PainterInfo & pi, int x, int y) const
{
	setPosCache(pi, x, y);
	graphic_->draw(pi, x, y);
}


InsetBase::EDITABLE InsetGraphics::editable() const
{
	return IS_EDITABLE;
}


void InsetGraphics::write(Buffer const & buf, ostream & os) const
{
	os << "Graphics\n";
	params().Write(os, buf.filePath());
}


void InsetGraphics::read(Buffer const & buf, LyXLex & lex)
{
	string const token = lex.getString();

	if (token == "Graphics")
		readInsetGraphics(lex, buf.filePath());
	else
		lyxerr[Debug::GRAPHICS] << "Not a Graphics inset!" << endl;

	graphic_->update(params().as_grfxParams());
}


void InsetGraphics::readInsetGraphics(LyXLex & lex, string const & bufpath)
{
	bool finished = false;

	while (lex.isOK() && !finished) {
		lex.next();

		string const token = lex.getString();
		lyxerr[Debug::GRAPHICS] << "Token: '" << token << '\''
				    << endl;

		if (token.empty()) {
			continue;
		} else if (token == "\\end_inset") {
			finished = true;
		} else {
			if (!params_.Read(lex, token, bufpath))
				lyxerr << "Unknown token, " << token << ", skipping."
					<< std::endl;
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
	    options << " bb=" << rtrim(params().bb) << ",\n";
	if (params().draft)
	    options << " draft,\n";
	if (params().clip)
	    options << " clip,\n";
	double const scl = strToDbl(params().scale);
	if (!params().scale.empty() && !float_equal(scl, 0.0, 0.05)) {
		if (!float_equal(scl, 100.0, 0.05))
			options << " scale=" << scl / 100.0
				<< ",\n";
	} else {
		if (!params().width.zero())
			options << " width=" << params().width.asLatexString() << ",\n";
		if (!params().height.zero())
			options << " height=" << params().height.asLatexString() << ",\n";
		if (params().keepAspectRatio)
			options << " keepaspectratio,\n";
	}

	// Make sure rotation angle is not very close to zero;
	// a float can be effectively zero but not exactly zero.
	if (!params().rotateAngle.empty()
		&& !float_equal(strToDbl(params().rotateAngle), 0.0, 0.001)) {
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

	string opts = options.str();
	// delete last ",\n"
	return opts.substr(0, opts.size() - 2);
}


string const InsetGraphics::toDocbookLength(LyXLength const & len) const
{
	ostringstream result;
	switch (len.unit()) {
		case LyXLength::SP: // Scaled point (65536sp = 1pt) TeX's smallest unit.
			result << len.value() * 65536.0 * 72 / 72.27 << "pt";
			break;
		case LyXLength::PT: // Point = 1/72.27in = 0.351mm
			result << len.value() * 72 / 72.27 << "pt";
			break;
		case LyXLength::BP: // Big point (72bp = 1in), also PostScript point
			result << len.value() << "pt";
			break;
		case LyXLength::DD: // Didot point = 1/72 of a French inch, = 0.376mm
			result << len.value() * 0.376 << "mm";
			break;
		case LyXLength::MM: // Millimeter = 2.845pt
			result << len.value() << "mm";
			break;
		case LyXLength::PC: // Pica = 12pt = 4.218mm
			result << len.value() << "pc";
			break;
		case LyXLength::CC: // Cicero = 12dd = 4.531mm
			result << len.value() * 4.531 << "mm";
			break;
		case LyXLength::CM: // Centimeter = 10mm = 2.371pc
			result << len.value() << "cm";
			break;
		case LyXLength::IN: // Inch = 25.4mm = 72.27pt = 6.022pc
			result << len.value() << "in";
			break;
		case LyXLength::EX: // Height of a small "x" for the current font.
			// Obviously we have to compromise here. Any better ratio than 1.5 ?
			result << len.value() / 1.5 << "em";
			break;
		case LyXLength::EM: // Width of capital "M" in current font.
			result << len.value() << "em";
			break;
		case LyXLength::MU: // Math unit (18mu = 1em) for positioning in math mode
			result << len.value() * 18 << "em";
			break;
		case LyXLength::PTW: // Percent of TextWidth
		case LyXLength::PCW: // Percent of ColumnWidth
		case LyXLength::PPW: // Percent of PageWidth
		case LyXLength::PLW: // Percent of LineWidth
		case LyXLength::PTH: // Percent of TextHeight
		case LyXLength::PPH: // Percent of Paper
			// Sigh, this will go wrong.
			result << len.value() << "%";
			break;
		default:
			result << len.asString();
			break;
	}
	return result.str();
}

string const InsetGraphics::createDocBookAttributes() const
{
	// Calculate the options part of the command, we must do it to a string
	// stream since we copied the code from createLatexParams() ;-)

	// FIXME: av: need to translate spec -> Docbook XSL spec (http://www.sagehill.net/docbookxsl/ImageSizing.html)
	// Right now it only works with my version of db2latex :-)

	ostringstream options;
	double const scl = strToDbl(params().scale);
	if (!params().scale.empty() && !float_equal(scl, 0.0, 0.05)) {
		if (!float_equal(scl, 100.0, 0.05))
			options << " scale=\""
				<< static_cast<int>( (scl) + 0.5 )
				<< "\" ";
	} else {
		if (!params().width.zero()) {
			options << " width=\"" << toDocbookLength(params().width)  << "\" ";
		}
		if (!params().height.zero()) {
			options << " depth=\"" << toDocbookLength(params().height)  << "\" ";
		}
		if (params().keepAspectRatio) {
			// This will be irrelevant unless both width and height are set
			options << "scalefit=\"1\" ";
		}
	}


	if (!params().special.empty())
	    options << params().special << " ";

	string opts = options.str();
	// trailing blanks are ok ...
	return opts;
}


namespace {

enum CopyStatus {
	SUCCESS,
	FAILURE,
	IDENTICAL_PATHS,
	IDENTICAL_CONTENTS
};


std::pair<CopyStatus, string> const
copyFileIfNeeded(string const & file_in, string const & file_out)
{
	BOOST_ASSERT(AbsolutePath(file_in));
	BOOST_ASSERT(AbsolutePath(file_out));

	unsigned long const checksum_in  = support::sum(file_in);
	unsigned long const checksum_out = support::sum(file_out);

	if (checksum_in == checksum_out)
		// Nothing to do...
		return std::make_pair(IDENTICAL_CONTENTS, file_out);

	Mover const & mover = movers(formats.getFormatFromFile(file_in));
	bool const success = mover.copy(file_in, file_out);
	if (!success) {
		lyxerr[Debug::GRAPHICS]
			<< support::bformat(_("Could not copy the file\n%1$s\n"
					      "into the temporary directory."),
					    file_in)
			<< std::endl;
	}

	CopyStatus status = success ? SUCCESS : FAILURE;
	return std::make_pair(status, file_out);
}


std::pair<CopyStatus, string> const
copyToDirIfNeeded(string const & file_in, string const & dir, bool zipped)
{
	using support::rtrim;

	BOOST_ASSERT(AbsolutePath(file_in));

	string const only_path = support::OnlyPath(file_in);
	if (rtrim(support::OnlyPath(file_in) , "/") == rtrim(dir, "/"))
		return std::make_pair(IDENTICAL_PATHS, file_in);

	string mangled = FileName(file_in).mangledFilename();
	if (zipped) {
		// We need to change _eps.gz to .eps.gz. The mangled name is
		// still unique because of the counter in mangledFilename().
		// We can't just call mangledFilename() with the zip
		// extension removed, because base.eps and base.eps.gz may
		// have different content but would get the same mangled
		// name in this case.
		string const base = RemoveExtension(unzippedFileName(file_in));
		string::size_type const ext_len = file_in.length() - base.length();
		mangled[mangled.length() - ext_len] = '.';
	}
	string const file_out = support::MakeAbsPath(mangled, dir);

	return copyFileIfNeeded(file_in, file_out);
}


string const stripExtension(string const & file)
{
	// Remove the extension so the LaTeX will use whatever
	// is appropriate (when there are several versions in
	// different formats)
	// This works only if the filename contains no dots besides
	// the just removed one. We can fool here by replacing all
	// dots with a macro whose definition is just a dot ;-)
	return subst(RemoveExtension(file), ".", "\\lyxdot ");
}


string const stripExtensionIfPossible(string const & file, string const & to)
{
	// No conversion is needed. LaTeX can handle the graphic file as is.
	// This is true even if the orig_file is compressed.
	string const to_format = formats.getFormat(to)->extension();
	string const file_format = GetExtension(file);
	// for latex .ps == .eps
	if (to_format == file_format ||
	    (to_format == "eps" && file_format ==  "ps") ||
	    (to_format ==  "ps" && file_format == "eps"))
		return stripExtension(file);
	return file;
}

} // namespace anon


string const InsetGraphics::prepareFile(Buffer const & buf,
					OutputParams const & runparams) const
{
	// We assume that the file exists (the caller checks this)
	string const orig_file = params().filename.absFilename();
	string const rel_file = params().filename.relFilename(buf.filePath());

	// If the file is compressed and we have specified that it
	// should not be uncompressed, then just return its name and
	// let LaTeX do the rest!
	bool const zipped = params().filename.isZipped();

	// temp_file will contain the file for LaTeX to act on if, for example,
	// we move it to a temp dir or uncompress it.
	string temp_file = orig_file;

	// The master buffer. This is useful when there are multiple levels
	// of include files
	Buffer const * m_buffer = buf.getMasterBuffer();

	// We place all temporary files in the master buffer's temp dir.
	// This is possible because we use mangled file names.
	// This is necessary for DVI export.
	string const temp_path = m_buffer->temppath();

	CopyStatus status;
	boost::tie(status, temp_file) =
			copyToDirIfNeeded(orig_file, temp_path, zipped);

	if (status == FAILURE)
		return orig_file;

	// a relative filename should be relative to the master
	// buffer.
	// "nice" means that the buffer is exported to LaTeX format but not
	//        run through the LaTeX compiler.
	string const output_file = os::external_path(runparams.nice ?
		params().filename.outputFilename(m_buffer->filePath()) :
		OnlyFilename(temp_file));
	string const source_file = runparams.nice ? orig_file : temp_file;

	if (zipped) {
		if (params().noUnzip) {
			// We don't know whether latex can actually handle
			// this file, but we can't check, because that would
			// mean to unzip the file and thereby making the
			// noUnzip parameter meaningless.
			lyxerr[Debug::GRAPHICS]
				<< "\tpass zipped file to LaTeX.\n";

			string const bb_orig_file = ChangeExtension(orig_file, "bb");
			if (runparams.nice) {
				runparams.exportdata->addExternalFile("latex",
						bb_orig_file,
						ChangeExtension(output_file, "bb"));
			} else {
				// LaTeX needs the bounding box file in the
				// tmp dir
				string bb_file = ChangeExtension(temp_file, "bb");
				boost::tie(status, bb_file) =
					copyFileIfNeeded(bb_orig_file, bb_file);
				if (status == FAILURE)
					return orig_file;
				runparams.exportdata->addExternalFile("latex",
						bb_file);
			}
			runparams.exportdata->addExternalFile("latex",
					source_file, output_file);
			runparams.exportdata->addExternalFile("dvi",
					source_file, output_file);
			// We can't strip the extension, because we don't know
			// the unzipped file format
			return output_file;
		}

		string const unzipped_temp_file = unzippedFileName(temp_file);
		if (compare_timestamps(unzipped_temp_file, temp_file) > 0) {
			// temp_file has been unzipped already and
			// orig_file has not changed in the meantime.
			temp_file = unzipped_temp_file;
			lyxerr[Debug::GRAPHICS]
				<< "\twas already unzipped to " << temp_file
				<< endl;
		} else {
			// unzipped_temp_file does not exist or is too old
			temp_file = unzipFile(temp_file);
			lyxerr[Debug::GRAPHICS]
				<< "\tunzipped to " << temp_file << endl;
		}
	}

	string const from = formats.getFormatFromFile(temp_file);
	if (from.empty()) {
		lyxerr[Debug::GRAPHICS]
			<< "\tCould not get file format." << endl;
		return orig_file;
	}
	string const to   = findTargetFormat(from, runparams);
	string const ext  = formats.extension(to);
	lyxerr[Debug::GRAPHICS]
		<< "\t we have: from " << from << " to " << to << '\n';

	// We're going to be running the exported buffer through the LaTeX
	// compiler, so must ensure that LaTeX can cope with the graphics
	// file format.

	lyxerr[Debug::GRAPHICS]
		<< "\tthe orig file is: " << orig_file << endl;

	if (from == to) {
		// The extension of temp_file might be != ext!
		runparams.exportdata->addExternalFile("latex", source_file,
		                                      output_file);
		runparams.exportdata->addExternalFile("dvi", source_file,
		                                      output_file);
		return stripExtensionIfPossible(output_file, to);
	}

	string const to_file = ChangeExtension(temp_file, ext);
	string const output_to_file = ChangeExtension(output_file, ext);

	// Do we need to perform the conversion?
	// Yes if to_file does not exist or if temp_file is newer than to_file
	if (compare_timestamps(temp_file, to_file) < 0) {
		lyxerr[Debug::GRAPHICS]
			<< bformat(_("No conversion of %1$s is needed after all"),
				   rel_file)
			<< std::endl;
		runparams.exportdata->addExternalFile("latex", to_file,
		                                      output_to_file);
		runparams.exportdata->addExternalFile("dvi", to_file,
		                                      output_to_file);
		return stripExtension(output_file);
	}

	lyxerr[Debug::GRAPHICS]
		<< "\tThe original file is " << orig_file << "\n"
		<< "\tA copy has been made and convert is to be called with:\n"
		<< "\tfile to convert = " << temp_file << '\n'
		<< "\t from " << from << " to " << to << '\n';

	// if no special converter defined, then we take the default one
	// from ImageMagic: convert from:inname.from to:outname.to
	if (converters.convert(&buf, temp_file, temp_file, from, to)) {
		runparams.exportdata->addExternalFile("latex",
				to_file, output_to_file);
		runparams.exportdata->addExternalFile("dvi",
				to_file, output_to_file);
	} else {
		string const command =
			"sh " + LibFileSearch("scripts", "convertDefault.sh") +
				' ' + formats.extension(from) + ':' + temp_file +
				' ' + ext + ':' + to_file;
		lyxerr[Debug::GRAPHICS]
			<< "No converter defined! I use convertDefault.sh:\n\t"
			<< command << endl;
		Systemcall one;
		one.startscript(Systemcall::Wait, command);
		if (IsFileReadable(to_file)) {
			runparams.exportdata->addExternalFile("latex",
					to_file, output_to_file);
			runparams.exportdata->addExternalFile("dvi",
					to_file, output_to_file);
		} else {
			string str = bformat(_("No information for converting %1$s "
				"format files to %2$s.\n"
				"Try defining a convertor in the preferences."), from, to);
			Alert::error(_("Could not convert image"), str);
		}
	}

	return stripExtension(output_file);
}


int InsetGraphics::latex(Buffer const & buf, ostream & os,
			 OutputParams const & runparams) const
{
	// If there is no file specified or not existing,
	// just output a message about it in the latex output.
	lyxerr[Debug::GRAPHICS]
		<< "insetgraphics::latex: Filename = "
		<< params().filename.absFilename() << endl;

	string const relative_file =
		params().filename.relFilename(buf.filePath());

	string const file_ = params().filename.absFilename();
	bool const file_exists = !file_.empty() && IsFileReadable(file_);
	string const message = file_exists ?
		string() : string("bb = 0 0 200 100, draft, type=eps");
	// if !message.empty() then there was no existing file
	// "filename" found. In this case LaTeX
	// draws only a rectangle with the above bb and the
	// not found filename in it.
	lyxerr[Debug::GRAPHICS]
		<< "\tMessage = \"" << message << '\"' << endl;

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
	lyxerr[Debug::GRAPHICS] << "\tOpts = " << opts << endl;

	if (!opts.empty() && !message.empty())
		before += ("[%\n" + opts + ',' + message + ']');
	else if (!opts.empty() || !message.empty())
		before += ("[%\n" + opts + message + ']');

	lyxerr[Debug::GRAPHICS]
		<< "\tBefore = " << before
		<< "\n\tafter = " << after << endl;


	string latex_str = before + '{';
	if (file_exists)
		// Convert the file if necessary.
		// Remove the extension so the LaTeX will use whatever
		// is appropriate (when there are several versions in
		// different formats)
		latex_str += prepareFile(buf, runparams);
	else
		latex_str += relative_file + " not found!";

	latex_str += '}' + after;
	os << latex_str;

	lyxerr[Debug::GRAPHICS] << "InsetGraphics::latex outputting:\n"
				<< latex_str << endl;
	// Return how many newlines we issued.
	return int(lyx::count(latex_str.begin(), latex_str.end(),'\n') + 1);
}


int InsetGraphics::plaintext(Buffer const &, ostream & os,
			 OutputParams const &) const
{
	// No graphics in ascii output. Possible to use gifscii to convert
	// images to ascii approximation.
	// 1. Convert file to ascii using gifscii
	// 2. Read ascii output file and add it to the output stream.
	// at least we send the filename
	os << '<' << bformat(_("Graphics file: %1$s"),
			     params().filename.absFilename()) << ">\n";
	return 0;
}


int InsetGraphics::linuxdoc(Buffer const & buf, ostream & os,
			    OutputParams const & runparams) const
{
	string const file_name = runparams.nice ?
				params().filename.relFilename(buf.filePath()):
				params().filename.absFilename();

	runparams.exportdata->addExternalFile("linuxdoc",
	                                      params().filename.absFilename());
	os << "<eps file=\"" << file_name << "\">\n";
	os << "<img src=\"" << file_name << "\">";
	return 0;
}


namespace {

int writeImageObject(char * format, ostream& os, OutputParams const & runparams,
					 string const graphic_label, string const attributes)
{
		if (runparams.flavor != OutputParams::XML) {
			os << "<![ %output.print." << format << "; [" << std::endl;
		}
		os <<"<imageobject><imagedata fileref=\"&"
		   << graphic_label << ";." << format << "\" " << attributes ;
		if (runparams.flavor == OutputParams::XML) {
			os <<  " role=\"" << format << "\"/>" ;
		}
		else {
			os << " format=\"" << format << "\">" ;
		}
		os << "</imageobject>";
		if (runparams.flavor != OutputParams::XML) {
			os << std::endl << "]]>" ;
		}
		return runparams.flavor == OutputParams::XML ? 0 : 2;
}
// end anonymous namespace
}


// For explanation on inserting graphics into DocBook checkout:
// http://en.tldp.org/LDP/LDP-Author-Guide/html/inserting-pictures.html
// See also the docbook guide at http://www.docbook.org/
int InsetGraphics::docbook(Buffer const &, ostream & os,
			   OutputParams const & runparams) const
{
	// In DocBook v5.0, the graphic tag will be eliminated from DocBook, will
	// need to switch to MediaObject. However, for now this is sufficient and
	// easier to use.
	if (runparams.flavor == OutputParams::XML) {
		runparams.exportdata->addExternalFile("docbook-xml",
						      params().filename.absFilename());
	} else {
		runparams.exportdata->addExternalFile("docbook",
						      params().filename.absFilename());
	}
	os << "<inlinemediaobject>";

	int r = 0;
	string attributes = createDocBookAttributes();
	r += writeImageObject("png", os, runparams, graphic_label, attributes);
	r += writeImageObject("pdf", os, runparams, graphic_label, attributes);
	r += writeImageObject("eps", os, runparams, graphic_label, attributes);
	r += writeImageObject("bmp", os, runparams, graphic_label, attributes);

	os << "</inlinemediaobject>";
	return r;
}


void InsetGraphics::validate(LaTeXFeatures & features) const
{
	// If we have no image, we should not require anything.
	if (params().filename.empty())
		return;

	features.includeFile(graphic_label,
			     RemoveExtension(params().filename.absFilename()));

	features.require("graphicx");

	if (features.nice()) {
		Buffer const * m_buffer = features.buffer().getMasterBuffer();
		string basename =
			params().filename.outputFilename(m_buffer->filePath());
		basename = RemoveExtension(basename);
		if(params().filename.isZipped())
			basename = RemoveExtension(basename);
		if (contains(basename, "."))
			features.require("lyxdot");
	}

	if (params().subcaption)
		features.require("subfigure");
}


bool InsetGraphics::setParams(InsetGraphicsParams const & p)
{
	// If nothing is changed, just return and say so.
	if (params() == p && !p.filename.empty())
		return false;

	// Copy the new parameters.
	params_ = p;

	// Update the display using the new parameters.
	graphic_->update(params().as_grfxParams());

	// We have changed data, report it.
	return true;
}


InsetGraphicsParams const & InsetGraphics::params() const
{
	return params_;
}


void InsetGraphics::editGraphics(InsetGraphicsParams const & p,
                                 Buffer const & buffer) const
{
	string const file_with_path = p.filename.absFilename();
	formats.edit(buffer, file_with_path,
	             formats.getFormatFromFile(file_with_path));
}


string const InsetGraphicsMailer::name_("graphics");

InsetGraphicsMailer::InsetGraphicsMailer(InsetGraphics & inset)
	: inset_(inset)
{}


string const InsetGraphicsMailer::inset2string(Buffer const & buffer) const
{
	return params2string(inset_.params(), buffer);
}


void InsetGraphicsMailer::string2params(string const & in,
					Buffer const & buffer,
					InsetGraphicsParams & params)
{
	params = InsetGraphicsParams();
	if (in.empty())
		return;

	istringstream data(in);
	LyXLex lex(0,0);
	lex.setStream(data);

	string name;
	lex >> name;
	if (!lex || name != name_)
		return print_mailer_error("InsetGraphicsMailer", in, 1, name_);

	InsetGraphics inset;
	inset.readInsetGraphics(lex, buffer.filePath());
	params = inset.params();
}


string const
InsetGraphicsMailer::params2string(InsetGraphicsParams const & params,
				   Buffer const & buffer)
{
	ostringstream data;
	data << name_ << ' ';
	params.Write(data, buffer.filePath());
	data << "\\end_inset\n";
	return data.str();
}
