/**
 * \file InsetInclude.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Richard Heck (conversion to InsetCommand)
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
#include "DispatchResult.h"
#include "ErrorList.h"
#include "Exporter.h"
#include "Format.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LaTeXFeatures.h"
#include "LayoutFile.h"
#include "LayoutModuleList.h"
#include "LyX.h"
#include "LyXFunc.h"
#include "LyXRC.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "TextClass.h"
#include "TocBackend.h"

#include "frontends/alert.h"
#include "frontends/Painter.h"

#include "graphics/PreviewImage.h"
#include "graphics/PreviewLoader.h"

#include "insets/InsetLabel.h"
#include "insets/InsetListingsParams.h"
#include "insets/RenderPreview.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/docstream.h"
#include "support/FileNameList.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h" // contains
#include "support/lyxalgo.h"

#include <boost/bind.hpp>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace Alert = frontend::Alert;


namespace {

docstring const uniqueID()
{
	static unsigned int seed = 1000;
	return "file" + convert<docstring>(++seed);
}


/// the type of inclusion
enum Types {
	INCLUDE, VERB, INPUT, VERBAST, LISTINGS, NONE
};


Types type(string const & s)
{
	if (s == "input")
		return INPUT;
	if (s == "verbatiminput")
		return VERB;
	if (s == "verbatiminput*")
		return VERBAST;
	if (s == "lstinputlisting")
		return LISTINGS;
	if (s == "include")
		return INCLUDE;
	return NONE;
}


Types type(InsetCommandParams const & params)
{
	return type(params.getCmdName());
}


bool isListings(InsetCommandParams const & params)
{
	return type(params) == LISTINGS;
}


bool isVerbatim(InsetCommandParams const & params)
{
	Types const t = type(params);
	return t == VERB || t == VERBAST;
}


bool isInputOrInclude(InsetCommandParams const & params)
{
	Types const t = type(params);
 	return t == INPUT || t == INCLUDE;
}


FileName const masterFileName(Buffer const & buffer)
{
	return buffer.masterBuffer()->fileName();
}


void add_preview(RenderMonitoredPreview &, InsetInclude const &, Buffer const &);


string const parentFilename(Buffer const & buffer)
{
	return buffer.absFileName();
}


FileName const includedFilename(Buffer const & buffer,
			      InsetCommandParams const & params)
{
	return makeAbsPath(to_utf8(params["filename"]),
			onlyPath(parentFilename(buffer)));
}


InsetLabel * createLabel(docstring const & label_str)
{
	if (label_str.empty())
		return 0;
	InsetCommandParams icp(LABEL_CODE);
	icp["name"] = label_str;
	return new InsetLabel(icp);
}

} // namespace anon


InsetInclude::InsetInclude(InsetCommandParams const & p)
	: InsetCommand(p, "include"), include_label(uniqueID()),
	  preview_(new RenderMonitoredPreview(this)), failedtoload_(false),
	  set_label_(false), label_(0), child_buffer_(0)
{
	preview_->fileChanged(boost::bind(&InsetInclude::fileChanged, this));

	if (isListings(params())) {
		InsetListingsParams listing_params(to_utf8(p["lstparams"]));
		label_ = createLabel(from_utf8(listing_params.getParamValue("label")));
	}
}


InsetInclude::InsetInclude(InsetInclude const & other)
	: InsetCommand(other), include_label(other.include_label),
	  preview_(new RenderMonitoredPreview(this)), failedtoload_(false),
	  set_label_(false), label_(0), child_buffer_(0)
{
	preview_->fileChanged(boost::bind(&InsetInclude::fileChanged, this));

	if (other.label_)
		label_ = new InsetLabel(*other.label_);
}


InsetInclude::~InsetInclude()
{
	delete label_;
}


void InsetInclude::setBuffer(Buffer & buffer)
{
	InsetCommand::setBuffer(buffer);
	if (label_)
		label_->setBuffer(buffer);
}


ParamInfo const & InsetInclude::findInfo(string const & /* cmdName */)
{
	// FIXME
	// This is only correct for the case of listings, but it'll do for now.
	// In the other cases, this second parameter should just be empty.
	static ParamInfo param_info_;
	if (param_info_.empty()) {
		param_info_.add("filename", ParamInfo::LATEX_REQUIRED);
		param_info_.add("lstparams", ParamInfo::LATEX_OPTIONAL);
	}
	return param_info_;
}


