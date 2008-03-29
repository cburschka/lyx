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

#include "Buffer.h"
#include "FuncRequest.h"
#include "support/gettext.h"
#include "Length.h"
#include "LyXRC.h"

#include "insets/ExternalSupport.h"
#include "insets/ExternalTemplate.h"
#include "insets/InsetExternal.h"

#include "graphics/GraphicsCache.h"
#include "graphics/GraphicsCacheItem.h"
#include "graphics/GraphicsImage.h"

#include "frontends/alert.h"

#include "support/convert.h"
#include "support/ExceptionMessage.h"
#include "support/FileFilterList.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/os.h"

#include "LengthCombo.h"
#include "qt_helpers.h"
#include "Validator.h"

#include <QCheckBox>
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

RotationDataType origins_array[] = {
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


size_type const origins_array_size =
sizeof(origins_array) / sizeof(origins_array[0]);

vector<external::RotationDataType> const
all_origins(origins_array, origins_array + origins_array_size);

// These are the strings, corresponding to the above, that the GUI should
// use. Note that they can/should be translated.
char const * const origin_gui_strs[] = {
	N_("Default"),
	N_("Top left"), N_("Bottom left"), N_("Baseline left"),
	N_("Center"), N_("Top center"), N_("Bottom center"), N_("Baseline center"),
	N_("Top right"), N_("Bottom right"), N_("Baseline right")
};

} // namespace anon


