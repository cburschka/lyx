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
#include "insets/ExternalSupport.h"
#include "insets/ExternalTemplate.h"
#include "insets/render_button.h"
#include "insets/render_graphic.h"

#include "buffer.h"
#include "BufferView.h"
#include "debug.h"
#include "funcrequest.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "latexrunparams.h"
#include "lyx_main.h"
#include "lyxlex.h"
#include "lyxrc.h"
#include "metricsinfo.h"

#include "frontends/lyx_gui.h"
#include "frontends/LyXView.h"

#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/tostr.h"
#include "support/translator.h"

#include <boost/bind.hpp>

#include "support/std_sstream.h"

namespace support = lyx::support;
namespace external = lyx::external;

using std::endl;
using std::string;
using std::auto_ptr;
using std::istringstream;
using std::ostream;
using std::ostringstream;
using std::vector;


namespace {

lyx::graphics::DisplayType const defaultDisplayType = lyx::graphics::NoDisplay;

unsigned int defaultLyxScale = 100;

} // namespace anon


namespace lyx {
namespace external {

TempName::TempName()
{
	tempname_ = support::tempName(string(), "lyxext");
	support::unlink(tempname_);
	// must have an extension for the converter code to work correctly.
	tempname_ += ".tmp";
}


TempName::TempName(TempName const &)
{
	tempname_ = TempName()();
}


TempName::~TempName()
{
	support::unlink(tempname_);
}


TempName &
TempName::operator=(TempName const & other)
{
	if (this != &other)
		tempname_ = TempName()();
	return *this;
}

} // namespace external
} // namespace lyx


InsetExternalParams::InsetExternalParams()
	: display(defaultDisplayType),
	  lyxscale(defaultLyxScale)
{}


