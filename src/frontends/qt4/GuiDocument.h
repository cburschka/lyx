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

#include "BufferParams.h"
#include "BulletsModule.h"
#include "GuiDialog.h"
#include "GuiIdListModel.h"

#include "ui_BiblioUi.h"
#include "ui_ColorUi.h"
#include "ui_DocumentUi.h"
#include "ui_FontUi.h"
#include "ui_LanguageUi.h"
#include "ui_LaTeXUi.h"
#include "ui_ListingsSettingsUi.h"
#include "ui_LocalLayoutUi.h"
#include "ui_MarginsUi.h"
#include "ui_MasterChildUi.h"
#include "ui_MathsUi.h"
#include "ui_ModulesUi.h"
#include "ui_NumberingUi.h"
#include "ui_OutputUi.h"
#include "ui_PageLayoutUi.h"
#include "ui_PDFSupportUi.h"
#include "ui_PreambleUi.h"
#include "ui_TextLayoutUi.h"

namespace lyx {

class BufferParams;
class LayoutModuleList;
class LyXModule;
class TextClass;

namespace frontend {

class FloatPlacement;
class GuiBranches;
class GuiIndices;
class ModuleSelectionManager;
class PreambleModule;
class LocalLayout;
class FontModule;

///
typedef void const * BufferId;

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
	GuiDocument(GuiView & lv);

	void paramsToDialog();
	void updateFontsize(std::string const &, std::string const &);
	void updateFontlist();
	void updateDefaultFormat();
	void updateEngineType(std::string const &, CiteEngineType const &);
	void updatePagestyle(std::string const &, std::string const &);
	bool isChildIncluded(std::string const &);

	///
	BufferParams const & params() const { return bp_; }

private Q_SLOTS:
	void updateNumbering();
	void change_adaptor();
	void includeonlyClicked(QTreeWidgetItem * item, int);
	void setListingsMessage();
	void saveDefaultClicked();
	void useDefaultsClicked();
	void setLSpacing(int);
	void setMargins();
	void papersizeChanged(int);
	void setColSep();
	void setCustomMargins(bool);
	void fontencChanged(int);
	void romanChanged(int);
	void sansChanged(int);
	void ttChanged(int);
	void fontOsfToggled(bool state);
	void fontScToggled(bool state);
	void setIndent(int);
	void enableIndent(bool);
	void setSkip(int);
	void enableSkip(bool);
	void browseLayout();
	void browseMaster();
	void classChanged();
	void classChanged_adaptor();
	void languagePackageChanged(int);
	void biblioChanged();
	void bibtexChanged(int);
	void setAuthorYear(bool);
	void setNumerical(bool);
	void updateModuleInfo();
	void modulesChanged();
	void changeBackgroundColor();
	void deleteBackgroundColor();
	void changeFontColor();
	void deleteFontColor();
	void changeNoteFontColor();
	void deleteNoteFontColor();
	void changeBoxBackgroundColor();
	void deleteBoxBackgroundColor();
	void languageChanged(int);
	void osFontsChanged(bool);
	void mathFontChanged(int);
	void branchesRename(docstring const &, docstring const &);
	void allPackagesAuto();
	void allPackagesAlways();
	void allPackagesNot();
private:
	/// validate listings parameters and return an error message, if any
	QString validateListingsParameters();

	UiWidget<Ui::TextLayoutUi> *textLayoutModule;
	UiWidget<Ui::MasterChildUi> *masterChildModule;
	FontModule *fontModule;
	UiWidget<Ui::PageLayoutUi> *pageLayoutModule;
	UiWidget<Ui::MarginsUi> *marginsModule;
	UiWidget<Ui::LanguageUi> *langModule;
	UiWidget<Ui::ColorUi> *colorModule;
	UiWidget<Ui::NumberingUi> *numberingModule;
	UiWidget<Ui::BiblioUi> *biblioModule;
	UiWidget<Ui::MathsUi> *mathsModule;
	UiWidget<Ui::LaTeXUi> *latexModule;
	UiWidget<Ui::PDFSupportUi> *pdfSupportModule;
	UiWidget<Ui::ModulesUi> *modulesModule;
	UiWidget<Ui::OutputUi> *outputModule;
	UiWidget<Ui::ListingsSettingsUi> *listingsModule;
	PreambleModule * preambleModule;
	LocalLayout * localLayout;

	GuiBranches * branchesModule;
	GuiIndices * indicesModule;