bool InsetInclude::isCompatibleCommand(string const & s)
{
	return type(s) != NONE;
}


void InsetInclude::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	LASSERT(cur.buffer() == &buffer(), return);
	switch (cmd.action) {

	case LFUN_INSET_EDIT: {
		editIncluded(to_utf8(params()["filename"]));
		break;
	}

	case LFUN_INSET_MODIFY: {
		// It should be OK just to invalidate the cache is setParams()
		// If not....
		// child_buffer_ = 0;
		InsetCommandParams p(INCLUDE_CODE);
		if (cmd.getArg(0) == "changetype") {
			InsetCommand::doDispatch(cur, cmd);
			p = params();
		} else
			InsetCommand::string2params("include", to_utf8(cmd.argument()), p);
		if (!p.getCmdName().empty()) {
			if (isListings(p)){
				InsetListingsParams new_params(to_utf8(p["lstparams"]));
				docstring const new_label =
					from_utf8(new_params.getParamValue("label"));
				
				if (new_label.empty()) {
					delete label_;
					label_ = 0;
				} else {
					docstring old_label;
					if (label_) 
						old_label = label_->getParam("name");
					else {
						label_ = createLabel(new_label);
						label_->setBuffer(buffer());
					}					

					if (new_label != old_label) {
						label_->updateCommand(new_label);
						// the label might have been adapted (duplicate)
						if (new_label != label_->getParam("name")) {
							new_params.addParam("label", "{" + 
								to_utf8(label_->getParam("name")) + "}", true);
							p["lstparams"] = from_utf8(new_params.params());
						}
					}
				}
			}
			setParams(p);
		} else
			cur.noUpdate();
		break;
	}

	//pass everything else up the chain
	default:
		InsetCommand::doDispatch(cur, cmd);
		break;
	}
}


void InsetInclude::editIncluded(string const & file)
{
	string const ext = support::getExtension(file);
	if (ext == "lyx") {
		FuncRequest fr(LFUN_BUFFER_CHILD_OPEN, file);
		lyx::dispatch(fr);
	} else
		// tex file or other text file in verbatim mode
		formats.edit(buffer(),
			support::makeAbsPath(file, support::onlyPath(buffer().absFileName())),
			"text");
}


bool InsetInclude::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action) {

	case LFUN_INSET_EDIT:
	case LFUN_INSET_MODIFY:
		flag.setEnabled(true);
		return true;

	default:
		return InsetCommand::getStatus(cur, cmd, flag);
	}
}


void InsetInclude::setParams(InsetCommandParams const & p)
{
	// invalidate the cache
	child_buffer_ = 0;

	InsetCommand::setParams(p);
	set_label_ = false;

	if (preview_->monitoring())
		preview_->stopMonitoring();

	if (type(params()) == INPUT)
		add_preview(*preview_, *this, buffer());

	buffer().updateBibfilesCache();
}


docstring InsetInclude::screenLabel() const
{
	docstring temp;

	switch (type(params())) {
		case INPUT:
			temp = buffer().B_("Input");
			break;
		case VERB:
			temp = buffer().B_("Verbatim Input");
			break;
		case VERBAST:
			temp = buffer().B_("Verbatim Input*");
			break;
		case INCLUDE:
			temp = buffer().B_("Include");
			break;
		case LISTINGS:
			temp = listings_label_;
			break;
		case NONE:
			LASSERT(false, /**/);
	}

	temp += ": ";

	if (params()["filename"].empty())
		temp += "???";
	else
		temp += from_utf8(onlyFilename(to_utf8(params()["filename"])));

	return temp;
}


