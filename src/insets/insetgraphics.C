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
#include "format.h"
#include "funcrequest.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "lyx_main.h"
#include "lyxlex.h"
#include "lyxrc.h"
#include "metricsinfo.h"
#include "outputparams.h"

#include "frontends/Alert.h"
#include "frontends/LyXView.h"

#include "support/filetools.h"
#include "support/lyxalgo.h" // lyx::count
#include "support/lyxlib.h" // float_equal
#include "support/os.h"
#include "support/systemcall.h"
#include "support/tostr.h"
#include "support/std_sstream.h"

#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>

namespace support = lyx::support;
using lyx::support::AbsolutePath;
using lyx::support::bformat;
using lyx::support::ChangeExtension;
using lyx::support::contains;
using lyx::support::FileName;
using lyx::support::float_equal;
using lyx::support::GetExtension;
using lyx::support::getExtFromContents;
using lyx::support::IsFileReadable;
using lyx::support::LibFileSearch;
using lyx::support::rtrim;
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

///////////////////////////////////////////////////////////////////////////
int const VersionNumber = 1;
///////////////////////////////////////////////////////////////////////////

// This function is a utility function
// ... that should be with ChangeExtension ...
inline
string const RemoveExtension(string const & filename)
{
	return ChangeExtension(filename, string());
}


string const uniqueID()
{
	static unsigned int seed = 1000;
	return "graph" + tostr(++seed);
}


string findTargetFormat(string const & suffix, OutputParams const & runparams)
{
	// Are we using latex or pdflatex).
	if (runparams.flavor == OutputParams::PDFLATEX) {
		lyxerr[Debug::GRAPHICS] << "findTargetFormat: PDF mode" << endl;
		if (contains(suffix, "ps") || suffix == "pdf")
			return "pdf";
		if (suffix == "jpg")	// pdflatex can use jpeg
			return suffix;
		return "png";         // and also png
	}
	// If it's postscript, we always do eps.
	lyxerr[Debug::GRAPHICS] << "findTargetFormat: PostScript mode" << endl;
	if (suffix != "ps")     // any other than ps
		return "eps";         // is changed to eps
	return suffix;          // let ps untouched
}

} // namespace anon


InsetGraphics::InsetGraphics()
	: graphic_label(uniqueID()),
	  graphic_(new RenderGraphic)
{
	graphic_->connect(boost::bind(&InsetGraphics::statusChanged, this));
}


InsetGraphics::InsetGraphics(InsetGraphics const & ig)
	: InsetOld(ig),
	  boost::signals::trackable(),
	  graphic_label(uniqueID()),
	  graphic_(new RenderGraphic(*ig.graphic_))
{
	graphic_->connect(boost::bind(&InsetGraphics::statusChanged, this));
	setParams(ig.params());
}


auto_ptr<InsetBase> InsetGraphics::clone() const
{
	return auto_ptr<InsetBase>(new InsetGraphics(*this));
}


InsetGraphics::~InsetGraphics()
{
	InsetGraphicsMailer(*this).hideDialog();
}


void InsetGraphics::statusChanged() const
{
	LyX::cref().updateInset(this);
}


