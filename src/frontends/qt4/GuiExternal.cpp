/**
 * \file GuiExternal.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiExternal.h"

#include "ControlExternal.h"
#include "lengthcommon.h"
#include "LyXRC.h"

#include "insets/ExternalTemplate.h"
#include "insets/InsetExternal.h"

#include "support/lstrings.h"
#include "support/convert.h"
#include "support/os.h"
#include "support/lyxlib.h"

#include "LengthCombo.h"
#include "qt_helpers.h"
#include "Validator.h"

#include <QCloseEvent>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QTextBrowser>

namespace external = lyx::external;

using lyx::support::isStrDbl;
using lyx::support::token;
using lyx::support::trim;
using lyx::support::float_equal;

using lyx::support::os::internal_path;

using std::string;
using std::vector;


namespace lyx {
namespace frontend {

GuiExternalDialog::GuiExternalDialog(LyXView & lv)
	: GuiDialog(lv, "external")
{
	setupUi(this);
	setViewTitle(_("External Material"));
	setController(new ControlExternal(*this));

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	connect(displayCB, SIGNAL(toggled(bool)),
		showCO, SLOT(setEnabled(bool)));
	connect(displayCB, SIGNAL(toggled(bool)),
		displayscaleED, SLOT(setEnabled(bool)));
	connect(showCO, SIGNAL(activated(const QString&)),
		this, SLOT(change_adaptor()));
	connect(originCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(aspectratioCB, SIGNAL(stateChanged(int)),
		this, SLOT(change_adaptor()));
	connect(browsePB, SIGNAL(clicked()),
		this, SLOT(browseClicked()));
	connect(editPB, SIGNAL(clicked()),
		this, SLOT(editClicked()));
	connect(externalCO, SIGNAL(activated(const QString &)),
		this, SLOT(templateChanged()));
	connect(extraED, SIGNAL(textChanged(const QString &)),
		this, SLOT(extraChanged(const QString&)));
	connect(extraFormatCO, SIGNAL(activated(const QString &)),
		this, SLOT(formatChanged(const QString&)));
	connect(widthUnitCO, SIGNAL(activated(int)),
		this, SLOT(widthUnitChanged()));
	connect(heightUnitCO, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(change_adaptor()));
	connect(displayCB, SIGNAL(stateChanged(int)),
		this, SLOT(change_adaptor()));
	connect(displayscaleED, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(angleED, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(widthED, SIGNAL(textChanged(const QString &)),
		this, SLOT(sizeChanged()));
	connect(heightED, SIGNAL(textChanged(const QString &)),
		this, SLOT(sizeChanged()));
	connect(fileED, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(clipCB, SIGNAL(stateChanged(int)),
		this, SLOT(change_adaptor()));
	connect(getbbPB, SIGNAL(clicked()), this, SLOT(getbbClicked()));
	connect(xrED, SIGNAL(textChanged(const QString &)), this, SLOT(bbChanged()));
	connect(ytED, SIGNAL(textChanged(const QString &)), this, SLOT(bbChanged()));
	connect(xlED, SIGNAL(textChanged(const QString &)), this, SLOT(bbChanged()));
	connect(ybED, SIGNAL(textChanged(const QString &)), this, SLOT(bbChanged()));
	connect(draftCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));

	QIntValidator * validator = new QIntValidator(displayscaleED);
	validator->setBottom(1);
	displayscaleED->setValidator(validator);

	angleED->setValidator(new QDoubleValidator(-360, 360, 2, angleED));

	xlED->setValidator(new QIntValidator(xlED));
	ybED->setValidator(new QIntValidator(ybED));
	xrED->setValidator(new QIntValidator(xrED));
	ytED->setValidator(new QIntValidator(ytED));

	widthED->setValidator(unsignedLengthValidator(widthED));
	heightED->setValidator(unsignedLengthValidator(heightED));

	setFocusProxy(fileED);

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);

	bc().setOK(okPB);
	bc().setApply(applyPB);
	bc().setCancel(closePB);

	bc().addReadOnly(fileED);
	bc().addReadOnly(browsePB);
	bc().addReadOnly(editPB);
	bc().addReadOnly(externalCO);
	bc().addReadOnly(draftCB);
	bc().addReadOnly(displayscaleED);
	bc().addReadOnly(showCO);
	bc().addReadOnly(displayCB);
	bc().addReadOnly(angleED);
	bc().addReadOnly(originCO);
	bc().addReadOnly(heightUnitCO);
	bc().addReadOnly(heightED);
	bc().addReadOnly(aspectratioCB);
	bc().addReadOnly(widthUnitCO);
	bc().addReadOnly(widthED);
	bc().addReadOnly(clipCB);
	bc().addReadOnly(getbbPB);
	bc().addReadOnly(ytED);
	bc().addReadOnly(xlED);
	bc().addReadOnly(xrED);
	bc().addReadOnly(ybED);
	bc().addReadOnly(extraFormatCO);
	bc().addReadOnly(extraED);

	bc().addCheckedLineEdit(angleED, angleLA);
	bc().addCheckedLineEdit(displayscaleED, scaleLA);
	bc().addCheckedLineEdit(heightED, heightLA);
	bc().addCheckedLineEdit(widthED, widthLA);
	bc().addCheckedLineEdit(xlED, lbLA);
	bc().addCheckedLineEdit(ybED, lbLA);
	bc().addCheckedLineEdit(xrED, rtLA);
	bc().addCheckedLineEdit(ytED, rtLA);
	bc().addCheckedLineEdit(fileED, fileLA);

	std::vector<string> templates(controller().getTemplates());

	for (std::vector<string>::const_iterator cit = templates.begin();
		cit != templates.end(); ++cit) {
		externalCO->addItem(qt_(*cit));
	}

	// Fill the origins combo
	typedef vector<external::RotationDataType> Origins;
	Origins const & all_origins = external::all_origins();
	for (Origins::size_type i = 0; i != all_origins.size(); ++i)
		originCO->addItem(toqstr(external::origin_gui_str(i)));

	// Fill the width combo
	widthUnitCO->addItem(qt_("Scale%"));
	for (int i = 0; i < num_units; i++)
		widthUnitCO->addItem(qt_(unit_name_gui[i]));
}


ControlExternal & GuiExternalDialog::controller() const
{
	return static_cast<ControlExternal &>(GuiDialog::controller());
}


bool GuiExternalDialog::activateAspectratio() const
{
	if (widthUnitCO->currentIndex() == 0)
		return false;

	string const wstr = fromqstr(widthED->text());
	if (wstr.empty())
		return false;
	bool const wIsDbl = isStrDbl(wstr);
	if (wIsDbl && float_equal(convert<double>(wstr), 0.0, 0.05))
		return false;
	Length l;
	if (!wIsDbl && (!isValidLength(wstr, &l) || l.zero()))
		return false;

	string const hstr = fromqstr(heightED->text());
	if (hstr.empty())
		return false;
	bool const hIsDbl = isStrDbl(hstr);
	if (hIsDbl && float_equal(convert<double>(hstr), 0.0, 0.05))
		return false;
	if (!hIsDbl && (!isValidLength(hstr, &l) || l.zero()))
		return false;

	return true;
}


void GuiExternalDialog::bbChanged()
{
	controller().bbChanged(true);
	changed();
}


void GuiExternalDialog::browseClicked()
{
	int const choice =  externalCO->currentIndex();
	docstring const template_name =
		from_utf8(controller().getTemplate(choice).lyxName);
	docstring const str =
		controller().browse(qstring_to_ucs4(fileED->text()),
					   template_name);
	if (!str.empty()) {
		fileED->setText(toqstr(str));
		changed();
	}
}


void GuiExternalDialog::change_adaptor()
{
	changed();
}


void GuiExternalDialog::closeEvent(QCloseEvent * e)
{
	slotWMHide();
	e->accept();
}


void GuiExternalDialog::editClicked()
{
	controller().editExternal();
}



void GuiExternalDialog::extraChanged(const QString& text)
{
	std::string const format = fromqstr(extraFormatCO->currentText());
	extra_[format] = text;
	changed();
}


void GuiExternalDialog::formatChanged(const QString& format)
{
	extraED->setText(extra_[fromqstr(format)]);
}


void GuiExternalDialog::getbbClicked()
{
	getBB();
}


void GuiExternalDialog::sizeChanged()
{
	aspectratioCB->setEnabled(activateAspectratio());
	changed();
}


void GuiExternalDialog::templateChanged()
{
	updateTemplate();
	changed();
}


void GuiExternalDialog::widthUnitChanged()
{
	bool useHeight = (widthUnitCO->currentIndex() > 0);

	if (useHeight)
		widthED->setValidator(unsignedLengthValidator(widthED));
	else
		widthED->setValidator(new QDoubleValidator(0, 1000, 2, widthED));

	heightED->setEnabled(useHeight);
	heightUnitCO->setEnabled(useHeight);
	changed();
}



namespace {

Length::UNIT defaultUnit()
{
	Length::UNIT default_unit = Length::CM;
	switch (lyxrc.default_papersize) {
	case PAPER_USLETTER:
	case PAPER_USLEGAL:
	case PAPER_USEXECUTIVE:
		default_unit = Length::IN;
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

	showCO.setCurrentIndex(item);
	bool const no_display = display == external::NoDisplay;
	showCO.setEnabled(!no_display && !read_only);
	displayCB.setChecked(!no_display);
	scaleED.setEnabled(!no_display && !read_only);
	scaleED.setText(toqstr(convert<string>(scale)));
}


void getDisplay(external::DisplayType & display,
		unsigned int & scale,
		QCheckBox const & displayCB,
		QComboBox const & showCO,
		QLineEdit const & scaleED)
{
	switch (showCO.currentIndex()) {
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

	scale = convert<int>(fromqstr(scaleED.text()));
}


void setRotation(QLineEdit & angleED, QComboBox & originCO,
		 external::RotationData const & data)
{
	originCO.setCurrentIndex(int(data.origin()));
	angleED.setText(toqstr(data.angle));
}


void getRotation(external::RotationData & data,
		 QLineEdit const & angleED, QComboBox const & originCO)
{
	typedef external::RotationData::OriginType OriginType;

	data.origin(static_cast<OriginType>(originCO.currentIndex()));
	data.angle = fromqstr(angleED.text());
}


void setSize(QLineEdit & widthED, QComboBox & widthUnitCO,
	     QLineEdit & heightED, LengthCombo & heightUnitCO,
	     QCheckBox & aspectratioCB,
	     external::ResizeData const & data)
{
	bool using_scale = data.usingScale();
	std::string scale = data.scale;
	if (data.no_resize()) {
		// Everything is zero, so default to this!
		using_scale = true;
		scale = "100";
	}

	if (using_scale) {
		widthED.setText(toqstr(scale));
		widthUnitCO.setCurrentIndex(0);
	} else {
		widthED.setText(toqstr(convert<string>(data.width.value())));
		// Because 'Scale' is position 0...
		// Note also that width cannot be zero here, so
		// we don't need to worry about the default unit.
		widthUnitCO.setCurrentIndex(data.width.unit() + 1);
	}

	string const h = data.height.zero() ? string() : data.height.asString();
	Length::UNIT default_unit = data.width.zero() ?
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

	if (widthUnitCO.currentIndex() > 0) {
		// Subtract one, because scale is 0.
		int const unit = widthUnitCO.currentIndex() - 1;

		Length w;
		if (isValidLength(width, &w))
			data.width = w;
		else if (isStrDbl(width))
			data.width = Length(convert<double>(width),
					   static_cast<Length::UNIT>(unit));
		else
			data.width = Length();

		data.scale = string();

	} else {
		// scaling instead of a width
		data.scale = width;
		data.width = Length();
	}

	data.height = Length(widgetsToLength(&heightED, &heightUnitCO));

	data.keepAspectRatio = aspectratioCB.isChecked();
}


void setCrop(QCheckBox & clipCB,
	     QLineEdit & xlED, QLineEdit & ybED,
	     QLineEdit & xrED, QLineEdit & ytED,
	     external::ClipData const & data)
{
	clipCB.setChecked(data.clip);
	graphics::BoundingBox const & bbox = data.bbox;
	xlED.setText(toqstr(convert<string>(bbox.xl)));
	ybED.setText(toqstr(convert<string>(bbox.yb)));
	xrED.setText(toqstr(convert<string>(bbox.xr)));
	ytED.setText(toqstr(convert<string>(bbox.yt)));
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

	data.bbox.xl = convert<int>(fromqstr(xlED.text()));
	data.bbox.yb = convert<int>(fromqstr(ybED.text()));
	data.bbox.xr = convert<int>(fromqstr(xrED.text()));
	data.bbox.yt = convert<int>(fromqstr(ytED.text()));
}


void getExtra(external::ExtraData & data,
	      GuiExternalDialog::MapType const & extra)
{
	typedef GuiExternalDialog::MapType MapType;
	MapType::const_iterator it  = extra.begin();
	MapType::const_iterator const end = extra.end();
	for (; it != end; ++it)
		data.set(it->first, trim(fromqstr(it->second)));
}

} // namespace anon



void GuiExternalDialog::update_contents()
{
	tab->setCurrentIndex(0);
	InsetExternalParams const & params = controller().params();

	string const name =
		params.filename.outputFilename(controller().bufferFilepath());
	fileED->setText(toqstr(name));

	externalCO->setCurrentIndex(
		controller().getTemplateNumber(params.templatename()));
	updateTemplate();

	draftCB->setChecked(params.draft);

	setDisplay(*displayCB, *showCO,
		   *displayscaleED,
		   params.display, params.lyxscale, readOnly());

	setRotation(*angleED, *originCO, params.rotationdata);

	setSize(*widthED, *widthUnitCO,
		*heightED, *heightUnitCO,
		*aspectratioCB,
		params.resizedata);

	setCrop(*clipCB,
		*xlED, *ybED,
		*xrED, *ytED,
		params.clipdata);
	controller().bbChanged(!params.clipdata.bbox.empty());

	isValid();
}


void GuiExternalDialog::updateTemplate()
{
	external::Template templ =
		controller().getTemplate(externalCO->currentIndex());
	externalTB->setPlainText(qt_(templ.helpText));

	// Ascertain which (if any) transformations the template supports
	// and disable tabs hosting unsupported transforms.
	typedef vector<external::TransformID> TransformIDs;
	TransformIDs const transformIds = templ.transformIds;
	TransformIDs::const_iterator tr_begin = transformIds.begin();
	TransformIDs::const_iterator const tr_end = transformIds.end();

	bool found = std::find(tr_begin, tr_end, external::Rotate) != tr_end;
	tab->setTabEnabled(tab->indexOf(rotatetab), found);
	found = std::find(tr_begin, tr_end, external::Resize) != tr_end;
	tab->setTabEnabled(tab->indexOf(scaletab), found);

	found = std::find(tr_begin, tr_end, external::Clip) != tr_end;
	tab->setTabEnabled(tab->indexOf(croptab), found);

	found = std::find(tr_begin, tr_end, external::Extra) != tr_end;
	tab->setTabEnabled(tab->indexOf(optionstab), found);

	if (!found)
		return;

	// Ascertain whether the template has any formats supporting
	// the 'Extra' option
	extra_.clear();
	extraED->clear();
	extraFormatCO->clear();

	external::Template::Formats::const_iterator it  = templ.formats.begin();
	external::Template::Formats::const_iterator end = templ.formats.end();
	for (; it != end; ++it) {
		if (it->second.option_transformers.find(external::Extra) ==
		    it->second.option_transformers.end())
			continue;
		string const format = it->first;
		string const opt = controller().params().extradata.get(format);
		extraFormatCO->addItem(toqstr(format));
		extra_[format] = toqstr(opt);
	}

	bool const enabled = extraFormatCO->count()  > 0;

	tab->setTabEnabled(
		tab->indexOf(optionstab), enabled);
	extraED->setEnabled(enabled && !controller().isBufferReadonly());
	extraFormatCO->setEnabled(enabled);

	if (enabled) {
		extraFormatCO->setCurrentIndex(0);
		extraED->setText(extra_[fromqstr(extraFormatCO->currentText())]);
	}
}


void GuiExternalDialog::applyView()
{
	InsetExternalParams params = controller().params();

	params.filename.set(internal_path(fromqstr(fileED->text())),
			    controller().bufferFilepath());

	params.settemplate(controller().getTemplate(
				   externalCO->currentIndex()).lyxName);

	params.draft = draftCB->isChecked();

	getDisplay(params.display, params.lyxscale,
		   *displayCB, *showCO,
		   *displayscaleED);

	if (tab->isTabEnabled(tab->indexOf(rotatetab)))
		getRotation(params.rotationdata, *angleED, *originCO);

	if (tab->isTabEnabled(tab->indexOf(scaletab)))
		getSize(params.resizedata, *widthED, *widthUnitCO,
			*heightED, *heightUnitCO, *aspectratioCB);

	if (tab->isTabEnabled(tab->indexOf(croptab)))
		getCrop(params.clipdata, *clipCB, *xlED, *ybED,
			*xrED, *ytED, controller().bbChanged());

	if (tab->isTabEnabled(tab->indexOf(optionstab)))
		getExtra(params.extradata, extra_);

	controller().setParams(params);
}


void GuiExternalDialog::getBB()
{
	xlED->setText("0");
	ybED->setText("0");
	xrED->setText("0");
	ytED->setText("0");

	string const filename = fromqstr(fileED->text());
	if (filename.empty())
		return;

	string const bb = controller().readBB(filename);
	if (bb.empty())
		return;

	xlED->setText(toqstr(token(bb, ' ', 0)));
	ybED->setText(toqstr(token(bb, ' ', 1)));
	xrED->setText(toqstr(token(bb, ' ', 2)));
	ytED->setText(toqstr(token(bb, ' ', 3)));

	controller().bbChanged(false);
}

} // namespace frontend
} // namespace lyx

#include "GuiExternal_moc.cpp"