Buffer * InsetInclude::getChildBuffer() const
{
	Buffer * childBuffer = loadIfNeeded(); 

	// FIXME: recursive includes
	return (childBuffer == &buffer()) ? 0 : childBuffer;
}


Buffer * InsetInclude::loadIfNeeded() const
{
	// Don't try to load it again if we failed before.
	if (failedtoload_)
		return 0;

	// Use cached Buffer if possible.
	if (child_buffer_ != 0) {
		if (theBufferList().isLoaded(child_buffer_))
			return child_buffer_;
		// Buffer vanished, so invalidate cache and try to reload.
		child_buffer_ = 0;
	}

	InsetCommandParams const & p = params();
	if (isVerbatim(p) || isListings(p))
		return 0;

	string const parent_filename = buffer().absFileName();
	FileName const included_file = 
		makeAbsPath(to_utf8(p["filename"]), onlyPath(parent_filename));

	if (!isLyXFilename(included_file.absFilename()))
		return 0;

	Buffer * child = theBufferList().getBuffer(included_file);
	if (!child) {
		// the readonly flag can/will be wrong, not anymore I think.
		if (!included_file.exists())
			return 0;

		child = theBufferList().newBuffer(included_file.absFilename());
		if (!child)
			// Buffer creation is not possible.
			return 0;

		if (!child->loadLyXFile(included_file)) {
			failedtoload_ = true;
			//close the buffer we just opened
			theBufferList().release(child);
			return 0;
		}
	
		if (!child->errorList("Parse").empty()) {
			// FIXME: Do something.
		}
	}
	child->setParent(&buffer());
	// Cache the child buffer.
	child_buffer_ = child;
	return child;
}


