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

#include <QCheckBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QList>
#include <QValidator>


namespace lyx {
namespace frontend {

static void setWidgetEnabled(QWidget * obj, bool enabled)
{
	if (QLineEdit * le = qobject_cast<QLineEdit*>(obj))
		le->setReadOnly(!enabled);
	else
		obj->setEnabled(enabled);

	obj->setFocusPolicy(enabled ? Qt::StrongFocus : Qt::NoFocus);
}


/////////////////////////////////////////////////////////////////////////
//
// CheckedLineEdit
//
/////////////////////////////////////////////////////////////////////////

class CheckedLineEdit
{
public:
	CheckedLineEdit(QLineEdit * input, QWidget * label = 0);
	bool check() const;

private:
	// non-owned
	QLineEdit * input_;
	QWidget * label_;
};


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


/////////////////////////////////////////////////////////////////////////
//
// ButtonController::Private
//
/////////////////////////////////////////////////////////////////////////

class ButtonController::Private
{
public:
	typedef QList<CheckedLineEdit> CheckedWidgetList;

	Private()
		: okay_(0), apply_(0), cancel_(0), restore_(0), auto_apply_(0),
			policy_(ButtonPolicy::IgnorantPolicy)
	{}

	/// \return true if all CheckedWidgets are in a valid state.
	bool checkWidgets() const
	{
		bool valid = true;
		for (const CheckedLineEdit & w : checked_widgets_)
			valid &= w.check();
		return valid;
	}

public:
	CheckedWidgetList checked_widgets_;

	QPushButton * okay_;
	QPushButton * apply_;
	QPushButton * cancel_;
	QPushButton * restore_;
	QCheckBox * auto_apply_;

	typedef QList<QWidget *> Widgets;
	Widgets read_only_;

	ButtonPolicy policy_;
};


/////////////////////////////////////////////////////////////////////////
//
// ButtonController
//
/////////////////////////////////////////////////////////////////////////

ButtonController::ButtonController()
	: d(new Private)
{}


ButtonController::~ButtonController()
{
	delete d;
}


void ButtonController::setPolicy(ButtonPolicy::Policy policy)
{
	d->policy_.setPolicy(policy);
}


void ButtonController::ok()
{
	input(ButtonPolicy::SMI_OKAY);
}


void ButtonController::input(ButtonPolicy::SMInput in)
{
	if (ButtonPolicy::SMI_NOOP == in)
		return;
	d->policy_.input(in);
	refresh();
}


void ButtonController::apply()
{
	input(ButtonPolicy::SMI_APPLY);
}


void ButtonController::autoApply()
{
	input(ButtonPolicy::SMI_AUTOAPPLY);
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

	d->policy_.input(ro ?
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

	bool const all_valid = d->checkWidgets();

	if (d->okay_) {
		bool const enabled =
			all_valid && policy().buttonStatus(ButtonPolicy::OKAY);
		d->okay_->setEnabled(enabled);
	}
	if (d->apply_) {
		bool const enabled =
			all_valid && policy().buttonStatus(ButtonPolicy::APPLY);
		d->apply_->setEnabled(enabled);
	}
	if (d->restore_) {
		bool const enabled =
			all_valid && policy().buttonStatus(ButtonPolicy::RESTORE);
		d->restore_->setEnabled(enabled);
	}
	if (d->cancel_) {
		bool const enabled = policy().buttonStatus(ButtonPolicy::CANCEL);
		if (enabled)
			d->cancel_->setText(qt_("Cancel"));
		else
			d->cancel_->setText(qt_("Close"));
	}
	if (d->auto_apply_) {
		bool const enabled = policy().buttonStatus(ButtonPolicy::AUTOAPPLY);
		d->auto_apply_->setEnabled(enabled);
	}

}


void ButtonController::refreshReadOnly() const
{
	if (d->read_only_.empty())
		return;
	bool const enable = !policy().isReadOnly();
	for(QWidget * w : d->read_only_)
		setWidgetEnabled(w, enable);
}


void ButtonController::addCheckedLineEdit(QLineEdit * input, QWidget * label)
{
	d->checked_widgets_.append(CheckedLineEdit(input, label));
}


void ButtonController::setOK(QPushButton * obj)
{
	d->okay_ = obj;
}


void ButtonController::setApply(QPushButton * obj)
{
	d->apply_ = obj;
}


void ButtonController::setAutoApply(QCheckBox * obj)
{
	d->auto_apply_ = obj;
}


void ButtonController::setCancel(QPushButton * obj)
{
	d->cancel_ = obj;
}


void ButtonController::setRestore(QPushButton * obj)
{
	d->restore_ = obj;
}


void ButtonController::addReadOnly(QWidget * obj)
{
	d->read_only_.push_back(obj);
}

ButtonPolicy const & ButtonController::policy() const
{
	return d->policy_;
}


ButtonPolicy & ButtonController::policy()
{
	return d->policy_;
}

} // namespace frontend
} // namespace lyx
