// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *		   LyX, The Document Processor
 *
 *		   Copyright 2000 The LyX Team.
 *
 * ======================================================
 *
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef QT2BASE_H
#define QT2BASE_H

#include <config.h>

#include <qfont.h>
#include <qdialog.h>
#include <qobject.h>
#include <qapplication.h>

#ifdef __GNUG__
#pragma interface
#endif

#include "ViewBase.h"
#include "LString.h"
#include "debug.h"
#include "ButtonPolicies.h"
#include "ControlButtons.h"

#include <boost/smart_ptr.hpp>

class Qt2BC;

/** This class is an Qt2 GUI base class.
 */
class Qt2Base : public QObject, public ViewBC<Qt2BC>
{
	Q_OBJECT
public:
	///
	Qt2Base(ControlButtons &, QString const &);
	///
	virtual ~Qt2Base() {}

protected:
	/// build the actual dialog
	virtual void build_dialog() = 0;
	/// Hide the dialog.
	virtual void hide();
	/// Create the dialog if necessary, update it and display it.
	virtual void show();
	/// update the dialog's contents
	virtual void update_contents() = 0;

	/// the dialog has changed contents
	virtual void changed();

	/// is the dialog currently valid ?
	virtual bool isValid();

	/// are we updating ?
	bool updating_;

protected slots:
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

private:
	/// Pointer to the actual instantiation of xform's form
	virtual QDialog * form() const = 0;

private:
	/// dialog title, displayed by WM.
	QString title_;
};


template <class Dialog>
class Qt2DB: public Qt2Base
{
protected:
	Qt2DB(ControlButtons &, QString const &);

	/// update the dialog
	virtual void update();

	/// Build the dialog
	virtual void build();

	/// Pointer to the actual instantiation of the Qt dialog
	virtual QDialog * form() const;

	/// Real GUI implementation.
	boost::scoped_ptr<Dialog> dialog_;

};


template <class Dialog>
Qt2DB<Dialog>::Qt2DB(ControlButtons & c, QString const & t)
	: Qt2Base(c, t)
{}


template <class Dialog>
QDialog * Qt2DB<Dialog>::form() const
{
	return dialog_.get();
}


template <class Dialog>
void Qt2DB<Dialog>::update()
{
	form()->setUpdatesEnabled(false);

	// protect the BC from unwarranted state transitions

	qApp->processEvents();
	updating_ = true;
	update_contents();
	qApp->processEvents();
	updating_ = false;

	form()->setUpdatesEnabled(true);
	form()->update();
}


template <class Dialog>
void Qt2DB<Dialog>::build()
{
	// protect the BC from unwarranted state transitions

	qApp->processEvents();
	updating_ = true;
	build_dialog();
	qApp->processEvents();
	updating_ = false;
}


template <class Controller, class Base>
class Qt2CB: public Base
{
public:
	bool readOnly() const {
		return controller().isReadonly();
	}

protected:
	///
	Qt2CB(ControlButtons &, QString const &);
	/// The parent controller
	Controller & controller() const;
};


template <class Controller, class Base>
Qt2CB<Controller, Base>::Qt2CB(ControlButtons & c, QString const & t)
	: Base(c, t)
{}


template <class Controller, class Base>
Controller & Qt2CB<Controller, Base>::controller() const
{
	return static_cast<Controller &>(controller_);
	//return dynamic_cast<Controller &>(controller_);
}


#endif // FORMBASE_H
