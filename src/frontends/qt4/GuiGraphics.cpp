/**
 * \file GuiGraphics.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author John Levon
 * \author Edwin Leuven
 * \author Herbert Voß
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiGraphics.h"
#include "frontends/alert.h"
#include "qt_helpers.h"
#include "Validator.h"

#include "Buffer.h"
#include "FuncRequest.h"
#include "LengthCombo.h"
#include "Length.h"
#include "LyXRC.h"

#include "graphics/epstools.h"
#include "graphics/GraphicsCache.h"
#include "graphics/GraphicsCacheItem.h"
#include "graphics/GraphicsImage.h"

#include "insets/InsetGraphics.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/Package.h"
#include "support/types.h"

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QValidator>

#include <cmath>
#include <utility>

using namespace std;
using namespace lyx::support;

namespace {

// These are the strings that are stored in the LyX file and which
// correspond to the LaTeX identifiers shown in the comments at the
// end of each line.
char const * const rorigin_lyx_strs[] = {
	// the LaTeX default is leftBaseline
	"",
	"leftTop",  "leftBottom", "leftBaseline", // lt lb lB
	"center", "centerTop", "centerBottom", "centerBaseline", // c ct cb cB
	"rightTop", "rightBottom", "rightBaseline" }; // rt rb rB

// These are the strings, corresponding to the above, that the GUI should
// use. Note that they can/should be translated.
char const * const rorigin_gui_strs[] = {
	N_("Default"),
	N_("Top left"), N_("Bottom left"), N_("Baseline left"),
	N_("Center"), N_("Top center"), N_("Bottom center"), N_("Baseline center"),
	N_("Top right"), N_("Bottom right"), N_("Baseline right") };

size_t const rorigin_size = sizeof(rorigin_lyx_strs) / sizeof(char *);

static string autostr = N_("automatically");

} // namespace anon


namespace lyx {
namespace frontend {

//FIXME setAutoTextCB should really take an argument, as indicated, that
//determines what text is to be written for "auto". But making
//that work involves more extensive revisions than we now want
//to make, since "auto" also appears in paramsToDialog().
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
static void setAutoTextCB(QCheckBox * checkBox, QLineEdit * lineEdit,
	LengthCombo * lengthCombo/*, string text = "auto"*/)
{
	if (!checkBox->isChecked())
		lengthToWidgets(lineEdit, lengthCombo,
				_(autostr), lengthCombo->currentLengthItem());
	else if (lineEdit->text() == qt_(autostr))
		lengthToWidgets(lineEdit, lengthCombo, string(),
				lengthCombo->currentLengthItem());
}


