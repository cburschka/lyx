// -*- C++ -*-
/**
 * \file QDialogView.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QDIALOGVIEW_H
#define QDIALOGVIEW_H


#include "Dialog.h"
#include <boost/scoped_ptr.hpp>

#include <qfont.h>
#include <qdialog.h>
#include <qobject.h>
#include <qapplication.h>

class Qt2BC;

/** This class is an Qt2 GUI base class.
 */
class QDialogView : public QObject, public Dialog::View {
	Q_OBJECT
public:
	///
	QDialogView(Dialog &, QString const &);
	///
	virtual ~QDialogView() {}
	///
	bool readOnly() const;

protected:
	/// build the actual dialog
	virtual void build_dialog() = 0;
	///
	virtual void build() = 0;
	/// Hide the dialog.
	virtual void hide();
	/// Create the dialog if necessary, update it and display it.
	virtual void show();
	/// update the dialog's contents
	virtual void update_contents() = 0;
	///
	virtual bool isVisible() const;

	/// the dialog has changed contents
	virtual void changed();

	/// is the dialog currently valid ?
	virtual bool isValid();

	///
	Qt2BC & bc();

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
	/// Pointer to the actual instantiation of the Qt dialog
	virtual QDialog * form() const = 0;

private:
	/// dialog title, displayed by WM.
	QString title_;
};


template <class GUIDialog>
class QView: public QDialogView {
protected:
	QView(Dialog &, QString const &);

	/// update the dialog
	virtual void update();

	/// Build the dialog
	virtual void build();

	/// Pointer to the actual instantiation of the Qt dialog
	virtual QDialog * form() const;

	/// Real GUI implementation.
	boost::scoped_ptr<GUIDialog> dialog_;

};


template <class GUIDialog>
QView<GUIDialog>::QView(Dialog & p, QString const & t)
	: QDialogView(p, t)
{}


template <class GUIDialog>
QDialog * QView<GUIDialog>::form() const
{
	return dialog_.get();
}


template <class GUIDialog>
void QView<GUIDialog>::update()
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


template <class GUIDialog>
void QView<GUIDialog>::build()
{
	// protect the BC from unwarranted state transitions

	qApp->processEvents();
	updating_ = true;
	build_dialog();
	qApp->processEvents();
	updating_ = false;
}


template <class Controller, class Base>
class QController: public Base
{
protected:
	///
	QController(Dialog &, QString const &);
public:
	/// The parent controller
	Controller & controller();
	/// The parent controller
	Controller const & controller() const;
};


template <class Controller, class Base>
QController<Controller, Base>::QController(Dialog & p, QString const & t)
	: Base(p, t)
{}


template <class Controller, class Base>
Controller & QController<Controller, Base>::controller()
{
	return static_cast<Controller &>(getController());
}


template <class Controller, class Base>
Controller const & QController<Controller, Base>::controller() const
{
	return static_cast<Controller const &>(getController());
}


#endif // QDIALOGVIEW_H
