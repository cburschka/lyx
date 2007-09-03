// -*- C++ -*-
/**
 * \file GuiCitation.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Kalle Dalheimer
 * \author Abdelrazak Younes
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUICITATION_H
#define GUICITATION_H

#include "GuiDialog.h"
#include "GuiSelectionManager.h"
#include "ui_CitationUi.h"
#include "ControlCitation.h"
#include "support/docstring.h"

#include <QKeyEvent>
#include <QStringList>
#include <QStringListModel>

namespace lyx {
namespace frontend {

class GuiCitation;

class GuiCitationDialog : public QDialog,
	public Ui::CitationUi, public Dialog::View
{
	Q_OBJECT

public:
	GuiCitationDialog(Dialog &, GuiCitation * form );

	virtual ~GuiCitationDialog();

	virtual void applyView();

	/// Hide the dialog from sight
	void hideView();

	/// Redraw the dialog (e.g. if the colors have been remapped).
	void redrawView() {}

	/// Create the dialog if necessary, update it and display it.
	void showView();

	/// \return true if the dialog is visible.
	bool isVisibleView() const;

public Q_SLOTS:
	/// Update the display of the dialog whilst it is still visible.
	void updateView();

protected:
	void closeEvent(QCloseEvent * e);
	/// prepares a call to GuiCitation::searchKeys when we
	/// are ready to search the BibTeX entries
	void findText(QString const & text, bool reset = false);
	/// check whether key is already selected
	bool isSelected(const QModelIndex &);
	/// update the display of BibTeX information
	void updateInfo(QModelIndex const &);

protected Q_SLOTS:
	void cleanUp();
	void on_okPB_clicked();
	void on_cancelPB_clicked();
	void on_restorePB_clicked();
	void on_applyPB_clicked();
	void on_findLE_textChanged(const QString & text);
	void on_fieldsCO_currentIndexChanged(int index);
	void on_entriesCO_currentIndexChanged(int index);
	void on_caseCB_stateChanged(int);
	void on_regexCB_stateChanged(int);
	virtual void changed();
	///
	void setCitedKeys();
	/// performs a limited update, suitable for internal call
	void updateDialog();
	
private:
	/// enable/disable buttons
	void setButtons();
	/// fill the styles combo
	void fillStyles();
	/// fill the fields combo
	void fillFields();
	/// fill the entries combo
	void fillEntries();
	/// set the styles combo
	void updateStyle();
	/// last used citation style
	int style_;
	
	GuiCitation * form_;

	GuiSelectionManager * selectionManager;
};


class GuiCitation : public ControlCitation
{
public:
	///
	GuiCitation(GuiDialog &);
	virtual ~GuiCitation() {}
	virtual bool initialiseParams(std::string const & data);

	///
	void init();

	/// Available keys
	QStringListModel * available() { return &available_model_; }

	/// Selected keys
	QStringListModel * selected() { return &selected_model_; }

	/// Text before cite
	QString textBefore();

	/// Text after cite
	QString textAfter();

	/// Get key description
	QString getKeyInfo(QString const &);

	/// Clear selected keys
	void clearSelection();
	
	/// Return a list of available fields 
	QStringList getFieldsAsQStringList();
	
	/// Return a list of available fields 
	QStringList getEntriesAsQStringList();
	
	/// Find keys containing a string.
	void findKey(
		QString const & str, //< string expression
		bool only_keys, //< set to true if only keys shall be searched.
		docstring field, //<field to search, empty for all fields
		docstring entryType, //<entry type to display, empty for all
		bool case_sensitive, //< set to true for case sensitive search.
		bool reg_exp, //< set to true if \c str is a regular expression.
		bool reset = false //< whether to reset and search all keys
		);

	/// List of example cite strings
	QStringList citationStyles(int);

	/// Set the Params variable for the Controller.
	virtual void apply(int const choice, bool const full, bool const force,
					  QString before, QString after);
	
	void setCitedKeys();

private:
	/// available keys.
	QStringListModel available_model_;
	/// selected keys.
	QStringListModel selected_model_;
	/// All keys.
	QStringList all_keys_;
	/// Cited keys.
	QStringList cited_keys_;
};


} // namespace frontend
} // namespace lyx

#endif // GUICITATION_H
