/**
 * \file GuiDocument.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author Richard Heck (modules)
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiDocument.h"

#include "GuiApplication.h"
#include "GuiBranches.h"
#include "GuiSelectionManager.h"
#include "LaTeXHighlighter.h"
#include "LengthCombo.h"
#include "PanelStack.h"
#include "Validator.h"

#include "LayoutFile.h"
#include "BranchList.h"
#include "buffer_funcs.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Color.h"
#include "ColorCache.h"
#include "Encoding.h"
#include "FloatPlacement.h"
#include "Format.h"
#include "FuncRequest.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "Layout.h"
#include "LayoutModuleList.h"
#include "LyXRC.h"
#include "ModuleList.h"
#include "OutputParams.h"
#include "PDFOptions.h"
#include "qt_helpers.h"
#include "Spacing.h"

#include "insets/InsetListingsParams.h"

#include "support/debug.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include "frontends/alert.h"

#include <QAbstractItemModel>
#include <QColor>
#include <QColorDialog>
#include <QCloseEvent>
#include <QFontDatabase>
#include <QScrollBar>
#include <QTextCursor>

#include <sstream>
#include <vector>

#ifdef IN
#undef IN
#endif


// a style sheet for buttons
// this is for example used for the background color setting button
static inline QString colorButtonStyleSheet(const QColor &bgColor)
{
    if (bgColor.isValid()) {
		QString rc = QLatin1String("background:");
        rc += bgColor.name();
        return rc;
    }
    return QLatin1String("");
}


using namespace std;
using namespace lyx::support;


namespace {

char const * const tex_graphics[] =
{
	"default", "dvialw", "dvilaser", "dvipdf", "dvipdfm", "dvipdfmx",
	"dvips", "dvipsone", "dvitops", "dviwin", "dviwindo", "dvi2ps", "emtex",
	"ln", "oztex", "pctexhp", "pctexps", "pctexwin", "pctex32", "pdftex",
	"psprint", "pubps", "tcidvi", "textures", "truetex", "vtex", "xdvi",
	"xetex", "none", ""
};


char const * const tex_graphics_gui[] =
{
	N_("Default"), "dvialw", "DviLaser", "dvipdf", "DVIPDFM", "DVIPDFMx",
	"Dvips", "DVIPSONE", "DVItoPS", "DVIWIN", "DVIWindo", "dvi2ps", "EmTeX",
	"LN", "OzTeX", "pctexhp", "pctexps", "pctexwin", "PCTeX32", "pdfTeX",
	"psprint", "pubps", "tcidvi", "Textures", "TrueTeX", "VTeX", "xdvi",
	"XeTeX", N_("None"), ""
};


char const * const tex_fonts_roman[] =
{
	"default", "cmr", "lmodern", "ae", "times", "palatino",
	"charter", "newcent", "bookman", "utopia", "beraserif",
	"ccfonts", "chancery", ""
};


char const * tex_fonts_roman_gui[] =
{
	N_("Default"), N_("Computer Modern Roman"), N_("Latin Modern Roman"),
	N_("AE (Almost European)"), N_("Times Roman"), N_("Palatino"),
	N_("Bitstream Charter"), N_("New Century Schoolbook"), N_("Bookman"),
	N_("Utopia"),  N_("Bera Serif"), N_("Concrete Roman"), N_("Zapf Chancery"),
	""
};


char const * const tex_fonts_sans[] =
{
	"default", "cmss", "lmss", "helvet", "avant", "berasans", "cmbr", ""
};


char const * tex_fonts_sans_gui[] =
{
	N_("Default"), N_("Computer Modern Sans"), N_("Latin Modern Sans"),
	N_("Helvetica"), N_("Avant Garde"), N_("Bera Sans"), N_("CM Bright"), ""
};


char const * const tex_fonts_monospaced[] =
{
	"default", "cmtt", "lmtt", "courier", "beramono", "luximono", "cmtl", ""
};


char const * tex_fonts_monospaced_gui[] =
{
	N_("Default"), N_("Computer Modern Typewriter"),
	N_("Latin Modern Typewriter"), N_("Courier"), N_("Bera Mono"),
	N_("LuxiMono"), N_("CM Typewriter Light"), ""
};


char const * backref_opts[] =
{
	"false", "section", "slide", "page", ""
};


char const * backref_opts_gui[] =
{
	N_("Off"), N_("Section"), N_("Slide"), N_("Page"), ""
};


vector<pair<string, QString> > pagestyles;


} // anonymous namespace

namespace lyx {

RGBColor set_backgroundcolor;

namespace {
// used when sorting the textclass list.
class less_textclass_avail_desc
	: public binary_function<string, string, int>
{
public:
	bool operator()(string const & lhs, string const & rhs) const
	{
		// Ordering criteria:
		//   1. Availability of text class
		//   2. Description (lexicographic)
		LayoutFile const & tc1 = LayoutFileList::get()[lhs];
		LayoutFile const & tc2 = LayoutFileList::get()[rhs];
		int const rel = compare_no_case(
			translateIfPossible(from_utf8(tc1.description())),
			translateIfPossible(from_utf8(tc2.description())));
		return (tc1.isTeXClassAvailable() && !tc2.isTeXClassAvailable()) ||
			(tc1.isTeXClassAvailable() == tc2.isTeXClassAvailable() && rel < 0);
	}
};

}

namespace frontend {
namespace {

vector<string> getRequiredList(string const & modName) 
{
	LyXModule const * const mod = moduleList[modName];
	if (!mod)
		return vector<string>(); //empty such thing
	return mod->getRequiredModules();
}


vector<string> getExcludedList(string const & modName)
{
	LyXModule const * const mod = moduleList[modName];
	if (!mod)
		return vector<string>(); //empty such thing
	return mod->getExcludedModules();
}


docstring getModuleDescription(string const & modName)
{
	LyXModule const * const mod = moduleList[modName];
	if (!mod)
		return _("Module not found!");
	// FIXME Unicode
	return translateIfPossible(from_utf8(mod->getDescription()));
}


vector<string> getPackageList(string const & modName)
{
	LyXModule const * const mod = moduleList[modName];
	if (!mod)
		return vector<string>(); //empty such thing
	return mod->getPackageList();
}


bool isModuleAvailable(string const & modName)
{
	LyXModule * mod = moduleList[modName];
	if (!mod)
		return false;
	return mod->isAvailable();
}

} // anonymous namespace


/////////////////////////////////////////////////////////////////////
//
// ModuleSelectionManager
//
/////////////////////////////////////////////////////////////////////

/// SelectionManager for use with modules
class ModuleSelectionManager : public GuiSelectionManager 
{
public:
	///
	ModuleSelectionManager(
		QListView * availableLV, 
		QListView * selectedLV,
		QPushButton * addPB, 
		QPushButton * delPB, 
		QPushButton * upPB, 
		QPushButton * downPB,
		GuiIdListModel * availableModel,
		GuiIdListModel * selectedModel,
		GuiDocument const * container)
	: GuiSelectionManager(availableLV, selectedLV, addPB, delPB,
				upPB, downPB, availableModel, selectedModel), container_(container)
		{}
	///
	void updateProvidedModules(LayoutModuleList const & pm) 
			{ provided_modules_ = pm.list(); }
	///
	void updateExcludedModules(LayoutModuleList const & em) 
			{ excluded_modules_ = em.list(); }
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
	}
	/// returns selectedModel as a GuiIdListModel
	GuiIdListModel * getSelectedModel() 
	{
		return dynamic_cast<GuiIdListModel *>(selectedModel);
	}
	/// keeps a list of the modules the text class provides
	std::list<std::string> provided_modules_;
	/// similarly...
	std::list<std::string> excluded_modules_;
	/// 
	GuiDocument const * container_;
};

void ModuleSelectionManager::updateAddPB() 
{
	int const arows = availableModel->rowCount();
	QModelIndexList const avail_sels = 
			availableLV->selectionModel()->selectedIndexes();

	// disable if there aren't any modules (?), if none of them is chosen
	// in the dialog, or if the chosen one is already selected for use.
	if (arows == 0 || avail_sels.isEmpty() || isSelected(avail_sels.first())) {
		addPB->setEnabled(false);
		return;
	}

	QModelIndex const & idx = availableLV->selectionModel()->currentIndex();
	string const modname = getAvailableModel()->getIDString(idx.row());

	bool const enable = 
		container_->params().moduleCanBeAdded(modname);
	addPB->setEnabled(enable);
}


void ModuleSelectionManager::updateDownPB()
{
	int const srows = selectedModel->rowCount();
	if (srows == 0) {
		downPB->setEnabled(false);
		return;
	}
	QModelIndex const & curidx = selectedLV->selectionModel()->currentIndex();
	int const curRow = curidx.row();
	if (curRow < 0 || curRow >= srows - 1) { // invalid or last item
		downPB->setEnabled(false);
		return;
	}

	// determine whether immediately succeding element requires this one
	string const curmodname = getSelectedModel()->getIDString(curRow);
	string const nextmodname = getSelectedModel()->getIDString(curRow + 1);

	vector<string> reqs = getRequiredList(nextmodname);

	// if it doesn't require anything....
	if (reqs.empty()) {
		downPB->setEnabled(true);
		return;
	}

	// Enable it if this module isn't required.
	// FIXME This should perhaps be more flexible and check whether, even 
	// if the next one is required, there is also an earlier one that will do.
	downPB->setEnabled(
			find(reqs.begin(), reqs.end(), curmodname) == reqs.end());
}

void ModuleSelectionManager::updateUpPB() 
{
	int const srows = selectedModel->rowCount();
	if (srows == 0) {
		upPB->setEnabled(false);
		return;
	}

	QModelIndex const & curIdx = selectedLV->selectionModel()->currentIndex();
	int curRow = curIdx.row();
	if (curRow <= 0 || curRow > srows - 1) { // first item or invalid
		upPB->setEnabled(false);
		return;
	}
	string const curmodname = getSelectedModel()->getIDString(curRow);

	// determine whether immediately preceding element is required by this one
	vector<string> reqs = getRequiredList(curmodname);

	// if this one doesn't require anything....
	if (reqs.empty()) {
		upPB->setEnabled(true);
		return;
	}


	// Enable it if the preceding module isn't required.
	// NOTE This is less flexible than it might be. We could check whether, even 
	// if the previous one is required, there is an earlier one that would do.
	string const premod = getSelectedModel()->getIDString(curRow - 1);
	upPB->setEnabled(find(reqs.begin(), reqs.end(), premod) == reqs.end());
}

void ModuleSelectionManager::updateDelPB() 
{
	int const srows = selectedModel->rowCount();
	if (srows == 0) {
		deletePB->setEnabled(false);
		return;
	}

	QModelIndex const & curidx = 
		selectedLV->selectionModel()->currentIndex();
	int const curRow = curidx.row();
	if (curRow < 0 || curRow >= srows) { // invalid index?
		deletePB->setEnabled(false);
		return;
	}

	string const curmodname = getSelectedModel()->getIDString(curRow);

	// We're looking here for a reason NOT to enable the button. If we
	// find one, we disable it and return. If we don't, we'll end up at
	// the end of the function, and then we enable it.
	for (int i = curRow + 1; i < srows; ++i) {
		string const thisMod = getSelectedModel()->getIDString(i);
		vector<string> reqs = getRequiredList(thisMod);
		//does this one require us?
		if (find(reqs.begin(), reqs.end(), curmodname) == reqs.end())
			//no...
			continue;

		// OK, so this module requires us
		// is there an EARLIER module that also satisfies the require?
		// NOTE We demand that it be earlier to keep the list of modules
		// consistent with the rule that a module must be proceeded by a
		// required module. There would be more flexible ways to proceed,
		// but that would be a lot more complicated, and the logic here is
		// already complicated. (That's why I've left the debugging code.)
		// lyxerr << "Testing " << thisMod << std::endl;
		bool foundone = false;
		for (int j = 0; j < curRow; ++j) {
			string const mod = getSelectedModel()->getIDString(j);
			// lyxerr << "In loop: Testing " << mod << std::endl;
			// do we satisfy the require? 
			if (find(reqs.begin(), reqs.end(), mod) != reqs.end()) {
				// lyxerr << mod << " does the trick." << std::endl;
				foundone = true;
				break;
			}
		}
		// did we find a module to satisfy the require?
		if (!foundone) {
			// lyxerr << "No matching module found." << std::endl;
			deletePB->setEnabled(false);
			return;
		}
	}
	// lyxerr << "All's well that ends well." << std::endl;	
	deletePB->setEnabled(true);
}


/////////////////////////////////////////////////////////////////////
//
// PreambleModule
//
/////////////////////////////////////////////////////////////////////

PreambleModule::PreambleModule() : current_id_(0)
{
	// This is not a memory leak. The object will be destroyed
	// with this.
	(void) new LaTeXHighlighter(preambleTE->document());
	setFocusProxy(preambleTE);
	connect(preambleTE, SIGNAL(textChanged()), this, SIGNAL(changed()));
}


void PreambleModule::update(BufferParams const & params, BufferId id)
{
	QString preamble = toqstr(params.preamble);
	// Nothing to do if the params and preamble are unchanged.
	if (id == current_id_
		&& preamble == preambleTE->document()->toPlainText())
		return;

	QTextCursor cur = preambleTE->textCursor();
	// Save the coords before switching to the new one.
	preamble_coords_[current_id_] =
		make_pair(cur.position(), preambleTE->verticalScrollBar()->value());

	// Save the params address for further use.
	current_id_ = id;
	preambleTE->document()->setPlainText(preamble);
	Coords::const_iterator it = preamble_coords_.find(current_id_);
	if (it == preamble_coords_.end())
		// First time we open this one.
		preamble_coords_[current_id_] = make_pair(0, 0);
	else {
		// Restore saved coords.
		QTextCursor cur = preambleTE->textCursor();
		cur.setPosition(it->second.first);
		preambleTE->setTextCursor(cur);
		preambleTE->verticalScrollBar()->setValue(it->second.second);
	}
}


void PreambleModule::apply(BufferParams & params)
{
	params.preamble = fromqstr(preambleTE->document()->toPlainText());
}


void PreambleModule::closeEvent(QCloseEvent * e)
{
	// Save the coords before closing.
	QTextCursor cur = preambleTE->textCursor();
	preamble_coords_[current_id_] =
		make_pair(cur.position(), preambleTE->verticalScrollBar()->value());
	e->accept();
}


/////////////////////////////////////////////////////////////////////
//
// DocumentDialog
//
/////////////////////////////////////////////////////////////////////


GuiDocument::GuiDocument(GuiView & lv)
	: GuiDialog(lv, "document", qt_("Document Settings"))
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(restorePB, SIGNAL(clicked()), this, SLOT(slotRestore()));

	connect(savePB, SIGNAL(clicked()), this, SLOT(saveDefaultClicked()));
	connect(defaultPB, SIGNAL(clicked()), this, SLOT(useDefaultsClicked()));

	// Manage the restore, ok, apply, restore and cancel/close buttons
	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setApply(applyPB);
	bc().setCancel(closePB);
	bc().setRestore(restorePB);

	textLayoutModule = new UiWidget<Ui::TextLayoutUi>;
	// text layout
	connect(textLayoutModule->lspacingCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->lspacingCO, SIGNAL(activated(int)),
		this, SLOT(setLSpacing(int)));
	connect(textLayoutModule->lspacingLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->skipRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->indentRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->skipCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->skipLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->skipLengthCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->skipCO, SIGNAL(activated(int)),
		this, SLOT(setSkip(int)));
	connect(textLayoutModule->skipRB, SIGNAL(toggled(bool)),
		this, SLOT(enableSkip(bool)));
	connect(textLayoutModule->twoColumnCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->twoColumnCB, SIGNAL(clicked()),
		this, SLOT(setColSep()));
	connect(textLayoutModule->listingsED, SIGNAL(textChanged()),
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->bypassCB, SIGNAL(clicked()), 
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->bypassCB, SIGNAL(clicked()), 
		this, SLOT(setListingsMessage()));
	connect(textLayoutModule->listingsED, SIGNAL(textChanged()),
		this, SLOT(setListingsMessage()));
	textLayoutModule->listingsTB->setPlainText(
		qt_("Input listings parameters on the right. Enter ? for a list of parameters."));
	textLayoutModule->lspacingLE->setValidator(new QDoubleValidator(
		textLayoutModule->lspacingLE));
	textLayoutModule->skipLE->setValidator(unsignedLengthValidator(
		textLayoutModule->skipLE));

	textLayoutModule->skipCO->addItem(qt_("SmallSkip"));
	textLayoutModule->skipCO->addItem(qt_("MedSkip"));
	textLayoutModule->skipCO->addItem(qt_("BigSkip"));
	textLayoutModule->skipCO->addItem(qt_("Length"));
	// remove the %-items from the unit choice
	textLayoutModule->skipLengthCO->noPercents();
	textLayoutModule->lspacingCO->insertItem(
		Spacing::Single, qt_("Single"));
	textLayoutModule->lspacingCO->insertItem(
		Spacing::Onehalf, qt_("OneHalf"));
	textLayoutModule->lspacingCO->insertItem(
		Spacing::Double, qt_("Double"));
	textLayoutModule->lspacingCO->insertItem(
		Spacing::Other, qt_("Custom"));

	// initialize the length validator
	bc().addCheckedLineEdit(textLayoutModule->skipLE);

	// output
	outputModule = new UiWidget<Ui::OutputUi>;

	connect(outputModule->xetexCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(outputModule->xetexCB, SIGNAL(toggled(bool)),
		this, SLOT(xetexChanged(bool)));
	connect(outputModule->defaultFormatCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));

	// fonts
	fontModule = new UiWidget<Ui::FontUi>;
	connect(fontModule->fontsRomanCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(fontModule->fontsRomanCO, SIGNAL(activated(int)),
		this, SLOT(romanChanged(int)));
	connect(fontModule->fontsSansCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(fontModule->fontsSansCO, SIGNAL(activated(int)),
		this, SLOT(sansChanged(int)));
	connect(fontModule->fontsTypewriterCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(fontModule->fontsTypewriterCO, SIGNAL(activated(int)),
		this, SLOT(ttChanged(int)));
	connect(fontModule->fontsDefaultCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(fontModule->fontsizeCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(fontModule->cjkFontLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(fontModule->scaleSansSB, SIGNAL(valueChanged(int)),
		this, SLOT(change_adaptor()));
	connect(fontModule->scaleTypewriterSB, SIGNAL(valueChanged(int)),
		this, SLOT(change_adaptor()));
	connect(fontModule->fontScCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(fontModule->fontOsfCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));

	updateFontlist();

	fontModule->fontsizeCO->addItem(qt_("Default"));
	fontModule->fontsizeCO->addItem(qt_("10"));
	fontModule->fontsizeCO->addItem(qt_("11"));
	fontModule->fontsizeCO->addItem(qt_("12"));

	for (int n = 0; GuiDocument::fontfamilies_gui[n][0]; ++n)
		fontModule->fontsDefaultCO->addItem(
			qt_(GuiDocument::fontfamilies_gui[n]));


	pageLayoutModule = new UiWidget<Ui::PageLayoutUi>;
	// page layout
	connect(pageLayoutModule->papersizeCO, SIGNAL(activated(int)),
		this, SLOT(setCustomPapersize(int)));
	connect(pageLayoutModule->papersizeCO, SIGNAL(activated(int)),
		this, SLOT(setCustomPapersize(int)));
	connect(pageLayoutModule->portraitRB, SIGNAL(clicked()),
		this, SLOT(portraitChanged()));
	connect(pageLayoutModule->papersizeCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(pageLayoutModule->paperheightLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(pageLayoutModule->paperwidthLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(pageLayoutModule->paperwidthUnitCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(pageLayoutModule->paperheightUnitCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(pageLayoutModule->portraitRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(pageLayoutModule->landscapeRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(pageLayoutModule->facingPagesCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(pageLayoutModule->pagestyleCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(pageLayoutModule->backgroundTB, SIGNAL(clicked()),
        this, SLOT(changeBackgroundColor()));
    connect(pageLayoutModule->delbackgroundTB, SIGNAL(clicked()),
        this, SLOT(deleteBackgroundColor()));

	pageLayoutModule->pagestyleCO->addItem(qt_("Default"));
	pageLayoutModule->pagestyleCO->addItem(qt_("empty"));
	pageLayoutModule->pagestyleCO->addItem(qt_("plain"));
	pageLayoutModule->pagestyleCO->addItem(qt_("headings"));
	pageLayoutModule->pagestyleCO->addItem(qt_("fancy"));
	bc().addCheckedLineEdit(pageLayoutModule->paperheightLE,
		pageLayoutModule->paperheightL);
	bc().addCheckedLineEdit(pageLayoutModule->paperwidthLE,
		pageLayoutModule->paperwidthL);

	// paper
	QComboBox * cb = pageLayoutModule->papersizeCO;
	cb->addItem(qt_("Default"));
	cb->addItem(qt_("Custom"));
	cb->addItem(qt_("US letter"));
	cb->addItem(qt_("US legal"));
	cb->addItem(qt_("US executive"));
	cb->addItem(qt_("A3"));
	cb->addItem(qt_("A4"));
	cb->addItem(qt_("A5"));
	cb->addItem(qt_("B3"));
	cb->addItem(qt_("B4"));
	cb->addItem(qt_("B5"));
	// remove the %-items from the unit choice
	pageLayoutModule->paperwidthUnitCO->noPercents();
	pageLayoutModule->paperheightUnitCO->noPercents();
	pageLayoutModule->paperheightLE->setValidator(unsignedLengthValidator(
		pageLayoutModule->paperheightLE));
	pageLayoutModule->paperwidthLE->setValidator(unsignedLengthValidator(
		pageLayoutModule->paperwidthLE));


	marginsModule = new UiWidget<Ui::MarginsUi>;
	// margins
	connect(marginsModule->marginCB, SIGNAL(toggled(bool)),
		this, SLOT(setCustomMargins(bool)));
	connect(marginsModule->marginCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(marginsModule->topLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->topUnit, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->bottomLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->bottomUnit, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->innerLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->innerUnit, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->outerLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->outerUnit, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->headheightLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->headheightUnit, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->headsepLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->headsepUnit, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->footskipLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->footskipUnit, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->columnsepLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->columnsepUnit, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	marginsModule->topLE->setValidator(unsignedLengthValidator(
		marginsModule->topLE));
	marginsModule->bottomLE->setValidator(unsignedLengthValidator(
		marginsModule->bottomLE));
	marginsModule->innerLE->setValidator(unsignedLengthValidator(
		marginsModule->innerLE));
	marginsModule->outerLE->setValidator(unsignedLengthValidator(
		marginsModule->outerLE));
	marginsModule->headsepLE->setValidator(unsignedLengthValidator(
		marginsModule->headsepLE));
	marginsModule->headheightLE->setValidator(unsignedLengthValidator(
		marginsModule->headheightLE));
	marginsModule->footskipLE->setValidator(unsignedLengthValidator(
		marginsModule->footskipLE));
	marginsModule->columnsepLE->setValidator(unsignedLengthValidator(
		marginsModule->columnsepLE));

	bc().addCheckedLineEdit(marginsModule->topLE,
		marginsModule->topL);
	bc().addCheckedLineEdit(marginsModule->bottomLE,
		marginsModule->bottomL);
	bc().addCheckedLineEdit(marginsModule->innerLE,
		marginsModule->innerL);
	bc().addCheckedLineEdit(marginsModule->outerLE,
		marginsModule->outerL);
	bc().addCheckedLineEdit(marginsModule->headsepLE,
		marginsModule->headsepL);
	bc().addCheckedLineEdit(marginsModule->headheightLE,
		marginsModule->headheightL);
	bc().addCheckedLineEdit(marginsModule->footskipLE,
		marginsModule->footskipL);
	bc().addCheckedLineEdit(marginsModule->columnsepLE,
		marginsModule->columnsepL);


	langModule = new UiWidget<Ui::LanguageUi>;
	// language & quote
	connect(langModule->languageCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(langModule->defaultencodingRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(langModule->otherencodingRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(langModule->encodingCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(langModule->quoteStyleCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	// language & quotes
	QAbstractItemModel * language_model = guiApp->languageModel();
	// FIXME: it would be nice if sorting was enabled/disabled via a checkbox.
	language_model->sort(0);
	langModule->languageCO->setModel(language_model);

	// Always put the default encoding in the first position.
	langModule->encodingCO->addItem(qt_("Language Default (no inputenc)"));
	QStringList encodinglist;
	Encodings::const_iterator it = encodings.begin();
	Encodings::const_iterator const end = encodings.end();
	for (; it != end; ++it)
		encodinglist.append(qt_(it->guiName()));
	encodinglist.sort();
	langModule->encodingCO->addItems(encodinglist);

	langModule->quoteStyleCO->addItem(qt_("``text''"));
	langModule->quoteStyleCO->addItem(qt_("''text''"));
	langModule->quoteStyleCO->addItem(qt_(",,text``"));
	langModule->quoteStyleCO->addItem(qt_(",,text''"));
	langModule->quoteStyleCO->addItem(qt_("<<text>>"));
	langModule->quoteStyleCO->addItem(qt_(">>text<<"));


	numberingModule = new UiWidget<Ui::NumberingUi>;
	// numbering
	connect(numberingModule->depthSL, SIGNAL(valueChanged(int)),
		this, SLOT(change_adaptor()));
	connect(numberingModule->tocSL, SIGNAL(valueChanged(int)),
		this, SLOT(change_adaptor()));
	connect(numberingModule->depthSL, SIGNAL(valueChanged(int)),
		this, SLOT(updateNumbering()));
	connect(numberingModule->tocSL, SIGNAL(valueChanged(int)),
		this, SLOT(updateNumbering()));
	numberingModule->tocTW->setColumnCount(3);
	numberingModule->tocTW->headerItem()->setText(0, qt_("Example"));
	numberingModule->tocTW->headerItem()->setText(1, qt_("Numbered"));
	numberingModule->tocTW->headerItem()->setText(2, qt_("Appears in TOC"));


	biblioModule = new UiWidget<Ui::BiblioUi>;
	connect(biblioModule->citeNatbibRB, SIGNAL(toggled(bool)),
		biblioModule->citationStyleL, SLOT(setEnabled(bool)));
	connect(biblioModule->citeNatbibRB, SIGNAL(toggled(bool)),
		biblioModule->citeStyleCO, SLOT(setEnabled(bool)));
	// biblio
	connect(biblioModule->citeDefaultRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(biblioModule->citeNatbibRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(biblioModule->citeStyleCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(biblioModule->citeJurabibRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(biblioModule->bibtopicCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	// biblio
	biblioModule->citeStyleCO->addItem(qt_("Author-year"));
	biblioModule->citeStyleCO->addItem(qt_("Numerical"));
	biblioModule->citeStyleCO->setCurrentIndex(0);


	mathsModule = new UiWidget<Ui::MathsUi>;
	connect(mathsModule->amsautoCB, SIGNAL(toggled(bool)),
		mathsModule->amsCB, SLOT(setDisabled(bool)));
	connect(mathsModule->esintautoCB, SIGNAL(toggled(bool)),
		mathsModule->esintCB, SLOT(setDisabled(bool)));
	// maths
	connect(mathsModule->amsCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(mathsModule->amsautoCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(mathsModule->esintCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(mathsModule->esintautoCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));

	latexModule = new UiWidget<Ui::LaTeXUi>;
	// latex class
	connect(latexModule->optionsLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(latexModule->defaultOptionsCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(latexModule->psdriverCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(latexModule->classCO, SIGNAL(activated(int)),
		this, SLOT(classChanged()));
	connect(latexModule->classCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(latexModule->layoutPB, SIGNAL(clicked()),
		this, SLOT(browseLayout()));
	connect(latexModule->layoutPB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(latexModule->childDocGB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(latexModule->childDocLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(latexModule->childDocPB, SIGNAL(clicked()),
		this, SLOT(browseMaster()));

	// postscript drivers
	for (int n = 0; tex_graphics[n][0]; ++n) {
		QString enc = qt_(tex_graphics_gui[n]);
		latexModule->psdriverCO->addItem(enc);
	}
	// latex classes
	latexModule->classCO->setModel(&classes_model_);
	LayoutFileList const & bcl = LayoutFileList::get();
	vector<LayoutFileIndex> classList = bcl.classList();
	sort(classList.begin(), classList.end(), less_textclass_avail_desc());

	vector<LayoutFileIndex>::const_iterator cit  = classList.begin();
	vector<LayoutFileIndex>::const_iterator cen = classList.end();
	for (int i = 0; cit != cen; ++cit, ++i) {
		LayoutFile const & tc = bcl[*cit];
		docstring item = (tc.isTeXClassAvailable()) ?
			from_utf8(tc.description()) :
			bformat(_("Unavailable: %1$s"), from_utf8(tc.description()));
		classes_model_.insertRow(i, toqstr(item), *cit);
	}

	// branches
	branchesModule = new GuiBranches;
	connect(branchesModule, SIGNAL(changed()),
		this, SLOT(change_adaptor()));

	// preamble
	preambleModule = new PreambleModule;
	connect(preambleModule, SIGNAL(changed()),
		this, SLOT(change_adaptor()));

	// bullets
	bulletsModule = new BulletsModule;
	connect(bulletsModule, SIGNAL(changed()),
		this, SLOT(change_adaptor()));

	// Modules
	modulesModule = new UiWidget<Ui::ModulesUi>;

	selectionManager =
		new ModuleSelectionManager(modulesModule->availableLV,
			modulesModule->selectedLV,
			modulesModule->addPB, modulesModule->deletePB,
			modulesModule->upPB, modulesModule->downPB,
			availableModel(), selectedModel(), this);
	connect(selectionManager, SIGNAL(updateHook()),
		this, SLOT(updateModuleInfo()));
	connect(selectionManager, SIGNAL(updateHook()),
		this, SLOT(change_adaptor()));
	connect(selectionManager, SIGNAL(selectionChanged()),
		this, SLOT(modulesChanged()));

	// PDF support
	pdfSupportModule = new UiWidget<Ui::PDFSupportUi>;

	connect(pdfSupportModule->use_hyperrefGB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->titleLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->authorLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->subjectLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->keywordsLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->bookmarksGB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->bookmarksnumberedCB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->bookmarksopenGB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->bookmarksopenlevelSB, SIGNAL(valueChanged(int)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->breaklinksCB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->pdfborderCB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->colorlinksCB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->backrefCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->pdfusetitleCB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->fullscreenCB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->optionsLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));

	for (int i = 0; backref_opts[i][0]; ++i)
		pdfSupportModule->backrefCO->addItem(qt_(backref_opts_gui[i]));

	// float
	floatModule = new FloatPlacement;
	connect(floatModule, SIGNAL(changed()),
		this, SLOT(change_adaptor()));

	docPS->addPanel(latexModule, qt_("Document Class"));
	docPS->addPanel(modulesModule, qt_("Modules"));
	docPS->addPanel(fontModule, qt_("Fonts"));
	docPS->addPanel(textLayoutModule, qt_("Text Layout"));
	docPS->addPanel(pageLayoutModule, qt_("Page Layout"));
	docPS->addPanel(marginsModule, qt_("Page Margins"));
	docPS->addPanel(langModule, qt_("Language"));
	docPS->addPanel(numberingModule, qt_("Numbering & TOC"));
	docPS->addPanel(biblioModule, qt_("Bibliography"));
	docPS->addPanel(pdfSupportModule, qt_("PDF Properties"));
	docPS->addPanel(mathsModule, qt_("Math Options"));
	docPS->addPanel(floatModule, qt_("Float Placement"));
	docPS->addPanel(bulletsModule, qt_("Bullets"));
	docPS->addPanel(branchesModule, qt_("Branches"));
	docPS->addPanel(outputModule, qt_("Output"));
	docPS->addPanel(preambleModule, qt_("LaTeX Preamble"));
	docPS->setCurrentPanel(qt_("Document Class"));
// FIXME: hack to work around resizing bug in Qt >= 4.2
// bug verified with Qt 4.2.{0-3} (JSpitzm)
#if QT_VERSION >= 0x040200
	docPS->updateGeometry();
#endif
}


void GuiDocument::showPreamble()
{
	docPS->setCurrentPanel(qt_("LaTeX Preamble"));
}


void GuiDocument::saveDefaultClicked()
{
	saveDocDefault();
}


void GuiDocument::useDefaultsClicked()
{
	useClassDefaults();
}


void GuiDocument::change_adaptor()
{
	changed();
}


QString GuiDocument::validateListingsParameters()
{
	// use a cache here to avoid repeated validation
	// of the same parameters
	static string param_cache;
	static QString msg_cache;
	
	if (textLayoutModule->bypassCB->isChecked())
		return QString();

	string params = fromqstr(textLayoutModule->listingsED->toPlainText());
	if (params != param_cache) {
		param_cache = params;
		msg_cache = toqstr(InsetListingsParams(params).validate());
	}
	return msg_cache;
}


void GuiDocument::setListingsMessage()
{
	static bool isOK = true;
	QString msg = validateListingsParameters();
	if (msg.isEmpty()) {
		if (isOK)
			return;
		isOK = true;
		// listingsTB->setTextColor("black");
		textLayoutModule->listingsTB->setPlainText(
			qt_("Input listings parameters on the right. "
                "Enter ? for a list of parameters."));
	} else {
		isOK = false;
		// listingsTB->setTextColor("red");
		textLayoutModule->listingsTB->setPlainText(msg);
	}
}


void GuiDocument::setLSpacing(int item)
{
	textLayoutModule->lspacingLE->setEnabled(item == 3);
}


void GuiDocument::setSkip(int item)
{
	bool const enable = (item == 3);
	textLayoutModule->skipLE->setEnabled(enable);
	textLayoutModule->skipLengthCO->setEnabled(enable);
}


void GuiDocument::enableSkip(bool skip)
{
	textLayoutModule->skipCO->setEnabled(skip);
	textLayoutModule->skipLE->setEnabled(skip);
	textLayoutModule->skipLengthCO->setEnabled(skip);
	if (skip)
		setSkip(textLayoutModule->skipCO->currentIndex());
}


void GuiDocument::portraitChanged()
{
	setMargins(pageLayoutModule->papersizeCO->currentIndex());
}


void GuiDocument::setMargins(bool custom)
{
	bool const extern_geometry =
		documentClass().provides("geometry");
	marginsModule->marginCB->setEnabled(!extern_geometry);
	if (extern_geometry) {
		marginsModule->marginCB->setChecked(false);
		setCustomMargins(true);
		return;
	}
	marginsModule->marginCB->setChecked(custom);
	setCustomMargins(custom);
}


void GuiDocument::setCustomPapersize(int papersize)
{
	bool const custom = (papersize == 1);

	pageLayoutModule->paperwidthL->setEnabled(custom);
	pageLayoutModule->paperwidthLE->setEnabled(custom);
	pageLayoutModule->paperwidthUnitCO->setEnabled(custom);
	pageLayoutModule->paperheightL->setEnabled(custom);
	pageLayoutModule->paperheightLE->setEnabled(custom);
	pageLayoutModule->paperheightLE->setFocus();
	pageLayoutModule->paperheightUnitCO->setEnabled(custom);
}


void GuiDocument::setColSep()
{
	setCustomMargins(marginsModule->marginCB->checkState() == Qt::Checked);
}


void GuiDocument::setCustomMargins(bool custom)
{
	marginsModule->topL->setEnabled(!custom);
	marginsModule->topLE->setEnabled(!custom);
	marginsModule->topUnit->setEnabled(!custom);

	marginsModule->bottomL->setEnabled(!custom);
	marginsModule->bottomLE->setEnabled(!custom);
	marginsModule->bottomUnit->setEnabled(!custom);

	marginsModule->innerL->setEnabled(!custom);
	marginsModule->innerLE->setEnabled(!custom);
	marginsModule->innerUnit->setEnabled(!custom);

	marginsModule->outerL->setEnabled(!custom);
	marginsModule->outerLE->setEnabled(!custom);
	marginsModule->outerUnit->setEnabled(!custom);

	marginsModule->headheightL->setEnabled(!custom);
	marginsModule->headheightLE->setEnabled(!custom);
	marginsModule->headheightUnit->setEnabled(!custom);

	marginsModule->headsepL->setEnabled(!custom);
	marginsModule->headsepLE->setEnabled(!custom);
	marginsModule->headsepUnit->setEnabled(!custom);

	marginsModule->footskipL->setEnabled(!custom);
	marginsModule->footskipLE->setEnabled(!custom);
	marginsModule->footskipUnit->setEnabled(!custom);

	bool const enableColSep = !custom && 
			textLayoutModule->twoColumnCB->checkState() == Qt::Checked;
	marginsModule->columnsepL->setEnabled(enableColSep);
	marginsModule->columnsepLE->setEnabled(enableColSep);
	marginsModule->columnsepUnit->setEnabled(enableColSep);
}

void GuiDocument::changeBackgroundColor()
{
	const QColor newColor = QColorDialog::getColor(
		rgb2qcolor(set_backgroundcolor), qApp->focusWidget());
	if (!newColor.isValid())
		return;
	// set the button color
	pageLayoutModule->backgroundTB->setStyleSheet(
		colorButtonStyleSheet(newColor));
	// save white as the set color
	set_backgroundcolor = rgbFromHexName(fromqstr(newColor.name()));
	changed();
}


void GuiDocument::deleteBackgroundColor()
{
	// set the button color back to white
	pageLayoutModule->backgroundTB->setStyleSheet(
		colorButtonStyleSheet(QColor(Qt::white)));
	// save white as the set color
	set_backgroundcolor = rgbFromHexName("#ffffff");
	changed();
}


void GuiDocument::xetexChanged(bool xetex)
{
	updateFontlist();
	updateDefaultFormat();
	langModule->encodingCO->setEnabled(!xetex &&
		!langModule->defaultencodingRB->isChecked());
	langModule->defaultencodingRB->setEnabled(!xetex);
	langModule->otherencodingRB->setEnabled(!xetex);

	fontModule->fontsDefaultCO->setEnabled(!xetex);
	fontModule->fontsDefaultLA->setEnabled(!xetex);
	fontModule->cjkFontLE->setEnabled(!xetex);
	fontModule->cjkFontLA->setEnabled(!xetex);
	string font;
	if (!xetex)
		font = tex_fonts_sans[fontModule->fontsSansCO->currentIndex()];
	bool scaleable = providesScale(font);
	fontModule->scaleSansSB->setEnabled(scaleable);
	fontModule->scaleSansLA->setEnabled(scaleable);
	if (!xetex)
		font = tex_fonts_monospaced[fontModule->fontsTypewriterCO->currentIndex()];
	scaleable = providesScale(font);
	fontModule->scaleTypewriterSB->setEnabled(scaleable);
	fontModule->scaleTypewriterLA->setEnabled(scaleable);
	if (!xetex)
		font = tex_fonts_roman[fontModule->fontsRomanCO->currentIndex()];
	fontModule->fontScCB->setEnabled(providesSC(font));
	fontModule->fontOsfCB->setEnabled(providesOSF(font));
}


void GuiDocument::updateFontsize(string const & items, string const & sel)
{
	fontModule->fontsizeCO->clear();
	fontModule->fontsizeCO->addItem(qt_("Default"));

	for (int n = 0; !token(items,'|',n).empty(); ++n)
		fontModule->fontsizeCO->
			addItem(toqstr(token(items,'|',n)));

	for (int n = 0; n < fontModule->fontsizeCO->count(); ++n) {
		if (fromqstr(fontModule->fontsizeCO->itemText(n)) == sel) {
			fontModule->fontsizeCO->setCurrentIndex(n);
			break;
		}
	}
}


void GuiDocument::updateFontlist()
{
	fontModule->fontsRomanCO->clear();
	fontModule->fontsSansCO->clear();
	fontModule->fontsTypewriterCO->clear();

	// With XeTeX, we have access to all system fonts, but not the LaTeX fonts
	if (outputModule->xetexCB->isChecked()) {
		fontModule->fontsRomanCO->addItem(qt_("Default"));
		fontModule->fontsSansCO->addItem(qt_("Default"));
		fontModule->fontsTypewriterCO->addItem(qt_("Default"));
	
		QFontDatabase fontdb;
		QStringList families(fontdb.families());
		for (QStringList::Iterator it = families.begin(); it != families.end(); ++it) {
			fontModule->fontsRomanCO->addItem(*it);
			fontModule->fontsSansCO->addItem(*it);
			fontModule->fontsTypewriterCO->addItem(*it);
		}
		return;
	}

	for (int n = 0; tex_fonts_roman[n][0]; ++n) {
		QString font = qt_(tex_fonts_roman_gui[n]);
		if (!isFontAvailable(tex_fonts_roman[n]))
			font += qt_(" (not installed)");
		fontModule->fontsRomanCO->addItem(font);
	}
	for (int n = 0; tex_fonts_sans[n][0]; ++n) {
		QString font = qt_(tex_fonts_sans_gui[n]);
		if (!isFontAvailable(tex_fonts_sans[n]))
			font += qt_(" (not installed)");
		fontModule->fontsSansCO->addItem(font);
	}
	for (int n = 0; tex_fonts_monospaced[n][0]; ++n) {
		QString font = qt_(tex_fonts_monospaced_gui[n]);
		if (!isFontAvailable(tex_fonts_monospaced[n]))
			font += qt_(" (not installed)");
		fontModule->fontsTypewriterCO->addItem(font);
	}
}


void GuiDocument::romanChanged(int item)
{
	if (outputModule->xetexCB->isChecked())
		return;
	string const font = tex_fonts_roman[item];
	fontModule->fontScCB->setEnabled(providesSC(font));
	fontModule->fontOsfCB->setEnabled(providesOSF(font));
}


void GuiDocument::sansChanged(int item)
{
	if (outputModule->xetexCB->isChecked())
		return;
	string const font = tex_fonts_sans[item];
	bool scaleable = providesScale(font);
	fontModule->scaleSansSB->setEnabled(scaleable);
	fontModule->scaleSansLA->setEnabled(scaleable);
}


void GuiDocument::ttChanged(int item)
{
	if (outputModule->xetexCB->isChecked())
		return;
	string const font = tex_fonts_monospaced[item];
	bool scaleable = providesScale(font);
	fontModule->scaleTypewriterSB->setEnabled(scaleable);
	fontModule->scaleTypewriterLA->setEnabled(scaleable);
}


void GuiDocument::updatePagestyle(string const & items, string const & sel)
{
	pagestyles.clear();
	pageLayoutModule->pagestyleCO->clear();
	pageLayoutModule->pagestyleCO->addItem(qt_("Default"));

	for (int n = 0; !token(items, '|', n).empty(); ++n) {
		string style = token(items, '|', n);
		QString style_gui = qt_(style);
		pagestyles.push_back(pair<string, QString>(style, style_gui));
		pageLayoutModule->pagestyleCO->addItem(style_gui);
	}

	if (sel == "default") {
		pageLayoutModule->pagestyleCO->setCurrentIndex(0);
		return;
	}

	int nn = 0;

	for (size_t i = 0; i < pagestyles.size(); ++i)
		if (pagestyles[i].first == sel)
			nn = pageLayoutModule->pagestyleCO->findText(pagestyles[i].second);

	if (nn > 0)
		pageLayoutModule->pagestyleCO->setCurrentIndex(nn);
}


void GuiDocument::browseLayout()
{
	QString const label1 = qt_("Layouts|#o#O");
	QString const dir1 = toqstr(lyxrc.document_path);
	QStringList const filter(qt_("LyX Layout (*.layout)"));
	QString file = browseRelFile(QString(), bufferFilepath(),
		qt_("Local layout file"), filter, false,
		label1, dir1);

	if (!file.endsWith(".layout"))
		return;

	FileName layoutFile = support::makeAbsPath(fromqstr(file),
		fromqstr(bufferFilepath()));
	
	int const ret = Alert::prompt(_("Local layout file"),
		_("The layout file you have selected is a local layout\n"
		  "file, not one in the system or user directory. Your\n"
		  "document may not work with this layout if you do not\n"
		  "keep the layout file in the document directory."),
		  1, 1, _("&Set Layout"), _("&Cancel"));
	if (ret == 1)
		return;

	// load the layout file
	LayoutFileList & bcl = LayoutFileList::get();
	string classname = layoutFile.onlyFileName();
	// this will update an existing layout if that layout has been loaded before.
	LayoutFileIndex name = bcl.addLocalLayout(
		classname.substr(0, classname.size() - 7),
		layoutFile.onlyPath().absFilename());

	if (name.empty()) {
		Alert::error(_("Error"),
			_("Unable to read local layout file."));		
		return;
	}

	// do not trigger classChanged if there is no change.
	if (latexModule->classCO->currentText() == toqstr(name))
		return;
		
	// add to combo box
	int idx = latexModule->classCO->findText(toqstr(name));
	if (idx == -1) {
		classes_model_.insertRow(0, toqstr(name), name);
		latexModule->classCO->setCurrentIndex(0);
	} else
		latexModule->classCO->setCurrentIndex(idx);
	
	classChanged();
}


void GuiDocument::browseMaster()
{
	QString const title = qt_("Select master document");
	QString const dir1 = toqstr(lyxrc.document_path);
	QString const old = latexModule->childDocLE->text();
	QString const docpath = toqstr(support::onlyPath(buffer().absFileName()));
	QStringList const filter(qt_("LyX Files (*.lyx)"));
	QString file = browseRelFile(old, docpath, title, filter, false,
		qt_("Documents|#o#O"), toqstr(lyxrc.document_path));

	latexModule->childDocLE->setText(file);
}


void GuiDocument::classChanged()
{
	int idx = latexModule->classCO->currentIndex();
	if (idx < 0) 
		return;
	string const classname = classes_model_.getIDString(idx);

	// check whether the selected modules have changed.
	bool modules_changed = false;
	unsigned int const srows = selectedModel()->rowCount();
	if (srows != bp_.getModules().size())
		modules_changed = true;
	else {
		list<string>::const_iterator mit = bp_.getModules().begin();
		list<string>::const_iterator men = bp_.getModules().end();
		for (unsigned int i = 0; i < srows && mit != men; ++i, ++mit)
			if (selectedModel()->getIDString(i) != *mit) {
				modules_changed = true;
				break;
			}
	}

	if (modules_changed || lyxrc.auto_reset_options) {
		if (applyPB->isEnabled()) {
			int const ret = Alert::prompt(_("Unapplied changes"),
					_("Some changes in the dialog were not yet applied.\n"
					"If you do not apply now, they will be lost after this action."),
					1, 1, _("&Apply"), _("&Dismiss"));
			if (ret == 0)
				applyView();
		}
	}

	// We load the TextClass as soon as it is selected. This is
	// necessary so that other options in the dialog can be updated
	// according to the new class. Note, however, that, if you use 
	// the scroll wheel when sitting on the combo box, we'll load a 
	// lot of TextClass objects very quickly....
	if (!bp_.setBaseClass(classname)) {
		Alert::error(_("Error"), _("Unable to set document class."));
		return;
	}
	if (lyxrc.auto_reset_options)
		bp_.useClassDefaults();

	// With the introduction of modules came a distinction between the base 
	// class and the document class. The former corresponds to the main layout 
	// file; the latter is that plus the modules (or the document-specific layout,
	// or  whatever else there could be). Our parameters come from the document 
	// class. So when we set the base class, we also need to recreate the document 
	// class. Otherwise, we still have the old one.
	bp_.makeDocumentClass();
	paramsToDialog();
}


namespace {
	// This is an insanely complicated attempt to make this sort of thing
	// work with RTL languages.
	docstring formatStrVec(vector<string> const & v, docstring const & s) 
	{
		//this mess formats the list as "v[0], v[1], ..., [s] v[n]"
		if (v.size() == 0)
			return docstring();
		if (v.size() == 1) 
			return from_utf8(v[0]);
		if (v.size() == 2) {
			docstring retval = _("%1$s and %2$s");
			retval = subst(retval, _("and"), s);
			return bformat(retval, from_utf8(v[0]), from_utf8(v[1]));
		}
		// The idea here is to format all but the last two items...
		int const vSize = v.size();
		docstring t2 = _("%1$s, %2$s");
		docstring retval = from_utf8(v[0]);
		for (int i = 1; i < vSize - 2; ++i)
			retval = bformat(t2, retval, from_utf8(v[i])); 
		//...and then to  plug them, and the last two, into this schema
		docstring t = _("%1$s, %2$s, and %3$s");
		t = subst(t, _("and"), s);
		return bformat(t, retval, from_utf8(v[vSize - 2]), from_utf8(v[vSize - 1]));
	}
	
	vector<string> idsToNames(vector<string> const & idList)
	{
		vector<string> retval;
		vector<string>::const_iterator it  = idList.begin();
		vector<string>::const_iterator end = idList.end();
		for (; it != end; ++it) {
			LyXModule const * const mod = moduleList[*it];
			if (!mod)
				retval.push_back(*it + " (Unavailable)");
			else
				retval.push_back(mod->getName());
		}
		return retval;
	}
}


void GuiDocument::modulesToParams(BufferParams & bp)
{
	// update list of loaded modules
	bp.clearLayoutModules();
	int const srows = modules_sel_model_.rowCount();
	for (int i = 0; i < srows; ++i)
		bp.addLayoutModule(modules_sel_model_.getIDString(i));

	// update the list of removed modules
	bp.clearRemovedModules();
	LayoutModuleList const & reqmods = bp.baseClass()->defaultModules();
	list<string>::const_iterator rit = reqmods.begin();
	list<string>::const_iterator ren = reqmods.end();

	// check each of the default modules
	for (; rit != ren; rit++) {
		list<string>::const_iterator mit = bp.getModules().begin();
		list<string>::const_iterator men = bp.getModules().end();
		bool found = false;
		for (; mit != men; mit++) {
			if (*rit == *mit) {
				found = true;
				break;
			}
		}
		if (!found) {
			// the module isn't present so must have been removed by the user
			bp.addRemovedModule(*rit);
		}
	}
}

void GuiDocument::modulesChanged()
{
	modulesToParams(bp_);
	bp_.makeDocumentClass();
	paramsToDialog();
}


void GuiDocument::updateModuleInfo()
{
	selectionManager->update();
	
	//Module description
	bool const focus_on_selected = selectionManager->selectedFocused();
	QListView const * const lv = 
			focus_on_selected ? modulesModule->selectedLV : modulesModule->availableLV;
	if (lv->selectionModel()->selectedIndexes().isEmpty()) {
		modulesModule->infoML->document()->clear();
		return;
	}
	QModelIndex const & idx = lv->selectionModel()->currentIndex();
	GuiIdListModel const & id_model = 
			focus_on_selected  ? modules_sel_model_ : modules_av_model_;
	string const modName = id_model.getIDString(idx.row());
	docstring desc = getModuleDescription(modName);

	LayoutModuleList const & provmods = bp_.baseClass()->providedModules();
	if (std::find(provmods.begin(), provmods.end(), modName) != provmods.end()) {
		if (!desc.empty())
			desc += "\n";
		desc += _("Module provided by document class.");
	}

	vector<string> pkglist = getPackageList(modName);
	docstring pkgdesc = formatStrVec(pkglist, _("and"));
	if (!pkgdesc.empty()) {
		if (!desc.empty())
			desc += "\n";
		desc += bformat(_("Package(s) required: %1$s."), pkgdesc);
	}

	pkglist = getRequiredList(modName);
	if (!pkglist.empty()) {
		vector<string> const reqdescs = idsToNames(pkglist);
		pkgdesc = formatStrVec(reqdescs, _("or"));
		if (!desc.empty())
			desc += "\n";
		desc += bformat(_("Module required: %1$s."), pkgdesc);
	}

	pkglist = getExcludedList(modName);
	if (!pkglist.empty()) {
		vector<string> const reqdescs = idsToNames(pkglist);
		pkgdesc = formatStrVec(reqdescs, _( "and"));
		if (!desc.empty())
			desc += "\n";
		desc += bformat(_("Modules excluded: %1$s."), pkgdesc);
	}

	if (!isModuleAvailable(modName)) {
		if (!desc.empty())
			desc += "\n";
		desc += _("WARNING: Some required packages are unavailable!");
	}

	modulesModule->infoML->document()->setPlainText(toqstr(desc));
}


void GuiDocument::updateNumbering()
{
	DocumentClass const & tclass = documentClass();

	numberingModule->tocTW->setUpdatesEnabled(false);
	numberingModule->tocTW->clear();

	int const depth = numberingModule->depthSL->value();
	int const toc = numberingModule->tocSL->value();
	QString const no = qt_("No");
	QString const yes = qt_("Yes");
	QTreeWidgetItem * item = 0;

	DocumentClass::const_iterator lit = tclass.begin();
	DocumentClass::const_iterator len = tclass.end();
	for (; lit != len; ++lit) {
		int const toclevel = lit->toclevel;
		if (toclevel != Layout::NOT_IN_TOC && lit->labeltype == LABEL_COUNTER) {
			item = new QTreeWidgetItem(numberingModule->tocTW);
			item->setText(0, toqstr(translateIfPossible(lit->name())));
			item->setText(1, (toclevel <= depth) ? yes : no);
			item->setText(2, (toclevel <= toc) ? yes : no);
		}
	}

	numberingModule->tocTW->setUpdatesEnabled(true);
	numberingModule->tocTW->update();
}


void GuiDocument::updateDefaultFormat()
{
	// make a copy in order to consider unapplied changes
	Buffer * tmpbuf = const_cast<Buffer *>(&buffer());
	tmpbuf->params().useXetex = outputModule->xetexCB->isChecked();
	int idx = latexModule->classCO->currentIndex();
	if (idx >= 0) {
		string const classname = classes_model_.getIDString(idx);
		tmpbuf->params().setBaseClass(classname);
		tmpbuf->params().makeDocumentClass();
	}
	outputModule->defaultFormatCO->blockSignals(true);
	outputModule->defaultFormatCO->clear();
	outputModule->defaultFormatCO->addItem(qt_("Default"),
				QVariant(QString("default")));
	typedef vector<Format const *> Formats;
	Formats formats = tmpbuf->exportableFormats(true);
	Formats::const_iterator cit = formats.begin();
	Formats::const_iterator end = formats.end();
	for (; cit != end; ++cit)
		outputModule->defaultFormatCO->addItem(qt_((*cit)->prettyname()),
				QVariant(toqstr((*cit)->name())));
	outputModule->defaultFormatCO->blockSignals(false);
}


void GuiDocument::applyView()
{
	// preamble
	preambleModule->apply(bp_);

	// biblio
	bp_.setCiteEngine(ENGINE_BASIC);

	if (biblioModule->citeNatbibRB->isChecked()) {
		bool const use_numerical_citations =
			biblioModule->citeStyleCO->currentIndex();
		if (use_numerical_citations)
			bp_.setCiteEngine(ENGINE_NATBIB_NUMERICAL);
		else
			bp_.setCiteEngine(ENGINE_NATBIB_AUTHORYEAR);

	} else if (biblioModule->citeJurabibRB->isChecked())
		bp_.setCiteEngine(ENGINE_JURABIB);

	bp_.use_bibtopic =
		biblioModule->bibtopicCB->isChecked();

	// language & quotes
	if (langModule->defaultencodingRB->isChecked()) {
		bp_.inputenc = "auto";
	} else {
		int i = langModule->encodingCO->currentIndex();
		if (i == 0)
			bp_.inputenc = "default";
		else {
			QString const enc_gui =
				langModule->encodingCO->currentText();
			Encodings::const_iterator it = encodings.begin();
			Encodings::const_iterator const end = encodings.end();
			bool found = false;
			for (; it != end; ++it) {
				if (qt_(it->guiName()) == enc_gui) {
					bp_.inputenc = it->latexName();
					found = true;
					break;
				}
			}
			if (!found) {
				// should not happen
				lyxerr << "GuiDocument::apply: Unknown encoding! Resetting to default" << endl;
				bp_.inputenc = "default";
			}
		}
	}

	InsetQuotes::QuoteLanguage lga = InsetQuotes::EnglishQuotes;
	switch (langModule->quoteStyleCO->currentIndex()) {
	case 0:
		lga = InsetQuotes::EnglishQuotes;
		break;
	case 1:
		lga = InsetQuotes::SwedishQuotes;
		break;
	case 2:
		lga = InsetQuotes::GermanQuotes;
		break;
	case 3:
		lga = InsetQuotes::PolishQuotes;
		break;
	case 4:
		lga = InsetQuotes::FrenchQuotes;
		break;
	case 5:
		lga = InsetQuotes::DanishQuotes;
		break;
	}
	bp_.quotes_language = lga;

	QString const lang = langModule->languageCO->itemData(
		langModule->languageCO->currentIndex()).toString();
	bp_.language = languages.getLanguage(fromqstr(lang));

	// numbering
	if (bp_.documentClass().hasTocLevels()) {
		bp_.tocdepth = numberingModule->tocSL->value();
		bp_.secnumdepth = numberingModule->depthSL->value();
	}

	// bullets
	bp_.user_defined_bullet(0) = bulletsModule->bullet(0);
	bp_.user_defined_bullet(1) = bulletsModule->bullet(1);
	bp_.user_defined_bullet(2) = bulletsModule->bullet(2);
	bp_.user_defined_bullet(3) = bulletsModule->bullet(3);

	// packages
	bp_.graphicsDriver =
		tex_graphics[latexModule->psdriverCO->currentIndex()];
	
	// text layout
	int idx = latexModule->classCO->currentIndex();
	if (idx >= 0) {
		string const classname = classes_model_.getIDString(idx);
		bp_.setBaseClass(classname);
	}

	// Modules
	modulesToParams(bp_);

	if (mathsModule->amsautoCB->isChecked()) {
		bp_.use_amsmath = BufferParams::package_auto;
	} else {
		if (mathsModule->amsCB->isChecked())
			bp_.use_amsmath = BufferParams::package_on;
		else
			bp_.use_amsmath = BufferParams::package_off;
	}

	if (mathsModule->esintautoCB->isChecked())
		bp_.use_esint = BufferParams::package_auto;
	else {
		if (mathsModule->esintCB->isChecked())
			bp_.use_esint = BufferParams::package_on;
		else
			bp_.use_esint = BufferParams::package_off;
	}

	if (pageLayoutModule->pagestyleCO->currentIndex() == 0)
		bp_.pagestyle = "default";
	else {
		QString style_gui = pageLayoutModule->pagestyleCO->currentText();
		for (size_t i = 0; i != pagestyles.size(); ++i)
			if (pagestyles[i].second == style_gui)
				bp_.pagestyle = pagestyles[i].first;
	}

	switch (textLayoutModule->lspacingCO->currentIndex()) {
	case 0:
		bp_.spacing().set(Spacing::Single);
		break;
	case 1:
		bp_.spacing().set(Spacing::Onehalf);
		break;
	case 2:
		bp_.spacing().set(Spacing::Double);
		break;
	case 3:
		bp_.spacing().set(Spacing::Other,
			fromqstr(textLayoutModule->lspacingLE->text()));
		break;
	}

	if (textLayoutModule->twoColumnCB->isChecked())
		bp_.columns = 2;
	else
		bp_.columns = 1;

	// text should have passed validation
	bp_.listings_params =
		InsetListingsParams(fromqstr(textLayoutModule->listingsED->toPlainText())).params();

	if (textLayoutModule->indentRB->isChecked())
		bp_.paragraph_separation = BufferParams::ParagraphIndentSeparation;
	else
		bp_.paragraph_separation = BufferParams::ParagraphSkipSeparation;

	switch (textLayoutModule->skipCO->currentIndex()) {
	case 0:
		bp_.setDefSkip(VSpace(VSpace::SMALLSKIP));
		break;
	case 1:
		bp_.setDefSkip(VSpace(VSpace::MEDSKIP));
		break;
	case 2:
		bp_.setDefSkip(VSpace(VSpace::BIGSKIP));
		break;
	case 3:
	{
		VSpace vs = VSpace(
			widgetsToLength(textLayoutModule->skipLE,
				textLayoutModule->skipLengthCO)
			);
		bp_.setDefSkip(vs);
		break;
	}
	default:
		// DocumentDefskipCB assures that this never happens
		// so Assert then !!!  - jbl
		bp_.setDefSkip(VSpace(VSpace::MEDSKIP));
		break;
	}

	bp_.options =
		fromqstr(latexModule->optionsLE->text());

	bp_.use_default_options =
		latexModule->defaultOptionsCB->isChecked();

	if (latexModule->childDocGB->isChecked())
		bp_.master =
			fromqstr(latexModule->childDocLE->text());
	else
		bp_.master = string();

	bp_.float_placement = floatModule->get();

	// output
	bp_.defaultOutputFormat = fromqstr(outputModule->defaultFormatCO->itemData(
		outputModule->defaultFormatCO->currentIndex()).toString());

	bool const xetex = outputModule->xetexCB->isChecked();
	bp_.useXetex = xetex;

	// fonts
	if (xetex) {
		if (fontModule->fontsRomanCO->currentIndex() == 0)
			bp_.fontsRoman = "default";
		else
			bp_.fontsRoman =
				fromqstr(fontModule->fontsRomanCO->currentText());
	
		if (fontModule->fontsSansCO->currentIndex() == 0)
			bp_.fontsSans = "default";
		else
			bp_.fontsSans =
				fromqstr(fontModule->fontsSansCO->currentText());
	
		if (fontModule->fontsTypewriterCO->currentIndex() == 0)
			bp_.fontsTypewriter = "default";
		else
			bp_.fontsTypewriter =
				fromqstr(fontModule->fontsTypewriterCO->currentText());
	} else {
		bp_.fontsRoman =
			tex_fonts_roman[fontModule->fontsRomanCO->currentIndex()];
	
		bp_.fontsSans =
			tex_fonts_sans[fontModule->fontsSansCO->currentIndex()];
	
		bp_.fontsTypewriter =
			tex_fonts_monospaced[fontModule->fontsTypewriterCO->currentIndex()];
	}

	bp_.fontsCJK =
		fromqstr(fontModule->cjkFontLE->text());

	bp_.fontsSansScale = fontModule->scaleSansSB->value();

	bp_.fontsTypewriterScale = fontModule->scaleTypewriterSB->value();

	bp_.fontsSC = fontModule->fontScCB->isChecked();

	bp_.fontsOSF = fontModule->fontOsfCB->isChecked();

	if (xetex)
		bp_.fontsDefaultFamily = "default";
	else
		bp_.fontsDefaultFamily = GuiDocument::fontfamilies[
			fontModule->fontsDefaultCO->currentIndex()];

	if (fontModule->fontsizeCO->currentIndex() == 0)
		bp_.fontsize = "default";
	else
		bp_.fontsize =
			fromqstr(fontModule->fontsizeCO->currentText());

	// paper
	bp_.papersize = PAPER_SIZE(
		pageLayoutModule->papersizeCO->currentIndex());

	// custom, A3, B3 and B4 paper sizes need geometry
	int psize = pageLayoutModule->papersizeCO->currentIndex();
	bool geom_papersize = (psize == 1 || psize == 5 || psize == 8 || psize == 9);

	bp_.paperwidth = widgetsToLength(pageLayoutModule->paperwidthLE,
		pageLayoutModule->paperwidthUnitCO);

	bp_.paperheight = widgetsToLength(pageLayoutModule->paperheightLE,
		pageLayoutModule->paperheightUnitCO);

	if (pageLayoutModule->facingPagesCB->isChecked())
		bp_.sides = TwoSides;
	else
		bp_.sides = OneSide;

	if (pageLayoutModule->landscapeRB->isChecked())
		bp_.orientation = ORIENTATION_LANDSCAPE;
	else
		bp_.orientation = ORIENTATION_PORTRAIT;

	bp_.backgroundcolor = set_backgroundcolor;

	// margins
	bp_.use_geometry = !marginsModule->marginCB->isChecked()
		|| geom_papersize;

	Ui::MarginsUi const * m = marginsModule;

	bp_.leftmargin = widgetsToLength(m->innerLE, m->innerUnit);
	bp_.topmargin = widgetsToLength(m->topLE, m->topUnit);
	bp_.rightmargin = widgetsToLength(m->outerLE, m->outerUnit);
	bp_.bottommargin = widgetsToLength(m->bottomLE, m->bottomUnit);
	bp_.headheight = widgetsToLength(m->headheightLE, m->headheightUnit);
	bp_.headsep = widgetsToLength(m->headsepLE, m->headsepUnit);
	bp_.footskip = widgetsToLength(m->footskipLE, m->footskipUnit);
	bp_.columnsep = widgetsToLength(m->columnsepLE, m->columnsepUnit);

	branchesModule->apply(bp_);

	// PDF support
	PDFOptions & pdf = bp_.pdfoptions();
	pdf.use_hyperref = pdfSupportModule->use_hyperrefGB->isChecked();
	pdf.title = fromqstr(pdfSupportModule->titleLE->text());
	pdf.author = fromqstr(pdfSupportModule->authorLE->text());
	pdf.subject = fromqstr(pdfSupportModule->subjectLE->text());
	pdf.keywords = fromqstr(pdfSupportModule->keywordsLE->text());

	pdf.bookmarks = pdfSupportModule->bookmarksGB->isChecked();
	pdf.bookmarksnumbered = pdfSupportModule->bookmarksnumberedCB->isChecked();
	pdf.bookmarksopen = pdfSupportModule->bookmarksopenGB->isChecked();
	pdf.bookmarksopenlevel = pdfSupportModule->bookmarksopenlevelSB->value();

	pdf.breaklinks = pdfSupportModule->breaklinksCB->isChecked();
	pdf.pdfborder = pdfSupportModule->pdfborderCB->isChecked();
	pdf.pdfusetitle = pdfSupportModule->pdfusetitleCB->isChecked();
	pdf.colorlinks = pdfSupportModule->colorlinksCB->isChecked();
	pdf.backref =
		backref_opts[pdfSupportModule->backrefCO->currentIndex()];
	if (pdfSupportModule->fullscreenCB->isChecked())
		pdf.pagemode = pdf.pagemode_fullscreen;
	else
		pdf.pagemode.clear();
	pdf.quoted_options = pdf.quoted_options_check(
				fromqstr(pdfSupportModule->optionsLE->text()));
}


void GuiDocument::paramsToDialog()
{
	// set the default unit
	Length::UNIT const defaultUnit = Length::defaultUnit();

	// preamble
	preambleModule->update(bp_, id());

	// biblio
	biblioModule->citeDefaultRB->setChecked(
		bp_.citeEngine() == ENGINE_BASIC);

	biblioModule->citeNatbibRB->setChecked(
		bp_.citeEngine() == ENGINE_NATBIB_NUMERICAL ||
		bp_.citeEngine() == ENGINE_NATBIB_AUTHORYEAR);

	biblioModule->citeStyleCO->setCurrentIndex(
		bp_.citeEngine() == ENGINE_NATBIB_NUMERICAL);

	biblioModule->citeJurabibRB->setChecked(
		bp_.citeEngine() == ENGINE_JURABIB);

	biblioModule->bibtopicCB->setChecked(
		bp_.use_bibtopic);

	// language & quotes
	int const pos = langModule->languageCO->findData(toqstr(
		bp_.language->lang()));
	langModule->languageCO->setCurrentIndex(pos);

	langModule->quoteStyleCO->setCurrentIndex(
		bp_.quotes_language);

	bool default_enc = true;
	if (bp_.inputenc != "auto") {
		default_enc = false;
		if (bp_.inputenc == "default") {
			langModule->encodingCO->setCurrentIndex(0);
		} else {
			string enc_gui;
			Encodings::const_iterator it = encodings.begin();
			Encodings::const_iterator const end = encodings.end();
			for (; it != end; ++it) {
				if (it->latexName() == bp_.inputenc) {
					enc_gui = it->guiName();
					break;
				}
			}
			int const i = langModule->encodingCO->findText(
					qt_(enc_gui));
			if (i >= 0)
				langModule->encodingCO->setCurrentIndex(i);
			else
				// unknown encoding. Set to default.
				default_enc = true;
		}
	}
	langModule->defaultencodingRB->setChecked(default_enc);
	langModule->otherencodingRB->setChecked(!default_enc);

	// numbering
	int const min_toclevel = documentClass().min_toclevel();
	int const max_toclevel = documentClass().max_toclevel();
	if (documentClass().hasTocLevels()) {
		numberingModule->setEnabled(true);
		numberingModule->depthSL->setMinimum(min_toclevel - 1);
		numberingModule->depthSL->setMaximum(max_toclevel);
		numberingModule->depthSL->setValue(bp_.secnumdepth);
		numberingModule->tocSL->setMaximum(min_toclevel - 1);
		numberingModule->tocSL->setMaximum(max_toclevel);
		numberingModule->tocSL->setValue(bp_.tocdepth);
		updateNumbering();
	} else {
		numberingModule->setEnabled(false);
		numberingModule->tocTW->clear();
	}

	// bullets
	bulletsModule->setBullet(0, bp_.user_defined_bullet(0));
	bulletsModule->setBullet(1, bp_.user_defined_bullet(1));
	bulletsModule->setBullet(2, bp_.user_defined_bullet(2));
	bulletsModule->setBullet(3, bp_.user_defined_bullet(3));
	bulletsModule->init();

	// packages
	int nitem = findToken(tex_graphics, bp_.graphicsDriver);
	if (nitem >= 0)
		latexModule->psdriverCO->setCurrentIndex(nitem);
	updateModuleInfo();
	
	mathsModule->amsCB->setChecked(
		bp_.use_amsmath == BufferParams::package_on);
	mathsModule->amsautoCB->setChecked(
		bp_.use_amsmath == BufferParams::package_auto);

	mathsModule->esintCB->setChecked(
		bp_.use_esint == BufferParams::package_on);
	mathsModule->esintautoCB->setChecked(
		bp_.use_esint == BufferParams::package_auto);

	switch (bp_.spacing().getSpace()) {
		case Spacing::Other: nitem = 3; break;
		case Spacing::Double: nitem = 2; break;
		case Spacing::Onehalf: nitem = 1; break;
		case Spacing::Default: case Spacing::Single: nitem = 0; break;
	}

	// text layout
	string const & layoutID = bp_.baseClassID();
	setLayoutComboByIDString(layoutID);

	updatePagestyle(documentClass().opt_pagestyle(),
				 bp_.pagestyle);

	textLayoutModule->lspacingCO->setCurrentIndex(nitem);
	if (bp_.spacing().getSpace() == Spacing::Other) {
		textLayoutModule->lspacingLE->setText(
			toqstr(bp_.spacing().getValueAsString()));
	}
	setLSpacing(nitem);

	if (bp_.paragraph_separation == BufferParams::ParagraphIndentSeparation)
		textLayoutModule->indentRB->setChecked(true);
	else
		textLayoutModule->skipRB->setChecked(true);

	int skip = 0;
	switch (bp_.getDefSkip().kind()) {
	case VSpace::SMALLSKIP:
		skip = 0;
		break;
	case VSpace::MEDSKIP:
		skip = 1;
		break;
	case VSpace::BIGSKIP:
		skip = 2;
		break;
	case VSpace::LENGTH:
	{
		skip = 3;
		string const length = bp_.getDefSkip().asLyXCommand();
		lengthToWidgets(textLayoutModule->skipLE,
			textLayoutModule->skipLengthCO,
			length, defaultUnit);
		break;
	}
	default:
		skip = 0;
		break;
	}
	textLayoutModule->skipCO->setCurrentIndex(skip);
	setSkip(skip);

	textLayoutModule->twoColumnCB->setChecked(
		bp_.columns == 2);

	// break listings_params to multiple lines
	string lstparams =
		InsetListingsParams(bp_.listings_params).separatedParams();
	textLayoutModule->listingsED->setPlainText(toqstr(lstparams));

	if (!bp_.options.empty()) {
		latexModule->optionsLE->setText(
			toqstr(bp_.options));
	} else {
		latexModule->optionsLE->setText(QString());
	}

	// latex
	latexModule->defaultOptionsCB->setChecked(
			bp_.use_default_options);
	updateSelectedModules();
	selectionManager->updateProvidedModules(
			bp_.baseClass()->providedModules());
	selectionManager->updateExcludedModules(
			bp_.baseClass()->excludedModules());

	if (!documentClass().options().empty()) {
		latexModule->defaultOptionsLE->setText(
			toqstr(documentClass().options()));
	} else {
		latexModule->defaultOptionsLE->setText(
			toqstr(_("[No options predefined]")));
	}

	latexModule->defaultOptionsLE->setEnabled(
		bp_.use_default_options
		&& !documentClass().options().empty());

	latexModule->defaultOptionsCB->setEnabled(
		!documentClass().options().empty());

	if (!bp_.master.empty()) {
		latexModule->childDocGB->setChecked(true);
		latexModule->childDocLE->setText(
			toqstr(bp_.master));
	} else {
		latexModule->childDocLE->setText(QString());
		latexModule->childDocGB->setChecked(false);
	}

	floatModule->set(bp_.float_placement);

	// Output
	// update combobox with formats
	updateDefaultFormat();
	int index = outputModule->defaultFormatCO->findData(toqstr(
		bp_.defaultOutputFormat));
	// set to default if format is not found 
	if (index == -1)
		index = 0;
	outputModule->defaultFormatCO->setCurrentIndex(index);
	outputModule->xetexCB->setEnabled(bp_.baseClass()->outputType() == lyx::LATEX);
	outputModule->xetexCB->setChecked(
		bp_.baseClass()->outputType() == lyx::LATEX && bp_.useXetex);

	// Fonts
	updateFontsize(documentClass().opt_fontsize(),
			bp_.fontsize);

	if (bp_.useXetex) {
		for (int i = 0; i < fontModule->fontsRomanCO->count(); ++i) {
			if (fontModule->fontsRomanCO->itemText(i) == toqstr(bp_.fontsRoman)) {
				fontModule->fontsRomanCO->setCurrentIndex(i);
				return;
			}
		}
		
		for (int i = 0; i < fontModule->fontsSansCO->count(); ++i) {
			if (fontModule->fontsSansCO->itemText(i) == toqstr(bp_.fontsSans)) {
				fontModule->fontsSansCO->setCurrentIndex(i);
				return;
			}
		}
		for (int i = 0; i < fontModule->fontsTypewriterCO->count(); ++i) {
			if (fontModule->fontsTypewriterCO->itemText(i) == 
				toqstr(bp_.fontsTypewriter)) {
				fontModule->fontsTypewriterCO->setCurrentIndex(i);
				return;
			}
		}
	} else {
		int n = findToken(tex_fonts_roman, bp_.fontsRoman);
		if (n >= 0) {
			fontModule->fontsRomanCO->setCurrentIndex(n);
			romanChanged(n);
		}
	
		n = findToken(tex_fonts_sans, bp_.fontsSans);
		if (n >= 0) {
			fontModule->fontsSansCO->setCurrentIndex(n);
			sansChanged(n);
		}
	
		n = findToken(tex_fonts_monospaced, bp_.fontsTypewriter);
		if (n >= 0) {
			fontModule->fontsTypewriterCO->setCurrentIndex(n);
			ttChanged(n);
		}
	}

	if (!bp_.fontsCJK.empty())
		fontModule->cjkFontLE->setText(
			toqstr(bp_.fontsCJK));
	else
		fontModule->cjkFontLE->setText(QString());

	fontModule->fontScCB->setChecked(bp_.fontsSC);
	fontModule->fontOsfCB->setChecked(bp_.fontsOSF);
	fontModule->scaleSansSB->setValue(bp_.fontsSansScale);
	fontModule->scaleTypewriterSB->setValue(bp_.fontsTypewriterScale);

	int nn = findToken(GuiDocument::fontfamilies, bp_.fontsDefaultFamily);
	if (nn >= 0)
		fontModule->fontsDefaultCO->setCurrentIndex(nn);

	// paper
	bool const extern_geometry =
		documentClass().provides("geometry");
	int const psize = bp_.papersize;
	pageLayoutModule->papersizeCO->setCurrentIndex(psize);
	setCustomPapersize(!extern_geometry && psize);
	pageLayoutModule->papersizeCO->setEnabled(!extern_geometry);

	bool const landscape =
		bp_.orientation == ORIENTATION_LANDSCAPE;
	pageLayoutModule->landscapeRB->setChecked(landscape);
	pageLayoutModule->portraitRB->setChecked(!landscape);
	pageLayoutModule->landscapeRB->setEnabled(!extern_geometry);
	pageLayoutModule->portraitRB->setEnabled(!extern_geometry);

	pageLayoutModule->facingPagesCB->setChecked(
		bp_.sides == TwoSides);

	pageLayoutModule->backgroundTB->setStyleSheet(
		colorButtonStyleSheet(QColor(rgb2qcolor(bp_.backgroundcolor))));
	set_backgroundcolor = bp_.backgroundcolor;

	lengthToWidgets(pageLayoutModule->paperwidthLE,
		pageLayoutModule->paperwidthUnitCO, bp_.paperwidth, defaultUnit);
	lengthToWidgets(pageLayoutModule->paperheightLE,
		pageLayoutModule->paperheightUnitCO, bp_.paperheight, defaultUnit);

	// margins
	Ui::MarginsUi * m = marginsModule;

	setMargins(!bp_.use_geometry);

	lengthToWidgets(m->topLE, m->topUnit,
		bp_.topmargin, defaultUnit);

	lengthToWidgets(m->bottomLE, m->bottomUnit,
		bp_.bottommargin, defaultUnit);

	lengthToWidgets(m->innerLE, m->innerUnit,
		bp_.leftmargin, defaultUnit);

	lengthToWidgets(m->outerLE, m->outerUnit,
		bp_.rightmargin, defaultUnit);

	lengthToWidgets(m->headheightLE, m->headheightUnit,
		bp_.headheight, defaultUnit);

	lengthToWidgets(m->headsepLE, m->headsepUnit,
		bp_.headsep, defaultUnit);

	lengthToWidgets(m->footskipLE, m->footskipUnit,
		bp_.footskip, defaultUnit);

	lengthToWidgets(m->columnsepLE, m->columnsepUnit,
		bp_.columnsep, defaultUnit);

	branchesModule->update(bp_);

	// PDF support
	PDFOptions const & pdf = bp_.pdfoptions();
	pdfSupportModule->use_hyperrefGB->setChecked(pdf.use_hyperref);
	pdfSupportModule->titleLE->setText(toqstr(pdf.title));
	pdfSupportModule->authorLE->setText(toqstr(pdf.author));
	pdfSupportModule->subjectLE->setText(toqstr(pdf.subject));
	pdfSupportModule->keywordsLE->setText(toqstr(pdf.keywords));

	pdfSupportModule->bookmarksGB->setChecked(pdf.bookmarks);
	pdfSupportModule->bookmarksnumberedCB->setChecked(pdf.bookmarksnumbered);
	pdfSupportModule->bookmarksopenGB->setChecked(pdf.bookmarksopen);

	pdfSupportModule->bookmarksopenlevelSB->setValue(pdf.bookmarksopenlevel);

	pdfSupportModule->breaklinksCB->setChecked(pdf.breaklinks);
	pdfSupportModule->pdfborderCB->setChecked(pdf.pdfborder);
	pdfSupportModule->pdfusetitleCB->setChecked(pdf.pdfusetitle);
	pdfSupportModule->colorlinksCB->setChecked(pdf.colorlinks);

	nn = findToken(backref_opts, pdf.backref);
	if (nn >= 0)
		pdfSupportModule->backrefCO->setCurrentIndex(nn);

	pdfSupportModule->fullscreenCB->setChecked
		(pdf.pagemode == pdf.pagemode_fullscreen);

	pdfSupportModule->optionsLE->setText(
		toqstr(pdf.quoted_options));

	// Make sure that the bc is in the INITIAL state
	if (bc().policy().buttonStatus(ButtonPolicy::RESTORE))
		bc().restore();
}


void GuiDocument::saveDocDefault()
{
	// we have to apply the params first
	applyView();
	saveAsDefault();
}


void GuiDocument::updateAvailableModules() 
{
	modules_av_model_.clear();
	list<modInfoStruct> const & modInfoList = getModuleInfo();
	list<modInfoStruct>::const_iterator mit = modInfoList.begin();
	list<modInfoStruct>::const_iterator men = modInfoList.end();
	for (int i = 0; mit != men; ++mit, ++i)
		modules_av_model_.insertRow(i, mit->name, mit->id, 
				mit->description);
}


void GuiDocument::updateSelectedModules() 
{
	modules_sel_model_.clear();
	list<modInfoStruct> const selModList = getSelectedModules();
	list<modInfoStruct>::const_iterator mit = selModList.begin();
	list<modInfoStruct>::const_iterator men = selModList.end();
	for (int i = 0; mit != men; ++mit, ++i)
		modules_sel_model_.insertRow(i, mit->name, mit->id, 
				mit->description);
}


void GuiDocument::updateContents()
{
	// Nothing to do here as the document settings is not cursor dependant.
	return;
}


void GuiDocument::useClassDefaults()
{
	if (applyPB->isEnabled()) {
		int const ret = Alert::prompt(_("Unapplied changes"),
				_("Some changes in the dialog were not yet applied.\n"
				  "If you do not apply now, they will be lost after this action."),
				1, 1, _("&Apply"), _("&Dismiss"));
		if (ret == 0)
			applyView();
	}

	int idx = latexModule->classCO->currentIndex();
	string const classname = classes_model_.getIDString(idx);
	if (!bp_.setBaseClass(classname)) {
		Alert::error(_("Error"), _("Unable to set document class."));
		return;
	}
	bp_.useClassDefaults();
	paramsToDialog();
}


void GuiDocument::setLayoutComboByIDString(string const & idString)
{
	int idx = classes_model_.findIDString(idString);
	if (idx < 0)
		Alert::warning(_("Can't set layout!"), 
			bformat(_("Unable to set layout for ID: %1$s"), from_utf8(idString)));
	else 
		latexModule->classCO->setCurrentIndex(idx);
}


bool GuiDocument::isValid()
{
	return validateListingsParameters().isEmpty()
		&& (textLayoutModule->skipCO->currentIndex() != 3
			|| !textLayoutModule->skipLE->text().isEmpty());
}


char const * const GuiDocument::fontfamilies[5] = {
	"default", "rmdefault", "sfdefault", "ttdefault", ""
};


char const * GuiDocument::fontfamilies_gui[5] = {
	N_("Default"), N_("Roman"), N_("Sans Serif"), N_("Typewriter"), ""
};


bool GuiDocument::initialiseParams(string const &)
{
	BufferView const * view = bufferview();
	if (!view) {
		bp_ = BufferParams();
		paramsToDialog();
		return true;
	}
	bp_ = view->buffer().params();
	loadModuleInfo();
	updateAvailableModules();
	//FIXME It'd be nice to make sure here that the selected
	//modules are consistent: That required modules are actually
	//selected, and that we don't have conflicts. If so, we could
	//at least pop up a warning.
	paramsToDialog();
	return true;
}


void GuiDocument::clearParams()
{
	bp_ = BufferParams();
}


BufferId GuiDocument::id() const
{
	BufferView const * const view = bufferview();
	return view? &view->buffer() : 0;
}


list<GuiDocument::modInfoStruct> const & GuiDocument::getModuleInfo()
{
	return moduleNames_;
}


list<GuiDocument::modInfoStruct> const 
		GuiDocument::makeModuleInfo(LayoutModuleList const & mods)
{
	LayoutModuleList::const_iterator it =  mods.begin();
	LayoutModuleList::const_iterator end = mods.end();
	list<modInfoStruct> mInfo;
	for (; it != end; ++it) {
		modInfoStruct m;
		m.id = *it;
		LyXModule * mod = moduleList[*it];
		if (mod)
			// FIXME Unicode
			m.name = toqstr(translateIfPossible(from_utf8(mod->getName())));
		else 
			m.name = toqstr(*it) + toqstr(" (") + qt_("Not Found") + toqstr(")");
		mInfo.push_back(m);
	}
	return mInfo;
}


list<GuiDocument::modInfoStruct> const GuiDocument::getSelectedModules()
{
	return makeModuleInfo(params().getModules());
}


list<GuiDocument::modInfoStruct> const GuiDocument::getProvidedModules()
{
	return makeModuleInfo(params().baseClass()->providedModules());
}


DocumentClass const & GuiDocument::documentClass() const
{
	return bp_.documentClass();
}


static void dispatch_bufferparams(Dialog const & dialog,
	BufferParams const & bp, FuncCode lfun)
{
	ostringstream ss;
	ss << "\\begin_header\n";
	bp.writeFile(ss);
	ss << "\\end_header\n";
	dialog.dispatch(FuncRequest(lfun, ss.str()));
}


void GuiDocument::dispatchParams()
{
	// This must come first so that a language change is correctly noticed
	setLanguage();

	// Apply the BufferParams. Note that this will set the base class
	// and then update the buffer's layout.
	dispatch_bufferparams(*this, params(), LFUN_BUFFER_PARAMS_APPLY);

	if (!params().master.empty()) {
		FileName const master_file = support::makeAbsPath(params().master,
			   support::onlyPath(buffer().absFileName()));
		if (isLyXFilename(master_file.absFilename())) {
			Buffer * master = checkAndLoadLyXFile(master_file);
			if (master) {
				if (master->isChild(const_cast<Buffer *>(&buffer())))
					const_cast<Buffer &>(buffer()).setParent(master);
				else
					Alert::warning(_("Assigned master does not include this file"), 
						bformat(_("You must include this file in the document\n"
							  "'%1$s' in order to use the master document\n"
							  "feature."), from_utf8(params().master)));
			} else
				Alert::warning(_("Could not load master"), 
						bformat(_("The master document '%1$s'\n"
							   "could not be loaded."),
							   from_utf8(params().master)));
		}
	}

	// Generate the colours requested by each new branch.
	BranchList & branchlist = params().branchlist();
	if (!branchlist.empty()) {
		BranchList::const_iterator it = branchlist.begin();
		BranchList::const_iterator const end = branchlist.end();
		for (; it != end; ++it) {
			docstring const & current_branch = it->branch();
			Branch const * branch = branchlist.find(current_branch);
			string const x11hexname = X11hexname(branch->color());
			// display the new color
			docstring const str = current_branch + ' ' + from_ascii(x11hexname);
			dispatch(FuncRequest(LFUN_SET_COLOR, str));
		}

		// Open insets of selected branches, close deselected ones
		dispatch(FuncRequest(LFUN_ALL_INSETS_TOGGLE,
			"assign branch"));
	}
	// FIXME: If we used an LFUN, we would not need those two lines:
	BufferView * bv = const_cast<BufferView *>(bufferview());
	bv->processUpdateFlags(Update::Force | Update::FitCursor);
}


void GuiDocument::setLanguage() const
{
	Language const * const newL = bp_.language;
	if (buffer().params().language == newL)
		return;

	string const & lang_name = newL->lang();
	dispatch(FuncRequest(LFUN_BUFFER_LANGUAGE, lang_name));
}


void GuiDocument::saveAsDefault() const
{
	dispatch_bufferparams(*this, params(), LFUN_BUFFER_SAVE_AS_DEFAULT);
}


bool GuiDocument::isFontAvailable(string const & font) const
{
	if (font == "default" || font == "cmr"
	    || font == "cmss" || font == "cmtt")
		// these are standard
		return true;
	if (font == "lmodern" || font == "lmss" || font == "lmtt")
		return LaTeXFeatures::isAvailable("lmodern");
	if (font == "times" || font == "palatino"
		 || font == "helvet" || font == "courier")
		return LaTeXFeatures::isAvailable("psnfss");
	if (font == "cmbr" || font == "cmtl")
		return LaTeXFeatures::isAvailable("cmbright");
	if (font == "utopia")
		return LaTeXFeatures::isAvailable("utopia")
			|| LaTeXFeatures::isAvailable("fourier");
	if (font == "beraserif" || font == "berasans"
		|| font == "beramono")
		return LaTeXFeatures::isAvailable("bera");
	return LaTeXFeatures::isAvailable(font);
}


bool GuiDocument::providesOSF(string const & font) const
{
	if (outputModule->xetexCB->isChecked())
		// FIXME: we should check if the fonts really
		// have OSF support. But how?
		return true;
	if (font == "cmr")
		return isFontAvailable("eco");
	if (font == "palatino")
		return isFontAvailable("mathpazo");
	return false;
}


bool GuiDocument::providesSC(string const & font) const
{
	if (outputModule->xetexCB->isChecked())
		return false;
	if (font == "palatino")
		return isFontAvailable("mathpazo");
	if (font == "utopia")
		return isFontAvailable("fourier");
	return false;
}


bool GuiDocument::providesScale(string const & font) const
{
	if (outputModule->xetexCB->isChecked())
		return true;
	return font == "helvet" || font == "luximono"
		|| font == "berasans"  || font == "beramono";
}


void GuiDocument::loadModuleInfo()
{
	moduleNames_.clear();
	LyXModuleList::const_iterator it  = moduleList.begin();
	LyXModuleList::const_iterator end = moduleList.end();
	for (; it != end; ++it) {
		modInfoStruct m;
		m.id = it->getID();
		// FIXME Unicode
		m.name = toqstr(translateIfPossible(from_utf8(it->getName())));
		// this is supposed to give us the first sentence of the description
		// FIXME Unicode
		QString desc =
			toqstr(translateIfPossible(from_utf8(it->getDescription())));
		int const pos = desc.indexOf(".");
		if (pos > 0)
			desc.truncate(pos + 1);
		m.description = desc;
		moduleNames_.push_back(m);
	}
}


Dialog * createGuiDocument(GuiView & lv) { return new GuiDocument(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiDocument.cpp"