void InsetGraphics::priv_dispatch(LCursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {
	case LFUN_INSET_MODIFY: {
		Buffer const & buffer = *cur.bv().buffer();
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
		InsetOld::priv_dispatch(cur, cmd);
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


InsetOld::EDITABLE InsetGraphics::editable() const
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
	    options << "  bb=" << rtrim(params().bb) << ",\n";
	if (params().draft)
	    options << "  draft,\n";
	if (params().clip)
	    options << "  clip,\n";
	if (!float_equal(params().scale, 0.0, 0.05)) {
		if (!float_equal(params().scale, 100.0, 0.05))
			options << "  scale=" << params().scale / 100.0
				<< ",\n";
	} else {
		if (!params().width.zero())
			options << "  width=" << params().width.asLatexString() << ",\n";
		if (!params().height.zero())
			options << "  height=" << params().height.asLatexString() << ",\n";
		if (params().keepAspectRatio)
			options << "  keepaspectratio,\n";
	}

	// Make sure rotation angle is not very close to zero;
	// a float can be effectively zero but not exactly zero.
	if (!float_equal(params().rotateAngle, 0, 0.001)) {
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


namespace {

enum CopyStatus {
	SUCCESS,
	FAILURE,
	IDENTICAL_PATHS,
	IDENTICAL_CONTENTS
};


std::pair<CopyStatus, string> const
copyToDirIfNeeded(string const & file_in, string const & dir)
{
	using support::rtrim;

	BOOST_ASSERT(AbsolutePath(file_in));

	string const only_path = support::OnlyPath(file_in);
	if (rtrim(support::OnlyPath(file_in) , "/") == rtrim(dir, "/"))
		return std::make_pair(IDENTICAL_PATHS, file_in);

	string mangled;
	if (support::zippedFile(file_in)) {
		string const ext = GetExtension(file_in);
		string const unzipped = support::unzippedFileName(file_in);
		mangled = FileName(unzipped).mangledFilename();
		mangled += "." + ext;
	} else
		mangled = FileName(file_in).mangledFilename();

	string const file_out = support::MakeAbsPath(mangled, dir);

	unsigned long const checksum_in  = support::sum(file_in);
	unsigned long const checksum_out = support::sum(file_out);

	if (checksum_in == checksum_out)
		// Nothing to do...
		return std::make_pair(IDENTICAL_CONTENTS, file_out);

	bool const success = support::copy(file_in, file_out);
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


string const stripExtensionIfPossible(string const & file, string const & to)
{
	// No conversion is needed. LaTeX can handle the graphic file as is.
	// This is true even if the orig_file is compressed.
	if (formats.getFormat(to)->extension() == GetExtension(file))
		return RemoveExtension(file);
	return file;
}

} // namespace anon


string const InsetGraphics::prepareFile(Buffer const & buf,
					OutputParams const & runparams) const
{
	string orig_file = params().filename.absFilename();
	string const rel_file = params().filename.relFilename(buf.filePath());

	// LaTeX can cope if the graphics file doesn't exist, so just return the
	// filename.
	if (!IsFileReadable(orig_file)) {
		lyxerr[Debug::GRAPHICS]
			<< "InsetGraphics::prepareFile\n"
			<< "No file '" << orig_file << "' can be found!" << endl;
		return rel_file;
	}

	// If the file is compressed and we have specified that it
	// should not be uncompressed, then just return its name and
	// let LaTeX do the rest!
	bool const zipped = params().filename.isZipped();

	if (zipped && params().noUnzip) {
		lyxerr[Debug::GRAPHICS]
			<< "\tpass zipped file to LaTeX but with full path.\n";
		// LaTeX needs an absolute path, otherwise the
		// coresponding *.eps.bb file isn't found
		return orig_file;
	}

	// temp_file will contain the file for LaTeX to act on if, for example,
	// we move it to a temp dir or uncompress it.
	string temp_file = orig_file;

	if (zipped) {
		CopyStatus status;
		boost::tie(status, temp_file) =
			copyToDirIfNeeded(orig_file, buf.temppath());

		if (status == FAILURE)
			return orig_file;

		orig_file = unzippedFileName(temp_file);
		if (!IsFileReadable(orig_file)) {
			unzipFile(temp_file);
			lyxerr[Debug::GRAPHICS]
				<< "\tunzipped to " << orig_file << endl;
		}
	}

	string const from = getExtFromContents(orig_file);
	string const to   = findTargetFormat(from, runparams);
	lyxerr[Debug::GRAPHICS]
		<< "\t we have: from " << from << " to " << to << '\n';

	// We're going to be running the exported buffer through the LaTeX
	// compiler, so must ensure that LaTeX can cope with the graphics
	// file format.

	lyxerr[Debug::GRAPHICS]
		<< "\tthe orig file is: " << orig_file << endl;

	bool conversion_needed = true;
	CopyStatus status;
	boost::tie(status, temp_file) =
			copyToDirIfNeeded(orig_file, buf.temppath());

	if (status == FAILURE)
		return orig_file;
	else if (status == IDENTICAL_CONTENTS)
		conversion_needed = false;

	if (from == to)
		return stripExtensionIfPossible(temp_file, to);

	string const to_file_base = RemoveExtension(temp_file);
	string const to_file = ChangeExtension(to_file_base, to);

	// Do we need to perform the conversion?
	// Yes if to_file does not exist or if temp_file is newer than to_file
	if (!conversion_needed ||
	    support::compare_timestamps(temp_file, to_file) < 0) {
		lyxerr[Debug::GRAPHICS]
			<< bformat(_("No conversion of %1$s is needed after all"),
				   rel_file)
			<< std::endl;
		return to_file_base;
	}

	lyxerr[Debug::GRAPHICS]
		<< "\tThe original file is " << orig_file << "\n"
		<< "\tA copy has been made and convert is to be called with:\n"
		<< "\tfile to convert = " << temp_file << '\n'
		<< "\tto_file_base = " << to_file_base << '\n'
		<< "\t from " << from << " to " << to << '\n';

	// if no special converter defined, then we take the default one
	// from ImageMagic: convert from:inname.from to:outname.to
	if (!converters.convert(&buf, temp_file, to_file_base, from, to)) {
		string const command =
			"sh " + LibFileSearch("scripts", "convertDefault.sh") +
				' ' + from + ':' + temp_file + ' ' +
				to + ':' + to_file_base + '.' + to;
		lyxerr[Debug::GRAPHICS]
			<< "No converter defined! I use convertDefault.sh:\n\t"
			<< command << endl;
		Systemcall one;
		one.startscript(Systemcall::Wait, command);
		if (!IsFileReadable(ChangeExtension(to_file_base, to))) {
			string str = bformat(_("No information for converting %1$s "
				"format files to %2$s.\n"
				"Try defining a convertor in the preferences."), from, to);
			Alert::error(_("Could not convert image"), str);
		}
	}

	return to_file_base;
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

	// A missing (e)ps-extension is no problem for LaTeX, so
	// we have to test three different cases
#warning uh, but can our cache handle it ? no.
	string const file_ = params().filename.absFilename();
	bool const file_exists =
		!file_.empty() &&
		(IsFileReadable(file_) ||		// original
		 IsFileReadable(file_ + ".eps") ||	// original.eps
		 IsFileReadable(file_ + ".ps"));	// original.ps
	string const message = file_exists ?
		string() : string("bb = 0 0 200 100, draft, type=eps");
	// if !message.empty() than there was no existing file
	// "filename(.(e)ps)" found. In this case LaTeX
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


	// "nice" means that the buffer is exported to LaTeX format but not
	//        run through the LaTeX compiler.
	if (runparams.nice) {
		os << before <<'{' << relative_file << '}' << after;
		return 1;
	}

	// Make the filename relative to the lyx file
	// and remove the extension so the LaTeX will use whatever is
	// appropriate (when there are several versions in different formats)
	string const latex_str = message.empty() ?
		(before + '{' + os::external_path(prepareFile(buf, runparams)) + '}' + after) :
		(before + '{' + relative_file + " not found!}" + after);
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

	os << "<eps file=\"" << file_name << "\">\n";
	os << "<img src=\"" << file_name << "\">";
	return 0;
}


// For explanation on inserting graphics into DocBook checkout:
// http://en.tldp.org/LDP/LDP-Author-Guide/inserting-pictures.html
// See also the docbook guide at http://www.docbook.org/
int InsetGraphics::docbook(Buffer const &, ostream & os,
			   OutputParams const &) const
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
		return;

	features.includeFile(graphic_label,
			     RemoveExtension(params().filename.absFilename()));

	features.require("graphicx");

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
