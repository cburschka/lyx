/**
 * \file insetexternal.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 *
 * Full author contact details are available in file CREDITS.
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
#include "lyxlex.h"
#include "lyxrc.h"
#include "support/std_sstream.h"

#include "frontends/lyx_gui.h"

#include "support/filetools.h"
#include "support/forkedcall.h"
#include "support/lstrings.h"
#include "support/lyxalgo.h"
#include "support/lyxlib.h"
#include "support/path.h"
#include "support/path_defines.h"
#include "support/tostr.h"
#include "support/translator.h"

#include <boost/bind.hpp>

namespace support = lyx::support;

using std::endl;

using std::auto_ptr;
using std::istringstream;
using std::ostream;
using std::ostringstream;
using std::vector;


namespace lyx {
namespace graphics {
/// The translator between the DisplayType and the corresponding lyx string.
extern Translator<DisplayType, string> displayTranslator;
}
}

namespace {

lyx::graphics::DisplayType const defaultDisplayType = lyx::graphics::NoDisplay;

unsigned int defaultLyxScale = 100;

/// Substitute meta-variables in string s, makeing use of params and buffer.
string const doSubstitution(InsetExternal::Params const & params,
			    Buffer const & buffer, string const & s);

/// Invoke the external editor.
void editExternal(InsetExternal::Params const & params, Buffer const & buffer);


ExternalTemplate const * getTemplatePtr(string const & name)
{
	ExternalTemplateManager const & etm = ExternalTemplateManager::get();
	return etm.getTemplateByName(name);
}


ExternalTemplate const * getTemplatePtr(InsetExternal::Params const & params)
{
	ExternalTemplateManager const & etm = ExternalTemplateManager::get();
	return etm.getTemplateByName(params.templatename());
}

} // namespace anon


InsetExternal::TempName::TempName()
{
	tempname_ = support::tempName(string(), "lyxext");
	support::unlink(tempname_);
	// must have an extension for the converter code to work correctly.
	tempname_ += ".tmp";
}


InsetExternal::TempName::TempName(InsetExternal::TempName const &)
{
	tempname_ = TempName()();
}


InsetExternal::TempName::~TempName()
{
	support::unlink(tempname_);
}


InsetExternal::TempName &
InsetExternal::TempName::operator=(InsetExternal::TempName const & other)
{
	if (this != &other)
		tempname_ = TempName()();
	return *this;
}


InsetExternal::Params::Params()
	: display(defaultDisplayType),
	  lyxscale(defaultLyxScale)
{}


void InsetExternal::Params::settemplate(string const & name)
{
	templatename_ = name;
}


void InsetExternal::Params::write(Buffer const & buffer, ostream & os) const
{
	os << "External\n"
	   << "\ttemplate " << templatename() << '\n';

	if (!filename.empty())
		os << "\tfilename "
		   << filename.outputFilename(buffer.filePath())
		   << '\n';

	if (display != defaultDisplayType)
		os << "\tdisplay " << lyx::graphics::displayTranslator.find(display)
		   << '\n';

	if (lyxscale != defaultLyxScale)
		os << "\tlyxscale " << tostr(lyxscale) << '\n';
}


bool InsetExternal::Params::read(Buffer const & buffer, LyXLex & lex)
{
	enum ExternalTags {
		EX_TEMPLATE = 1,
		EX_FILENAME,
		EX_DISPLAY,
		EX_LYXSCALE,
		EX_END
	};

	keyword_item external_tags[] = {
		{ "\\end_inset",     EX_END },
		{ "display",         EX_DISPLAY},
		{ "filename",        EX_FILENAME},
		{ "lyxscale",        EX_LYXSCALE},
		{ "template",        EX_TEMPLATE }
	};

	pushpophelper pph(lex, external_tags, EX_END);

	bool found_end  = false;
	bool read_error = false;

	while (lex.isOK()) {
		switch (lex.lex()) {
		case EX_TEMPLATE:
			lex.next();
			templatename_ = lex.getString();
			break;

		case EX_FILENAME: {
			lex.next();
			string const name = lex.getString();
			filename.set(name, buffer.filePath());
			break;
		}

		case EX_DISPLAY: {
			lex.next();
			string const name = lex.getString();
			display = lyx::graphics::displayTranslator.find(name);
			break;
		}

		case EX_LYXSCALE:
			lex.next();
			lyxscale = lex.getInteger();
			break;

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

	// This is a trick to make sure that the data are self-consistent.
	settemplate(templatename_);

	lyxerr[Debug::EXTERNAL]
		<< "InsetExternal::Params::read: "
		<< "template: '"   << templatename()
		<< "' filename: '" << filename.absFilename()
		<< "' display: '"  << display
		<< "' scale: '"    << lyxscale
		<< '\'' << endl;

	return !read_error;
}
 
 
InsetExternal::InsetExternal()
	: renderer_(new ButtonRenderer)
{}


InsetExternal::InsetExternal(InsetExternal const & other)
	: InsetOld(other),
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
		BOOST_ASSERT(cmd.view());

		Buffer const & buffer = *cmd.view()->buffer();
		InsetExternal::Params p;
		InsetExternalMailer::string2params(cmd.argument, buffer, p);
		editExternal(p, buffer);
		return DISPATCHED_NOUPDATE;
	}

	case LFUN_INSET_MODIFY: {
		BOOST_ASSERT(cmd.view());

		Buffer const & buffer = *cmd.view()->buffer();
		InsetExternal::Params p;
		InsetExternalMailer::string2params(cmd.argument, buffer, p);
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


string const getScreenLabel(InsetExternal::Params const & params,
			    Buffer const & buffer)
{
	ExternalTemplate const * const ptr = getTemplatePtr(params);
	if (!ptr)
		return support::bformat(_("External template %1$s is not installed"),
					params.templatename());
	return doSubstitution(params, buffer, ptr->guiName);
}

} // namespace anon


InsetExternal::Params const & InsetExternal::params() const
{
	return params_;
}


void InsetExternal::setParams(Params const & p, Buffer const & buffer)
{
	// The stored params; what we would like to happen in an ideal world.
	params_ = p;

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


void InsetExternal::write(Buffer const & buffer, ostream & os) const
{
	params_.write(buffer, os);
}


void InsetExternal::read(Buffer const & buffer, LyXLex & lex)
{
	Params params;
	if (params.read(buffer, lex))
		setParams(params, buffer);
}


int InsetExternal::write(string const & format,
			 Buffer const & buf, ostream & os,
			 bool external_in_tmpdir) const
{
	ExternalTemplate const * const et_ptr = getTemplatePtr(params_);
	if (!et_ptr)
		return 0;
	ExternalTemplate const & et = *et_ptr;

	ExternalTemplate::Formats::const_iterator cit =
		et.formats.find(format);
	if (cit == et.formats.end()) {
		lyxerr[Debug::EXTERNAL]
			<< "External template format '" << format
			<< "' not specified in template "
			<< params_.templatename() << endl;
		return 0;
	}

	updateExternal(format, buf, external_in_tmpdir);
	string const str = doSubstitution(params_, buf, cit->second.product);
	os << str;
	return int(lyx::count(str.begin(), str.end(),'\n') + 1);
}


int InsetExternal::latex(Buffer const & buf, ostream & os,
			 LatexRunParams const & runparams) const
{
	// "nice" means that the buffer is exported to LaTeX format but not
	// run through the LaTeX compiler.
	// If we're running through the LaTeX compiler, we should write the
	// generated files in the bufer's temporary directory.
	bool const external_in_tmpdir =
		lyxrc.use_tempdir && !buf.temppath().empty() && !runparams.nice;

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


int InsetExternal::ascii(Buffer const & buf, ostream & os, int) const
{
	return write("Ascii", buf, os);
}


int InsetExternal::linuxdoc(Buffer const & buf, ostream & os) const
{
	return write("LinuxDoc", buf, os);
}


int InsetExternal::docbook(Buffer const & buf, ostream & os, bool) const
{
	return write("DocBook", buf, os);
}


void InsetExternal::validate(LaTeXFeatures & features) const
{
	ExternalTemplate const * const et_ptr = getTemplatePtr(params_);
	if (!et_ptr)
		return;
	ExternalTemplate const & et = *et_ptr;

	ExternalTemplate::Formats::const_iterator cit = et.formats.find("LaTeX");
	if (cit == et.formats.end())
		return;

	if (!cit->second.requirement.empty())
		features.require(cit->second.requirement);

	ExternalTemplateManager & etm = ExternalTemplateManager::get();

	vector<string>::const_iterator it  = cit->second.preambleNames.begin();
	vector<string>::const_iterator end = cit->second.preambleNames.end();
	for (; it != end; ++it) {
		string const preamble = etm.getPreambleDefByName(*it);
		if (!preamble.empty())
			features.addExternalPreamble(preamble);
	}
}


void InsetExternal::updateExternal(string const & format,
				   Buffer const & buf,
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
		from_format = support::getExtFromContents(from_file);
		if (from_format.empty())
			return;
	}

	string const to_format = outputFormat.updateFormat;
	if (to_format.empty())
		return;

	if (!converters.isReachable(from_format, to_format)) {
		lyxerr[Debug::EXTERNAL]
			<< "InsetExternal::updateExternal. "
			<< "Unable to convert from "
			<< from_format << " to " << to_format << endl;
		return;
	}

	if (external_in_tmpdir && !from_file.empty()) {
		// We are running stuff through LaTeX
		string const temp_file =
			support::MakeAbsPath(params_.filename.mangledFilename(),
					     buf.temppath());
		unsigned long const from_checksum = support::sum(from_file);
		unsigned long const temp_checksum = support::sum(temp_file);

		// Nothing to do...
		if (from_checksum == temp_checksum)
			return;

		// Cannot proceed...
		if (!support::copy(from_file, temp_file))
			return;
		from_file = temp_file;
	}

	string const to_file = doSubstitution(params_, buf,
					      outputFormat.updateResult);
	string const abs_to_file = support::MakeAbsPath(to_file, buf.filePath());

	// Do we need to perform the conversion?
	// Yes if to_file does not exist or if from_file is newer than to_file
	if (support::compare_timestamps(from_file, abs_to_file) < 0)
		return;

	string const to_filebase = support::ChangeExtension(to_file, string());
	converters.convert(&buf, from_file, to_filebase, from_format, to_format);
}


namespace {

/// Substitute meta-variables in this string
string const doSubstitution(InsetExternal::Params const & params,
			    Buffer const & buffer, string const & s)
{
	string result;
	string const buffer_path = buffer.filePath();
	string const filename = params.filename.outputFilename(buffer_path);
	string const basename = support::ChangeExtension(filename, string());
	string const filepath = support::OnlyPath(filename);

	result = support::subst(s, "$$FName", filename);
	result = support::subst(result, "$$Basename", basename);
	result = support::subst(result, "$$FPath", filepath);
	result = support::subst(result, "$$Tempname", params.tempname());
	result = support::subst(result, "$$Sysdir", support::system_lyxdir());

	// Handle the $$Contents(filename) syntax
	if (support::contains(result, "$$Contents(\"")) {

		string::size_type const pos = result.find("$$Contents(\"");
		string::size_type const end = result.find("\")", pos);
		string const file = result.substr(pos + 12, end - (pos + 12));
		string contents;

		string const filepath = support::IsFileReadable(file) ?
			buffer.filePath() : buffer.temppath();
		support::Path p(filepath);

		if (support::IsFileReadable(file))
			contents = support::GetFileContents(file);

		result = support::subst(result,
					("$$Contents(\"" + file + "\")").c_str(),
					contents);
	}

	return result;
}


void editExternal(InsetExternal::Params const & params, Buffer const & buffer)
{
	ExternalTemplate const * const et_ptr = getTemplatePtr(params);
	if (!et_ptr)
		return;
	ExternalTemplate const & et = *et_ptr;

	if (et.editCommand.empty())
		return;

	string const command = doSubstitution(params, buffer, et.editCommand);

	support::Path p(buffer.filePath());
	support::Forkedcall call;
	if (lyxerr.debugging(Debug::EXTERNAL)) {
		lyxerr << "Executing '" << command << "' in '"
		       << buffer.filePath() << '\'' << endl;
	}
	call.startscript(support::Forkedcall::DontWait, command);
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

	istringstream data(in);
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
		params.read(buffer, lex);
	}
}


string const
InsetExternalMailer::params2string(InsetExternal::Params const & params,
				   Buffer const & buffer)
{
	ostringstream data;
	data << name_ << ' ';
	params.write(buffer, data);
	data << "\\end_inset\n";
	return data.str();
}
