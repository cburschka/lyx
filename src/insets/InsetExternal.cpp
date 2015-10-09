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
#include "insets/RenderButton.h"
#include "insets/RenderGraphic.h"
#include "insets/RenderPreview.h"

#include "Buffer.h"
#include "BufferView.h"
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
#include "output_latex.h"
#include "TocBackend.h"

#include "frontends/alert.h"
#include "frontends/Application.h"

#include "graphics/PreviewLoader.h"

#include "support/bind.h"
#include "support/convert.h"
#include "support/debug.h"
#include "support/ExceptionMessage.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/TempFile.h"

#include <sstream>
#include <vector>

using namespace std;
using namespace lyx::support;

namespace {

unsigned int const defaultLyxScale = 100;

string defaultTemplateName;

} // namespace anon


namespace lyx {

namespace Alert = frontend::Alert;

namespace external {

TempName::TempName() : tempfile_(new support::TempFile("lyxextXXXXXX.tmp"))
{
	// must have an extension for the converter code to work correctly.
}


TempName::TempName(TempName const & that) : tempfile_(0)
{
	*this = that;
}


TempName::~TempName()
{
	delete tempfile_;
}


TempName & TempName::operator=(TempName const & other)
{
	if (this != &other) {
		delete tempfile_;
		tempfile_ = new support::TempFile("lyxextXXXXXX.tmp");
	}
	return *this;
}


support::FileName TempName::operator()() const
{
	return tempfile_->name();
}

} // namespace external


InsetExternalParams::InsetExternalParams()
	: display(true),
	  lyxscale(defaultLyxScale),
	  draft(false)
{
	if (defaultTemplateName.empty()) {
		external::TemplateManager const & etm =
			external::TemplateManager::get();
		if (!etm.getTemplates().empty())
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

	// 
	preview_mode = et->preview_mode;
}


void InsetExternalParams::write(Buffer const & buf, ostream & os) const
{
	os << "External\n"
	   << "\ttemplate " << templatename() << '\n';

	if (!filename.empty())
		os << "\tfilename " << filename.outputFileName(buf.filePath()) << '\n';

	if (!display)
		os << "\tdisplay false\n";

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
	enum {
		EX_TEMPLATE = 1,
		EX_FILENAME,
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

	LexerKeyword external_tags[] = {
		{ "\\end_inset",     EX_END },
		{ "boundingBox",     EX_BOUNDINGBOX },
		{ "clip",            EX_CLIP },
		{ "display",         EX_DISPLAY},
		{ "draft",           EX_DRAFT},
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

	PushPopHelper pph(lex, external_tags);

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
			filename.set(name, buffer.originFilePath());
			break;
		}
		
		case EX_DISPLAY: {
			lex.next();
			display = lex.getString() != "false";
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
			clipdata.bbox.xl = Length(lex.getString());
			lex.next();
			clipdata.bbox.yb = Length(lex.getString());
			lex.next();
			clipdata.bbox.xr = Length(lex.getString());
			lex.next();
			clipdata.bbox.yt = Length(lex.getString());
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


namespace {

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
	gui += ": ";

	if (params.filename.empty())
		gui += "???";
	else
		gui += from_utf8(params.filename.relFileName(buffer.filePath()));

	return gui;
}

} // namespace anon



InsetExternal::InsetExternal(Buffer * buf)
	: Inset(buf), renderer_(new RenderButton)
{
}


// Mouse hover is not copied and remains empty
InsetExternal::InsetExternal(InsetExternal const & other)
	: Inset(other),
	  boost::signals::trackable(),
	  params_(other.params_),
	  renderer_(other.renderer_->clone(this))
{}


InsetExternal::~InsetExternal()
{
	hideDialogs("external", this);

	map<BufferView const *, bool>::iterator it = mouse_hover_.begin();
	map<BufferView const *, bool>::iterator end = mouse_hover_.end();
	for (; it != end; ++it)
		if (it->second)
			it->first->clearLastInset(this);
}


bool InsetExternal::setMouseHover(BufferView const * bv, bool mouse_hover) const
{
	mouse_hover_[bv] = mouse_hover;
	return true;
}


void InsetExternal::statusChanged() const
{
	updateFrontend();
}


void InsetExternal::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {

	case LFUN_INSET_EDIT: {
		InsetExternalParams p =  params();
		if (!cmd.argument().empty())
			string2params(to_utf8(cmd.argument()), buffer(), p);
		external::editExternal(p, buffer());
		break;
	}

	case LFUN_INSET_MODIFY: {
		InsetExternalParams p;
		string2params(to_utf8(cmd.argument()), buffer(), p);
		cur.recordUndo();
		setParams(p);
		break;
	}

	case LFUN_INSET_DIALOG_UPDATE:
		cur.bv().updateDialog("external",
			params2string(params(), cur.bv().buffer()));
		break;

	default:
		Inset::doDispatch(cur, cmd);
	}
}


bool InsetExternal::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action()) {

	case LFUN_INSET_EDIT:
	case LFUN_INSET_MODIFY:
	case LFUN_INSET_DIALOG_UPDATE:
		flag.setEnabled(true);
		return true;

	default:
		return Inset::getStatus(cur, cmd, flag);
	}
}


void InsetExternal::addToToc(DocIterator const & cpit, bool output_active,
							 UpdateType) const
{
	DocIterator pit = cpit;
	pit.push_back(CursorSlice(const_cast<InsetExternal &>(*this)));
	shared_ptr<Toc> toc = buffer().tocBackend().toc("external");
	docstring str = screenLabel(params_, buffer());
	toc->push_back(TocItem(pit, 0, str, output_active));
}


bool InsetExternal::showInsetDialog(BufferView * bv) const
{
	bv->showDialog("external", params2string(params(), bv->buffer()),
		const_cast<InsetExternal *>(this));
	return true;
}


void InsetExternal::metrics(MetricsInfo & mi, Dimension & dim) const
{
	renderer_->metrics(mi, dim);
}


void InsetExternal::draw(PainterInfo & pi, int x, int y) const
{
	if (renderer_->asButton())
		renderer_->setRenderState(mouse_hover_[pi.base.bv]);
	renderer_->draw(pi, x, y);
}


namespace {

graphics::Params get_grfx_params(InsetExternalParams const & eparams)
{
	graphics::Params gparams;

	gparams.filename = eparams.filename;
	gparams.scale = eparams.lyxscale;
	if (eparams.clipdata.clip)
		gparams.bb = eparams.clipdata.bbox;
	gparams.angle = convert<double>(eparams.rotationdata.adjAngle());
	gparams.display = eparams.display;

	return gparams;
}

} // namespace anon


static bool isPreviewWanted(InsetExternalParams const & params)
{
	return params.display && params.filename.isReadableFile();
}


static docstring latexString(InsetExternal const & inset)
{
	TexRow texrow;
	odocstringstream ods;
	otexstream os(ods, texrow);
	// We don't need to set runparams.encoding since it is not used by
	// latex().
	OutputParams runparams(0);
	runparams.flavor = OutputParams::LATEX;
	inset.latex(os, runparams);
	return ods.str();
}


static void add_preview_and_start_loading(RenderMonitoredPreview & renderer,
				   InsetExternal const & inset,
				   Buffer const & buffer)
{
	InsetExternalParams const & params = inset.params();

	if (RenderPreview::previewText() && isPreviewWanted(params)) {
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


void InsetExternal::updatePreview()
{
	setParams(params_);
}


void InsetExternal::setParams(InsetExternalParams const & p)
{
	params_ = p;

	// Subsequent calls to the InsetExternal::Params default constructor
	// will use this.
	defaultTemplateName = params_.templatename();

	if (!external::getTemplatePtr(params_) || params_.filename.empty()
		|| !params_.display
		|| !lyxrc.display_graphics
		|| params_.preview_mode == PREVIEW_OFF
		|| (params_.preview_mode == PREVIEW_INSTANT
		    && !RenderPreview::previewText())) {
		RenderButton * button_ptr = renderer_->asButton();
		if (!button_ptr) {
			renderer_.reset(new RenderButton);
			button_ptr = renderer_->asButton();
		}
		button_ptr->update(screenLabel(params_, buffer()), true);
		return;
	}

	switch (params_.preview_mode) {
	case PREVIEW_OFF:
		// Already taken care of above.
		LASSERT(false, return);
		break;
	case PREVIEW_INSTANT: {
		renderer_.reset(new RenderMonitoredPreview(this));
		RenderMonitoredPreview * preview_ptr = preview_ptr = renderer_->asMonitoredPreview();
		preview_ptr->fileChanged(bind(&InsetExternal::fileChanged, this));
		if (preview_ptr->monitoring())
			preview_ptr->stopMonitoring();
		add_preview_and_start_loading(*preview_ptr, *this, buffer());
		break;
	} 
	case PREVIEW_GRAPHICS: {
		RenderGraphic * graphic_ptr = renderer_->asGraphic();
		if (!graphic_ptr) {
			renderer_.reset(new RenderGraphic(this));
			graphic_ptr = renderer_->asGraphic();
		}
		graphic_ptr->update(get_grfx_params(params_));
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
	LASSERT(ptr, return);

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
	if (params.read(buffer(), lex))
		setParams(params);
}


void InsetExternal::latex(otexstream & os, OutputParams const & runparams) const
{
	if (params_.draft) {
		// FIXME UNICODE
		os << "\\fbox{\\ttfamily{}"
		   << from_utf8(params_.filename.outputFileName(buffer().filePath()))
		   << "}\n";
		return;
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
			return;
		external::Template const & et = *et_ptr;

		external::Template::Formats::const_iterator cit =
			et.formats.find("PDFLaTeX");

		if (cit != et.formats.end()) {
			external::writeExternal(params_, "PDFLaTeX",
						buffer(), os,
						*(runparams.exportdata),
						external_in_tmpdir,
						dryrun);
			return;
		}
	}

	external::writeExternal(params_, "LaTeX", buffer(), os,
				*(runparams.exportdata),
				external_in_tmpdir,
				dryrun);
}


int InsetExternal::plaintext(odocstringstream & os,
			     OutputParams const & runparams, size_t) const
{
	// this is too slow for constant use
	if (runparams.for_tooltip)
		return 0;

	TexRow texrow;
	otexstream ots(os, texrow);
	ots << '\n'; // output external material on a new line
	external::writeExternal(params_, "Ascii", buffer(), ots,
				*(runparams.exportdata), false,
				runparams.dryrun || runparams.inComment);
	return PLAINTEXT_NEWLINE;
}


int InsetExternal::docbook(odocstream & os,
			   OutputParams const & runparams) const
{
	TexRow texrow;
	odocstringstream ods;
	otexstream ots(ods, texrow);
	external::writeExternal(params_, "DocBook", buffer(), ots,
				*(runparams.exportdata), false,
				runparams.dryrun || runparams.inComment);
	os << ods.str();
	return int(count(ods.str().begin(), ods.str().end(), '\n'));
}


docstring InsetExternal::xhtml(XHTMLStream  & /*xs*/,
			OutputParams const & /*rp*/) const
{
//	external::writeExternal(params_, "XHTML", buffer(), os,
//				       *(runparams.exportdata), false,
//				       runparams.dryrun || runparams.inComment);
	return docstring();
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
	case OutputParams::DVILUATEX:
		format = "LaTeX";
		break;
	case OutputParams::LUATEX:
	case OutputParams::PDFLATEX:
	case OutputParams::XETEX:
		format = "PDFLaTeX";
		break;
	case OutputParams::XML:
		format = "DocBook";
		break;
	case OutputParams::HTML:
		format = "html";
		break;
	case OutputParams::TEXT:
		format = "text";
		break;
	case OutputParams::LYX:
		format = "lyx";
		break;
	}
	external::Template::Formats::const_iterator cit =
		et.formats.find(format);

	if (cit == et.formats.end()) {
		// If the template has not specified a PDFLaTeX output,
		// we try the LaTeX format.
		if (format == "PDFLaTeX") {
			cit = et.formats.find("LaTeX");
			if (cit == et.formats.end())
				return;
		} else
			return;
	}

	// FIXME: We don't need that always, see InsetGraphics
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


void InsetExternal::addPreview(DocIterator const & /*inset_pos*/,
	graphics::PreviewLoader & ploader) const	
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


string InsetExternal::contextMenuName() const
{
	return "context-external";
}


void InsetExternal::string2params(string const & in, Buffer const & buffer,
	InsetExternalParams & params)
{
	params = InsetExternalParams();
	if (in.empty())
		return;

	istringstream data(in);
	Lexer lex;
	lex.setStream(data);

	string name;
	lex >> name;
	if (!lex || name != "external") {
		LYXERR0("InsetExternal::string2params(" << in << ")\n"
					  "Expected arg 1 to be \"external\"\n");
		return;
	}

	// This is part of the inset proper that is usually swallowed
	// by Text::readInset
	string id;
	lex >> id;
	if (!lex || id != "External") {
		LYXERR0("InsetExternal::string2params(" << in << ")\n"
					  "Expected arg 2 to be \"External\"\n");
		return;
	}

	params.read(buffer, lex);
}


string InsetExternal::params2string(InsetExternalParams const & params,
	Buffer const & buffer)
{
	ostringstream data;
	data << "external" << ' ';
	params.write(buffer, data);
	data << "\\end_inset\n";
	return data.str();
}

} // namespace lyx
