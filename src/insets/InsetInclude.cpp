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
#include "Converter.h"
#include "Cursor.h"
#include "DispatchResult.h"
#include "Encoding.h"
#include "ErrorList.h"
#include "Exporter.h"
#include "Format.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LaTeXFeatures.h"
#include "LayoutFile.h"
#include "LayoutModuleList.h"
#include "LyX.h"
#include "LyXRC.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "output_plaintext.h"
#include "output_xhtml.h"
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

#include "mathed/MacroTable.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/docstream.h"
#include "support/FileNameList.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h" // contains
#include "support/lyxalgo.h"
#include "support/mutex.h"

#include "support/bind.h"

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace Alert = frontend::Alert;


namespace {

docstring const uniqueID()
{
	static unsigned int seed = 1000;
	static Mutex mutex;
	Mutex::Locker lock(&mutex);
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


string const parentFileName(Buffer const & buffer)
{
	return buffer.absFileName();
}


FileName const includedFileName(Buffer const & buffer,
			      InsetCommandParams const & params)
{
	return makeAbsPath(to_utf8(params["filename"]),
			onlyPath(parentFileName(buffer)));
}


InsetLabel * createLabel(Buffer * buf, docstring const & label_str)
{
	if (label_str.empty())
		return 0;
	InsetCommandParams icp(LABEL_CODE);
	icp["name"] = label_str;
	return new InsetLabel(buf, icp);
}

} // namespace anon


InsetInclude::InsetInclude(Buffer * buf, InsetCommandParams const & p)
	: InsetCommand(buf, p), include_label(uniqueID()),
	  preview_(new RenderMonitoredPreview(this)), failedtoload_(false),
	  set_label_(false), label_(0), child_buffer_(0)
{
	preview_->fileChanged(bind(&InsetInclude::fileChanged, this));

	if (isListings(params())) {
		InsetListingsParams listing_params(to_utf8(p["lstparams"]));
		label_ = createLabel(buffer_, from_utf8(listing_params.getParamValue("label")));
	} else if (isInputOrInclude(params()) && buf)
		loadIfNeeded();
}


InsetInclude::InsetInclude(InsetInclude const & other)
	: InsetCommand(other), include_label(other.include_label),
	  preview_(new RenderMonitoredPreview(this)), failedtoload_(false),
	  set_label_(false), label_(0), child_buffer_(0)
{
	preview_->fileChanged(bind(&InsetInclude::fileChanged, this));

	if (other.label_)
		label_ = new InsetLabel(*other.label_);
}


InsetInclude::~InsetInclude()
{
	if (isBufferLoaded())
		buffer().invalidateBibfileCache();
	delete label_;
}


void InsetInclude::setBuffer(Buffer & buffer)
{
	InsetCommand::setBuffer(buffer);
	if (label_)
		label_->setBuffer(buffer);
}


void InsetInclude::setChildBuffer(Buffer * buffer)
{
	child_buffer_ = buffer;
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
	switch (cmd.action()) {

	case LFUN_INSET_EDIT: {
		editIncluded(to_utf8(params()["filename"]));
		break;
	}

	case LFUN_INSET_MODIFY: {
		// It should be OK just to invalidate the cache in setParams()
		// If not....
		// child_buffer_ = 0;
		InsetCommandParams p(INCLUDE_CODE);
		if (cmd.getArg(0) == "changetype") {
			cur.recordUndo();
			InsetCommand::doDispatch(cur, cmd);
			p = params();
		} else
			InsetCommand::string2params(to_utf8(cmd.argument()), p);
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
						label_ = createLabel(buffer_, new_label);
						label_->setBuffer(buffer());
					}					

					if (new_label != old_label) {
						label_->updateLabelAndRefs(new_label, &cur);
						// the label might have been adapted (duplicate)
						if (new_label != label_->getParam("name")) {
							new_params.addParam("label", "{" + 
								to_utf8(label_->getParam("name")) + "}", true);
							p["lstparams"] = from_utf8(new_params.params());
						}
					}
				}
			}
			cur.recordUndo();
			setParams(p);
			cur.forceBufferUpdate();
		} else
			cur.noScreenUpdate();
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
	switch (cmd.action()) {

	case LFUN_INSET_EDIT:
		flag.setEnabled(true);
		return true;

	case LFUN_INSET_MODIFY:
		if (cmd.getArg(0) == "changetype")
			return InsetCommand::getStatus(cur, cmd, flag);
		else
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

	buffer().invalidateBibfileCache();
}