int InsetInclude::latex(odocstream & os, OutputParams const & runparams) const
{
	string incfile = to_utf8(params()["filename"]);

	// Do nothing if no file name has been specified
	if (incfile.empty())
		return 0;

	FileName const included_file = includedFilename(buffer(), params());

	// Check we're not trying to include ourselves.
	// FIXME RECURSIVE INCLUDE
	// This isn't sufficient, as the inclusion could be downstream.
	// But it'll have to do for now.
	if (isInputOrInclude(params()) &&
		buffer().absFileName() == included_file.absFilename())
	{
		Alert::error(_("Recursive input"),
			       bformat(_("Attempted to include file %1$s in itself! "
			       "Ignoring inclusion."), from_utf8(incfile)));
		return 0;
	}

	Buffer const * const masterBuffer = buffer().masterBuffer();

	// if incfile is relative, make it relative to the master
	// buffer directory.
	if (!FileName(incfile).isAbsolute()) {
		// FIXME UNICODE
		incfile = to_utf8(makeRelPath(from_utf8(included_file.absFilename()),
					      from_utf8(masterBuffer->filePath())));
	}

	// write it to a file (so far the complete file)
	string exportfile;
	string mangled;
	// bug 5681
	if (type(params()) == LISTINGS) {
		exportfile = incfile;
		mangled = DocFileName(included_file).mangledFilename();
	} else {
		exportfile = changeExtension(incfile, ".tex");
		mangled = DocFileName(changeExtension(included_file.absFilename(), ".tex")).
			mangledFilename();
	}

	FileName const writefile(makeAbsPath(mangled, masterBuffer->temppath()));

	if (!runparams.nice)
		incfile = mangled;
	else if (!isValidLaTeXFilename(incfile)) {
		frontend::Alert::warning(_("Invalid filename"),
					 _("The following filename is likely to cause trouble "
					   "when running the exported file through LaTeX: ") +
					    from_utf8(incfile));
	}
	LYXERR(Debug::LATEX, "incfile:" << incfile);
	LYXERR(Debug::LATEX, "exportfile:" << exportfile);
	LYXERR(Debug::LATEX, "writefile:" << writefile);

	if (runparams.inComment || runparams.dryrun) {
		//Don't try to load or copy the file if we're
		//in a comment or doing a dryrun
	} else if (isInputOrInclude(params()) &&
		 isLyXFilename(included_file.absFilename())) {
		//if it's a LyX file and we're inputting or including,
		//try to load it so we can write the associated latex
		if (!loadIfNeeded())
			return false;

		Buffer * tmp = theBufferList().getBuffer(included_file);

		if (tmp->params().baseClass() != masterBuffer->params().baseClass()) {
			// FIXME UNICODE
			docstring text = bformat(_("Included file `%1$s'\n"
				"has textclass `%2$s'\n"
				"while parent file has textclass `%3$s'."),
				included_file.displayName(),
				from_utf8(tmp->params().documentClass().name()),
				from_utf8(masterBuffer->params().documentClass().name()));
			Alert::warning(_("Different textclasses"), text);
		}

		// Make sure modules used in child are all included in master
		//FIXME It might be worth loading the children's modules into the master
		//over in BufferParams rather than doing this check.
		LayoutModuleList const masterModules = masterBuffer->params().getModules();
		LayoutModuleList const childModules = tmp->params().getModules();
		LayoutModuleList::const_iterator it = childModules.begin();
		LayoutModuleList::const_iterator end = childModules.end();
		for (; it != end; ++it) {
			string const module = *it;
			LayoutModuleList::const_iterator found =
				find(masterModules.begin(), masterModules.end(), module);
			if (found == masterModules.end()) {
				docstring text = bformat(_("Included file `%1$s'\n"
					"uses module `%2$s'\n"
					"which is not used in parent file."),
					included_file.displayName(), from_utf8(module));
				Alert::warning(_("Module not found"), text);
			}
		}

		tmp->markDepClean(masterBuffer->temppath());

// FIXME: handle non existing files
// FIXME: Second argument is irrelevant!
// since only_body is true, makeLaTeXFile will not look at second
// argument. Should we set it to string(), or should makeLaTeXFile
// make use of it somehow? (JMarc 20031002)
		// The included file might be written in a different encoding
		Encoding const * const oldEnc = runparams.encoding;
		runparams.encoding = &tmp->params().encoding();
		tmp->makeLaTeXFile(writefile,
				   masterFileName(buffer()).onlyPath().absFilename(),
				   runparams, false);
		runparams.encoding = oldEnc;
	} else {
		// In this case, it's not a LyX file, so we copy the file
		// to the temp dir, so that .aux files etc. are not created
		// in the original dir. Files included by this file will be
		// found via input@path, see ../Buffer.cpp.
		unsigned long const checksum_in  = included_file.checksum();
		unsigned long const checksum_out = writefile.checksum();

		if (checksum_in != checksum_out) {
			if (!included_file.copyTo(writefile)) {
				// FIXME UNICODE
				LYXERR(Debug::LATEX,
					to_utf8(bformat(_("Could not copy the file\n%1$s\n"
								  "into the temporary directory."),
						   from_utf8(included_file.absFilename()))));
				return 0;
			}
		}
	}

	string const tex_format = (runparams.flavor == OutputParams::LATEX) ?
			"latex" : "pdflatex";
	if (isVerbatim(params())) {
		incfile = latex_path(incfile);
		// FIXME UNICODE
		os << '\\' << from_ascii(params().getCmdName()) << '{'
		   << from_utf8(incfile) << '}';
	} else if (type(params()) == INPUT) {
		runparams.exportdata->addExternalFile(tex_format, writefile,
						      exportfile);

		// \input wants file with extension (default is .tex)
		if (!isLyXFilename(included_file.absFilename())) {
			incfile = latex_path(incfile);
			// FIXME UNICODE
			os << '\\' << from_ascii(params().getCmdName())
			   << '{' << from_utf8(incfile) << '}';
		} else {
		incfile = changeExtension(incfile, ".tex");
		incfile = latex_path(incfile);
			// FIXME UNICODE
			os << '\\' << from_ascii(params().getCmdName())
			   << '{' << from_utf8(incfile) <<  '}';
		}
	} else if (type(params()) == LISTINGS) {
		os << '\\' << from_ascii(params().getCmdName());
		string const opt = to_utf8(params()["lstparams"]);
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
		os << '\\' << from_ascii(params().getCmdName()) << '{'
		   << from_utf8(incfile) << '}';
	}

	return 0;
}


