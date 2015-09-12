/**
 * \file CustomizedWidgets.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 * \author Edwin Leuven
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

/*
	The code for the ShortcutWidget class was adapted from
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
#include <QKeyEvent>
#include <QMouseEvent>
#include <QShowEvent>

#include "support/qstring_helpers.h"


using lyx::KeySymbol;
using lyx::KeySequence;
using lyx::KeyModifier;
using lyx::toqstr;

namespace lyx {
namespace frontend {

ShortcutWidget::ShortcutWidget(QWidget * parent)
	: QLabel(parent), keysequence_()
{
	QApplication::instance()->installEventFilter(this);
	has_cursor_ = false;
	setFrameShape(QFrame::Panel);
	setFrameShadow(QFrame::Raised);
	setLineWidth(3);
	setFocusPolicy(Qt::StrongFocus);
	setAlignment(Qt::AlignCenter);
}


void ShortcutWidget::reset()
{
	clear();
	keysequence_ = KeySequence();
}


bool ShortcutWidget::eventFilter(QObject * obj, QEvent * e)
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


KeySequence const ShortcutWidget::getKeySequence() const
{
	return keysequence_;
}


void ShortcutWidget::setKeySequence(lyx::KeySequence const & s)
{
	keysequence_ = s;
}


void ShortcutWidget::keyPressEvent(QKeyEvent * e)
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
			setText(toqstr(keysequence_.print(KeySequence::ForGui)));
	}
}


bool ShortcutWidget::event(QEvent * e)
{
	switch (e->type()) {
		case QEvent::FocusOut:
			has_cursor_ = false;
			setFrameShadow(QFrame::Raised);
			break;
		case QEvent::FocusIn:
			has_cursor_ = true;
			setFrameShadow(QFrame::Sunken);
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
	return QLabel::event(e);
}


void ShortcutWidget::appendToSequence(QKeyEvent * e)
{
	KeySymbol sym;
	setKeySymbol(&sym, e);
	
	if (sym.isOK()) {
		KeyModifier mod = lyx::q_key_state(e->modifiers());
		keysequence_.addkey(sym, mod, lyx::NoModifier);
	}
}


void ShortcutWidget::removeFromSequence()
{
	keysequence_.removeKey();
	setText(toqstr(keysequence_.print(KeySequence::ForGui)));
}

} // namespace frontend
} // namespace lyx

#include "moc_CustomizedWidgets.cpp"
