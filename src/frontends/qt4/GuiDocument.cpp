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

#include "LayoutFile.h"
#include "BranchList.h"
#include "buffer_funcs.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Color.h"
#include "Encoding.h"
#include "FloatPlacement.h"
#include "FuncRequest.h"
#include "support/gettext.h"
#include "GuiBranches.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "LaTeXHighlighter.h"
#include "Layout.h"
#include "LengthCombo.h"
#include "LyXRC.h" // defaultUnit
#include "ModuleList.h"
#include "OutputParams.h"
#include "PanelStack.h"
#include "PDFOptions.h"
#include "qt_helpers.h"
#include "Spacing.h"
#include "Validator.h"

#include "insets/InsetListingsParams.h"

#include "support/debug.h"
#include "support/FileName.h"
#include "support/FileFilterList.h"
#include "support/filetools.h"
#include "support/lstrings.h"

#include "frontends/alert.h"

#include <QCloseEvent>
#include <QScrollBar>
#include <QTextCursor>

#include <sstream>

#ifdef IN
#undef IN
#endif


using namespace std;
using namespace lyx::support;


namespace {

char const * const tex_graphics[] =
{
	"default", "dvips", "dvitops", "emtex",
	"ln", "oztex", "textures", "none", ""
};


char const * const tex_graphics_gui[] =
{
	N_("Default"), "Dvips", "DVItoPS", "EmTeX",
	"LN", "OzTeX", "Textures", N_("None"), ""
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


vector<pair<string, lyx::docstring> > pagestyles;


} // anonymous namespace

namespace lyx {

namespace {
// used when sorting the textclass list.
class less_textclass_avail_desc
	: public binary_function<string, string, int>
{
public:
	int operator()(string const & lhs, string const & rhs) const
	{
		// Ordering criteria:
		//   1. Availability of text class
		//   2. Description (lexicographic)
		LayoutFile const & tc1 = LayoutFileList::get()[lhs];
		LayoutFile const & tc2 = LayoutFileList::get()[rhs];
		return (tc1.isTeXClassAvailable() && !tc2.isTeXClassAvailable()) ||
			(tc1.isTeXClassAvailable() == tc2.isTeXClassAvailable() &&
			 _(tc1.description()) < _(tc2.description()));
	}
};

}

namespace frontend {


/// 
QModelIndex getSelectedIndex(QListView * lv)
{
	QModelIndex retval = QModelIndex();
	QModelIndexList selIdx = 
			lv->selectionModel()->selectedIndexes();
	if (!selIdx.empty())
		retval = selIdx.first();
	return retval;
}


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
		return _(mod->getDescription());
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
} //anonymous namespace


ModuleSelMan::ModuleSelMan(
	QListView * availableLV, 
	QListView * selectedLV,
	QPushButton * addPB, 
	QPushButton * delPB, 
	QPushButton * upPB, 
	QPushButton * downPB,
	GuiIdListModel * availableModel,
	GuiIdListModel * selectedModel) :
GuiSelectionManager(availableLV, selectedLV, addPB, delPB,
                    upPB, downPB, availableModel, selectedModel) 
{}
	

void ModuleSelMan::updateAddPB() 
{
	int const arows = availableModel->rowCount();
	QModelIndexList const availSels = 
			availableLV->selectionModel()->selectedIndexes();
	if (arows == 0 || availSels.isEmpty()  || isSelected(availSels.first())) {
		addPB->setEnabled(false);
		return;
	}
	
	QModelIndex const & idx = availableLV->selectionModel()->currentIndex();
	string const modName = getAvailableModel()->getIDString(idx.row());
	vector<string> reqs = getRequiredList(modName);
	vector<string> excl = getExcludedList(modName);
	
	if (reqs.empty() && excl.empty()) {
		addPB->setEnabled(true);
		return;
	}

	int const srows = selectedModel->rowCount();
	vector<string> selModList;
	for (int i = 0; i < srows; ++i)
		selModList.push_back(getSelectedModel()->getIDString(i));

	vector<string>::const_iterator selModStart = selModList.begin();
	vector<string>::const_iterator selModEnd   = selModList.end();
	
	//Check whether some required module is available
	if (!reqs.empty()) {
		bool foundOne = false;
		vector<string>::const_iterator it  = reqs.begin();
		vector<string>::const_iterator end = reqs.end();
		for (; it != end; ++it) {
			if (find(selModStart, selModEnd, *it) != selModEnd) {
				foundOne = true;
				break;
			}
		}
		if (!foundOne) {
			addPB->setEnabled(false);
			return;
		}
	}
	
	//Check whether any excluded module is being used
	if (!excl.empty()) {
		vector<string>::const_iterator it  = excl.begin();
		vector<string>::const_iterator end = excl.end();
		for (; it != end; ++it) {
			if (find(selModStart, selModEnd, *it) != selModEnd) {
				addPB->setEnabled(false);
				return;
			}
		}
	}

	addPB->setEnabled(true);
}


void ModuleSelMan::updateDownPB()
{
	int const srows = selectedModel->rowCount();
	if (srows == 0) {
		downPB->setEnabled(false);
		return;
	}
	QModelIndexList const selSels = 
			selectedLV->selectionModel()->selectedIndexes();
	//disable if empty or last item is selected
	if (selSels.empty() || selSels.first().row() == srows - 1) {
		downPB->setEnabled(false);
		return;
	}
	//determine whether immediately succeding element requires this one
	QModelIndex const & curIdx = selectedLV->selectionModel()->currentIndex();
	int curRow = curIdx.row();
	if (curRow < 0 || curRow >= srows - 1) { //this shouldn't happen...
		downPB->setEnabled(false);
		return;
	}
	string const curModName = getSelectedModel()->getIDString(curRow);
	string const nextModName = getSelectedModel()->getIDString(curRow + 1);

	vector<string> reqs = getRequiredList(nextModName);

	//if it doesn't require anything....
	if (reqs.empty()) {
		downPB->setEnabled(true);
		return;
	}

	//FIXME This should perhaps be more flexible and check whether, even 
	//if this one is required, there is also an earlier one that is required.
	//enable it if this module isn't required
	downPB->setEnabled(
			find(reqs.begin(), reqs.end(), curModName) == reqs.end());
}

void ModuleSelMan::updateUpPB() 
{
	int const srows = selectedModel->rowCount();
	if (srows == 0) {
		upPB->setEnabled(false);
		return;
	}
	QModelIndexList const selSels = 
			selectedLV->selectionModel()->selectedIndexes();
	//disable if empty or first item is selected
	if (selSels.empty() || selSels.first().row() == 0) {
		upPB->setEnabled(false);
		return;
	}

	//determine whether immediately preceding element is required by this one
	QModelIndex const & curIdx = selectedLV->selectionModel()->currentIndex();
	int curRow = curIdx.row();
	if (curRow <= -1 || curRow > srows - 1) { //sanity check
		downPB->setEnabled(false);
		return;
	}
	string const curModName = getSelectedModel()->getIDString(curRow);
	vector<string> reqs = getRequiredList(curModName);
	
	//if this one doesn't require anything....
	if (reqs.empty()) {
		upPB->setEnabled(true);
		return;
	}

	string preModName = getSelectedModel()->getIDString(curRow - 1);

	//NOTE This is less flexible than it might be. You could check whether, even 
	//if this one is required, there is also an earlier one that is required.
	//enable it if the preceding module isn't required
	upPB->setEnabled(find(reqs.begin(), reqs.end(), preModName) == reqs.end());
}

void ModuleSelMan::updateDelPB() 
{
	int const srows = selectedModel->rowCount();
	if (srows == 0) {
		deletePB->setEnabled(false);
		return;
	}
	QModelIndexList const selSels = 
			selectedLV->selectionModel()->selectedIndexes();
	if (selSels.empty() || selSels.first().row() < 0) {
		deletePB->setEnabled(false);
		return;
	}
	
	//determine whether some LATER module requires this one
	//NOTE Things are arranged so that this is the only way there
	//can be a problem. At least, we hope so.
	QModelIndex const & curIdx = 
		selectedLV->selectionModel()->currentIndex();
	int const curRow = curIdx.row();
	if (curRow < 0 || curRow >= srows) { //this shouldn't happen
		deletePB->setEnabled(false);
		return;
	}
		
	QString const curModName = curIdx.data().toString();
	
	//We're looking here for a reason NOT to enable the button. If we
	//find one, we disable it and return. If we don't, we'll end up at
	//the end of the function, and then we enable it.
	for (int i = curRow + 1; i < srows; ++i) {
		string const thisMod = getSelectedModel()->getIDString(i);
		vector<string> reqs = getRequiredList(thisMod);
		//does this one require us?
		if (find(reqs.begin(), reqs.end(), fromqstr(curModName)) == reqs.end())
			//no...
			continue;

		//OK, so this module requires us
		//is there an EARLIER module that also satisfies the require?
		//NOTE We demand that it be earlier to keep the list of modules
		//consistent with the rule that a module must be proceeded by a
		//required module. There would be more flexible ways to proceed,
		//but that would be a lot more complicated, and the logic here is
		//already complicated. (That's why I've left the debugging code.)
		//lyxerr << "Testing " << thisMod << std::endl;
		bool foundOne = false;
		for (int j = 0; j < curRow; ++j) {
			string const mod = getSelectedModel()->getIDString(j);
			//lyxerr << "In loop: Testing " << mod << std::endl;
			//do we satisfy the require? 
			if (find(reqs.begin(), reqs.end(), mod) != reqs.end()) {
				//lyxerr << mod << " does the trick." << std::endl;
				foundOne = true;
				break;
			}
		}
		//did we find a module to satisfy the require?
		if (!foundOne) {
			//lyxerr << "No matching module found." << std::endl;
			deletePB->setEnabled(false);
			return;
		}
	}
	//lyxerr << "All's well that ends well." << std::endl;	
	deletePB->setEnabled(true);
}


/////////////////////////////////////////////////////////////////////
//
// PreambleModule
//
/////////////////////////////////////////////////////////////////////

PreambleModule::PreambleModule(): current_id_(0)
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
		preamble_coords_[current_id_] = make_pair(0,0);
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
	: GuiDialog(lv, "document", qt_("Document Settings")), current_id_(0)
{
	setupUi(this);

	QList<LanguagePair> langs = languageData(false);	
	for (int i = 0; i != langs.size(); ++i)
		lang_.append(langs[i].second);

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
		this, SLOT(set_listings_msg()));
	connect(textLayoutModule->listingsED, SIGNAL(textChanged()),
		this, SLOT(set_listings_msg()));
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

