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

class QCloseEvent;
class QShowEvent;

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
	explicit GuiDialog(LyXView & lv, std::string const & name);
	~GuiDialog();

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

public:
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

	void setViewTitle(docstring const & title);


	/// the dialog has changed contents
	virtual void changed();

	/// default: do nothing
	virtual void applyView() {}
	/// default: do nothing
	virtual void updateContents() {}
	///
	void closeEvent(QCloseEvent *);
	///
	void showEvent(QShowEvent *);

protected:
	/// Hide the dialog.
	virtual void hideView();
	/// Create the dialog if necessary, update it and display it.
	virtual void showView();
	///
	virtual bool isVisibleView() const;
	/// is the dialog currently valid ?
	virtual bool isValid() { return true; }

public:
	/** \name Container Access
	 *  These methods are publicly accessible because they are invoked
	 *  by the parent container acting on commands from the LyX kernel.
	 */
	//@{
	/// \param data is a string encoding of the data to be displayed.
	/// It is passed to the Controller to be translated into a useable form.
	void showData(std::string const & data);
	void updateData(std::string const & data);

	void hide();

	/** This function is called, for example, if the GUI colours
	 *  have been changed.
	 */
	void redraw() { redrawView(); }
	//@}

	/** When applying, it's useful to know whether the dialog is about
	 *  to close or not (no point refreshing the display for example).
	 */
	bool isClosing() const { return is_closing_; }

	/** \name Dialog Specialization
	 *  Methods to set the Controller and View and so specialise
	 *  to a particular dialog.
	 */
	//@{
	/// \param ptr is stored and destroyed by \c Dialog.
	void setController(Controller * ptr);
	//@}

	/** \name Dialog Components
	 *  Methods to access the various components making up a dialog.
	 */
	//@{
	virtual Controller & controller() { return *controller_; }
	//@}

	/** Defaults to nothing. Can be used by the Controller, however, to
	 *  indicate to the View that something has changed and that the
	 *  dialog therefore needs updating.
	 *  \param id identifies what should be updated.
	 */
	virtual void partialUpdateView(int /*id*/) {}

	///
	std::string name() const { return name_; }

	void apply();
	void redrawView() {}

private:
	/// Update the display of the dialog whilst it is still visible.
	virtual void updateView();

	ButtonController bc_;
	/// are we updating ?
	bool updating_;

	bool is_closing_;
	/** The Dialog's name is the means by which a dialog identifies
	 *  itself to the kernel.
	 */
	std::string name_;
	Controller * controller_;
	LyXView * lyxview_; // FIXME: replace by moving to constructor
};

} // namespace frontend
} // namespace lyx

#endif // GUIDIALOG_H