bool InsetInclude::isChildIncluded() const
{
	std::list<std::string> includeonlys =
		buffer().params().getIncludedChildren();
	if (includeonlys.empty())
		return true;
	return (std::find(includeonlys.begin(),
			  includeonlys.end(),
			  to_utf8(params()["filename"])) != includeonlys.end());
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
			if (isChildIncluded())
				temp = buffer().B_("Include");
			else
				temp += buffer().B_("Include (excluded)");
			break;
		case LISTINGS:
			temp = listings_label_;
			break;
		case NONE:
			LASSERT(false, temp = buffer().B_("Unknown"));
			break;
	}

	temp += ": ";

	if (params()["filename"].empty())
		temp += "???";
	else
		temp += from_utf8(onlyFileName(to_utf8(params()["filename"])));

	return temp;
}


Buffer * InsetInclude::getChildBuffer() const
{
	Buffer * childBuffer = loadIfNeeded(); 

	// FIXME RECURSIVE INCLUDE
	// This isn't sufficient, as the inclusion could be downstream.
	// But it'll have to do for now.
	return (childBuffer == &buffer()) ? 0 : childBuffer;
}


Buffer * InsetInclude::loadIfNeeded() const
{
	// This is for background export and preview. We don't even want to
	// try to load the cloned child document again.
	if (buffer().isClone())
		return child_buffer_;
	
	// Don't try to load it again if we failed before.
	if (failedtoload_ || isVerbatim(params()) || isListings(params()))
		return 0;

	FileName const included_file = includedFileName(buffer(), params());
	// Use cached Buffer if possible.
	if (child_buffer_ != 0) {
		if (theBufferList().isLoaded(child_buffer_)
    		// additional sanity check: make sure the Buffer really is
		    // associated with the file we want.
		    && child_buffer_ == theBufferList().getBuffer(included_file))
			return child_buffer_;
		// Buffer vanished, so invalidate cache and try to reload.
		child_buffer_ = 0;
	}

	if (!isLyXFileName(included_file.absFileName()))
		return 0;

	Buffer * child = theBufferList().getBuffer(included_file);
	if (!child) {
		// the readonly flag can/will be wrong, not anymore I think.
		if (!included_file.exists())
			return 0;

		child = theBufferList().newBuffer(included_file.absFileName());
		if (!child)
			// Buffer creation is not possible.
			return 0;

		// Set parent before loading, such that macros can be tracked
		child->setParent(&buffer());

		if (child->loadLyXFile() != Buffer::ReadSuccess) {
			failedtoload_ = true;
			child->setParent(0);
			//close the buffer we just opened
			theBufferList().release(child);
			return 0;
		}

		if (!child->errorList("Parse").empty()) {
			// FIXME: Do something.
		}
	} else {
		// The file was already loaded, so, simply
		// inform parent buffer about local macros.
		Buffer const * parent = &buffer();
		child->setParent(parent);
		MacroNameSet macros;
		child->listMacroNames(macros);
		MacroNameSet::const_iterator cit = macros.begin();
		MacroNameSet::const_iterator end = macros.end();
		for (; cit != end; ++cit)
			parent->usermacros.insert(*cit);
	}

	// Cache the child buffer.
	child_buffer_ = child;
	return child;
}


