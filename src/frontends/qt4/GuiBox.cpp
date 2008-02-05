/**
 * \file GuiBox.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna (Minipage stuff)
 * \author Martin Vermeer
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiBox.h"

#include "FuncRequest.h"
#include "support/gettext.h"

#include "LengthCombo.h"
#include "Length.h"
#include "qt_helpers.h"
#include "LyXRC.h" // to set the default length values
#include "Validator.h"

#include "insets/InsetBox.h"

#include "support/lstrings.h"

#include <QPushButton>
#include <QLineEdit>
#include <QCloseEvent>

using namespace std;

namespace lyx {
namespace frontend {


void box_gui_tokens(vector<string> & ids, vector<docstring> & gui_names)
{
	char const * const ids_[] = {
		"Frameless", "Boxed", "ovalbox",
		"Ovalbox", "Shadowbox", "Shaded", "Doublebox"};
	size_t const ids_size = sizeof(ids_) / sizeof(char *);
	ids = vector<string>(ids_, ids_ + ids_size);
	gui_names.clear();
	gui_names.push_back(_("No frame"));
	gui_names.push_back(_("Simple rectangular frame"));
	gui_names.push_back(_("Oval frame, thin"));
	gui_names.push_back(_("Oval frame, thick"));
	gui_names.push_back(_("Drop shadow"));
	gui_names.push_back(_("Shaded background"));
	gui_names.push_back(_("Double rectangular frame"));
}


void box_gui_tokens_special_length(vector<string> & ids,
	vector<docstring> & gui_names)
{
	char const * const ids_[] = {
		"none", "height", "depth",
		"totalheight", "width"};
	size_t const ids_size = sizeof(ids_) / sizeof(char *);
	ids = vector<string>(ids_, ids_ + ids_size);
	gui_names.clear();
	gui_names.push_back(_("None"));
	gui_names.push_back(_("Height"));
	gui_names.push_back(_("Depth"));
	gui_names.push_back(_("Total Height"));
	gui_names.push_back(_("Width"));
}


GuiBox::GuiBox(GuiView & lv)
	: GuiDialog(lv, "box", qt_("Box Settings")), params_("")
{
	setupUi(this);

	// fill the box type choice
	box_gui_tokens(ids_, gui_names_);
	for (unsigned int i = 0; i < gui_names_.size(); ++i)
		typeCO->addItem(toqstr(gui_names_[i]));

	// add the special units to the height choice
	// width needs different handling
	box_gui_tokens_special_length(ids_spec_, gui_names_spec_);
	for (unsigned int i = 1; i < gui_names_spec_.size(); ++i)
		heightUnitsLC->addItem(toqstr(gui_names_spec_[i]));

	connect(restorePB, SIGNAL(clicked()), this, SLOT(slotRestore()));
	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	connect(widthED, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(widthUnitsLC, SIGNAL(selectionChanged(lyx::Length::UNIT)),
		this, SLOT(change_adaptor()));
	connect(valignCO, SIGNAL(highlighted(QString)),
		this, SLOT(change_adaptor()));
	connect(heightCB, SIGNAL(stateChanged(int)),
		this, SLOT(change_adaptor()));
	connect(heightED, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(heightUnitsLC, SIGNAL(selectionChanged(lyx::Length::UNIT) ),
		this, SLOT(change_adaptor()));
	connect(restorePB, SIGNAL(clicked()), this, SLOT(restoreClicked()));
	connect(typeCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(typeCO, SIGNAL(activated(int)), this, SLOT(typeChanged(int)));
	connect(halignCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(ialignCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(innerBoxCO, SIGNAL(activated(const QString&)),
		this, SLOT(innerBoxChanged(const QString &)));
	connect(innerBoxCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(pagebreakCB, SIGNAL(stateChanged(int)),
		this, SLOT(pagebreakClicked()));

	heightED->setValidator(unsignedLengthValidator(heightED));
	widthED->setValidator(unsignedLengthValidator(widthED));

	bc().setPolicy(ButtonPolicy::OkApplyCancelReadOnlyPolicy);

	bc().addReadOnly(typeCO);
	bc().addReadOnly(innerBoxCO);
	bc().addReadOnly(valignCO);
	bc().addReadOnly(ialignCO);
	bc().addReadOnly(halignCO);
	bc().addReadOnly(widthED);
	bc().addReadOnly(widthUnitsLC);
	bc().addReadOnly(heightCB);
	bc().addReadOnly(heightED);
	bc().addReadOnly(heightUnitsLC);
	bc().addReadOnly(pagebreakCB);

	bc().setRestore(restorePB);
	bc().setOK(okPB);
	bc().setApply(applyPB);
	bc().setCancel(closePB);

	// initialize the length validator
	bc().addCheckedLineEdit(widthED, widthLA);
	bc().addCheckedLineEdit(heightED, heightCB);
}


void GuiBox::closeEvent(QCloseEvent * e)
{
	slotClose();
	e->accept();
}


void GuiBox::change_adaptor()
{
	changed();
}


void GuiBox::innerBoxChanged(const QString & str)
{
	bool const ibox = (str != qt_("None"));
	valignCO->setEnabled(ibox);
	ialignCO->setEnabled(ibox);
	halignCO->setEnabled(!ibox);
	heightCB->setEnabled(ibox);
	if (heightCB->checkState() == Qt::Checked && ibox) {
		heightED->setEnabled(true);
		heightUnitsLC->setEnabled(true);
	}
	setSpecial(ibox);
}


void GuiBox::typeChanged(int index)
{
	bool const frameless = (index == 0);
	if (frameless) {
		valignCO->setEnabled(true);
		ialignCO->setEnabled(true);
		halignCO->setEnabled(false);
		heightCB->setEnabled(true);
		heightED->setEnabled(true);
		heightUnitsLC->setEnabled(true);
		setSpecial(true);
	}
	if (index != 1)
		pagebreakCB->setChecked(false);
	pagebreakCB->setEnabled(index == 1);
	int itype = innerBoxCO->currentIndex();
	setInnerType(frameless, itype);
}


void GuiBox::restoreClicked()
{
	setInnerType(true, 2);
	widthED->setText("100");
	widthUnitsLC->setCurrentItem(Length::PCW);
	heightCB->setCheckState(Qt::Checked);
	heightED->setText("1");
	for (int j = 0; j < heightUnitsLC->count(); j++) {
		if (heightUnitsLC->itemText(j) == qt_("Total Height"))
			heightUnitsLC->setCurrentItem(j);
	}
}


void GuiBox::pagebreakClicked()
{
	bool pbreak = (pagebreakCB->checkState() == Qt::Checked);
	innerBoxCO->setEnabled(!pbreak);
	if (pbreak) {
		valignCO->setEnabled(false);
		ialignCO->setEnabled(false);
		halignCO->setEnabled(false);
		heightCB->setEnabled(false);
		heightED->setEnabled(false);
		heightUnitsLC->setEnabled(false);
		setSpecial(false);
	} else
		typeChanged(typeCO->currentIndex());
	change_adaptor();
}

void GuiBox::updateContents()
{
	string type = params_.type;
	if (type == "Framed") {
		pagebreakCB->setChecked(true);
		type = "Boxed";
	} else
		pagebreakCB->setChecked(false);

	pagebreakCB->setEnabled(type == "Boxed");

	for (unsigned int i = 0; i < gui_names_.size(); ++i) {
		if (type == ids_[i])
			typeCO->setCurrentIndex(i);
	}

	// default: minipage
	unsigned int inner_type = 2;
	if (!params_.inner_box)
		// none
		inner_type = 0;
	if (params_.use_parbox)
		// parbox
		inner_type = 1;
	bool frameless = (params_.type == "Frameless");
	setInnerType(frameless, inner_type);

	char c = params_.pos;
	valignCO->setCurrentIndex(string("tcb").find(c, 0));
	c = params_.inner_pos;
	ialignCO->setCurrentIndex(string("tcbs").find(c, 0));
	c = params_.hor_pos;
	halignCO->setCurrentIndex(string("lcrs").find(c, 0));

	bool ibox = params_.inner_box;
	valignCO->setEnabled(ibox);
	ialignCO->setEnabled(ibox);
	halignCO->setEnabled(!ibox);
	setSpecial(ibox);

	Length::UNIT default_unit =
		(lyxrc.default_papersize > 3) ? Length::CM : Length::IN;

	lengthToWidgets(widthED, widthUnitsLC,
		(params_.width).asString(), default_unit);

	string const special = params_.special;
	if (!special.empty() && special != "none") {
		QString spc;
		for (unsigned int i = 0; i < gui_names_spec_.size(); i++) {
			if (special == ids_spec_[i])
				spc = toqstr(gui_names_spec_[i].c_str());
		}
		for (int j = 0; j < widthUnitsLC->count(); j++) {
			if (widthUnitsLC->itemText(j) == spc)
				widthUnitsLC->setCurrentIndex(j);
		}
	}

	lengthToWidgets(heightED, heightUnitsLC,
		(params_.height).asString(), default_unit);
	
	string const height_special = params_.height_special;
	if (!height_special.empty() && height_special != "none") {
		QString hspc;
		for (unsigned int i = 0; i != gui_names_spec_.size(); i++) {
			if (height_special == ids_spec_[i])
				hspc = toqstr(gui_names_spec_[i].c_str());
		}
		for (int j = 0; j != heightUnitsLC->count(); j++) {
			if (heightUnitsLC->itemText(j) == hspc)
				heightUnitsLC->setCurrentIndex(j);
		}
	}
	// set no optional height when the value is the default "1\height"
	// (special units like \height are handled as "in",
	if (height_special == "totalheight" &&  
		params_.height == Length("1in"))
		heightCB->setCheckState(Qt::Unchecked);
	else
		heightCB->setCheckState(Qt::Checked);

	heightCB->setEnabled(ibox);
}


void GuiBox::applyView()
{
	bool pagebreak = pagebreakCB->isChecked();
	if (pagebreak)
		params_.type = "Framed";
	else
		params_.type = ids_[typeCO->currentIndex()];

	params_.inner_box = (!pagebreak && innerBoxCO->currentText() != qt_("None"));
	params_.use_parbox = (!pagebreak && innerBoxCO->currentText() == qt_("Parbox"));

	params_.pos = "tcb"[valignCO->currentIndex()];
	params_.inner_pos = "tcbs"[ialignCO->currentIndex()];
	params_.hor_pos = "lcrs"[halignCO->currentIndex()];

	int i = 0;
	bool spec = false;
	QString special = widthUnitsLC->currentText();
	QString value = widthED->text();
	if (special == qt_("Height")) {
		i = 1;
		spec = true;
	} else if (special == qt_("Depth")) {
		i = 2;
		spec = true;
	} else if (special == qt_("Total Height")) {
		i = 3;
		spec = true;
	} else if (special == qt_("Width")) {
		i = 4;
		spec = true;
	}
	// the user might insert a non-special value in the line edit
	if (isValidLength(fromqstr(value))) {
		i = 0;
		spec = false;
	}
	params_.special = ids_spec_[i];

	string width;
	if (spec) {
		width = fromqstr(value);
		// beware: bogosity! the unit is simply ignored in this case
		width += "in";
	} else {
		width = widgetsToLength(widthED, widthUnitsLC);
	}

	params_.width = Length(width);

	i = 0;
	spec = false;
	special = heightUnitsLC->currentText();
	value = heightED->text();
	if (special == qt_("Height")) {
		i = 1;
		spec = true;
	} else if (special == qt_("Depth")) {
		i = 2;
		spec = true;
	} else if (special == qt_("Total Height")) {
		i = 3;
		spec = true;
	} else if (special == qt_("Width")) {
		i = 4;
		spec = true;
	}
	// the user might insert a non-special value in the line edit
	if (isValidLength(fromqstr(value))) {
		i = 0;
		spec = false;
	}
	params_.height_special = ids_spec_[i];

	string height;
	if (spec  && !isValidLength(fromqstr(heightED->text()))) {
		height = fromqstr(value);
		// beware: bogosity! the unit is simply ignored in this case
		height += "in";
	} else
		height = widgetsToLength(heightED, heightUnitsLC);

	// the height parameter is omitted in InsetBox.cpp when the value
	// is "1in" and "Total Height" is used as unit.
	// 1in + "Total Height" means "1\height" which is the LaTeX default when
	// no height is given
	if (heightCB->checkState() == Qt::Checked)
		params_.height = Length(height);
	else {
		params_.height = Length("1in");
		params_.height_special = ids_spec_[3];
	}
}


void GuiBox::setSpecial(bool ibox)
{
	box_gui_tokens_special_length(ids_spec_, gui_names_spec_);
	// check if the widget contains the special units
	int count = widthUnitsLC->count();
	bool has_special = false;
	for (int i = 0; i < count; i++)
		if (widthUnitsLC->itemText(i).contains(qt_("Total Height")) > 0)
			has_special = true;
	// insert 'em if needed...
	if (!ibox && !has_special) {
		for (unsigned int i = 1; i < gui_names_spec_.size(); i++)
			widthUnitsLC->addItem(toqstr(gui_names_spec_[i]));
	// ... or remove 'em if needed
	} else if (ibox && has_special) {
		widthUnitsLC->clear();
		for (int i = 0; i < num_units; i++)
			widthUnitsLC->addItem(qt_(unit_name_gui[i]));
	}
}


void GuiBox::setInnerType(bool frameless, int i)
{
	// with "frameless" boxes, inner box is mandatory (i.e. is the actual box)
	// we have to remove "none" then and adjust the combo
	if (frameless) {
		innerBoxCO->clear();
		innerBoxCO->addItem(qt_("Parbox"));
		innerBoxCO->addItem(qt_("Minipage"));
		if (i != 0)
			innerBoxCO->setCurrentIndex(i - 1);
		else
			innerBoxCO->setCurrentIndex(i);
	} else {
		if (innerBoxCO->count() == 2)
			++i;
		innerBoxCO->clear();
		innerBoxCO->addItem(qt_("None"));
		innerBoxCO->addItem(qt_("Parbox"));
		innerBoxCO->addItem(qt_("Minipage"));
		innerBoxCO->setCurrentIndex(i);
	}
}

bool GuiBox::initialiseParams(string const & data)
{
	InsetBoxMailer::string2params(data, params_);
	return true;

}


void GuiBox::clearParams()
{
	params_ = InsetBoxParams("");
}


void GuiBox::dispatchParams()
{
	dispatch(FuncRequest(getLfun(), InsetBoxMailer::params2string(params_)));
}


Dialog * createGuiBox(GuiView & lv) { return new GuiBox(lv); }


} // namespace frontend
} // namespace lyx


#include "GuiBox_moc.cpp"
