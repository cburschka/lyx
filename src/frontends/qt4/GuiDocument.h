// -*- C++ -*-
/**
 * \file GuiDocument.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIDOCUMENT_H
#define GUIDOCUMENT_H

#include "GuiDialogView.h"
#include "BulletsModule.h"
#include "ControlDocument.h"

#include "ui_DocumentUi.h"
#include "ui_FontUi.h"
#include "ui_TextLayoutUi.h"
#include "ui_MathsUi.h"
#include "ui_LaTeXUi.h"
#include "ui_PageLayoutUi.h"
#include "ui_LanguageUi.h"
#include "ui_BiblioUi.h"
#include "ui_NumberingUi.h"
#include "ui_MarginsUi.h"
#include "ui_PreambleUi.h"

#include <QDialog>

#include <vector>
#include <string>

class FloatPlacement;

template<class UI>
class UiWidget : public QWidget, public UI
{
public:
	UiWidget(QWidget * parent = 0) : QWidget(parent)
	{
		UI::setupUi(this);
	}
};

namespace lyx {
namespace frontend {

class GuiBranches;
class GuiDocument;
class PreambleModule;

class GuiDocumentDialog : public QDialog, public Ui::DocumentUi {
	Q_OBJECT
public:
	friend class GuiDocument;

	GuiDocumentDialog(GuiDocument *);

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

	GuiBranches *branchesModule;

	BulletsModule * bulletsModule;
	FloatPlacement * floatModule;

	GuiDocument * form_;

	/// FIXME
	std::vector<std::string> lang_;
};


class GuiDocument : public GuiView<GuiDocumentDialog>
{
public:

	friend class GuiDocumentDialog;

	GuiDocument(GuiDialog &);

	void showPreamble();
	/// parent controller
	ControlDocument & controller()
	{ return static_cast<ControlDocument &>(this->getController()); }
	/// parent controller
	ControlDocument const & controller() const
	{ return static_cast<ControlDocument const &>(this->getController()); }
private:
	/// Apply changes
	void apply();
	/// update
	void update_contents();
	/// build the dialog
	void build_dialog();
	/// save as default template
	void saveDocDefault();
	/// reset to default params
	void useClassDefaults();
protected:
	/// return false if validate_listings_params returns error
	virtual bool isValid();
};


typedef void const * BufferId;


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

#endif // GUIDOCUMENT_H
