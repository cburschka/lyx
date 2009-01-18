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

class QWidget;
class QPushButton;
class QLineEdit;
class QCheckBox;

namespace lyx {
namespace frontend {

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
	~ButtonController();

	//@{
	/** Methods to set and get the ButtonPolicy.
	 *  \param ptr is owned by the ButtonController.
	 */
	void setPolicy(ButtonPolicy::Policy policy);
	ButtonPolicy const & policy() const;
	ButtonPolicy & policy();
	//@}

	///
	void input(ButtonPolicy::SMInput);

	//@{
	/// Tell the BC that a particular button has been pressed.
	void ok();
	void apply();
	void cancel();
	void restore();
	void autoApply();
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
	bool setReadOnly(bool);

	/** \param validity Tell the BC that the data is, or is not, valid.
	 *  Sets the activation state of the buttons immediately.
	 */
	void setValid(bool);

	//
	// View
	//

	//@{
	/** Store pointers to these widgets.
	 */
	void setOK(QPushButton * obj);
	void setApply(QPushButton * obj);
	void setCancel(QPushButton * obj);
	void setRestore(QPushButton * obj);
	void setAutoApply(QCheckBox * obj);
	//@}

	/** Add a pointer to the list of widgets whose activation
	 *  state is dependent upon the read-only status of the
	 *  underlying buffer.
	 */
	void addReadOnly(QWidget * obj);

	/** Add a widget to the list of all widgets whose validity should
	 *  be checked explicitly when the buttons are refreshed.
	 */
	void addCheckedLineEdit(QLineEdit * input, QWidget * label = 0);

private:
	/// noncopyable
	ButtonController(ButtonController const &);
	void operator=(ButtonController const &);
	
	/// pimpl
	class Private;
	Private * d;
};

} // namespace frontend
} // namespace lyx

#endif // BUTTONCONTROLLER_H
