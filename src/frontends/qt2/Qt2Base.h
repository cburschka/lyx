// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 * ======================================================
 *
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef QT2BASE_H
#define QT2BASE_H

class QDialog;

#include <qfont.h>
#include <qobject.h>

#include <boost/smart_ptr.hpp>

#ifdef __GNUG__
#pragma interface
#endif

#include "ViewBase.h"
#include "LString.h"
#include "ButtonPolicies.h"

class qt2BC;

/** This class is an Qt2 GUI base class.
 */
class Qt2Base : public QObject, public ViewBC<qt2BC>
{
    Q_OBJECT
public:
	///
	Qt2Base(ControlButton &, string const &);
	///
	virtual ~Qt2Base() {}

protected:
	/// Build the dialog
	virtual void build() = 0;
	/// Hide the dialog.
	void hide();
	/// Create the dialog if necessary, update it and display it.
	void show();

protected slots:
    // dialog closed from WM
    void slotWMHide();

    // Apply button clicked
    void slotApply();

    // OK button clicked
    void slotOK();

    // Cancel button clicked
    void slotCancel();

    // Restore button clicked
    void slotRestore();

private:
	/// Pointer to the actual instantiation of xform's form
	virtual QDialog* form() const = 0;
 	/** Filter the inputs on callback from xforms
 	    Return true if inputs are valid. */
 	virtual ButtonPolicy::SMInput input(QWidget*, long);

private:
	/// dialog title, displayed by WM.
  	string title_;
};


template <class Dialog>
class Qt2DB: public Qt2Base
{
protected:
	///
	Qt2DB(ControlButton &, string const &);
	/// Pointer to the actual instantiation of the Qt dialog
	virtual QDialog* form() const;
	/// Real GUI implementation.
	boost::scoped_ptr<Dialog> dialog_;
};


template <class Dialog>
Qt2DB<Dialog>::Qt2DB(ControlButton & c, string const & t)
	: Qt2Base(c, t)
{}


template <class Dialog>
QDialog* Qt2DB<Dialog>::form() const
{
    return dialog_.get();
}


template <class Controller, class Base>
class Qt2CB: public Base
{
protected:
	///
	Qt2CB(ControlButton &, string const &);
	/// The parent controller
	Controller & controller() const;
};


template <class Controller, class Base>
Qt2CB<Controller, Base>::Qt2CB(ControlButton & c, string const & t)
	: Base(c, t)
{}


template <class Controller, class Base>
Controller & Qt2CB<Controller, Base>::controller() const
{
	return static_cast<Controller &>(controller_);
	//return dynamic_cast<Controller &>(controller_);
}


#endif // FORMBASE_H
