// -*- C++ -*-
/**
 * \file QDocumentDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QDOCUMENTDIALOG_H
#define QDOCUMENTDIALOG_H

#include "ui/QDocumentUi.h"
#include "ui/TextLayoutUi.h"
#include "ui/MathsUi.h"
#include "ui/LaTeXUi.h"
#include "ui/PageLayoutUi.h"
#include "ui/LanguageUi.h"
#include "ui/BiblioUi.h"
#include "ui/NumberingUi.h"
#include "ui/MarginsUi.h"
#include "ui/PreambleUi.h"

#include "BulletsModule.h"

#include "QBranches.h"

#include <QCloseEvent>
#include <QDialog>
#include <vector>
#include <string>

class FloatPlacement;

template<class UI>
	class UiWidget: public QWidget, public UI
	{
	public:
		UiWidget(QWidget * Parent=0): QWidget(Parent)
		{
			UI::setupUi(this);
		}
	};

namespace lyx {
namespace frontend {

class QDocument;

class QDocumentDialog : public QDialog, public Ui::QDocumentUi {
	Q_OBJECT
public:
	friend class QDocument;

	QDocumentDialog(QDocument *);
	~QDocumentDialog();

	void update(BufferParams const & params);
	void apply(BufferParams & params);

	void updateFontsize(std::string const &, std::string const &);
	void updatePagestyle(std::string const &, std::string const &);

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

protected:
	void closeEvent(QCloseEvent * e);

private:

	UiWidget<Ui::TextLayoutUi> *textLayoutModule;
	UiWidget<Ui::PageLayoutUi> *pageLayoutModule;
	UiWidget<Ui::MarginsUi> *marginsModule;
	UiWidget<Ui::LanguageUi> *langModule;
	UiWidget<Ui::NumberingUi> *numberingModule;
	UiWidget<Ui::BiblioUi> *biblioModule;
	UiWidget<Ui::MathsUi> *mathsModule;
	UiWidget<Ui::LaTeXUi> *latexModule;
	UiWidget<Ui::PreambleUi> *preambleModule;

	QBranches *branchesModule;

	BulletsModule * bulletsModule;
	FloatPlacement * floatModule;

	QDocument * form_;

	/// FIXME
	std::vector<std::string> lang_;
};

} // namespace frontend
} // namespace lyx

#endif // DOCUMENTDIALOG_H
