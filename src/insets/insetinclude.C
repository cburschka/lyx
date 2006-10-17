/**
 * \file insetinclude.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetinclude.h"

#include "buffer.h"
#include "buffer_funcs.h"
#include "bufferlist.h"
#include "bufferparams.h"
#include "BufferView.h"
#include "cursor.h"
#include "debug.h"
#include "dispatchresult.h"
#include "exporter.h"
#include "funcrequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "lyx_main.h"
#include "lyxrc.h"
#include "lyxlex.h"
#include "metricsinfo.h"
#include "outputparams.h"

#include "frontends/Alert.h"
#include "frontends/Painter.h"

#include "graphics/PreviewImage.h"
#include "graphics/PreviewLoader.h"

#include "insets/render_preview.h"

#include "support/filename.h"
#include "support/filetools.h"
#include "support/lstrings.h" // contains
#include "support/lyxalgo.h"
#include "support/lyxlib.h"
#include "support/convert.h"

#include <boost/bind.hpp>
#include <boost/filesystem/operations.hpp>

#include "support/std_ostream.h"

#include <sstream>

using lyx::docstring;
using lyx::support::addName;
using lyx::support::absolutePath;
using lyx::support::bformat;
using lyx::support::changeExtension;
using lyx::support::contains;
using lyx::support::copy;
using lyx::support::FileName;
using lyx::support::getFileContents;
using lyx::support::isFileReadable;
using lyx::support::isLyXFilename;
using lyx::support::latex_path;
using lyx::support::makeAbsPath;
using lyx::support::makeDisplayPath;
using lyx::support::makeRelPath;
using lyx::support::onlyFilename;
using lyx::support::onlyPath;
using lyx::support::subst;
using lyx::support::sum;

using std::endl;
using std::string;
using std::auto_ptr;
using std::istringstream;
using std::ostream;
using std::ostringstream;

namespace Alert = lyx::frontend::Alert;
namespace fs = boost::filesystem;


namespace {

string const uniqueID()
{
	static unsigned int seed = 1000;
	return "file" + convert<string>(++seed);
}

} // namespace anon


InsetInclude::InsetInclude(InsetCommandParams const & p)
	: params_(p), include_label(uniqueID()),
	  preview_(new RenderMonitoredPreview(this)),
	  set_label_(false)
{
	preview_->fileChanged(boost::bind(&InsetInclude::fileChanged, this));
}


InsetInclude::InsetInclude(InsetInclude const & other)
	: InsetOld(other),
	  params_(other.params_),
	  include_label(other.include_label),
	  preview_(new RenderMonitoredPreview(this)),
	  set_label_(false)
{
	preview_->fileChanged(boost::bind(&InsetInclude::fileChanged, this));
}


InsetInclude::~InsetInclude()
{
	InsetIncludeMailer(*this).hideDialog();
}


void InsetInclude::doDispatch(LCursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetCommandParams p("include");
		InsetIncludeMailer::string2params(lyx::to_utf8(cmd.argument()), p);
		if (!p.getCmdName().empty()) {
			set(p, cur.buffer());
			cur.buffer().updateBibfilesCache();
		} else
			cur.noUpdate();
		break;
	}

	case LFUN_INSET_DIALOG_UPDATE:
		InsetIncludeMailer(*this).updateDialog(&cur.bv());
		break;

	case LFUN_MOUSE_RELEASE:
		InsetIncludeMailer(*this).showDialog(&cur.bv());
		break;

	default:
		InsetBase::doDispatch(cur, cmd);
		break;
	}
}


bool InsetInclude::getStatus(LCursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY:
	case LFUN_INSET_DIALOG_UPDATE:
		flag.enabled(true);
		return true;

	default:
		return InsetBase::getStatus(cur, cmd, flag);
	}
}


InsetCommandParams const & InsetInclude::params() const
{
	return params_;
}


namespace {

/// the type of inclusion
enum Types {
	INCLUDE = 0,
	VERB = 1,
	INPUT = 2,
	VERBAST = 3
};


Types type(InsetCommandParams const & params)
{
	string const command_name = params.getCmdName();

	if (command_name == "input")
		return INPUT;
	if  (command_name == "verbatiminput")
		return VERB;
	if  (command_name == "verbatiminput*")
		return VERBAST;
	return INCLUDE;
}


bool isVerbatim(InsetCommandParams const & params)
{
	string const command_name = params.getCmdName();
	return command_name == "verbatiminput" ||
		command_name == "verbatiminput*";
}


string const masterFilename(Buffer const & buffer)
{
	return buffer.getMasterBuffer()->fileName();
}


string const parentFilename(Buffer const & buffer)
{
	return buffer.fileName();
}


string const includedFilename(Buffer const & buffer,
			      InsetCommandParams const & params)
{
	return makeAbsPath(params.getContents(),
			   onlyPath(parentFilename(buffer)));
}


void add_preview(RenderMonitoredPreview &, InsetInclude const &, Buffer const &);

} // namespace anon


void InsetInclude::set(InsetCommandParams const & p, Buffer const & buffer)
{
	params_ = p;
	set_label_ = false;

	if (preview_->monitoring())
		preview_->stopMonitoring();

	if (type(params_) == INPUT)
		add_preview(*preview_, *this, buffer);
}


auto_ptr<InsetBase> InsetInclude::doClone() const
{
	return auto_ptr<InsetBase>(new InsetInclude(*this));
}


void InsetInclude::write(Buffer const &, ostream & os) const
{
	write(os);
}


void InsetInclude::write(ostream & os) const
{
	os << "Include " << params_.getCommand() << '\n'
	   << "preview " << convert<string>(params_.preview()) << '\n';
}


void InsetInclude::read(Buffer const &, LyXLex & lex)
{
	read(lex);
}


void InsetInclude::read(LyXLex & lex)
{
	params_.read(lex);
}


docstring const InsetInclude::getScreenLabel(Buffer const &) const
{
	docstring temp;

	switch (type(params_)) {
		case INPUT:
			temp += _("Input");
			break;
		case VERB:
			temp += _("Verbatim Input");
			break;
		case VERBAST:
			temp += _("Verbatim Input*");
			break;
		case INCLUDE:
			temp += _("Include");
			break;
	}

	temp += ": ";

	if (params_.getContents().empty())
		temp += "???";
	else
		// FIXME: We don't know the encoding of the filename
		temp += lyx::from_ascii(onlyFilename(params_.getContents()));

	return temp;
}


namespace {

/// return the child buffer if the file is a LyX doc and is loaded
Buffer * getChildBuffer(Buffer const & buffer, InsetCommandParams const & params)
{
	if (isVerbatim(params))
		return 0;

	string const included_file = includedFilename(buffer, params);
	if (!isLyXFilename(included_file))
		return 0;

	return theBufferList().getBuffer(included_file);
}


/// return true if the file is or got loaded.
bool loadIfNeeded(Buffer const & buffer, InsetCommandParams const & params)
{
	if (isVerbatim(params))
		return false;

	string const included_file = includedFilename(buffer, params);
	if (!isLyXFilename(included_file))
		return false;

	Buffer * buf = theBufferList().getBuffer(included_file);
	if (!buf) {
		// the readonly flag can/will be wrong, not anymore I think.
		if (!fs::exists(included_file))
			return false;
		buf = theBufferList().newBuffer(included_file);
		if (!loadLyXFile(buf, included_file))
			return false;
	}
	if (buf)
		buf->setParentName(parentFilename(buffer));
	return buf != 0;
}


} // namespace anon


int InsetInclude::latex(Buffer const & buffer, ostream & os,
			OutputParams const & runparams) const
{
	string incfile(params_.getContents());

	// Do nothing if no file name has been specified
	if (incfile.empty())
		return 0;

	string const included_file = includedFilename(buffer, params_);
	Buffer const * const m_buffer = buffer.getMasterBuffer();

	// if incfile is relative, make it relative to the master
	// buffer directory.
	if (!absolutePath(incfile)) {
		incfile = makeRelPath(included_file,
				      m_buffer->filePath());
	}

	// write it to a file (so far the complete file)
	string const exportfile = changeExtension(incfile, ".tex");
	string const mangled = FileName(changeExtension(included_file,
							".tex")).mangledFilename();
	string const writefile = makeAbsPath(mangled, m_buffer->temppath());

	if (!runparams.nice)
		incfile = mangled;
	lyxerr[Debug::LATEX] << "incfile:" << incfile << endl;
	lyxerr[Debug::LATEX] << "exportfile:" << exportfile << endl;
	lyxerr[Debug::LATEX] << "writefile:" << writefile << endl;

	if (runparams.inComment || runparams.dryrun)
		// Don't try to load or copy the file
		;
	else if (loadIfNeeded(buffer, params_)) {
		Buffer * tmp = theBufferList().getBuffer(included_file);

		if (tmp->params().textclass != m_buffer->params().textclass) {
			// FIXME UNICODE
			docstring text = bformat(_("Included file `%1$s'\n"
						"has textclass `%2$s'\n"
							     "while parent file has textclass `%3$s'."),
					      makeDisplayPath(included_file),
					      lyx::from_utf8(tmp->params().getLyXTextClass().name()),
					      lyx::from_utf8(m_buffer->params().getLyXTextClass().name()));
			Alert::warning(_("Different textclasses"), text);
			//return 0;
		}

		tmp->markDepClean(m_buffer->temppath());

#ifdef WITH_WARNINGS
#warning handle non existing files
#warning Second argument is irrelevant!
// since only_body is true, makeLaTeXFile will not look at second
// argument. Should we set it to string(), or should makeLaTeXFile
// make use of it somehow? (JMarc 20031002)
#endif
		tmp->makeLaTeXFile(writefile,
				   onlyPath(masterFilename(buffer)),
				   runparams, false);
	} else {
		// Copy the file to the temp dir, so that .aux files etc.
		// are not created in the original dir. Files included by
		// this file will be found via input@path, see ../buffer.C.
		unsigned long const checksum_in  = sum(included_file);
		unsigned long const checksum_out = sum(writefile);

		if (checksum_in != checksum_out) {
			if (!copy(included_file, writefile)) {
				// FIXME UNICODE
				lyxerr[Debug::LATEX]
					<< lyx::to_utf8(bformat(_("Could not copy the file\n%1$s\n"
								  "into the temporary directory."),
						   lyx::from_utf8(included_file)))
					<< endl;
				return 0;
			}
		}
	}

	string const tex_format = (runparams.flavor == OutputParams::LATEX) ?
			"latex" : "pdflatex";
	if (isVerbatim(params_)) {
		incfile = latex_path(incfile);
		os << '\\' << params_.getCmdName() << '{' << incfile << '}';
	} else if (type(params_) == INPUT) {
		runparams.exportdata->addExternalFile(tex_format, writefile,
						      exportfile);

		// \input wants file with extension (default is .tex)
		if (!isLyXFilename(included_file)) {
			incfile = latex_path(incfile);
			os << '\\' << params_.getCmdName() << '{' << incfile << '}';
		} else {
		incfile = changeExtension(incfile, ".tex");
		incfile = latex_path(incfile);
			os << '\\' << params_.getCmdName() << '{'
			   << incfile
			   <<  '}';
		}
	} else {
		runparams.exportdata->addExternalFile(tex_format, writefile,
						      exportfile);

		// \include don't want extension and demands that the
		// file really have .tex
		incfile = changeExtension(incfile, string());
		incfile = latex_path(incfile);
		os << '\\' << params_.getCmdName() << '{'
		   << incfile
		   << '}';
	}

	return 0;
}


int InsetInclude::plaintext(Buffer const & buffer, lyx::odocstream & os,
			OutputParams const &) const
{
	if (isVerbatim(params_)) {
		// FIXME: We don't know the encoding of the file
		docstring const str = lyx::from_utf8(
			getFileContents(includedFilename(buffer, params_)));
		os << str;
		// Return how many newlines we issued.
		return int(lyx::count(str.begin(), str.end(), '\n'));
	}
	return 0;
}


int InsetInclude::docbook(Buffer const & buffer, ostream & os,
			  OutputParams const & runparams) const
{
	string incfile(params_.getContents());

	// Do nothing if no file name has been specified
	if (incfile.empty())
		return 0;

	string const included_file = includedFilename(buffer, params_);

	// write it to a file (so far the complete file)
	string const exportfile = changeExtension(incfile, ".sgml");
	string writefile = changeExtension(included_file, ".sgml");

	if (loadIfNeeded(buffer, params_)) {
		Buffer * tmp = theBufferList().getBuffer(included_file);

		string const mangled = FileName(writefile).mangledFilename();
		writefile = makeAbsPath(mangled,
					buffer.getMasterBuffer()->temppath());
		if (!runparams.nice)
			incfile = mangled;

		lyxerr[Debug::LATEX] << "incfile:" << incfile << endl;
		lyxerr[Debug::LATEX] << "exportfile:" << exportfile << endl;
		lyxerr[Debug::LATEX] << "writefile:" << writefile << endl;

		tmp->makeDocBookFile(writefile, runparams, true);
	}

	runparams.exportdata->addExternalFile("docbook", writefile,
					      exportfile);
	runparams.exportdata->addExternalFile("docbook-xml", writefile,
					      exportfile);

	if (isVerbatim(params_)) {
		os << "<inlinegraphic fileref=\""
		   << '&' << include_label << ';'
		   << "\" format=\"linespecific\">";
	} else
		os << '&' << include_label << ';';

	return 0;
}


void InsetInclude::validate(LaTeXFeatures & features) const
{
	string incfile(params_.getContents());
	string writefile;

	Buffer const & buffer = features.buffer();

	string const included_file = includedFilename(buffer, params_);

	if (isLyXFilename(included_file))
		writefile = changeExtension(included_file, ".sgml");
	else
		writefile = included_file;

	if (!features.runparams().nice && !isVerbatim(params_)) {
		incfile = FileName(writefile).mangledFilename();
		writefile = makeAbsPath(incfile,
					buffer.getMasterBuffer()->temppath());
	}

	features.includeFile(include_label, writefile);

	if (isVerbatim(params_))
		features.require("verbatim");

	// Here we must do the fun stuff...
	// Load the file in the include if it needs
	// to be loaded:
	if (loadIfNeeded(buffer, params_)) {
		// a file got loaded
		Buffer * const tmp = theBufferList().getBuffer(included_file);
		if (tmp) {
			// We must temporarily change features.buffer,
			// otherwise it would always be the master buffer,
			// and nested includes would not work.
			features.setBuffer(*tmp);
			tmp->validate(features);
			features.setBuffer(buffer);
		}
	}
}


void InsetInclude::getLabelList(Buffer const & buffer,
				std::vector<docstring> & list) const
{
	if (loadIfNeeded(buffer, params_)) {
		string const included_file = includedFilename(buffer, params_);
		Buffer * tmp = theBufferList().getBuffer(included_file);
		tmp->setParentName("");
		tmp->getLabelList(list);
		tmp->setParentName(parentFilename(buffer));
	}
}


void InsetInclude::fillWithBibKeys(Buffer const & buffer,
				   std::vector<std::pair<string,string> > & keys) const
{
	if (loadIfNeeded(buffer, params_)) {
		string const included_file = includedFilename(buffer, params_);
		Buffer * tmp = theBufferList().getBuffer(included_file);
		tmp->setParentName("");
		tmp->fillWithBibKeys(keys);
		tmp->setParentName(parentFilename(buffer));
	}
}


void InsetInclude::updateBibfilesCache(Buffer const & buffer)
{
	Buffer * const tmp = getChildBuffer(buffer, params_);
	if (tmp) {
		tmp->setParentName("");
		tmp->updateBibfilesCache();
		tmp->setParentName(parentFilename(buffer));
	}
}


std::vector<string> const &
InsetInclude::getBibfilesCache(Buffer const & buffer) const
{
	Buffer * const tmp = getChildBuffer(buffer, params_);
	if (tmp) {
		tmp->setParentName("");
		std::vector<string> const & cache = tmp->getBibfilesCache();
		tmp->setParentName(parentFilename(buffer));
		return cache;
	}
	static std::vector<string> const empty;
	return empty;
}


void InsetInclude::metrics(MetricsInfo & mi, Dimension & dim) const
{
	BOOST_ASSERT(mi.base.bv && mi.base.bv->buffer());

	bool use_preview = false;
	if (RenderPreview::status() != LyXRC::PREVIEW_OFF) {
		lyx::graphics::PreviewImage const * pimage =
			preview_->getPreviewImage(*mi.base.bv->buffer());
		use_preview = pimage && pimage->image();
	}

	if (use_preview) {
		preview_->metrics(mi, dim);
	} else {
		if (!set_label_) {
			set_label_ = true;
			button_.update(getScreenLabel(*mi.base.bv->buffer()),
				       true);
		}
		button_.metrics(mi, dim);
	}

	Box b(0, dim.wid, -dim.asc, dim.des);
	button_.setBox(b);

	dim_ = dim;
}


void InsetInclude::draw(PainterInfo & pi, int x, int y) const
{
	setPosCache(pi, x, y);

	BOOST_ASSERT(pi.base.bv && pi.base.bv->buffer());

	bool use_preview = false;
	if (RenderPreview::status() != LyXRC::PREVIEW_OFF) {
		lyx::graphics::PreviewImage const * pimage =
			preview_->getPreviewImage(*pi.base.bv->buffer());
		use_preview = pimage && pimage->image();
	}

	if (use_preview)
		preview_->draw(pi, x, y);
	else
		button_.draw(pi, x, y);
}

bool InsetInclude::display() const
{
	return type(params_) != INPUT;
}



//
// preview stuff
//

void InsetInclude::fileChanged() const
{
	Buffer const * const buffer_ptr = LyX::cref().updateInset(this);
	if (!buffer_ptr)
		return;

	Buffer const & buffer = *buffer_ptr;
	preview_->removePreview(buffer);
	add_preview(*preview_.get(), *this, buffer);
	preview_->startLoading(buffer);
}


namespace {

bool preview_wanted(InsetCommandParams const & params, Buffer const & buffer)
{
	string const included_file = includedFilename(buffer, params);

	return type(params) == INPUT && params.preview() &&
		isFileReadable(included_file);
}


string const latex_string(InsetInclude const & inset, Buffer const & buffer)
{
	ostringstream os;
	OutputParams runparams;
	runparams.flavor = OutputParams::LATEX;
	inset.latex(buffer, os, runparams);

	return os.str();
}


void add_preview(RenderMonitoredPreview & renderer, InsetInclude const & inset,
		 Buffer const & buffer)
{
	InsetCommandParams const & params = inset.params();
	if (RenderPreview::status() != LyXRC::PREVIEW_OFF &&
	    preview_wanted(params, buffer)) {
		renderer.setAbsFile(includedFilename(buffer, params));
		string const snippet = latex_string(inset, buffer);
		renderer.addPreview(snippet, buffer);
	}
}

} // namespace anon


void InsetInclude::addPreview(lyx::graphics::PreviewLoader & ploader) const
{
	Buffer const & buffer = ploader.buffer();
	if (preview_wanted(params(), buffer)) {
		preview_->setAbsFile(includedFilename(buffer, params()));
		string const snippet = latex_string(*this, buffer);
		preview_->addPreview(snippet, ploader);
	}
}


string const InsetIncludeMailer::name_("include");

InsetIncludeMailer::InsetIncludeMailer(InsetInclude & inset)
	: inset_(inset)
{}


string const InsetIncludeMailer::inset2string(Buffer const &) const
{
	return params2string(inset_.params());
}


void InsetIncludeMailer::string2params(string const & in,
				       InsetCommandParams & params)
{
	params.clear();
	if (in.empty())
		return;

	istringstream data(in);
	LyXLex lex(0,0);
	lex.setStream(data);

	string name;
	lex >> name;
	if (!lex || name != name_)
		return print_mailer_error("InsetIncludeMailer", in, 1, name_);

	// This is part of the inset proper that is usually swallowed
	// by LyXText::readInset
	string id;
	lex >> id;
	if (!lex || id != "Include")
		return print_mailer_error("InsetIncludeMailer", in, 2, "Include");

	InsetInclude inset(params);
	inset.read(lex);
	params = inset.params();
}


string const
InsetIncludeMailer::params2string(InsetCommandParams const & params)
{
	InsetInclude inset(params);
	ostringstream data;
	data << name_ << ' ';
	inset.write(data);
	data << "\\end_inset\n";
	return data.str();
}
