/**
 * \file QExternal.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "debug.h"
#include "lengthcommon.h"
#include "lyxrc.h"

#include "controllers/ControlExternal.h"

#include "insets/ExternalTemplate.h"
#include "insets/insetexternal.h"

#include "support/lstrings.h"
#include "support/tostr.h"

#include "QExternal.h"
#include "QExternalDialog.h"
#include "Qt2BC.h"

#include "checkedwidgets.h"
#include "lengthcombo.h"
#include "qt_helpers.h"

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qtabwidget.h>
#include <qtextview.h>

namespace external = lyx::external;

using lyx::support::isStrDbl;
using lyx::support::strToDbl;
using lyx::support::strToInt;
using lyx::support::token;
using lyx::support::trim;

using std::string;
using std::vector;
using std::find;


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


void setDisplay(QCheckBox & displayCB, QComboBox & showCO, QLineEdit & scaleED,
		external::DisplayType display, unsigned int scale,
		bool read_only)
{
	int item = 0;
	switch (display) {
	case external::DefaultDisplay:
		item = 0;
		break;
	case external::MonochromeDisplay:
		item = 1;
		break;
	case external::GrayscaleDisplay:
		item = 2;
		break;
	case external::ColorDisplay:
		item = 3;
		break;
	case external::PreviewDisplay:
		item = 4;
		break;
	case external::NoDisplay:
		item = 0;
		break;
	}

	showCO.setCurrentItem(item);
	bool const no_display = display == lyx::external::NoDisplay;
	showCO.setEnabled(!no_display && !read_only);
	displayCB.setChecked(!no_display);
	scaleED.setEnabled(!no_display && !read_only);
	scaleED.setText(toqstr(tostr(scale)));
}


void getDisplay(external::DisplayType & display,
		unsigned int & scale,
		QCheckBox const & displayCB,
		QComboBox const & showCO,
		QLineEdit const & scaleED)
{
	switch (showCO.currentItem()) {
	case 0:
		display = external::DefaultDisplay;
		break;
	case 1:
		display = external::MonochromeDisplay;
		break;
	case 2:
		display = external::GrayscaleDisplay;
		break;
	case 3:
		display = external::ColorDisplay;
		break;
	case 4:
		display = external::PreviewDisplay;
		break;
	}

	if (!displayCB.isChecked())
		display = external::NoDisplay;

	scale = strToInt(fromqstr(scaleED.text()));
}


void setRotation(QLineEdit & angleED, QComboBox & originCO,
		 external::RotationData const & data)
{
	originCO.setCurrentItem(int(data.origin()));
	angleED.setText(toqstr(tostr(data.angle())));
}


void getRotation(external::RotationData & data,
		 QLineEdit const & angleED, QComboBox const & originCO)
{
	typedef external::RotationData::OriginType OriginType;

	data.origin(static_cast<OriginType>(originCO.currentItem()));
	data.angle(strToDbl(fromqstr(angleED.text())));
}


void setSize(QLineEdit & widthED, QComboBox & widthUnitCO,
	     QLineEdit & heightED, LengthCombo & heightUnitCO,
	     QCheckBox & aspectratioCB,
	     external::ResizeData const & data)
{
	bool using_scale = data.usingScale();
	double scale =  data.scale;
	if (data.no_resize()) {
		// Everything is zero, so default to this!
		using_scale = true;
		scale = 100;
	}

	if (using_scale) {
		widthED.setText(toqstr(tostr(scale)));
		widthUnitCO.setCurrentItem(0);
	} else {
		widthED.setText(toqstr(tostr(data.width.value())));
		// Because 'Scale' is position 0...
		// Note also that width cannot be zero here, so
		// we don't need to worry about the default unit.
		widthUnitCO.setCurrentItem(data.width.unit() + 1);
	}

	string const h = data.height.zero() ? string() : data.height.asString();
	LyXLength::UNIT default_unit = data.width.zero() ?
		defaultUnit() : data.width.unit();
	lengthToWidgets(&heightED, &heightUnitCO, h, default_unit);

	heightED.setEnabled(!using_scale);
	heightUnitCO.setEnabled(!using_scale);

	aspectratioCB.setChecked(data.keepAspectRatio);

	bool const disable_aspectRatio = using_scale ||
		data.width.zero() || data.height.zero();
	aspectratioCB.setEnabled(!disable_aspectRatio);
}


void getSize(external::ResizeData & data,
	     QLineEdit const & widthED, QComboBox const & widthUnitCO,
	     QLineEdit const & heightED, LengthCombo const & heightUnitCO,
	     QCheckBox const & aspectratioCB)
{
	string const width = fromqstr(widthED.text());

	if (widthUnitCO.currentItem() > 0) {
		// Subtract one, because scale is 0.
		int const unit = widthUnitCO.currentItem() - 1;

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

	data.height = LyXLength(widgetsToLength(&heightED, &heightUnitCO));

	data.keepAspectRatio = aspectratioCB.isChecked();
}


void setCrop(QCheckBox & clipCB,
	     QLineEdit & xlED, QLineEdit & ybED,
	     QLineEdit & xrED, QLineEdit & ytED,
	     external::ClipData const & data)
{
	clipCB.setChecked(data.clip);
	lyx::graphics::BoundingBox const & bbox = data.bbox;
	xlED.setText(toqstr(tostr(bbox.xl)));
	ybED.setText(toqstr(tostr(bbox.yb)));
	xrED.setText(toqstr(tostr(bbox.xr)));
	ytED.setText(toqstr(tostr(bbox.yt)));
}


void getCrop(external::ClipData & data,
	     QCheckBox const & clipCB,
	     QLineEdit const & xlED, QLineEdit const & ybED,
	     QLineEdit const & xrED, QLineEdit const & ytED,
	     bool bb_changed)
{
	data.clip = clipCB.isChecked();

	if (!bb_changed)
		return;

	data.bbox.xl = strToInt(fromqstr(xlED.text()));
	data.bbox.yb = strToInt(fromqstr(ybED.text()));
	data.bbox.xr = strToInt(fromqstr(xrED.text()));
	data.bbox.yt = strToInt(fromqstr(ytED.text()));
}


void getExtra(external::ExtraData & data,
	      QExternal::MapType const & extra)
{
	typedef QExternal::MapType MapType;
	MapType::const_iterator it  = extra.begin();
	MapType::const_iterator const end = extra.end();
	for (; it != end; ++it)
		data.set(it->first, trim(fromqstr(it->second)));
}

} // namespace anon


typedef QController<ControlExternal, QView<QExternalDialog> > base_class;

QExternal::QExternal(Dialog & parent)
	: base_class(parent, _("LyX: External Material"))
{}


void QExternal::build_dialog()
{
	dialog_.reset(new QExternalDialog(this));

	bcview().setOK(dialog_->okPB);
	bcview().setApply(dialog_->applyPB);
	bcview().setCancel(dialog_->closePB);

	bcview().addReadOnly(dialog_->fileED);
	bcview().addReadOnly(dialog_->browsePB);
	bcview().addReadOnly(dialog_->editPB);
	bcview().addReadOnly(dialog_->externalCO);
	bcview().addReadOnly(dialog_->displayscaleED);
	bcview().addReadOnly(dialog_->showCO);
	bcview().addReadOnly(dialog_->displayCB);
	bcview().addReadOnly(dialog_->angleED);
	bcview().addReadOnly(dialog_->originCO);
	bcview().addReadOnly(dialog_->heightUnitCO);
	bcview().addReadOnly(dialog_->heightED);
	bcview().addReadOnly(dialog_->aspectratioCB);
	bcview().addReadOnly(dialog_->widthUnitCO);
	bcview().addReadOnly(dialog_->widthED);
	bcview().addReadOnly(dialog_->clipCB);
	bcview().addReadOnly(dialog_->getbbPB);
	bcview().addReadOnly(dialog_->ytED);
	bcview().addReadOnly(dialog_->xlED);
	bcview().addReadOnly(dialog_->xrED);
	bcview().addReadOnly(dialog_->ybED);
	bcview().addReadOnly(dialog_->extraFormatCO);
	bcview().addReadOnly(dialog_->extraED);

	addCheckedLineEdit(bcview(), dialog_->angleED, dialog_->angleLA);
	addCheckedLineEdit(bcview(), dialog_->displayscaleED, dialog_->scaleLA);
	addCheckedLineEdit(bcview(), dialog_->heightED, dialog_->heightLA);
	addCheckedLineEdit(bcview(), dialog_->widthED, dialog_->widthLA);
	addCheckedLineEdit(bcview(), dialog_->xlED, dialog_->lbLA);
	addCheckedLineEdit(bcview(), dialog_->ybED, dialog_->lbLA);
	addCheckedLineEdit(bcview(), dialog_->xrED, dialog_->rtLA);
	addCheckedLineEdit(bcview(), dialog_->ytED, dialog_->rtLA);

	std::vector<string> templates(controller().getTemplates());

	for (std::vector<string>::const_iterator cit = templates.begin();
		cit != templates.end(); ++cit) {
		dialog_->externalCO->insertItem(toqstr(*cit), -1);
	}

	// Fill the origins combo
	typedef vector<external::RotationDataType> Origins;
	Origins const & all_origins = external::all_origins();
	for (Origins::size_type i = 0; i != all_origins.size(); ++i)
		dialog_->originCO->insertItem(toqstr(external::origin_gui_str(i)));

	// Fill the width combo
	dialog_->widthUnitCO->insertItem(qt_("Scale%"));
	for (int i = 0; i < num_units; i++)
		dialog_->widthUnitCO->insertItem(unit_name_gui[i], -1);
}


void QExternal::update_contents()
{
	dialog_->tab->setCurrentPage(0);
	InsetExternalParams const & params = controller().params();

	string const name =
		params.filename.outputFilename(kernel().bufferFilepath());
	dialog_->fileED->setText(toqstr(name));

	dialog_->externalCO->setCurrentItem(
		controller().getTemplateNumber(params.templatename()));
	updateTemplate();

	setDisplay(*dialog_->displayCB, *dialog_->showCO,
		   *dialog_->displayscaleED,
		   params.display, params.lyxscale, readOnly());

	setRotation(*dialog_->angleED, *dialog_->originCO, params.rotationdata);

	setSize(*dialog_->widthED, *dialog_->widthUnitCO,
		*dialog_->heightED, *dialog_->heightUnitCO,
		*dialog_->aspectratioCB,
		params.resizedata);

	setCrop(*dialog_->clipCB,
		*dialog_->xlED, *dialog_->ybED,
		*dialog_->xrED, *dialog_->ytED,
		params.clipdata);
	controller().bbChanged(!params.clipdata.bbox.empty());

	isValid();
}


void QExternal::updateTemplate()
{
	external::Template templ =
		controller().getTemplate(dialog_->externalCO->currentItem());
	dialog_->externalTV->setText(toqstr(templ.helpText));

	// Ascertain which (if any) transformations the template supports
	// and disable tabs hosting unsupported transforms.
	typedef vector<external::TransformID> TransformIDs;
	TransformIDs const transformIds = templ.transformIds;
	TransformIDs::const_iterator tr_begin = transformIds.begin();
	TransformIDs::const_iterator const tr_end = transformIds.end();

	bool found = find(tr_begin, tr_end, external::Rotate) != tr_end;
	dialog_->tab->setTabEnabled(dialog_->rotatetab, found);

	found = find(tr_begin, tr_end, external::Resize) != tr_end;
	dialog_->tab->setTabEnabled(dialog_->scaletab, found);

	found = find(tr_begin, tr_end, external::Clip) != tr_end;
	dialog_->tab->setTabEnabled(dialog_->croptab, found);

	found = find(tr_begin, tr_end, external::Extra) != tr_end;
	dialog_->tab->setTabEnabled(dialog_->optionstab, found);
	if (!found)
		return;

	// Ascertain whether the template has any formats supporting
	// the 'Extra' option
	QLineEdit * const extraED = dialog_->extraED;
	QComboBox * const extraCB = dialog_->extraFormatCO;

	extra_.clear();
	extraED->clear();
	extraCB->clear();

	external::Template::Formats::const_iterator it  = templ.formats.begin();
	external::Template::Formats::const_iterator end = templ.formats.end();
	for (; it != end; ++it) {
		if (it->second.option_transformers.find(external::Extra) ==
		    it->second.option_transformers.end())
			continue;
		string const format = it->first;
		string const opt = controller().params().extradata.get(format);
		extraCB->insertItem(toqstr(format));
		extra_[format] = toqstr(opt);
	}

	bool const enabled = extraCB->count()  > 0;

	dialog_->tab->setTabEnabled(dialog_->optionstab, enabled);
	extraED->setEnabled(enabled && !kernel().isBufferReadonly());
	extraCB->setEnabled(enabled);

	if (enabled) {
		extraCB->setCurrentItem(0);
		extraED->setText(extra_[fromqstr(extraCB->currentText())]);
	}
}


void QExternal::apply()
{
	InsetExternalParams params = controller().params();

	params.filename.set(fromqstr(dialog_->fileED->text()),
			    kernel().bufferFilepath());

	params.settemplate(controller().getTemplate(
				   dialog_->externalCO->currentItem()).lyxName);

	getDisplay(params.display, params.lyxscale,
		   *dialog_->displayCB, *dialog_->showCO,
		   *dialog_->displayscaleED);

	if (dialog_->tab->isTabEnabled(dialog_->rotatetab))
		getRotation(params.rotationdata,
			    *dialog_->angleED, *dialog_->originCO);

	if (dialog_->tab->isTabEnabled(dialog_->scaletab))
		getSize(params.resizedata,
			*dialog_->widthED, *dialog_->widthUnitCO,
			*dialog_->heightED, *dialog_->heightUnitCO,
			*dialog_->aspectratioCB);

	if (dialog_->tab->isTabEnabled(dialog_->croptab))
		getCrop(params.clipdata,
			*dialog_->clipCB,
			*dialog_->xlED, *dialog_->ybED,
			*dialog_->xrED, *dialog_->ytED,
			controller().bbChanged());

	if (dialog_->tab->isTabEnabled(dialog_->optionstab))
		getExtra(params.extradata, extra_);

	controller().setParams(params);
}


void QExternal::getBB()
{
	dialog_->xlED->setText("0");
	dialog_->ybED->setText("0");
	dialog_->xrED->setText("0");
	dialog_->ytED->setText("0");

	string const filename = fromqstr(dialog_->fileED->text());
	if (filename.empty())
		return;

	string const bb = controller().readBB(filename);
	if (bb.empty())
		return;

	dialog_->xlED->setText(toqstr(token(bb, ' ', 0)));
	dialog_->ybED->setText(toqstr(token(bb, ' ', 1)));
	dialog_->xrED->setText(toqstr(token(bb, ' ', 2)));
	dialog_->ytED->setText(toqstr(token(bb, ' ', 3)));

	controller().bbChanged(false);
}
