/**
 * \file QExternal.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

// Qt defines a macro 'signals' that clashes with a boost namespace.
// All is well if the namespace is visible first.
#include "lengthcommon.h"
#include "LyXRC.h"

#include "controllers/ControlExternal.h"
#include "controllers/ButtonController.h"

#include "insets/ExternalTemplate.h"
#include "insets/InsetExternal.h"

#include "support/lstrings.h"
#include "support/convert.h"
#include "support/os.h"
#include "support/lyxlib.h"

#include "QExternal.h"
#include "Qt2BC.h"

#include "CheckedLineEdit.h"
#include "LengthCombo.h"
#include "qt_helpers.h"
#include "Validator.h"

#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
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
using std::find;


namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////////
//
// QExternalDialog
//
/////////////////////////////////////////////////////////////////////


QExternalDialog::QExternalDialog(QExternal * form)
	: form_(form)
{
	setupUi(this);
	connect(okPB, SIGNAL(clicked()), form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), form, SLOT(slotClose()));

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

	fileED->setValidator(new PathValidator(true, fileED));
	setFocusProxy(fileED);
}


void QExternalDialog::show()
{
	QDialog::show();
}



bool QExternalDialog::activateAspectratio() const
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


void QExternalDialog::bbChanged()
{
	form_->controller().bbChanged(true);
	form_->changed();
}


void QExternalDialog::browseClicked()
{
	int const choice =  externalCO->currentIndex();
	docstring const template_name =
		from_utf8(form_->controller().getTemplate(choice).lyxName);
	docstring const str =
		form_->controller().browse(qstring_to_ucs4(fileED->text()),
					   template_name);
	if(!str.empty()) {
		fileED->setText(toqstr(str));
		form_->changed();
	}
}


void QExternalDialog::change_adaptor()
{
	form_->changed();
}


void QExternalDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QExternalDialog::editClicked()
{
	form_->controller().editExternal();
}



void QExternalDialog::extraChanged(const QString& text)
{
	std::string const format = fromqstr(extraFormatCO->currentText());
	form_->extra_[format] = text;
	form_->changed();
}


void QExternalDialog::formatChanged(const QString& format)
{
	extraED->setText(form_->extra_[fromqstr(format)]);
}


void QExternalDialog::getbbClicked()
{
	form_->getBB();
}


void QExternalDialog::sizeChanged()
{
	aspectratioCB->setEnabled(activateAspectratio());
	form_->changed();
}


void QExternalDialog::templateChanged()
{
	form_->updateTemplate();
	form_->changed();
}


void QExternalDialog::widthUnitChanged()
{
	bool useHeight = (widthUnitCO->currentIndex() > 0);

	if (useHeight)
		widthED->setValidator(unsignedLengthValidator(widthED));
	else
		widthED->setValidator(new QDoubleValidator(0, 1000, 2, widthED));

	heightED->setEnabled(useHeight);
	heightUnitCO->setEnabled(useHeight);
	form_->changed();
}


/////////////////////////////////////////////////////////////////////
//
// QExternal
//
/////////////////////////////////////////////////////////////////////

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
	      QExternal::MapType const & extra)
{
	typedef QExternal::MapType MapType;
	MapType::const_iterator it  = extra.begin();
	MapType::const_iterator const end = extra.end();
	for (; it != end; ++it)
		data.set(it->first, trim(fromqstr(it->second)));
}

} // namespace anon


typedef QController<ControlExternal, QView<QExternalDialog> >
	ExternalBase;

QExternal::QExternal(Dialog & parent)
	: ExternalBase(parent, _("External Material"))
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
	bcview().addReadOnly(dialog_->draftCB);
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
	addCheckedLineEdit(bcview(), dialog_->fileED, dialog_->fileLA);

	std::vector<string> templates(controller().getTemplates());

	for (std::vector<string>::const_iterator cit = templates.begin();
		cit != templates.end(); ++cit) {
		dialog_->externalCO->addItem(qt_(*cit));
	}

	// Fill the origins combo
	typedef vector<external::RotationDataType> Origins;
	Origins const & all_origins = external::all_origins();
	for (Origins::size_type i = 0; i != all_origins.size(); ++i)
		dialog_->originCO->addItem(toqstr(external::origin_gui_str(i)));

	// Fill the width combo
	dialog_->widthUnitCO->addItem(qt_("Scale%"));
	for (int i = 0; i < num_units; i++)
		dialog_->widthUnitCO->addItem(qt_(unit_name_gui[i]));
}


void QExternal::update_contents()
{
	PathValidator * path_validator = getPathValidator(dialog_->fileED);
	if (path_validator)
		path_validator->setChecker(kernel().docType(), lyxrc);

	dialog_->tab->setCurrentIndex(0);
	InsetExternalParams const & params = controller().params();

	string const name =
		params.filename.outputFilename(kernel().bufferFilepath());
	dialog_->fileED->setText(toqstr(name));

	dialog_->externalCO->setCurrentIndex(
		controller().getTemplateNumber(params.templatename()));
	updateTemplate();

	dialog_->draftCB->setChecked(params.draft);

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
		controller().getTemplate(dialog_->externalCO->currentIndex());
	dialog_->externalTB->setPlainText(qt_(templ.helpText));

	// Ascertain which (if any) transformations the template supports
	// and disable tabs hosting unsupported transforms.
	typedef vector<external::TransformID> TransformIDs;
	TransformIDs const transformIds = templ.transformIds;
	TransformIDs::const_iterator tr_begin = transformIds.begin();
	TransformIDs::const_iterator const tr_end = transformIds.end();

	bool found = find(tr_begin, tr_end, external::Rotate) != tr_end;
	dialog_->tab->setTabEnabled(
		dialog_->tab->indexOf(dialog_->rotatetab), found);
	found = find(tr_begin, tr_end, external::Resize) != tr_end;
	dialog_->tab->setTabEnabled(
		dialog_->tab->indexOf(dialog_->scaletab), found);

	found = find(tr_begin, tr_end, external::Clip) != tr_end;
	dialog_->tab->setTabEnabled(
		dialog_->tab->indexOf(dialog_->croptab), found);

	found = find(tr_begin, tr_end, external::Extra) != tr_end;
	dialog_->tab->setTabEnabled(
		dialog_->tab->indexOf(dialog_->optionstab), found);

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
		extraCB->addItem(toqstr(format));
		extra_[format] = toqstr(opt);
	}

	bool const enabled = extraCB->count()  > 0;

	dialog_->tab->setTabEnabled(
		dialog_->tab->indexOf(dialog_->optionstab), enabled);
	extraED->setEnabled(enabled && !kernel().isBufferReadonly());
	extraCB->setEnabled(enabled);

	if (enabled) {
		extraCB->setCurrentIndex(0);
		extraED->setText(extra_[fromqstr(extraCB->currentText())]);
	}
}


void QExternal::apply()
{
	InsetExternalParams params = controller().params();

	params.filename.set(internal_path(fromqstr(dialog_->fileED->text())),
			    kernel().bufferFilepath());

	params.settemplate(controller().getTemplate(
				   dialog_->externalCO->currentIndex()).lyxName);

	params.draft = dialog_->draftCB->isChecked();

	getDisplay(params.display, params.lyxscale,
		   *dialog_->displayCB, *dialog_->showCO,
		   *dialog_->displayscaleED);

	if (dialog_->tab->isTabEnabled(
		dialog_->tab->indexOf(dialog_->rotatetab)))
		getRotation(params.rotationdata,
			    *dialog_->angleED, *dialog_->originCO);

	if (dialog_->tab->isTabEnabled(
		dialog_->tab->indexOf(dialog_->scaletab)))
		getSize(params.resizedata,
			*dialog_->widthED, *dialog_->widthUnitCO,
			*dialog_->heightED, *dialog_->heightUnitCO,
			*dialog_->aspectratioCB);

	if (dialog_->tab->isTabEnabled(
		dialog_->tab->indexOf(dialog_->croptab)))
		getCrop(params.clipdata,
			*dialog_->clipCB,
			*dialog_->xlED, *dialog_->ybED,
			*dialog_->xrED, *dialog_->ytED,
			controller().bbChanged());

	if (dialog_->tab->isTabEnabled(
		dialog_->tab->indexOf(dialog_->optionstab)))
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

} // namespace frontend
} // namespace lyx

#include "QExternal_moc.cpp"
