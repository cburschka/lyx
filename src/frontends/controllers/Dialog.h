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


/** \class Dialog collects the different parts of a Model-Controller-View
 *  split of a generic dialog together.
 */
class Dialog : boost::noncopyable {
public:
	/** \param name is the identifier given to the dialog by its parent
	 *  container.
	 */
	Dialog(LyXView &, string const & name);
	~Dialog();

	/** the Dialog's name is the means by which a dialog identifies
	 *  itself to the kernel.
	 */
	string const & name() const { return name_; }

	//@{
	/** These methods are publicly accessible because they are invoked
	 *  by the View when the user presses... guess what ;-)
	 */
	void ApplyButton();
	void OKButton();
	void CancelButton();
	void RestoreButton();
	//@}

	//@{
	/** These methods are publicly accessible because they are invoked
	 *  by the parent container acting on commands from the kernel.
	 */
	/** \param data The dialog is passed a string encoding the data
	 *  that it is to display. This string is passed to the Controller
	 *  which translates it into a useable form.
	 */
	void show(string const & data);
	/// \param data \see show().
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

	//@{
	/** Different dialogs will have different Controllers and Views.
	 * deriving from these base classes.
	 */
	class Controller;
	class View;
	//@}

	//@{
	/** Methods to set the Controller and View and so specialise
	 *  to a particular dialog.
	 *  \param ptr is stored here.
	 */
	void setController(Controller * ptr);
	void setView(View * ptr);
	//@}

	//@{
	/// Get methods for the various components making up a dialog.
	Controller & controller() const;
	ButtonController & bc() const;
	View & view() const;
	//@}

private:
	/// Invoked by both OKButton() and ApplyButton().
	void apply();

	bool is_closing_;
	Kernel kernel_;
	string name_;
	boost::scoped_ptr<ButtonController> bc_ptr_;
	boost::scoped_ptr<Controller> controller_ptr_;
	boost::scoped_ptr<View> view_ptr_;
};


/** \class Dialog::Controller is an abstract base class for the Controller
 *  of a Model-Controller-View split of a generic dialog.
 */
class Dialog::Controller : boost::noncopyable {
public:
	Controller(Dialog & parent) : parent_(parent) {}
	virtual ~Controller() {}

	//@{
	/** These few methods are all that a generic dialog needs of a
	 *  controller.
	 */
	/** \param data The controller is passed a string encoding of the
	 *  parameters that the dialog is to display.
	 *  \return true if the translation was successful.
	 */
	virtual bool initialiseParams(string const & data) = 0;
	/** Invoked by Dialog::hide, allowing the controller to
	 *  clean up its data structures.
	 */
	virtual void clearParams() = 0;
	/** Invoked by Dialog::apply, enabling the Controller to
	 *  dispatch its data back to the LyX kernel.
	 */
	virtual void dispatchParams() = 0;
	/** \return true if the dialog should be shown only when
	 *  a buffer is open
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
	//@{
	/** Enable the derived classes to access the other parts of the
	 * whole.
	 */
	Dialog & dialog() { return parent_; }
	Dialog const & dialog() const { return parent_; }

	Kernel & kernel() { return parent_.kernel(); }
	Kernel const & kernel() const { return parent_.kernel(); }
	//@}

private:
	Dialog & parent_;
};


/** \class Dialog::View is an abstract base class to the View
 *  of a Model-Controller-View split of a generic dialog.
 */
class Dialog::View : boost::noncopyable {
public:
	View(Dialog & parent) : p_(parent) {}
	virtual ~View() {}

	//@{
	/** These few methods are all that a generic dialog needs of a
	 *  view.
	 */
	/** A request to modify the data structures stored by the
	 *  accompanying Controller in preparation for their dispatch to
	 *  the LyX kernel.
	 *  Invoked by Dialog::apply.
	 */
	virtual void apply() = 0;
	/** Hide the dialog from sight
	 *  Invoked by Dialog::hide.
	 */
	virtual void hide() = 0;
	/** Redraw the dialog (e.g. if the colors have been remapped).
	 *  Invoked by Dialog::redraw.
	 */
	virtual void redraw() {}
	/** Create the dialog if necessary, update it and display it.
	 *  Invoked by Dialog::show.
	 */
	virtual void show() = 0;
	/** Update the display of the dialog whilst it is still visible.
	 *  Invoked by Dialog::update.
	 */
	virtual void update() = 0;
	/// \return true if the dialog is visible.
	virtual bool isVisible() const = 0;
	//@}

	/** Defaults to nothing. Can be used by the Controller, however, to
	 *  indicate to the View that something has changed and that the
	 *  dialog therefore needs updating.
	 */
	virtual void partialUpdate(int) {}

	//@{
	/** Enable the derived classes to access the other parts of the
	 * whole.
	 */
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
	///
	Dialog & p_;
};


#endif // DIALOG_H
