// -*- C++ -*-
/**
 * \file QDocumentDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QDOCUMENTDIALOG_H
#define QDOCUMENTDIALOG_H

#include "ui/QDocumentDialogBase.h"

#include "BulletsModule.h"

#include "ui/TextLayoutModuleBase.h"
#include "ui/MathsModuleBase.h"
#include "ui/LaTeXModuleBase.h"
#include "ui/PageLayoutModuleBase.h"
#include "ui/LanguageModuleBase.h"
#include "ui/BiblioModuleBase.h"
#include "ui/NumberingModuleBase.h"
#include "ui/MarginsModuleBase.h"
#include "ui/PreambleModuleBase.h"
#include "ui/BranchesModuleBase.h"

class FloatPlacement;

namespace lyx {
namespace frontend {

class QDocument;

class QDocumentDialog : public QDocumentDialogBase {
	Q_OBJECT
public:
	friend class QDocument;

	QDocumentDialog(QDocument *);
	~QDocumentDialog();

	void updateFontsize(std::string const &, std::string const &);
	void updatePagestyle(std::string const &, std::string const &);
	void updateBranchView();
	void toggleBranch(QListViewItem * selItem);

	void showPreamble();

public slots:
	void updateNumbering();
	void change_adaptor();
	void saveDefaultClicked();
	void useDefaultsClicked();

protected slots:
	void setLSpacing(int);
	void setMargins(bool);
	void setCustomPapersize(int);
	void setCustomMargins(bool);
	void setSkip(int);
	void enableSkip(bool);
	void portraitChanged();
	void classChanged();
	void addBranchPressed();
	void deleteBranchPressed();
	void toggleBranchPressed();
	void branchDoubleClicked(QListViewItem * selItem);
	void toggleBranchColor();

protected:
	void closeEvent(QCloseEvent * e);

private:
	TextLayoutModuleBase * textLayoutModule;
	PageLayoutModuleBase * pageLayoutModule;
	MarginsModuleBase * marginsModule;
	LanguageModuleBase * langModule;
	BulletsModule * bulletsModule;
	NumberingModuleBase * numberingModule;
	BiblioModuleBase * biblioModule;
	MathsModuleBase * mathsModule;
	LaTeXModuleBase * latexModule;
	PreambleModuleBase * preambleModule;
	FloatPlacement * floatModule;
	BranchesModuleBase * branchesModule;

	QDocument * form_;
};

} // namespace frontend
} // namespace lyx

#endif // DOCUMENTDIALOG_H
