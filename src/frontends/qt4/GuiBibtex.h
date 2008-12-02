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
#include "ButtonController.h"
#include "ui_BibtexUi.h"
#include "ui_BibtexAddUi.h"

#include "insets/InsetCommandParams.h"


namespace lyx {
namespace frontend {

class GuiBibtexAddDialog : public QDialog, public Ui::BibtexAddUi
{
public:
	GuiBibtexAddDialog(QWidget * parent) : QDialog(parent)
	{
		Ui::BibtexAddUi::setupUi(this);
		QDialog::setModal(true);
	}
};


class GuiBibtex : public GuiDialog, public Ui::BibtexUi
{
	Q_OBJECT

public:
	explicit GuiBibtex(GuiView & lv);

private Q_SLOTS:
	void change_adaptor();
	void browsePressed();
	void browseBibPressed();
	void addPressed();
	void addDatabase();
	void deletePressed();
	void upPressed();
	void downPressed();
	void databaseChanged();
	void availableChanged();
	void bibEDChanged();
	void rescanClicked();

private:
	///
	bool isValid();
	/// Apply changes
	void applyView();
	/// update
	void updateContents();

	/// Browse for a .bib file
	QString browseBib(QString const & in_name) const;
	/// Browse for a .bst file
	QString browseBst(QString const & in_name) const;
	/// get the list of bst files
	QStringList bibStyles() const;
	/// get the list of bib files
	QStringList bibFiles() const;
	/// build filelists of all availabe bib/bst/cls/sty-files. done through
	/// kpsewhich and an external script, saved in *Files.lst
	void rescanBibStyles() const;
	/// do we use bibtopic (for sectioned bibliography)?
	bool usingBibtopic() const;
	/// should we put the bibliography to the TOC?
	bool bibtotoc() const;
	/// which stylefile do we use?
	QString styleFile() const;

	///
	bool initialiseParams(std::string const & data);
	/// clean-up on hide.
	void clearParams() { params_.clear(); }
	/// clean-up on hide.
	void dispatchParams();
	///
	bool isBufferDependent() const { return true; }

private:
	///
	InsetCommandParams params_;
	///
	GuiBibtexAddDialog * add_;
	///
	ButtonController add_bc_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIBIBTEX_H