GuiGraphics::GuiGraphics(GuiView & lv)
	: GuiDialog(lv, "graphics", qt_("Graphics"))
{
	setupUi(this);
	
	//main buttons
	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(restorePB, SIGNAL(clicked()), this, SLOT(slotRestore()));

	//graphics pane
	connect(filename, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(WidthCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(HeightCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(Width, SIGNAL(textChanged(const QString &)),
		this, SLOT(updateAspectRatioStatus()));
	connect(Width, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(Height, SIGNAL(textChanged(const QString &)),
		this, SLOT(updateAspectRatioStatus()));
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

	QDoubleValidator * scaleValidator = 
		new DoubleAutoValidator(Scale, qt_(autostr));
	scaleValidator->setBottom(0);
	scaleValidator->setDecimals(256); //I guess that will do
	Scale->setValidator(scaleValidator);
	Height->setValidator(unsignedLengthAutoValidator(Height, qt_(autostr)));
	Width->setValidator(unsignedLengthAutoValidator(Width, qt_(autostr)));
	angle->setValidator(new QDoubleValidator(-360, 360, 2, angle));

	//clipping pane
	connect(clip, SIGNAL(stateChanged(int)),
		this, SLOT(change_adaptor()));
	connect(lbY, SIGNAL(textChanged(const QString&)),
		this, SLOT(changeBB()));
	connect(lbYunit, SIGNAL(activated(int)),
		this, SLOT(changeBB()));
	connect(rtY, SIGNAL(textChanged(const QString&)),
		this, SLOT(changeBB()));
	connect(rtYunit, SIGNAL(activated(int)),
		this, SLOT(changeBB()));
	connect(lbX, SIGNAL(textChanged(const QString&)),
		this, SLOT(changeBB()));
	connect(lbXunit, SIGNAL(activated(int)),
		this, SLOT(changeBB()));
	connect(rtX, SIGNAL(textChanged(const QString&)),
		this, SLOT(changeBB()));
	connect(rtXunit, SIGNAL(activated(int)),
		this, SLOT(changeBB()));
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
	connect(displayGB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(displayscale, SIGNAL(textChanged(const QString&)),
		this, SLOT(change_adaptor()));
	connect(groupCO, SIGNAL(currentIndexChanged(int)),
		this, SLOT(changeGroup(int)));

	displayscale->setValidator(new QIntValidator(displayscale));

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setApply(applyPB);
	bc().setRestore(restorePB);
	bc().setCancel(closePB);

	bc().addReadOnly(latexoptions);
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


void GuiGraphics::change_adaptor()
{
	changed();
}


void GuiGraphics::changeGroup(int /* index */)
{
	QString const new_group = groupCO->itemData(
		groupCO->currentIndex()).toString();
	
	// check if the old group consisted only of this member
	if (current_group_ != fromqstr(new_group)
	    && graphics::countGroupMembers(buffer(), current_group_) == 1) {
		if (!new_group.isEmpty()) {
			if (Alert::prompt(_("Dissolve previous group?"), 
				bformat(_("If you assign this graphic to group '%2$s',\n"
					  "the previously assigned group '%1$s' will be dissolved,\n"
					  "because this graphic was its only member.\n"
					  "How do you want to proceed?"),
					from_utf8(current_group_), qstring_to_ucs4(new_group)),
					0, 0,
					bformat(_("Stick with group '%1$s'"),
						from_utf8(current_group_)),
					bformat(_("Assign to group '%1$s' anyway"),
						qstring_to_ucs4(new_group))) == 0) {
				groupCO->setCurrentIndex(
					groupCO->findData(toqstr(current_group_), Qt::MatchExactly));
				return;
			}
		} else {
			if (Alert::prompt(_("Dissolve previous group?"), 
			bformat(_("If you sign off this graphic from group '%1$s',\n"
				  "the group will be dissolved,\n"
				  "because this graphic was its only member.\n"
				  "How do you want to proceed?"),
				from_utf8(current_group_)),
				0, 0,
				bformat(_("Stick with group '%1$s'"),
				from_utf8(current_group_)),
				bformat(_("Sign off from group '%1$s'"),
				from_utf8(current_group_))) == 0) {
			groupCO->setCurrentIndex(
				groupCO->findData(toqstr(current_group_), Qt::MatchExactly));
			return;
			}
		}
	} 

	if (new_group.isEmpty()) {
		changed();
		return;
	}

	string grp = graphics::getGroupParams(buffer(), fromqstr(new_group));
	if (grp.empty()) {
		// group does not exist yet
		changed();
		return;
	}
	
	// filename might have been changed
	QString current_filename = filename->text();

	// group exists: load params into the dialog
	groupCO->blockSignals(true);
	InsetGraphics::string2params(grp, buffer(), params_);
	paramsToDialog(params_);
	groupCO->blockSignals(false);
	
	// reset filename
	filename->setText(current_filename);

	changed();
}


void GuiGraphics::on_newGroupPB_clicked()
{
	docstring newgroup;
	if (!Alert::askForText(newgroup, _("Enter unique group name:")))
		return;
	if (newgroup.empty())
		return;
	if (groupCO->findData(toqstr(newgroup), Qt::MatchExactly) != -1) {
		Alert::warning(_("Group already defined!"), 
			bformat(_("A graphics group with the name '%1$s' already exists."),
				newgroup));
		return;
	}
	groupCO->addItem(toqstr(newgroup), toqstr(newgroup));
	groupCO->setCurrentIndex(
		groupCO->findData(toqstr(newgroup), Qt::MatchExactly));
}


void GuiGraphics::changeBB()
{
	bbChanged = true;
	LYXERR(Debug::GRAPHICS, "[bb_Changed set to true]");
	changed();
}


void GuiGraphics::on_browsePB_clicked()
{
	QString const str = browse(filename->text());
	if (!str.isEmpty()) {
		filename->setText(str);
		changed();
	}
}


void GuiGraphics::on_getPB_clicked()
{
	getBB();
}


void GuiGraphics::setAutoText()
{
	if (scaleCB->isChecked())
		return;
	if (!Scale->isEnabled() && Scale->text() != "100")
		Scale->setText(qt_(autostr));

	setAutoTextCB(WidthCB, Width, widthUnit);
	setAutoTextCB(HeightCB, Height, heightUnit);
}


void GuiGraphics::on_scaleCB_toggled(bool setScale)
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

	rotateOrderCB->setEnabled((WidthCB->isChecked() ||
				 HeightCB->isChecked() ||
				 scaleCB->isChecked()) &&
				 (angle->text() != "0"));

	setAutoText();
	updateAspectRatioStatus();
}


void GuiGraphics::on_WidthCB_toggled(bool setWidth)
{
	Width->setEnabled(setWidth);
	widthUnit->setEnabled(setWidth);
	if (setWidth)
		Width->setFocus(Qt::OtherFocusReason);

	bool const setHeight = HeightCB->isChecked();
	scaleCB->setEnabled(!setWidth && !setHeight);
	//already will be unchecked, so don't need to do that
	Scale->setEnabled((!setWidth && !setHeight) //=scaleCB->isEnabled()
			&& scaleCB->isChecked()); //should be false, but let's check
	rotateOrderCB->setEnabled((setWidth || setHeight ||
				 scaleCB->isChecked()) &&
				 (angle->text() != "0"));

	setAutoText();
	updateAspectRatioStatus();
}


void GuiGraphics::on_HeightCB_toggled(bool setHeight)
{
	Height->setEnabled(setHeight);
	heightUnit->setEnabled(setHeight);
	if (setHeight)
		Height->setFocus(Qt::OtherFocusReason);

	bool const setWidth = WidthCB->isChecked();
	scaleCB->setEnabled(!setWidth && !setHeight);
	//already unchecked
	Scale->setEnabled((!setWidth && !setHeight) //=scaleCB->isEnabled()
		&& scaleCB->isChecked()); //should be false
	rotateOrderCB->setEnabled((setWidth || setHeight ||
				 scaleCB->isChecked()) &&
				 (angle->text() != "0"));

	setAutoText();
	updateAspectRatioStatus();
}


void GuiGraphics::updateAspectRatioStatus()
{
	// keepaspectratio only makes sense if both a width _and_ a
	// height are given, since its function is (see graphics manual):
	// "If set to true then specifying both 'width' and 'height'
	// (or 'totalheight') does not distort the figure but scales
	// such that neither of the specified dimensions is _exceeded_."
	aspectratio->setEnabled(
		WidthCB->isChecked() && !Width->text().isEmpty()
		&& Width->text() != qt_(autostr)
		&& HeightCB->isChecked() && !Height->text().isEmpty()
		&& Height->text() != qt_(autostr)
		);
	if (!aspectratio->isEnabled())
		aspectratio->setChecked(false);
}


void GuiGraphics::on_aspectratio_toggled(bool aspectratio)
{
	if (aspectratio) {
		WidthCB->setText(qt_("Set max. &width:"));
		HeightCB->setText(qt_("Set max. &height:"));
		Width->setToolTip(qt_("Maximal width of image in output"));
		Height->setToolTip(qt_("Maximal height of image in output"));
	} else {
		WidthCB->setText(qt_("Set &width:"));
		HeightCB->setText(qt_("Set &height:"));
		Width->setToolTip(qt_("Width of image in output"));
		Height->setToolTip(qt_("Height of image in output"));
	}
}


void GuiGraphics::on_angle_textChanged(const QString & filename)
{
	rotateOrderCB->setEnabled((WidthCB->isChecked() ||
				 HeightCB->isChecked() ||
				 scaleCB->isChecked()) &&
				 (filename != "0"));
}


void GuiGraphics::paramsToDialog(InsetGraphicsParams const & igp)
{
	static char const * const bb_units[] = { "bp", "cm", "mm", "in" };
	static char const * const bb_units_gui[] = { N_("bp"), N_("cm"), N_("mm"), N_("in[[unit of measure]]") };
	size_t const bb_size = sizeof(bb_units) / sizeof(bb_units[0]);

	lbXunit->clear();
	lbYunit->clear();
	rtXunit->clear();
	rtYunit->clear();
	
	for (size_t i = 0; i < bb_size; i++) {
		lbXunit->addItem(qt_(bb_units_gui[i]),
			toqstr(bb_units[i]));
		lbYunit->addItem(qt_(bb_units_gui[i]),
			toqstr(bb_units[i]));
		rtXunit->addItem(qt_(bb_units_gui[i]),
			toqstr(bb_units[i]));
		rtYunit->addItem(qt_(bb_units_gui[i]),
			toqstr(bb_units[i]));
	}
	
	// set the right default unit
	Length::UNIT const defaultUnit = Length::defaultUnit();

	//lyxerr << bufferFilePath();
	string const name =
		igp.filename.outputFileName(fromqstr(bufferFilePath()));
	filename->setText(toqstr(name));

	// set the bounding box values
	if (igp.bb.empty()) {
		string const bb = readBoundingBox(igp.filename.absFileName());
		// the values from the file always have the bigpoint-unit bp
		doubleToWidget(lbX, token(bb, ' ', 0));
		doubleToWidget(lbY, token(bb, ' ', 1));
		doubleToWidget(rtX, token(bb, ' ', 2));
		doubleToWidget(rtY, token(bb, ' ', 3));
		lbXunit->setCurrentIndex(0);
		lbYunit->setCurrentIndex(0);
		rtXunit->setCurrentIndex(0);
		rtYunit->setCurrentIndex(0);
		bbChanged = false;
	} else {
		// get the values from the inset
		Length anyLength;
		string const xl = token(igp.bb, ' ', 0);
		string const yl = token(igp.bb, ' ', 1);
		string const xr = token(igp.bb, ' ', 2);
		string const yr = token(igp.bb, ' ', 3);
		if (isValidLength(xl, &anyLength)) {
			doubleToWidget(lbX, anyLength.value());
			string const unit = unit_name[anyLength.unit()];
			lbXunit->setCurrentIndex(lbXunit->findData(toqstr(unit)));
		} else {
			lbX->setText(toqstr(xl));
		}
		if (isValidLength(yl, &anyLength)) {
			doubleToWidget(lbY, anyLength.value());
			string const unit = unit_name[anyLength.unit()];
			lbYunit->setCurrentIndex(lbYunit->findData(toqstr(unit)));
		} else {
			lbY->setText(toqstr(xl));
		}
		if (isValidLength(xr, &anyLength)) {
			doubleToWidget(rtX, anyLength.value());
			string const unit = unit_name[anyLength.unit()];
			rtXunit->setCurrentIndex(rtXunit->findData(toqstr(unit)));
		} else {
			rtX->setText(toqstr(xl));
		}
		if (isValidLength(yr, &anyLength)) {
			doubleToWidget(rtY, anyLength.value());
			string const unit = unit_name[anyLength.unit()];
			rtYunit->setCurrentIndex(rtYunit->findData(toqstr(unit)));
		} else {
			rtY->setText(toqstr(xl));
		}
		bbChanged = true;
	}

	// Update the draft and clip mode
	draftCB->setChecked(igp.draft);
	clip->setChecked(igp.clip);
	unzipCB->setChecked(igp.noUnzip);
	displayGB->setChecked(igp.display);
	displayscale->setText(toqstr(convert<string>(igp.lyxscale)));

	// the output section (width/height)

	doubleToWidget(Scale, igp.scale);
	//igp.scale defaults to 100, so we treat it as empty
	bool const scaleChecked = !igp.scale.empty() && igp.scale != "100";
	scaleCB->blockSignals(true);
	scaleCB->setChecked(scaleChecked);
	scaleCB->blockSignals(false);
	Scale->setEnabled(scaleChecked);
	displayGB->setEnabled(lyxrc.display_graphics);

	set<string> grp;
	graphics::getGraphicsGroups(buffer(), grp);
	set<string>::const_iterator it = grp.begin();
	set<string>::const_iterator end = grp.end();
	groupCO->blockSignals(true);
	groupCO->clear();
	for (; it != end; ++it)
		groupCO->addItem(toqstr(*it), toqstr(*it));
	groupCO->insertItem(0, qt_("None"), QString());
	if (igp.groupId.empty())
		groupCO->setCurrentIndex(0);
	else
		groupCO->setCurrentIndex(
			groupCO->findData(toqstr(igp.groupId), Qt::MatchExactly));
	groupCO->blockSignals(false);

	if (igp.width.value() == 0)
		lengthToWidgets(Width, widthUnit, _(autostr), defaultUnit);
	else
		lengthToWidgets(Width, widthUnit, igp.width, defaultUnit);

	bool const widthChecked = !Width->text().isEmpty() &&
		Width->text() != qt_(autostr);
	WidthCB->blockSignals(true);
	WidthCB->setChecked(widthChecked);
	WidthCB->blockSignals(false);
	Width->setEnabled(widthChecked);
	widthUnit->setEnabled(widthChecked);

	if (igp.height.value() == 0)
		lengthToWidgets(Height, heightUnit, _(autostr), defaultUnit);
	else
		lengthToWidgets(Height, heightUnit, igp.height, defaultUnit);

	bool const heightChecked = !Height->text().isEmpty()
		&& Height->text() != qt_(autostr);
	HeightCB->blockSignals(true);
	HeightCB->setChecked(heightChecked);
	HeightCB->blockSignals(false);
	Height->setEnabled(heightChecked);
	heightUnit->setEnabled(heightChecked);

	scaleCB->setEnabled(!widthChecked && !heightChecked);
	WidthCB->setEnabled(!scaleChecked);
	HeightCB->setEnabled(!scaleChecked);

	setAutoText();
	updateAspectRatioStatus();

	doubleToWidget(angle, igp.rotateAngle);
	rotateOrderCB->setChecked(igp.scaleBeforeRotation);

	rotateOrderCB->setEnabled( (widthChecked || heightChecked || scaleChecked)
		&& igp.rotateAngle != "0");

	origin->clear();

	for (size_t i = 0; i < rorigin_size; i++) {
		origin->addItem(qt_(rorigin_gui_strs[i]),
			toqstr(rorigin_lyx_strs[i]));
	}

	if (!igp.rotateOrigin.empty())
		origin->setCurrentIndex(origin->findData(toqstr(igp.rotateOrigin)));
	else
		origin->setCurrentIndex(0);

	// latex section
	latexoptions->setText(toqstr(igp.special));
	// cf bug #3852
	filename->setFocus();
}


void GuiGraphics::applyView()
{
	InsetGraphicsParams & igp = params_;

	igp.filename.set(fromqstr(filename->text()), fromqstr(bufferFilePath()));

	// the bb section
	igp.bb.erase();
	if (bbChanged) {
		string bb;
		string lbXs = widgetToDoubleStr(lbX);
		string lbYs = widgetToDoubleStr(lbY);
		string rtXs = widgetToDoubleStr(rtX);
		string rtYs = widgetToDoubleStr(rtY);
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
	igp.display = displayGB->isChecked();

	//the graphics section
	if (scaleCB->isChecked() && !Scale->text().isEmpty()) {
		igp.scale = widgetToDoubleStr(Scale);
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
		igp.keepAspectRatio = aspectratio->isChecked();
	}

	igp.noUnzip = unzipCB->isChecked();
	igp.lyxscale = displayscale->text().toInt();
	igp.rotateAngle = widgetToDoubleStr(angle);

	double rotAngle = widgetToDouble(angle);
	if (abs(rotAngle) > 360.0) {
		rotAngle -= 360.0 * floor(rotAngle / 360.0);
		igp.rotateAngle = convert<string>(rotAngle);
	}

	// save the latex name for the origin. If it is the default
	// then origin_ltx returns ""
	igp.rotateOrigin =
		fromqstr(origin->itemData(origin->currentIndex()).toString());
	igp.scaleBeforeRotation = rotateOrderCB->isChecked();

	// more latex options
	igp.special = fromqstr(latexoptions->text());

	igp.groupId = fromqstr(groupCO->itemData(
		groupCO->currentIndex()).toString());
	current_group_ = igp.groupId;
}


void GuiGraphics::getBB()
{
	string const fn = fromqstr(filename->text());
	if (fn.empty())
		return;
	string const bb = readBoundingBox(fn);
	bbChanged = false;
	if (bb.empty())
		return;
	doubleToWidget(lbX, token(bb, ' ', 0));
	doubleToWidget(lbY, token(bb, ' ', 1));
	doubleToWidget(rtX, token(bb, ' ', 2));
	doubleToWidget(rtY, token(bb, ' ', 3));
	// the default units for the bb values when reading
	// it from the file
	lbXunit->setCurrentIndex(0);
	lbYunit->setCurrentIndex(0);
	rtXunit->setCurrentIndex(0);
	rtYunit->setCurrentIndex(0);
}


bool GuiGraphics::isValid()
{
	return !filename->text().isEmpty();
}


bool GuiGraphics::initialiseParams(string const & data)
{
	InsetGraphics::string2params(data, buffer(), params_);
	paramsToDialog(params_);
	current_group_ = params_.groupId;
	return true;
}


void GuiGraphics::clearParams()
{
	params_ = InsetGraphicsParams();
}


void GuiGraphics::dispatchParams()
{
	InsetGraphicsParams tmp_params(params_);
	string const lfun = InsetGraphics::params2string(tmp_params, buffer());
	dispatch(FuncRequest(getLfun(), lfun));
}


QString GuiGraphics::browse(QString const & in_name) const
{
	QString const title = qt_("Select graphics file");

	// Does user clipart directory exist?
	string clipdir = addName(package().user_support().absFileName(), "clipart");
	FileName clip(clipdir);

	// bail out to system clipart directory
	if (!clip.isDirectory())
		clipdir = addName(package().system_support().absFileName(), "clipart");

	return browseRelToParent(in_name, bufferFilePath(),
		title, fileFilters(QString()), false, 
		qt_("Clipart|#C#c"), toqstr(clipdir),
		qt_("Documents|#o#O"), toqstr(lyxrc.document_path));
}


string GuiGraphics::readBoundingBox(string const & file)
{
	FileName const abs_file = support::makeAbsPath(file, fromqstr(bufferFilePath()));

	// try to get it from the file, if possible. Zipped files are
	// unzipped in the readBB_from_PSFile-Function
	string const bb = graphics::readBB_from_PSFile(abs_file);
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


bool GuiGraphics::isFileNameValid(string const & fname) const
{
	// It may be that the filename is relative.
	return support::makeAbsPath(fname, fromqstr(bufferFilePath())).isReadableFile();
}


Dialog * createGuiGraphics(GuiView & lv) { return new GuiGraphics(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiGraphics.cpp"
