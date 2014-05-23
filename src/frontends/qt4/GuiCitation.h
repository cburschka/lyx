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

#include "DialogView.h"
#include "ui_CitationUi.h"

#include "insets/InsetCommandParams.h"

#include "Citation.h"

#include <QStringList>
#include <QStringListModel>

namespace lyx {

class BiblioInfo;

namespace frontend {

class GuiSelectionManager;

class GuiCitation : public DialogView, public Ui::CitationUi
{
	Q_OBJECT

public:
	///
	GuiCitation(GuiView & lv);
	///
	~GuiCitation();

private Q_SLOTS:
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
	void on_asTypeCB_stateChanged(int);
	void changed();
	/// set the citation keys, mark as changed
	void setCitedKeys();
	/// update the styles for the style combo, mark as changed
	void updateStyles();
	/// performs a limited update, suitable for internal call
	void updateControls();


private:
	/// Dialog inherited methods
	//@{
	void applyView();
	void updateView() {}
	bool initialiseParams(std::string const & data);
	void clearParams();
	void dispatchParams();
	bool isBufferDependent() const { return true; }
	void saveSession() const;
	void restoreSession();
	/** Disconnect from the inset when the Apply button is pressed.
	 *  Allows easy insertion of multiple citations.
	 */
	bool disconnectOnApply() const { return true; }
	//@}

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
	void updateInfo(BiblioInfo const & bi, QModelIndex const &);
	/// enable/disable buttons
	void setButtons();
	/// fill the fields combo
	void fillFields(BiblioInfo const & bi);
	/// fill the entries combo
	void fillEntries(BiblioInfo const & bi);
	/// set the styles combo
	void updateStyles(BiblioInfo const & bi);
	/// set the formatting widgets
	void updateFormatting(CitationStyle currentStyle);
	///
	void updateControls(BiblioInfo const & bi);
	///
	void init();
	/// Clear selected keys
	void clearSelection();

	/// Find keys containing a string.
	void findKey(
		BiblioInfo const & bi, //< optimize by passing this
		QString const & str, //< string expression
		bool only_keys, //< set to true if only keys shall be searched.
		docstring field, //<field to search, empty for all fields
		docstring entryType, //<entry type to display, empty for all
		bool case_sensitive, //< set to true for case sensitive search.
		bool reg_exp, //< set to true if \c str is a regular expression.
		bool reset = false //< whether to reset and search all keys
		);

	/// List of example cite strings
	QStringList citationStyles(BiblioInfo const & bi, size_t max_size);

	/// Set the Params variable for the Controller.
	void apply(int const choice, bool const full, bool const force,
					  QString before, QString after);

	///
	void filterByEntryType(BiblioInfo const & bi,
		std::vector<docstring> & keyVector, docstring entryType);

	/// Search a given string within the passed keys.
	/// \return the vector of matched keys.
	std::vector<docstring> searchKeys(
		BiblioInfo const & bi, //< optimize by passing this
		std::vector<docstring> const & keys_to_search, //< Keys to search.
		bool only_keys, //< whether to search only the keys
		docstring const & search_expression, //< Search expression (regex possible)
		docstring field, //< field to search, empty for all fields
		bool case_sensitive = false, //< set to true is the search should be case sensitive
		bool regex = false //< \set to true if \c search_expression is a regex
		); //

	/// The BibTeX information available to the dialog
	/// Calls to this method will lead to checks of modification times and
	/// the like, so it should be avoided.
	BiblioInfo const & bibInfo() const;

	/// last used citation style
	int style_;
	///
	GuiSelectionManager * selectionManager;
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
};

} // namespace frontend
} // namespace lyx

#endif // GUICITATION_H