int InsetInclude::plaintext(odocstream & os, OutputParams const &) const
{
	if (isVerbatim(params()) || isListings(params())) {
		os << '[' << screenLabel() << '\n';
		// FIXME: We don't know the encoding of the file, default to UTF-8.
		os << includedFilename(buffer(), params()).fileContents("UTF-8");
		os << "\n]";
		return PLAINTEXT_NEWLINE + 1; // one char on a separate line
	} else {
		docstring const str = '[' + screenLabel() + ']';
		os << str;
		return str.size();
	}
}


int InsetInclude::docbook(odocstream & os, OutputParams const & runparams) const
{
	string incfile = to_utf8(params()["filename"]);

	// Do nothing if no file name has been specified
	if (incfile.empty())
		return 0;

	string const included_file = includedFilename(buffer(), params()).absFilename();

	// Check we're not trying to include ourselves.
	// FIXME RECURSIVE INCLUDE
	// This isn't sufficient, as the inclusion could be downstream.
	// But it'll have to do for now.
	if (buffer().absFileName() == included_file) {
		Alert::error(_("Recursive input"),
			       bformat(_("Attempted to include file %1$s in itself! "
			       "Ignoring inclusion."), from_utf8(incfile)));
		return 0;
	}

	// write it to a file (so far the complete file)
	string const exportfile = changeExtension(incfile, ".sgml");
	DocFileName writefile(changeExtension(included_file, ".sgml"));

	if (loadIfNeeded()) {
		Buffer * tmp = theBufferList().getBuffer(FileName(included_file));

		string const mangled = writefile.mangledFilename();
		writefile = makeAbsPath(mangled,
					buffer().masterBuffer()->temppath());
		if (!runparams.nice)
			incfile = mangled;

		LYXERR(Debug::LATEX, "incfile:" << incfile);
		LYXERR(Debug::LATEX, "exportfile:" << exportfile);
		LYXERR(Debug::LATEX, "writefile:" << writefile);

		tmp->makeDocBookFile(writefile, runparams, true);
	}

	runparams.exportdata->addExternalFile("docbook", writefile,
					      exportfile);
	runparams.exportdata->addExternalFile("docbook-xml", writefile,
					      exportfile);

	if (isVerbatim(params()) || isListings(params())) {
		os << "<inlinegraphic fileref=\""
		   << '&' << include_label << ';'
		   << "\" format=\"linespecific\">";
	} else
		os << '&' << include_label << ';';

	return 0;
}


