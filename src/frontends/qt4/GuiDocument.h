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

#include <QDialog>

#include "BufferParams.h"
#include "BulletsModule.h"
#include "GuiDialog.h"
#include "GuiIdListModel.h"
#include "GuiSelectionManager.h"

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
QModelIndex getSelectedIndex(QListView * lv);

///
typedef void const * BufferId;

template<class UI>
class UiWidget : public QWidget, public UI
{
public:
	UiWidget(QWidget * parent = 0) : QWidget(parent) { UI::setupUi(this); }
};


/// SelectionManager for use with modules
class ModuleSelMan : public GuiSelectionManager 
{
public:
	ModuleSelMan(
		QListView * availableLV, 
		QListView * selectedLV,
		QPushButton * addPB, 
		QPushButton * delPB, 
		QPushButton * upPB, 
		QPushButton * downPB,
		GuiIdListModel * availableModel,
		GuiIdListModel * selectedModel);
private:
	///
	virtual void updateAddPB();
	///
	virtual void updateUpPB();
	///
	virtual void updateDownPB();
	///
	virtual void updateDelPB();
	/// returns availableModel as a GuiIdListModel
	GuiIdListModel * getAvailableModel() 
	{
		return dynamic_cast<GuiIdListModel *>(availableModel);
	};
	/// returns selectedModel as a GuiIdListModel
	GuiIdListModel * getSelectedModel() 
	{
		return dynamic_cast<GuiIdListModel *>(selectedModel);
	};
};


class GuiDocument : public GuiDialog, public Ui::DocumentUi
{
	Q_OBJECT
public:
	GuiDocument(GuiView & lv);

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
	void setColSep();
	void setCustomMargins(bool);
	void romanChanged(int);
	void sansChanged(int);
	void ttChanged(int);
	void setSkip(int);
	void enableSkip(bool);
	void portraitChanged();
	void browseLayout();
	void browseMaster();
	void classChanged();
	void updateModuleInfo();

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
	PreambleModule *preambleModule;
	
	GuiBranches *branchesModule;

	BulletsModule * bulletsModule;
	FloatPlacement * floatModule;

	GuiSelectionManager * selectionManager;

	///
	QStringList lang_;

	/// Available modules
	GuiIdListModel * availableModel() { return &modules_av_model_; }
	/// Selected modules
	GuiIdListModel * selectedModel() { return &modules_sel_model_; }
private:
	/// Apply changes
	void applyView();
	/// update
	void updateContents();
	/// force content update
	void forceUpdate();
	///
	void updateAvailableModules();
	///
	void updateSelectedModules();
	/// save as default template
	void saveDocDefault();
	/// reset to default params
	void useClassDefaults();
	///
	void setLayoutComboByIDString(std::string const & idString);
	/// available classes
	GuiIdListModel classes_model_;
	/// available modules
	GuiIdListModel modules_av_model_;
	/// selected modules
	GuiIdListModel modules_sel_model_;
	/// current buffer
	BufferId current_id_;

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
	DocumentClass const & documentClass() const;
	///
	BufferParams & params() { return bp_; }
	///
	BufferParams const & params() const { return bp_; }
	///
	BufferId id() const;
	///
	struct modInfoStruct {
		QString name;
		std::string id;
		QString description;
	};
	/// List of available modules
	std::vector<modInfoStruct> const & getModuleInfo();
	/// Modules in use in current buffer
	std::vector<modInfoStruct> const getSelectedModules();
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
	void loadModuleInfo();
	///
	BufferParams bp_;
	/// List of names of available modules
	std::vector<modInfoStruct> moduleNames_;
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

#endif // GUIDOCUMENT_H