	fontModule = new UiWidget<Ui::FontUi>;
	// fonts
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
	connect(fontModule->scaleSansSB, SIGNAL(valueChanged(int)),
		this, SLOT(change_adaptor()));
	connect(fontModule->scaleTypewriterSB, SIGNAL(valueChanged(int)),
		this, SLOT(change_adaptor()));
	connect(fontModule->fontScCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(fontModule->fontOsfCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));

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
	connect(marginsModule->topLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->topUnit, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->bottomLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->bottomUnit, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->innerLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->innerUnit, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->outerLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->outerUnit, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->headheightLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->headheightUnit, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->headsepLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->headsepUnit, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->footskipLE, SIGNAL(textChanged(const QString&)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->footskipUnit, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(marginsModule->columnsepLE, SIGNAL(textChanged(const QString&)),
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

	QList<LanguagePair>::const_iterator lit  = langs.begin();
	QList<LanguagePair>::const_iterator lend = langs.end();
	for (; lit != lend; ++lit)
		langModule->languageCO->addItem(lit->first);

	// Always put the default encoding in the first position.
	// It is special because the displayed text is translated.
	langModule->encodingCO->addItem(qt_("LaTeX default"));
	Encodings::const_iterator it = encodings.begin();
	Encodings::const_iterator const end = encodings.end();
	for (; it != end; ++it)
		langModule->encodingCO->addItem(toqstr(it->latexName()));

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
	connect(latexModule->optionsLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(latexModule->psdriverCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(latexModule->classCO, SIGNAL(activated(int)),
		this, SLOT(classChanged()));
	connect(latexModule->classCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(latexModule->layoutPB, SIGNAL(clicked()),
		this, SLOT(browseLayout()));
	
	selectionManager = 
		new ModuleSelMan(latexModule->availableLV, latexModule->selectedLV, 
			latexModule->addPB, latexModule->deletePB, 
	 		latexModule->upPB, latexModule->downPB, 
			availableModel(), selectedModel());
	connect(selectionManager, SIGNAL(updateHook()),
		this, SLOT(updateModuleInfo()));
	connect(selectionManager, SIGNAL(updateHook()),
		this, SLOT(change_adaptor()));
	
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

	// PDF support
	pdfSupportModule = new UiWidget<Ui::PDFSupportUi>;

	connect(pdfSupportModule->use_hyperrefGB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->titleLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->authorLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->subjectLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->keywordsLE, SIGNAL(textChanged(const QString &)),
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
	connect(pdfSupportModule->backrefCB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->pdfusetitleCB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->pagebackrefCB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->fullscreenCB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(pdfSupportModule->optionsLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));

	// float
	floatModule = new FloatPlacement;
	connect(floatModule, SIGNAL(changed()),
		this, SLOT(change_adaptor()));

	docPS->addPanel(latexModule, qt_("Document Class"));
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


docstring GuiDocument::validate_listings_params()
{
	// use a cache here to avoid repeated validation
	// of the same parameters
	static string param_cache = string();
	static docstring msg_cache = docstring();
	
	if (textLayoutModule->bypassCB->isChecked())
		return docstring();

	string params = fromqstr(textLayoutModule->listingsED->toPlainText());
	if (params != param_cache) {
		param_cache = params;
		msg_cache = InsetListingsParams(params).validate();
	}
	return msg_cache;
}


void GuiDocument::set_listings_msg()
{
	static bool isOK = true;
	docstring msg = validate_listings_params();
	if (msg.empty()) {
		if (isOK)
			return;
		isOK = true;
		// listingsTB->setTextColor("black");
		textLayoutModule->listingsTB->setPlainText(
			qt_("Input listings parameters on the right. Enter ? for a list of parameters."));
	} else {
		isOK = false;
		// listingsTB->setTextColor("red");
		textLayoutModule->listingsTB->setPlainText(toqstr(msg));
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


void GuiDocument::romanChanged(int item)
{
	string const font = tex_fonts_roman[item];
	fontModule->fontScCB->setEnabled(providesSC(font));
	fontModule->fontOsfCB->setEnabled(providesOSF(font));
}


void GuiDocument::sansChanged(int item)
{
	string const font = tex_fonts_sans[item];
	bool scaleable = providesScale(font);
	fontModule->scaleSansSB->setEnabled(scaleable);
	fontModule->scaleSansLA->setEnabled(scaleable);
}


void GuiDocument::ttChanged(int item)
{
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

	for (int n = 0; !token(items,'|',n).empty(); ++n) {
		string style = token(items, '|', n);
		docstring style_gui = _(style);
		pagestyles.push_back(pair<string, docstring>(style, style_gui));
		pageLayoutModule->pagestyleCO->addItem(toqstr(style_gui));
	}

	if (sel == "default") {
		pageLayoutModule->pagestyleCO->setCurrentIndex(0);
		return;
	}

	int nn = 0;

	for (size_t i = 0; i < pagestyles.size(); ++i)
		if (pagestyles[i].first == sel)
			nn = pageLayoutModule->pagestyleCO->findText(
					toqstr(pagestyles[i].second));

	if (nn > 0)
		pageLayoutModule->pagestyleCO->setCurrentIndex(nn);
}


void GuiDocument::browseLayout()
{
	QString const label1 = qt_("Layouts|#o#O");
	QString const dir1 = toqstr(lyxrc.document_path);
	FileFilterList const filter(_("LyX Layout (*.layout)"));
	QString file = browseRelFile(QString(), bufferFilepath(),
		qt_("Local layout file"), filter, false,
		label1, dir1);

	if (!suffixIs(fromqstr(file), ".layout"))
		return;

	FileName layoutFile = makeAbsPath(fromqstr(file),
		fromqstr(bufferFilepath()));
	
	// load the layout file
	LayoutFileList & bcl = LayoutFileList::get();
	string classname = layoutFile.onlyFileName();
	LayoutFileIndex name = bcl.addLayoutFile(
		classname.substr(0, classname.size() - 7),
		layoutFile.onlyPath().absFilename(),
		LayoutFileList::Local);

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


void GuiDocument::classChanged()
{
	int idx = latexModule->classCO->currentIndex();
	if (idx < 0) 
		return;
	string const classname = classes_model_.getIDString(idx);
	// check if this is a local layout file
	if (prefixIs(classname, LayoutFileList::localPrefix)) {
		int const ret = Alert::prompt(_("Local layout file"),
				_("The layout file you have selected is a local layout\n"
				  "file, not one in the system or user directory. Your\n"
				  "document may not work with this layout if you do not\n"
				  "keep the layout file in the same directory."),
				  1, 1, _("&Set Layout"), _("&Cancel"));
		if (ret == 1) {
			// try to reset the layout combo
			setLayoutComboByIDString(bp_.baseClassID());
			return;
		}
	} else if (prefixIs(classname, LayoutFileList::embeddedPrefix)) {
		int const ret = Alert::prompt(_("Embedded layout"),
				_("The layout file you have selected is an embedded layout that\n"
				  "is embedded to a buffer. You cannot make use of it unless\n"
				  "it is already embedded to this buffer.\n"),
				  1, 1, _("&Set Layout"), _("&Cancel"));
		if (ret == 1) {
			setLayoutComboByIDString(bp_.baseClassID());
			return;
		}
	}
	// FIXME Note that by doing things this way, we load the TextClass
	// as soon as it is selected. So, if you use the scroll wheel when
	// sitting on the combo box, we'll load a lot of TextClass objects
	// very quickly. This could be changed.
	if (!bp_.setBaseClass(classname)) {
		Alert::error(_("Error"), _("Unable to set document class."));
		return;
	}
	if (lyxrc.auto_reset_options) {
		if (applyPB->isEnabled()) {
			int const ret = Alert::prompt(_("Unapplied changes"),
					_("Some changes in the dialog were not yet applied.\n"
					"If you do not apply now, they will be lost after this action."),
					1, 1, _("&Apply"), _("&Dismiss"));
			if (ret == 0)
				applyView();
		}
		bp_.useClassDefaults();
		forceUpdate();
	}
}


namespace {
	// This is an insanely complicated attempt to make this sort of thing
	// work with RTL languages.
	docstring formatStrVec(vector<string> const & v, docstring const & s) 
	{
		//this mess formats the list as "v[0], v[1], ..., [s] v[n]"
		int const vSize = v.size();
		if (v.size() == 0)
			return docstring();
		else if (v.size() == 1) 
			return from_ascii(v[0]);
		else if (v.size() == 2) {
			docstring retval = _("%1$s and %2$s");
			retval = subst(retval, _("and"), s);
			return bformat(retval, from_ascii(v[0]), from_ascii(v[1]));
		}
		//The idea here is to format all but the last two items...
		docstring t2 = _("%1$s, %2$s");
		docstring retval = from_ascii(v[0]);
		for (int i = 1; i < vSize - 2; ++i)
			retval = bformat(t2, retval, from_ascii(v[i])); 
		//...and then to  plug them, and the last two, into this schema
		docstring t = _("%1$s, %2$s, and %3$s");
		t = subst(t, _("and"), s);
		return bformat(t, retval, from_ascii(v[vSize - 2]), from_ascii(v[vSize - 1]));
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


void GuiDocument::updateModuleInfo()
{
	selectionManager->update();
	
	//Module description
	bool const focusOnSelected = selectionManager->selectedFocused();
	QListView const * const lv = 
			focusOnSelected ? latexModule->selectedLV : latexModule->availableLV;
	if (lv->selectionModel()->selectedIndexes().isEmpty()) {
		latexModule->infoML->document()->clear();
		return;
	}
	QModelIndex const & idx = lv->selectionModel()->currentIndex();
	GuiIdListModel const & idModel = 
			focusOnSelected  ? modules_sel_model_ : modules_av_model_;
	string const modName = idModel.getIDString(idx.row());
	docstring desc = getModuleDescription(modName);

	vector<string> pkgList = getPackageList(modName);
	docstring pkgdesc = formatStrVec(pkgList, _("and"));
	if (!pkgdesc.empty()) {
		if (!desc.empty())
			desc += "\n";
		desc += bformat(_("Package(s) required: %1$s."), pkgdesc);
	}

	pkgList = getRequiredList(modName);
	if (!pkgList.empty()) {
		vector<string> const reqDescs = idsToNames(pkgList);
		pkgdesc = formatStrVec(reqDescs, _("or"));
		if (!desc.empty())
			desc += "\n";
		desc += bformat(_("Module required: %1$s."), pkgdesc);
	}

	pkgList = getExcludedList(modName);
	if (!pkgList.empty()) {
		vector<string> const reqDescs = idsToNames(pkgList);
		pkgdesc = formatStrVec(reqDescs, _( "and"));
		if (!desc.empty())
			desc += "\n";
		desc += bformat(_("Modules excluded: %1$s."), pkgdesc);
	}

	if (!isModuleAvailable(modName)) {
		if (!desc.empty())
			desc += "\n";
		desc += _("WARNING: Some packages are unavailable!");
	}

	latexModule->infoML->document()->setPlainText(toqstr(desc));
}


void GuiDocument::updateNumbering()
{
	DocumentClass const & tclass = bp_.documentClass();

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


void GuiDocument::apply(BufferParams & params)
{
	// preamble
	preambleModule->apply(params);

	// biblio
	params.setCiteEngine(biblio::ENGINE_BASIC);

	if (biblioModule->citeNatbibRB->isChecked()) {
		bool const use_numerical_citations =
			biblioModule->citeStyleCO->currentIndex();
		if (use_numerical_citations)
			params.setCiteEngine(biblio::ENGINE_NATBIB_NUMERICAL);
		else
			params.setCiteEngine(biblio::ENGINE_NATBIB_AUTHORYEAR);

	} else if (biblioModule->citeJurabibRB->isChecked())
		params.setCiteEngine(biblio::ENGINE_JURABIB);

	params.use_bibtopic =
		biblioModule->bibtopicCB->isChecked();

	// language & quotes
	if (langModule->defaultencodingRB->isChecked()) {
		params.inputenc = "auto";
	} else {
		int i = langModule->encodingCO->currentIndex();
		if (i == 0)
			params.inputenc = "default";
		else
			params.inputenc =
				fromqstr(langModule->encodingCO->currentText());
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
	params.quotes_language = lga;

	int const pos = langModule->languageCO->currentIndex();
	params.language = lyx::languages.getLanguage(fromqstr(lang_[pos]));

	// numbering
	if (params.documentClass().hasTocLevels()) {
		params.tocdepth = numberingModule->tocSL->value();
		params.secnumdepth = numberingModule->depthSL->value();
	}

	// bullets
	params.user_defined_bullet(0) = bulletsModule->getBullet(0);
	params.user_defined_bullet(1) = bulletsModule->getBullet(1);
	params.user_defined_bullet(2) = bulletsModule->getBullet(2);
	params.user_defined_bullet(3) = bulletsModule->getBullet(3);

	// packages
	params.graphicsDriver =
		tex_graphics[latexModule->psdriverCO->currentIndex()];
	
	// text layout
	int idx = latexModule->classCO->currentIndex();
	if (idx >= 0) {
		string const classname = classes_model_.getIDString(idx);
		params.setBaseClass(classname);
	}

	// Modules
	params.clearLayoutModules();
	int const srows = modules_sel_model_.rowCount();
	vector<string> selModList;
	for (int i = 0; i < srows; ++i)
		params.addLayoutModule(modules_sel_model_.getIDString(i));

	if (mathsModule->amsautoCB->isChecked()) {
		params.use_amsmath = BufferParams::package_auto;
	} else {
		if (mathsModule->amsCB->isChecked())
			params.use_amsmath = BufferParams::package_on;
		else
			params.use_amsmath = BufferParams::package_off;
	}

	if (mathsModule->esintautoCB->isChecked())
		params.use_esint = BufferParams::package_auto;
	else {
		if (mathsModule->esintCB->isChecked())
			params.use_esint = BufferParams::package_on;
		else
			params.use_esint = BufferParams::package_off;
	}

	if (pageLayoutModule->pagestyleCO->currentIndex() == 0)
		params.pagestyle = "default";
	else {
		docstring style_gui =
			qstring_to_ucs4(pageLayoutModule->pagestyleCO->currentText());
		for (size_t i = 0; i < pagestyles.size(); ++i)
			if (pagestyles[i].second == style_gui)
				params.pagestyle = pagestyles[i].first;
	}

	switch (textLayoutModule->lspacingCO->currentIndex()) {
	case 0:
		params.spacing().set(Spacing::Single);
		break;
	case 1:
		params.spacing().set(Spacing::Onehalf);
		break;
	case 2:
		params.spacing().set(Spacing::Double);
		break;
	case 3:
		params.spacing().set(Spacing::Other,
			fromqstr(textLayoutModule->lspacingLE->text()));
		break;
	}

	if (textLayoutModule->twoColumnCB->isChecked())
		params.columns = 2;
	else
		params.columns = 1;

	// text should have passed validation
	params.listings_params =
		InsetListingsParams(fromqstr(textLayoutModule->listingsED->toPlainText())).params();

	if (textLayoutModule->indentRB->isChecked())
		params.paragraph_separation = BufferParams::ParagraphIndentSeparation;
	else
		params.paragraph_separation = BufferParams::ParagraphSkipSeparation;

	switch (textLayoutModule->skipCO->currentIndex()) {
	case 0:
		params.setDefSkip(VSpace(VSpace::SMALLSKIP));
		break;
	case 1:
		params.setDefSkip(VSpace(VSpace::MEDSKIP));
		break;
	case 2:
		params.setDefSkip(VSpace(VSpace::BIGSKIP));
		break;
	case 3:
	{
		VSpace vs = VSpace(
			widgetsToLength(textLayoutModule->skipLE,
				textLayoutModule->skipLengthCO)
			);
		params.setDefSkip(vs);
		break;
	}
	default:
		// DocumentDefskipCB assures that this never happens
		// so Assert then !!!  - jbl
		params.setDefSkip(VSpace(VSpace::MEDSKIP));
		break;
	}

	params.options =
		fromqstr(latexModule->optionsLE->text());

	params.float_placement = floatModule->get();

	// fonts
	params.fontsRoman =
		tex_fonts_roman[fontModule->fontsRomanCO->currentIndex()];

	params.fontsSans =
		tex_fonts_sans[fontModule->fontsSansCO->currentIndex()];

	params.fontsTypewriter =
		tex_fonts_monospaced[fontModule->fontsTypewriterCO->currentIndex()];

	params.fontsSansScale = fontModule->scaleSansSB->value();

	params.fontsTypewriterScale = fontModule->scaleTypewriterSB->value();

	params.fontsSC = fontModule->fontScCB->isChecked();

	params.fontsOSF = fontModule->fontOsfCB->isChecked();

	params.fontsDefaultFamily = GuiDocument::fontfamilies[
		fontModule->fontsDefaultCO->currentIndex()];

	if (fontModule->fontsizeCO->currentIndex() == 0)
		params.fontsize = "default";
	else
		params.fontsize =
			fromqstr(fontModule->fontsizeCO->currentText());

	// paper
	params.papersize = PAPER_SIZE(
		pageLayoutModule->papersizeCO->currentIndex());

	// custom, A3, B3 and B4 paper sizes need geometry
	int psize = pageLayoutModule->papersizeCO->currentIndex();
	bool geom_papersize = (psize == 1 || psize == 5 || psize == 8 || psize == 9);

	params.paperwidth = widgetsToLength(pageLayoutModule->paperwidthLE,
		pageLayoutModule->paperwidthUnitCO);

	params.paperheight = widgetsToLength(pageLayoutModule->paperheightLE,
		pageLayoutModule->paperheightUnitCO);

	if (pageLayoutModule->facingPagesCB->isChecked())
		params.sides = TwoSides;
	else
		params.sides = OneSide;

	if (pageLayoutModule->landscapeRB->isChecked())
		params.orientation = ORIENTATION_LANDSCAPE;
	else
		params.orientation = ORIENTATION_PORTRAIT;

	// margins
	params.use_geometry = !marginsModule->marginCB->isChecked()
		|| geom_papersize;

	Ui::MarginsUi const * m = marginsModule;

	params.leftmargin = widgetsToLength(m->innerLE, m->innerUnit);
	params.topmargin = widgetsToLength(m->topLE, m->topUnit);
	params.rightmargin = widgetsToLength(m->outerLE, m->outerUnit);
	params.bottommargin = widgetsToLength(m->bottomLE, m->bottomUnit);
	params.headheight = widgetsToLength(m->headheightLE, m->headheightUnit);
	params.headsep = widgetsToLength(m->headsepLE, m->headsepUnit);
	params.footskip = widgetsToLength(m->footskipLE, m->footskipUnit);
	params.columnsep = widgetsToLength(m->columnsepLE, m->columnsepUnit);

	branchesModule->apply(params);

	// PDF support
	PDFOptions & pdf = params.pdfoptions();
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
	pdf.backref = pdfSupportModule->backrefCB->isChecked();
	pdf.pagebackref	= pdfSupportModule->pagebackrefCB->isChecked();
	if (pdfSupportModule->fullscreenCB->isChecked())
		pdf.pagemode = pdf.pagemode_fullscreen;
	else
		pdf.pagemode.clear();
	pdf.quoted_options = pdf.quoted_options_check(
				fromqstr(pdfSupportModule->optionsLE->text()));
}


static int findPos(QStringList const & vec, QString const & val)
{
	for (int i = 0; i != vec.size(); ++i)
		if (vec[i] == val)
			return i;
	return 0;
}


void GuiDocument::updateParams()
{
	updateParams(bp_);
}


void GuiDocument::updateParams(BufferParams const & params)
{
	// set the default unit
	Length::UNIT defaultUnit = Length::CM;
	switch (lyxrc.default_papersize) {
		case PAPER_DEFAULT: break;

		case PAPER_USLETTER:
		case PAPER_USLEGAL:
		case PAPER_USEXECUTIVE:
			defaultUnit = Length::IN;
			break;

		case PAPER_A3:
		case PAPER_A4:
		case PAPER_A5:
		case PAPER_B3:
		case PAPER_B4:
		case PAPER_B5:
			defaultUnit = Length::CM;
			break;
		case PAPER_CUSTOM:
			break;
	}

	// preamble
	preambleModule->update(params, id());

	// biblio
	biblioModule->citeDefaultRB->setChecked(
		params.citeEngine() == biblio::ENGINE_BASIC);

	biblioModule->citeNatbibRB->setChecked(
		params.citeEngine() == biblio::ENGINE_NATBIB_NUMERICAL ||
		params.citeEngine() == biblio::ENGINE_NATBIB_AUTHORYEAR);

	biblioModule->citeStyleCO->setCurrentIndex(
		params.citeEngine() == biblio::ENGINE_NATBIB_NUMERICAL);

	biblioModule->citeJurabibRB->setChecked(
		params.citeEngine() == biblio::ENGINE_JURABIB);

	biblioModule->bibtopicCB->setChecked(
		params.use_bibtopic);

	// language & quotes
	int const pos = findPos(lang_, toqstr(params.language->lang()));
	langModule->languageCO->setCurrentIndex(pos);

	langModule->quoteStyleCO->setCurrentIndex(
		params.quotes_language);

	bool default_enc = true;
	if (params.inputenc != "auto") {
		default_enc = false;
		if (params.inputenc == "default") {
			langModule->encodingCO->setCurrentIndex(0);
		} else {
			int const i = langModule->encodingCO->findText(
					toqstr(params.inputenc));
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
		numberingModule->depthSL->setValue(params.secnumdepth);
		numberingModule->tocSL->setMaximum(min_toclevel - 1);
		numberingModule->tocSL->setMaximum(max_toclevel);
		numberingModule->tocSL->setValue(params.tocdepth);
		updateNumbering();
	} else {
		numberingModule->setEnabled(false);
		numberingModule->tocTW->clear();
	}

	// bullets
	bulletsModule->setBullet(0, params.user_defined_bullet(0));
	bulletsModule->setBullet(1, params.user_defined_bullet(1));
	bulletsModule->setBullet(2, params.user_defined_bullet(2));
	bulletsModule->setBullet(3, params.user_defined_bullet(3));
	bulletsModule->init();

	// packages
	int nitem = findToken(tex_graphics, params.graphicsDriver);
	if (nitem >= 0)
		latexModule->psdriverCO->setCurrentIndex(nitem);
	updateModuleInfo();
	
	mathsModule->amsCB->setChecked(
		params.use_amsmath == BufferParams::package_on);
	mathsModule->amsautoCB->setChecked(
		params.use_amsmath == BufferParams::package_auto);

	mathsModule->esintCB->setChecked(
		params.use_esint == BufferParams::package_on);
	mathsModule->esintautoCB->setChecked(
		params.use_esint == BufferParams::package_auto);

	switch (params.spacing().getSpace()) {
		case Spacing::Other: nitem = 3; break;
		case Spacing::Double: nitem = 2; break;
		case Spacing::Onehalf: nitem = 1; break;
		case Spacing::Default: case Spacing::Single: nitem = 0; break;
	}

	// text layout
	string const & layoutID = params.baseClassID();
	setLayoutComboByIDString(layoutID);

	updatePagestyle(documentClass().opt_pagestyle(),
				 params.pagestyle);

	textLayoutModule->lspacingCO->setCurrentIndex(nitem);
	if (params.spacing().getSpace() == Spacing::Other) {
		textLayoutModule->lspacingLE->setText(
			toqstr(params.spacing().getValueAsString()));
	}
	setLSpacing(nitem);

	if (params.paragraph_separation == BufferParams::ParagraphIndentSeparation)
		textLayoutModule->indentRB->setChecked(true);
	else
		textLayoutModule->skipRB->setChecked(true);

	int skip = 0;
	switch (params.getDefSkip().kind()) {
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
		string const length = params.getDefSkip().asLyXCommand();
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
		params.columns == 2);

	// break listings_params to multiple lines
	string lstparams =
		InsetListingsParams(params.listings_params).separatedParams();
	textLayoutModule->listingsED->setPlainText(toqstr(lstparams));

	if (!params.options.empty()) {
		latexModule->optionsLE->setText(
			toqstr(params.options));
	} else {
		latexModule->optionsLE->setText(QString());
	}

	floatModule->set(params.float_placement);

	// Fonts
	updateFontsize(documentClass().opt_fontsize(),
			params.fontsize);

	int n = findToken(tex_fonts_roman, params.fontsRoman);
	if (n >= 0) {
		fontModule->fontsRomanCO->setCurrentIndex(n);
		romanChanged(n);
	}

	n = findToken(tex_fonts_sans, params.fontsSans);
	if (n >= 0)	{
		fontModule->fontsSansCO->setCurrentIndex(n);
		sansChanged(n);
	}

	n = findToken(tex_fonts_monospaced, params.fontsTypewriter);
	if (n >= 0) {
		fontModule->fontsTypewriterCO->setCurrentIndex(n);
		ttChanged(n);
	}

	fontModule->fontScCB->setChecked(params.fontsSC);
	fontModule->fontOsfCB->setChecked(params.fontsOSF);
	fontModule->scaleSansSB->setValue(params.fontsSansScale);
	fontModule->scaleTypewriterSB->setValue(params.fontsTypewriterScale);
	n = findToken(GuiDocument::fontfamilies, params.fontsDefaultFamily);
	if (n >= 0)
		fontModule->fontsDefaultCO->setCurrentIndex(n);

	// paper
	int const psize = params.papersize;
	pageLayoutModule->papersizeCO->setCurrentIndex(psize);
	setCustomPapersize(psize);

	bool const landscape =
		params.orientation == ORIENTATION_LANDSCAPE;
	pageLayoutModule->landscapeRB->setChecked(landscape);
	pageLayoutModule->portraitRB->setChecked(!landscape);

	pageLayoutModule->facingPagesCB->setChecked(
		params.sides == TwoSides);


	lengthToWidgets(pageLayoutModule->paperwidthLE,
		pageLayoutModule->paperwidthUnitCO, params.paperwidth, defaultUnit);

	lengthToWidgets(pageLayoutModule->paperheightLE,
		pageLayoutModule->paperheightUnitCO, params.paperheight, defaultUnit);

	// margins
	Ui::MarginsUi * m = marginsModule;

	setMargins(!params.use_geometry);

	lengthToWidgets(m->topLE, m->topUnit,
		params.topmargin, defaultUnit);

	lengthToWidgets(m->bottomLE, m->bottomUnit,
		params.bottommargin, defaultUnit);

	lengthToWidgets(m->innerLE, m->innerUnit,
		params.leftmargin, defaultUnit);

	lengthToWidgets(m->outerLE, m->outerUnit,
		params.rightmargin, defaultUnit);

	lengthToWidgets(m->headheightLE, m->headheightUnit,
		params.headheight, defaultUnit);

	lengthToWidgets(m->headsepLE, m->headsepUnit,
		params.headsep, defaultUnit);

	lengthToWidgets(m->footskipLE, m->footskipUnit,
		params.footskip, defaultUnit);

	lengthToWidgets(m->columnsepLE, m->columnsepUnit,
		params.columnsep, defaultUnit);

	branchesModule->update(params);

	// PDF support
	PDFOptions const & pdf = params.pdfoptions();
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
	pdfSupportModule->backrefCB->setChecked(pdf.backref);
	pdfSupportModule->pagebackrefCB->setChecked(pdf.pagebackref);
	pdfSupportModule->fullscreenCB->setChecked
		(pdf.pagemode == pdf.pagemode_fullscreen);

	pdfSupportModule->optionsLE->setText(
		toqstr(pdf.quoted_options));
}


void GuiDocument::applyView()
{
	apply(params());
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
	vector<modInfoStruct> const & modInfoList = getModuleInfo();
	int const mSize = modInfoList.size();
	for (int i = 0; i != mSize; ++i) {
		modInfoStruct const & modInfo = modInfoList[i];
		modules_av_model_.insertRow(i, modInfo.name, modInfo.id, 
				modInfo.description);
	}
}


void GuiDocument::updateSelectedModules() 
{
	// and selected ones, too
	modules_sel_model_.clear();
	vector<modInfoStruct> const selModList = getSelectedModules();
	int const sSize = selModList.size();
	for (int i = 0; i != sSize; ++i) {
		modInfoStruct const & modInfo = selModList[i];
		modules_sel_model_.insertRow(i, modInfo.name, modInfo.id,
				modInfo.description);
	}
}


void GuiDocument::updateContents()
{
	if (id() == current_id_)
		return;

	updateAvailableModules();
	updateSelectedModules();
	
	//FIXME It'd be nice to make sure here that the selected
	//modules are consistent: That required modules are actually
	//selected, and that we don't have conflicts. If so, we could
	//at least pop up a warning.
	updateParams(bp_);
	current_id_ = id();
}


void GuiDocument::forceUpdate()
{
	// reset to force dialog update
	current_id_ = 0;
	updateContents();
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
	forceUpdate();
}


void GuiDocument::setLayoutComboByIDString(std::string const & idString)
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
	return validate_listings_params().empty()
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
	bp_ = buffer().params();
	loadModuleInfo();
	return true;
}


void GuiDocument::clearParams()
{
	bp_ = BufferParams();
}


BufferId GuiDocument::id() const
{
	return &buffer();
}


vector<GuiDocument::modInfoStruct> const & GuiDocument::getModuleInfo()
{
	return moduleNames_;
}


vector<GuiDocument::modInfoStruct> const GuiDocument::getSelectedModules()
{
	vector<string> const & mods = params().getModules();
	vector<string>::const_iterator it =  mods.begin();
	vector<string>::const_iterator end = mods.end();
	vector<modInfoStruct> mInfo;
	for (; it != end; ++it) {
		modInfoStruct m;
		m.id = *it;
		LyXModule * mod = moduleList[*it];
		if (mod)
			m.name = qt_(mod->getName());
		else 
			m.name = toqstr(*it) + toqstr(" (") + qt_("Not Found") + toqstr(")");
		mInfo.push_back(m);
	}
	return mInfo;
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

	// Generate the colours requested by each new branch.
	BranchList & branchlist = params().branchlist();
	if (!branchlist.empty()) {
		BranchList::const_iterator it = branchlist.begin();
		BranchList::const_iterator const end = branchlist.end();
		for (; it != end; ++it) {
			docstring const & current_branch = it->getBranch();
			Branch const * branch = branchlist.find(current_branch);
			string const x11hexname = X11hexname(branch->getColor());
			// display the new color
			docstring const str = current_branch + ' ' + from_ascii(x11hexname);
			dispatch(FuncRequest(LFUN_SET_COLOR, str));
		}

		// Open insets of selected branches, close deselected ones
		dispatch(FuncRequest(LFUN_ALL_INSETS_TOGGLE,
			"assign branch"));
	}
	// FIXME: If we used an LFUN, we would not need those two lines:
	bufferview()->processUpdateFlags(Update::Force | Update::FitCursor);
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
	if (font == "cmr")
		return isFontAvailable("eco");
	if (font == "palatino")
		return isFontAvailable("mathpazo");
	return false;
}


bool GuiDocument::providesSC(string const & font) const
{
	if (font == "palatino")
		return isFontAvailable("mathpazo");
	if (font == "utopia")
		return isFontAvailable("fourier");
	return false;
}


bool GuiDocument::providesScale(string const & font) const
{
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
		m.name = qt_(it->getName());
		// this is supposed to give us the first sentence of the description
		QString desc = qt_(it->getDescription());
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

#include "GuiDocument_moc.cpp"
