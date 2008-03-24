/**
 * \file InsetExternal.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetExternal.h"
#include "insets/ExternalSupport.h"
#include "insets/ExternalTemplate.h"
#include "insets/RenderButton.h"
#include "insets/RenderGraphic.h"
#include "insets/RenderPreview.h"

#include "Buffer.h"
#include "Cursor.h"
#include "DispatchResult.h"
#include "Exporter.h"
#include "FuncStatus.h"
#include "FuncRequest.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "LyX.h" // use_gui
#include "LyXRC.h"
#include "MetricsInfo.h"
#include "OutputParams.h"

#include "frontends/alert.h"

#include "graphics/PreviewLoader.h"

#include "support/debug.h"
#include "support/ExceptionMessage.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/convert.h"
#include "support/Translator.h"

#include <boost/bind.hpp>

#include <sstream>

using namespace std;
using namespace lyx::support;

namespace {

lyx::external::DisplayType const defaultDisplayType = lyx::external::NoDisplay;

unsigned int const defaultLyxScale = 100;

string defaultTemplateName;

} // namespace anon


namespace lyx {

namespace Alert = frontend::Alert;

namespace external {

TempName::TempName()
{
	FileName const tempname = FileName::tempName("lyxext");
	// FIXME: This is unsafe
	tempname.removeFile();
	// must have an extension for the converter code to work correctly.
	tempname_ = FileName(tempname.absFilename() + ".tmp");
}


TempName::TempName(TempName const &)
{
	tempname_ = TempName()();
}


TempName::~TempName()
{
	tempname_.removeFile();
}


TempName & TempName::operator=(TempName const & other)
{
	if (this != &other)
		tempname_ = TempName()();
	return *this;
}


namespace {

/// The translator between the Display enum and corresponding lyx string.
Translator<DisplayType, string> const initTranslator()
{
	Translator<DisplayType, string> translator(DefaultDisplay, "default");

	// Fill the display translator
	translator.addPair(MonochromeDisplay, "monochrome");
	translator.addPair(GrayscaleDisplay, "grayscale");
	translator.addPair(ColorDisplay, "color");
	translator.addPair(PreviewDisplay, "preview");
	translator.addPair(NoDisplay, "none");

	return translator;
}

} // namespace anon


Translator<DisplayType, string> const & displayTranslator()
{
	static Translator<DisplayType, string> const translator =
		initTranslator();
	return translator;
}

} // namespace external


InsetExternalParams::InsetExternalParams()
	: display(defaultDisplayType),
	  lyxscale(defaultLyxScale),
	  draft(false)
{
	if (defaultTemplateName.empty()) {
		external::TemplateManager const & etm =
			external::TemplateManager::get();
		templatename_ = etm.getTemplates().begin()->first;
	} else
		templatename_ = defaultTemplateName;
}


namespace {

template <typename T>
void clearIfNotFound(T & data, external::TransformID value,
		     vector<external::TransformID> const & ids)
{
	typedef vector<external::TransformID>::const_iterator
		const_iterator;

	const_iterator it  = ids.begin();
	const_iterator end = ids.end();
	it = find(it, end, value);
	if (it == end)
		data = T();
}

} // namespace anon


void InsetExternalParams::settemplate(string const & name)
{
	templatename_ = name;

	external::TemplateManager const & etm =
		external::TemplateManager::get();
	external::Template const * const et = etm.getTemplateByName(name);
	if (!et)
		// Be safe. Don't lose data.
		return;

	// Ascertain which transforms the template supports.
	// Empty all those that it doesn't.
	vector<external::TransformID> const & ids = et->transformIds;
	clearIfNotFound(clipdata,     external::Clip,   ids);
	clearIfNotFound(extradata,    external::Extra,  ids);
	clearIfNotFound(resizedata,   external::Resize, ids);
	clearIfNotFound(rotationdata, external::Rotate, ids);
}


void InsetExternalParams::write(Buffer const & buf, ostream & os) const
{
	os << "External\n"
	   << "\ttemplate " << templatename() << '\n';

	if (!filename.empty()) {
		os << "\tfilename " << filename.outputFilename(buf.filePath()) << '\n';
		os << "\tembed " << (filename.embedded() ? filename.inzipName() : "\"\"") << '\n';
	}
	if (display != defaultDisplayType)
		os << "\tdisplay "
		   << external::displayTranslator().find(display)
		   << '\n';

	if (lyxscale != defaultLyxScale)
		os << "\tlyxscale " << convert<string>(lyxscale) << '\n';

	if (draft)
		os << "\tdraft\n";

	if (!clipdata.bbox.empty())
		os << "\tboundingBox " << clipdata.bbox << '\n';
	if (clipdata.clip)
		os << "\tclip\n";

	external::ExtraData::const_iterator it  = extradata.begin();
	external::ExtraData::const_iterator end = extradata.end();
	for (; it != end; ++it) {
		if (!it->second.empty())
			os << "\textra " << it->first << " \""
			   << it->second << "\"\n";
	}

	if (!rotationdata.no_rotation()) {
		os << "\trotateAngle " << rotationdata.adjAngle() << '\n';
		if (rotationdata.origin() != external::RotationData::DEFAULT)
			os << "\trotateOrigin "
			   << rotationdata.originString() << '\n';
	}

	if (!resizedata.no_resize()) {
		double const scl = convert<double>(resizedata.scale);
		if (!float_equal(scl, 0.0, 0.05)) {
			if (!float_equal(scl, 100.0, 0.05))
				os << "\tscale "
				   << resizedata.scale << '\n';
		} else {
			if (!resizedata.width.zero())
				os << "\twidth "
				   << resizedata.width.asString() << '\n';
			if (!resizedata.height.zero())
				os << "\theight "
				   << resizedata.height.asString() << '\n';
		}
		if (resizedata.keepAspectRatio)
			os << "\tkeepAspectRatio\n";
	}
}


bool InsetExternalParams::read(Buffer const & buffer, Lexer & lex)
{
	enum ExternalTags {
		EX_TEMPLATE = 1,
		EX_FILENAME,
		EX_EMBED,
		EX_DISPLAY,
		EX_LYXSCALE,
		EX_DRAFT,
		EX_BOUNDINGBOX,
		EX_CLIP,
		EX_EXTRA,
		EX_HEIGHT,
		EX_KEEPASPECTRATIO,
		EX_ROTATEANGLE,
		EX_ROTATEORIGIN,
		EX_SCALE,
		EX_WIDTH,
		EX_END
	};

	keyword_item external_tags[] = {
		{ "\\end_inset",     EX_END },
		{ "boundingBox",     EX_BOUNDINGBOX },
		{ "clip",            EX_CLIP },
		{ "display",         EX_DISPLAY},
		{ "draft",           EX_DRAFT},
		{ "embed",           EX_EMBED},
		{ "extra",           EX_EXTRA },
		{ "filename",        EX_FILENAME},
		{ "height",          EX_HEIGHT },
		{ "keepAspectRatio", EX_KEEPASPECTRATIO },
		{ "lyxscale",        EX_LYXSCALE},
		{ "rotateAngle",     EX_ROTATEANGLE },
		{ "rotateOrigin",    EX_ROTATEORIGIN },
		{ "scale",           EX_SCALE },
		{ "template",        EX_TEMPLATE },
		{ "width",           EX_WIDTH }
	};

	PushPopHelper pph(lex, external_tags, EX_END);

	bool found_end  = false;
	bool read_error = false;

	while (lex.isOK()) {
		switch (lex.lex()) {
		case EX_TEMPLATE:
			lex.next();
			templatename_ = lex.getString();
			break;

		case EX_FILENAME: {
			lex.eatLine();
			string const name = lex.getString();
			filename.set(name, buffer.filePath());
			break;
		}
		
		case EX_EMBED: {
			lex.next();
			string const name = lex.getString();
			filename.setInzipName(name);
			filename.setEmbed(!name.empty());
			break;
		}

		case EX_DISPLAY: {
			lex.next();
			string const name = lex.getString();
			display = external::displayTranslator().find(name);
			break;
		}

		case EX_LYXSCALE:
			lex.next();
			lyxscale = lex.getInteger();
			break;

		case EX_DRAFT:
			draft = true;
			break;

		case EX_BOUNDINGBOX:
			lex.next();
			clipdata.bbox.xl = lex.getInteger();
			lex.next();
			clipdata.bbox.yb = lex.getInteger();
			lex.next();
			clipdata.bbox.xr = lex.getInteger();
			lex.next();
			clipdata.bbox.yt = lex.getInteger();
			break;

		case EX_CLIP:
			clipdata.clip = true;
			break;

		case EX_EXTRA: {
			lex.next();
			string const name = lex.getString();
			lex.next();
			extradata.set(name, lex.getString());
			break;
		}

		case EX_HEIGHT:
			lex.next();
			resizedata.height = Length(lex.getString());
			break;

		case EX_KEEPASPECTRATIO:
			resizedata.keepAspectRatio = true;
			break;

		case EX_ROTATEANGLE:
			lex.next();
			rotationdata.angle = lex.getString();
			break;

		case EX_ROTATEORIGIN:
			lex.next();
			rotationdata.origin(lex.getString());
			break;

		case EX_SCALE:
			lex.next();
			resizedata.scale = lex.getString();
			break;

		case EX_WIDTH:
			lex.next();
			resizedata.width = Length(lex.getString());
			break;

		case EX_END:
			found_end = true;
			break;

		default:
			lex.printError("ExternalInset::read: Wrong tag: $$Token");
			read_error = true;
			break;
		}

		if (found_end || read_error)
			break;
	}

	if (!found_end)
		lex.printError("ExternalInsetParams::read: Missing \\end_inset.");

	// This is a trick to make sure that the data are self-consistent.
	settemplate(templatename_);

	if (lyxerr.debugging(Debug::EXTERNAL)) {
		lyxerr	<< "InsetExternalParams::read:\n";
		write(buffer, lyxerr);
	}

	return !read_error;
}


InsetExternal::InsetExternal()
	: renderer_(new RenderButton)
{}


InsetExternal::InsetExternal(InsetExternal const & other)
	: Inset(other),
	  boost::signals::trackable(),
	  params_(other.params_),
	  renderer_(other.renderer_->clone(this))
{}


InsetExternal::~InsetExternal()
{
	InsetExternalMailer(*this).hideDialog();
}


void InsetExternal::setBuffer(Buffer & buffer)
{
	if (buffer_) {
		try {
			// a file may not be copied successfully when, e.g. buffer_
			// has already been closed.
			params_.filename = params_.filename.copyTo(&buffer);
		} catch (ExceptionMessage const & message) {
			Alert::error(message.title_, message.details_);
			// failed to embed
			params_.filename.setEmbed(false);
		}
	}
	Inset::setBuffer(buffer);
}


void InsetExternal::statusChanged() const
{
	updateFrontend();
}


void InsetExternal::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {

	case LFUN_EXTERNAL_EDIT: {
		InsetExternalParams p =  params();
		if (!cmd.argument().empty())
			InsetExternalMailer::string2params(to_utf8(cmd.argument()), buffer(), p);
		external::editExternal(p, buffer());
		break;
	}

	case LFUN_INSET_MODIFY: {
		InsetExternalParams p;
		InsetExternalMailer::string2params(to_utf8(cmd.argument()), buffer(), p);
		setParams(p);
		break;
	}

	case LFUN_INSET_DIALOG_UPDATE:
		InsetExternalMailer(*this).updateDialog(&cur.bv());
		break;

	case LFUN_MOUSE_RELEASE:
		if (!cur.selection())
			InsetExternalMailer(*this).showDialog(&cur.bv());
		break;

	default:
		Inset::doDispatch(cur, cmd);
	}
}


bool InsetExternal::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action) {

	case LFUN_EXTERNAL_EDIT:
	case LFUN_INSET_MODIFY:
	case LFUN_INSET_DIALOG_UPDATE:
		flag.enabled(true);
		return true;

	default:
		return Inset::getStatus(cur, cmd, flag);
	}
}


void InsetExternal::registerEmbeddedFiles(EmbeddedFileList & files) const
{
	files.registerFile(params_.filename, this, buffer());
}


void InsetExternal::updateEmbeddedFile(EmbeddedFile const & file)
{
	params_.filename = file;
}


void InsetExternal::edit(Cursor & cur, bool, EntryDirection)
{
	InsetExternalMailer(*this).showDialog(&cur.bv());
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

enum RenderType {
	RENDERBUTTON,
	RENDERGRAPHIC,
	RENDERPREVIEW
};


RenderType getRenderType(InsetExternalParams const & p)
{
	if (!external::getTemplatePtr(p) ||
	    p.filename.empty() ||
	    p.display == external::NoDisplay)
		return RENDERBUTTON;

	if (p.display == external::PreviewDisplay) {
		if (RenderPreview::status() != LyXRC::PREVIEW_OFF)
			return RENDERPREVIEW;
		return RENDERBUTTON;
	}

	if (p.display == external::DefaultDisplay &&
	    lyxrc.display_graphics == graphics::NoDisplay)
		return RENDERBUTTON;
	return RENDERGRAPHIC;
}


graphics::Params get_grfx_params(InsetExternalParams const & eparams)
{
	graphics::Params gparams;

	gparams.filename = eparams.filename.availableFile();
	gparams.icon = eparams.filename.embedded() ? "pin.png" : "";
	gparams.scale = eparams.lyxscale;
	if (eparams.clipdata.clip)
		gparams.bb = eparams.clipdata.bbox;
	gparams.angle = convert<double>(eparams.rotationdata.adjAngle());

	switch (eparams.display) {
	case external::DefaultDisplay:
		gparams.display = graphics::DefaultDisplay;
		break;
	case external::MonochromeDisplay:
		gparams.display = graphics::MonochromeDisplay;
		break;
	case external::GrayscaleDisplay:
		gparams.display = graphics::GrayscaleDisplay;
		break;
	case external::ColorDisplay:
		gparams.display = graphics::ColorDisplay;
		break;
	case external::NoDisplay:
		gparams.display = graphics::NoDisplay;
		break;
	default:
		BOOST_ASSERT(false);
	}
	if (gparams.display == graphics::DefaultDisplay)
		gparams.display = graphics::DisplayType(lyxrc.display_graphics);
	// Override the above if we're not using a gui
	if (!use_gui)
		gparams.display = graphics::NoDisplay;

	return gparams;
}


docstring screenLabel(InsetExternalParams const & params,
			    Buffer const & buffer)
{
	external::Template const * const ptr =
		external::getTemplatePtr(params);
	if (!ptr)
		// FIXME UNICODE
		return bformat((_("External template %1$s is not installed")),
					from_utf8(params.templatename()));
	// FIXME UNICODE
	docstring gui = _(ptr->guiName);
	return from_utf8(external::doSubstitution(params, buffer,
				to_utf8(gui), false));
}

} // namespace anon


static bool isPreviewWanted(InsetExternalParams const & params)
{
	return params.display == external::PreviewDisplay &&
		params.filename.isReadableFile();
}


static docstring latexString(InsetExternal const & inset)
{
	odocstringstream os;
	// We don't need to set runparams.encoding since it is not used by
	// latex().
	OutputParams runparams(0);
	runparams.flavor = OutputParams::LATEX;
	inset.latex(os, runparams);
	return os.str();
}


static void add_preview_and_start_loading(RenderMonitoredPreview & renderer,
				   InsetExternal const & inset,
				   Buffer const & buffer)
{
	InsetExternalParams const & params = inset.params();

	if (RenderPreview::status() != LyXRC::PREVIEW_OFF &&
	    isPreviewWanted(params)) {
		renderer.setAbsFile(params.filename);
		docstring const snippet = latexString(inset);
		renderer.addPreview(snippet, buffer);
		renderer.startLoading(buffer);
	}
}


InsetExternalParams const & InsetExternal::params() const
{
	return params_;
}


void InsetExternal::setParams(InsetExternalParams const & p)
{
	params_ = p;

	// Subsequent calls to the InsetExternal::Params default constructor
	// will use this.
	defaultTemplateName = params_.templatename();

	switch (getRenderType(params_)) {
	case RENDERBUTTON: {
		RenderButton * button_ptr = renderer_->asButton();
		if (!button_ptr) {
			renderer_.reset(new RenderButton);
			button_ptr = renderer_->asButton();
		}

		button_ptr->update(screenLabel(params_, buffer()), true);
		break;
	}

	case RENDERGRAPHIC: {
		RenderGraphic * graphic_ptr = renderer_->asGraphic();
		if (!graphic_ptr) {
			renderer_.reset(new RenderGraphic(this));
			graphic_ptr = renderer_->asGraphic();
		}

		graphic_ptr->update(get_grfx_params(params_));

		break;
	}

	case RENDERPREVIEW: {
		RenderMonitoredPreview * preview_ptr =
			renderer_->asMonitoredPreview();
		if (!preview_ptr) {
			renderer_.reset(new RenderMonitoredPreview(this));
			preview_ptr = renderer_->asMonitoredPreview();
			preview_ptr->fileChanged(
				boost::bind(&InsetExternal::fileChanged, this));
		}

		if (preview_ptr->monitoring())
			preview_ptr->stopMonitoring();
		add_preview_and_start_loading(*preview_ptr, *this, buffer());

		break;
	}
	}
}


void InsetExternal::fileChanged() const
{
	Buffer const * const buffer = updateFrontend();
	if (!buffer)
		return;

	RenderMonitoredPreview * const ptr = renderer_->asMonitoredPreview();
	BOOST_ASSERT(ptr);

	ptr->removePreview(*buffer);
	add_preview_and_start_loading(*ptr, *this, *buffer);
}


void InsetExternal::write(ostream & os) const
{
	params_.write(buffer(), os);
}


void InsetExternal::read(Lexer & lex)
{
	InsetExternalParams params;
	if (params.read(buffer(), lex)) {
		// exception handling is not needed as long as embedded files are in place.
		params.filename.enable(buffer().embedded(), &buffer(), false);
		setParams(params);
	}
}


int InsetExternal::latex(odocstream & os, OutputParams const & runparams) const
{
	if (params_.draft) {
		// FIXME UNICODE
		os << "\\fbox{\\ttfamily{}"
		   << from_utf8(params_.filename.outputFilename(buffer().filePath()))
		   << "}\n";
		return 1;
	}

	// "nice" means that the buffer is exported to LaTeX format but not
	// run through the LaTeX compiler.
	// If we're running through the LaTeX compiler, we should write the
	// generated files in the buffer's temporary directory.
	bool const external_in_tmpdir = !runparams.nice;
	bool const dryrun = runparams.dryrun || runparams.inComment;

	// If the template has specified a PDFLaTeX output, then we try and
	// use that.
	if (runparams.flavor == OutputParams::PDFLATEX) {
		external::Template const * const et_ptr =
			external::getTemplatePtr(params_);
		if (!et_ptr)
			return 0;
		external::Template const & et = *et_ptr;

		external::Template::Formats::const_iterator cit =
			et.formats.find("PDFLaTeX");

		if (cit != et.formats.end()) {
			return external::writeExternal(params_, "PDFLaTeX",
						       buffer(), os,
						       *(runparams.exportdata),
						       external_in_tmpdir,
						       dryrun);
		}
	}

	return external::writeExternal(params_, "LaTeX", buffer(), os,
				       *(runparams.exportdata),
				       external_in_tmpdir,
				       dryrun);
}


int InsetExternal::plaintext(odocstream & os,
			     OutputParams const & runparams) const
{
	os << '\n'; // output external material on a new line
	external::writeExternal(params_, "Ascii", buffer(), os,
				*(runparams.exportdata), false,
				runparams.dryrun || runparams.inComment);
	return PLAINTEXT_NEWLINE;
}


int InsetExternal::docbook(odocstream & os,
			   OutputParams const & runparams) const
{
	return external::writeExternal(params_, "DocBook", buffer(), os,
				       *(runparams.exportdata), false,
				       runparams.dryrun || runparams.inComment);
}


void InsetExternal::validate(LaTeXFeatures & features) const
{
	if (params_.draft)
		return;

	external::Template const * const et_ptr =
		external::getTemplatePtr(params_);
	if (!et_ptr)
		return;
	external::Template const & et = *et_ptr;

	string format;
	switch (features.runparams().flavor) {
	case OutputParams::LATEX:
		format = "LaTeX";
		break;
	case OutputParams::PDFLATEX:
		format = "PDFLaTeX";
		break;
	case OutputParams::XML:
		format = "DocBook";
		break;
	}
	external::Template::Formats::const_iterator cit =
		et.formats.find(format);
	if (cit == et.formats.end())
		return;

	// FIXME: We don't need that always
	features.require("lyxdot");

	vector<string>::const_iterator it  = cit->second.requirements.begin();
	vector<string>::const_iterator end = cit->second.requirements.end();
	for (; it != end; ++it)
		features.require(*it);

	external::TemplateManager & etm = external::TemplateManager::get();

	it  = cit->second.preambleNames.begin();
	end = cit->second.preambleNames.end();
	for (; it != end; ++it) {
		string const preamble = etm.getPreambleDefByName(*it);
		if (!preamble.empty())
			features.addPreambleSnippet(preamble);
	}
}


//
// preview stuff
//

void InsetExternal::addPreview(graphics::PreviewLoader & ploader) const
{
	RenderMonitoredPreview * const ptr = renderer_->asMonitoredPreview();
	if (!ptr)
		return;

	if (isPreviewWanted(params())) {
		ptr->setAbsFile(params_.filename);
		docstring const snippet = latexString(*this);
		ptr->addPreview(snippet, ploader);
	}
}


/// Mailer stuff

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
					InsetExternalParams & params)
{
	params = InsetExternalParams();
	if (in.empty())
		return;

	istringstream data(in);
	Lexer lex(0,0);
	lex.setStream(data);

	string name;
	lex >> name;
	if (!lex || name != name_)
		return print_mailer_error("InsetExternalMailer", in, 1, name_);

	// This is part of the inset proper that is usually swallowed
	// by Text::readInset
	string id;
	lex >> id;
	if (!lex || id != "External")
		return print_mailer_error("InsetBoxMailer", in, 2, "External");

	params.read(buffer, lex);
}


string const
InsetExternalMailer::params2string(InsetExternalParams const & params,
				   Buffer const & buffer)
{
	ostringstream data;
	data << name_ << ' ';
	params.write(buffer, data);
	data << "\\end_inset\n";
	return data.str();
}

} // namespace lyx
