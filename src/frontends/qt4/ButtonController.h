// -*- C++ -*-
/**
 * \file ButtonController.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BUTTONCONTROLLER_H
#define BUTTONCONTROLLER_H

#include "ButtonPolicy.h"
#include "gettext.h"

class QWidget;
class QPushButton;
class QLineEdit;

namespace lyx {
namespace frontend {

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

/** General purpose button controller for up to four buttons.
    Controls the activation of the OK, Apply and Cancel buttons.
    Actually supports 4 buttons in all and it's up to the user to decide on
    the activation policy and which buttons correspond to which output of the
    state machine.
*/


/** \c ButtonController controls the activation of the OK, Apply and
 *  Cancel buttons.
 *
 * It actually supports 4 buttons in all and it's up to the user to decide on
 * the activation policy and which buttons correspond to which output of the
 * state machine.
 */

class ButtonController
{
public:
	ButtonController();

	//@{
	/** Methods to set and get the ButtonPolicy.
	 *  \param ptr is owned by the ButtonController.
	 */
	void setPolicy(ButtonPolicy::Policy policy);
	ButtonPolicy const & policy() const { return policy_; }
	ButtonPolicy & policy() { return policy_; }
	//@}

	///
	void input(ButtonPolicy::SMInput);

	//@{
	/// Tell the BC that a particular button has been pressed.
	void ok();
	void apply();
	void cancel();
	void restore();
	//@}

	/// Tell the BC that the dialog is being hidden
	void hide();

	/**Refresh the activation state of the Ok, Apply, Close and
	 * Restore buttons.
	 */
	void refresh() const;

	/** Refresh the activation state of all the widgets under the control
	 *  of the BC to reflect the read-only status of the underlying buffer.
	 */
	void refreshReadOnly() const;

	/** Passthrough function -- returns its input value
	 *  Tell the BC about the read-only status of the underlying buffer.
	 */
	bool setReadOnly(bool = true);

	/** \param validity Tell the BC that the data is, or is not, valid.
	 *  Sets the activation state of the buttons immediately.
	 */
	void setValid(bool = true);

	//
	// View
	//

	//@{
	/** Store pointers to these widgets.
	 */
	void setOK(QPushButton * obj) { okay_ = obj; }
	void setApply(QPushButton * obj) { apply_ = obj; }
	void setCancel(QPushButton * obj) { cancel_ = obj; }
	void setRestore(QPushButton * obj) { restore_ = obj; }
	//@}

	/** Add a pointer to the list of widgets whose activation
	 *  state is dependent upon the read-only status of the
	 *  underlying buffer.
	 */
	void addReadOnly(QWidget * obj) { read_only_.push_back(obj); }

	/** Add a widget to the list of all widgets whose validity should
	 *  be checked explicitly when the buttons are refreshed.
	 */
	void addCheckedLineEdit(QLineEdit * input, QWidget * label = 0);

protected:
	/// \return true if all CheckedWidgets are in a valid state.
	bool checkWidgets() const;

private:
	typedef std::list<CheckedLineEdit> CheckedWidgetList;
	CheckedWidgetList checked_widgets;

private:
	/// Updates the widget sensitivity (enabled/disabled)
	void setWidgetEnabled(QWidget *, bool enabled) const;

	QPushButton * okay_;
	QPushButton * apply_;
	QPushButton * cancel_;
	QPushButton * restore_;

	typedef std::list<QWidget *> Widgets;
	Widgets read_only_;

private:
	ButtonPolicy policy_;
};

} // namespace frontend
} // namespace lyx

#endif // BUTTONCONTROLLER_H
