// -*- C++ -*-
/**
 * \file Dialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef DIALOG_H
#define DIALOG_H

#include "Kernel.h"
#include "lfuns.h"

#include "support/docstring.h"
#include <boost/scoped_ptr.hpp>

namespace lyx {
namespace frontend {

class LyXView;

/** Different dialogs will have different Controllers and Views.
 *  deriving from these base classes.
 */
//@{
class Controller;
//@}

/** \c Dialog collects the different parts of a Model-Controller-View
 *  split of a generic dialog together.
 */
class Dialog 
{
public:
	/// \param lv is the access point for the dialog to the LyX kernel.
	/// \param name is the identifier given to the dialog by its parent
	/// container.
	Dialog(LyXView & lv, std::string const & name);
	virtual ~Dialog();

	/** \name Container Access
	 *  These methods are publicly accessible because they are invoked
	 *  by the parent container acting on commands from the LyX kernel.
	 */
	//@{
	/// \param data is a string encoding of the data to be displayed.
	/// It is passed to the Controller to be translated into a useable form.
	void show(std::string const & data);
	void update(std::string const & data);

	void hide();
	bool isVisible() const;

	// Override in GuiDialog
	virtual void preShow() {}
	virtual void postShow() {}
	virtual void preUpdate() {}
	virtual void postUpdate() {}

	virtual void OkButton() {}
	virtual void ApplyButton() {}
	virtual void CancelButton() {}
	virtual void RestoreButton() {}

	/** This function is called, for example, if the GUI colours
	 *  have been changed.
	 */
	void redraw();
	//@}

	/** Check whether we may apply our data.
	 *
	 *  The buttons are disabled if not and (re-)enabled if yes.
	 */
	void checkStatus();

	/** When applying, it's useful to know whether the dialog is about
	 *  to close or not (no point refreshing the display for example).
	 */
	bool isClosing() const { return is_closing_; }

	/** The LyX kernel is made available through this wrapper class.
	 *  In an ideal world, it will shrink as more info is passed to the
	 *  show() and update() methods.
	 */
	Kernel & kernel() { return kernel_; }
	Kernel const & kernel() const { return kernel_; }

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
	virtual Controller & controller() const;
	//@}

	/** \c Button controller part
	 */
	virtual void setButtonsValid(bool /*valid*/) {}


	/** \c View part
	 *  of a Model-Controller-View split of a generic dialog.
	 *  These few methods are all that a generic dialog needs of a
	 *  view.
	 */
	//@{
	/** A request to modify the data structures stored by the
	 *  accompanying Controller in preparation for their dispatch to
	 *  the LyX kernel.
	 */
	virtual void applyView() = 0;

	/// Hide the dialog from sight
	virtual void hideView() = 0;

	/// Redraw the dialog (e.g. if the colors have been remapped).
	virtual void redrawView() {}

	/// Create the dialog if necessary, update it and display it.
	virtual void showView() = 0;

	/// Update the display of the dialog whilst it is still visible.
	virtual void updateView() = 0;

	/// \return true if the dialog is visible.
	virtual bool isVisibleView() const = 0;
	//@}

	/** Defaults to nothing. Can be used by the Controller, however, to
	 *  indicate to the View that something has changed and that the
	 *  dialog therefore needs updating.
	 *  \param id identifies what should be updated.
	 */
	virtual void partialUpdateView(int /*id*/) {}

	/// sets the title of the dialog (window caption)
	void setViewTitle(docstring const &);
	/// gets the title of the dialog (window caption)
	docstring const & getViewTitle() const;
	///
	std::string name() const { return name_; }

private:
	/// intentionally unimplemented, therefore uncopiable
	Dialog(Dialog const &);
	void operator=(Dialog const &);

protected:
	void apply();

	bool is_closing_;
	Kernel kernel_;
	/** The Dialog's name is the means by which a dialog identifies
	 *  itself to the kernel.
	 */
	std::string name_;
	Controller * controller_;

	docstring title_;
};


/** \c Controller is an abstract base class for the Controller
 *  of a Model-Controller-View split of a generic dialog.
 */
class Controller 
{
public:
	/// \param parent Dialog owning this Controller.
	Controller(Dialog & parent);
	virtual ~Controller() {}

	/** \name Generic Controller
	 *  These few methods are all that a generic dialog needs of a
	 *  controller.
	 */
	//@{
	/** Enable the controller to initialise its data structures.
	 *  \param data is a string encoding of the parameters to be displayed.
	 *  \return true if the translation was successful.
	 */
	virtual bool initialiseParams(std::string const & data) = 0;

	/// Enable the controller to clean up its data structures.
	virtual void clearParams() = 0;

	/// Enable the Controller to dispatch its data back to the LyX kernel.
	virtual void dispatchParams() = 0;

	/** \return true if the dialog should be shown only when
	 *  a buffer is open.
	 */
	virtual bool isBufferDependent() const = 0;

	/** \return true if the dialog can apply data also
	 *  for ReadOnly buffers.
	 *  This has to be distinguished from isBufferDependent()
	 */
	virtual bool canApplyToReadOnly() const { return false; }

	/** The lfun that is sent for applying the data.
	 *
	 * This method is used by the default implementation of canApply()
	 * for buffer dependent dialogs that send one lfun when applying the
	 * data.
	 * It should be used in dispatchParams(), too for consistency reasons.
	 *  \returns the lfun that is sent for applying the data.
	 */
	virtual kb_action getLfun() const { return LFUN_INSET_APPLY; }

	/** Check whether we may apply our data.
	 *
	 * The default implementation works for all dialogs that send one
	 * lfun when applying the data. Dialogs that send none or more than
	 * one lfun need to reimplement it.
	 *  \returns whether the data can be applied or not.
	 */
	virtual bool canApply() const;

	/** \return true if the kernel should disconnect the dialog from
	 *  a particular inset after the data has been applied to it.
	 *  Clearly this makes sense only for dialogs modifying the contents
	 *  of an inset :-)
	 *  In practise, only a very few dialogs (e.g. the citation dialog)
	 *  return true.
	 */
	virtual bool disconnectOnApply() const { return false; }

	/** \return true if Dialog::View::show() should not display the dialog
	 *   after running update. Currently, only ControlSpellchecker
	 *   makes use of that.
	*/
	virtual bool exitEarly() const { return false; }
	//@}

	/// Main Window access.
	/// This is unfortunately needed for the qt4 frontend and the \c
	/// QDialogView framework. This permits to give a parent to the
	/// constructed \c QDialog via a cast to \c GuiView.
	LyXView * view() { return &parent_.kernel().lyxview(); }

protected:
	/** \name Controller Access
	 *  Enable the derived classes to access the other parts of the whole.
	 */
	//@{
	Dialog & dialog() { return parent_; }
	Dialog const & dialog() const { return parent_; }

	Kernel & kernel() { return parent_.kernel(); }
	Kernel const & kernel() const { return parent_.kernel(); }
	//@}

private:
	/// intentionally unimplemented, therefore uncopiable
	Controller(Controller const &);
	void operator=(Controller const &);

private:
	Dialog & parent_;
};



} // namespace frontend
} // namespace lyx

#endif // DIALOG_H
