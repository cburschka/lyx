/**
 * \file InsetGraphics.cpp
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

#include "insets/InsetGraphics.h"
#include "insets/RenderGraphic.h"

#include "Buffer.h"
#include "BufferView.h"
#include "Converter.h"
#include "Cursor.h"
#include "debug.h"
#include "DispatchResult.h"
#include "ErrorList.h"
#include "Exporter.h"
#include "Format.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "Length.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "Mover.h"
#include "OutputParams.h"
#include "sgml.h"

#include "frontends/alert.h"

#include "support/convert.h"
#include "support/filetools.h"
#include "support/lyxalgo.h" // count
#include "support/lyxlib.h" // sum
#include "support/lstrings.h"
#include "support/os.h"
#include "support/Systemcall.h"

#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>

#include <sstream>


namespace lyx {

using support::bformat;
using support::changeExtension;
using support::compare_timestamps;
using support::contains;
using support::DocFileName;
using support::FileName;
using support::float_equal;
using support::getExtension;
using support::isFileReadable;
using support::isValidLaTeXFilename;
using support::latex_path;
using support::onlyFilename;
using support::removeExtension;
using support::rtrim;
using support::subst;
using support::suffixIs;
using support::Systemcall;
using support::unzipFile;
using support::unzippedFileName;

using std::endl;
using std::string;
using std::auto_ptr;
using std::istringstream;
using std::ostream;
using std::ostringstream;


namespace {

/// Find the most suitable image format for images in \p format
/// Note that \p format may be unknown (i. e. an empty string)
string findTargetFormat(string const & format, OutputParams const & runparams)
{
	// Are we using latex or pdflatex?
	if (runparams.flavor == OutputParams::PDFLATEX) {
		LYXERR(Debug::GRAPHICS) << "findTargetFormat: PDF mode" << endl;
		Format const * const f = formats.getFormat(format);
		// Convert vector graphics to pdf
		if (f && f->vectorFormat())
			return "pdf";
		// pdflatex can use jpeg, png and pdf directly
		if (format == "jpg")
			return format;
		// Convert everything else to png
		return "png";
	}
	// If it's postscript, we always do eps.
	LYXERR(Debug::GRAPHICS) << "findTargetFormat: PostScript mode" << endl;
	if (format != "ps")
		// any other than ps is changed to eps
		return "eps";
	// let ps untouched
	return format;
}

} // namespace anon


InsetGraphics::InsetGraphics()
	: graphic_label(sgml::uniqueID(from_ascii("graph"))),
	  graphic_(new RenderGraphic(this))
{}


InsetGraphics::InsetGraphics(InsetGraphics const & ig)
	: Inset(ig),
	  boost::signals::trackable(),
		graphic_label(sgml::uniqueID(from_ascii("graph"))),
	  graphic_(new RenderGraphic(*ig.graphic_, this))
{
	setParams(ig.params());
}


auto_ptr<Inset> InsetGraphics::doClone() const
{
	return auto_ptr<Inset>(new InsetGraphics(*this));
}


InsetGraphics::~InsetGraphics()
{
	InsetGraphicsMailer(*this).hideDialog();
}


void InsetGraphics::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {
	case LFUN_GRAPHICS_EDIT: {
		Buffer const & buffer = *cur.bv().buffer();
		InsetGraphicsParams p;
		InsetGraphicsMailer::string2params(to_utf8(cmd.argument()), buffer, p);
		editGraphics(p, buffer);
		break;
	}

	case LFUN_INSET_MODIFY: {
		Buffer const & buffer = cur.buffer();
		InsetGraphicsParams p;
		InsetGraphicsMailer::string2params(to_utf8(cmd.argument()), buffer, p);
		if (!p.filename.empty())
			setParams(p);
		else
			cur.noUpdate();
		break;
	}

	case LFUN_INSET_DIALOG_UPDATE:
		InsetGraphicsMailer(*this).updateDialog(&cur.bv());
		break;

	case LFUN_MOUSE_RELEASE:
		if (!cur.selection())
			InsetGraphicsMailer(*this).showDialog(&cur.bv());
		break;

	default:
		Inset::doDispatch(cur, cmd);
		break;
	}
}


bool InsetGraphics::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action) {
	case LFUN_GRAPHICS_EDIT:
	case LFUN_INSET_MODIFY:
	case LFUN_INSET_DIALOG_UPDATE:
		flag.enabled(true);
		return true;

	default:
		return Inset::getStatus(cur, cmd, flag);
	}
}


void InsetGraphics::edit(Cursor & cur, bool)
{
	InsetGraphicsMailer(*this).showDialog(&cur.bv());
}


bool InsetGraphics::metrics(MetricsInfo & mi, Dimension & dim) const
{
	graphic_->metrics(mi, dim);
	bool const changed = dim_ != dim;
	dim_ = dim;
	return changed;
}


void InsetGraphics::draw(PainterInfo & pi, int x, int y) const
{
	setPosCache(pi, x, y);
	graphic_->draw(pi, x, y);
}


Inset::EDITABLE InsetGraphics::editable() const
{
	return IS_EDITABLE;
}


void InsetGraphics::write(Buffer const & buf, ostream & os) const
{
	os << "Graphics\n";
	params().Write(os, buf.filePath());
}


void InsetGraphics::read(Buffer const & buf, Lexer & lex)
{
	string const token = lex.getString();

	if (token == "Graphics")
		readInsetGraphics(lex, buf.filePath());
	else
		LYXERR(Debug::GRAPHICS) << "Not a Graphics inset!" << endl;

	graphic_->update(params().as_grfxParams());
}


void InsetGraphics::readInsetGraphics(Lexer & lex, string const & bufpath)
{
	bool finished = false;

	while (lex.isOK() && !finished) {
		lex.next();

		string const token = lex.getString();
		LYXERR(Debug::GRAPHICS) << "Token: '" << token << '\''
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
	    options << "bb=" << rtrim(params().bb) << ',';
	if (params().draft)
	    options << "draft,";
	if (params().clip)
	    options << "clip,";
	ostringstream size;
	double const scl = convert<double>(params().scale);
	if (!params().scale.empty() && !float_equal(scl, 0.0, 0.05)) {
		if (!float_equal(scl, 100.0, 0.05))
			size << "scale=" << scl / 100.0 << ',';
	} else {
		if (!params().width.zero())
			size << "width=" << params().width.asLatexString() << ',';
		if (!params().height.zero())
			size << "height=" << params().height.asLatexString() << ',';
		if (params().keepAspectRatio)
			size << "keepaspectratio,";
	}
	if (params().scaleBeforeRotation && !size.str().empty())
		options << size.str();

	// Make sure rotation angle is not very close to zero;
	// a float can be effectively zero but not exactly zero.
	if (!params().rotateAngle.empty()
		&& !float_equal(convert<double>(params().rotateAngle), 0.0, 0.001)) {
	    options << "angle=" << params().rotateAngle << ',';
	    if (!params().rotateOrigin.empty()) {
		options << "origin=" << params().rotateOrigin[0];
		if (contains(params().rotateOrigin,"Top"))
		    options << 't';
		else if (contains(params().rotateOrigin,"Bottom"))
		    options << 'b';
		else if (contains(params().rotateOrigin,"Baseline"))
		    options << 'B';
		options << ',';
	    }
	}
	if (!params().scaleBeforeRotation && !size.str().empty())
		options << size.str();

	if (!params().special.empty())
	    options << params().special << ',';

	string opts = options.str();
	// delete last ','
	if (suffixIs(opts, ','))
		opts = opts.substr(0, opts.size() - 1);

	return opts;
}


docstring const InsetGraphics::toDocbookLength(Length const & len) const
{
	odocstringstream result;
	switch (len.unit()) {
		case Length::SP: // Scaled point (65536sp = 1pt) TeX's smallest unit.
			result << len.value() * 65536.0 * 72 / 72.27 << "pt";
			break;
		case Length::PT: // Point = 1/72.27in = 0.351mm
			result << len.value() * 72 / 72.27 << "pt";
			break;
		case Length::BP: // Big point (72bp = 1in), also PostScript point
			result << len.value() << "pt";
			break;
		case Length::DD: // Didot point = 1/72 of a French inch, = 0.376mm
			result << len.value() * 0.376 << "mm";
			break;
		case Length::MM: // Millimeter = 2.845pt
			result << len.value() << "mm";
			break;
		case Length::PC: // Pica = 12pt = 4.218mm
			result << len.value() << "pc";
			break;
		case Length::CC: // Cicero = 12dd = 4.531mm
			result << len.value() * 4.531 << "mm";
			break;
		case Length::CM: // Centimeter = 10mm = 2.371pc
			result << len.value() << "cm";
			break;
		case Length::IN: // Inch = 25.4mm = 72.27pt = 6.022pc
			result << len.value() << "in";
			break;
		case Length::EX: // Height of a small "x" for the current font.
			// Obviously we have to compromise here. Any better ratio than 1.5 ?
			result << len.value() / 1.5 << "em";
			break;
		case Length::EM: // Width of capital "M" in current font.
			result << len.value() << "em";
			break;
		case Length::MU: // Math unit (18mu = 1em) for positioning in math mode
			result << len.value() * 18 << "em";
			break;
		case Length::PTW: // Percent of TextWidth
		case Length::PCW: // Percent of ColumnWidth
		case Length::PPW: // Percent of PageWidth
		case Length::PLW: // Percent of LineWidth
		case Length::PTH: // Percent of TextHeight
		case Length::PPH: // Percent of Paper
			// Sigh, this will go wrong.
			result << len.value() << "%";
			break;
		default:
			result << len.asDocstring();
			break;
	}
	return result.str();
}

docstring const InsetGraphics::createDocBookAttributes() const
{
	// Calculate the options part of the command, we must do it to a string
	// stream since we copied the code from createLatexParams() ;-)

	// FIXME: av: need to translate spec -> Docbook XSL spec (http://www.sagehill.net/docbookxsl/ImageSizing.html)
	// Right now it only works with my version of db2latex :-)

	odocstringstream options;
	double const scl = convert<double>(params().scale);
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
		options << from_ascii(params().special) << " ";

	// trailing blanks are ok ...
	return options.str();
}


namespace {

enum CopyStatus {
	SUCCESS,
	FAILURE,
	IDENTICAL_PATHS,
	IDENTICAL_CONTENTS
};


std::pair<CopyStatus, FileName> const
copyFileIfNeeded(FileName const & file_in, FileName const & file_out)
{
	unsigned long const checksum_in  = support::sum(file_in);
	unsigned long const checksum_out = support::sum(file_out);

	if (checksum_in == checksum_out)
		// Nothing to do...
		return std::make_pair(IDENTICAL_CONTENTS, file_out);

	Mover const & mover = getMover(formats.getFormatFromFile(file_in));
	bool const success = mover.copy(file_in, file_out);
	if (!success) {
		// FIXME UNICODE
		LYXERR(Debug::GRAPHICS)
			<< to_utf8(support::bformat(_("Could not copy the file\n%1$s\n"
							   "into the temporary directory."),
						from_utf8(file_in.absFilename())))
			<< std::endl;
	}

	CopyStatus status = success ? SUCCESS : FAILURE;
	return std::make_pair(status, file_out);
}


std::pair<CopyStatus, FileName> const
copyToDirIfNeeded(DocFileName const & file, string const & dir)
{
	using support::rtrim;

	string const file_in = file.absFilename();
	string const only_path = support::onlyPath(file_in);
	if (rtrim(support::onlyPath(file_in) , "/") == rtrim(dir, "/"))
		return std::make_pair(IDENTICAL_PATHS, file_in);

	string mangled = file.mangledFilename();
	if (file.isZipped()) {
		// We need to change _eps.gz to .eps.gz. The mangled name is
		// still unique because of the counter in mangledFilename().
		// We can't just call mangledFilename() with the zip
		// extension removed, because base.eps and base.eps.gz may
		// have different content but would get the same mangled
		// name in this case.
		string const base = removeExtension(file.unzippedFilename());
		string::size_type const ext_len = file_in.length() - base.length();
		mangled[mangled.length() - ext_len] = '.';
	}
	FileName const file_out(support::makeAbsPath(mangled, dir));

	return copyFileIfNeeded(file, file_out);
}


string const stripExtensionIfPossible(string const & file, bool nice)
{
	// Remove the extension so the LaTeX compiler will use whatever
	// is appropriate (when there are several versions in different
	// formats).
	// Do this only if we are not exporting for internal usage, because
	// pdflatex prefers png over pdf and it would pick up the png images
	// that we generate for preview.
	// This works only if the filename contains no dots besides
	// the just removed one. We can fool here by replacing all
	// dots with a macro whose definition is just a dot ;-)
	// The automatic format selection does not work if the file
	// name is escaped.
	string const latex_name = latex_path(file,
					     support::EXCLUDE_EXTENSION);
	if (!nice || contains(latex_name, '"'))
		return latex_name;
	return latex_path(removeExtension(file),
			  support::PROTECT_EXTENSION,
			  support::ESCAPE_DOTS);
}


string const stripExtensionIfPossible(string const & file, string const & to, bool nice)
{
	// No conversion is needed. LaTeX can handle the graphic file as is.
	// This is true even if the orig_file is compressed.
	string const to_format = formats.getFormat(to)->extension();
	string const file_format = getExtension(file);
	// for latex .ps == .eps
	if (to_format == file_format ||
	    (to_format == "eps" && file_format ==  "ps") ||
	    (to_format ==  "ps" && file_format == "eps"))
		return stripExtensionIfPossible(file, nice);
	return latex_path(file, support::EXCLUDE_EXTENSION);
}

} // namespace anon


string const InsetGraphics::prepareFile(Buffer const & buf,
					OutputParams const & runparams) const
{
	// The following code depends on non-empty filenames
	if (params().filename.empty())
		return string();

	string const orig_file = params().filename.absFilename();
	string const rel_file = params().filename.relFilename(buf.filePath());

	// previewing source code, no file copying or file format conversion
	if (runparams.dryrun)
		return stripExtensionIfPossible(rel_file, runparams.nice);

	// temp_file will contain the file for LaTeX to act on if, for example,
	// we move it to a temp dir or uncompress it.
	FileName temp_file = params().filename;

	// The master buffer. This is useful when there are multiple levels
	// of include files
	Buffer const * m_buffer = buf.getMasterBuffer();

	// Return the output name if we are inside a comment or the file does
	// not exist.
	// We are not going to change the extension or using the name of the
	// temporary file, the code is already complicated enough.
	if (runparams.inComment || !isFileReadable(params().filename))
		return params().filename.outputFilename(m_buffer->filePath());

	// We place all temporary files in the master buffer's temp dir.
	// This is possible because we use mangled file names.
	// This is necessary for DVI export.
	string const temp_path = m_buffer->temppath();

	CopyStatus status;
	boost::tie(status, temp_file) =
			copyToDirIfNeeded(params().filename, temp_path);

	if (status == FAILURE)
		return orig_file;

	// a relative filename should be relative to the master
	// buffer.
	// "nice" means that the buffer is exported to LaTeX format but not
	//        run through the LaTeX compiler.
	string output_file = support::os::external_path(runparams.nice ?
		params().filename.outputFilename(m_buffer->filePath()) :
		onlyFilename(temp_file.absFilename()));

	if (runparams.nice && !isValidLaTeXFilename(output_file)) {
		frontend::Alert::warning(_("Invalid filename"),
				         _("The following filename is likely to cause trouble "
					   "when running the exported file through LaTeX: ") +
					    from_utf8(output_file));
	}

	FileName source_file = runparams.nice ? FileName(params().filename) : temp_file;
	string const tex_format = (runparams.flavor == OutputParams::LATEX) ?
			"latex" : "pdflatex";

	// If the file is compressed and we have specified that it
	// should not be uncompressed, then just return its name and
	// let LaTeX do the rest!
	if (params().filename.isZipped()) {
		if (params().noUnzip) {
			// We don't know whether latex can actually handle
			// this file, but we can't check, because that would
			// mean to unzip the file and thereby making the
			// noUnzip parameter meaningless.
			LYXERR(Debug::GRAPHICS)
				<< "\tpass zipped file to LaTeX.\n";

			FileName const bb_orig_file = FileName(changeExtension(orig_file, "bb"));
			if (runparams.nice) {
				runparams.exportdata->addExternalFile(tex_format,
						bb_orig_file,
						changeExtension(output_file, "bb"));
			} else {
				// LaTeX needs the bounding box file in the
				// tmp dir
				FileName bb_file = FileName(changeExtension(temp_file.absFilename(), "bb"));
				boost::tie(status, bb_file) =
					copyFileIfNeeded(bb_orig_file, bb_file);
				if (status == FAILURE)
					return orig_file;
				runparams.exportdata->addExternalFile(tex_format,
						bb_file);
			}
			runparams.exportdata->addExternalFile(tex_format,
					source_file, output_file);
			runparams.exportdata->addExternalFile("dvi",
					source_file, output_file);
			// We can't strip the extension, because we don't know
			// the unzipped file format
			return latex_path(output_file,
					  support::EXCLUDE_EXTENSION);
		}

		FileName const unzipped_temp_file =
			FileName(unzippedFileName(temp_file.absFilename()));
		output_file = unzippedFileName(output_file);
		source_file = FileName(unzippedFileName(source_file.absFilename()));
		if (compare_timestamps(unzipped_temp_file, temp_file) > 0) {
			// temp_file has been unzipped already and
			// orig_file has not changed in the meantime.
			temp_file = unzipped_temp_file;
			LYXERR(Debug::GRAPHICS)
				<< "\twas already unzipped to " << temp_file
				<< endl;
		} else {
			// unzipped_temp_file does not exist or is too old
			temp_file = unzipFile(temp_file);
			LYXERR(Debug::GRAPHICS)
				<< "\tunzipped to " << temp_file << endl;
		}
	}

	string const from = formats.getFormatFromFile(temp_file);
	if (from.empty()) {
		LYXERR(Debug::GRAPHICS)
			<< "\tCould not get file format." << endl;
	}
	string const to   = findTargetFormat(from, runparams);
	string const ext  = formats.extension(to);
	LYXERR(Debug::GRAPHICS)
		<< "\t we have: from " << from << " to " << to << '\n';

	// We're going to be running the exported buffer through the LaTeX
	// compiler, so must ensure that LaTeX can cope with the graphics
	// file format.

	LYXERR(Debug::GRAPHICS)
		<< "\tthe orig file is: " << orig_file << endl;

	if (from == to) {
		if (!runparams.nice && getExtension(temp_file.absFilename()) != ext) {
			// The LaTeX compiler will not be able to determine
			// the file format from the extension, so we must
			// change it.
			FileName const new_file = FileName(changeExtension(temp_file.absFilename(), ext));
			if (support::rename(temp_file, new_file)) {
				temp_file = new_file;
				output_file = changeExtension(output_file, ext);
				source_file = FileName(changeExtension(source_file.absFilename(), ext));
			} else
				LYXERR(Debug::GRAPHICS)
					<< "Could not rename file `"
					<< temp_file << "' to `" << new_file
					<< "'." << endl;
		}
		// The extension of temp_file might be != ext!
		runparams.exportdata->addExternalFile(tex_format, source_file,
						      output_file);
		runparams.exportdata->addExternalFile("dvi", source_file,
						      output_file);
		return stripExtensionIfPossible(output_file, to, runparams.nice);
	}

	FileName const to_file = FileName(changeExtension(temp_file.absFilename(), ext));
	string const output_to_file = changeExtension(output_file, ext);

	// Do we need to perform the conversion?
	// Yes if to_file does not exist or if temp_file is newer than to_file
	if (compare_timestamps(temp_file, to_file) < 0) {
		// FIXME UNICODE
		LYXERR(Debug::GRAPHICS)
			<< to_utf8(bformat(_("No conversion of %1$s is needed after all"),
				   from_utf8(rel_file)))
			<< std::endl;
		runparams.exportdata->addExternalFile(tex_format, to_file,
						      output_to_file);
		runparams.exportdata->addExternalFile("dvi", to_file,
						      output_to_file);
		return stripExtensionIfPossible(output_to_file, runparams.nice);
	}

	LYXERR(Debug::GRAPHICS)
		<< "\tThe original file is " << orig_file << "\n"
		<< "\tA copy has been made and convert is to be called with:\n"
		<< "\tfile to convert = " << temp_file << '\n'
		<< "\t from " << from << " to " << to << '\n';

	// FIXME (Abdel 12/08/06): Is there a need to show these errors?
	ErrorList el;
	if (theConverters().convert(&buf, temp_file, to_file, params().filename,
			       from, to, el,
			       Converters::try_default | Converters::try_cache)) {
		runparams.exportdata->addExternalFile(tex_format,
				to_file, output_to_file);
		runparams.exportdata->addExternalFile("dvi",
				to_file, output_to_file);
	}

	return stripExtensionIfPossible(output_to_file, runparams.nice);
}


int InsetGraphics::latex(Buffer const & buf, odocstream & os,
			 OutputParams const & runparams) const
{
	// If there is no file specified or not existing,
	// just output a message about it in the latex output.
	LYXERR(Debug::GRAPHICS)
		<< "insetgraphics::latex: Filename = "
		<< params().filename.absFilename() << endl;

	string const relative_file =
		params().filename.relFilename(buf.filePath());

	bool const file_exists = !params().filename.empty() &&
				 isFileReadable(params().filename);
	string const message = file_exists ?
		string() : string("bb = 0 0 200 100, draft, type=eps");
	// if !message.empty() then there was no existing file
	// "filename" found. In this case LaTeX
	// draws only a rectangle with the above bb and the
	// not found filename in it.
	LYXERR(Debug::GRAPHICS)
		<< "\tMessage = \"" << message << '\"' << endl;

	// These variables collect all the latex code that should be before and
	// after the actual includegraphics command.
	string before;
	string after;
	// Do we want subcaptions?
	if (params().subcaption) {
		if (runparams.moving_arg)
			before += "\\protect";
		before += "\\subfigure[" + params().subcaptionText + "]{";
		after = '}';
	}

	if (runparams.moving_arg)
		before += "\\protect";

	// We never use the starred form, we use the "clip" option instead.
	before += "\\includegraphics";

	// Write the options if there are any.
	string const opts = createLatexOptions();
	LYXERR(Debug::GRAPHICS) << "\tOpts = " << opts << endl;

	if (!opts.empty() && !message.empty())
		before += ('[' + opts + ',' + message + ']');
	else if (!opts.empty() || !message.empty())
		before += ('[' + opts + message + ']');

	LYXERR(Debug::GRAPHICS)
		<< "\tBefore = " << before
		<< "\n\tafter = " << after << endl;

	string latex_str = before + '{';
	// Convert the file if necessary.
	// Remove the extension so LaTeX will use whatever is appropriate
	// (when there are several versions in different formats)
	latex_str += prepareFile(buf, runparams);
	latex_str += '}' + after;
	// FIXME UNICODE
	os << from_utf8(latex_str);

	LYXERR(Debug::GRAPHICS) << "InsetGraphics::latex outputting:\n"
				<< latex_str << endl;
	// Return how many newlines we issued.
	return int(lyx::count(latex_str.begin(), latex_str.end(),'\n'));
}


int InsetGraphics::plaintext(Buffer const & buf, odocstream & os,
			     OutputParams const &) const
{
	// No graphics in ascii output. Possible to use gifscii to convert
	// images to ascii approximation.
	// 1. Convert file to ascii using gifscii
	// 2. Read ascii output file and add it to the output stream.
	// at least we send the filename
	// FIXME UNICODE
	// FIXME: We have no idea what the encoding of the filename is

	docstring const str = bformat(buf.B_("Graphics file: %1$s"),
				      from_utf8(params().filename.absFilename()));
	os << '<' << str << '>';

	return 2 + str.size();
}


namespace {

int writeImageObject(char const * format,
                     odocstream & os,
                     OutputParams const & runparams,
		     docstring const & graphic_label,
                     docstring const & attributes)
{
		if (runparams.flavor != OutputParams::XML) {
			os << "<![ %output.print."
			   << format
			   << "; ["
			   << std::endl;
		}
		os <<"<imageobject><imagedata fileref=\"&"
		   << graphic_label
		   << ";."
		   << format
		   << "\" "
		   << attributes;
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
int InsetGraphics::docbook(Buffer const &, odocstream & os,
			   OutputParams const & runparams) const
{
	// In DocBook v5.0, the graphic tag will be eliminated from DocBook, will
	// need to switch to MediaObject. However, for now this is sufficient and
	// easier to use.
	if (runparams.flavor == OutputParams::XML) {
		runparams.exportdata->addExternalFile("docbook-xml",
						      params().filename);
	} else {
		runparams.exportdata->addExternalFile("docbook",
						      params().filename);
	}
	os << "<inlinemediaobject>";

	int r = 0;
	docstring attributes = createDocBookAttributes();
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
			     removeExtension(params().filename.absFilename()));

	features.require("graphicx");

	if (features.runparams().nice) {
		Buffer const * m_buffer = features.buffer().getMasterBuffer();
		string const rel_file = removeExtension(params().filename.relFilename(m_buffer->filePath()));
		if (contains(rel_file, "."))
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
	formats.edit(buffer, p.filename,
		     formats.getFormatFromFile(p.filename));
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
	Lexer lex(0,0);
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


} // namespace lyx
