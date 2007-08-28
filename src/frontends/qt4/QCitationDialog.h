// -*- C++ -*-
/**
 * \file QCitationDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kalle Dalheimer
 * \author Abdelrazak Younes
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QCITATIONDIALOG_H
#define QCITATIONDIALOG_H

#include "Dialog.h"
#include "QSelectionManager.h"
#include "ui_CitationUi.h"

#include <QCloseEvent>
#include <QKeyEvent>
#include <QStringList>
#include <QStringListModel>

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

	/// \return true if the dialog is visible.
	bool isVisible() const;

public Q_SLOTS:
	/// Update the display of the dialog whilst it is still visible.
	void update();

protected:
	void closeEvent (QCloseEvent * e);
	/// prepares a call to QCitation::searchKeys when we
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
	
	QCitation * form_;

	QSelectionManager * selectionManager;
};

} // namespace frontend
} // namespace lyx

#endif // QCITATIOINDIALOG_H
