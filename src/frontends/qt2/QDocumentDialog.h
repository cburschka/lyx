// -*- C++ -*-
/**
 * \file QDocumentDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QDOCUMENTDIALOG_H
#define QDOCUMENTDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QDocumentDialogBase.h"
#include "LString.h"

#include "ui/BulletsModuleBase.h"
#include "BulletsModule.h"

#include "ui/ClassModuleBase.h"
#include "ui/PackagesModuleBase.h"
#include "ui/PaperModuleBase.h"
#include "ui/LanguageModuleBase.h"
#include "ui/BiblioModuleBase.h"
#include "ui/NumberingModuleBase.h"
#include "ui/MarginsModuleBase.h"
#include "ui/PreambleModuleBase.h"

class QDocument;

class QDocumentDialog : public QDocumentDialogBase {
	Q_OBJECT
public:
	friend class QDocument;

	QDocumentDialog(QDocument *);
	~QDocumentDialog();

	void updateFontsize(string const &, string const &);
	void updatePagestyle(string const &, string const &);

public slots:
	void setTitle(int);
	void change_adaptor();
	void saveDefaultClicked();
	void useDefaultsClicked();

protected slots:
	void setLSpacing(int);
	void setMargins(int);
	void setCustomPapersize(int);
	void setCustomMargins(int);
	void setSkip(int);
	void enableSkip(bool);
	void portraitChanged();
	void classChanged();

protected:
	void closeEvent(QCloseEvent * e);

private:
	enum Module {
		LAYOUT,
		PAPER,
		MARGINS,
		LANGUAGE,
		BULLETS,
		NUMBERING,
		BIBLIOGRAPHY,
		PACKAGES,
		PREAMBLE
	};

	ClassModuleBase * layoutModule;
	PaperModuleBase * paperModule;
	MarginsModuleBase * marginsModule;
	LanguageModuleBase * langModule;
	BulletsModule * bulletsModule;
	NumberingModuleBase * numberingModule;
	BiblioModuleBase * biblioModule;
	PackagesModuleBase * packagesModule;
	PreambleModuleBase * preambleModule;

	QDocument * form_;
};

#endif // DOCUMENTDIALOG_H
