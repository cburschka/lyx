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
#include "insets/renderers.h"

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

using namespace lyx::support;

using std::ostream;
using std::endl;
using std::auto_ptr;

namespace {

lyx::graphics::DisplayType const defaultDisplayType = lyx::graphics::NoDisplay;

unsigned int defaultLyxScale = 100;

/// Substitute meta-variables in string s, makeing use of params and buffer.
string const doSubstitution(InsetExternal::Params const & params,
			    Buffer const * buffer, string const & s);

/// Invoke the external editor.
void editExternal(InsetExternal::Params const & params, Buffer const * buffer);

} // namespace anon


InsetExternal::Params::Params()
	: display(defaultDisplayType),
	  lyxscale(defaultLyxScale)
{
	tempname = tempName(string(), "lyxext");
	unlink(tempname);
	// must have an extension for the converter code to work correctly.
	tempname += ".tmp";
}


InsetExternal::Params::~Params()
{
	unlink(tempname);
}


InsetExternal::InsetExternal()
	: renderer_(new ButtonRenderer)
{}


InsetExternal::InsetExternal(InsetExternal const & other)
	: Inset(other),
	  boost::signals::trackable(),
	  params_(other.params_),
	  renderer_(other.renderer_->clone())
{
	GraphicRenderer * ptr = dynamic_cast<GraphicRenderer *>(renderer_.get());
	if (ptr) {
		ptr->connect(boost::bind(&InsetExternal::statusChanged, this));
	}
}


auto_ptr<InsetBase> InsetExternal::clone() const
{
	return auto_ptr<InsetBase>(new InsetExternal(*this));
}


InsetExternal::~InsetExternal()
{
	InsetExternalMailer(*this).hideDialog();
}


void InsetExternal::statusChanged()
{
	BufferView * bv = renderer_->view();
	if (bv)
		bv->updateInset(this);
}


