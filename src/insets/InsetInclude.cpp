/**
 * \file InsetInclude.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetInclude.h"

#include "Buffer.h"
#include "buffer_funcs.h"
#include "BufferList.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Cursor.h"
#include "debug.h"
#include "DispatchResult.h"
#include "Exporter.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "LyX.h"
#include "LyXRC.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "TocBackend.h"

#include "frontends/alert.h"
#include "frontends/Painter.h"

#include "graphics/PreviewImage.h"
#include "graphics/PreviewLoader.h"

#include "insets/RenderPreview.h"
#include "insets/InsetListingsParams.h"

#include "support/filetools.h"
#include "support/lstrings.h" // contains
#include "support/lyxalgo.h"
#include "support/lyxlib.h"
#include "support/convert.h"

#include <boost/bind.hpp>
#include <boost/filesystem/operations.hpp>


namespace lyx {
  
// Implementation is in LyX.cpp
extern void dispatch(FuncRequest const & action);

using support::addName;
using support::absolutePath;
using support::bformat;
using support::changeExtension;
using support::contains;
using support::copy;
using support::doesFileExist;
using support::DocFileName;
using support::FileName;
using support::getFileContents;
using support::getVectorFromString;
using support::isFileReadable;
using support::isLyXFilename;
using support::isValidLaTeXFilename;
using support::latex_path;
using support::makeAbsPath;
using support::makeDisplayPath;
using support::makeRelPath;
using support::onlyFilename;
using support::onlyPath;
using support::prefixIs;
using support::subst;
using support::sum;

using std::endl;
using std::string;
using std::auto_ptr;
using std::istringstream;
using std::ostream;
using std::ostringstream;
using std::vector;

namespace Alert = frontend::Alert;
namespace fs = boost::filesystem;


namespace {

docstring const uniqueID()
{
	static unsigned int seed = 1000;
	return "file" + convert<docstring>(++seed);
}


bool isListings(InsetCommandParams const & params)
{
	return params.getCmdName() == "lstinputlisting";
}

} // namespace anon


InsetInclude::InsetInclude(InsetCommandParams const & p)
	: params_(p), include_label(uniqueID()),
	  preview_(new RenderMonitoredPreview(this)),
	  set_label_(false), counter_(0)
{
	preview_->fileChanged(boost::bind(&InsetInclude::fileChanged, this));
}


InsetInclude::InsetInclude(InsetInclude const & other)
	: Inset(other),
	  params_(other.params_),
	  include_label(other.include_label),
	  preview_(new RenderMonitoredPreview(this)),
	  set_label_(false), counter_(0)
{
	preview_->fileChanged(boost::bind(&InsetInclude::fileChanged, this));
}


InsetInclude::~InsetInclude()
{
	InsetIncludeMailer(*this).hideDialog();
}


void InsetInclude::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetCommandParams p("include");
		InsetIncludeMailer::string2params(to_utf8(cmd.argument()), p);
		if (!p.getCmdName().empty()) {
			if (isListings(p)){
				InsetListingsParams par_old(params().getOptions());
				InsetListingsParams par_new(p.getOptions());
				if (par_old.getParamValue("label") !=
				    par_new.getParamValue("label")
				    && !par_new.getParamValue("label").empty())
					cur.bv().buffer()->changeRefsIfUnique(
						from_utf8(par_old.getParamValue("label")),
						from_utf8(par_new.getParamValue("label")),
						Inset::REF_CODE);
			}
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
		if (!cur.selection())
			InsetIncludeMailer(*this).showDialog(&cur.bv());
		break;

	default:
		Inset::doDispatch(cur, cmd);
		break;
	}
}


bool InsetInclude::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY:
	case LFUN_INSET_DIALOG_UPDATE:
		flag.enabled(true);
		return true;

	default:
		return Inset::getStatus(cur, cmd, flag);
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
	VERBAST = 3,
	LISTINGS = 4,
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
	if  (command_name == "lstinputlisting")
		return LISTINGS;
	return INCLUDE;
}


bool isVerbatim(InsetCommandParams const & params)
{
	string const command_name = params.getCmdName();
	return command_name == "verbatiminput" ||
		command_name == "verbatiminput*";
}


bool isInputOrInclude(InsetCommandParams const & params)
{
	Types const t = type(params);
	return (t == INPUT) || (t == INCLUDE);
}


string const masterFilename(Buffer const & buffer)
{
	return buffer.getMasterBuffer()->fileName();
}


string const parentFilename(Buffer const & buffer)
{
	return buffer.fileName();
}


FileName const includedFilename(Buffer const & buffer,
			      InsetCommandParams const & params)
{
	return makeAbsPath(to_utf8(params["filename"]),
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


auto_ptr<Inset> InsetInclude::doClone() const
{
	return auto_ptr<Inset>(new InsetInclude(*this));
}


void InsetInclude::write(Buffer const &, ostream & os) const
{
	write(os);
}


void InsetInclude::write(ostream & os) const
{
	os << "Include " << to_utf8(params_.getCommand()) << '\n'
	   << "preview " << convert<string>(params_.preview()) << '\n';
}


void InsetInclude::read(Buffer const &, Lexer & lex)
{
	read(lex);
}


void InsetInclude::read(Lexer & lex)
{
	if (lex.isOK()) {
		lex.eatLine();
		string const command = lex.getString();
		params_.scanCommand(command);
	}
	string token;
	while (lex.isOK()) {
		lex.next();
		token = lex.getString();
		if (token == "\\end_inset")
			break;
		if (token == "preview") {
			lex.next();
			params_.preview(lex.getBool());
		} else
			lex.printError("Unknown parameter name `$$Token' for command " + params_.getCmdName());
	}
	if (token != "\\end_inset") {
		lex.printError("Missing \\end_inset at this point. "
			       "Read: `$$Token'");
	}
}


docstring const InsetInclude::getScreenLabel(Buffer const & buf) const
{
	docstring temp;

	switch (type(params_)) {
		case INPUT:
			temp += buf.B_("Input");
			break;
		case VERB:
			temp += buf.B_("Verbatim Input");
			break;
		case VERBAST:
			temp += buf.B_("Verbatim Input*");
			break;
		case INCLUDE:
			temp += buf.B_("Include");
			break;
		case LISTINGS: {
			if (counter_ > 0)
				temp += buf.B_("Program Listing ") + convert<docstring>(counter_);
			else
				temp += buf.B_("Program Listing");
			break;
		}
	}

	temp += ": ";

	if (params_["filename"].empty())
		temp += "???";
	else
		temp += from_utf8(onlyFilename(to_utf8(params_["filename"])));

	return temp;
}


namespace {

/// return the child buffer if the file is a LyX doc and is loaded
Buffer * getChildBuffer(Buffer const & buffer, InsetCommandParams const & params)
{
	if (isVerbatim(params) || isListings(params))
		return 0;

	string const included_file = includedFilename(buffer, params).absFilename();
	if (!isLyXFilename(included_file))
		return 0;

	Buffer * childBuffer = theBufferList().getBuffer(included_file);

	//FIXME RECURSIVE INCLUDES
	if (childBuffer == & buffer)
		return 0;
	else
		return childBuffer;
}


/// return true if the file is or got loaded.
bool loadIfNeeded(Buffer const & buffer, InsetCommandParams const & params)
{
	if (isVerbatim(params) || isListings(params))
		return false;

	FileName const included_file = includedFilename(buffer, params);
	if (!isLyXFilename(included_file.absFilename()))
		return false;

	Buffer * buf = theBufferList().getBuffer(included_file.absFilename());
	if (!buf) {
		// the readonly flag can/will be wrong, not anymore I think.
		if (!doesFileExist(included_file))
			return false;
		if (use_gui) {
			lyx::dispatch(FuncRequest(LFUN_BUFFER_CHILD_OPEN,
				included_file.absFilename() + "|true"));
			buf = theBufferList().getBuffer(included_file.absFilename());
		}
		else {
			buf = theBufferList().newBuffer(included_file.absFilename());
			if (!loadLyXFile(buf, included_file)) {
				//close the buffer we just opened
				theBufferList().close(buf, false);
				return false;
			}
		}
		return buf;
	}
	buf->setParentName(parentFilename(buffer));
	return true;
}


} // namespace anon


int InsetInclude::latex(Buffer const & buffer, odocstream & os,
			OutputParams const & runparams) const
{
	string incfile(to_utf8(params_["filename"]));

	// Do nothing if no file name has been specified
	if (incfile.empty())
		return 0;

	FileName const included_file(includedFilename(buffer, params_));

	//Check we're not trying to include ourselves.
	//FIXME RECURSIVE INCLUDE
	//This isn't sufficient, as the inclusion could be downstream.
	//But it'll have to do for now.
	if (isInputOrInclude(params_) &&
		buffer.fileName() == included_file.absFilename())
	{
		Alert::error(_("Recursive input"),
			       bformat(_("Attempted to include file %1$s in itself! "
			       "Ignoring inclusion."), from_utf8(incfile)));
		return 0;
	}

	Buffer const * const m_buffer = buffer.getMasterBuffer();

	// if incfile is relative, make it relative to the master
	// buffer directory.
	if (!absolutePath(incfile)) {
		// FIXME UNICODE
		incfile = to_utf8(makeRelPath(from_utf8(included_file.absFilename()),
					      from_utf8(m_buffer->filePath())));
	}

	// write it to a file (so far the complete file)
	string const exportfile = changeExtension(incfile, ".tex");
	string const mangled =
		DocFileName(changeExtension(included_file.absFilename(),".tex")).
			mangledFilename();
	FileName const writefile(makeAbsPath(mangled, m_buffer->temppath()));

	if (!runparams.nice)
		incfile = mangled;
	else if (!isValidLaTeXFilename(incfile)) {
		frontend::Alert::warning(_("Invalid filename"),
				         _("The following filename is likely to cause trouble "
					   "when running the exported file through LaTeX: ") +
					    from_utf8(incfile));
	}
	LYXERR(Debug::LATEX) << "incfile:" << incfile << endl;
	LYXERR(Debug::LATEX) << "exportfile:" << exportfile << endl;
	LYXERR(Debug::LATEX) << "writefile:" << writefile << endl;

	if (runparams.inComment || runparams.dryrun) {
		//Don't try to load or copy the file if we're
		//in a comment or doing a dryrun
	} else if (isInputOrInclude(params_) &&
		 isLyXFilename(included_file.absFilename())) {
		//if it's a LyX file and we're inputting or including,
		//try to load it so we can write the associated latex
		if (!loadIfNeeded(buffer, params_))
			return false;

		Buffer * tmp = theBufferList().getBuffer(included_file.absFilename());

		if (tmp->params().textclass != m_buffer->params().textclass) {
			// FIXME UNICODE
			docstring text = bformat(_("Included file `%1$s'\n"
						"has textclass `%2$s'\n"
							     "while parent file has textclass `%3$s'."),
					      makeDisplayPath(included_file.absFilename()),
					      from_utf8(tmp->params().getTextClass().name()),
					      from_utf8(m_buffer->params().getTextClass().name()));
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
		// The included file might be written in a different encoding
		Encoding const * const oldEnc = runparams.encoding;
		runparams.encoding = &tmp->params().encoding();
		tmp->makeLaTeXFile(writefile,
				   onlyPath(masterFilename(buffer)),
				   runparams, false);
		runparams.encoding = oldEnc;
	} else {
		// In this case, it's not a LyX file, so we copy the file
		// to the temp dir, so that .aux files etc. are not created
		// in the original dir. Files included by this file will be
		// found via input@path, see ../Buffer.cpp.
		unsigned long const checksum_in  = sum(included_file);
		unsigned long const checksum_out = sum(writefile);

		if (checksum_in != checksum_out) {
			if (!copy(included_file, writefile)) {
				// FIXME UNICODE
				LYXERR(Debug::LATEX)
					<< to_utf8(bformat(_("Could not copy the file\n%1$s\n"
								  "into the temporary directory."),
						   from_utf8(included_file.absFilename())))
					<< endl;
				return 0;
			}
		}
	}

	string const tex_format = (runparams.flavor == OutputParams::LATEX) ?
			"latex" : "pdflatex";
	if (isVerbatim(params_)) {
		incfile = latex_path(incfile);
		// FIXME UNICODE
		os << '\\' << from_ascii(params_.getCmdName()) << '{'
		   << from_utf8(incfile) << '}';
	} else if (type(params_) == INPUT) {
		runparams.exportdata->addExternalFile(tex_format, writefile,
						      exportfile);

		// \input wants file with extension (default is .tex)
		if (!isLyXFilename(included_file.absFilename())) {
			incfile = latex_path(incfile);
			// FIXME UNICODE
			os << '\\' << from_ascii(params_.getCmdName())
			   << '{' << from_utf8(incfile) << '}';
		} else {
		incfile = changeExtension(incfile, ".tex");
		incfile = latex_path(incfile);
			// FIXME UNICODE
			os << '\\' << from_ascii(params_.getCmdName())
			   << '{' << from_utf8(incfile) <<  '}';
		}
	} else if (type(params_) == LISTINGS) {
		os << '\\' << from_ascii(params_.getCmdName());
		string opt = params_.getOptions();
		// opt is set in QInclude dialog and should have passed validation.
		InsetListingsParams params(opt);
		if (!params.params().empty())
			os << "[" << from_utf8(params.params()) << "]";
		os << '{'  << from_utf8(incfile) << '}';
	} else {
		runparams.exportdata->addExternalFile(tex_format, writefile,
						      exportfile);

		// \include don't want extension and demands that the
		// file really have .tex
		incfile = changeExtension(incfile, string());
		incfile = latex_path(incfile);
		// FIXME UNICODE
		os << '\\' << from_ascii(params_.getCmdName()) << '{'
		   << from_utf8(incfile) << '}';
	}

	return 0;
}


int InsetInclude::plaintext(Buffer const & buffer, odocstream & os,
			    OutputParams const &) const
{
	if (isVerbatim(params_) || isListings(params_)) {
		os << '[' << getScreenLabel(buffer) << '\n';
		// FIXME: We don't know the encoding of the file
		docstring const str =
		     from_utf8(getFileContents(includedFilename(buffer, params_)));
		os << str;
		os << "\n]";
		return PLAINTEXT_NEWLINE + 1; // one char on a separate line
	} else {
		docstring const str = '[' + getScreenLabel(buffer) + ']';
		os << str;
		return str.size();
	}
}


int InsetInclude::docbook(Buffer const & buffer, odocstream & os,
			  OutputParams const & runparams) const
{
	string incfile = to_utf8(params_["filename"]);

	// Do nothing if no file name has been specified
	if (incfile.empty())
		return 0;

	string const included_file = includedFilename(buffer, params_).absFilename();

	//Check we're not trying to include ourselves.
	//FIXME RECURSIVE INCLUDE
	//This isn't sufficient, as the inclusion could be downstream.
	//But it'll have to do for now.
	if (buffer.fileName() == included_file) {
		Alert::error(_("Recursive input"),
			       bformat(_("Attempted to include file %1$s in itself! "
			       "Ignoring inclusion."), from_utf8(incfile)));
		return 0;
	}

	// write it to a file (so far the complete file)
	string const exportfile = changeExtension(incfile, ".sgml");
	DocFileName writefile(changeExtension(included_file, ".sgml"));

	if (loadIfNeeded(buffer, params_)) {
		Buffer * tmp = theBufferList().getBuffer(included_file);

		string const mangled = writefile.mangledFilename();
		writefile = makeAbsPath(mangled,
					buffer.getMasterBuffer()->temppath());
		if (!runparams.nice)
			incfile = mangled;

		LYXERR(Debug::LATEX) << "incfile:" << incfile << endl;
		LYXERR(Debug::LATEX) << "exportfile:" << exportfile << endl;
		LYXERR(Debug::LATEX) << "writefile:" << writefile << endl;

		tmp->makeDocBookFile(writefile, runparams, true);
	}

	runparams.exportdata->addExternalFile("docbook", writefile,
					      exportfile);
	runparams.exportdata->addExternalFile("docbook-xml", writefile,
					      exportfile);

	if (isVerbatim(params_) || isListings(params_)) {
		os << "<inlinegraphic fileref=\""
		   << '&' << include_label << ';'
		   << "\" format=\"linespecific\">";
	} else
		os << '&' << include_label << ';';

	return 0;
}


void InsetInclude::validate(LaTeXFeatures & features) const
{
	string incfile(to_utf8(params_["filename"]));
	string writefile;

	Buffer const & buffer = features.buffer();

	string const included_file = includedFilename(buffer, params_).absFilename();

	if (isLyXFilename(included_file))
		writefile = changeExtension(included_file, ".sgml");
	else
		writefile = included_file;

	if (!features.runparams().nice && !isVerbatim(params_) && !isListings(params_)) {
		incfile = DocFileName(writefile).mangledFilename();
		writefile = makeAbsPath(incfile,
					buffer.getMasterBuffer()->temppath()).absFilename();
	}

	features.includeFile(include_label, writefile);

	if (isVerbatim(params_))
		features.require("verbatim");
	else if (isListings(params_))
		features.require("listings");

	// Here we must do the fun stuff...
	// Load the file in the include if it needs
	// to be loaded:
	if (loadIfNeeded(buffer, params_)) {
		// a file got loaded
		Buffer * const tmp = theBufferList().getBuffer(included_file);
		// make sure the buffer isn't us
		// FIXME RECURSIVE INCLUDES
		// This is not sufficient, as recursive includes could be
		// more than a file away. But it will do for now.
		if (tmp && tmp != & buffer) {
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
	if (isListings(params_)) {
		InsetListingsParams params(params_.getOptions());
		string label = params.getParamValue("label");
		if (!label.empty())
			list.push_back(from_utf8(label));
	}
	else if (loadIfNeeded(buffer, params_)) {
		string const included_file = includedFilename(buffer, params_).absFilename();
		Buffer * tmp = theBufferList().getBuffer(included_file);
		tmp->setParentName("");
		tmp->getLabelList(list);
		tmp->setParentName(parentFilename(buffer));
	}
}


void InsetInclude::fillWithBibKeys(Buffer const & buffer,
		std::vector<std::pair<string, docstring> > & keys) const
{
	if (loadIfNeeded(buffer, params_)) {
		string const included_file = includedFilename(buffer, params_).absFilename();
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


std::vector<FileName> const &
InsetInclude::getBibfilesCache(Buffer const & buffer) const
{
	Buffer * const tmp = getChildBuffer(buffer, params_);
	if (tmp) {
		tmp->setParentName("");
		std::vector<FileName> const & cache = tmp->getBibfilesCache();
		tmp->setParentName(parentFilename(buffer));
		return cache;
	}
	static std::vector<FileName> const empty;
	return empty;
}


bool InsetInclude::metrics(MetricsInfo & mi, Dimension & dim) const
{
	BOOST_ASSERT(mi.base.bv && mi.base.bv->buffer());

	bool use_preview = false;
	if (RenderPreview::status() != LyXRC::PREVIEW_OFF) {
		graphics::PreviewImage const * pimage =
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

	bool const changed = dim_ != dim;
	dim_ = dim;
	return changed;
}


void InsetInclude::draw(PainterInfo & pi, int x, int y) const
{
	setPosCache(pi, x, y);

	BOOST_ASSERT(pi.base.bv && pi.base.bv->buffer());

	bool use_preview = false;
	if (RenderPreview::status() != LyXRC::PREVIEW_OFF) {
		graphics::PreviewImage const * pimage =
			preview_->getPreviewImage(*pi.base.bv->buffer());
		use_preview = pimage && pimage->image();
	}

	if (use_preview)
		preview_->draw(pi, x, y);
	else
		button_.draw(pi, x, y);
}


Inset::DisplayType InsetInclude::display() const
{
	return type(params_) == INPUT ? Inline : AlignCenter;
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
	FileName const included_file = includedFilename(buffer, params);

	return type(params) == INPUT && params.preview() &&
		isFileReadable(included_file);
}


docstring const latex_string(InsetInclude const & inset, Buffer const & buffer)
{
	odocstringstream os;
	// We don't need to set runparams.encoding since this will be done
	// by latex() anyway.
	OutputParams runparams(0);
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
		docstring const snippet = latex_string(inset, buffer);
		renderer.addPreview(snippet, buffer);
	}
}

} // namespace anon


void InsetInclude::addPreview(graphics::PreviewLoader & ploader) const
{
	Buffer const & buffer = ploader.buffer();
	if (preview_wanted(params(), buffer)) {
		preview_->setAbsFile(includedFilename(buffer, params()));
		docstring const snippet = latex_string(*this, buffer);
		preview_->addPreview(snippet, ploader);
	}
}


void InsetInclude::addToToc(TocList & toclist, Buffer const & buffer, ParConstIterator const & pit) const
{
	if (isListings(params_)) {
		InsetListingsParams params(params_.getOptions());
		string caption = params.getParamValue("caption");
		if (!caption.empty()) {
			Toc & toc = toclist["listing"];
			docstring const str = convert<docstring>(toc.size() + 1)
				+ ". " +  from_utf8(caption);
			// This inset does not have a valid ParConstIterator 
			// so it has to use the iterator of its parent paragraph
			toc.push_back(TocItem(pit, 0, str));
		}
		return;
	}
	Buffer * childbuffer = getChildBuffer(buffer, params_);
	if (!childbuffer)
		return;

	childbuffer->tocBackend().update();
	TocList const & childtoclist = childbuffer->tocBackend().tocs();
	TocList::const_iterator it = childtoclist.begin();
	TocList::const_iterator const end = childtoclist.end();
	for(; it != end; ++it)
		toclist[it->first].insert(toclist[it->first].end(),
				it->second.begin(), it->second.end());
}


void InsetInclude::updateLabels(Buffer const & buffer) const
{
	Buffer const * const childbuffer = getChildBuffer(buffer, params_);
	if (!childbuffer)
		return;

	lyx::updateLabels(*childbuffer, true);
}


void InsetInclude::updateCounter(Counters & counters)
{
	if (!isListings(params_))
		return;

	InsetListingsParams const par = params_.getOptions();
	if (par.getParamValue("caption").empty())
		counter_ = 0;
	else {
		counters.step(from_ascii("listing"));
		counter_ = counters.value(from_ascii("listing"));
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
	Lexer lex(0,0);
	lex.setStream(data);

	string name;
	lex >> name;
	if (!lex || name != name_)
		return print_mailer_error("InsetIncludeMailer", in, 1, name_);

	// This is part of the inset proper that is usually swallowed
	// by Text::readInset
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


} // namespace lyx
