// -*- C++ -*-
/**
 * \file QCitationDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kalle Dalheimer
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QCITATIONDIALOG_H
#define QCITATIONDIALOG_H

#include "Dialog.h"
#include "ui_CitationUi.h"

#include <QCloseEvent>
#include <QKeyEvent>

namespace lyx {
namespace frontend {

class QCitation;

class QCitationDialog: public QDialog, public Ui::QCitationUi, public Dialog::View {
	Q_OBJECT

public:
	QCitationDialog(Dialog &, QCitation * form );

	virtual ~QCitationDialog();

	virtual void apply();

	/// Hide the dialog from sight
	void hide();

	/// Redraw the dialog (e.g. if the colors have been remapped).
	void redraw() {}

	/// Create the dialog if necessary, update it and display it.
	void show();

	/// Update the display of the dialog whilst it is still visible.
	void update();

	/// \return true if the dialog is visible.
	bool isVisible() const;
	
	///
	bool eventFilter(QObject *, QEvent *);

protected:
	void closeEvent (QCloseEvent * e);
	void findText(QString const & text);

protected Q_SLOTS:
	void cleanUp();
	void on_okPB_clicked();
	void on_cancelPB_clicked();
	void on_restorePB_clicked();
	void on_applyPB_clicked();
	void on_addPB_clicked();
	void on_deletePB_clicked();
	void on_upPB_clicked();
	void on_downPB_clicked();
	void on_findLE_textChanged(const QString & text);
	void on_caseCB_stateChanged(int);
	void on_regexCB_stateChanged(int);
	void on_selectedLV_clicked(const QModelIndex &);
	void selectedChanged(const QModelIndex &, const QModelIndex &);
	void on_availableLV_clicked(const QModelIndex &);
	void on_availableLV_doubleClicked(const QModelIndex &);
	void availableChanged(const QModelIndex &, const QModelIndex &);
	virtual void changed();
	/// check whether key is already selected
	bool isSelected(const QModelIndex &);
	/// update infobox
	void updateInfo(const QModelIndex &);

private:
	/// enable/disable buttons
	void setButtons();

	/// fill the styles combo
	void fillStyles();

	/// set the styles combo
	void updateStyle();
	
	/// last used citation style
	int style_;
	
	QCitation * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QCITATIOINDIALOG_H