void InsetInclude::validate(LaTeXFeatures & features) const
{
	string incfile = to_utf8(params()["filename"]);
	string writefile;

	LASSERT(&buffer() == &features.buffer(), /**/);

	string const included_file =
		includedFilename(buffer(), params()).absFilename();

	if (isLyXFilename(included_file))
		writefile = changeExtension(included_file, ".sgml");
	else
		writefile = included_file;

	if (!features.runparams().nice && !isVerbatim(params()) && !isListings(params())) {
		incfile = DocFileName(writefile).mangledFilename();
		writefile = makeAbsPath(incfile,
					buffer().masterBuffer()->temppath()).absFilename();
	}

	features.includeFile(include_label, writefile);

	if (isVerbatim(params()))
		features.require("verbatim");
	else if (isListings(params()))
		features.require("listings");

	// Here we must do the fun stuff...
	// Load the file in the include if it needs
	// to be loaded:
	if (loadIfNeeded()) {
		// a file got loaded
		Buffer * const tmp = theBufferList().getBuffer(FileName(included_file));
		// make sure the buffer isn't us
		// FIXME RECURSIVE INCLUDES
		// This is not sufficient, as recursive includes could be
		// more than a file away. But it will do for now.
		if (tmp && tmp != &buffer()) {
			// We must temporarily change features.buffer,
			// otherwise it would always be the master buffer,
			// and nested includes would not work.
			features.setBuffer(*tmp);
			tmp->validate(features);
			features.setBuffer(buffer());
		}
	}
}


void InsetInclude::fillWithBibKeys(BiblioInfo & keys,
	InsetIterator const & /*di*/) const
{
	if (loadIfNeeded()) {
		string const included_file = includedFilename(buffer(), params()).absFilename();
		Buffer * tmp = theBufferList().getBuffer(FileName(included_file));
		BiblioInfo const & newkeys = tmp->localBibInfo();
		keys.mergeBiblioInfo(newkeys);
	}
}


void InsetInclude::updateBibfilesCache()
{
	Buffer const * const child = getChildBuffer();
	if (child)
		child->updateBibfilesCache(Buffer::UpdateChildOnly);
}


support::FileNameList const &
	InsetInclude::getBibfilesCache() const
{
	Buffer const * const child = getChildBuffer();
	if (child)
		return child->getBibfilesCache(Buffer::UpdateChildOnly);

	static support::FileNameList const empty;
	return empty;
}


void InsetInclude::metrics(MetricsInfo & mi, Dimension & dim) const
{
	LASSERT(mi.base.bv, /**/);

	bool use_preview = false;
	if (RenderPreview::status() != LyXRC::PREVIEW_OFF) {
		graphics::PreviewImage const * pimage =
			preview_->getPreviewImage(mi.base.bv->buffer());
		use_preview = pimage && pimage->image();
	}

	if (use_preview) {
		preview_->metrics(mi, dim);
	} else {
		if (!set_label_) {
			set_label_ = true;
			button_.update(screenLabel(), true);
		}
		button_.metrics(mi, dim);
	}

	Box b(0, dim.wid, -dim.asc, dim.des);
	button_.setBox(b);
}


void InsetInclude::draw(PainterInfo & pi, int x, int y) const
{
	LASSERT(pi.base.bv, /**/);

	bool use_preview = false;
	if (RenderPreview::status() != LyXRC::PREVIEW_OFF) {
		graphics::PreviewImage const * pimage =
			preview_->getPreviewImage(pi.base.bv->buffer());
		use_preview = pimage && pimage->image();
	}

	if (use_preview)
		preview_->draw(pi, x, y);
	else
		button_.draw(pi, x, y);
}


docstring InsetInclude::contextMenu(BufferView const &, int, int) const
{
	return from_ascii("context-include");
}


Inset::DisplayType InsetInclude::display() const
{
	return type(params()) == INPUT ? Inline : AlignCenter;
}



//
// preview stuff
//

void InsetInclude::fileChanged() const
{
	Buffer const * const buffer = updateFrontend();
	if (!buffer)
		return;

	preview_->removePreview(*buffer);
	add_preview(*preview_.get(), *this, *buffer);
	preview_->startLoading(*buffer);
}