namespace {

template <typename T>
void clearIfNotFound(T & data, external::TransformID value,
		     vector<external::TransformID> const & ids)
{
	typedef vector<external::TransformID>::const_iterator
		const_iterator;

	const_iterator it  = ids.begin();
	const_iterator end = ids.end();
	it = std::find(it, end, value);
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


void InsetExternalParams::write(Buffer const & buffer, ostream & os) const
{
	os << "External\n"
	   << "\ttemplate " << templatename() << '\n';

	if (!filename.empty())
		os << "\tfilename "
		   << filename.outputFilename(buffer.filePath())
		   << '\n';

	if (display != defaultDisplayType)
		os << "\tdisplay "
		   << lyx::graphics::displayTranslator().find(display)
		   << '\n';

	if (lyxscale != defaultLyxScale)
		os << "\tlyxscale " << tostr(lyxscale) << '\n';

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
		os << "\trotateAngle " << rotationdata.angle() << '\n';
		if (rotationdata.origin() != external::RotationData::DEFAULT)
			os << "\trotateOrigin "
			   << rotationdata.originString() << '\n';
	}

	if (!resizedata.no_resize()) {
		using support::float_equal;

		if (!float_equal(resizedata.scale, 0.0, 0.05)) {
			if (!float_equal(resizedata.scale, 100.0, 0.05))
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


bool InsetExternalParams::read(Buffer const & buffer, LyXLex & lex)
{
	enum ExternalTags {
		EX_TEMPLATE = 1,
		EX_FILENAME,
		EX_DISPLAY,
		EX_LYXSCALE,
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
			display = lyx::graphics::displayTranslator().find(name);
			break;
		}

		case EX_LYXSCALE:
			lex.next();
			lyxscale = lex.getInteger();
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
			resizedata.height = LyXLength(lex.getString());
			break;

		case EX_KEEPASPECTRATIO:
			resizedata.keepAspectRatio = true;
			break;

		case EX_ROTATEANGLE:
			lex.next();
			rotationdata.angle(lex.getFloat());
			break;

		case EX_ROTATEORIGIN:
			lex.next();
			rotationdata.origin(lex.getString());
			break;

		case EX_SCALE:
			lex.next();
			resizedata.scale = lex.getFloat();
			break;

		case EX_WIDTH:
			lex.next();
			resizedata.width = LyXLength(lex.getString());
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

	if (!found_end)
		lex.printError("ExternalInset::read: Missing \\end_inset.");

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
	: InsetOld(other),
	  boost::signals::trackable(),
	  params_(other.params_),
	  renderer_(other.renderer_->clone())
{
	RenderGraphic * ptr =
		dynamic_cast<RenderGraphic *>(renderer_.get());
	if (ptr)
		ptr->connect(boost::bind(&InsetExternal::statusChanged, this));
}


auto_ptr<InsetBase> InsetExternal::clone() const
{
	return auto_ptr<InsetBase>(new InsetExternal(*this));
}


InsetExternal::~InsetExternal()
{
	InsetExternalMailer(*this).hideDialog();
}


void InsetExternal::statusChanged() const
{
	LyX::cref().updateInset(this);
}


dispatch_result InsetExternal::localDispatch(FuncRequest const & cmd)
{
	switch (cmd.action) {

	case LFUN_EXTERNAL_EDIT: {
		BOOST_ASSERT(cmd.view());

		Buffer const & buffer = *cmd.view()->buffer();
		InsetExternalParams p;
		InsetExternalMailer::string2params(cmd.argument, buffer, p);
		external::editExternal(p, buffer);
		return DISPATCHED_NOUPDATE;
	}

	case LFUN_INSET_MODIFY: {
		BOOST_ASSERT(cmd.view());

		Buffer const & buffer = *cmd.view()->buffer();
		InsetExternalParams p;
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

lyx::graphics::Params get_grfx_params(InsetExternalParams const & eparams)
{
	lyx::graphics::Params gparams;

	gparams.filename = eparams.filename.absFilename();
	gparams.scale = eparams.lyxscale;
	if (eparams.clipdata.clip)
		gparams.bb = eparams.clipdata.bbox;
	gparams.angle = eparams.rotationdata.angle();

	gparams.display = eparams.display;
	if (gparams.display == lyx::graphics::DefaultDisplay)
		gparams.display = lyxrc.display_graphics;
	// Override the above if we're not using a gui
	if (!lyx_gui::use_gui)
		gparams.display = lyx::graphics::NoDisplay;

	return gparams;
}


string const getScreenLabel(InsetExternalParams const & params,
			    Buffer const & buffer)
{
	external::Template const * const ptr =
		external::getTemplatePtr(params);
	if (!ptr)
		return support::bformat(_("External template %1$s is not installed"),
					params.templatename());
	return external::doSubstitution(params, buffer, ptr->guiName);
}

} // namespace anon


InsetExternalParams const & InsetExternal::params() const
{
	return params_;
}


void InsetExternal::setParams(InsetExternalParams const & p,
			      Buffer const & buffer)
{
	// The stored params; what we would like to happen in an ideal world.
	params_ = p;

	// We display the inset as a button by default.
	bool display_button = (!external::getTemplatePtr(params_) ||
			       params_.filename.empty() ||
			       params_.display == lyx::graphics::NoDisplay);

	if (display_button) {
		RenderButton * button_ptr =
			dynamic_cast<RenderButton *>(renderer_.get());
		if (!button_ptr) {
			button_ptr = new RenderButton;
			renderer_.reset(button_ptr);
		}

		button_ptr->update(getScreenLabel(params_, buffer), true);

	} else {
		RenderGraphic * graphic_ptr =
			dynamic_cast<RenderGraphic *>(renderer_.get());
		if (!graphic_ptr) {
			graphic_ptr = new RenderGraphic;
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
	InsetExternalParams params;
	if (params.read(buffer, lex))
		setParams(params, buffer);
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
		external::Template const * const et_ptr =
			external::getTemplatePtr(params_);
		if (!et_ptr)
			return 0;
		external::Template const & et = *et_ptr;

		external::Template::Formats::const_iterator cit =
			et.formats.find("PDFLaTeX");
		if (cit != et.formats.end())
			return external::writeExternal(params_, "PDFLaTeX",
					     buf, os, external_in_tmpdir);
	}

	return external::writeExternal(params_, "LaTeX",
				       buf, os, external_in_tmpdir);
}


int InsetExternal::ascii(Buffer const & buf, ostream & os, int) const
{
	return external::writeExternal(params_, "Ascii", buf, os);
}


int InsetExternal::linuxdoc(Buffer const & buf, ostream & os) const
{
	return external::writeExternal(params_, "LinuxDoc", buf, os);
}


int InsetExternal::docbook(Buffer const & buf, ostream & os, bool) const
{
	return external::writeExternal(params_, "DocBook", buf, os);
}


void InsetExternal::validate(LaTeXFeatures & features) const
{
	external::Template const * const et_ptr =
		external::getTemplatePtr(params_);
	if (!et_ptr)
		return;
	external::Template const & et = *et_ptr;

	external::Template::Formats::const_iterator cit =
		et.formats.find("LaTeX");
	if (cit == et.formats.end())
		return;

	if (!cit->second.requirement.empty())
		features.require(cit->second.requirement);

	external::TemplateManager & etm = external::TemplateManager::get();

	vector<string>::const_iterator it  = cit->second.preambleNames.begin();
	vector<string>::const_iterator end = cit->second.preambleNames.end();
	for (; it != end; ++it) {
		string const preamble = etm.getPreambleDefByName(*it);
		if (!preamble.empty())
			features.addExternalPreamble(preamble);
	}
}


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
InsetExternalMailer::params2string(InsetExternalParams const & params,
				   Buffer const & buffer)
{
	ostringstream data;
	data << name_ << ' ';
	params.write(buffer, data);
	data << "\\end_inset\n";
	return data.str();
}
