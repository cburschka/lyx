// -*- C++ -*-
/**
 * \file Dialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef DIALOG_H
#define DIALOG_H


#include "Kernel.h"
#include "LString.h"
#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>


class LyXView;
class ButtonController;


/** \c Dialog collects the different parts of a Model-Controller-View
 *  split of a generic dialog together.
 */
class Dialog : boost::noncopyable {
public:
	/// \param lv is the access point for the dialog to the LyX kernel.
	/// \param name is the identifier given to the dialog by its parent
	/// container.
	Dialog(LyXView & lv, string const & name);
	~Dialog();

	/** The Dialog's name is the means by which a dialog identifies
	 *  itself to the kernel.
	 */
	string const & name() const { return name_; }

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

	/** \name Container Access
	 *  These methods are publicly accessible because they are invoked
	 *  by the parent container acting on commands from the LyX kernel.
	 */
	//@{
	/// \param data is a string encoding of the data to be displayed.
	/// It is passed to the Controller to be translated into a useable form.
	void show(string const & data);
	void update(string const & data);

	void hide();
	bool isVisible() const;

	/** This function is called, for example, if the GUI colours
	 *  have been changed.
	 */
	void redraw();
	//@}

	/** When applying, it's useful to know whether the dialog is about
	 *  to close or not (no point refreshing the display for example).
	 */
	bool isClosing() const { return is_closing_; }

	/** The LyX kernel is made available through this wrapper class.
	 *  In an ideal world, it will shrink as more info is passed to the
	 *  show() and update() methods.
	 */
	Kernel & kernel() { return kernel_; }

	/** Different dialogs will have different Controllers and Views.
	 *  deriving from these base classes.
	 */
	//@{
	class Controller;
	class View;
	//@}

	/** \name Dialog Specialization
	 *  Methods to set the Controller and View and so specialise
	 *  to a particular dialog.
	 */
	//@{
	/// \param ptr is stored and destroyed by \c Dialog.
	void setController(Controller * ptr);
	/// \param ptr is stored and destroyed by \c Dialog.
	void setView(View * ptr);
	//@}

	/** \name Dialog Components
	 *  Methods to access the various components making up a dialog.
	 */
	//@{
	Controller & controller() const;
	ButtonController & bc() const;
	View & view() const;
	//@}

private:
	void apply();

	bool is_closing_;
	Kernel kernel_;
	string name_;
	boost::scoped_ptr<ButtonController> bc_ptr_;
	boost::scoped_ptr<Controller> controller_ptr_;
	boost::scoped_ptr<View> view_ptr_;
};


/** \c Dialog::Controller is an abstract base class for the Controller
 *  of a Model-Controller-View split of a generic dialog.
 */
class Dialog::Controller : boost::noncopyable {
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
	virtual bool initialiseParams(string const & data) = 0;

	/// Enable the controller to clean up its data structures.
	virtual void clearParams() = 0;

	/// Enable the Controller to dispatch its data back to the LyX kernel.
	virtual void dispatchParams() = 0;

	/** \return true if the dialog should be shown only when
	 *  a buffer is open.
	 */
	virtual bool isBufferDependent() const = 0;

	/** \return true if the kernel should disconnect the dialog from
	 *  a particular inset after the data has been applied to it.
	 *  Clearly this makes sense only for dialogs modifying the contents
	 *  of an inset :-)
	 *  In practise, only a very few dialogs (e.g. the citation dialog)
	 *  return true.
	 */
	virtual bool disconnectOnApply() const { return false; }
	//@}

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
	Dialog & parent_;
};


/** \c Dialog::View is an abstract base class to the View
 *  of a Model-Controller-View split of a generic dialog.
 */
class Dialog::View : boost::noncopyable {
public:
	/** \param parent Dialog owning this Controller.
	 *  \param title  is the dialog title displayed by the WM.
	 */
	View(Dialog & parent, string title);
	virtual ~View() {}

	/** \name Generic View
	 *  These few methods are all that a generic dialog needs of a
	 *  view.
	 */
	//@{
	/** A request to modify the data structures stored by the
	 *  accompanying Controller in preparation for their dispatch to
	 *  the LyX kernel.
	 */
	virtual void apply() = 0;

	/// Hide the dialog from sight
	virtual void hide() = 0;

	/// Redraw the dialog (e.g. if the colors have been remapped).
	virtual void redraw() {}

	/// Create the dialog if necessary, update it and display it.
	virtual void show() = 0;

	/// Update the display of the dialog whilst it is still visible.
	virtual void update() = 0;

	/// \return true if the dialog is visible.
	virtual bool isVisible() const = 0;
	//@}

	/** Defaults to nothing. Can be used by the Controller, however, to
	 *  indicate to the View that something has changed and that the
	 *  dialog therefore needs updating.
	 *  \param id identifies what should be updated.
	 */
	virtual void partialUpdate(int id) {}

	/// sets the title of the dialog (window caption)
	void setTitle(string const &);
	/// gets the title of the dialog (window caption)
	string const & getTitle() const;

	/** \name View Access
	 *  Enable the derived classes to access the other parts of the whole.
	 */
	//@{
	Dialog & dialog() { return p_; }
	Dialog const & dialog() const { return p_; }

protected:
	Kernel & kernel() { return p_.kernel(); }
	Kernel const & kernel() const { return p_.kernel(); }

	Controller & getController() { return p_.controller(); }
	Controller const & getController() const { return p_.controller(); }

	ButtonController & bc() { return p_.bc(); }
	ButtonController const & bc() const { return p_.bc(); }
	//@}

private:
	Dialog & p_;
	string title_;
};


#endif // DIALOG_H
