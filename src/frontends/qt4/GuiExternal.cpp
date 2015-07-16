/**
 * \file GuiExternal.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 * \author Asger Alstrup
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiExternal.h"

#include "FuncRequest.h"
#include "support/gettext.h"
#include "Length.h"
#include "LyXRC.h"

#include "insets/ExternalSupport.h"
#include "insets/ExternalTemplate.h"
#include "insets/InsetExternal.h"

#include "graphics/epstools.h"
#include "graphics/GraphicsCache.h"
#include "graphics/GraphicsCacheItem.h"
#include "graphics/GraphicsImage.h"

#include "support/convert.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/os.h"

#include "LengthCombo.h"
#include "qt_helpers.h"
#include "Validator.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QTextBrowser>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {

using namespace external;

namespace {

RotationDataType origins[] = {
	RotationData::DEFAULT,
	RotationData::TOPLEFT,
	RotationData::BOTTOMLEFT,
	RotationData::BASELINELEFT,
	RotationData::CENTER,
	RotationData::TOPCENTER,
	RotationData::BOTTOMCENTER,
	RotationData::BASELINECENTER,
	RotationData::TOPRIGHT,
	RotationData::BOTTOMRIGHT,
	RotationData::BASELINERIGHT
};


// These are the strings, corresponding to the above, that the GUI should
// use. Note that they can/should be translated.
char const * const origin_gui_strs[] = {
	N_("Default"),
	N_("Top left"), N_("Bottom left"), N_("Baseline left"),
	N_("Center"), N_("Top center"), N_("Bottom center"), N_("Baseline center"),
	N_("Top right"), N_("Bottom right"), N_("Baseline right")
};

external::Template getTemplate(int i)
{
	if (external::TemplateManager::get().getTemplates().empty())
		return Template();
	external::TemplateManager::Templates::const_iterator i1
		= external::TemplateManager::get().getTemplates().begin();
	advance(i1, i);
	return i1->second;
}

} // namespace anon


GuiExternal::GuiExternal(GuiView & lv)
	: GuiDialog(lv, "external", qt_("External Material")), bbChanged_(false)
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	/*
	connect(displayGB, SIGNAL(toggled(bool)),
		displayscaleED, SLOT(setEnabled(bool)));
		*/
	connect(originCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(aspectratioCB, SIGNAL(stateChanged(int)),
		this, SLOT(change_adaptor()));
	connect(browsePB, SIGNAL(clicked()),
		this, SLOT(browseClicked()));
	connect(externalCO, SIGNAL(activated(QString)),
		this, SLOT(templateChanged()));
	connect(extraED, SIGNAL(textChanged(QString)),
		this, SLOT(extraChanged(QString)));
	connect(extraFormatCO, SIGNAL(activated(QString)),
		this, SLOT(formatChanged(QString)));
	connect(widthUnitCO, SIGNAL(activated(int)),
		this, SLOT(widthUnitChanged()));
	connect(heightUnitCO, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(change_adaptor()));
	connect(displayGB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(displayscaleED, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(angleED, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(widthED, SIGNAL(textChanged(QString)),
		this, SLOT(sizeChanged()));
	connect(heightED, SIGNAL(textChanged(QString)),
		this, SLOT(sizeChanged()));
	connect(fileED, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(clipCB, SIGNAL(stateChanged(int)),
		this, SLOT(change_adaptor()));
	connect(getbbPB, SIGNAL(clicked()), this, SLOT(getbbClicked()));
	connect(xrED, SIGNAL(textChanged(QString)), this, SLOT(bbChanged()));
	connect(ytED, SIGNAL(textChanged(QString)), this, SLOT(bbChanged()));
	connect(xlED, SIGNAL(textChanged(QString)), this, SLOT(bbChanged()));
	connect(ybED, SIGNAL(textChanged(QString)), this, SLOT(bbChanged()));
	connect(xrUnitCO, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(bbChanged()));
	connect(ytUnitCO, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(bbChanged()));
	connect(xlUnitCO, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(bbChanged()));
	connect(ybUnitCO, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(bbChanged()));
	connect(draftCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));

	QIntValidator * validator = new QIntValidator(displayscaleED);
	validator->setBottom(1);
	displayscaleED->setValidator(validator);

	angleED->setValidator(new QDoubleValidator(-360, 360, 2, angleED));

	xlED->setValidator(unsignedLengthValidator(xlED));
	ybED->setValidator(unsignedLengthValidator(ybED));
	xrED->setValidator(unsignedLengthValidator(xrED));
	ytED->setValidator(unsignedLengthValidator(ytED));

	widthED->setValidator(unsignedLengthValidator(widthED));
	heightED->setValidator(unsignedLengthValidator(heightED));

	setFocusProxy(fileED);

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);

	bc().setOK(okPB);
	bc().setApply(applyPB);
	bc().setCancel(closePB);

	bc().addReadOnly(fileED);
	bc().addReadOnly(browsePB);
	bc().addReadOnly(externalCO);
	bc().addReadOnly(draftCB);
	bc().addReadOnly(displayscaleED);
	bc().addReadOnly(displayGB);
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
	bc().addReadOnly(ytUnitCO);
	bc().addReadOnly(xlUnitCO);
	bc().addReadOnly(xrUnitCO);
	bc().addReadOnly(ybUnitCO);
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

	external::TemplateManager::Templates::const_iterator i1, i2;
	i1 = external::TemplateManager::get().getTemplates().begin();
	i2 = external::TemplateManager::get().getTemplates().end();
	for (; i1 != i2; ++i1)
		externalCO->addItem(qt_(i1->second.guiName));

	// Fill the origins combo
	for (size_t i = 0; i != sizeof(origins) / sizeof(origins[0]); ++i)
		originCO->addItem(qt_(origin_gui_strs[i]));

	// add scale item
	widthUnitCO->insertItem(0, qt_("Scale%"), "scale");
}


bool GuiExternal::activateAspectratio() const
{
	if (usingScale())
		return false;

	QString const wstr = widthED->text();
	if (wstr.isEmpty())
		return false;
	bool wIsDbl;
	double val = wstr.trimmed().toDouble(&wIsDbl);
	if (wIsDbl && float_equal(val, 0.0, 0.05))
		return false;
	Length l;
	if (!wIsDbl && (!isValidLength(fromqstr(wstr), &l) || l.zero()))
		return false;

	QString const hstr = heightED->text();
	if (hstr.isEmpty())
		return false;
	bool hIsDbl;
	val = hstr.trimmed().toDouble(&hIsDbl);
	if (hIsDbl && float_equal(val, 0.0, 0.05))
		return false;
	if (!hIsDbl && (!isValidLength(fromqstr(hstr), &l) || l.zero()))
		return false;

	return true;
}


bool GuiExternal::usingScale() const
{
	return (widthUnitCO->itemData(
		widthUnitCO->currentIndex()).toString() == "scale");
}


void GuiExternal::bbChanged()
{
	bbChanged_ = true;
	changed();
}


void GuiExternal::browseClicked()
{
	int const choice =  externalCO->currentIndex();
	QString const template_name = toqstr(getTemplate(choice).lyxName);
	QString const str = browse(fileED->text(), template_name);
	if (!str.isEmpty()) {
		fileED->setText(str);
		changed();
	}
}


void GuiExternal::change_adaptor()
{
	changed();
}


void GuiExternal::extraChanged(const QString & text)
{
	extra_[extraFormatCO->currentText()] = text;
	changed();
}


void GuiExternal::formatChanged(const QString & format)
{
	extraED->setText(extra_[format]);
}


void GuiExternal::getbbClicked()
{
	xlED->setText("0");
	ybED->setText("0");
	xrED->setText("0");
	ytED->setText("0");

	string const filename = fromqstr(fileED->text());
	if (filename.empty())
		return;

	FileName const abs_file(support::makeAbsPath(filename, fromqstr(bufferFilePath())));

	// try to get it from the file, if possible
	string bb = graphics::readBB_from_PSFile(abs_file);
	if (bb.empty()) {
		// we don't, so ask the Graphics Cache if it has loaded the file
		int width = 0;
		int height = 0;

		graphics::Cache & gc = graphics::Cache::get();
		if (gc.inCache(abs_file)) {
			graphics::Image const * image = gc.item(abs_file)->image();

			if (image) {
				width  = image->width();
				height = image->height();
			}
		}
		bb = "0 0 " + convert<string>(width) + ' ' + convert<string>(height);
	}

	doubleToWidget(xlED, token(bb, ' ', 0));
	doubleToWidget(ybED, token(bb, ' ', 1));
	doubleToWidget(xrED, token(bb, ' ', 2));
	doubleToWidget(ytED, token(bb, ' ', 3));
	// the values from the file always have the bigpoint-unit bp
	xlUnitCO->setCurrentIndex(0);
	ybUnitCO->setCurrentIndex(0);
	xrUnitCO->setCurrentIndex(0);
	ytUnitCO->setCurrentIndex(0);

	bbChanged_ = false;
}


void GuiExternal::sizeChanged()
{
	aspectratioCB->setEnabled(activateAspectratio());
	changed();
}


void GuiExternal::templateChanged()
{
	updateTemplate();
	changed();
}


void GuiExternal::widthUnitChanged()
{
	if (usingScale())
		widthED->setValidator(new QDoubleValidator(0, 1000, 2, widthED));
	else
		widthED->setValidator(unsignedLengthValidator(widthED));

	heightED->setEnabled(!usingScale());
	heightUnitCO->setEnabled(!usingScale());
	changed();
}


static void setRotation(QLineEdit & angleED, QComboBox & originCO,
	external::RotationData const & data)
{
	originCO.setCurrentIndex(int(data.origin()));
	doubleToWidget(&angleED, data.angle);
}


static void getRotation(external::RotationData & data,
	QLineEdit const & angleED, QComboBox const & originCO)
{
	typedef external::RotationData::OriginType OriginType;

	data.origin(static_cast<OriginType>(originCO.currentIndex()));
	data.angle = widgetToDoubleStr(&angleED);
}


static void setSize(QLineEdit & widthED, LengthCombo & widthUnitCO,
	QLineEdit & heightED, LengthCombo & heightUnitCO,
	QCheckBox & aspectratioCB,
	external::ResizeData const & data)
{
	bool using_scale = data.usingScale();
	string scale = data.scale;
	if (data.no_resize()) {
		// Everything is zero, so default to this!
		using_scale = true;
		scale = "100";
	}

	if (using_scale) {
		doubleToWidget(&widthED, scale);
		widthUnitCO.setCurrentItem("scale");
	} else
		lengthToWidgets(&widthED, &widthUnitCO,
				data.width.asString(), Length::defaultUnit());

	string const h = data.height.zero() ? string() : data.height.asString();
	Length::UNIT const default_unit = data.width.zero() ?
		Length::defaultUnit() : data.width.unit();
	lengthToWidgets(&heightED, &heightUnitCO, h, default_unit);

	heightED.setEnabled(!using_scale);
	heightUnitCO.setEnabled(!using_scale);

	aspectratioCB.setChecked(data.keepAspectRatio);

	bool const disable_aspectRatio = using_scale ||
		data.width.zero() || data.height.zero();
	aspectratioCB.setEnabled(!disable_aspectRatio);
}


static void getSize(external::ResizeData & data,
	QLineEdit const & widthED, QComboBox const & widthUnitCO,
	QLineEdit const & heightED, LengthCombo const & heightUnitCO,
	QCheckBox const & aspectratioCB, bool const scaling)
{
	if (scaling) {
		// scaling instead of a width
		data.scale = widgetToDoubleStr(&widthED);
		data.width = Length();
	} else {
		data.width = Length(widgetsToLength(&widthED, &widthUnitCO));
		data.scale = string();
	}
	data.height = Length(widgetsToLength(&heightED, &heightUnitCO));
	data.keepAspectRatio = aspectratioCB.isChecked();
}


void setCrop(QCheckBox & clipCB,
	QLineEdit & xlED, QLineEdit & ybED,
	QLineEdit & xrED, QLineEdit & ytED,
	LengthCombo & xlUnitCO, LengthCombo & ybUnitCO,
	LengthCombo & xrUnitCO, LengthCombo & ytUnitCO,
	external::ClipData const & data)
{
	clipCB.setChecked(data.clip);
	Length::UNIT const default_unit = data.bbox.xl.zero() ?
		Length::defaultUnit() : data.bbox.xl.unit();
	lengthToWidgets(&xlED, &xlUnitCO, data.bbox.xl, default_unit);
	lengthToWidgets(&ybED, &ybUnitCO, data.bbox.yb, default_unit);
	lengthToWidgets(&xrED, &xrUnitCO, data.bbox.xr, default_unit);
	lengthToWidgets(&ytED, &ytUnitCO, data.bbox.yt, default_unit);
}


static void getCrop(external::ClipData & data,
	QCheckBox const & clipCB,
	QLineEdit const & xlED, QLineEdit const & ybED,
	QLineEdit const & xrED, QLineEdit const & ytED,
	LengthCombo const & xlUnitCO, LengthCombo const & ybUnitCO,
	LengthCombo const & xrUnitCO, LengthCombo const & ytUnitCO,
	bool bb_changed)
{
	data.clip = clipCB.isChecked();

	if (!bb_changed)
		return;

	data.bbox.xl = Length(widgetsToLength(&xlED, &xlUnitCO));
	data.bbox.yb = Length(widgetsToLength(&ybED, &ybUnitCO));
	data.bbox.xr = Length(widgetsToLength(&xrED, &xrUnitCO));
	data.bbox.yt = Length(widgetsToLength(&ytED, &ytUnitCO));
}


void GuiExternal::updateContents()
{
	string const name =
		params_.filename.outputFileName(fromqstr(bufferFilePath()));
	fileED->setText(toqstr(name));

	int index = 0;
	external::TemplateManager::Templates::const_iterator i1, i2;
	i1 = external::TemplateManager::get().getTemplates().begin();
	i2 = external::TemplateManager::get().getTemplates().end();
	for (int i = 0; i1 != i2; ++i1, ++i) {
		if (i1->second.lyxName == params_.templatename()) {
			index = i;
			break;
		}
	}

	externalCO->setCurrentIndex(index);
	updateTemplate();

	draftCB->setChecked(params_.draft);

	displayGB->setChecked(params_.display);
	displayscaleED->setText(QString::number(params_.lyxscale));
	bool scaled = params_.display && !isBufferReadonly() &&
			(params_.preview_mode != PREVIEW_INSTANT);
	displayscaleED->setEnabled(scaled);
	scaleLA->setEnabled(scaled);
	displayGB->setEnabled(lyxrc.display_graphics);


	setRotation(*angleED, *originCO, params_.rotationdata);

	setSize(*widthED, *widthUnitCO, *heightED, *heightUnitCO,
		*aspectratioCB, params_.resizedata);

	setCrop(*clipCB, *xlED, *ybED, *xrED, *ytED,
	        *xlUnitCO, *ybUnitCO, *xrUnitCO, *ytUnitCO, params_.clipdata);
	bbChanged_ = !params_.clipdata.bbox.empty();

	isValid();
}


void GuiExternal::updateTemplate()
{
	external::Template templ = getTemplate(externalCO->currentIndex());
	externalTB->setPlainText(qt_(templ.helpText));

	// Ascertain which (if any) transformations the template supports
	// and disable tabs and Group Boxes hosting unsupported transforms.
	typedef vector<external::TransformID> TransformIDs;
	TransformIDs const transformIds = templ.transformIds;
	TransformIDs::const_iterator tr_begin = transformIds.begin();
	TransformIDs::const_iterator const tr_end = transformIds.end();

	bool rotate = std::find(tr_begin, tr_end, external::Rotate) != tr_end;
	rotationGB->setEnabled(rotate);

	bool resize = std::find(tr_begin, tr_end, external::Resize) != tr_end;
	scaleGB->setEnabled(resize);

	bool clip = std::find(tr_begin, tr_end, external::Clip) != tr_end;
	cropGB->setEnabled(clip);

	sizetab->setEnabled(rotate || resize || clip);
	tab->setTabEnabled(tab->indexOf(sizetab), rotate || resize || clip);

	bool found = std::find(tr_begin, tr_end, external::Extra) != tr_end;
	optionsGB->setEnabled(found);

	bool scaled = displayGB->isChecked() && displayGB->isEnabled() &&
			!isBufferReadonly() && (templ.preview_mode != PREVIEW_INSTANT);
	displayscaleED->setEnabled(scaled);
	scaleLA->setEnabled(scaled);

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
		string const opt = params_.extradata.get(format);
		extraFormatCO->addItem(toqstr(format));
		extra_[toqstr(format)] = toqstr(opt);
	}

	bool const enabled = extraFormatCO->count()  > 0;

	optionsGB->setEnabled(enabled);
	extraED->setEnabled(enabled && !isBufferReadonly());
	extraFormatCO->setEnabled(enabled);

	if (enabled) {
		extraFormatCO->setCurrentIndex(0);
		extraED->setText(extra_[extraFormatCO->currentText()]);
	}
}


void GuiExternal::applyView()
{
	params_.filename.set(fromqstr(fileED->text()), fromqstr(bufferFilePath()));
	params_.settemplate(getTemplate(externalCO->currentIndex()).lyxName);

	params_.draft = draftCB->isChecked();
	params_.lyxscale = displayscaleED->text().toInt();
	params_.display = displayGB->isChecked();

	if (rotationGB->isEnabled())
		getRotation(params_.rotationdata, *angleED, *originCO);

	if (scaleGB->isEnabled())
		getSize(params_.resizedata, *widthED, *widthUnitCO,
			*heightED, *heightUnitCO, *aspectratioCB, usingScale());

	if (cropGB->isEnabled())
		getCrop(params_.clipdata, *clipCB, *xlED, *ybED, *xrED, *ytED,
		        *xlUnitCO, *ybUnitCO, *xrUnitCO, *ytUnitCO, bbChanged_);

	if (optionsGB->isEnabled()) {
		MapType::const_iterator it = extra_.begin();
		MapType::const_iterator const end = extra_.end();
		for (; it != end; ++it)
			params_.extradata.set(fromqstr(it.key()), fromqstr(it.value().trimmed()));
	}
}


bool GuiExternal::initialiseParams(string const & data)
{
	InsetExternal::string2params(data, buffer(), params_);
	return true;
}


void GuiExternal::clearParams()
{
	params_ = InsetExternalParams();
}


void GuiExternal::dispatchParams()
{
	string const lfun = InsetExternal::params2string(params_, buffer());
	dispatch(FuncRequest(getLfun(), lfun));
}


static QStringList templateFilters(QString const & template_name)
{
	/// Determine the template file extension
	external::TemplateManager const & etm =
		external::TemplateManager::get();
	external::Template const * const et_ptr =
		etm.getTemplateByName(fromqstr(template_name));

	string filter;
	if (et_ptr && et_ptr->fileRegExp != "" && et_ptr->fileRegExp != "*") {
		filter += et_ptr->guiName;
		filter += " (";
		filter += et_ptr->fileRegExp;
		filter += ")";
	}
	return fileFilters(toqstr(filter));
}


QString GuiExternal::browse(QString const & input,
				     QString const & template_name) const
{
	QString const title = qt_("Select external file");
	QString const bufpath = bufferFilePath();
	QStringList const filter = templateFilters(template_name);

	QString const label1 = qt_("Documents|#o#O");
	QString const dir1 = toqstr(lyxrc.document_path);

	return browseRelToParent(input, bufpath, title, filter, false, label1, dir1);
}


Dialog * createGuiExternal(GuiView & lv) { return new GuiExternal(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiExternal.cpp"
