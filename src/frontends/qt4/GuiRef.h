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

#include "GuiDialog.h"
#include "ui_RefUi.h"
#include "FancyLineEdit.h"

#include "insets/InsetCommandParams.h"

#include <vector>

class QTreeWidgetItem;

namespace lyx {
namespace frontend {

class GuiRef : public GuiDialog, public Ui::RefUi
{
	Q_OBJECT

public:
	GuiRef(GuiView & lv);
	/// Dialog inherited methods
	//@{
	void enableView(bool enable);
	//@}

private Q_SLOTS:
	void changed_adaptor();
	void gotoClicked();
	void filterLabels();
	void refHighlighted(QTreeWidgetItem *);
	void selectionChanged();
	void refTextChanged(QString const &);
	void refSelected(QTreeWidgetItem *);
	void sortToggled();
	void groupToggled();
	void updateClicked();
	void resetDialog();
	void dialogRejected();

private:
	///
	bool isBufferDependent() const { return true; }
	/** disconnect from the inset when the Apply button is pressed.
	 Allows easy insertion of multiple references. */
	bool disconnectOnApply() const { return true; }
	///
	void gotoRef(std::string const &);
	///
	void gotoBookmark();
	///
	void closeEvent(QCloseEvent * e);
	///
	bool isValid();
	/// apply changes
	void applyView();
	/// update dialog
	void updateContents();

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
	///
	bool initialiseParams(std::string const & data);
	/// clean-up on hide.
	void clearParams() { params_.clear(); }
	/// clean-up on hide.
	void dispatchParams();

private:
	///
	InsetCommandParams params_;

	/// contains the search box
	FancyLineEdit * filter_;

	/// went to a reference ?
	bool at_ref_;
	/// the last reference entered or examined
	QString last_reference_;
	/// store the buffer settings
	int restored_buffer_;
	/// store the last active buffer
	int active_buffer_;
	/// the references
	std::vector<docstring> refs_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIREF_H