void InsetInclude::latex(otexstream & os, OutputParams const & runparams) const
{
	string incfile = to_utf8(params()["filename"]);

	// Do nothing if no file name has been specified
	if (incfile.empty())
		return;

	FileName const included_file = includedFileName(buffer(), params());

	// Check we're not trying to include ourselves.
	// FIXME RECURSIVE INCLUDE
	// This isn't sufficient, as the inclusion could be downstream.
	// But it'll have to do for now.
	if (isInputOrInclude(params()) &&
		buffer().absFileName() == included_file.absFileName())
	{
		Alert::error(_("Recursive input"),
			       bformat(_("Attempted to include file %1$s in itself! "
			       "Ignoring inclusion."), from_utf8(incfile)));
		return;
	}

	Buffer const * const masterBuffer = buffer().masterBuffer();

	// if incfile is relative, make it relative to the master
	// buffer directory.
	if (!FileName::isAbsolute(incfile)) {
		// FIXME UNICODE
		incfile = to_utf8(makeRelPath(from_utf8(included_file.absFileName()),
					      from_utf8(masterBuffer->filePath())));
	}

	string exppath = incfile;
	if (!runparams.export_folder.empty()) {
		exppath = makeAbsPath(exppath, runparams.export_folder).realPath();
		FileName(exppath).onlyPath().createPath();
	}

	// write it to a file (so far the complete file)
	string exportfile;
	string mangled;
	// bug 5681
	if (type(params()) == LISTINGS) {
		exportfile = exppath;
		mangled = DocFileName(included_file).mangledFileName();
	} else {
		exportfile = changeExtension(exppath, ".tex");
		mangled = DocFileName(changeExtension(included_file.absFileName(), ".tex")).
			mangledFileName();
	}

	if (!runparams.nice)
		incfile = mangled;
	else if (!runparams.silent)
		; // no warning wanted
	else if (!isValidLaTeXFileName(incfile)) {
		frontend::Alert::warning(_("Invalid filename"),
			_("The following filename will cause troubles "
				"when running the exported file through LaTeX: ") +
			from_utf8(incfile));
	} else if (!isValidDVIFileName(incfile)) {
		frontend::Alert::warning(_("Problematic filename for DVI"),
			_("The following filename can cause troubles "
				"when running the exported file through LaTeX "
				"and opening the resulting DVI: ") +
			from_utf8(incfile), true);
	}

	FileName const writefile(makeAbsPath(mangled, runparams.for_preview ?
						 buffer().temppath() : masterBuffer->temppath()));

	LYXERR(Debug::LATEX, "incfile:" << incfile);
	LYXERR(Debug::LATEX, "exportfile:" << exportfile);
	LYXERR(Debug::LATEX, "writefile:" << writefile);

	string const tex_format = flavor2format(runparams.flavor);

	switch (type(params())) {
	case VERB:
	case VERBAST: {
		incfile = latex_path(incfile);
		// FIXME UNICODE
		os << '\\' << from_ascii(params().getCmdName()) << '{'
		   << from_utf8(incfile) << '}';
		break;
	}
	case INPUT: {
		runparams.exportdata->addExternalFile(tex_format, writefile,
						      exportfile);

		// \input wants file with extension (default is .tex)
		if (!isLyXFileName(included_file.absFileName())) {
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
		break;
	}
	case LISTINGS: {
		runparams.exportdata->addExternalFile(tex_format, writefile,
						      exportfile);
		os << '\\' << from_ascii(params().getCmdName());
		string const opt = to_utf8(params()["lstparams"]);
		// opt is set in QInclude dialog and should have passed validation.
		InsetListingsParams params(opt);
		if (!params.params().empty())
			os << "[" << from_utf8(params.params()) << "]";
		os << '{'  << from_utf8(incfile) << '}';
		break;
	}
	case INCLUDE: {
		runparams.exportdata->addExternalFile(tex_format, writefile,
						      exportfile);

		// \include don't want extension and demands that the
		// file really have .tex
		incfile = changeExtension(incfile, string());
		incfile = latex_path(incfile);
		// FIXME UNICODE
		os << '\\' << from_ascii(params().getCmdName()) << '{'
		   << from_utf8(incfile) << '}';
		break;
	}
	case NONE:
		break;
	}

	if (runparams.inComment || runparams.dryrun)
		// Don't try to load or copy the file if we're
		// in a comment or doing a dryrun
		return;

	if (isInputOrInclude(params()) &&
		 isLyXFileName(included_file.absFileName())) {
		// if it's a LyX file and we're inputting or including,
		// try to load it so we can write the associated latex

		Buffer * tmp = loadIfNeeded();
		if (!tmp) {
			if (!runparams.silent) {
				docstring text = bformat(_("Could not load included "
					"file\n`%1$s'\n"
					"Please, check whether it actually exists."),
					included_file.displayName());
				Alert::warning(_("Missing included file"), text);
			}
			return;
		}

		if (!runparams.silent) {
			if (tmp->params().baseClass() != masterBuffer->params().baseClass()) {
				// FIXME UNICODE
				docstring text = bformat(_("Included file `%1$s'\n"
					"has textclass `%2$s'\n"
					"while parent file has textclass `%3$s'."),
					included_file.displayName(),
					from_utf8(tmp->params().documentClass().name()),
					from_utf8(masterBuffer->params().documentClass().name()));
				Alert::warning(_("Different textclasses"), text, true);
			}

			// Make sure modules used in child are all included in master
			// FIXME It might be worth loading the children's modules into the master
			// over in BufferParams rather than doing this check.
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
		}

		tmp->markDepClean(masterBuffer->temppath());

		// Don't assume the child's format is latex
		string const inc_format = tmp->params().bufferFormat();
		FileName const tmpwritefile(changeExtension(writefile.absFileName(),
			formats.extension(inc_format)));

		// FIXME: handle non existing files
		// The included file might be written in a different encoding
		// and language.
		Encoding const * const oldEnc = runparams.encoding;
		Language const * const oldLang = runparams.master_language;
		// If the master has full unicode flavor (XeTeX, LuaTeX),
		// the children must be encoded in plain utf8!
		runparams.encoding = runparams.isFullUnicode() ?
			encodings.fromLyXName("utf8-plain")
			: &tmp->params().encoding();
		runparams.master_language = buffer().params().language;
		runparams.par_begin = 0;
		runparams.par_end = tmp->paragraphs().size();
		runparams.is_child = true;
		if (!tmp->makeLaTeXFile(tmpwritefile, masterFileName(buffer()).
				onlyPath().absFileName(), runparams, Buffer::OnlyBody)) {
			if (!runparams.silent) {
				docstring msg = bformat(_("Included file `%1$s' "
					"was not exported correctly.\nWarning: "
					"LaTeX export is probably incomplete."),
					included_file.displayName());
				ErrorList const & el = tmp->errorList("Export");
				if (!el.empty())
					msg = bformat(from_ascii("%1$s\n\n%2$s\n\n%3$s"),
						msg, el.begin()->error,
						el.begin()->description);
				Alert::warning(_("Export failure"), msg);
			}
		}
		runparams.encoding = oldEnc;
		runparams.master_language = oldLang;
		runparams.is_child = false;

		// If needed, use converters to produce a latex file from the child
		if (tmpwritefile != writefile) {
			ErrorList el;
			bool const success =
				theConverters().convert(tmp, tmpwritefile, writefile,
							included_file,
							inc_format, tex_format, el);

			if (!success && !runparams.silent) {
				docstring msg = bformat(_("Included file `%1$s' "
						"was not exported correctly.\nWarning: "
						"LaTeX export is probably incomplete."),
						included_file.displayName());
				if (!el.empty())
					msg = bformat(from_ascii("%1$s\n\n%2$s\n\n%3$s"),
							msg, el.begin()->error,
							el.begin()->description);
				Alert::warning(_("Export failure"), msg);
			}
		}
	} else {
		// In this case, it's not a LyX file, so we copy the file
		// to the temp dir, so that .aux files etc. are not created
		// in the original dir. Files included by this file will be
		// found via either the environment variable TEXINPUTS, or
		// input@path, see ../Buffer.cpp.
		unsigned long const checksum_in  = included_file.checksum();
		unsigned long const checksum_out = writefile.checksum();

		if (checksum_in != checksum_out) {
			if (!included_file.copyTo(writefile)) {
				// FIXME UNICODE
				LYXERR(Debug::LATEX,
					to_utf8(bformat(_("Could not copy the file\n%1$s\n"
									"into the temporary directory."),
							 from_utf8(included_file.absFileName()))));
				return;
			}
		}
	}
}


docstring InsetInclude::xhtml(XHTMLStream & xs, OutputParams const & rp) const
{
	if (rp.inComment)
		 return docstring();

	// For verbatim and listings, we just include the contents of the file as-is.
	// In the case of listings, we wrap it in <pre>.
	bool const listing = isListings(params());
	if (listing || isVerbatim(params())) {
		if (listing)
			xs << html::StartTag("pre");
		// FIXME: We don't know the encoding of the file, default to UTF-8.
		xs << includedFileName(buffer(), params()).fileContents("UTF-8");
		if (listing)
			xs << html::EndTag("pre");
		return docstring();
	}

	// We don't (yet) know how to Input or Include non-LyX files.
	// (If we wanted to get really arcane, we could run some tex2html
	// converter on the included file. But that's just masochistic.)
	FileName const included_file = includedFileName(buffer(), params());
	if (!isLyXFileName(included_file.absFileName())) {
		if (!rp.silent)
			frontend::Alert::warning(_("Unsupported Inclusion"),
					 bformat(_("LyX does not know how to include non-LyX files when "
					           "generating HTML output. Offending file:\n%1$s"),
					            params()["filename"]));
		return docstring();
	}

	// In the other cases, we will generate the HTML and include it.

	// Check we're not trying to include ourselves.
	// FIXME RECURSIVE INCLUDE
	if (buffer().absFileName() == included_file.absFileName()) {
		Alert::error(_("Recursive input"),
			       bformat(_("Attempted to include file %1$s in itself! "
			       "Ignoring inclusion."), params()["filename"]));
		return docstring();
	}

	Buffer const * const ibuf = loadIfNeeded();
	if (!ibuf)
		return docstring();

	// are we generating only some paragraphs, or all of them?
	bool const all_pars = !rp.dryrun || 
			(rp.par_begin == 0 && 
			 rp.par_end == (int)buffer().text().paragraphs().size());
	
	OutputParams op = rp;
	if (all_pars) {
		op.par_begin = 0;
		op.par_end = 0;
		ibuf->writeLyXHTMLSource(xs.os(), op, Buffer::IncludedFile);
	} else
		xs << XHTMLStream::ESCAPE_NONE 
		   << "<!-- Included file: " 
		   << from_utf8(included_file.absFileName()) 
		   << XHTMLStream::ESCAPE_NONE 
			 << " -->";
	return docstring();
}


int InsetInclude::plaintext(odocstringstream & os,
        OutputParams const & op, size_t) const
{
	// just write the filename if we're making a tooltip or toc entry,
	// or are generating this for advanced search
	if (op.for_tooltip || op.for_toc || op.for_search) {
		os << '[' << screenLabel() << '\n'
		   << getParam("filename") << "\n]";
		return PLAINTEXT_NEWLINE + 1; // one char on a separate line
	}

	if (isVerbatim(params()) || isListings(params())) {
		os << '[' << screenLabel() << '\n'
		   // FIXME: We don't know the encoding of the file, default to UTF-8.
		   << includedFileName(buffer(), params()).fileContents("UTF-8")
		   << "\n]";
		return PLAINTEXT_NEWLINE + 1; // one char on a separate line
	}

	Buffer const * const ibuf = loadIfNeeded();
	if (!ibuf) {
		docstring const str = '[' + screenLabel() + ']';
		os << str;
		return str.size();
	}
	writePlaintextFile(*ibuf, os, op);
	return 0;
}


int InsetInclude::docbook(odocstream & os, OutputParams const & runparams) const
{
	string incfile = to_utf8(params()["filename"]);

	// Do nothing if no file name has been specified
	if (incfile.empty())
		return 0;

	string const included_file = includedFileName(buffer(), params()).absFileName();

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

	string exppath = incfile;
	if (!runparams.export_folder.empty()) {
		exppath = makeAbsPath(exppath, runparams.export_folder).realPath();
		FileName(exppath).onlyPath().createPath();
	}

	// write it to a file (so far the complete file)
	string const exportfile = changeExtension(exppath, ".sgml");
	DocFileName writefile(changeExtension(included_file, ".sgml"));

	Buffer * tmp = loadIfNeeded();
	if (tmp) {
		string const mangled = writefile.mangledFileName();
		writefile = makeAbsPath(mangled,
					buffer().masterBuffer()->temppath());
		if (!runparams.nice)
			incfile = mangled;

		LYXERR(Debug::LATEX, "incfile:" << incfile);
		LYXERR(Debug::LATEX, "exportfile:" << exportfile);
		LYXERR(Debug::LATEX, "writefile:" << writefile);

		tmp->makeDocBookFile(writefile, runparams, Buffer::OnlyBody);
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
	LATTEST(&buffer() == &features.buffer());

	string incfile = to_utf8(params()["filename"]);
	string const included_file =
		includedFileName(buffer(), params()).absFileName();

	string writefile;
	if (isLyXFileName(included_file))
		writefile = changeExtension(included_file, ".sgml");
	else
		writefile = included_file;

	if (!features.runparams().nice && !isVerbatim(params()) && !isListings(params())) {
		incfile = DocFileName(writefile).mangledFileName();
		writefile = makeAbsPath(incfile,
					buffer().masterBuffer()->temppath()).absFileName();
	}

	features.includeFile(include_label, writefile);

	features.useInsetLayout(getLayout());
	if (isVerbatim(params()))
		features.require("verbatim");
	else if (isListings(params()))
		features.require("listings");

	// Here we must do the fun stuff...
	// Load the file in the include if it needs
	// to be loaded:
	Buffer * const tmp = loadIfNeeded();
	if (tmp) {
		// the file is loaded
		// make sure the buffer isn't us
		// FIXME RECURSIVE INCLUDES
		// This is not sufficient, as recursive includes could be
		// more than a file away. But it will do for now.
		if (tmp && tmp != &buffer()) {
			// We must temporarily change features.buffer,
			// otherwise it would always be the master buffer,
			// and nested includes would not work.
			features.setBuffer(*tmp);
			// Maybe this is already a child
			bool const is_child =
				features.runparams().is_child;
			features.runparams().is_child = true;
			tmp->validate(features);
			features.runparams().is_child = is_child;
			features.setBuffer(buffer());
		}
	}
}


void InsetInclude::collectBibKeys(InsetIterator const & /*di*/) const
{
	Buffer * child = loadIfNeeded();
	if (!child)
		return;
	// FIXME RECURSIVE INCLUDE
	// This isn't sufficient, as the inclusion could be downstream.
	// But it'll have to do for now.
	if (child == &buffer())
		return;
	child->collectBibKeys();
}


void InsetInclude::metrics(MetricsInfo & mi, Dimension & dim) const
{
	LBUFERR(mi.base.bv);

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
	LBUFERR(pi.base.bv);

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


void InsetInclude::write(ostream & os) const
{
	params().Write(os, &buffer());
}


string InsetInclude::contextMenuName() const
{
	return "context-include";
}


Inset::DisplayType InsetInclude::display() const
{
	return type(params()) == INPUT ? Inline : AlignCenter;
}


docstring InsetInclude::layoutName() const
{
	if (isListings(params()))
		return from_ascii("IncludeListings");
	return InsetCommand::layoutName();
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
	FileName const included_file = includedFileName(buffer, params);

	return type(params) == INPUT && params.preview() &&
		included_file.isReadableFile();
}


docstring latexString(InsetInclude const & inset)
{
	TexRow texrow;
	odocstringstream ods;
	otexstream os(ods, texrow);
	// We don't need to set runparams.encoding since this will be done
	// by latex() anyway.
	OutputParams runparams(0);
	runparams.flavor = OutputParams::LATEX;
	runparams.for_preview = true;
	inset.latex(os, runparams);

	return ods.str();
}


void add_preview(RenderMonitoredPreview & renderer, InsetInclude const & inset,
		 Buffer const & buffer)
{
	InsetCommandParams const & params = inset.params();
	if (RenderPreview::status() != LyXRC::PREVIEW_OFF &&
	    preview_wanted(params, buffer)) {
		renderer.setAbsFile(includedFileName(buffer, params));
		docstring const snippet = latexString(inset);
		renderer.addPreview(snippet, buffer);
	}
}

} // namespace anon


void InsetInclude::addPreview(DocIterator const & /*inset_pos*/,
	graphics::PreviewLoader & ploader) const
{
	Buffer const & buffer = ploader.buffer();
	if (!preview_wanted(params(), buffer))
		return;
	preview_->setAbsFile(includedFileName(buffer, params()));
	docstring const snippet = latexString(*this);
	preview_->addPreview(snippet, ploader);
}


void InsetInclude::addToToc(DocIterator const & cpit, bool output_active) const
{
	TocBackend & backend = buffer().tocBackend();

	if (isListings(params())) {
		if (label_)
			label_->addToToc(cpit, output_active);

		InsetListingsParams p(to_utf8(params()["lstparams"]));
		string caption = p.getParamValue("caption");
		if (caption.empty())
			return;
		Toc & toc = backend.toc("listing");
		docstring str = convert<docstring>(toc.size() + 1)
			+ ". " +  from_utf8(caption);
		DocIterator pit = cpit;
		toc.push_back(TocItem(pit, 0, str, output_active));
		return;
	}
	Buffer const * const childbuffer = getChildBuffer();
	if (!childbuffer)
		return;

	Toc & toc = backend.toc("child");
	docstring str = childbuffer->fileName().displayName();
	toc.push_back(TocItem(cpit, 0, str, output_active));

	TocList & toclist = backend.tocs();
	childbuffer->tocBackend().update(output_active);
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
	label_->updateLabel(old_label);
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


void InsetInclude::updateBuffer(ParIterator const & it, UpdateType utype)
{
	button_.update(screenLabel(), true);

	Buffer const * const childbuffer = getChildBuffer();
	if (childbuffer) {
		childbuffer->updateBuffer(Buffer::UpdateChildOnly, utype);
		return;
	}
	if (!isListings(params()))
		return;

	if (label_)
		label_->updateBuffer(it, utype);

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
		counters.step(cnt, utype);
		listings_label_ += " " + convert<docstring>(counters.value(cnt));
	}
}


} // namespace lyx