GuiExternal::GuiExternal(GuiView & lv)
	: GuiDialog(lv, "external", qt_("External Material")), bbChanged_(false)
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	connect(displayCB, SIGNAL(toggled(bool)),
		showCO, SLOT(setEnabled(bool)));
	connect(displayCB, SIGNAL(toggled(bool)),
		displayscaleED, SLOT(setEnabled(bool)));
	connect(showCO, SIGNAL(activated(QString)),
		this, SLOT(change_adaptor()));
	connect(originCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(aspectratioCB, SIGNAL(stateChanged(int)),
		this, SLOT(change_adaptor()));
	connect(browsePB, SIGNAL(clicked()),
		this, SLOT(browseClicked()));
	connect(embedCB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(editPB, SIGNAL(clicked()),
		this, SLOT(editClicked()));
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
	connect(displayCB, SIGNAL(stateChanged(int)),
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

	vector<string> templates = getTemplates();

	for (vector<string>::const_iterator cit = templates.begin();
		cit != templates.end(); ++cit) {
		externalCO->addItem(qt_(*cit));
	}

	// Fill the origins combo
	for (size_t i = 0; i != all_origins.size(); ++i)
		originCO->addItem(qt_(origin_gui_strs[i]));

	// Fill the width combo
	widthUnitCO->addItem(qt_("Scale%"));
	for (int i = 0; i < num_units; i++)
		widthUnitCO->addItem(qt_(unit_name_gui[i]));
}


bool GuiExternal::activateAspectratio() const
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


void GuiExternal::editClicked()
{
	editExternal();
}



void GuiExternal::extraChanged(const QString& text)
{
	string const format = fromqstr(extraFormatCO->currentText());
	extra_[format] = text;
	changed();
}


void GuiExternal::formatChanged(const QString& format)
{
	extraED->setText(extra_[fromqstr(format)]);
}


void GuiExternal::getbbClicked()
{
	getBB();
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
	bool useHeight = (widthUnitCO->currentIndex() > 0);

	if (useHeight)
		widthED->setValidator(unsignedLengthValidator(widthED));
	else
		widthED->setValidator(new QDoubleValidator(0, 1000, 2, widthED));

	heightED->setEnabled(useHeight);
	heightUnitCO->setEnabled(useHeight);
	changed();
}


static Length::UNIT defaultUnit()
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


static void setDisplay(
	QCheckBox & displayCB, QComboBox & showCO, QLineEdit & scaleED,
	external::DisplayType display, unsigned int scale, bool read_only)
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
	scaleED.setText(QString::number(scale));
}


static void getDisplay(external::DisplayType & display,
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

	scale = scaleED.text().toInt();
}


static void setRotation(QLineEdit & angleED, QComboBox & originCO,
	external::RotationData const & data)
{
	originCO.setCurrentIndex(int(data.origin()));
	angleED.setText(toqstr(data.angle));
}


static void getRotation(external::RotationData & data,
	QLineEdit const & angleED, QComboBox const & originCO)
{
	typedef external::RotationData::OriginType OriginType;

	data.origin(static_cast<OriginType>(originCO.currentIndex()));
	data.angle = fromqstr(angleED.text());
}


static void setSize(QLineEdit & widthED, QComboBox & widthUnitCO,
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
		widthED.setText(toqstr(scale));
		widthUnitCO.setCurrentIndex(0);
	} else {
		widthED.setText(QString::number(data.width.value()));
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


static void getSize(external::ResizeData & data,
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
	xlED.setText(QString::number(bbox.xl));
	ybED.setText(QString::number(bbox.yb));
	xrED.setText(QString::number(bbox.xr));
	ytED.setText(QString::number(bbox.yt));
}


static void getCrop(external::ClipData & data,
	QCheckBox const & clipCB,
	QLineEdit const & xlED, QLineEdit const & ybED,
	QLineEdit const & xrED, QLineEdit const & ytED,
	bool bb_changed)
{
	data.clip = clipCB.isChecked();

	if (!bb_changed)
		return;

	data.bbox.xl = xlED.text().toInt();
	data.bbox.yb = ybED.text().toInt();
	data.bbox.xr = xrED.text().toInt();
	data.bbox.yt = ytED.text().toInt();
}


static void getExtra(external::ExtraData & data,
	      GuiExternal::MapType const & extra)
{
	typedef GuiExternal::MapType MapType;
	MapType::const_iterator it = extra.begin();
	MapType::const_iterator const end = extra.end();
	for (; it != end; ++it)
		data.set(it->first, trim(fromqstr(it->second)));
}


void GuiExternal::updateContents()
{
	tab->setCurrentIndex(0);

	string const name =
		params_.filename.outputFilename(fromqstr(bufferFilepath()));
	fileED->setText(toqstr(name));
	embedCB->setCheckState(params_.filename.embedded() ? Qt::Checked : Qt::Unchecked);

	externalCO->setCurrentIndex(getTemplateNumber(params_.templatename()));
	updateTemplate();

	draftCB->setChecked(params_.draft);

	setDisplay(*displayCB, *showCO, *displayscaleED,
		   params_.display, params_.lyxscale, isBufferReadonly());

	setRotation(*angleED, *originCO, params_.rotationdata);

	setSize(*widthED, *widthUnitCO, *heightED, *heightUnitCO,
		*aspectratioCB, params_.resizedata);

	setCrop(*clipCB, *xlED, *ybED, *xrED, *ytED, params_.clipdata);
	bbChanged_ = !params_.clipdata.bbox.empty();

	isValid();
}


void GuiExternal::updateTemplate()
{
	external::Template templ = getTemplate(externalCO->currentIndex());
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
		string const opt = params_.extradata.get(format);
		extraFormatCO->addItem(toqstr(format));
		extra_[format] = toqstr(opt);
	}

	bool const enabled = extraFormatCO->count()  > 0;

	tab->setTabEnabled(
		tab->indexOf(optionstab), enabled);
	extraED->setEnabled(enabled && !isBufferReadonly());
	extraFormatCO->setEnabled(enabled);

	if (enabled) {
		extraFormatCO->setCurrentIndex(0);
		extraED->setText(extra_[fromqstr(extraFormatCO->currentText())]);
	}
}


void GuiExternal::applyView()
{
	params_.filename.set(fromqstr(fileED->text()), fromqstr(bufferFilepath()));

	try {
		Buffer & buf = buffer();
		EmbeddedFile file(fromqstr(fileED->text()), buf.filePath());
		file.setEmbed(embedCB->checkState() == Qt::Checked);
		// move file around if needed, an exception may be raised.
		file.enable(buf.embedded(), buf, true);
		// if things are OK..., embed params_.filename
		params_.filename.setEmbed(file.embedded());
	} catch (ExceptionMessage const & message) {
		Alert::error(message.title_, message.details_);
		// failed to embed
		params_.filename.setEmbed(false);
	}
	
	params_.settemplate(getTemplate(externalCO->currentIndex()).lyxName);

	params_.draft = draftCB->isChecked();

	getDisplay(params_.display, params_.lyxscale,
		   *displayCB, *showCO,
		   *displayscaleED);

	if (tab->isTabEnabled(tab->indexOf(rotatetab)))
		getRotation(params_.rotationdata, *angleED, *originCO);

	if (tab->isTabEnabled(tab->indexOf(scaletab)))
		getSize(params_.resizedata, *widthED, *widthUnitCO,
			*heightED, *heightUnitCO, *aspectratioCB);

	if (tab->isTabEnabled(tab->indexOf(croptab)))
		getCrop(params_.clipdata, *clipCB, *xlED, *ybED,
			*xrED, *ytED, bbChanged_);

	if (tab->isTabEnabled(tab->indexOf(optionstab)))
		getExtra(params_.extradata, extra_);
}


void GuiExternal::getBB()
{
	xlED->setText("0");
	ybED->setText("0");
	xrED->setText("0");
	ytED->setText("0");

	string const filename = fromqstr(fileED->text());
	if (filename.empty())
		return;

	string const bb = readBB(filename);
	if (bb.empty())
		return;

	xlED->setText(toqstr(token(bb, ' ', 0)));
	ybED->setText(toqstr(token(bb, ' ', 1)));
	xrED->setText(toqstr(token(bb, ' ', 2)));
	ytED->setText(toqstr(token(bb, ' ', 3)));

	bbChanged_ = false;
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


void GuiExternal::editExternal()
{
	applyView();
	string const lfun = InsetExternal::params2string(params_, buffer());
	dispatch(FuncRequest(LFUN_EXTERNAL_EDIT, lfun));
}


vector<string> const GuiExternal::getTemplates() const
{
	vector<string> result;

	external::TemplateManager::Templates::const_iterator i1, i2;
	i1 = external::TemplateManager::get().getTemplates().begin();
	i2 = external::TemplateManager::get().getTemplates().end();

	for (; i1 != i2; ++i1) {
		result.push_back(i1->second.lyxName);
	}
	return result;
}


int GuiExternal::getTemplateNumber(string const & name) const
{
	external::TemplateManager::Templates::const_iterator i1, i2;
	i1 = external::TemplateManager::get().getTemplates().begin();
	i2 = external::TemplateManager::get().getTemplates().end();
	for (int i = 0; i1 != i2; ++i1, ++i) {
		if (i1->second.lyxName == name)
			return i;
	}

	// we can get here if a LyX document has a template not installed
	// on this machine.
	return -1;
}


external::Template GuiExternal::getTemplate(int i) const
{
	external::TemplateManager::Templates::const_iterator i1
		= external::TemplateManager::get().getTemplates().begin();

	advance(i1, i);

	return i1->second;
}


string const
GuiExternal::getTemplateFilters(string const & template_name) const
{
	/// Determine the template file extension
	external::TemplateManager const & etm =
		external::TemplateManager::get();
	external::Template const * const et_ptr =
		etm.getTemplateByName(template_name);

	if (et_ptr)
		return et_ptr->fileRegExp;

	return string();
}


QString GuiExternal::browse(QString const & input,
				     QString const & template_name) const
{
	QString const title = qt_("Select external file");
	QString const bufpath = bufferFilepath();
	FileFilterList const filter =
		FileFilterList(from_utf8(getTemplateFilters(fromqstr(template_name))));

	QString const label1 = qt_("Documents|#o#O");
	QString const dir1 = toqstr(lyxrc.document_path);

	return browseRelFile(input, bufpath, title, filter, false, label1, dir1);
}


string const GuiExternal::readBB(string const & file)
{
	FileName const abs_file(makeAbsPath(file, fromqstr(bufferFilepath())));

	// try to get it from the file, if possible. Zipped files are
	// unzipped in the readBB_from_PSFile-Function
	string const bb = readBB_from_PSFile(abs_file);
	if (!bb.empty())
		return bb;

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

	return ("0 0 " + convert<string>(width) + ' ' + convert<string>(height));
}


Dialog * createGuiExternal(GuiView & lv) { return new GuiExternal(lv); }


} // namespace frontend
} // namespace lyx

#include "GuiExternal_moc.cpp"
