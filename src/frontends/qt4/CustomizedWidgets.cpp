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
#include "FuncRequest.h"
#include "GuiKeySymbol.h"

#include <QApplication>
#include <QCloseEvent>

#include "support/qstring_helpers.h"


using lyx::KeySymbol;
using lyx::KeySequence;
using lyx::KeyModifier;
using lyx::toqstr;

ShortcutLineEdit::ShortcutLineEdit(QWidget * parent)
	: QLineEdit(parent), keysequence_()
{
	QApplication::instance()->installEventFilter(this);
	has_cursor_ = false;
}


void ShortcutLineEdit::reset()
{
	clear();
	keysequence_ = KeySequence();
}


bool ShortcutLineEdit::eventFilter(QObject * obj, QEvent * e)
{
	if (!has_cursor_)
		return false;

	switch (e->type()) {
		// swallow these if we have focus and they come from elsewhere
		case QEvent::Shortcut:
		case QEvent::ShortcutOverride:
			if (obj != this)
				return true;
		default: 
			break;
	}        
	return false;
}


KeySequence const ShortcutLineEdit::getKeySequence() const
{
	return keysequence_;
}


void ShortcutLineEdit::keyPressEvent(QKeyEvent * e)
{
	int const keyQt = e->key();
	if (!keyQt)
		return;

	switch(keyQt) {
		case Qt::Key_AltGr: //or else we get unicode salad
		case Qt::Key_Shift:
		case Qt::Key_Control:
		case Qt::Key_Alt:
		case Qt::Key_Meta:
			break;
		default:
			appendToSequence(e);
			setText(toqstr(keysequence_.print(KeySequence::BindFile)));
	}
}


bool ShortcutLineEdit::event(QEvent * e)
{
	switch (e->type()) {
		case QEvent::FocusOut:
			has_cursor_ = false;
			break;
		case QEvent::FocusIn:
			has_cursor_ = true;
			break;
		case QEvent::ShortcutOverride:
			keyPressEvent(static_cast<QKeyEvent *>(e));
			return true;
		case QEvent::KeyRelease:
		case QEvent::Shortcut:
		case QEvent::KeyPress:
			return true;
		default: 
			break;
	}
	return QLineEdit::event(e);
}


void ShortcutLineEdit::appendToSequence(QKeyEvent * e)
{
	KeySymbol sym;
	setKeySymbol(&sym, e);

	KeyModifier mod = lyx::q_key_state(e->modifiers());
	
	keysequence_.addkey(sym, mod, lyx::NoModifier);
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
