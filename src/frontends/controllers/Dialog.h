// -*- C++ -*-
/**
 * \file Dialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 *
 * The dialogs use a Model-Controller-View split, instantiated here
 * by class Dialog.
 */

#ifndef DIALOG_H
#define DIALOG_H


#include "Kernel.h"
#include "LString.h"
#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>


class LyXView;
class ButtonControllerBase;


class Dialog : boost::noncopyable {
public:
	/** the Dialog's "name" is the means with which a dialog identifies
	 *  itself to the kernel.
	 */
	Dialog(LyXView &, string const & name);

	///
	string const & name() const { return name_; }

	/** These methods are publicly accessible because they are invoked
	    by the View.
	*/
	//@{
	///
	void ApplyButton();
	///
	void OKButton();
	///
	void CancelButton();
	///
	void RestoreButton();
	//@}

	/** These methods are publicly accessible because they are invoked
	 *  by the Dialogs class.
	 */
	//@{
	/** Some dialogs, eg the Tabular or Preferences dialog, can extract
	    the information they require from the kernel. These dialogs will
	    probably be passed an empty string by the calling Dialogs class.
	    The inset dialogs, however, require information specific to
	    an individual inset. This information will be encoded in "data"
	    and must be translated into a set of parameters that can be
	    updated from the dialog.
	 */
	void show(string const & data = string());
	///
	void update(string const & data = string());
	///
	void hide();
	///
	bool isVisible() const;
	/// (Eg, the GUI colours have been changed.)
	void redraw();
	//@}

	/** When Applying it's useful to know whether the dialog is about
	 *  to close or not (no point refreshing the display for example).
	 */
	bool isClosing() const { return is_closing_; }

	/// The LyX kernel is made available through this.
	Kernel & kernel() { return kernel_; }

	/** Different dialogs will have different
	    Controllers, Views and ButtonControllers.
	*/
	//@{
	///
	class Controller;
	///
	class View;

	///
	void setController(Controller *);
	///
	void setView(View *);
	///
	void setButtonController(ButtonControllerBase *);

	///
	Controller & controller() const;
	///
	ButtonControllerBase & bc() const;
	///
	View & view() const;
	//@}

private:
	///
	void apply();

	///
	bool is_closing_;
	///
	Kernel kernel_;
	///
	string name_;
	///
	boost::scoped_ptr<ButtonControllerBase> bc_ptr_;
	///
	boost::scoped_ptr<Controller> controller_ptr_;
	///
	boost::scoped_ptr<View> view_ptr_;
};


class Dialog::Controller : boost::noncopyable {
public:
	///
	Controller(Dialog & parent) : parent_(parent) {}
	///
	virtual ~Controller() {}
	///
	virtual void initialiseParams(string const & data) = 0;
	///
	virtual void clearParams() = 0;
	///
	virtual void dispatchParams() = 0;
	///
	virtual bool isBufferDependent() const = 0;
	///
	virtual bool disconnectOnApply() const { return false; }
	///
	Dialog & dialog() { return parent_; }
	///
	Dialog const & dialog() const { return parent_; }
	///
	Kernel & kernel() { return parent_.kernel(); }
	///
	Kernel const & kernel() const { return parent_.kernel(); }

private:
	///
	Dialog & parent_;
};


class Dialog::View : boost::noncopyable {
public:
	///
	View(Dialog & parent) : p_(parent) {}
	///
	virtual ~View() {}

	/// Apply changes to LyX data from dialog.
	virtual void apply() = 0;
	/// Hide the dialog.
	virtual void hide() = 0;
	/// Redraw the dialog (e.g. if the colors have been remapped).
	virtual void redraw() {}
	/// Create the dialog if necessary, update it and display it.
	virtual void show() = 0;
	/// Update dialog before/whilst showing it.
	virtual void update() = 0;
	///
	virtual bool isVisible() const = 0;

	/** Defaults to nothing. Can be used by the controller, however, to
	 *  indicate to the view that something has changed and that the
	 *  dialog therefore needs updating.
	 */
	virtual void partialUpdate(int) {}

	///
	Dialog & dialog() { return p_; }
	///
	Dialog const & dialog() const { return p_; }

	///
	Kernel & kernel() { return p_.kernel(); }
	///
	Kernel const & kernel() const { return p_.kernel(); }

	///
	Controller & getController() { return p_.controller(); }
	///
	Controller const & getController() const { return p_.controller(); }

protected:
	///
	Dialog & p_;
};


#endif // DIALOG_H
