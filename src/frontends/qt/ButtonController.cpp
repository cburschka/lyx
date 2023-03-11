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
#include "GuiApplication.h"
#include "PanelStack.h"

#include "qt_helpers.h"

#include "support/debug.h"

#include <QCheckBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QList>
#include <QTabWidget>
#include <QValidator>


namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////////////
//
// CheckedLineEdit
//
/////////////////////////////////////////////////////////////////////////

class CheckedLineEdit
{
public:
	CheckedLineEdit(QLineEdit * input, QWidget * label = nullptr,
			int tabindex = -1, QString const panel = QString());
	/// check the widget and do visual marking
	bool check() const;
	/// reset all visual markings for tabs or panel sections
	void setSectionsValid() const;

private:
	// non-owned
	QLineEdit * input_;
	QWidget * target_;
	int tab_index_;
	QString panel_name_;
};


CheckedLineEdit::CheckedLineEdit(QLineEdit * input, QWidget * label,
				 int tabindex, QString const panel)
	: input_(input), target_(label), tab_index_(tabindex), panel_name_(panel)
{}


bool CheckedLineEdit::check() const
{
	if (!input_->isEnabled()) {
		// we do not check diabled widgets
		if (target_)
			setValid(target_, true);
		return true;
	}

	QValidator const * validator = input_->validator();
	if (!validator)
		return true;

	QString t = input_->text();
	int p = 0;
	bool const valid = validator->validate(t, p) == QValidator::Acceptable;

	// Visual feedback.
	setValid(input_, valid);
	if (target_) {
		if (!valid && !panel_name_.isEmpty() && qobject_cast<PanelStack*>(target_) != nullptr) {
			qobject_cast<PanelStack*>(target_)->markPanelValid(panel_name_, false);
			// this is a panel, so stop here.
			return valid;
		}
		setValid(target_, valid);
		if (!valid && tab_index_ >= 0 && qobject_cast<QTabWidget*>(target_) != nullptr) {
			QIcon warn(guiApp ? guiApp->getScaledPixmap("images/", "emblem-shellescape")
					  : getPixmap("images/", "emblem-shellescape", "svgz,png"));
			QTabBar * tb = qobject_cast<QTabWidget*>(target_)->tabBar();
			tb->setTabIcon(tab_index_, warn);
			tb->setTabToolTip(tab_index_, qt_("This tab contains invalid input. Please fix!"));
		}
	}

	return valid;
}


void CheckedLineEdit::setSectionsValid() const
{
	if (target_ && tab_index_ >= 0 && qobject_cast<QTabWidget*>(target_) != nullptr) {
		QTabBar * tb = qobject_cast<QTabWidget*>(target_)->tabBar();
		tb->setTabIcon(tab_index_, QIcon());
		tb->setTabToolTip(tab_index_, QString());
	}
	else if (!panel_name_.isEmpty() && qobject_cast<PanelStack*>(target_) != nullptr)
		qobject_cast<PanelStack*>(target_)->markPanelValid(panel_name_, true);
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

	Private() {}

	/// \return true if all CheckedWidgets are in a valid state.
	bool checkWidgets() const
	{
		bool valid = true;
		for (const CheckedLineEdit & w : checked_widgets_) {
			w.setSectionsValid();
		}
		for (const CheckedLineEdit & w : checked_widgets_)
			valid &= w.check();
		return valid;
	}

public:
	CheckedWidgetList checked_widgets_;

	QPushButton * okay_ = nullptr;
	QPushButton * apply_ = nullptr;
	QPushButton * cancel_ = nullptr;
	QPushButton * restore_ = nullptr;
	QCheckBox * auto_apply_ = nullptr;
	QPushButton * default_ = nullptr;

	typedef QList<QWidget *> Widgets;
	Widgets read_only_;

	ButtonPolicy policy_ {ButtonPolicy::IgnorantPolicy};
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
	if (d->default_)
		// Somewhere in the chain this can lose default status (#11417)
		d->default_->setDefault(true);
}


void ButtonController::addCheckedLineEdit(QLineEdit * input, QWidget * target, int tabindex)
{
	d->checked_widgets_.append(CheckedLineEdit(input, target, tabindex));
}


void ButtonController::addCheckedLineEditPanel(QLineEdit * input, QWidget * target, QString const panel)
{
	d->checked_widgets_.append(CheckedLineEdit(input, target, -1, panel));
}


void ButtonController::setOK(QPushButton * obj, bool const default_button)
{
	d->okay_ = obj;
	if (default_button)
		d->default_ = obj;
}


void ButtonController::setApply(QPushButton * obj, bool const default_button)
{
	d->apply_ = obj;
	if (default_button)
		d->default_ = obj;
}


void ButtonController::setAutoApply(QCheckBox * obj)
{
	d->auto_apply_ = obj;
}


void ButtonController::setCancel(QPushButton * obj, bool const default_button)
{
	d->cancel_ = obj;
	if (default_button)
		d->default_ = obj;
}


void ButtonController::setRestore(QPushButton * obj, bool const default_button)
{
	d->restore_ = obj;
	if (default_button)
		d->default_ = obj;
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
