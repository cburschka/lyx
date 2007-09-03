// -*- C++ -*-
/**
 * \file GuiDialogView.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIDIALOGVIEW_H
#define GUIDIALOGVIEW_H

#include "GuiDialog.h"

#include <boost/scoped_ptr.hpp>

#include <QApplication>
#include <QWidget>
#include <QObject>

namespace lyx {
namespace frontend {

/** This class is an Qt2 GUI base class.
 */
class GuiDialogView : public QObject, public Dialog::View
{
	Q_OBJECT
public:
	///
	GuiDialogView(GuiDialog &, docstring const &);
	///
	virtual ~GuiDialogView() {}
	///
	bool readOnly() const;

	/// the dialog has changed contents
	virtual void changed();

	///
	ButtonController & bc();

protected:
	/// build the actual dialog
	virtual void build_dialog() = 0;
	/// Build the dialog
	virtual void build();
	/// Hide the dialog.
	virtual void hide();
	/// Create the dialog if necessary, update it and display it.
	virtual void show();
	/// update the dialog's contents
	virtual void update_contents() = 0;
	///
	virtual bool isVisible() const;
	/// is the dialog currently valid ?
	virtual bool isValid();

	/// are we updating ?
	bool updating_;
	///
	GuiDialog & parent_;

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

private:
	/// Pointer to the actual instantiation of the Qt dialog
	virtual QWidget * form() const = 0;
};


template <class GUIDialog>
class GuiView : public GuiDialogView {
protected:
	GuiView(GuiDialog & p, docstring const & t)
		: GuiDialogView(p, t)
	{}

	virtual ~GuiView() {}

	/// update the dialog
	virtual void update() {
		dialog_->setUpdatesEnabled(false);

		// protect the BC from unwarranted state transitions
		updating_ = true;
		update_contents();
		updating_ = false;

		dialog_->setUpdatesEnabled(true);
		dialog_->update();
	}

	/// Pointer to the actual instantiation of the Qt dialog
	virtual GUIDialog * form() const { return dialog_.get(); }

	/// Real GUI implementation.
	boost::scoped_ptr<GUIDialog> dialog_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIDIALOGVIEW_H
