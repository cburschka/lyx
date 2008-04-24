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

#include "BiblioInfo.h"
#include "Citation.h"

#include "insets/InsetCommandParams.h"

#include <QStringList>
#include <QStringListModel>

namespace lyx {
namespace frontend {

class GuiCitation : public GuiDialog, public Ui::CitationUi
{
	Q_OBJECT

public:
	///
	GuiCitation(GuiView & lv);

	///
	void applyView();

public Q_SLOTS:
	/// Update the display of the dialog whilst it is still visible.
	void updateView();

private:
	///
	void showEvent(QShowEvent * e);
	///
	void closeEvent(QCloseEvent * e);
	/// prepares a call to GuiCitation::searchKeys when we
	/// are ready to search the BibTeX entries
	void findText(QString const & text, bool reset = false);
	/// check whether key is already selected
	bool isSelected(const QModelIndex &);
	/// update the display of BibTeX information
	void updateInfo(QModelIndex const &);

private Q_SLOTS:
	void cleanUp();
	void on_okPB_clicked();
	void on_cancelPB_clicked();
	void on_restorePB_clicked();
	void on_applyPB_clicked();
	void on_searchPB_clicked();
	void on_findLE_textChanged(const QString & text);
	void on_fieldsCO_currentIndexChanged(int index);
	void on_entriesCO_currentIndexChanged(int index);
	void on_citationStyleCO_currentIndexChanged(int index);
	void on_caseCB_stateChanged(int);
	void on_regexCB_stateChanged(int);
	void changed();
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
	/// set the formatting widgets
	void updateFormatting(CiteStyle currentStyle);
	/// last used citation style
	int style_;
	
	GuiSelectionManager * selectionManager;

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
	void apply(int const choice, bool const full, bool const force,
					  QString before, QString after);
	///
	bool initialiseParams(std::string const & data);
	/// clean-up on hide.
	void clearParams();
	/// clean-up on hide.
	void dispatchParams();
	///
	bool isBufferDependent() const { return true; }

private:
	/// available keys.
	QStringListModel available_model_;
	/// selected keys.
	QStringListModel selected_model_;
	/// All keys.
	QStringList all_keys_;
	/// Cited keys.
	QStringList cited_keys_;
	///
	InsetCommandParams params_;

	/** Disconnect from the inset when the Apply button is pressed.
	 *  Allows easy insertion of multiple citations.
	 */
	bool disconnectOnApply() const { return true; }

	/// \return the list of all available bibliography keys.
	std::vector<docstring> availableKeys() const;
	/// \return the list of all used BibTeX fields
	std::vector<docstring> availableFields() const;
	/// \return the list of all used BibTeX entry types
	std::vector<docstring> availableEntries() const;
	///
	void filterByEntryType(
		std::vector<docstring> & keyVector, docstring entryType);
	///
	CiteEngine citeEngine() const;

	/// \return information for this key.
	docstring getInfo(docstring const & key) const;

	/// Search a given string within the passed keys.
	/// \return the vector of matched keys.
	std::vector<docstring> searchKeys(
		std::vector<docstring> const & keys_to_search, //< Keys to search.
		bool only_keys, //< whether to search only the keys
		docstring const & search_expression, //< Search expression (regex possible)
		docstring field, //< field to search, empty for all fields
		bool case_sensitive = false, //< set to true is the search should be case sensitive
		bool regex = false //< \set to true if \c search_expression is a regex
		); //

private:
	/// The BibTeX information available to the dialog
	BiblioInfo bibkeysInfo_;
};

} // namespace frontend
} // namespace lyx

#endif // GUICITATION_H
