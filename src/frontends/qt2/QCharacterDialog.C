/**
 * \file QCharacterDialog.C
 * Copyright 2001 the LyX Team
 * See the file COPYING.
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#include "QCharacterDialog.h"
#include "lyxtext.h"
#include "language.h"

#include <qcombobox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qgroupbox.h>

QCharacterDialog::QCharacterDialog(QCharacter * form, QWidget * parent,  const char * name, bool modal, WFlags fl)
	 : QCharacterDialogBase(parent, name, modal, fl), form_(form)
{
	setCaption(name);
}
 

QCharacterDialog::~QCharacterDialog()
{
}
 

LyXFont QCharacterDialog::getChar()
{
	LyXFont font(LyXFont::ALL_IGNORE);
	
	switch (family->currentItem()) {
		case 0: font.setFamily(LyXFont::IGNORE_FAMILY); break;
		case 1: font.setFamily(LyXFont::INHERIT_FAMILY); break;
		case 2: font.setFamily(LyXFont::ROMAN_FAMILY); break;
		case 3: font.setFamily(LyXFont::SANS_FAMILY); break;
		case 4: font.setFamily(LyXFont::TYPEWRITER_FAMILY); break;
	}
	
	switch (series->currentItem()) {
		case 0: font.setSeries(LyXFont::IGNORE_SERIES); break;
		case 1: font.setSeries(LyXFont::INHERIT_SERIES); break;
		case 2: font.setSeries(LyXFont::MEDIUM_SERIES); break;
		case 3: font.setSeries(LyXFont::BOLD_SERIES); break;
	}
	
	switch (shape->currentItem()) {
		case 0: font.setShape(LyXFont::IGNORE_SHAPE); break;
		case 1: font.setShape(LyXFont::INHERIT_SHAPE); break;
		case 2: font.setShape(LyXFont::UP_SHAPE); break;
		case 3: font.setShape(LyXFont::ITALIC_SHAPE); break;
		case 4: font.setShape(LyXFont::SLANTED_SHAPE); break;
		case 5: font.setShape(LyXFont::SMALLCAPS_SHAPE); break;
	}
	
	switch (size->currentItem()) {
		case 0: font.setSize(LyXFont::IGNORE_SIZE); break;
		case 1: font.setSize(LyXFont::INHERIT_SIZE); break;
		case 2: font.setSize(LyXFont::SIZE_TINY); break;
		case 3: font.setSize(LyXFont::SIZE_SCRIPT); break;
		case 4: font.setSize(LyXFont::SIZE_FOOTNOTE); break;
		case 5: font.setSize(LyXFont::SIZE_SMALL); break;
		case 6: font.setSize(LyXFont::SIZE_NORMAL); break;
		case 7: font.setSize(LyXFont::SIZE_LARGE); break;
		case 8: font.setSize(LyXFont::SIZE_LARGER); break;
		case 9: font.setSize(LyXFont::SIZE_LARGEST); break;
		case 10: font.setSize(LyXFont::SIZE_HUGE); break;
		case 11: font.setSize(LyXFont::SIZE_HUGER); break;
		case 12: font.setSize(LyXFont::INCREASE_SIZE); break;
		case 13: font.setSize(LyXFont::DECREASE_SIZE); break;
	}
	
	switch (misc->currentItem()) {
		case 0: font.setEmph(LyXFont::IGNORE);
			font.setUnderbar(LyXFont::IGNORE);
			font.setNoun(LyXFont::IGNORE);
			// FIXME font.setLatex(LyXFont::IGNORE);
			break;
		case 1: font.setEmph(LyXFont::INHERIT);
			font.setUnderbar(LyXFont::INHERIT);
			font.setNoun(LyXFont::INHERIT);
			// FIXME font.setLatex(LyXFont::INHERIT);
			break;
		case 2: font.setEmph(LyXFont::TOGGLE); break;
		case 3: font.setUnderbar(LyXFont::TOGGLE); break;
		case 4: font.setNoun(LyXFont::TOGGLE); break;
		case 5:  break; // FIXMEfont.setLatex(LyXFont::TOGGLE); break;
	}
	
	switch (color->currentItem()) {
		case 0: font.setColor(LColor::ignore); break;
		case 1: font.setColor(LColor::inherit); break;
		case 2: font.setColor(LColor::none); break;
		case 3: font.setColor(LColor::black); break;
		case 4: font.setColor(LColor::white); break;
		case 5: font.setColor(LColor::red); break;
		case 6: font.setColor(LColor::green); break;
		case 7: font.setColor(LColor::blue); break;
		case 8: font.setColor(LColor::cyan); break;
		case 9: font.setColor(LColor::magenta); break;
		case 10: font.setColor(LColor::yellow); break;
	}
	
	int const pos = lang->currentItem();
	if (pos == 0) {
		font.setLanguage(ignore_language);
	} else if (pos != 1) {
		font.setLanguage(languages.getLanguage(lang->currentText().latin1()));
	};

	return font;
}

 
void QCharacterDialog::closeEvent(QCloseEvent * e)
{
	form_->close();
	e->accept();
}

 
void QCharacterDialog::setReadOnly(bool readonly)
{
	sizeGB->setEnabled(!readonly);
	charGB->setEnabled(!readonly);
	miscGB->setEnabled(!readonly);
	okPB->setEnabled(!readonly);
	applyPB->setEnabled(!readonly);
}
