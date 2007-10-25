/**
 * \file GuiPrefs.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

/*
	The code for the ShortcutLineEdit class was adapted from
	kkeysequencewidget.cpp, which is part of the KDE libraries.
	Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
	Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>
	Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>
	Licensed under version 2 of the General Public License and
	used here in accordance with the terms of that license.
*/

#include <config.h>

#include "CustomizedWidgets.h"
#include "GuiKeySymbol.h"

#include "support/qstring_helpers.h"


using lyx::KeySymbol;
using lyx::toqstr;

void ShortcutLineEdit::keyPressEvent(QKeyEvent * e)
{
	int keyQt = e->key();
	switch (e->key()) {
		case Qt::Key_AltGr: //or else we get unicode salad
		case Qt::Key_Shift:
		case Qt::Key_Control:
		case Qt::Key_Alt:
		case Qt::Key_Meta:
			break;
		default:
			if (keyQt) {
				uint modifierKeys = e->modifiers();

				QString txt;
				if (modifierKeys & Qt::SHIFT)
					txt += "S-";
				if (modifierKeys & Qt::CTRL)
					txt += "C-";
				if (modifierKeys & Qt::ALT)
					txt += "M-";

				KeySymbol sym;
				setKeySymbol(&sym, e);
				txt += toqstr(sym.getSymbolName());

				if (text().isEmpty())
					setText(txt);
				else
					setText(text() + " " + txt);
			}
	}
}


//prevent Qt from special casing Tab and Backtab
bool ShortcutLineEdit::event(QEvent* e)
{
	if (e->type() == QEvent::ShortcutOverride)
		return false;

	if (e->type() == QEvent::KeyPress) {
		keyPressEvent(static_cast<QKeyEvent *>(e));
		return true;
	}

	return QLineEdit::event(e);
}


QString const SearchLineEdit::hintMessage() const
{
	return toqstr("Search ...");
}


void SearchLineEdit::focusInEvent(QFocusEvent * e)
{
	if (text() == hintMessage())
		clear();
}


void SearchLineEdit::focusOutEvent(QFocusEvent * e)
{
	if (text().isEmpty())
		setText(hintMessage());
}


#include "CustomizedWidgets_moc.cpp"
