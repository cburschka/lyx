// -*- C++ -*-
/**
 * \file QCitationDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QCITATIONDIALOG_H
#define QCITATIONDIALOG_H

#include "ui/QCitationUi.h"
#include "ui/QCitationFindUi.h"
#include "controllers/biblio.h"

#include <QDialog>
#include <vector>

class Q3ListBox;
class Q3ListBoxItem;

class InsetCommandParams;

namespace lyx {
namespace frontend {

class QCitation;
class QCitationFind;

class QCitationDialog : public QDialog, public Ui::QCitationUi {
	Q_OBJECT

public:
	QCitationDialog(QCitation * form);

	~QCitationDialog();

	void update(InsetCommandParams const & params);
	void apply(InsetCommandParams & params);

//	virtual bool isValid();

protected slots:

//	void on_selectedLB_currentChanged(Q3ListBoxItem*);
	
	void on_okPB_clicked();
	void on_cancelPB_clicked();
	void on_restorePB_clicked();
	void on_applyPB_clicked();
	void on_addPB_clicked();

	void on_deletePB_clicked();
	void on_upPB_clicked();
	void on_downPB_clicked();
	void on_findLE_textChanged(const QString & text);
	void on_advancedSearchPB_clicked();

	virtual void changed();

private:
	void setButtons();
	/// open the find dialog if nothing selected
	void openFind();

	/// fill the styles combo
	void fillStyles();

	/// set the styles combo
	void updateStyle();

	/// check if apply has been pressed
	bool open_find_;

	/// selected keys
	std::vector<std::string> citekeys;

	/// selected natbib style
	int style_;

	QCitation * form_;
	QCitationFind * find_;
};


class QCitationFind: public QDialog, public Ui::QCitationFindUi {
	Q_OBJECT

public:
	QCitationFind(QCitation * form, QWidget * parent = 0, Qt::WFlags f = 0);

	void update();

	std::vector<std::string> const & foundCitations()
	{ return foundkeys;	}

signals:
	void newCitations();

protected slots:

	void on_availableLB_currentChanged(Q3ListBoxItem *);
	void on_availableLB_selected(Q3ListBoxItem *);
	void on_addPB_clicked();
	virtual void previous();
	virtual void next();

private:
	void find(biblio::Direction dir);

	/// selected keys
	std::vector<std::string> foundkeys;

	QCitation * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QCITATIOINDIALOG_H
