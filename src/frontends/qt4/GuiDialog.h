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


namespace lyx {
namespace frontend {

/// Base class for historical LyX dialogs.
/** 
  * \warning New dialogs should use the leaner classes \c DialogView or
  * \c DockView depending on the intent. Eventally, old dialog should be
  * converted to \c DialogView too.
  */
class GuiDialog : public QDialog, public Dialog
{
	Q_OBJECT

public:
	/// \param lv is the access point for the dialog to the LyX kernel.
	/// \param name is the identifier given to the dialog by its parent
	/// container.
	/// \param title is the window title used for decoration.
	GuiDialog(GuiView & lv, QString const & name, QString const & title);

	virtual QWidget * asQWidget() { return this; }
	virtual QWidget const * asQWidget() const { return this; }

public Q_SLOTS:
	/** \name Buttons
	 *  These methods are publicly accessible because they are invoked
	 *  by the View when the user presses... guess what ;-)
	 */
	// Restore button clicked
	void slotRestore();
	// OK button clicked
	void slotOK();
	// Apply button clicked
	void slotApply();
	// Close button clicked or closed from WindowManager
	void slotClose();
	///
	void closeEvent(QCloseEvent * e);

public:
	/** Check whether we may apply our data.
	 *
	 *  The buttons are disabled if not and (re-)enabled if yes.
	 */
	void setButtonsValid(bool valid);

	/** \name Dialog Components
	 *  Methods to access the various components making up a dialog.
	 */
	//@{
	ButtonController const & bc() const { return bc_; }
	ButtonController & bc() { return bc_; }
	//@}

	/// the dialog has changed contents
	virtual void changed();

	virtual void enableView(bool enable);

	/// default: do nothing
	virtual void applyView() {}
	/// default: do nothing
	virtual void updateContents() {}

public:
	/// is the dialog currently valid ?
	virtual bool isValid() { return true; }

public:

	/** When applying, it's useful to know whether the dialog is about
	 *  to close or not (no point refreshing the display for example).
	 */
	bool isClosing() const { return is_closing_; }

	/// Update the display of the dialog whilst it is still visible.
	virtual void updateView();

private:
	ButtonController bc_;
	/// are we updating ?
	bool updating_;

	bool is_closing_;
};


} // namespace frontend
} // namespace lyx

#endif // GUIDIALOG_H
