/**
 * \file insetexternal.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "insetexternal.h"
#include "insets/graphicinset.h"

#include "buffer.h"
#include "BufferView.h"
#include "converter.h"
#include "debug.h"
#include "ExternalTemplate.h"
#include "funcrequest.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "latexrunparams.h"
#include "lyx_main.h"
#include "lyxlex.h"
#include "lyxrc.h"
#include "Lsstream.h"

#include "frontends/lyx_gui.h"
#include "frontends/LyXView.h"
#include "frontends/Dialogs.h"

#include "support/FileInfo.h"
#include "support/filetools.h"
#include "support/forkedcall.h"
#include "support/lstrings.h"
#include "support/lyxalgo.h"
#include "support/path.h"
#include "support/tostr.h"

#include <boost/bind.hpp>

#include <cstdio>
#include <utility>

using std::ostream;
using std::endl;


namespace {

grfx::DisplayType const defaultDisplayType = grfx::NoDisplay;

unsigned int defaultLyxScale = 100;

} // namespace anon


InsetExternal::Params::Params()
	: display(defaultDisplayType),
	  lyxscale(defaultLyxScale)
{}


InsetExternal::InsetExternal()
	: renderer_(new GraphicInset)
{
	renderer_->connect(boost::bind(&InsetExternal::statusChanged, this));
	params_.templ = ExternalTemplateManager::get().getTemplates().begin()->second;
}


InsetExternal::InsetExternal(InsetExternal const & other)
	: Inset(other),
	  boost::signals::trackable(),
	  params_(other.params_),
	  renderer_(new GraphicInset(*other.renderer_))
{
	renderer_->connect(boost::bind(&InsetExternal::statusChanged, this));
}


Inset * InsetExternal::clone() const
{
	InsetExternal * inset = new InsetExternal(*this);
	return inset;
}


InsetExternal::~InsetExternal()
{
	if (!tempname_.empty())
		lyx::unlink(tempname_);
	InsetExternalMailer(*this).hideDialog();
}


void InsetExternal::statusChanged()
{
	BufferView * bv = renderer_->view();
	if (bv)
		bv->updateInset(this);
}
	

InsetExternal::Params const & InsetExternal::params() const
{
	return params_;
}


dispatch_result InsetExternal::localDispatch(FuncRequest const & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetExternal::Params p;
		InsetExternalMailer::string2params(cmd.argument, p);
		setParams(p, cmd.view()->buffer()->filePath());
		cmd.view()->updateInset(this);
		return DISPATCHED;
	}

	case LFUN_INSET_DIALOG_UPDATE:
		InsetExternalMailer(*this).updateDialog(cmd.view());
		return DISPATCHED;

	case LFUN_MOUSE_RELEASE:
	case LFUN_INSET_EDIT:
		InsetExternalMailer(*this).showDialog(cmd.view());
		return DISPATCHED;

	default:
		return UNDISPATCHED;
	}
}


void InsetExternal::cache(BufferView * bv) const
{
	renderer_->view(bv);
}


void InsetExternal::metrics(MetricsInfo & mi, Dimension & dim) const
{
	renderer_->metrics(mi, dim);
}


void InsetExternal::draw(PainterInfo & pi, int x, int y) const
{
	renderer_->draw(pi, x, y);
}


namespace {

grfx::Params get_grfx_params(InsetExternal::Params const & eparams,
			     string const & filepath)
{
	grfx::Params gparams;

	if (!eparams.filename.empty()) {
		lyx::Assert(AbsolutePath(filepath));
		gparams.filename = MakeAbsPath(eparams.filename, filepath);
	}

	gparams.scale = eparams.lyxscale;
	gparams.display = eparams.display;

	if (gparams.display == grfx::DefaultDisplay)
		gparams.display = lyxrc.display_graphics;

	// Override the above if we're not using a gui
	if (!lyx_gui::use_gui)
		gparams.display = grfx::NoDisplay;

	return gparams;
}

} // namespace anon


void InsetExternal::setParams(Params const & p, string const & filepath)
{
	params_.filename = p.filename;
	params_.templ = p.templ;
	params_.display = p.display;
	params_.lyxscale = p.lyxscale;

	// Update the display using the new parameters.
	if (params_.filename.empty() || !filepath.empty())
		renderer_->update(get_grfx_params(params_, filepath));	
	string const msg = doSubstitution(0, params_.templ.guiName);
	renderer_->setNoDisplayMessage(msg);
}


string const InsetExternal::editMessage() const
{
	return doSubstitution(0, params_.templ.guiName);
}


void InsetExternal::write(Buffer const *, ostream & os) const
{
	os << "External\n"
	   << "\ttemplate " << params_.templ.lyxName << '\n';

	if (!params_.filename.empty())
		os << "\tfilename " << params_.filename << '\n';

	if (params_.display != defaultDisplayType)
		os << "\tdisplay " << grfx::displayTranslator.find(params_.display)
		   << '\n';

	if (params_.lyxscale != defaultLyxScale)
		os << "\tlyxscale " << tostr(params_.lyxscale) << '\n';
}


void InsetExternal::read(Buffer const * buffer, LyXLex & lex)
{
	enum ExternalTags {
		EX_TEMPLATE = 1,
		EX_FILENAME,
		EX_DISPLAY,
		EX_LYXSCALE,
		EX_END
	};

	keyword_item external_tags[] = {
		{ "\\end_inset", EX_END },
		{ "display", EX_DISPLAY},
		{ "filename", EX_FILENAME},
		{ "lyxscale", EX_LYXSCALE},
		{ "template", EX_TEMPLATE }
	};

	lex.pushTable(external_tags, EX_END);

	bool found_end  = false;
	bool read_error = false;

	InsetExternal::Params params;
	while (lex.isOK()) {
		switch (lex.lex()) {
		case EX_TEMPLATE: {
			lex.next();
			string const name = lex.getString();
			ExternalTemplateManager & etm =
				ExternalTemplateManager::get();
			params.templ = etm.getTemplateByName(name);
			break;
		}

		case EX_FILENAME: {
			lex.next();
			string const name = lex.getString();
			params.filename = name;
			break;
		}

		case EX_DISPLAY: {
			lex.next();
			string const name = lex.getString();
			params.display = grfx::displayTranslator.find(name);
			break;
		}

		case EX_LYXSCALE: {
			lex.next();
			params.lyxscale = lex.getInteger();
			break;
		}

		case EX_END:
			found_end = true;
			break;

		default:
			lex.printError("ExternalInset::read: "
				       "Wrong tag: $$Token");
			read_error = true;
			break;
		}

		if (found_end || read_error)
			break;
	}

	if (!found_end) {
		lex.printError("ExternalInset::read: "
			       "Missing \\end_inset.");
	}

	lex.popTable();

	// Replace the inset's store
	params_ = params;

	lyxerr[Debug::INFO] << "InsetExternal::Read: "
	       << "template: '" << params_.templ.lyxName
	       << "' filename: '" << params_.filename
	       << "' display: '" << params_.display
	       << "' scale: '" << params_.lyxscale
	       << '\'' << endl;

	// Update the display using the new parameters.
	if (buffer)
		renderer_->update(get_grfx_params(params_, buffer->filePath()));
	string const msg = doSubstitution(0, params_.templ.guiName);
	renderer_->setNoDisplayMessage(msg);
}


int InsetExternal::write(string const & format,
			 Buffer const * buf, ostream & os,
			 bool external_in_tmpdir) const
{
	ExternalTemplate const & et = params_.templ;
	ExternalTemplate::Formats::const_iterator cit =
		et.formats.find(format);
	if (cit == et.formats.end()) {
		lyxerr << "External template format '" << format
		       << "' not specified in template "
		       << params_.templ.lyxName << endl;
		return 0;
	}

	updateExternal(format, buf, external_in_tmpdir);
	string const str = doSubstitution(buf, cit->second.product);
	os << str;
	return int(lyx::count(str.begin(), str.end(),'\n') + 1);
}


int InsetExternal::latex(Buffer const * buf, ostream & os,
			 LatexRunParams const & runparams) const
{
	// "nice" means that the buffer is exported to LaTeX format but not
	// run through the LaTeX compiler.
	// If we're running through the LaTeX compiler, we should write the
	// generated files in the bufer's temporary directory.
	bool const external_in_tmpdir =
		lyxrc.use_tempdir && !buf->tmppath.empty() && !runparams.nice;

	// If the template has specified a PDFLaTeX output, then we try and
	// use that.
	if (runparams.flavor == LatexRunParams::PDFLATEX) {
		ExternalTemplate const & et = params_.templ;
		ExternalTemplate::Formats::const_iterator cit =
			et.formats.find("PDFLaTeX");
		if (cit != et.formats.end())
			return write("PDFLaTeX", buf, os, external_in_tmpdir);
	}

	return write("LaTeX", buf, os, external_in_tmpdir);
}


int InsetExternal::ascii(Buffer const * buf, ostream & os, int) const
{
	return write("Ascii", buf, os);
}


int InsetExternal::linuxdoc(Buffer const * buf, ostream & os) const
{
	return write("LinuxDoc", buf, os);
}


int InsetExternal::docbook(Buffer const * buf, ostream & os, bool) const
{
	return write("DocBook", buf, os);
}


void InsetExternal::validate(LaTeXFeatures & features) const
{
	ExternalTemplate const & et = params_.templ;
	ExternalTemplate::Formats::const_iterator cit =
		et.formats.find("LaTeX");

	if (cit == et.formats.end())
		return;

	if (!cit->second.requirement.empty()) {
		features.require(cit->second.requirement);
	}
	if (!cit->second.preamble.empty()) {
		features.addExternalPreamble(cit->second.preamble + "\n");
	}
}


string const InsetExternal::doSubstitution(Buffer const * buffer,
					   string const & s) const
{
	string result;
	string const basename = ChangeExtension(params_.filename, string());
	string filepath;
	bool external_in_tmpdir = false;
	if (buffer && !buffer->tmppath.empty() && !buffer->niceFile) {
		filepath = buffer->filePath();
		if (lyxrc.use_tempdir)
			external_in_tmpdir = true;
	}
	if (tempname_.empty()) {
		string const path = external_in_tmpdir ? buffer->tmppath : string();
		tempname_ = lyx::tempName(path, "lyxext");
		lyx::unlink(tempname_);
		// must have an extension for the converter code to work correctly.
		tempname_ += ".tmp";
	}
	result = subst(s, "$$FName", params_.filename);
	result = subst(result, "$$Basename", basename);
	result = subst(result, "$$FPath", filepath);
	result = subst(result, "$$Tempname", tempname_);
	result = subst(result, "$$Sysdir", system_lyxdir);

	// Handle the $$Contents(filename) syntax
	if (contains(result, "$$Contents(\"")) {

		string::size_type const pos = result.find("$$Contents(\"");
		string::size_type const end = result.find("\")", pos);
		string const file = result.substr(pos + 12, end - (pos + 12));
		string contents;
		if (buffer) {
			Path p(buffer->filePath());
			if (!IsFileReadable(file))
				Path p(buffer->tmppath);
			if (IsFileReadable(file))
				contents = GetFileContents(file);
		} else {
			contents = GetFileContents(file);
		}
		result = subst(result,
			       ("$$Contents(\"" + file + "\")").c_str(),
			       contents);
	}

	return result;
}


void InsetExternal::updateExternal(string const & format,
				   Buffer const * buf,
				   bool external_in_tmpdir) const
{
	ExternalTemplate const & et = params_.templ;
	if (!et.automaticProduction)
		return;

	ExternalTemplate::Formats::const_iterator cit =
		et.formats.find(format);
	if (cit == et.formats.end())
		return;

	ExternalTemplate::FormatTemplate const & outputFormat = cit->second;
	if (outputFormat.updateResult.empty())
		return;

	string from_format = et.inputFormat;
	if (from_format.empty())
		return;

	string from_file = params_.filename.empty() ?
		string() : MakeAbsPath(params_.filename, buf->filePath());

	if (from_format == "*") {
		if (from_file.empty())
			return;

		// Try and ascertain the file format from its contents.
		from_format = getExtFromContents(from_file);
		if (from_format.empty())
			return;
	}

	string const to_format = outputFormat.updateFormat;
	if (to_format.empty())
		return;

	if (!converters.isReachable(from_format, to_format)) {
		lyxerr << "InsetExternal::updateExternal. "
			"Unable to convert from "
		       << from_format << " to " << to_format << endl;
		return;
	}

	if (external_in_tmpdir && !from_file.empty()) {
		// We are running stuff through LaTeX
		from_file = copyFileToDir(buf->tmppath, from_file);
		if (from_file.empty())
			return;
	}

	string const to_file = doSubstitution(buf, outputFormat.updateResult);

	FileInfo fi(from_file);
	string abs_to_file = to_file;
	if (!AbsolutePath(to_file))
		abs_to_file = MakeAbsPath(to_file, OnlyPath(from_file));
	FileInfo fi2(abs_to_file);
	if (fi2.exist() && fi.exist() &&
	    difftime(fi2.getModificationTime(),
		     fi.getModificationTime()) >= 0) {
	} else {
		string const to_filebase = ChangeExtension(to_file, string());
		converters.convert(buf, from_file, to_filebase,
				   from_format, to_format);
	}
}


void InsetExternal::editExternal() const
{
	ExternalTemplate const & et = params_.templ;
	if (et.editCommand.empty())
		return;

	BufferView const * bv = renderer_->view();
	Buffer const * buffer = bv ? bv->buffer() : 0;
	if (!buffer)
		return;

	string const command = doSubstitution(buffer, et.editCommand);

	Path p(buffer->filePath());
	Forkedcall call;
	if (lyxerr.debugging()) {
		lyxerr << "Executing '" << command << "' in '"
		       << buffer->filePath() << '\'' << endl;
	}
	call.startscript(Forkedcall::DontWait, command);
}


string const InsetExternalMailer::name_("external");

InsetExternalMailer::InsetExternalMailer(InsetExternal & inset)
	: inset_(inset)
{}


string const InsetExternalMailer::inset2string() const
{
	return params2string(inset_.params());
}


void InsetExternalMailer::string2params(string const & in,
					InsetExternal::Params & params)
{
	params = InsetExternal::Params();

	if (in.empty())
		return;

	istringstream data(STRCONV(in));
	LyXLex lex(0,0);
	lex.setStream(data);

	if (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token != name_)
			return;
	}

	// This is part of the inset proper that is usually swallowed
	// by Buffer::readInset
	if (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token != "External")
			return;
	}

	if (lex.isOK()) {
		InsetExternal inset;
		inset.read(0, lex);
		params = inset.params();
	}
}


string const
InsetExternalMailer::params2string(InsetExternal::Params const & params)
{
	InsetExternal inset;
	inset.setParams(params, string());
	ostringstream data;
	data << name_ << ' ';
	inset.write(0, data);
	data << "\\end_inset\n";
	return STRCONV(data.str());
}