dispatch_result InsetExternal::localDispatch(FuncRequest const & cmd)
{
	switch (cmd.action) {

	case LFUN_EXTERNAL_EDIT: {
		Assert(cmd.view());

		Buffer const & buffer = *cmd.view()->buffer();
		InsetExternal::Params p;
		InsetExternalMailer::string2params(cmd.argument, buffer, p);
		editExternal(p, &buffer);
		return DISPATCHED_NOUPDATE;
	}

	case LFUN_INSET_MODIFY: {
		Assert(cmd.view());

		Buffer const * buffer = cmd.view()->buffer();
		InsetExternal::Params p;
		InsetExternalMailer::string2params(cmd.argument, *buffer, p);
		setParams(p, buffer);
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


void InsetExternal::metrics(MetricsInfo & mi, Dimension & dim) const
{
	renderer_->metrics(mi, dim);
	dim_ = dim;
}


void InsetExternal::draw(PainterInfo & pi, int x, int y) const
{
	renderer_->draw(pi, x, y);
}


namespace {

lyx::graphics::Params get_grfx_params(InsetExternal::Params const & eparams)
{
	lyx::graphics::Params gparams;

	gparams.filename = eparams.filename.absFilename();
	gparams.scale = eparams.lyxscale;
	gparams.display = eparams.display;

	if (gparams.display == lyx::graphics::DefaultDisplay)
		gparams.display = lyxrc.display_graphics;

	// Override the above if we're not using a gui
	if (!lyx_gui::use_gui)
		gparams.display = lyx::graphics::NoDisplay;

	return gparams;
}


ExternalTemplate const * getTemplatePtr(InsetExternal::Params const & params)
{
	ExternalTemplateManager & etm = ExternalTemplateManager::get();
	ExternalTemplate const & templ = etm.getTemplateByName(params.templatename);
	if (templ.lyxName.empty())
		return 0;
	return &templ;
}


string const getScreenLabel(InsetExternal::Params const & params,
			    Buffer const * buffer)
{
	ExternalTemplate const * const ptr = getTemplatePtr(params);
	if (!ptr)
		return bformat(_("External template %1$s is not installed"),
			       params.templatename);
	return doSubstitution(params, buffer, ptr->guiName);
}

} // namespace anon


InsetExternal::Params const & InsetExternal::params() const
{
	return params_;
}


void InsetExternal::setParams(Params const & p, Buffer const * buffer)
{
	// The stored params; what we would like to happen in an ideal world.
	params_.filename = p.filename;
	params_.templatename = p.templatename;
	params_.display = p.display;
	params_.lyxscale = p.lyxscale;

	// We display the inset as a button by default.
	bool display_button = (!getTemplatePtr(params_) ||
			       params_.filename.empty() ||
			       params_.display == lyx::graphics::NoDisplay);

	if (display_button) {
		ButtonRenderer * button_ptr =
			dynamic_cast<ButtonRenderer *>(renderer_.get());
		if (!button_ptr) {
			button_ptr = new ButtonRenderer;
			renderer_.reset(button_ptr);
		}

		button_ptr->update(getScreenLabel(params_, buffer), true);

	} else {
		GraphicRenderer * graphic_ptr =
			dynamic_cast<GraphicRenderer *>(renderer_.get());
		if (!graphic_ptr) {
			graphic_ptr = new GraphicRenderer;
			graphic_ptr->connect(
				boost::bind(&InsetExternal::statusChanged, this));
			renderer_.reset(graphic_ptr);
		}

		graphic_ptr->update(get_grfx_params(params_));
	}
}


void InsetExternal::write(Buffer const * buffer, ostream & os) const
{
	os << "External\n"
	   << "\ttemplate " << params_.templatename << '\n';

	if (!params_.filename.empty())
		os << "\tfilename "
		   << params_.filename.outputFilename(buffer->filePath())
		   << '\n';

	if (params_.display != defaultDisplayType)
		os << "\tdisplay " << lyx::graphics::displayTranslator.find(params_.display)
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
			params.templatename = lex.getString();
			break;
		}

		case EX_FILENAME: {
			lex.next();
			string const name = lex.getString();
			params.filename.set(name, buffer->filePath());
			break;
		}

		case EX_DISPLAY: {
			lex.next();
			string const name = lex.getString();
			params.display = lyx::graphics::displayTranslator.find(name);
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
	setParams(params, buffer);

	lyxerr[Debug::INFO] << "InsetExternal::Read: "
			    << "template: '" << params_.templatename
			    << "' filename: '" << params_.filename.absFilename()
			    << "' display: '" << params_.display
			    << "' scale: '" << params_.lyxscale
			    << '\'' << endl;
}


int InsetExternal::write(string const & format,
			 Buffer const * buf, ostream & os,
			 bool external_in_tmpdir) const
{
	ExternalTemplate const * const et_ptr = getTemplatePtr(params_);
	if (!et_ptr)
		return 0;
	ExternalTemplate const & et = *et_ptr;

	ExternalTemplate::Formats::const_iterator cit =
		et.formats.find(format);
	if (cit == et.formats.end()) {
		lyxerr << "External template format '" << format
		       << "' not specified in template "
		       << params_.templatename << endl;
		return 0;
	}

	updateExternal(format, buf, external_in_tmpdir);
	string const str = doSubstitution(params_, buf, cit->second.product);
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
		ExternalTemplate const * const et_ptr = getTemplatePtr(params_);
		if (!et_ptr)
			return 0;
		ExternalTemplate const & et = *et_ptr;

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
	ExternalTemplate const * const et_ptr = getTemplatePtr(params_);
	if (!et_ptr)
		return;
	ExternalTemplate const & et = *et_ptr;

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


void InsetExternal::updateExternal(string const & format,
				   Buffer const * buf,
				   bool external_in_tmpdir) const
{
	ExternalTemplate const * const et_ptr = getTemplatePtr(params_);
	if (!et_ptr)
		return;
	ExternalTemplate const & et = *et_ptr;

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

	string from_file = params_.filename.absFilename();

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

	string const to_file = doSubstitution(params_, buf,
					      outputFormat.updateResult);

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


namespace {

/// Substitute meta-variables in this string
string const doSubstitution(InsetExternal::Params const & params,
			    Buffer const * buffer, string const & s)
{
	string result;
	string const buffer_path = buffer ? buffer->filePath() : string();
	string const filename = params.filename.outputFilename(buffer_path);
	string const basename = ChangeExtension(filename, string());
	string const filepath = OnlyPath(filename);

	result = subst(s, "$$FName", filename);
	result = subst(result, "$$Basename", basename);
	result = subst(result, "$$FPath", filepath);
	result = subst(result, "$$Tempname", params.tempname);
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


void editExternal(InsetExternal::Params const & params, Buffer const * buffer)
{
	if (!buffer)
		return;

	ExternalTemplate const * const et_ptr = getTemplatePtr(params);
	if (!et_ptr)
		return;
	ExternalTemplate const & et = *et_ptr;

	if (et.editCommand.empty())
		return;

	string const command = doSubstitution(params, buffer, et.editCommand);

	Path p(buffer->filePath());
	Forkedcall call;
	if (lyxerr.debugging()) {
		lyxerr << "Executing '" << command << "' in '"
		       << buffer->filePath() << '\'' << endl;
	}
	call.startscript(Forkedcall::DontWait, command);
}

} // namespace anon

string const InsetExternalMailer::name_("external");

InsetExternalMailer::InsetExternalMailer(InsetExternal & inset)
	: inset_(inset)
{}


string const InsetExternalMailer::inset2string(Buffer const & buffer) const
{
	return params2string(inset_.params(), buffer);
}


void InsetExternalMailer::string2params(string const & in,
					Buffer const & buffer,
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
		inset.read(&buffer, lex);
		params = inset.params();
	}
}


string const
InsetExternalMailer::params2string(InsetExternal::Params const & params,
				   Buffer const & buffer)
{
	InsetExternal inset;
	inset.setParams(params, &buffer);
	ostringstream data;
	data << name_ << ' ';
	inset.write(&buffer, data);
	data << "\\end_inset\n";
	return STRCONV(data.str());
}
