// -*- C++ -*-
/**
 * \file GuiBibtex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIBIBTEX_H
#define GUIBIBTEX_H

#include "GuiDialog.h"
#include "GuiSelectionManager.h"
#include "ButtonController.h"
#include "ui_BibtexUi.h"

#include "insets/InsetCommandParams.h"

#include <QStandardItemModel>
#include <QStringListModel>

namespace lyx {
namespace frontend {

class FancyLineEdit;

class GuiBibtex : public GuiDialog, public Ui::BibtexUi
{
	Q_OBJECT

public:
	explicit GuiBibtex(GuiView & lv);

private Q_SLOTS:
	void change_adaptor();
	void on_buttonBox_accepted();
	void browseBstPressed();
	void browseBibPressed();
	void on_editPB_clicked();
	void databaseChanged();
	void rescanClicked();
	void selUpdated();
	void filterPressed();
	void filterChanged(const QString & text);
	void resetFilter();

private:
	/// Apply changes
	void applyView() override;
	/// update
	void updateContents() override;

	/// Browse for a .bib file
	QString browseBib(QString const & in_name) const;
	/// Browse for a .bst file
	QString browseBst(QString const & in_name) const;
	/// get the list of bst files
	QStringList bibStyles() const;
	/// get the list of bib files
	QStringList bibFiles(bool const extension = true) const;
	/// build filelists of all available bib/bst/cls/sty-files. done through
	/// kpsewhich and an external script, saved in *Files.lst
	void rescanBibStyles() const;
	/// do we use bibtopic (for sectioned bibliography)?
	bool usingBibtopic() const;
	/// should we put the bibliography to the TOC?
	bool bibtotoc() const;
	/// do we use biblatex?
	bool usingBiblatex() const;
	/// which stylefile do we use?
	QString styleFile() const;
	/// Clear selected keys
	void clearSelection();
	/// Set selected keys
	void setSelectedBibs(QStringList const &);
	/// prepares a call to GuiCitation::searchKeys when we
	/// are ready to search the Bib entries
	void findText(QString const & text);
	///
	void init();
	/// Get selected keys
	QStringList selectedBibs();
	///
	void setButtons();
	///
	std::vector<docstring> getFileEncodings();
	///
	void setFileEncodings(std::vector<docstring> const & m);

	///
	bool initialiseParams(std::string const & data) override;
	/// clean-up on hide.
	void clearParams() override { params_.clear(); }
	/// clean-up on hide.
	void dispatchParams() override;
	///
	bool isBufferDependent() const override { return true; }

private:
	///
	InsetCommandParams params_;
	///
	GuiSelectionManager * selectionManager;
	/// available keys.
	QStringListModel available_model_;
	/// selected keys.
	QStandardItemModel selected_model_;
	/// All keys.
	QStringList all_bibs_;
	/// Cited keys.
	QStringList selected_bibs_;
	/// contains the search box
	FancyLineEdit * filter_;
	///
	QMap<QString, QString> encodings_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIBIBTEX_H
