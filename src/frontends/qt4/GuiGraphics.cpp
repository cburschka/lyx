/**
 * \file GuiGraphics.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 * \author Herbert Voﬂ
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiGraphics.h"

#include "ControlGraphics.h"
#include "debug.h"
#include "LengthCombo.h"
#include "lengthcommon.h"
#include "LyXRC.h"
#include "qt_helpers.h"
#include "Validator.h"

#include "insets/InsetGraphicsParams.h"

#include "support/convert.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/os.h"

#include <boost/bind.hpp>

#include <QCheckBox>
#include <QCloseEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QValidator>

#include <algorithm>
#include <cmath>
#include <utility>

using lyx::support::float_equal;
using lyx::support::token;

using lyx::support::os::internal_path;

using std::find;

#ifndef CXX_GLOBAL_CSTD
using std::floor;
#endif

using std::vector;
using std::string;
using std::transform;

namespace lyx {
namespace frontend {


//FIXME setAutoTextCB should really take an argument, as indicated, that
//determines what text is to be written for "auto". But making
//that work involves more extensive revisions than we now want
//to make, since "auto" also appears in updateContents() (see
//GuiGraphics.cpp).
//The right way to do this, I think, would be to define a class
//checkedLengthSet (and a partnering labeledLengthSete) that encapsulated
//the checkbox, line edit, and length combo together, and then made e.g.
//lengthToWidgets, widgetsToLength, etc, all public methods of that class.
//Perhaps even the validator could be exposed through it.
/**
 * sets a checkbox-line edit-length combo group, using "text" if the
 * checkbox is unchecked and clearing the line edit if it previously
 * said "text".
*/
void setAutoTextCB(QCheckBox * checkBox, QLineEdit * lineEdit,
	LengthCombo * lengthCombo/*, string text = "auto"*/)
{
	if (!checkBox->isChecked())
		lengthToWidgets(lineEdit, lengthCombo,
				"auto", lengthCombo->currentLengthItem());
	else if (lineEdit->text() == "auto")
		lengthToWidgets(lineEdit, lengthCombo, string(),
				lengthCombo->currentLengthItem());
}



template<class Pair>
vector<typename Pair::first_type> const
getFirst(vector<Pair> const & pr)
{
	vector<typename Pair::first_type> tmp(pr.size());
	transform(pr.begin(), pr.end(), tmp.begin(),
		  boost::bind(&Pair::first, _1));
	return tmp;
}

///
template<class Pair>
vector<typename Pair::second_type> const
getSecond(vector<Pair> const & pr)
{
	 vector<typename Pair::second_type> tmp(pr.size());
	 transform(pr.begin(), pr.end(), tmp.begin(),
		   boost::bind(&Pair::second, _1));
	 return tmp;
}