	BulletsModule * bulletsModule;
	FloatPlacement * floatModule;

	ModuleSelectionManager * selectionManager;

	/// Available modules
	GuiIdListModel * availableModel() { return &modules_av_model_; }
	/// Selected modules
	GuiIdListModel * selectedModel() { return &modules_sel_model_; }

	/// Apply changes
	void applyView();
	/// update
	void updateContents();
	///
	void updateAvailableModules();
	///
	void updateSelectedModules();
	///
	void updateIncludeonlys();
	/// save as default template
	void saveDocDefault();
	/// reset to default params
	void useClassDefaults();
	///
	void setLayoutComboByIDString(std::string const & idString);

	/// available modules
	GuiIdListModel modules_av_model_;
	/// selected modules
	GuiIdListModel modules_sel_model_;

	/// return false if validate_listings_params returns error
	bool isValid();

	/// font family names for BufferParams::fonts_default_family
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
	void modulesToParams(BufferParams &);
	///
	bool isBufferDependent() const { return true; }
	/// always true since we don't manipulate document contents
	bool canApply() const { return true; }
	///
	DocumentClass const & documentClass() const;
	///
	BufferParams & params() { return bp_; }
	///
	BufferId id() const;
	///
	struct modInfoStruct {
		QString name;
		std::string id;
		QString description;
	};
	///
	static modInfoStruct modInfo(LyXModule const & mod);
	/// List of available modules
	std::list<modInfoStruct> const & getModuleInfo();
	/// Modules in use in current buffer
	std::list<modInfoStruct> const getSelectedModules();
	///
	std::list<modInfoStruct> const getProvidedModules();
	///
	std::list<modInfoStruct> const
			makeModuleInfo(LayoutModuleList const & mods);
	///
	void setLanguage() const;
	///
	void saveAsDefault() const;
	/// does this font provide Old Style figures?
	bool providesOSF(QString const & font) const;
	/// does this font provide true Small Caps?
	bool providesSC(QString const & font) const;
	/** does this font provide OSF and Small Caps only via 
	 * a single, undifferentiated expert option?
	 */
	bool hasMonolithicExpertSet(QString const & font) const;
	/// does this font provide size adjustment?
	bool providesScale(QString const & font) const;
	/// does this font provide an alternative without math?
	bool providesNoMath(QString const & font) const;
	///
	void executeBranchRenaming() const;
	///
	void setCustomPapersize(bool custom);
private:
	///
	void loadModuleInfo();
	///
	void updateUnknownBranches();
	///
	void updateTexFonts();
	///
	void updateMathFonts(QString const & rm);
	///
	void updateFontOptions();
	///
	bool ot1() const;
	///
	bool completeFontset() const;
	///
	bool noMathFont() const;
	///
	void allPackages(int);
	///
	BufferParams bp_;
	/// List of names of available modules
	std::list<modInfoStruct> moduleNames_;
	///
	std::map<docstring, docstring> changedBranches_;
	///
	std::list<std::string> includeonlys_;
	///
	bool biblioChanged_;
	/// Track if a non-module document param changed
	bool nonModuleChanged_;
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


class LocalLayout : public UiWidget<Ui::LocalLayoutUi>
{
	Q_OBJECT
public:
	LocalLayout();
	void update(BufferParams const & params, BufferId id);
	void apply(BufferParams & params);
	bool isValid() const { return validated_; }

Q_SIGNALS:
	/// signal that something's changed in the Widget.
	void changed();

private:
	void validate();
	void convert();
private Q_SLOTS:
	void textChanged();
	void validatePressed();
	void convertPressed();

private:
	BufferId current_id_;
	bool validated_;
};


class FontModule : public UiWidget<Ui::FontUi>
{
	Q_OBJECT
public:
	/// The roman font currently not selected by osFontsCB->isChecked()
	QString font_roman;
	/// The sans font currently not selected by osFontsCB->isChecked()
	QString font_sans;
	/// The typewriter font currently not selected by osFontsCB->isChecked()
	QString font_typewriter;
	/// The math font currently not selected by osFontsCB->isChecked()
	QString font_math;
	/// The sans font scale currently not selected by osFontsCB->isChecked()
	int font_sf_scale;
	/// The typewriter font scale currently not selected by osFontsCB->isChecked()
	int font_tt_scale;
};


} // namespace frontend
} // namespace lyx

#endif // GUIDOCUMENT_H
