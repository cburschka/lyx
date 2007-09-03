// -*- C++ -*-
/**
 * \file GuiRef.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIREF_H
#define GUIREF_H

#include "GuiDialogView.h"
#include "ControlRef.h"
#include "ui_RefUi.h"

#include <QDialog>

#include <vector>

class QListWidgetItem;

namespace lyx {
namespace frontend {

class GuiRef;

class GuiRefDialog : public QDialog, public Ui::RefUi {
	Q_OBJECT
public:
	GuiRefDialog(GuiRef * form);

	virtual void show();

public Q_SLOTS:
	void changed_adaptor();
	void gotoClicked();
	void refHighlighted(QListWidgetItem *);
	void selectionChanged();
	void refSelected(QListWidgetItem *);
	void sortToggled(bool);
	void updateClicked();

protected Q_SLOTS:
	void reset_dialog();
protected:
	void closeEvent(QCloseEvent * e);
private:
	GuiRef * form_;
};


class GuiRef : public GuiView<GuiRefDialog>
{
public:
	friend class GuiRefDialog;

	GuiRef(GuiDialog &);

	/// parent controller
	ControlRef & controller()
	{ return static_cast<ControlRef &>(this->getController()); }
	/// parent controller
	ControlRef const & controller() const
	{ return static_cast<ControlRef const &>(this->getController()); }
protected:
	virtual bool isValid();

private:
	/// apply changes
	virtual void apply();
	/// build dialog
	virtual void build_dialog();
	/// update dialog
	virtual void update_contents();

	/// is name allowed for this ?
	bool nameAllowed();
	/// is type allowed for this ?
	bool typeAllowed();
	/// go to current reference
	void gotoRef();
	/// set go back button
	void setGoBack();
	/// set goto ref button
	void setGotoRef();
	/// re-enter references
	void redoRefs();
	/// update references
	void updateRefs();

	/// sort or not persistent state
	bool sort_;
	/// went to a reference ?
	bool at_ref_;
	/// the last reference entered or examined
	QString last_reference_;
	/// store the buffer settings
	int restored_buffer_;
	/// the references
	std::vector<docstring> refs_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIREF_H
