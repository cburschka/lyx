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

#include "BiblioInfo.h"

#include <QAbstractListModel>
#include <QStandardItemModel>
#include <QStringList>
#include <QStringListModel>

namespace lyx {

class CitationStyle;

namespace frontend {

class FancyLineEdit;
class GuiSelectionManager;


class GuiCitation : public DialogView, public Ui::CitationUi
{
	Q_OBJECT

public:
	///
	GuiCitation(GuiView & lv);

private Q_SLOTS:
	void on_buttonBox_accepted();
	void on_buttonBox_rejected();
	void on_buttonBox_clicked(QAbstractButton *);
	void on_literalCB_clicked();
	void filterPressed();
	void filterChanged(const QString & text);
	void on_fieldsCO_currentIndexChanged(int index);
	void on_entriesCO_currentIndexChanged(int index);
	void on_citationStyleCO_currentIndexChanged(int index);
	void resetFilter();
	void caseChanged();
	void regexChanged();
	void instantChanged(bool checked);
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
	void applyView() override;
	void updateView() override {}
	bool initialiseParams(std::string const & data) override;
	void clearParams() override;
	void dispatchParams() override;
	bool isBufferDependent() const override { return true; }
	void saveSession(QSettings & settings) const override;
	void restoreSession() override;
	/** Disconnect from the inset when the Apply button is pressed.
	 *  Allows easy insertion of multiple citations.
	 */
	bool disconnectOnApply() const override { return true; }
	//@}

	///
	void showEvent(QShowEvent * e) override;
	///
	void closeEvent(QCloseEvent * e) override;
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
	void updateFormatting(CitationStyle const & currentStyle);
	///
	void updateControls(BiblioInfo const & bi);
	/// Set the appropriate hinting text on the filter bar
	void updateFilterHint();
	///
	void init();
	/// Clear selected keys
	void clearSelection();

	/// Set selected keys
	void setSelectedKeys(QStringList const &);
	/// Get selected keys
	QStringList selectedKeys();
	/// Set pre texts of qualified lists
	void setPreTexts(std::vector<docstring> const & m);
	/// Get pre texts of qualified lists
	std::vector<docstring> getPreTexts();
	/// Set post texts of qualified lists
	void setPostTexts(std::vector<docstring> const & m);
	/// Get post texts of qualified lists
	std::vector<docstring> getPostTexts();

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

	/// List of example cite strings and their correlating lyx name
	BiblioInfo::CiteStringMap citationStyles(BiblioInfo const & bi,
								size_t max_size);

	/// Set the Params variable for the Controller.
	void applyParams(int const choice, bool const full, bool const force,
					  QString before, QString after);

	///
	void filterByEntryType(BiblioInfo const & bi,
		std::vector<docstring> & keyVector, docstring const & entryType);

	/// Search a given string within the passed keys.
	/// \return the vector of matched keys.
	std::vector<docstring> searchKeys(
		BiblioInfo const & bi, //< optimize by passing this
		std::vector<docstring> const & keys_to_search, //< Keys to search.
		bool only_keys, //< whether to search only the keys
		docstring const & search_expression, //< Search expression (regex possible)
		docstring const & field, //< field to search, empty for all fields
		bool case_sensitive = false, //< set to true is the search should be case sensitive
		bool regex = false //< \set to true if \c search_expression is a regex
		); //

	/// The BibTeX information available to the dialog
	/// Calls to this method will lead to checks of modification times and
	/// the like, so it should be avoided.
	BiblioInfo const & bibInfo() const;

	/// contains the search box
	FancyLineEdit * filter_;

	/// Regexp action
	QAction * regexp_;
	/// Case sensitive action
	QAction * casesense_;
	/// Search as you type action
	QAction * instant_;

	/// last used citation style
	QString style_;
        /// this is the last style chosen in the current dialog
        QString last_chosen_style_;
	///
	GuiSelectionManager * selectionManager;
	/// available keys.
	QStringListModel available_model_;
	/// selected keys.
	QStandardItemModel selected_model_;
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
