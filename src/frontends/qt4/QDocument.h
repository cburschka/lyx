// -*- C++ -*-
/**
 * \file QDocument.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QDOCUMENT_H
#define QDOCUMENT_H

#include "QDialogView.h"
#include "BulletsModule.h"

#include "ui/DocumentUi.h"
#include "ui/FontUi.h"
#include "ui/TextLayoutUi.h"
#include "ui/MathsUi.h"
#include "ui/LaTeXUi.h"
#include "ui/PageLayoutUi.h"
#include "ui/LanguageUi.h"
#include "ui/BiblioUi.h"
#include "ui/NumberingUi.h"
#include "ui/MarginsUi.h"

// For the Preamble module
#include "ui/PreambleUi.h"

#include <QCloseEvent>
#include <QDialog>

#include <vector>
#include <string>

class FloatPlacement;

template<class UI>
class UiWidget: public QWidget, public UI
{
public:
	UiWidget(QWidget * parent = 0) : QWidget(parent)
	{
		UI::setupUi(this);
	}
};

namespace lyx {
namespace frontend {

class QBranches;
class QDocument;
class PreambleModule;

class QDocumentDialog : public QDialog, public Ui::QDocumentUi {
	Q_OBJECT
public:
	friend class QDocument;

	QDocumentDialog(QDocument *);

	void updateParams(BufferParams const & params);
	void apply(BufferParams & params);

	void updateFontsize(std::string const &, std::string const &);
	void updatePagestyle(std::string const &, std::string const &);

	void showPreamble();
	/// validate listings parameters and return an error message, if any
	docstring validate_listings_params();

public Q_SLOTS:
	void updateNumbering();
	void change_adaptor();
	void set_listings_msg();
	void saveDefaultClicked();
	void useDefaultsClicked();

protected Q_SLOTS:
	void setLSpacing(int);
	void setMargins(bool);
	void setCustomPapersize(int);
	void setCustomMargins(bool);
	void romanChanged(int);
	void sansChanged(int);
	void ttChanged(int);
	void setSkip(int);
	void enableSkip(bool);
	void portraitChanged();
	void classChanged();

protected:
	void closeEvent(QCloseEvent * e);

private:

	UiWidget<Ui::TextLayoutUi> *textLayoutModule;
	UiWidget<Ui::FontUi> *fontModule;
	UiWidget<Ui::PageLayoutUi> *pageLayoutModule;
	UiWidget<Ui::MarginsUi> *marginsModule;
	UiWidget<Ui::LanguageUi> *langModule;
	UiWidget<Ui::NumberingUi> *numberingModule;
	UiWidget<Ui::BiblioUi> *biblioModule;
	UiWidget<Ui::MathsUi> *mathsModule;
	UiWidget<Ui::LaTeXUi> *latexModule;
	PreambleModule *preambleModule;

	QBranches *branchesModule;

	BulletsModule * bulletsModule;
	FloatPlacement * floatModule;

	QDocument * form_;

	/// FIXME
	std::vector<std::string> lang_;
};


class ControlDocument;

typedef void const * BufferId;

class QDocument
	: public QController<ControlDocument, QView<QDocumentDialog> >
{
public:

	friend class QDocumentDialog;

	QDocument(Dialog &);

	void showPreamble();

private:
	/// Apply changes
	void apply();
	/// update
	void update_contents();
	/// force content update
	void forceUpdate();
	/// build the dialog
	void build_dialog();
	/// save as default template
	void saveDocDefault();
	/// reset to default params
	void useClassDefaults();
	/// current buffer
	BufferId current_id_;
protected:
	/// return false if validate_listings_params returns error
	virtual bool isValid();
};


class PreambleModule : public UiWidget<Ui::PreambleUi>
{
	Q_OBJECT
public:
	PreambleModule();
	void update(BufferParams const & params, BufferId id);
	void apply(BufferParams & params);

Q_SIGNALS:
	/// signal that something's changed in the Widget.
	void changed();

protected:
	void closeEvent(QCloseEvent *);
	void on_preambleTE_textChanged() { changed(); }

private:
	typedef std::map<BufferId, std::pair<int,int> > Coords;
	Coords preamble_coords_;
	BufferId current_id_;
};



} // namespace frontend
} // namespace lyx

#endif // QDOCUMENT_H
