/**
 * \file FormExternal.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FormExternal.h"
#include "forms/form_external.h"

#include "checkedwidgets.h"
#include "input_validators.h"
#include "Tooltips.h"
#include "xforms_helpers.h"
#include "xformsBC.h"

#include "controllers/ControlExternal.h"

#include "lengthcommon.h"
#include "lyxrc.h"

#include "insets/ExternalTemplate.h"
#include "insets/insetexternal.h"

#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/tostr.h"

#include "lyx_forms.h"

namespace external = lyx::external;

using lyx::support::bformat;
using lyx::support::float_equal;
using lyx::support::getStringFromVector;
using lyx::support::isStrDbl;
using lyx::support::strToDbl;
using lyx::support::strToInt;
using lyx::support::token;
using lyx::support::trim;

using std::find;
using std::string;
using std::vector;


namespace {

LyXLength::UNIT defaultUnit()
{
	LyXLength::UNIT default_unit = LyXLength::CM;
	switch (lyxrc.default_papersize) {
	case PAPER_USLETTER:
	case PAPER_LEGALPAPER:
	case PAPER_EXECUTIVEPAPER:
		default_unit = LyXLength::IN;
		break;
	default:
		break;
	}
	return default_unit;
}


void setDisplay(FL_OBJECT * displayCB, FL_OBJECT * showCO, FL_OBJECT * scaleED,
		external::DisplayType display, unsigned int scale,
		bool read_only)
{
	BOOST_ASSERT(displayCB && displayCB->objclass == FL_CHECKBUTTON);
	BOOST_ASSERT(showCO && showCO->objclass == FL_CHOICE);
	BOOST_ASSERT(scaleED && scaleED->objclass == FL_INPUT);

	int item = 1;
	switch (display) {
	case external::DefaultDisplay:
		item = 1;
		break;
	case external::MonochromeDisplay:
		item = 2;
		break;
	case external::GrayscaleDisplay:
		item = 3;
		break;
	case external::ColorDisplay:
		item = 4;
		break;
	case external::PreviewDisplay:
		item = 5;
		break;
	case external::NoDisplay:
		item = 1;
		break;
	}

	fl_set_choice(showCO, item);

	bool const no_display = display == lyx::external::NoDisplay;
	setEnabled(showCO, !no_display && !read_only);

	fl_set_button(displayCB, !no_display);

	fl_set_input(scaleED, tostr(scale).c_str());
	setEnabled(scaleED, !no_display && !read_only);
}


void getDisplay(external::DisplayType & display,
		unsigned int & scale,
		FL_OBJECT * displayCB,
		FL_OBJECT * showCO,
		FL_OBJECT * scaleED)
{
	BOOST_ASSERT(displayCB && displayCB->objclass == FL_CHECKBUTTON);
	BOOST_ASSERT(showCO && showCO->objclass == FL_CHOICE);
	BOOST_ASSERT(scaleED && scaleED->objclass == FL_INPUT);

	switch (fl_get_choice(showCO)) {
	case 1:
		display = external::DefaultDisplay;
		break;
	case 2:
		display = external::MonochromeDisplay;
		break;
	case 3:
		display = external::GrayscaleDisplay;
		break;
	case 4:
		display = external::ColorDisplay;
		break;
	case 5:
		display = external::PreviewDisplay;
		break;
	}

	if (!fl_get_button(displayCB))
		display = external::NoDisplay;

	scale = strToInt(getString(scaleED));
}


void setRotation(FL_OBJECT * angleED, FL_OBJECT * originCO,
		 external::RotationData const & data)
{
	BOOST_ASSERT(angleED && angleED->objclass == FL_INPUT);
	BOOST_ASSERT(originCO && originCO->objclass == FL_CHOICE);

	fl_set_choice(originCO, 1 + int(data.origin()));
	fl_set_input(angleED, tostr(data.angle()).c_str());
}


void getRotation(external::RotationData & data,
		 FL_OBJECT * angleED, FL_OBJECT * originCO)
{
	BOOST_ASSERT(angleED && angleED->objclass == FL_INPUT);
	BOOST_ASSERT(originCO && originCO->objclass == FL_CHOICE);

	typedef external::RotationData::OriginType OriginType;

	data.origin(static_cast<OriginType>(fl_get_choice(originCO) - 1));
	data.angle(strToDbl(getString(angleED)));
}


void setSize(FL_OBJECT * widthED, FL_OBJECT * widthUnitCO,
	     FL_OBJECT * heightED, FL_OBJECT * heightUnitCO,
	     FL_OBJECT * aspectratioCB,
	     external::ResizeData const & data)
{
	BOOST_ASSERT(widthED && widthED->objclass == FL_INPUT);
	BOOST_ASSERT(widthUnitCO && widthUnitCO->objclass == FL_CHOICE);
	BOOST_ASSERT(heightED && heightED->objclass == FL_INPUT);
	BOOST_ASSERT(heightUnitCO && heightUnitCO->objclass == FL_CHOICE);
	BOOST_ASSERT(aspectratioCB &&
		     aspectratioCB->objclass == FL_CHECKBUTTON);

	bool using_scale = data.usingScale();
	double scale =  data.scale;
	if (data.no_resize()) {
		// Everything is zero, so default to this!
		using_scale = true;
		scale = 100;
	}

	if (using_scale) {
		fl_set_input(widthED, tostr(scale).c_str());
		fl_set_choice(widthUnitCO, 1);
	} else {
		fl_set_input(widthED, tostr(data.width.value()).c_str());
		// Because 'Scale' is position 1...
		// Note also that width cannot be zero here, so
		// we don't need to worry about the default unit.
		fl_set_choice(widthUnitCO, data.width.unit() + 2);
	}

	string const h = data.height.zero() ? string() : data.height.asString();
	LyXLength::UNIT default_unit = data.width.zero() ?
		defaultUnit() : data.width.unit();
	updateWidgetsFromLengthString(heightED, heightUnitCO,
				      h, stringFromUnit(default_unit));

	setEnabled(heightED, !using_scale);
	setEnabled(heightUnitCO, !using_scale);

	fl_set_button(aspectratioCB, data.keepAspectRatio);

	bool const disable_aspectRatio = using_scale ||
		data.width.zero() || data.height.zero();
	setEnabled(aspectratioCB, !disable_aspectRatio);
}


void getSize(external::ResizeData & data,
	     FL_OBJECT * widthED, FL_OBJECT * widthUnitCO,
	     FL_OBJECT * heightED, FL_OBJECT * heightUnitCO,
	     FL_OBJECT * aspectratioCB)
{
	BOOST_ASSERT(widthED && widthED->objclass == FL_INPUT);
	BOOST_ASSERT(widthUnitCO && widthUnitCO->objclass == FL_CHOICE);
	BOOST_ASSERT(heightED && heightED->objclass == FL_INPUT);
	BOOST_ASSERT(heightUnitCO && heightUnitCO->objclass == FL_CHOICE);
	BOOST_ASSERT(aspectratioCB &&
		     aspectratioCB->objclass == FL_CHECKBUTTON);

	string const width = getString(widthED);

	if (fl_get_choice(widthUnitCO) > 1) {
		// Subtract one, because scale is 1.
		int const unit = fl_get_choice(widthUnitCO) - 1;

		LyXLength w;
		if (isValidLength(width, &w))
			data.width = w;
		else if (isStrDbl(width))
			data.width = LyXLength(strToDbl(width),
					   static_cast<LyXLength::UNIT>(unit));
		else
			data.width = LyXLength();

		data.scale = 0.0;

	} else {
		// scaling instead of a width
		data.scale = strToDbl(width);
		data.width = LyXLength();
	}

	data.height = LyXLength(getLengthFromWidgets(heightED, heightUnitCO));

	data.keepAspectRatio = fl_get_button(aspectratioCB);
}


void setCrop(FL_OBJECT * clipCB,
	     FL_OBJECT * xlED, FL_OBJECT * ybED,
	     FL_OBJECT * xrED, FL_OBJECT * ytED,
	     external::ClipData const & data)
{
	BOOST_ASSERT(clipCB && clipCB->objclass == FL_CHECKBUTTON);
	BOOST_ASSERT(xlED && xlED->objclass == FL_INPUT);
	BOOST_ASSERT(ybED && ybED->objclass == FL_INPUT);
	BOOST_ASSERT(xrED && xrED->objclass == FL_INPUT);
	BOOST_ASSERT(ytED && ytED->objclass == FL_INPUT);

	fl_set_button(clipCB, data.clip);
	lyx::graphics::BoundingBox const & bbox = data.bbox;
	fl_set_input(xlED, tostr(bbox.xl).c_str());
	fl_set_input(ybED, tostr(bbox.yb).c_str());
	fl_set_input(xrED, tostr(bbox.xr).c_str());
	fl_set_input(ytED, tostr(bbox.yt).c_str());
}


void getCrop(external::ClipData & data,
	     FL_OBJECT * clipCB,
	     FL_OBJECT * xlED, FL_OBJECT * ybED,
	     FL_OBJECT * xrED, FL_OBJECT * ytED,
	     bool bb_changed)
{
	BOOST_ASSERT(clipCB && clipCB->objclass == FL_CHECKBUTTON);
	BOOST_ASSERT(xlED && xlED->objclass == FL_INPUT);
	BOOST_ASSERT(ybED && ybED->objclass == FL_INPUT);
	BOOST_ASSERT(xrED && xrED->objclass == FL_INPUT);
	BOOST_ASSERT(ytED && ytED->objclass == FL_INPUT);

	data.clip = fl_get_button(clipCB);

	if (!bb_changed)
		return;

	data.bbox.xl = strToInt(getString(xlED));
	data.bbox.yb = strToInt(getString(ybED));
	data.bbox.xr = strToInt(getString(xrED));
	data.bbox.yt = strToInt(getString(ytED));
}


void getExtra(external::ExtraData & data,
	      FormExternal::MapType const & extra)
{
	typedef FormExternal::MapType MapType;
	MapType::const_iterator it  = extra.begin();
	MapType::const_iterator const end = extra.end();
	for (; it != end; ++it)
		data.set(it->first, trim(it->second));
}

} // namespace anon


typedef FormController<ControlExternal, FormView<FD_external> > base_class;

FormExternal::FormExternal(Dialog & parent)
	: base_class(parent, _("External Material"))
{}


void FormExternal::build()
{
	dialog_.reset(build_external(this));
	file_.reset(build_external_file(this));
	lyxview_.reset(build_external_lyxview(this));
	rotate_.reset(build_external_rotate(this));
	scale_.reset(build_external_scale(this));
	crop_.reset(build_external_crop(this));
	options_.reset(build_external_options(this));

	bcview().setOK(dialog_->button_ok);
	bcview().setApply(dialog_->button_apply);
	bcview().setCancel(dialog_->button_close);

	bcview().addReadOnly(file_->input_file);
	bcview().addReadOnly(file_->button_browse);
	bcview().addReadOnly(file_->button_edit);
	bcview().addReadOnly(file_->choice_template);
	bcview().addReadOnly(file_->check_draft);

	bcview().addReadOnly(lyxview_->check_show);
	bcview().addReadOnly(lyxview_->choice_show);
	bcview().addReadOnly(lyxview_->input_displayscale);

	bcview().addReadOnly(rotate_->input_angle);
	bcview().addReadOnly(rotate_->choice_origin);

	bcview().addReadOnly(scale_->input_width);
	bcview().addReadOnly(scale_->choice_width);
	bcview().addReadOnly(scale_->input_height);
	bcview().addReadOnly(scale_->choice_height);
	bcview().addReadOnly(scale_->check_aspectratio);

	bcview().addReadOnly(crop_->check_bbox);
	bcview().addReadOnly(crop_->button_get_bbox);
	bcview().addReadOnly(crop_->input_xr);
	bcview().addReadOnly(crop_->input_yt);
	bcview().addReadOnly(crop_->input_xl);
	bcview().addReadOnly(crop_->input_yb);

	bcview().addReadOnly(options_->choice_option);
	bcview().addReadOnly(options_->input_option);

	// initial setting
//	addCheckedPositiveFloat(bcview(), scale_->input_width);
	// As I haven't written addCheckedPositiveFloat, we default to
	// always checking that it is a valide LyXLength, even when
	// I'm 'scaling'. No harm done, just not as strict as it might be.
	addCheckedLyXLength(bcview(), scale_->input_width);
	addCheckedLyXLength(bcview(), scale_->input_height);

//	addCheckedPositiveFloat(bcview(), input_displayscale);
	fl_set_input_filter(lyxview_->input_displayscale,
			    fl_unsigned_int_filter);

	fl_set_input_filter(crop_->input_xr, fl_unsigned_int_filter);
	fl_set_input_filter(crop_->input_yt, fl_unsigned_int_filter);
	fl_set_input_filter(crop_->input_xl, fl_unsigned_int_filter);
	fl_set_input_filter(crop_->input_yb, fl_unsigned_int_filter);

	fl_set_input_return(file_->input_file,            FL_RETURN_CHANGED);
	fl_set_input_return(lyxview_->input_displayscale, FL_RETURN_CHANGED);
	fl_set_input_return(rotate_->input_angle,         FL_RETURN_CHANGED);
	fl_set_input_return(scale_->input_width,          FL_RETURN_CHANGED);
	fl_set_input_return(scale_->input_height,         FL_RETURN_CHANGED);
	fl_set_input_return(crop_->input_xr,              FL_RETURN_CHANGED);
	fl_set_input_return(crop_->input_yt,              FL_RETURN_CHANGED);
	fl_set_input_return(crop_->input_xl,              FL_RETURN_CHANGED);
	fl_set_input_return(crop_->input_yb,              FL_RETURN_CHANGED);
	fl_set_input_return(options_->input_option,       FL_RETURN_CHANGED);

	// Trigger an input event for cut&paste with middle mouse button.
	setPrehandler(file_->input_file);
	setPrehandler(lyxview_->input_displayscale);
	setPrehandler(rotate_->input_angle);
	setPrehandler(scale_->input_width);
	setPrehandler(scale_->input_height);
	setPrehandler(crop_->input_xr);
	setPrehandler(crop_->input_yt);
	setPrehandler(crop_->input_xl);
	setPrehandler(crop_->input_yb);
	setPrehandler(options_->input_option);

	string const choice =
		' ' + getStringFromVector(controller().getTemplates(), " | ") +
		' ';
	fl_addto_choice(file_->choice_template, choice.c_str());

	string const display_list =
		_("Default|Monochrome|Grayscale|Color|Preview");
	fl_addto_choice(lyxview_->choice_show, display_list.c_str());

	// Fill the origins combo
	typedef vector<external::RotationDataType> Origins;
	Origins const & all_origins = external::all_origins();
	for (Origins::size_type i = 0; i != all_origins.size(); ++i)
		fl_addto_choice(rotate_->choice_origin,
				external::origin_gui_str(i).c_str());

	string const width_list = bformat(_("Scale%%%%|%1$s"),
					  choice_Length_All);
	fl_addto_choice(scale_->choice_width, width_list.c_str());

	fl_addto_choice(scale_->choice_height, choice_Length_All.c_str());

	// Set up the tooltips.
	string str = _("The file you want to insert.");
	tooltips().init(file_->input_file, str);
	str = _("Browse the directories.");
	tooltips().init(file_->button_browse, str);

	str = _("Scale the image to inserted percentage value.");
	tooltips().init(lyxview_->input_displayscale, str);
	str = _("Select display mode for this image.");
	tooltips().init(options_->choice_option, str);

	// Stack tabs
	tabmap_[FILETAB] =
		fl_addto_tabfolder(dialog_->tabfolder, _("File").c_str(),
				   file_->form);

	tabmap_[LYXVIEWTAB] =
		fl_addto_tabfolder(dialog_->tabfolder, _("LyX View").c_str(),
				   lyxview_->form);
	tabmap_[ROTATETAB] =
		fl_addto_tabfolder(dialog_->tabfolder, _("Rotate").c_str(),
				   rotate_->form);
	tabmap_[SCALETAB] =
		fl_addto_tabfolder(dialog_->tabfolder, _("Scale").c_str(),
				   scale_->form);
	tabmap_[CROPTAB] =
		fl_addto_tabfolder(dialog_->tabfolder, _("Crop").c_str(),
				   crop_->form);
	tabmap_[OPTIONSTAB] =
		fl_addto_tabfolder(dialog_->tabfolder, _("Options").c_str(),
				   options_->form);
}


void FormExternal::update()
{
	fl_set_folder_bynumber(dialog_->tabfolder, 1);
	InsetExternalParams const & params = controller().params();

	string const buffer_path = kernel().bufferFilepath();
	string const name = params.filename.outputFilename(buffer_path);
	fl_set_input(file_->input_file, name.c_str());

	int ID = controller().getTemplateNumber(params.templatename());
	if (ID < 0) ID = 0;
	fl_set_choice(file_->choice_template, ID+1);

	updateComboChange();

	fl_set_button(file_->check_draft, params.draft);

	setDisplay(lyxview_->check_show, lyxview_->choice_show,
		   lyxview_->input_displayscale,
		   params.display, params.lyxscale,
		   kernel().isBufferReadonly());

	setRotation(rotate_->input_angle, rotate_->choice_origin,
		    params.rotationdata);

	setSize(scale_->input_width, scale_->choice_width,
		scale_->input_height, scale_->choice_height,
		scale_->check_aspectratio,
		params.resizedata);

	setCrop(crop_->check_bbox,
		crop_->input_xl, crop_->input_yb,
		crop_->input_xr, crop_->input_yt,
		params.clipdata);
	controller().bbChanged(!params.clipdata.bbox.empty());
}


void FormExternal::updateComboChange()
{
	namespace external = lyx::external;

	int const choice = fl_get_choice(file_->choice_template) - 1;
	external::Template templ = controller().getTemplate(choice);

	// Update the help text
	string const txt = formatted(templ.helpText,
				     file_->browser_template->w - 20);
	fl_clear_browser(file_->browser_template);
	fl_addto_browser(file_->browser_template, txt.c_str());
	fl_set_browser_topline(file_->browser_template, 0);

	// Ascertain which (if any) transformations the template supports
	// and disable tabs hosting unsupported transforms.
	typedef vector<external::TransformID> TransformIDs;
	TransformIDs const transformIds = templ.transformIds;
	TransformIDs::const_iterator tr_begin = transformIds.begin();
	TransformIDs::const_iterator const tr_end = transformIds.end();

	tabmap_[FILETAB];
	tabmap_[LYXVIEWTAB];
	tabmap_[ROTATETAB];
	tabmap_[SCALETAB];
	tabmap_[CROPTAB];
	tabmap_[OPTIONSTAB];

	bool found = find(tr_begin, tr_end, external::Rotate) != tr_end;
	setEnabled(tabmap_[ROTATETAB], found);

	found = find(tr_begin, tr_end, external::Resize) != tr_end;
	setEnabled(tabmap_[SCALETAB], found);

	found = find(tr_begin, tr_end, external::Clip) != tr_end;
	setEnabled(tabmap_[CROPTAB], found);

	found = find(tr_begin, tr_end, external::Extra) != tr_end;
	setEnabled(tabmap_[OPTIONSTAB], found);

	if (!found)
		return;

	// Ascertain whether the template has any formats supporting
	// the 'Extra' option
	FL_OBJECT * const ob_input  = options_->input_option;
	FL_OBJECT * const ob_choice = options_->choice_option;
	extra_.clear();
	fl_set_input(ob_input, "");
	fl_clear_choice(ob_choice);

	external::Template::Formats::const_iterator it  = templ.formats.begin();
	external::Template::Formats::const_iterator end = templ.formats.end();
	for (; it != end; ++it) {
		if (it->second.option_transformers.find(external::Extra) ==
		    it->second.option_transformers.end())
			continue;
		string const format = it->first;
		string const opt = controller().params().extradata.get(format);
		fl_addto_choice(ob_choice, format.c_str());
		extra_[format] = opt;
	}

	bool const enabled = fl_get_choice_maxitems(ob_choice) > 0;

	setEnabled(tabmap_[OPTIONSTAB], enabled);
	setEnabled(ob_input,  enabled && !kernel().isBufferReadonly());
	setEnabled(ob_choice, enabled);

	if (enabled) {
		fl_set_choice(ob_choice, 1);
		string const format = fl_get_choice_text(ob_choice);
		fl_set_input(ob_input, extra_[format].c_str());
	}
}


void FormExternal::apply()
{
	InsetExternalParams params = controller().params();

	string const buffer_path = kernel().bufferFilepath();
	params.filename.set(getString(file_->input_file), buffer_path);

	int const choice = fl_get_choice(file_->choice_template) - 1;
	params.settemplate(controller().getTemplate(choice).lyxName);

	params.draft = fl_get_button(file_->check_draft);

	getDisplay(params.display, params.lyxscale,
		   lyxview_->check_show, lyxview_->choice_show,
		   lyxview_->input_displayscale);

	if (isActive(tabmap_[ROTATETAB]))
		getRotation(params.rotationdata,
			    rotate_->input_angle, rotate_->choice_origin);

	if (isActive(tabmap_[SCALETAB]))
		getSize(params.resizedata,
			scale_->input_width, scale_->choice_width,
			scale_->input_height, scale_->choice_height,
			scale_->check_aspectratio);

	if (isActive(tabmap_[CROPTAB]))
		getCrop(params.clipdata,
			crop_->check_bbox,
			crop_->input_xl, crop_->input_yb,
			crop_->input_xr, crop_->input_yt,
			controller().bbChanged());

	if (isActive(tabmap_[OPTIONSTAB]))
		getExtra(params.extradata, extra_);

	controller().setParams(params);
}


ButtonPolicy::SMInput FormExternal::input(FL_OBJECT * ob, long)
{
	ButtonPolicy::SMInput result = ButtonPolicy::SMI_VALID;

	if (ob == file_->choice_template) {

		// set to the chosen template
		updateComboChange();

	} else if (ob == file_->button_browse) {

		string const in_name  = fl_get_input(file_->input_file);

		int const choice = fl_get_choice(file_->choice_template) - 1;
		string const template_name =
			controller().getTemplate(choice).lyxName;
		string const out_name =
			controller().browse(in_name, template_name);
		fl_set_input(file_->input_file, out_name.c_str());

	} else if (ob == file_->button_edit) {
		controller().editExternal();
		result = ButtonPolicy::SMI_NOOP;

	} else if (ob == lyxview_->check_show) {

		bool const checked = fl_get_button(ob);
		setEnabled(lyxview_->choice_show, checked);
		setEnabled(lyxview_->input_displayscale, checked);

	} else if (ob == crop_->button_get_bbox) {

		getBB();

	} else if (ob == scale_->input_width ||
		   ob == scale_->input_height) {

		setEnabled(scale_->check_aspectratio,
			   activateAspectratio());

	} else if (ob == scale_->choice_width) {

		widthUnitChanged();

	} else if (ob == crop_->input_xr ||
		   ob == crop_->input_yt ||
		   ob == crop_->input_xl ||
		   ob == crop_->input_yb) {

		controller().bbChanged(true);

	} else if (ob == options_->input_option) {

		string const format =
			fl_get_choice_text(options_->choice_option);
		extra_[format] = getString(options_->input_option);

	} else if (ob == options_->choice_option) {

		string const format =
			fl_get_choice_text(options_->choice_option);
		fl_set_input(options_->input_option, extra_[format].c_str());
		result = ButtonPolicy::SMI_NOOP;
	}

	return result;
}


bool FormExternal::activateAspectratio() const
{
	if (fl_get_choice(scale_->choice_width) == 1)
		return false;

	string const wstr = getString(scale_->input_width);
	if (wstr.empty())
		return false;
	bool const wIsDbl = isStrDbl(wstr);
	if (wIsDbl && float_equal(strToDbl(wstr), 0.0, 0.05))
		return false;
	LyXLength l;
	if (!wIsDbl && (!isValidLength(wstr, &l) || l.zero()))
		return false;

	string const hstr = getString(scale_->input_height);
	if (hstr.empty())
		return false;
	bool const hIsDbl = isStrDbl(hstr);
	if (hIsDbl && float_equal(strToDbl(hstr), 0.0, 0.05))
		return false;
	if (!hIsDbl && (!isValidLength(hstr, &l) || l.zero()))
		return false;

	return true;
}


void FormExternal::getBB()
{
	fl_set_input(crop_->input_xl, "0");
	fl_set_input(crop_->input_yb, "0");
	fl_set_input(crop_->input_xr, "0");
	fl_set_input(crop_->input_yt, "0");

	string const filename = getString(file_->input_file);
	if (filename.empty())
		return;

	string const bb = controller().readBB(filename);
	if (bb.empty())
		return;

	fl_set_input(crop_->input_xl, token(bb, ' ', 0).c_str());
	fl_set_input(crop_->input_yb, token(bb, ' ', 1).c_str());
	fl_set_input(crop_->input_xr, token(bb, ' ', 2).c_str());
	fl_set_input(crop_->input_yt, token(bb, ' ', 3).c_str());

	controller().bbChanged(false);
}


void FormExternal::widthUnitChanged()
{
	if (fl_get_choice(scale_->choice_width) == 1)
		return;

	bool useHeight = fl_get_choice(scale_->choice_width) > 1;

//	if (useHeight)
//		widthED->setValidator(unsignedLengthValidator(widthED));
//	else
//		widthED->setValidator(new QDoubleValidator(0, 1000, 2, widthED));

	setEnabled(scale_->input_height, useHeight);
	setEnabled(scale_->choice_height, useHeight);
}
