/**
 * \file Qt2BC.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Qt2BC.h"
#include "BCView.h"
#include "ButtonPolicy.h"
#include "debug.h"
#include "qt_helpers.h"

#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QValidator>


namespace lyx {
namespace frontend {


Qt2BC::Qt2BC(ButtonController & parent)
	: BCView(parent), okay_(0), apply_(0), cancel_(0), restore_(0)
{}


void Qt2BC::refresh() const
{
	LYXERR(Debug::GUI) << "Calling BC refresh()" << std::endl;

	bool const all_valid = checkWidgets();

	if (okay_) {
		bool const enabled =
			all_valid && bp().buttonStatus(ButtonPolicy::OKAY);
		okay_->setEnabled(enabled);
	}
	if (apply_) {
		bool const enabled =
			all_valid && bp().buttonStatus(ButtonPolicy::APPLY);
		apply_->setEnabled(enabled);
	}
	if (restore_) {
		bool const enabled =
			all_valid && bp().buttonStatus(ButtonPolicy::RESTORE);
		restore_->setEnabled(enabled);
	}
	if (cancel_) {
		bool const enabled = bp().buttonStatus(ButtonPolicy::CANCEL);
		if (enabled)
			cancel_->setText(toqstr(_("Cancel")));
		else
			cancel_->setText(toqstr(_("Close")));
	}
}


void Qt2BC::refreshReadOnly() const
{
	if (read_only_.empty())
		return;

	bool const enable = !bp().isReadOnly();

	Widgets::const_iterator end = read_only_.end();
	Widgets::const_iterator iter = read_only_.begin();
	for (; iter != end; ++iter)
		setWidgetEnabled(*iter, enable);
}


void Qt2BC::setWidgetEnabled(QWidget * obj, bool enabled) const
{
	if (QLineEdit * le = qobject_cast<QLineEdit*>(obj))
		le->setReadOnly(!enabled);
	else
		obj->setEnabled(enabled);

	obj->setFocusPolicy(enabled ? Qt::StrongFocus : Qt::NoFocus);
}


void Qt2BC::addCheckedLineEdit(QLineEdit * input, QWidget * label)
{
	checked_widgets.push_back(CheckedLineEdit(input, label));
}


bool Qt2BC::checkWidgets() const
{
	bool valid = true;

	CheckedWidgetList::const_iterator it  = checked_widgets.begin();
	CheckedWidgetList::const_iterator end = checked_widgets.end();

	for (; it != end; ++it)
		valid &= it->check();

	// return valid status after checking ALL widgets
	return valid;
}


//////////////////////////////////////////////////////////////
//
// CheckedLineEdit
//
//////////////////////////////////////////////////////////////

void addCheckedLineEdit(BCView & bcview, QLineEdit * input, QWidget * label)
{
	Qt2BC * bc = static_cast<Qt2BC *>(&bcview);
	bc->addCheckedLineEdit(input, label);
}


static void setWarningColor(QWidget * widget)
{
	QPalette pal = widget->palette();
	pal.setColor(QPalette::Active, QPalette::Foreground, QColor(255, 0, 0));
	widget->setPalette(pal);
}



CheckedLineEdit::CheckedLineEdit(QLineEdit * input, QWidget * label)
	: input_(input), label_(label)
{}


bool CheckedLineEdit::check() const
{
	QValidator const * validator = input_->validator();
	if (!validator)
		return true;

	QString t = input_->text();
	int p = 0;
	bool const valid = validator->validate(t, p) == QValidator::Acceptable;

	// Visual feedback.
	if (valid)
		input_->setPalette(QPalette());
	else
		setWarningColor(input_);

	if (!label_) {
		if (valid)
			label_->setPalette(QPalette());
		else
			setWarningColor(label_);
	}

	return valid;
}

} // namespace frontend
} // namespace lyx