namespace {

bool preview_wanted(InsetCommandParams const & params, Buffer const & buffer)
{
	FileName const included_file = includedFilename(buffer, params);

	return type(params) == INPUT && params.preview() &&
		included_file.isReadableFile();
}


docstring latexString(InsetInclude const & inset)
{
	odocstringstream os;
	// We don't need to set runparams.encoding since this will be done
	// by latex() anyway.
	OutputParams runparams(0);
	runparams.flavor = OutputParams::LATEX;
	inset.latex(os, runparams);

	return os.str();
}


void add_preview(RenderMonitoredPreview & renderer, InsetInclude const & inset,
		 Buffer const & buffer)
{
	InsetCommandParams const & params = inset.params();
	if (RenderPreview::status() != LyXRC::PREVIEW_OFF &&
	    preview_wanted(params, buffer)) {
		renderer.setAbsFile(includedFilename(buffer, params));
		docstring const snippet = latexString(inset);
		renderer.addPreview(snippet, buffer);
	}
}

} // namespace anon


void InsetInclude::addPreview(graphics::PreviewLoader & ploader) const
{
	Buffer const & buffer = ploader.buffer();
	if (!preview_wanted(params(), buffer))
		return;
	preview_->setAbsFile(includedFilename(buffer, params()));
	docstring const snippet = latexString(*this);
	preview_->addPreview(snippet, ploader);
}


void InsetInclude::addToToc(DocIterator const & cpit)
{
	TocBackend & backend = buffer().tocBackend();

	if (isListings(params())) {
		if (label_)
			label_->addToToc(cpit);

		InsetListingsParams p(to_utf8(params()["lstparams"]));
		string caption = p.getParamValue("caption");
		if (caption.empty())
			return;
		Toc & toc = backend.toc("listing");
		docstring str = convert<docstring>(toc.size() + 1)
			+ ". " +  from_utf8(caption);
		DocIterator pit = cpit;
		toc.push_back(TocItem(pit, 0, str));
		return;
	}
	Buffer const * const childbuffer = getChildBuffer();
	if (!childbuffer)
		return;

	Toc & toc = backend.toc("child");
	docstring str = childbuffer->fileName().displayName();
	toc.push_back(TocItem(cpit, 0, str));

	TocList & toclist = backend.tocs();
	childbuffer->tocBackend().update();
	TocList const & childtoclist = childbuffer->tocBackend().tocs();
	TocList::const_iterator it = childtoclist.begin();
	TocList::const_iterator const end = childtoclist.end();
	for(; it != end; ++it)
		toclist[it->first].insert(toclist[it->first].end(),
			it->second.begin(), it->second.end());
}


void InsetInclude::updateCommand()
{
	if (!label_)
		return;

	docstring old_label = label_->getParam("name");
	label_->updateCommand(old_label, false);
	// the label might have been adapted (duplicate)
	docstring new_label = label_->getParam("name");
	if (old_label == new_label)
		return;

	// update listings parameters...
	InsetCommandParams p(INCLUDE_CODE);
	p = params();
	InsetListingsParams par(to_utf8(params()["lstparams"]));
	par.addParam("label", "{" + to_utf8(new_label) + "}", true);
	p["lstparams"] = from_utf8(par.params());
	setParams(p);	
}

void InsetInclude::updateLabels(ParIterator const & it)
{
	Buffer const * const childbuffer = getChildBuffer();
	if (childbuffer) {
		childbuffer->updateLabels(Buffer::UpdateChildOnly);
		return;
	}
	if (!isListings(params()))
		return;

	if (label_)
		label_->updateLabels(it);

	InsetListingsParams const par(to_utf8(params()["lstparams"]));
	if (par.getParamValue("caption").empty()) {
		listings_label_ = buffer().B_("Program Listing");
		return;
	}
	Buffer const & master = *buffer().masterBuffer();
	Counters & counters = master.params().documentClass().counters();
	docstring const cnt = from_ascii("listing");
	listings_label_ = master.B_("Program Listing");
	if (counters.hasCounter(cnt)) {
		counters.step(cnt);
		listings_label_ += " " + convert<docstring>(counters.value(cnt));
	}
}


} // namespace lyx
