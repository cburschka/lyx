// -*- C++ -*-
/**
 * \file GuiDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIDIALOG_H
#define GUIDIALOG_H

#include "Dialog.h"
#include "ButtonController.h"

#include <QDialog>
#include <QObject>

namespace lyx {
namespace frontend {

/** \c Dialog collects the different parts of a Model-Controller-View
 *  split of a generic dialog together.
 */
class GuiDialog : public QDialog, public Dialog
{
	Q_OBJECT

public:
	/// \param lv is the access point for the dialog to the LyX kernel.
	/// \param name is the identifier given to the dialog by its parent
	/// container.
	GuiDialog(LyXView & lv, std::string const & name);
	//GuiDialog(GuiDialog &, docstring const &);

public Q_SLOTS:
	// dialog closed from WM
	void slotWMHide();
	// Restore button clicked
	void slotRestore();
	// OK button clicked
	void slotOK();
	// Apply button clicked
	void slotApply();
	// Close button clicked
	void slotClose();

public:
	/** \name Buttons
	 *  These methods are publicly accessible because they are invoked
	 *  by the View when the user presses... guess what ;-)
	 */
	//@{
	void ApplyButton();
	void OKButton();
	void CancelButton();
	void RestoreButton();
	//@}

	/** Check whether we may apply our data.
	 *
	 *  The buttons are disabled if not and (re-)enabled if yes.
	 */
	void checkStatus();
	void setButtonsValid(bool valid);

	/** \name Dialog Components
	 *  Methods to access the various components making up a dialog.
	 */
	//@{
	ButtonController const & bc() const { return bc_; }
	ButtonController & bc() { return bc_; }
	//@}

	void preShow();
	void postShow();
	void preUpdate();
	void postUpdate();

	///
	bool readOnly() const;

	/// the dialog has changed contents
	virtual void changed();

	/// default: do nothing
	virtual void applyView() {}
	/// default: do nothing
	virtual void update_contents() {}

protected:
	/// Hide the dialog.
	virtual void hideView();
	/// Create the dialog if necessary, update it and display it.
	virtual void showView();
	///
	virtual bool isVisibleView() const;
	/// is the dialog currently valid ?
	virtual bool isValid();

private:
	/// update the dialog
	virtual void updateView();

	ButtonController bc_;
	/// are we updating ?
	bool updating_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIDIALOG_H
