// -*- C++ -*-
/**
 * \file GuiDocument.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author Richard Heck (modules)
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIDOCUMENT_H
#define GUIDOCUMENT_H

#include "GuiDialog.h"
#include "BulletsModule.h"
#include "GuiSelectionManager.h"
#include "BufferParams.h"

#include "support/FileName.h"
#include "support/filetools.h"
#include "support/types.h"

#include "ui_DocumentUi.h"
#include "ui_EmbeddedFilesUi.h"
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
#include "ui_PDFSupportUi.h"

#include <map>
#include <vector>

class FloatPlacement;

namespace lyx {

class BufferParams;
class TextClass;

namespace frontend {

class GuiBranches;
class PreambleModule;

///
typedef void const * BufferId;
///
typedef std::map<std::string, support::FileName> ModuleMap;

#include <QDialog>
#include <QStringList>
#include <QStringListModel>

#include <vector>
#include <string>

template<class UI>
class UiWidget : public QWidget, public UI
{
public:
	UiWidget(QWidget * parent = 0) : QWidget(parent) { UI::setupUi(this); }
};


class GuiDocument : public GuiDialog, public Ui::DocumentUi
{
	Q_OBJECT
public:
	GuiDocument(LyXView & lv);

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

private Q_SLOTS:
	void updateParams();
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
	void updateModuleInfo();
	void updateEmbeddedFileList();
	
private:
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
	UiWidget<Ui::PDFSupportUi> *pdfSupportModule;
	UiWidget<Ui::EmbeddedFilesUi> *embeddedFilesModule;
	PreambleModule *preambleModule;
	
	GuiBranches *branchesModule;

	BulletsModule * bulletsModule;
	FloatPlacement * floatModule;

	GuiSelectionManager * selectionManager;

	// FIXME
	std::vector<std::string> lang_;

	/// Available modules
	QStringListModel * availableModel() { return &available_model_; }
	/// Selected modules
	QStringListModel * selectedModel() { return &selected_model_; }
private:
	/// Apply changes
	void applyView();
	/// update
	void updateContents();
	/// save as default template
	void saveDocDefault();
	/// reset to default params
	void useClassDefaults();
	/// available modules
	QStringListModel available_model_;
	/// selected modules
	QStringListModel selected_model_;

protected:
	/// return false if validate_listings_params returns error
	bool isValid();

	/// font family names for BufferParams::fontsDefaultFamily
	static char const * const fontfamilies[5];
	/// GUI names corresponding fontfamilies
	static char const * fontfamilies_gui[5];
	///
	bool initialiseParams(std::string const & data);
	///
	void clearParams();
	///
	void dispatchParams();
	///
	bool isBufferDependent() const { return true; }
	/// always true since we don't manipulate document contents
	bool canApply() const { return true; }
	///
	TextClass const & textClass() const;
	///
	BufferParams & params() { return bp_; }
	///
	BufferParams const & params() const { return bp_; }
	///
	BufferId id() const;
	/// List of available modules
	std::vector<std::string> getModuleNames();
	/// Modules in use in current buffer
	std::vector<std::string> const & getSelectedModules();
	///
	std::string getModuleDescription(std::string const & modName) const;
	///
	std::vector<std::string> getPackageList(std::string const & modName) const;
	///
	void setLanguage() const;
	///
	void saveAsDefault() const;
	///
	bool isFontAvailable(std::string const & font) const;
	/// does this font provide Old Style figures?
	bool providesOSF(std::string const & font) const;
	/// does this font provide true Small Caps?
	bool providesSC(std::string const & font) const;
	/// does this font provide size adjustment?
	bool providesScale(std::string const & font) const;
private:
	///
	void loadModuleNames();
	///
	BufferParams bp_;
	/// List of names of available modules
	std::vector<std::string> moduleNames_;
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

private:
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
