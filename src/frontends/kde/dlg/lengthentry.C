/*
 * lengthentry.C
 * (C) 2001 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 */

#include <config.h>

#include "lengthentry.h"

#include <qlayout.h>
#include <krestrictedline.h>
#include <qcombobox.h>

#include "support/lstrings.h"
#include <gettext.h>

#include "debug.h"

/**
 * Allows editing of LyXLengths
 */

using std::endl;

LengthEntry::LengthEntry(QWidget * parent, const char * name)
	: QWidget(parent,name), value(0), box(0), topLayout(0)
{
	value = new KRestrictedLine(this, "value");
	value->setMinimumSize(value->sizeHint());
	value->setMaximumHeight(value->sizeHint().height());
	value->setValidChars("0123456789.,");

	box = new QComboBox(this, "box");
	box->insertItem(_("Centimetres"));
	box->insertItem(_("Inches"));
	box->insertItem(_("Points"));
	box->insertItem(_("Millimetres"));
	box->insertItem(_("Picas"));
	box->insertItem(_("ex units"));
	box->insertItem(_("em units"));
	box->insertItem(_("Scaled points"));
	box->insertItem(_("Big/PS points"));
	box->insertItem(_("Didot points"));
	box->insertItem(_("Cicero points"));
	box->setMinimumSize(box->sizeHint());
	box->setMaximumHeight(box->sizeHint().height());

	topLayout = new QVBoxLayout(this, 10);
	QHBoxLayout *tmp = new QHBoxLayout();
	topLayout->addLayout(tmp);

	tmp->addStretch(1);
	tmp->addWidget(value, 1);
	tmp->addStretch(1);
	tmp->addSpacing(5); 
	tmp->addWidget(box, 2);
	tmp->addStretch(1);
}

LengthEntry::~LengthEntry()
{
}

const string LengthEntry::getLengthStr() const
{
	if (value->text() == "")
		return string("");

	double val = getValue();

	return tostr(val) + getStrUnits();
}

double LengthEntry::getValue() const
{
	return strToDbl(value->text());
}

const string LengthEntry::getStrUnits() const
{
	switch (box->currentItem()) {
		case 0: return string("cm");
		case 1: return string("in");
		case 2: return string("pt");
		case 3: return string("mm");
		case 4: return string("pc");
		case 5: return string("ex");
		case 6: return string("em");
		case 7: return string("sp");
		case 8: return string("bp");
		case 9: return string("dd");
		case 10: return string("cc");
		default:
			lyxerr[Debug::GUI] <<
				"LengthEntry: Unknown combo choice " << box->currentItem() << endl;
	}
	return string("");
}
	
LyXLength::UNIT LengthEntry::getUnits() const
{
	switch (box->currentItem()) {
		case 0: return LyXLength::CM;
		case 1: return LyXLength::IN;
		case 2: return LyXLength::PT;
		case 3: return LyXLength::MM;
		case 4: return LyXLength::PC;
		case 5: return LyXLength::EX;
		case 6: return LyXLength::EM;
		case 7: return LyXLength::SP;
		case 8: return LyXLength::BP;
		case 9: return LyXLength::DD;
		case 10: return LyXLength::CC;
		default:
			lyxerr[Debug::GUI] <<
				"LengthEntry: Unknown combo choice " << box->currentItem() << endl;
	}
	return LyXLength::UNIT_NONE;
}

bool LengthEntry::setFromLengthStr(const string & str)
{
	string units;
	string val;

	lyxerr[Debug::GUI] << "setFromLengthStr: " << str << endl;

	string::size_type i = str.find_first_not_of("0123456789.,");

	setValue(strToDbl(str.substr(0, i)));

	if (i == string::npos)
		return true;

	return setUnits(str.substr(i));
}

void LengthEntry::setValue(double val)
{
	value->setText(tostr(val).c_str());
}

void LengthEntry::setValue(const string & str)
{
	value->setText(str.c_str());
}

bool LengthEntry::setUnits(const string & str)
{
	if (str == "cm") { setUnits(LyXLength::CM);
	} else if (str == "in") { setUnits(LyXLength::IN);
	} else if (str == "pt") { setUnits(LyXLength::PT);
	} else if (str == "mm") { setUnits(LyXLength::MM);
	} else if (str == "pc") { setUnits(LyXLength::PC);
	} else if (str == "ex") { setUnits(LyXLength::EX);
	} else if (str == "em") { setUnits(LyXLength::EM);
	} else if (str == "sp") { setUnits(LyXLength::SP);
	} else if (str == "bp") { setUnits(LyXLength::BP);
	} else if (str == "dd") { setUnits(LyXLength::DD);
	} else if (str == "cc") { setUnits(LyXLength::CC);
	} else {
		lyxerr[Debug::GUI] <<
			"LengthEntry: Unknown unit " << str << endl;
		box->setCurrentItem(0);
		return false;
	}
	return true;
}

bool LengthEntry::setUnits(LyXLength::UNIT unit)
{
	switch (unit) {
		case LyXLength::CM: box->setCurrentItem(0); break;
		case LyXLength::IN: box->setCurrentItem(1); break;
		case LyXLength::PT: box->setCurrentItem(2); break;
		case LyXLength::MM: box->setCurrentItem(3); break;
		case LyXLength::PC: box->setCurrentItem(4); break;
		case LyXLength::EX: box->setCurrentItem(5); break;
		case LyXLength::EM: box->setCurrentItem(6); break;
		case LyXLength::SP: box->setCurrentItem(7); break;
		case LyXLength::BP: box->setCurrentItem(8); break;
		case LyXLength::DD: box->setCurrentItem(9); break;
		case LyXLength::CC: box->setCurrentItem(10); break;
		case LyXLength::MU: box->setCurrentItem(0); break;
		case LyXLength::UNIT_NONE: box->setCurrentItem(0); break;
		default:
			lyxerr[Debug::GUI] << "Unknown unit " << long(unit) << endl;
			return false;
	}
	return true;
}

void LengthEntry::setEnabled(bool enable)
{
	box->setEnabled(enable);
	value->setEnabled(enable);
}