GuiGraphicsDialog::GuiGraphicsDialog(LyXView & lv)
	: GuiDialog(lv, "graphics")
{
	setupUi(this);
	setViewTitle(_("Graphics"));
	setController(new ControlGraphics(*this));

	//main buttons
	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(restorePB, SIGNAL(clicked()), this, SLOT(slotRestore()));

	//graphics pane
	connect(filename, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(WidthCB, SIGNAL( clicked()),
		this, SLOT(change_adaptor()));
	connect(HeightCB, SIGNAL( clicked()),
		this, SLOT(change_adaptor()));
	connect(Width, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(Height, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(heightUnit, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(change_adaptor()));
	connect(widthUnit, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(change_adaptor()));
	connect(aspectratio, SIGNAL(stateChanged(int)),
		this, SLOT(change_adaptor()));
	connect(angle, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(origin, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(scaleCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(Scale, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(rotateOrderCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));

	filename->setValidator(new PathValidator(true, filename));
	setFocusProxy(filename);

	QDoubleValidator * scaleValidator = new DoubleAutoValidator(Scale);
	scaleValidator->setBottom(0);
	scaleValidator->setDecimals(256); //I guess that will do
	Scale->setValidator(scaleValidator);
	Height->setValidator(unsignedLengthAutoValidator(Height));
	Width->setValidator(unsignedLengthAutoValidator(Width));
	angle->setValidator(new QDoubleValidator(-360, 360, 2, angle));

	//clipping pane
	connect(clip, SIGNAL(stateChanged(int)),
		this, SLOT(change_adaptor()));
	connect(lbY, SIGNAL(textChanged(const QString&)),
		this, SLOT(change_bb()));
	connect(lbYunit, SIGNAL(activated(int)),
		this, SLOT(change_bb()));
	connect(rtY, SIGNAL(textChanged(const QString&)),
		this, SLOT(change_bb()));
	connect(rtYunit, SIGNAL(activated(int)),
		this, SLOT(change_bb()));
	connect(lbX, SIGNAL(textChanged(const QString&)),
		this, SLOT(change_bb()));
	connect(lbXunit, SIGNAL(activated(int)),
		this, SLOT(change_bb()));
	connect(rtX, SIGNAL(textChanged(const QString&)),
		this, SLOT(change_bb()));
	connect(rtXunit, SIGNAL(activated(int)),
		this, SLOT(change_bb()));
	connect(getPB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));

	lbX->setValidator(new QDoubleValidator(lbX));
	lbY->setValidator(new QDoubleValidator(lbY));
	rtX->setValidator(new QDoubleValidator(rtX));
	rtY->setValidator(new QDoubleValidator(rtY));

	//extra options pane
	connect(latexoptions, SIGNAL(textChanged(const QString&)),
		this, SLOT(change_adaptor()));
	connect(draftCB, SIGNAL(stateChanged(int)),
		this, SLOT(change_adaptor()));
	connect(unzipCB, SIGNAL(stateChanged(int)),
		this, SLOT(change_adaptor()));
	// FIXME: we should connect to clicked() when we move to Qt 4.2	because
	// the toggled(bool) signal is also trigged when we update the widgets
	// (rgh-4/07) this isn't as much or a problem as it was, because we're now
	// using blockSignals() to keep from triggering that signal when we call
	// setChecked(). Note, too, that clicked() would get called whenever it
	// is clicked, even right clicked (I think), not just whenever it is
	// toggled.
	connect(subfigure, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(subcaption, SIGNAL(textChanged(const QString&)),
		this, SLOT(change_adaptor()));
	connect(displayGB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(showCB, SIGNAL(currentIndexChanged(int)),
		this, SLOT(change_adaptor()));
	connect(displayscale, SIGNAL(textChanged(const QString&)),
		this, SLOT(change_adaptor()));
	displayscale->setValidator(new QIntValidator(displayscale));

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setApply(applyPB);
	bc().setRestore(restorePB);
	bc().setCancel(closePB);

	bc().addReadOnly(latexoptions);
	bc().addReadOnly(subfigure);
	bc().addReadOnly(filenameL);
	bc().addReadOnly(filename);
	bc().addReadOnly(browsePB);
	bc().addReadOnly(unzipCB);
	bc().addReadOnly(bbFrame);
	bc().addReadOnly(draftCB);
	bc().addReadOnly(clip);
	bc().addReadOnly(unzipCB);
	bc().addReadOnly(displayGB);
	bc().addReadOnly(sizeGB);
	bc().addReadOnly(rotationGB);
	bc().addReadOnly(latexoptions);
	bc().addReadOnly(getPB);
	bc().addReadOnly(rotateOrderCB);

	// initialize the length validator
	bc().addCheckedLineEdit(Scale, scaleCB);
	bc().addCheckedLineEdit(Width, WidthCB);
	bc().addCheckedLineEdit(Height, HeightCB);
	bc().addCheckedLineEdit(displayscale, scaleLA);
	bc().addCheckedLineEdit(angle, angleL);
	bc().addCheckedLineEdit(lbX, xL);
	bc().addCheckedLineEdit(lbY, yL);
	bc().addCheckedLineEdit(rtX, xL_2);
	bc().addCheckedLineEdit(rtY, yL_2);
	bc().addCheckedLineEdit(filename, filenameL);
}


ControlGraphics & GuiGraphicsDialog::controller()
{
	return static_cast<ControlGraphics &>(GuiDialog::controller());
}


void GuiGraphicsDialog::change_adaptor()
{
	changed();
}


void GuiGraphicsDialog::change_bb()
{
	controller().bbChanged = true;
	LYXERR(Debug::GRAPHICS)
		<< "[controller().bb_Changed set to true]\n";
	changed();
}


void GuiGraphicsDialog::closeEvent(QCloseEvent * e)
{
	slotClose();
	GuiDialog::closeEvent(e);
}


void GuiGraphicsDialog::on_browsePB_clicked()
{
	docstring const str =
		controller().browse(qstring_to_ucs4(filename->text()));
	if (!str.empty()) {
		filename->setText(toqstr(str));
		embedCB->setCheckState(Qt::Unchecked);
		changed();
	}
}


void GuiGraphicsDialog::on_getPB_clicked()
{
	getBB();
}


void GuiGraphicsDialog::on_editPB_clicked()
{
	controller().editGraphics();
}


void GuiGraphicsDialog::on_filename_textChanged(const QString & filename)
{
	editPB->setDisabled(filename.isEmpty());
}


void GuiGraphicsDialog::setAutoText()
{
	if (scaleCB->isChecked())
		return;
	if (!Scale->isEnabled() && Scale->text() != "100")
		Scale->setText(QString("auto"));

	setAutoTextCB(WidthCB, Width, widthUnit);
	setAutoTextCB(HeightCB, Height, heightUnit);
}


void GuiGraphicsDialog::on_scaleCB_toggled(bool setScale)
{
	Scale->setEnabled(setScale);
	if (setScale) {
		Scale->setText("100");
		Scale->setFocus(Qt::OtherFocusReason);
	}

	WidthCB->setDisabled(setScale);
	WidthCB->blockSignals(true);
	WidthCB->setChecked(false);
	WidthCB->blockSignals(false);
	Width->setEnabled(false);
	widthUnit->setEnabled(false);

	HeightCB->setDisabled(setScale);
	HeightCB->blockSignals(true);
	HeightCB->setChecked(false);
	HeightCB->blockSignals(false);
	Height->setEnabled(false);
	heightUnit->setEnabled(false);

	aspectratio->setDisabled(true);
	aspectratio->setChecked(true);

	rotateOrderCB->setEnabled((WidthCB->isChecked() ||
				 HeightCB->isChecked() ||
				 scaleCB->isChecked()) &&
				 (angle->text() != "0"));

	setAutoText();
}


void GuiGraphicsDialog::on_WidthCB_toggled(bool setWidth)
{
	Width->setEnabled(setWidth);
	widthUnit->setEnabled(setWidth);
	if (setWidth)
		Width->setFocus(Qt::OtherFocusReason);

	bool const setHeight = HeightCB->isChecked();
	aspectratio->setEnabled(setWidth && setHeight);
	aspectratio->blockSignals(true);
	aspectratio->setChecked(!(setWidth && setHeight));
	aspectratio->blockSignals(false);

	scaleCB->setEnabled(!setWidth && !setHeight);
	//already will be unchecked, so don't need to do that
	Scale->setEnabled((!setWidth && !setHeight) //=scaleCB->isEnabled()
			&& scaleCB->isChecked()); //should be false, but let's check
	rotateOrderCB->setEnabled((setWidth || setHeight ||
				 scaleCB->isChecked()) &&
				 (angle->text() != "0"));

	setAutoText();
}


void GuiGraphicsDialog::on_HeightCB_toggled(bool setHeight)
{
	Height->setEnabled(setHeight);
	heightUnit->setEnabled(setHeight);
	if (setHeight)
		Height->setFocus(Qt::OtherFocusReason);

	bool const setWidth = WidthCB->isChecked();
	aspectratio->setEnabled(setWidth && setHeight);
	aspectratio->blockSignals(true);
	aspectratio->setChecked(!(setWidth && setHeight));
	aspectratio->blockSignals(false);

	scaleCB->setEnabled(!setWidth && !setHeight);
	//already unchecked
	Scale->setEnabled((!setWidth && !setHeight) //=scaleCB->isEnabled()
		&& scaleCB->isChecked()); //should be false
	rotateOrderCB->setEnabled((setWidth || setHeight ||
				 scaleCB->isChecked()) &&
				 (angle->text() != "0"));

	setAutoText();
}


void GuiGraphicsDialog::on_angle_textChanged(const QString & filename)
{
	rotateOrderCB->setEnabled((WidthCB->isChecked() ||
				 HeightCB->isChecked() ||
				 scaleCB->isChecked()) &&
				 (filename != "0"));
}

// returns the number of the string s in the vector v
static int getItemNo(const vector<string> & v, string const & s)
{
	vector<string>::const_iterator cit =
		    find(v.begin(), v.end(), s);
	return (cit != v.end()) ? int(cit - v.begin()) : 0;
}


void GuiGraphicsDialog::updateContents()
{
	// clear and fill in the comboboxes
	vector<string> const bb_units = frontend::getBBUnits();
	lbXunit->clear();
	lbYunit->clear();
	rtXunit->clear();
	rtYunit->clear();
	for (vector<string>::const_iterator it = bb_units.begin();
	    it != bb_units.end(); ++it) {
		lbXunit->addItem(toqstr(*it));
		lbYunit->addItem(toqstr(*it));
		rtXunit->addItem(toqstr(*it));
		rtYunit->addItem(toqstr(*it));
	}

	InsetGraphicsParams & igp = controller().params();

	// set the right default unit
	Length::UNIT unitDefault = Length::CM;
	switch (lyxrc.default_papersize) {
		case PAPER_USLETTER:
		case PAPER_USLEGAL:
		case PAPER_USEXECUTIVE:
			unitDefault = Length::IN;
			break;
		default:
			break;
	}

	string const name =
		igp.filename.outputFilename(controller().bufferFilepath());
	filename->setText(toqstr(name));
    embedCB->setCheckState(igp.filename.embedded() ? Qt::Checked : Qt::Unchecked);

	// set the bounding box values
	if (igp.bb.empty()) {
		string const bb = controller().readBB(igp.filename.absFilename());
		// the values from the file always have the bigpoint-unit bp
		lbX->setText(toqstr(token(bb, ' ', 0)));
		lbY->setText(toqstr(token(bb, ' ', 1)));
		rtX->setText(toqstr(token(bb, ' ', 2)));
		rtY->setText(toqstr(token(bb, ' ', 3)));
		lbXunit->setCurrentIndex(0);
		lbYunit->setCurrentIndex(0);
		rtXunit->setCurrentIndex(0);
		rtYunit->setCurrentIndex(0);
		controller().bbChanged = false;
	} else {
		// get the values from the inset
		Length anyLength;
		string const xl(token(igp.bb, ' ', 0));
		string const yl(token(igp.bb, ' ', 1));
		string const xr(token(igp.bb, ' ', 2));
		string const yr(token(igp.bb, ' ', 3));
		if (isValidLength(xl, &anyLength)) {
			lbX->setText(toqstr(convert<string>(anyLength.value())));
			string const unit(unit_name[anyLength.unit()]);
			lbXunit->setCurrentIndex(getItemNo(bb_units, unit));
		} else {
			lbX->setText(toqstr(xl));
		}
		if (isValidLength(yl, &anyLength)) {
			lbY->setText(toqstr(convert<string>(anyLength.value())));
			string const unit(unit_name[anyLength.unit()]);
			lbYunit->setCurrentIndex(getItemNo(bb_units, unit));
		} else {
			lbY->setText(toqstr(xl));
		}
		if (isValidLength(xr, &anyLength)) {
			rtX->setText(toqstr(convert<string>(anyLength.value())));
			string const unit(unit_name[anyLength.unit()]);
			rtXunit->setCurrentIndex(getItemNo(bb_units, unit));
		} else {
			rtX->setText(toqstr(xl));
		}
		if (isValidLength(yr, &anyLength)) {
			rtY->setText(toqstr(convert<string>(anyLength.value())));
			string const unit(unit_name[anyLength.unit()]);
			rtYunit->setCurrentIndex(getItemNo(bb_units, unit));
		} else {
			rtY->setText(toqstr(xl));
		}
		controller().bbChanged = true;
	}

	// Update the draft and clip mode
	draftCB->setChecked(igp.draft);
	clip->setChecked(igp.clip);
	unzipCB->setChecked(igp.noUnzip);

	// Update the subcaption check button and input field
	subfigure->setChecked(igp.subcaption);
	subcaption->setText(toqstr(igp.subcaptionText));

	int item = 0;
	switch (igp.display) {
		case graphics::DefaultDisplay: item = 0; break;
		case graphics::MonochromeDisplay: item = 1; break;
		case graphics::GrayscaleDisplay: item = 2; break;
		case graphics::ColorDisplay: item = 3; break;
		case graphics::NoDisplay: item = 0; break;
	}
	showCB->setCurrentIndex(item);
	displayscale->setText(toqstr(convert<string>(igp.lyxscale)));
	displayGB->setChecked(igp.display != graphics::NoDisplay);

	// the output section (width/height)

	Scale->setText(toqstr(igp.scale));
	//igp.scale defaults to 100, so we treat it as empty
	bool const scaleChecked = !igp.scale.empty() && igp.scale != "100";
	scaleCB->blockSignals(true);
	scaleCB->setChecked(scaleChecked);
	scaleCB->blockSignals(false);
	Scale->setEnabled(scaleChecked);

	lengthAutoToWidgets(Width, widthUnit, igp.width,
		unitDefault);
	bool const widthChecked = !Width->text().isEmpty() &&
		Width->text() != "auto";
	WidthCB->blockSignals(true);
	WidthCB->setChecked(widthChecked);
	WidthCB->blockSignals(false);
	Width->setEnabled(widthChecked);
	widthUnit->setEnabled(widthChecked);

	lengthAutoToWidgets(Height, heightUnit, igp.height,
		unitDefault);
	bool const heightChecked = !Height->text().isEmpty()
		&& Height->text() != "auto";
	HeightCB->blockSignals(true);
	HeightCB->setChecked(heightChecked);
	HeightCB->blockSignals(false);
	Height->setEnabled(heightChecked);
	heightUnit->setEnabled(heightChecked);

	scaleCB->setEnabled(!widthChecked && !heightChecked);
	WidthCB->setEnabled(!scaleChecked);
	HeightCB->setEnabled(!scaleChecked);
	aspectratio->setEnabled(widthChecked && heightChecked);

	setAutoText();

	angle->setText(toqstr(igp.rotateAngle));
	rotateOrderCB->setChecked(igp.scaleBeforeRotation);

	rotateOrderCB->setEnabled( (widthChecked || heightChecked || scaleChecked)
		&& igp.rotateAngle != "0");

	origin->clear();

	vector<RotationOriginPair> origindata = getRotationOriginData();
	vector<docstring> const origin_lang = getFirst(origindata);
	origin_ltx = getSecond(origindata);

	for (vector<docstring>::const_iterator it = origin_lang.begin();
	    it != origin_lang.end(); ++it)
		origin->addItem(toqstr(*it));

	if (!igp.rotateOrigin.empty())
		origin->setCurrentIndex(
			getItemNo(origin_ltx, igp.rotateOrigin));
	else
		origin->setCurrentIndex(0);

	// disable edit button when no filename is present
	editPB->setDisabled(filename->text().isEmpty());

	//// latex section
	latexoptions->setText(toqstr(igp.special));
}


void GuiGraphicsDialog::applyView()
{
	InsetGraphicsParams & igp = controller().params();

	igp.filename.set(internal_path(fromqstr(filename->text())),
			 controller().bufferFilepath());
	igp.filename.setEmbed(embedCB->checkState() == Qt::Checked);

	// the bb section
	igp.bb.erase();
	if (controller().bbChanged) {
		string bb;
		string lbXs = fromqstr(lbX->text());
		string lbYs = fromqstr(lbY->text());
		string rtXs = fromqstr(rtX->text());
		string rtYs = fromqstr(rtY->text());
		int bb_sum =
			convert<int>(lbXs) + convert<int>(lbYs) +
			convert<int>(rtXs) + convert<int>(rtXs);
		if (bb_sum) {
			if (lbXs.empty())
				bb = "0 ";
			else
				bb = lbXs + fromqstr(lbXunit->currentText()) + ' ';
			if (lbYs.empty())
				bb += "0 ";
			else
				bb += (lbYs + fromqstr(lbYunit->currentText()) + ' ');
			if (rtXs.empty())
				bb += "0 ";
			else
				bb += (rtXs + fromqstr(rtXunit->currentText()) + ' ');
			if (rtYs.empty())
				bb += '0';
			else
				bb += (rtYs + fromqstr(rtYunit->currentText()));
			igp.bb = bb;
		}
	}

	igp.draft = draftCB->isChecked();
	igp.clip = clip->isChecked();
	igp.subcaption = subfigure->isChecked();
	igp.subcaptionText = fromqstr(subcaption->text());

	switch (showCB->currentIndex()) {
		case 0: igp.display = graphics::DefaultDisplay; break;
		case 1: igp.display = graphics::MonochromeDisplay; break;
		case 2: igp.display = graphics::GrayscaleDisplay; break;
		case 3: igp.display = graphics::ColorDisplay; break;
		default:;
	}

	if (!displayGB->isChecked())
		igp.display = graphics::NoDisplay;

	//the graphics section
	if (scaleCB->isChecked() && !Scale->text().isEmpty()) {
		igp.scale = fromqstr(Scale->text());
		igp.width = Length("0pt");
		igp.height = Length("0pt");
		igp.keepAspectRatio = false;
	} else {
		igp.scale = string();
		igp.width = WidthCB->isChecked() ?
			//Note that this works even if Width is "auto", since in
			//that case we get "0pt".
			Length(widgetsToLength(Width, widthUnit)):
			Length("0pt");
		igp.height = HeightCB->isChecked() ?
			Length(widgetsToLength(Height, heightUnit)) :
			Length("0pt");
		igp.keepAspectRatio = aspectratio->isEnabled() &&
			aspectratio->isChecked() &&
			igp.width.value() > 0 && igp.height.value() > 0;
	}

	igp.noUnzip = unzipCB->isChecked();
	igp.lyxscale = displayscale->text().toInt();
	igp.rotateAngle = fromqstr(angle->text());

	double rotAngle = convert<double>(igp.rotateAngle);
	if (std::abs(rotAngle) > 360.0) {
		rotAngle -= 360.0 * floor(rotAngle / 360.0);
		igp.rotateAngle = convert<string>(rotAngle);
	}

	// save the latex name for the origin. If it is the default
	// then origin_ltx returns ""
	igp.rotateOrigin = origin_ltx[origin->currentIndex()];
	igp.scaleBeforeRotation = rotateOrderCB->isChecked();

	// more latex options
	igp.special = fromqstr(latexoptions->text());
}


void GuiGraphicsDialog::getBB()
{
	string const fn = fromqstr(filename->text());
	if (!fn.empty()) {
		string const bb = controller().readBB(fn);
		if (!bb.empty()) {
			lbX->setText(toqstr(token(bb, ' ', 0)));
			lbY->setText(toqstr(token(bb, ' ', 1)));
			rtX->setText(toqstr(token(bb, ' ', 2)));
			rtY->setText(toqstr(token(bb, ' ', 3)));
			// the default units for the bb values when reading
			// it from the file
			lbXunit->setCurrentIndex(0);
			lbYunit->setCurrentIndex(0);
			rtXunit->setCurrentIndex(0);
			rtYunit->setCurrentIndex(0);
		}
		controller().bbChanged = false;
	}
}


bool GuiGraphicsDialog::isValid()
{
	return !filename->text().isEmpty();
}

} // namespace frontend
} // namespace lyx


#include "GuiGraphics_moc.cpp"
