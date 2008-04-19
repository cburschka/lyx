/**
 * \file ButtonController.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ButtonController.h"

#include "qt_helpers.h"

#include "support/debug.h"

#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QValidator>

namespace lyx {
namespace frontend {

ButtonController::ButtonController()
	: okay_(0), apply_(0), cancel_(0), restore_(0),
		policy_(ButtonPolicy::IgnorantPolicy)
{}


void ButtonController::setPolicy(ButtonPolicy::Policy policy)
{
	policy_.setPolicy(policy);
}


void ButtonController::ok()
{
	input(ButtonPolicy::SMI_OKAY);
}


void ButtonController::input(ButtonPolicy::SMInput in)
{
	if (ButtonPolicy::SMI_NOOP == in)
		return;
	policy_.input(in);
	refresh();
}


void ButtonController::apply()
{
	input(ButtonPolicy::SMI_APPLY);
}


void ButtonController::cancel()
{
	input(ButtonPolicy::SMI_CANCEL);
}


void ButtonController::restore()
{
	input(ButtonPolicy::SMI_RESTORE);
}


void ButtonController::hide()
{
	input(ButtonPolicy::SMI_HIDE);
}


void ButtonController::setValid(bool v)
{
	input(v ? ButtonPolicy::SMI_VALID : ButtonPolicy::SMI_INVALID);
}


bool ButtonController::setReadOnly(bool ro)
{
	LYXERR(Debug::GUI, "Setting controller ro: " << ro);

	policy_.input(ro ?
		ButtonPolicy::SMI_READ_ONLY : ButtonPolicy::SMI_READ_WRITE);
	// refreshReadOnly(); This will enable all widgets in dialogs, no matter if
	//                    they allowed to be enabled, so when you plan to
	//                    reenable this call, read this before:
    // http://www.mail-archive.com/lyx-devel@lists.lyx.org/msg128222.html
	refresh();
	return ro;
}


void ButtonController::refresh() const
{
	LYXERR(Debug::GUI, "Calling BC refresh()");

	bool const all_valid = checkWidgets();

	if (okay_) {
		bool const enabled =
			all_valid && policy().buttonStatus(ButtonPolicy::OKAY);
		okay_->setEnabled(enabled);
	}
	if (apply_) {
		bool const enabled =
			all_valid && policy().buttonStatus(ButtonPolicy::APPLY);
		apply_->setEnabled(enabled);
	}
	if (restore_) {
		bool const enabled =
			all_valid && policy().buttonStatus(ButtonPolicy::RESTORE);
		restore_->setEnabled(enabled);
	}
	if (cancel_) {
		bool const enabled = policy().buttonStatus(ButtonPolicy::CANCEL);
		if (enabled)
			cancel_->setText(qt_("Cancel"));
		else
			cancel_->setText(qt_("Close"));
	}
}


void ButtonController::refreshReadOnly() const
{
	if (read_only_.empty())
		return;

	bool const enable = !policy().isReadOnly();

	Widgets::const_iterator end = read_only_.end();
	Widgets::const_iterator iter = read_only_.begin();
	for (; iter != end; ++iter)
		setWidgetEnabled(*iter, enable);
}


void ButtonController::setWidgetEnabled(QWidget * obj, bool enabled) const
{
	if (QLineEdit * le = qobject_cast<QLineEdit*>(obj))
		le->setReadOnly(!enabled);
	else
		obj->setEnabled(enabled);

	obj->setFocusPolicy(enabled ? Qt::StrongFocus : Qt::NoFocus);
}


void ButtonController::addCheckedLineEdit(QLineEdit * input, QWidget * label)
{
	checked_widgets.push_back(CheckedLineEdit(input, label));
}


bool ButtonController::checkWidgets() const
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
	setValid(input_, valid);
	if (label_)
		setValid(label_, valid);

	return valid;
}

} // namespace frontend
} // namespace lyx
