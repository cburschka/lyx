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

#include "CategorizedCombo.h"
#include "GuiApplication.h"
#include "GuiBranches.h"
#include "GuiIndices.h"
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
#include "Cursor.h"
#include "Encoding.h"
#include "FloatPlacement.h"
#include "Format.h"
#include "FuncRequest.h"
#include "HSpace.h"
#include "IndicesList.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "LaTeXFonts.h"
#include "Layout.h"
#include "LayoutEnums.h"
#include "LayoutModuleList.h"
#include "LyXRC.h"
#include "ModuleList.h"
#include "OutputParams.h"
#include "PDFOptions.h"
#include "qt_helpers.h"
#include "Spacing.h"
#include "TextClass.h"
#include "Undo.h"
#include "VSpace.h"

#include "insets/InsetListingsParams.h"

#include "support/debug.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"

#include "frontends/alert.h"

#include <QAbstractItemModel>
#include <QHeaderView>
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
static inline QString colorButtonStyleSheet(QColor const & bgColor)
{
	if (bgColor.isValid()) {
		QString rc = QLatin1String("background-color:");
		rc += bgColor.name();
		return rc;
	}
	return QString();
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


char const * backref_opts[] =
{
	"false", "section", "slide", "page", ""
};


char const * backref_opts_gui[] =
{
	N_("Off"), N_("Section"), N_("Slide"), N_("Page"), ""
};


vector<string> engine_types_;
vector<pair<string, QString> > pagestyles;

QMap<QString, QString> rmfonts_;
QMap<QString, QString> sffonts_;
QMap<QString, QString> ttfonts_;
QMap<QString, QString> mathfonts_;


} // anonymous namespace

namespace lyx {

RGBColor set_backgroundcolor;
bool is_backgroundcolor;
RGBColor set_fontcolor;
bool is_fontcolor;
RGBColor set_notefontcolor;
RGBColor set_boxbgcolor;
bool forced_fontspec_activation;

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
		int const order = compare_no_case(
			translateIfPossible(from_utf8(tc1.description())),
			translateIfPossible(from_utf8(tc2.description())));
		return (tc1.isTeXClassAvailable() && !tc2.isTeXClassAvailable()) ||
			(tc1.isTeXClassAvailable() == tc2.isTeXClassAvailable() && order < 0);
	}
};

}

namespace frontend {
namespace {

vector<string> getRequiredList(string const & modName)
{
	LyXModule const * const mod = theModuleList[modName];
	if (!mod)
		return vector<string>(); //empty such thing
	return mod->getRequiredModules();
}


vector<string> getExcludedList(string const & modName)
{
	LyXModule const * const mod = theModuleList[modName];
	if (!mod)
		return vector<string>(); //empty such thing
	return mod->getExcludedModules();
}


docstring getModuleCategory(string const & modName)
{
	LyXModule const * const mod = theModuleList[modName];
	if (!mod)
		return docstring();
	return from_utf8(mod->category());
}


docstring getModuleDescription(string const & modName)
{
	LyXModule const * const mod = theModuleList[modName];
	if (!mod)
		return _("Module not found!");
	// FIXME Unicode
	return translateIfPossible(from_utf8(mod->getDescription()));
}


vector<string> getPackageList(string const & modName)
{
	LyXModule const * const mod = theModuleList[modName];
	if (!mod)
		return vector<string>(); //empty such thing
	return mod->getPackageList();
}


bool isModuleAvailable(string const & modName)
{
	LyXModule const * const mod = theModuleList[modName];
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
		QTreeView * availableLV,
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
	list<string> provided_modules_;
	/// similarly...
	list<string> excluded_modules_;
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
		container_->params().layoutModuleCanBeAdded(modname);
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
		// lyxerr << "Testing " << thisMod << endl;
		bool foundone = false;
		for (int j = 0; j < curRow; ++j) {
			string const mod = getSelectedModel()->getIDString(j);
			// lyxerr << "In loop: Testing " << mod << endl;
			// do we satisfy the require?
			if (find(reqs.begin(), reqs.end(), mod) != reqs.end()) {
				// lyxerr << mod << " does the trick." << endl;
				foundone = true;
				break;
			}
		}
		// did we find a module to satisfy the require?
		if (!foundone) {
			// lyxerr << "No matching module found." << endl;
			deletePB->setEnabled(false);
			return;
		}
	}
	// lyxerr << "All's well that ends well." << endl;
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
	QFont font(guiApp->typewriterFontName());
	font.setFixedPitch(true);
	font.setStyleHint(QFont::TypeWriter);
	preambleTE->setFont(font);
	preambleTE->setWordWrapMode(QTextOption::NoWrap);
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
// LocalLayout
//
/////////////////////////////////////////////////////////////////////


LocalLayout::LocalLayout() : current_id_(0), validated_(false)
{
	connect(locallayoutTE, SIGNAL(textChanged()), this, SLOT(textChanged()));
	connect(validatePB, SIGNAL(clicked()), this, SLOT(validatePressed()));
	connect(convertPB, SIGNAL(clicked()), this, SLOT(convertPressed()));
}


void LocalLayout::update(BufferParams const & params, BufferId id)
{
	QString layout = toqstr(params.getLocalLayout(false));
	// Nothing to do if the params and preamble are unchanged.
	if (id == current_id_
		&& layout == locallayoutTE->document()->toPlainText())
		return;

	// Save the params address for further use.
	current_id_ = id;
	locallayoutTE->document()->setPlainText(layout);
	validate();
}


void LocalLayout::apply(BufferParams & params)
{
	string const layout = fromqstr(locallayoutTE->document()->toPlainText());
	params.setLocalLayout(layout, false);
}


void LocalLayout::textChanged()
{
	static const QString message =
		qt_("Press button to check validity...");
	string const layout =
		fromqstr(locallayoutTE->document()->toPlainText().trimmed());

	if (layout.empty()) {
		validated_ = true;
		validatePB->setEnabled(false);
		validLB->setText("");
		convertPB->hide();
		convertLB->hide();
		changed();
	} else if (!validatePB->isEnabled()) {
		// if that's already enabled, we shouldn't need to do anything.
		validated_ = false;
		validLB->setText(message);
		validatePB->setEnabled(true);
		convertPB->setEnabled(false);
		changed();
	}
}


void LocalLayout::convert() {
	string const layout =
		fromqstr(locallayoutTE->document()->toPlainText().trimmed());
	string const newlayout = TextClass::convert(layout);
	LYXERR0(newlayout);
	if (newlayout.empty()) {
		Alert::error(_("Conversion Failed!"),
		      _("Failed to convert local layout to current format."));
	} else {
		locallayoutTE->setPlainText(toqstr(newlayout));
	}
	validate();
}


void LocalLayout::convertPressed() {
	convert();
	changed();
}


void LocalLayout::validate() {
	static const QString valid = qt_("Layout is valid!");
	static const QString vtext =
		toqstr("<p style=\"font-weight: bold; \">")
		  + valid + toqstr("</p>");
	static const QString invalid = qt_("Layout is invalid!");
	static const QString ivtext =
		toqstr("<p style=\"color: #c00000; font-weight: bold; \">")
		  + invalid + toqstr("</p>");

	string const layout =
		fromqstr(locallayoutTE->document()->toPlainText().trimmed());
	if (!layout.empty()) {
		TextClass::ReturnValues const ret = TextClass::validate(layout);
		validated_ = (ret == TextClass::OK) || (ret == TextClass::OK_OLDFORMAT);
		validatePB->setEnabled(false);
		validLB->setText(validated_ ? vtext : ivtext);
		if (ret == TextClass::OK_OLDFORMAT) {
			convertPB->show();
			convertPB->setEnabled(true);
			convertLB->setText(qt_("Convert to current format"));
			convertLB->show();
		} else {
			convertPB->hide();
			convertLB->hide();
		}
	}
}


void LocalLayout::validatePressed() {
	validate();
	changed();
}


/////////////////////////////////////////////////////////////////////
//
// DocumentDialog
//
/////////////////////////////////////////////////////////////////////


GuiDocument::GuiDocument(GuiView & lv)
	: GuiDialog(lv, "document", qt_("Document Settings")),
	  nonModuleChanged_(false)
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


	// text layout
	textLayoutModule = new UiWidget<Ui::TextLayoutUi>;
	connect(textLayoutModule->lspacingCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->lspacingCO, SIGNAL(activated(int)),
		this, SLOT(setLSpacing(int)));
	connect(textLayoutModule->lspacingLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));

	connect(textLayoutModule->indentRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->indentRB, SIGNAL(toggled(bool)),
		textLayoutModule->indentCO, SLOT(setEnabled(bool)));
	connect(textLayoutModule->indentCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->indentCO, SIGNAL(activated(int)),
		this, SLOT(setIndent(int)));
	connect(textLayoutModule->indentLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->indentLengthCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));

	connect(textLayoutModule->skipRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->skipRB, SIGNAL(toggled(bool)),
		textLayoutModule->skipCO, SLOT(setEnabled(bool)));
	connect(textLayoutModule->skipCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->skipCO, SIGNAL(activated(int)),
		this, SLOT(setSkip(int)));
	connect(textLayoutModule->skipLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->skipLengthCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));

	connect(textLayoutModule->indentRB, SIGNAL(toggled(bool)),
		this, SLOT(enableIndent(bool)));
	connect(textLayoutModule->skipRB, SIGNAL(toggled(bool)),
		this, SLOT(enableSkip(bool)));

	connect(textLayoutModule->twoColumnCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(textLayoutModule->twoColumnCB, SIGNAL(clicked()),
		this, SLOT(setColSep()));
	connect(textLayoutModule->justCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));

	textLayoutModule->lspacingLE->setValidator(new QDoubleValidator(
		textLayoutModule->lspacingLE));
	textLayoutModule->indentLE->setValidator(unsignedLengthValidator(
		textLayoutModule->indentLE));
	textLayoutModule->skipLE->setValidator(unsignedGlueLengthValidator(
		textLayoutModule->skipLE));

	textLayoutModule->indentCO->addItem(qt_("Default"));
	textLayoutModule->indentCO->addItem(qt_("Custom"));
	textLayoutModule->skipCO->addItem(qt_("SmallSkip"));
	textLayoutModule->skipCO->addItem(qt_("MedSkip"));
	textLayoutModule->skipCO->addItem(qt_("BigSkip"));
	textLayoutModule->skipCO->addItem(qt_("Custom"));
	textLayoutModule->lspacingCO->insertItem(
		Spacing::Single, qt_("Single"));
	textLayoutModule->lspacingCO->insertItem(
		Spacing::Onehalf, qt_("OneHalf"));
	textLayoutModule->lspacingCO->insertItem(
		Spacing::Double, qt_("Double"));
	textLayoutModule->lspacingCO->insertItem(
		Spacing::Other, qt_("Custom"));
	// initialize the length validator
	bc().addCheckedLineEdit(textLayoutModule->indentLE);
	bc().addCheckedLineEdit(textLayoutModule->skipLE);


	// master/child handling
	masterChildModule = new UiWidget<Ui::MasterChildUi>;

	connect(masterChildModule->childrenTW, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
		this, SLOT(includeonlyClicked(QTreeWidgetItem *, int)));
	connect(masterChildModule->includeonlyRB, SIGNAL(toggled(bool)),
		masterChildModule->childrenTW, SLOT(setEnabled(bool)));
	connect(masterChildModule->includeonlyRB, SIGNAL(toggled(bool)),
		masterChildModule->maintainAuxCB, SLOT(setEnabled(bool)));
	connect(masterChildModule->includeallRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(masterChildModule->includeonlyRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(masterChildModule->maintainAuxCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	masterChildModule->childrenTW->setColumnCount(2);
	masterChildModule->childrenTW->headerItem()->setText(0, qt_("Child Document"));
	masterChildModule->childrenTW->headerItem()->setText(1, qt_("Include to Output"));
	masterChildModule->childrenTW->resizeColumnToContents(1);
	masterChildModule->childrenTW->resizeColumnToContents(2);


	// output
	outputModule = new UiWidget<Ui::OutputUi>;

	connect(outputModule->defaultFormatCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(outputModule->mathimgSB, SIGNAL(valueChanged(double)),
		this, SLOT(change_adaptor()));
	connect(outputModule->strictCB, SIGNAL(stateChanged(int)),
		this, SLOT(change_adaptor()));
	connect(outputModule->cssCB, SIGNAL(stateChanged(int)),
		this, SLOT(change_adaptor()));
	connect(outputModule->mathoutCB, SIGNAL(currentIndexChanged(int)),
		this, SLOT(change_adaptor()));

	connect(outputModule->outputsyncCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(outputModule->synccustomCB, SIGNAL(editTextChanged(QString)),
		this, SLOT(change_adaptor()));
	outputModule->synccustomCB->addItem("");
	outputModule->synccustomCB->addItem("\\synctex=1");
	outputModule->synccustomCB->addItem("\\synctex=-1");
	outputModule->synccustomCB->addItem("\\usepackage[active]{srcltx}");

	outputModule->synccustomCB->setValidator(new NoNewLineValidator(
		outputModule->synccustomCB));

	// fonts
	fontModule = new FontModule;
	connect(fontModule->osFontsCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(fontModule->osFontsCB, SIGNAL(toggled(bool)),
		this, SLOT(osFontsChanged(bool)));
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
	connect(fontModule->fontsMathCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(fontModule->fontsMathCO, SIGNAL(activated(int)),
		this, SLOT(mathFontChanged(int)));
	connect(fontModule->fontsDefaultCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(fontModule->fontencCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(fontModule->fontencCO, SIGNAL(activated(int)),
		this, SLOT(fontencChanged(int)));
	connect(fontModule->fontencLE, SIGNAL(textChanged(const QString &)),
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
	connect(fontModule->fontScCB, SIGNAL(toggled(bool)),
		this, SLOT(fontScToggled(bool)));
	connect(fontModule->fontOsfCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(fontModule->fontOsfCB, SIGNAL(toggled(bool)),
		this, SLOT(fontOsfToggled(bool)));

	fontModule->fontencLE->setValidator(new NoNewLineValidator(
		fontModule->fontencLE));
	fontModule->cjkFontLE->setValidator(new NoNewLineValidator(
		fontModule->cjkFontLE));

	updateFontlist();

	fontModule->fontsizeCO->addItem(qt_("Default"));
	fontModule->fontsizeCO->addItem(qt_("10"));
	fontModule->fontsizeCO->addItem(qt_("11"));
	fontModule->fontsizeCO->addItem(qt_("12"));

	fontModule->fontencCO->addItem(qt_("Default"), QString("global"));
	fontModule->fontencCO->addItem(qt_("Custom"), QString("custom"));
	fontModule->fontencCO->addItem(qt_("None (no fontenc)"), QString("default"));

	for (int n = 0; GuiDocument::fontfamilies_gui[n][0]; ++n)
		fontModule->fontsDefaultCO->addItem(
			qt_(GuiDocument::fontfamilies_gui[n]));

	if (!LaTeXFeatures::isAvailable("fontspec"))
		fontModule->osFontsCB->setToolTip(
			qt_("Use OpenType and TrueType fonts directly (requires XeTeX or LuaTeX)\n"
			    "You need to install the package \"fontspec\" to use this feature"));


	// page layout
	pageLayoutModule = new UiWidget<Ui::PageLayoutUi>;
	connect(pageLayoutModule->papersizeCO, SIGNAL(activated(int)),
		this, SLOT(papersizeChanged(int)));
	connect(pageLayoutModule->papersizeCO, SIGNAL(activated(int)),
		this, SLOT(papersizeChanged(int)));
	connect(pageLayoutModule->portraitRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
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

	QComboBox * cb = pageLayoutModule->papersizeCO;
	cb->addItem(qt_("Default"));
	cb->addItem(qt_("Custom"));
	cb->addItem(qt_("US letter"));
	cb->addItem(qt_("US legal"));
	cb->addItem(qt_("US executive"));
	cb->addItem(qt_("A0"));
	cb->addItem(qt_("A1"));
	cb->addItem(qt_("A2"));
	cb->addItem(qt_("A3"));
	cb->addItem(qt_("A4"));
	cb->addItem(qt_("A5"));
	cb->addItem(qt_("A6"));
	cb->addItem(qt_("B0"));
	cb->addItem(qt_("B1"));
	cb->addItem(qt_("B2"));
	cb->addItem(qt_("B3"));
	cb->addItem(qt_("B4"));
	cb->addItem(qt_("B5"));
	cb->addItem(qt_("B6"));
	cb->addItem(qt_("C0"));
	cb->addItem(qt_("C1"));
	cb->addItem(qt_("C2"));
	cb->addItem(qt_("C3"));
	cb->addItem(qt_("C4"));
	cb->addItem(qt_("C5"));
	cb->addItem(qt_("C6"));
	cb->addItem(qt_("JIS B0"));
	cb->addItem(qt_("JIS B1"));
	cb->addItem(qt_("JIS B2"));
	cb->addItem(qt_("JIS B3"));
	cb->addItem(qt_("JIS B4"));
	cb->addItem(qt_("JIS B5"));
	cb->addItem(qt_("JIS B6"));
	// remove the %-items from the unit choice
	pageLayoutModule->paperwidthUnitCO->noPercents();
	pageLayoutModule->paperheightUnitCO->noPercents();
	pageLayoutModule->paperheightLE->setValidator(unsignedLengthValidator(
		pageLayoutModule->paperheightLE));
	pageLayoutModule->paperwidthLE->setValidator(unsignedLengthValidator(
		pageLayoutModule->paperwidthLE));


	// margins
	marginsModule = new UiWidget<Ui::MarginsUi>;
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


	// language & quote
	langModule = new UiWidget<Ui::LanguageUi>;
	connect(langModule->languageCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(langModule->languageCO, SIGNAL(activated(int)),
		this, SLOT(languageChanged(int)));
	connect(langModule->defaultencodingRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(langModule->otherencodingRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(langModule->encodingCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(langModule->quoteStyleCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(langModule->languagePackageCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(langModule->languagePackageLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(langModule->languagePackageCO, SIGNAL(currentIndexChanged(int)),
		this, SLOT(languagePackageChanged(int)));

	langModule->languagePackageLE->setValidator(new NoNewLineValidator(
		langModule->languagePackageLE));

	QAbstractItemModel * language_model = guiApp->languageModel();
	// FIXME: it would be nice if sorting was enabled/disabled via a checkbox.
	language_model->sort(0);
	langModule->languageCO->setModel(language_model);
	langModule->languageCO->setModelColumn(0);

	// Always put the default encoding in the first position.
	langModule->encodingCO->addItem(qt_("Language Default (no inputenc)"));
	QStringList encodinglist;
	Encodings::const_iterator it = encodings.begin();
	Encodings::const_iterator const end = encodings.end();
	for (; it != end; ++it)
		if (!it->unsafe())
			encodinglist.append(qt_(it->guiName()));
	encodinglist.sort();
	langModule->encodingCO->addItems(encodinglist);

	langModule->quoteStyleCO->addItem(
		qt_("``text''"),InsetQuotes::EnglishQuotes);
	langModule->quoteStyleCO->addItem(
		qt_("''text''"), InsetQuotes::SwedishQuotes);
	langModule->quoteStyleCO->addItem
		(qt_(",,text``"), InsetQuotes::GermanQuotes);
	langModule->quoteStyleCO->addItem(
		qt_(",,text''"), InsetQuotes::PolishQuotes);
	langModule->quoteStyleCO->addItem(
		qt_("<<text>>"), InsetQuotes::FrenchQuotes);
	langModule->quoteStyleCO->addItem(
		qt_(">>text<<"), InsetQuotes::DanishQuotes);

	langModule->languagePackageCO->addItem(
		qt_("Default"), toqstr("default"));
	langModule->languagePackageCO->addItem(
		qt_("Automatic"), toqstr("auto"));
	langModule->languagePackageCO->addItem(
		qt_("Always Babel"), toqstr("babel"));
	langModule->languagePackageCO->addItem(
		qt_("Custom"), toqstr("custom"));
	langModule->languagePackageCO->addItem(
		qt_("None[[language package]]"), toqstr("none"));


	// color
	colorModule = new UiWidget<Ui::ColorUi>;
	connect(colorModule->fontColorPB, SIGNAL(clicked()),
		this, SLOT(changeFontColor()));
	connect(colorModule->delFontColorTB, SIGNAL(clicked()),
		this, SLOT(deleteFontColor()));
	connect(colorModule->noteFontColorPB, SIGNAL(clicked()),
		this, SLOT(changeNoteFontColor()));
	connect(colorModule->delNoteFontColorTB, SIGNAL(clicked()),
		this, SLOT(deleteNoteFontColor()));
	connect(colorModule->backgroundPB, SIGNAL(clicked()),
		this, SLOT(changeBackgroundColor()));
	connect(colorModule->delBackgroundTB, SIGNAL(clicked()),
		this, SLOT(deleteBackgroundColor()));
	connect(colorModule->boxBackgroundPB, SIGNAL(clicked()),
		this, SLOT(changeBoxBackgroundColor()));
	connect(colorModule->delBoxBackgroundTB, SIGNAL(clicked()),
		this, SLOT(deleteBoxBackgroundColor()));


	// numbering
	numberingModule = new UiWidget<Ui::NumberingUi>;
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
	setSectionResizeMode(numberingModule->tocTW->header(), QHeaderView::ResizeToContents);

	// biblio
	biblioModule = new UiWidget<Ui::BiblioUi>;
	connect(biblioModule->citeDefaultRB, SIGNAL(toggled(bool)),
		this, SLOT(setNumerical(bool)));
	connect(biblioModule->citeJurabibRB, SIGNAL(toggled(bool)),
		this, SLOT(setAuthorYear(bool)));
	connect(biblioModule->citeNatbibRB, SIGNAL(toggled(bool)),
		biblioModule->citationStyleL, SLOT(setEnabled(bool)));
	connect(biblioModule->citeNatbibRB, SIGNAL(toggled(bool)),
		biblioModule->citeStyleCO, SLOT(setEnabled(bool)));
	connect(biblioModule->citeDefaultRB, SIGNAL(clicked()),
		this, SLOT(biblioChanged()));
	connect(biblioModule->citeNatbibRB, SIGNAL(clicked()),
		this, SLOT(biblioChanged()));
	connect(biblioModule->citeStyleCO, SIGNAL(activated(int)),
		this, SLOT(biblioChanged()));
	connect(biblioModule->citeJurabibRB, SIGNAL(clicked()),
		this, SLOT(biblioChanged()));
	connect(biblioModule->bibtopicCB, SIGNAL(clicked()),
		this, SLOT(biblioChanged()));
	connect(biblioModule->bibtexCO, SIGNAL(activated(int)),
		this, SLOT(bibtexChanged(int)));
	connect(biblioModule->bibtexOptionsLE, SIGNAL(textChanged(QString)),
		this, SLOT(biblioChanged()));
	connect(biblioModule->bibtexStyleLE, SIGNAL(textChanged(QString)),
		this, SLOT(biblioChanged()));

	biblioModule->bibtexOptionsLE->setValidator(new NoNewLineValidator(
		biblioModule->bibtexOptionsLE));
	biblioModule->bibtexStyleLE->setValidator(new NoNewLineValidator(
		biblioModule->bibtexStyleLE));

	biblioModule->citeStyleCO->addItem(qt_("Author-year"));
	biblioModule->citeStyleCO->addItem(qt_("Numerical"));
	biblioModule->citeStyleCO->setCurrentIndex(0);

	// NOTE: we do not provide "custom" here for security reasons!
	biblioModule->bibtexCO->clear();
	biblioModule->bibtexCO->addItem(qt_("Default"), QString("default"));
	for (set<string>::const_iterator it = lyxrc.bibtex_alternatives.begin();
			     it != lyxrc.bibtex_alternatives.end(); ++it) {
		QString const command = toqstr(*it).left(toqstr(*it).indexOf(" "));
		biblioModule->bibtexCO->addItem(command, command);
	}


	// indices
	indicesModule = new GuiIndices;
	connect(indicesModule, SIGNAL(changed()),
		this, SLOT(change_adaptor()));


	// maths
	mathsModule = new UiWidget<Ui::MathsUi>;
	QStringList headers;
	headers << qt_("Package") << qt_("Load automatically")
		<< qt_("Load always") << qt_("Do not load");
	mathsModule->packagesTW->setHorizontalHeaderLabels(headers);
	setSectionResizeMode(mathsModule->packagesTW->horizontalHeader(), QHeaderView::Stretch);
	map<string, string> const & packages = BufferParams::auto_packages();
	mathsModule->packagesTW->setRowCount(packages.size());
	int i = 0;
	for (map<string, string>::const_iterator it = packages.begin();
	     it != packages.end(); ++it) {
		docstring const package = from_ascii(it->first);
		QString autoTooltip = qt_(it->second);
		QString alwaysTooltip;
		if (package == "amsmath")
			alwaysTooltip =
				qt_("The AMS LaTeX packages are always used");
		else
			alwaysTooltip = toqstr(bformat(
				_("The LaTeX package %1$s is always used"),
				package));
		QString neverTooltip;
		if (package == "amsmath")
			neverTooltip =
				qt_("The AMS LaTeX packages are never used");
		else
			neverTooltip = toqstr(bformat(
				_("The LaTeX package %1$s is never used"),
				package));
		QRadioButton * autoRB = new QRadioButton(mathsModule);
		QRadioButton * alwaysRB = new QRadioButton(mathsModule);
		QRadioButton * neverRB = new QRadioButton(mathsModule);
		QButtonGroup * packageGroup = new QButtonGroup(mathsModule);
		packageGroup->addButton(autoRB);
		packageGroup->addButton(alwaysRB);
		packageGroup->addButton(neverRB);
		autoRB->setToolTip(autoTooltip);
		alwaysRB->setToolTip(alwaysTooltip);
		neverRB->setToolTip(neverTooltip);
		QTableWidgetItem * pack = new QTableWidgetItem(toqstr(package));
		mathsModule->packagesTW->setItem(i, 0, pack);
		mathsModule->packagesTW->setCellWidget(i, 1, autoRB);
		mathsModule->packagesTW->setCellWidget(i, 2, alwaysRB);
		mathsModule->packagesTW->setCellWidget(i, 3, neverRB);

		connect(autoRB, SIGNAL(clicked()),
		        this, SLOT(change_adaptor()));
		connect(alwaysRB, SIGNAL(clicked()),
		        this, SLOT(change_adaptor()));
		connect(neverRB, SIGNAL(clicked()),
		        this, SLOT(change_adaptor()));
		++i;
	}
	connect(mathsModule->allPackagesAutoPB, SIGNAL(clicked()),
		this, SLOT(allPackagesAuto()));
	connect(mathsModule->allPackagesAlwaysPB, SIGNAL(clicked()),
		this, SLOT(allPackagesAlways()));
	connect(mathsModule->allPackagesNotPB, SIGNAL(clicked()),
		this, SLOT(allPackagesNot()));
	connect(mathsModule->allPackagesAutoPB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(mathsModule->allPackagesAlwaysPB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(mathsModule->allPackagesNotPB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));


	// latex class
	latexModule = new UiWidget<Ui::LaTeXUi>;
	connect(latexModule->optionsLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(latexModule->defaultOptionsCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(latexModule->psdriverCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(latexModule->classCO, SIGNAL(activated(int)),
		this, SLOT(classChanged_adaptor()));
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
	connect(latexModule->suppressDateCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(latexModule->refstyleCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));

	latexModule->optionsLE->setValidator(new NoNewLineValidator(
		latexModule->optionsLE));
	latexModule->childDocLE->setValidator(new NoNewLineValidator(
		latexModule->childDocLE));

	// postscript drivers
	for (int n = 0; tex_graphics[n][0]; ++n) {
		QString enc = qt_(tex_graphics_gui[n]);
		latexModule->psdriverCO->addItem(enc);
	}
	// latex classes
	LayoutFileList const & bcl = LayoutFileList::get();
	vector<LayoutFileIndex> classList = bcl.classList();
	sort(classList.begin(), classList.end(), less_textclass_avail_desc());

	vector<LayoutFileIndex>::const_iterator cit  = classList.begin();
	vector<LayoutFileIndex>::const_iterator cen = classList.end();
	for (int i = 0; cit != cen; ++cit, ++i) {
		LayoutFile const & tc = bcl[*cit];
		bool const available = tc.isTeXClassAvailable();
		docstring const guiname = translateIfPossible(from_utf8(tc.description()));
		// tooltip sensu "KOMA-Script Article [Class 'scrartcl']"
		QString tooltip = toqstr(bformat(_("%1$s [Class '%2$s']"), guiname, from_utf8(tc.latexname())));
		if (!available) {
			docstring const output_type = (tc.outputType() == lyx::DOCBOOK) ? _("DocBook") : _("LaTeX");
			tooltip += '\n' + toqstr(wrap(bformat(_("Class not found by LyX. "
							   "Please check if you have the matching %1$s class "
							   "and all required packages (%2$s) installed."),
							 output_type, from_utf8(tc.prerequisites(", ")))));
		}
		latexModule->classCO->addItemSort(toqstr(tc.name()),
						  toqstr(guiname),
						  toqstr(translateIfPossible(from_utf8(tc.category()))),
						  tooltip,
						  true, true, true, available);
	}


	// branches
	branchesModule = new GuiBranches;
	connect(branchesModule, SIGNAL(changed()),
		this, SLOT(change_adaptor()));
	connect(branchesModule, SIGNAL(renameBranches(docstring const &, docstring const &)),
		this, SLOT(branchesRename(docstring const &, docstring const &)));
	connect(branchesModule, SIGNAL(okPressed()), this, SLOT(slotOK()));
	updateUnknownBranches();


	// preamble
	preambleModule = new PreambleModule;
	connect(preambleModule, SIGNAL(changed()),
		this, SLOT(change_adaptor()));

	localLayout = new LocalLayout;
	connect(localLayout, SIGNAL(changed()),
		this, SLOT(change_adaptor()));


	// bullets
	bulletsModule = new BulletsModule;
	connect(bulletsModule, SIGNAL(changed()),
		this, SLOT(change_adaptor()));


	// Modules
	modulesModule = new UiWidget<Ui::ModulesUi>;
	modulesModule->availableLV->header()->setVisible(false);
	setSectionResizeMode(modulesModule->availableLV->header(), QHeaderView::ResizeToContents);
	modulesModule->availableLV->header()->setStretchLastSection(false);
	selectionManager =
		new ModuleSelectionManager(modulesModule->availableLV,
			modulesModule->selectedLV,
			modulesModule->addPB, modulesModule->deletePB,
			modulesModule->upPB, modulesModule->downPB,
			availableModel(), selectedModel(), this);
	connect(selectionManager, SIGNAL(updateHook()),
		this, SLOT(updateModuleInfo()));
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

	pdfSupportModule->titleLE->setValidator(new NoNewLineValidator(
		pdfSupportModule->titleLE));
	pdfSupportModule->authorLE->setValidator(new NoNewLineValidator(
		pdfSupportModule->authorLE));
	pdfSupportModule->subjectLE->setValidator(new NoNewLineValidator(
		pdfSupportModule->subjectLE));
	pdfSupportModule->keywordsLE->setValidator(new NoNewLineValidator(
		pdfSupportModule->keywordsLE));
	pdfSupportModule->optionsLE->setValidator(new NoNewLineValidator(
		pdfSupportModule->optionsLE));

	for (int i = 0; backref_opts[i][0]; ++i)
		pdfSupportModule->backrefCO->addItem(qt_(backref_opts_gui[i]));


	// float
	floatModule = new FloatPlacement;
	connect(floatModule, SIGNAL(changed()),
		this, SLOT(change_adaptor()));


	// listings
	listingsModule = new UiWidget<Ui::ListingsSettingsUi>;
	connect(listingsModule->listingsED, SIGNAL(textChanged()),
		this, SLOT(change_adaptor()));
	connect(listingsModule->bypassCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(listingsModule->bypassCB, SIGNAL(clicked()),
		this, SLOT(setListingsMessage()));
	connect(listingsModule->listingsED, SIGNAL(textChanged()),
		this, SLOT(setListingsMessage()));
	listingsModule->listingsTB->setPlainText(
		qt_("Input listings parameters below. Enter ? for a list of parameters."));


	// add the panels
	docPS->addPanel(latexModule, N_("Document Class"));
	docPS->addPanel(masterChildModule, N_("Child Documents"));
	docPS->addPanel(modulesModule, N_("Modules"));
	docPS->addPanel(localLayout, N_("Local Layout"));
	docPS->addPanel(fontModule, N_("Fonts"));
	docPS->addPanel(textLayoutModule, N_("Text Layout"));
	docPS->addPanel(pageLayoutModule, N_("Page Layout"));
	docPS->addPanel(marginsModule, N_("Page Margins"));
	docPS->addPanel(langModule, N_("Language"));
	docPS->addPanel(colorModule, N_("Colors"));
	docPS->addPanel(numberingModule, N_("Numbering & TOC"));
	docPS->addPanel(biblioModule, N_("Bibliography"));
	docPS->addPanel(indicesModule, N_("Indexes"));
	docPS->addPanel(pdfSupportModule, N_("PDF Properties"));
	docPS->addPanel(mathsModule, N_("Math Options"));
	docPS->addPanel(floatModule, N_("Float Placement"));
	docPS->addPanel(listingsModule, N_("Listings[[inset]]"));
	docPS->addPanel(bulletsModule, N_("Bullets"));
	docPS->addPanel(branchesModule, N_("Branches"));
	docPS->addPanel(outputModule, N_("Output"));
	docPS->addPanel(preambleModule, N_("LaTeX Preamble"));
	docPS->setCurrentPanel("Document Class");
// FIXME: hack to work around resizing bug in Qt >= 4.2
// bug verified with Qt 4.2.{0-3} (JSpitzm)
#if QT_VERSION >= 0x040200
	docPS->updateGeometry();
#endif
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
	nonModuleChanged_ = true;
	changed();
}


void GuiDocument::includeonlyClicked(QTreeWidgetItem * item, int)
{
	if (item == 0)
		return;

	string child = fromqstr(item->text(0));
	if (child.empty())
		return;

	if (std::find(includeonlys_.begin(),
		      includeonlys_.end(), child) != includeonlys_.end())
		includeonlys_.remove(child);
	else
		includeonlys_.push_back(child);

	updateIncludeonlys();
	change_adaptor();
}


QString GuiDocument::validateListingsParameters()
{
	// use a cache here to avoid repeated validation
	// of the same parameters
	// FIXME THREAD
	static string param_cache;
	static QString msg_cache;

	if (listingsModule->bypassCB->isChecked())
		return QString();

	string params = fromqstr(listingsModule->listingsED->toPlainText());
	if (params != param_cache) {
		param_cache = params;
		msg_cache = toqstr(InsetListingsParams(params).validate());
	}
	return msg_cache;
}


void GuiDocument::setListingsMessage()
{
	// FIXME THREAD
	static bool isOK = true;
	QString msg = validateListingsParameters();
	if (msg.isEmpty()) {
		if (isOK)
			return;
		isOK = true;
		// listingsTB->setTextColor("black");
		listingsModule->listingsTB->setPlainText(
			qt_("Input listings parameters below. "
		            "Enter ? for a list of parameters."));
	} else {
		isOK = false;
		// listingsTB->setTextColor("red");
		listingsModule->listingsTB->setPlainText(msg);
	}
}


void GuiDocument::setLSpacing(int item)
{
	textLayoutModule->lspacingLE->setEnabled(item == 3);
}


void GuiDocument::setIndent(int item)
{
	bool const enable = (item == 1);
	textLayoutModule->indentLE->setEnabled(enable);
	textLayoutModule->indentLengthCO->setEnabled(enable);
	textLayoutModule->skipLE->setEnabled(false);
	textLayoutModule->skipLengthCO->setEnabled(false);
	isValid();
}


void GuiDocument::enableIndent(bool indent)
{
	textLayoutModule->skipLE->setEnabled(!indent);
	textLayoutModule->skipLengthCO->setEnabled(!indent);
	if (indent)
		setIndent(textLayoutModule->indentCO->currentIndex());
}


void GuiDocument::setSkip(int item)
{
	bool const enable = (item == 3);
	textLayoutModule->skipLE->setEnabled(enable);
	textLayoutModule->skipLengthCO->setEnabled(enable);
	isValid();
}


void GuiDocument::enableSkip(bool skip)
{
	textLayoutModule->indentLE->setEnabled(!skip);
	textLayoutModule->indentLengthCO->setEnabled(!skip);
	if (skip)
		setSkip(textLayoutModule->skipCO->currentIndex());
}


void GuiDocument::setMargins()
{
	bool const extern_geometry =
		documentClass().provides("geometry");
	marginsModule->marginCB->setEnabled(!extern_geometry);
	if (extern_geometry) {
		marginsModule->marginCB->setChecked(false);
		setCustomMargins(true);
	} else {
		marginsModule->marginCB->setChecked(!bp_.use_geometry);
		setCustomMargins(!bp_.use_geometry);
	}
}


void GuiDocument::papersizeChanged(int paper_size)
{
	setCustomPapersize(paper_size == 1);
}


void GuiDocument::setCustomPapersize(bool custom)
{
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
	QColor const & newColor = QColorDialog::getColor(
		rgb2qcolor(set_backgroundcolor), asQWidget());
	if (!newColor.isValid())
		return;
	// set the button color and text
	colorModule->backgroundPB->setStyleSheet(
		colorButtonStyleSheet(newColor));
	colorModule->backgroundPB->setText(qt_("&Change..."));
	// save color
	set_backgroundcolor = rgbFromHexName(fromqstr(newColor.name()));
	is_backgroundcolor = true;
	change_adaptor();
}


void GuiDocument::deleteBackgroundColor()
{
	// set the button color back to default by setting an empty StyleSheet
	colorModule->backgroundPB->setStyleSheet(QLatin1String(""));
	// change button text
	colorModule->backgroundPB->setText(qt_("&Default..."));
	// save default color (white)
	set_backgroundcolor = rgbFromHexName("#ffffff");
	is_backgroundcolor = false;
	change_adaptor();
}


void GuiDocument::changeFontColor()
{
	QColor const & newColor = QColorDialog::getColor(
		rgb2qcolor(set_fontcolor), asQWidget());
	if (!newColor.isValid())
		return;
	// set the button color and text
	colorModule->fontColorPB->setStyleSheet(
		colorButtonStyleSheet(newColor));
	colorModule->fontColorPB->setText(qt_("&Change..."));
	// save color
	set_fontcolor = rgbFromHexName(fromqstr(newColor.name()));
	is_fontcolor = true;
	change_adaptor();
}


void GuiDocument::deleteFontColor()
{
	// set the button color back to default by setting an empty StyleSheet
	colorModule->fontColorPB->setStyleSheet(QLatin1String(""));
	// change button text
	colorModule->fontColorPB->setText(qt_("&Default..."));
	// save default color (black)
	set_fontcolor = rgbFromHexName("#000000");
	is_fontcolor = false;
	change_adaptor();
}


void GuiDocument::changeNoteFontColor()
{
	QColor const & newColor = QColorDialog::getColor(
		rgb2qcolor(set_notefontcolor), asQWidget());
	if (!newColor.isValid())
		return;
	// set the button color
	colorModule->noteFontColorPB->setStyleSheet(
		colorButtonStyleSheet(newColor));
	// save color
	set_notefontcolor = rgbFromHexName(fromqstr(newColor.name()));
	change_adaptor();
}


void GuiDocument::deleteNoteFontColor()
{
	// set the button color back to pref
	theApp()->getRgbColor(Color_greyedouttext, set_notefontcolor);
	colorModule->noteFontColorPB->setStyleSheet(
		colorButtonStyleSheet(rgb2qcolor(set_notefontcolor)));
	change_adaptor();
}


void GuiDocument::changeBoxBackgroundColor()
{
	QColor const & newColor = QColorDialog::getColor(
		rgb2qcolor(set_boxbgcolor), asQWidget());
	if (!newColor.isValid())
		return;
	// set the button color
	colorModule->boxBackgroundPB->setStyleSheet(
		colorButtonStyleSheet(newColor));
	// save color
	set_boxbgcolor = rgbFromHexName(fromqstr(newColor.name()));
	change_adaptor();
}


void GuiDocument::deleteBoxBackgroundColor()
{
	// set the button color back to pref
	theApp()->getRgbColor(Color_shadedbg, set_boxbgcolor);
	colorModule->boxBackgroundPB->setStyleSheet(
		colorButtonStyleSheet(rgb2qcolor(set_boxbgcolor)));
	change_adaptor();
}


void GuiDocument::languageChanged(int i)
{
	// some languages only work with polyglossia/XeTeX
	Language const * lang = lyx::languages.getLanguage(
		fromqstr(langModule->languageCO->itemData(i).toString()));
	if (lang->babel().empty() && !lang->polyglossia().empty()) {
			// If we force to switch fontspec on, store
			// current state (#8717)
			if (fontModule->osFontsCB->isEnabled())
				forced_fontspec_activation =
					!fontModule->osFontsCB->isChecked();
			fontModule->osFontsCB->setChecked(true);
			fontModule->osFontsCB->setEnabled(false);
	}
	else {
		fontModule->osFontsCB->setEnabled(true);
		// If we have forced to switch fontspec on,
		// restore previous state (#8717)
		if (forced_fontspec_activation)
			fontModule->osFontsCB->setChecked(false);
		forced_fontspec_activation = false;
	}

	// set appropriate quotation mark style
	if (!lang->quoteStyle().empty()) {
		langModule->quoteStyleCO->setCurrentIndex(
			bp_.getQuoteStyle(lang->quoteStyle()));
	}
}


void GuiDocument::osFontsChanged(bool nontexfonts)
{
	bool const tex_fonts = !nontexfonts;
	// store current fonts
	QString const font_roman = fontModule->fontsRomanCO->itemData(
			fontModule->fontsRomanCO->currentIndex()).toString();
	QString const font_sans = fontModule->fontsSansCO->itemData(
			fontModule->fontsSansCO->currentIndex()).toString();
	QString const font_typewriter = fontModule->fontsTypewriterCO->itemData(
			fontModule->fontsTypewriterCO->currentIndex()).toString();
	QString const font_math = fontModule->fontsMathCO->itemData(
			fontModule->fontsMathCO->currentIndex()).toString();
	int const font_sf_scale = fontModule->scaleSansSB->value();
	int const font_tt_scale = fontModule->scaleTypewriterSB->value();

	updateFontlist();
	// store default format
	QString const dformat = outputModule->defaultFormatCO->itemData(
		outputModule->defaultFormatCO->currentIndex()).toString();
	updateDefaultFormat();
	// try to restore default format
	int index = outputModule->defaultFormatCO->findData(dformat);
	// set to default if format is not found
	if (index == -1)
		index = 0;
	outputModule->defaultFormatCO->setCurrentIndex(index);

	// try to restore fonts which were selected two toggles ago
	index = fontModule->fontsRomanCO->findData(fontModule->font_roman);
	if (index != -1)
		fontModule->fontsRomanCO->setCurrentIndex(index);
	index = fontModule->fontsSansCO->findData(fontModule->font_sans);
	if (index != -1)
		fontModule->fontsSansCO->setCurrentIndex(index);
	index = fontModule->fontsTypewriterCO->findData(fontModule->font_typewriter);
	if (index != -1)
		fontModule->fontsTypewriterCO->setCurrentIndex(index);
	index = fontModule->fontsMathCO->findData(fontModule->font_math);
	if (index != -1)
		fontModule->fontsMathCO->setCurrentIndex(index);
	// save fonts for next next toggle
	fontModule->font_roman = font_roman;
	fontModule->font_sans = font_sans;
	fontModule->font_typewriter = font_typewriter;
	fontModule->font_math = font_math;
	fontModule->font_sf_scale = font_sf_scale;
	fontModule->font_tt_scale = font_tt_scale;

	langModule->encodingCO->setEnabled(tex_fonts &&
		!langModule->defaultencodingRB->isChecked());
	langModule->defaultencodingRB->setEnabled(tex_fonts);
	langModule->otherencodingRB->setEnabled(tex_fonts);

	fontModule->fontsDefaultCO->setEnabled(tex_fonts);
	fontModule->fontsDefaultLA->setEnabled(tex_fonts);
	fontModule->cjkFontLE->setEnabled(tex_fonts);
	fontModule->cjkFontLA->setEnabled(tex_fonts);

	updateFontOptions();

	fontModule->fontencLA->setEnabled(tex_fonts);
	fontModule->fontencCO->setEnabled(tex_fonts);
	if (!tex_fonts)
		fontModule->fontencLE->setEnabled(false);
	else
		fontencChanged(fontModule->fontencCO->currentIndex());
}


void GuiDocument::mathFontChanged(int)
{
	updateFontOptions();
}


void GuiDocument::fontOsfToggled(bool state)
{
	if (fontModule->osFontsCB->isChecked())
		return;
	QString font = fontModule->fontsRomanCO->itemData(
			fontModule->fontsRomanCO->currentIndex()).toString();
	if (hasMonolithicExpertSet(font))
		fontModule->fontScCB->setChecked(state);
}


void GuiDocument::fontScToggled(bool state)
{
	if (fontModule->osFontsCB->isChecked())
		return;
	QString font = fontModule->fontsRomanCO->itemData(
			fontModule->fontsRomanCO->currentIndex()).toString();
	if (hasMonolithicExpertSet(font))
		fontModule->fontOsfCB->setChecked(state);
}


void GuiDocument::updateFontOptions()
{
	bool const tex_fonts = !fontModule->osFontsCB->isChecked();
	QString font;
	if (tex_fonts)
		font = fontModule->fontsSansCO->itemData(
				fontModule->fontsSansCO->currentIndex()).toString();
	bool scaleable = providesScale(font);
	fontModule->scaleSansSB->setEnabled(scaleable);
	fontModule->scaleSansLA->setEnabled(scaleable);
	if (tex_fonts)
		font = fontModule->fontsTypewriterCO->itemData(
				fontModule->fontsTypewriterCO->currentIndex()).toString();
	scaleable = providesScale(font);
	fontModule->scaleTypewriterSB->setEnabled(scaleable);
	fontModule->scaleTypewriterLA->setEnabled(scaleable);
	if (tex_fonts)
		font = fontModule->fontsRomanCO->itemData(
				fontModule->fontsRomanCO->currentIndex()).toString();
	fontModule->fontScCB->setEnabled(providesSC(font));
	fontModule->fontOsfCB->setEnabled(providesOSF(font));
	updateMathFonts(font);
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


bool GuiDocument::ot1() const
{
	QString const fontenc =
		fontModule->fontencCO->itemData(fontModule->fontencCO->currentIndex()).toString();
	return (fontenc == "default"
		|| (fontenc == "global" && (lyxrc.fontenc == "default" || lyxrc.fontenc == "OT1"))
		|| (fontenc == "custom" && fontModule->fontencLE->text() == "OT1"));
}


bool GuiDocument::completeFontset() const
{
	return (fontModule->fontsSansCO->itemData(
			fontModule->fontsSansCO->currentIndex()).toString() == "default"
		&& fontModule->fontsSansCO->itemData(
			fontModule->fontsTypewriterCO->currentIndex()).toString() == "default");
}


bool GuiDocument::noMathFont() const
{
	return (fontModule->fontsMathCO->itemData(
	        fontModule->fontsMathCO->currentIndex()).toString() == "default");
}


void GuiDocument::updateTexFonts()
{
	LaTeXFonts::TexFontMap texfontmap = theLaTeXFonts().getLaTeXFonts();

	LaTeXFonts::TexFontMap::const_iterator it = texfontmap.begin();
	LaTeXFonts::TexFontMap::const_iterator end = texfontmap.end();
	for (; it != end; ++it) {
		LaTeXFont lf = it->second;
		if (lf.name().empty()) {
			LYXERR0("Error: Unnamed font: " << it->first);
			continue;
		}
		docstring const family = lf.family();
		docstring guiname = translateIfPossible(lf.guiname());
		if (!lf.available(ot1(), noMathFont()))
			guiname += _(" (not installed)");
		if (family == "rm")
			rmfonts_.insert(toqstr(guiname), toqstr(it->first));
		else if (family == "sf")
			sffonts_.insert(toqstr(guiname), toqstr(it->first));
		else if (family == "tt")
			ttfonts_.insert(toqstr(guiname), toqstr(it->first));
		else if (family == "math")
			mathfonts_.insert(toqstr(guiname), toqstr(it->first));
	}
}


void GuiDocument::updateFontlist()
{
	fontModule->fontsRomanCO->clear();
	fontModule->fontsSansCO->clear();
	fontModule->fontsTypewriterCO->clear();
	fontModule->fontsMathCO->clear();

	// With fontspec (XeTeX, LuaTeX), we have access to all system fonts, but not the LaTeX fonts
	if (fontModule->osFontsCB->isChecked()) {
		fontModule->fontsRomanCO->addItem(qt_("Default"), QString("default"));
		fontModule->fontsSansCO->addItem(qt_("Default"), QString("default"));
		fontModule->fontsTypewriterCO->addItem(qt_("Default"), QString("default"));
		QString unimath = qt_("Non-TeX Fonts Default");
		if (!LaTeXFeatures::isAvailable("unicode-math"))
			unimath += qt_(" (not available)");
		fontModule->fontsMathCO->addItem(qt_("Class Default (TeX Fonts)"), QString("auto"));
		fontModule->fontsMathCO->addItem(unimath, QString("default"));

		QFontDatabase fontdb;
		QStringList families(fontdb.families());
		for (QStringList::Iterator it = families.begin(); it != families.end(); ++it) {
			fontModule->fontsRomanCO->addItem(*it, *it);
			fontModule->fontsSansCO->addItem(*it, *it);
			fontModule->fontsTypewriterCO->addItem(*it, *it);
		}
		return;
	}

	if (rmfonts_.empty())
		updateTexFonts();

	fontModule->fontsRomanCO->addItem(qt_("Default"), QString("default"));
	QMap<QString, QString>::const_iterator rmi = rmfonts_.constBegin();
	while (rmi != rmfonts_.constEnd()) {
		fontModule->fontsRomanCO->addItem(rmi.key(), rmi.value());
		++rmi;
	}

	fontModule->fontsSansCO->addItem(qt_("Default"), QString("default"));
	QMap<QString, QString>::const_iterator sfi = sffonts_.constBegin();
	while (sfi != sffonts_.constEnd()) {
		fontModule->fontsSansCO->addItem(sfi.key(), sfi.value());
		++sfi;
	}

	fontModule->fontsTypewriterCO->addItem(qt_("Default"), QString("default"));
	QMap<QString, QString>::const_iterator tti = ttfonts_.constBegin();
	while (tti != ttfonts_.constEnd()) {
		fontModule->fontsTypewriterCO->addItem(tti.key(), tti.value());
		++tti;
	}

	fontModule->fontsMathCO->addItem(qt_("Automatic"), QString("auto"));
	fontModule->fontsMathCO->addItem(qt_("Class Default"), QString("default"));
	QMap<QString, QString>::const_iterator mmi = mathfonts_.constBegin();
	while (mmi != mathfonts_.constEnd()) {
		fontModule->fontsMathCO->addItem(mmi.key(), mmi.value());
		++mmi;
	}
}


void GuiDocument::fontencChanged(int item)
{
	fontModule->fontencLE->setEnabled(
		fontModule->fontencCO->itemData(item).toString() == "custom");
	// The availability of TeX fonts depends on the font encoding
	updateTexFonts();
	updateFontOptions();
}


void GuiDocument::updateMathFonts(QString const & rm)
{
	if (fontModule->osFontsCB->isChecked())
		return;
	QString const math =
		fontModule->fontsMathCO->itemData(fontModule->fontsMathCO->currentIndex()).toString();
	int const i = fontModule->fontsMathCO->findData("default");
	if (providesNoMath(rm) && i == -1)
		fontModule->fontsMathCO->insertItem(1, qt_("Class Default"), QString("default"));
	else if (!providesNoMath(rm) && i != -1) {
		int const c = fontModule->fontsMathCO->currentIndex();
		fontModule->fontsMathCO->removeItem(i);
		if (c == i)
			fontModule->fontsMathCO->setCurrentIndex(0);
	}
}


void GuiDocument::romanChanged(int item)
{
	if (fontModule->osFontsCB->isChecked())
		return;
	QString const font =
		fontModule->fontsRomanCO->itemData(item).toString();
	fontModule->fontScCB->setEnabled(providesSC(font));
	fontModule->fontOsfCB->setEnabled(providesOSF(font));
	updateMathFonts(font);
}


void GuiDocument::sansChanged(int item)
{
	if (fontModule->osFontsCB->isChecked())
		return;
	QString const font =
		fontModule->fontsSansCO->itemData(item).toString();
	bool scaleable = providesScale(font);
	fontModule->scaleSansSB->setEnabled(scaleable);
	fontModule->scaleSansLA->setEnabled(scaleable);
}


void GuiDocument::ttChanged(int item)
{
	if (fontModule->osFontsCB->isChecked())
		return;
	QString const font =
		fontModule->fontsTypewriterCO->itemData(item).toString();
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
	QString file = browseRelToParent(QString(), bufferFilePath(),
		qt_("Local layout file"), filter, false,
		label1, dir1);

	if (!file.endsWith(".layout"))
		return;

	FileName layoutFile = support::makeAbsPath(fromqstr(file),
		fromqstr(bufferFilePath()));

	int const ret = Alert::prompt(_("Local layout file"),
		_("The layout file you have selected is a local layout\n"
		  "file, not one in the system or user directory.\n"
		  "Your document will not work with this layout if you\n"
		  "move the layout file to a different directory."),
		  1, 1, _("&Set Layout"), _("&Cancel"));
	if (ret == 1)
		return;

	// load the layout file
	LayoutFileList & bcl = LayoutFileList::get();
	string classname = layoutFile.onlyFileName();
	// this will update an existing layout if that layout has been loaded before.
	LayoutFileIndex name = support::onlyFileName(bcl.addLocalLayout(
		classname.substr(0, classname.size() - 7),
		layoutFile.onlyPath().absFileName()));

	if (name.empty()) {
		Alert::error(_("Error"),
			_("Unable to read local layout file."));
		return;
	}

	const_cast<Buffer &>(buffer()).setLayoutPos(layoutFile.onlyPath().absFileName());

	// do not trigger classChanged if there is no change.
	if (latexModule->classCO->currentText() == toqstr(name))
		return;

	// add to combo box
	bool const avail = latexModule->classCO->set(toqstr(name));
	if (!avail) {
		LayoutFile const & tc = bcl[name];
		docstring const guiname = translateIfPossible(from_utf8(tc.description()));
		// tooltip sensu "KOMA-Script Article [Class 'scrartcl']"
		QString tooltip = toqstr(bformat(_("%1$s [Class '%2$s']"), guiname, from_utf8(tc.latexname())));
		tooltip += '\n' + qt_("This is a local layout file.");
		latexModule->classCO->addItemSort(toqstr(tc.name()), toqstr(guiname),
						  toqstr(translateIfPossible(from_utf8(tc.category()))),
						  tooltip,
						  true, true, true, true);
		latexModule->classCO->set(toqstr(name));
	}

	classChanged();
}


void GuiDocument::browseMaster()
{
	QString const title = qt_("Select master document");
	QString const dir1 = toqstr(lyxrc.document_path);
	QString const old = latexModule->childDocLE->text();
	QString const docpath = toqstr(support::onlyPath(buffer().absFileName()));
	QStringList const filter(qt_("LyX Files (*.lyx)"));
	QString file = browseRelToSub(old, docpath, title, filter, false,
		qt_("Documents|#o#O"), toqstr(lyxrc.document_path));

	if (!file.isEmpty())
		latexModule->childDocLE->setText(file);
}


void GuiDocument::classChanged_adaptor()
{
	const_cast<Buffer &>(buffer()).setLayoutPos(string());
	classChanged();
}


void GuiDocument::classChanged()
{
	int idx = latexModule->classCO->currentIndex();
	if (idx < 0)
		return;
	string const classname = fromqstr(latexModule->classCO->getData(idx));

	if (applyPB->isEnabled()) {
		int const ret = Alert::prompt(_("Unapplied changes"),
				_("Some changes in the dialog were not yet applied.\n"
				"If you do not apply now, they will be lost after this action."),
				1, 1, _("&Apply"), _("&Dismiss"));
		if (ret == 0)
			applyView();
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


void GuiDocument::languagePackageChanged(int i)
{
	 langModule->languagePackageLE->setEnabled(
		langModule->languagePackageCO->itemData(i).toString() == "custom");
}


void GuiDocument::biblioChanged()
{
	biblioChanged_ = true;
	change_adaptor();
}


void GuiDocument::bibtexChanged(int n)
{
	biblioModule->bibtexOptionsLE->setEnabled(
		biblioModule->bibtexCO->itemData(n).toString() != "default");
	biblioChanged();
}


void GuiDocument::setAuthorYear(bool authoryear)
{
	if (authoryear)
		biblioModule->citeStyleCO->setCurrentIndex(0);
	biblioChanged();
}


void GuiDocument::setNumerical(bool numerical)
{
	if (numerical)
		biblioModule->citeStyleCO->setCurrentIndex(1);
	biblioChanged();
}


void GuiDocument::updateEngineType(string const & items, CiteEngineType const & sel)
{
	engine_types_.clear();

	int nn = 0;

	for (int n = 0; !token(items, '|', n).empty(); ++n) {
		nn += 1;
		string style = token(items, '|', n);
		engine_types_.push_back(style);
	}

	switch (sel) {
		case ENGINE_TYPE_AUTHORYEAR:
			biblioModule->citeStyleCO->setCurrentIndex(0);
			break;
		case ENGINE_TYPE_NUMERICAL:
		case ENGINE_TYPE_DEFAULT:
			biblioModule->citeStyleCO->setCurrentIndex(1);
			break;
	}

	biblioModule->citationStyleL->setEnabled(nn > 1);
	biblioModule->citeStyleCO->setEnabled(nn > 1);

	if (nn != 1)
		return;

	// If the textclass allows only one of authoryear or numerical,
	// we have no choice but to force that engine type.
	if (engine_types_[0] == "authoryear")
		biblioModule->citeStyleCO->setCurrentIndex(0);
	else
		biblioModule->citeStyleCO->setCurrentIndex(1);
}


namespace {
	// FIXME unicode
	// both of these should take a vector<docstring>

	// This is an insanely complicated attempt to make this sort of thing
	// work with RTL languages.
	docstring formatStrVec(vector<string> const & v, docstring const & s)
	{
		//this mess formats the list as "v[0], v[1], ..., [s] v[n]"
		if (v.empty())
			return docstring();
		if (v.size() == 1)
			return translateIfPossible(from_utf8(v[0]));
		if (v.size() == 2) {
			docstring retval = _("%1$s and %2$s");
			retval = subst(retval, _("and"), s);
			return bformat(retval, translateIfPossible(from_utf8(v[0])),
				       translateIfPossible(from_utf8(v[1])));
		}
		// The idea here is to format all but the last two items...
		int const vSize = v.size();
		docstring t2 = _("%1$s, %2$s");
		docstring retval = translateIfPossible(from_utf8(v[0]));
		for (int i = 1; i < vSize - 2; ++i)
			retval = bformat(t2, retval, translateIfPossible(from_utf8(v[i])));
		//...and then to  plug them, and the last two, into this schema
		docstring t = _("%1$s, %2$s, and %3$s");
		t = subst(t, _("and"), s);
		return bformat(t, retval, translateIfPossible(from_utf8(v[vSize - 2])),
			       translateIfPossible(from_utf8(v[vSize - 1])));
	}

	vector<string> idsToNames(vector<string> const & idList)
	{
		vector<string> retval;
		vector<string>::const_iterator it  = idList.begin();
		vector<string>::const_iterator end = idList.end();
		for (; it != end; ++it) {
			LyXModule const * const mod = theModuleList[*it];
			if (!mod)
				retval.push_back(to_utf8(bformat(_("%1$s (unavailable)"),
						translateIfPossible(from_utf8(*it)))));
			else
				retval.push_back(mod->getName());
		}
		return retval;
	}
} // end anonymous namespace


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
	for (; rit != ren; ++rit) {
		list<string>::const_iterator mit = bp.getModules().begin();
		list<string>::const_iterator men = bp.getModules().end();
		bool found = false;
		for (; mit != men; ++mit) {
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

	if (applyPB->isEnabled() && nonModuleChanged_) {
		int const ret = Alert::prompt(_("Unapplied changes"),
				_("Some changes in the dialog were not yet applied.\n"
				"If you do not apply now, they will be lost after this action."),
				1, 1, _("&Apply"), _("&Dismiss"));
		if (ret == 0)
			applyView();
	}

	bp_.makeDocumentClass();
	paramsToDialog();
	changed();
}


void GuiDocument::updateModuleInfo()
{
	selectionManager->update();

	//Module description
	bool const focus_on_selected = selectionManager->selectedFocused();
	QAbstractItemView * lv;
	if (focus_on_selected)
		lv = modulesModule->selectedLV;
	else
		lv = modulesModule->availableLV;
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

	docstring cat = getModuleCategory(modName);
	if (!cat.empty()) {
		if (!desc.empty())
			desc += "\n";
		desc += bformat(_("Category: %1$s."), cat);
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
		desc += bformat(_("Modules required: %1$s."), pkgdesc);
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
		if (toclevel != Layout::NOT_IN_TOC && !lit->counter.empty()) {
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
	if (!bufferview())
		return;
	// make a copy in order to consider unapplied changes
	BufferParams param_copy = buffer().params();
	param_copy.useNonTeXFonts = fontModule->osFontsCB->isChecked();
	int const idx = latexModule->classCO->currentIndex();
	if (idx >= 0) {
		string const classname = fromqstr(latexModule->classCO->getData(idx));
		param_copy.setBaseClass(classname);
		param_copy.makeDocumentClass(true);
	}
	outputModule->defaultFormatCO->blockSignals(true);
	outputModule->defaultFormatCO->clear();
	outputModule->defaultFormatCO->addItem(qt_("Default"),
				QVariant(QString("default")));
	typedef vector<Format const *> Formats;
	Formats formats = param_copy.exportableFormats(true);
	sort(formats.begin(), formats.end(), Format::formatSorter);
	Formats::const_iterator cit = formats.begin();
	Formats::const_iterator end = formats.end();
	for (; cit != end; ++cit)
		outputModule->defaultFormatCO->addItem(qt_((*cit)->prettyname()),
				QVariant(toqstr((*cit)->name())));
	outputModule->defaultFormatCO->blockSignals(false);
}


bool GuiDocument::isChildIncluded(string const & child)
{
	if (includeonlys_.empty())
		return false;
	return (std::find(includeonlys_.begin(),
			  includeonlys_.end(), child) != includeonlys_.end());
}


void GuiDocument::applyView()
{
	// preamble
	preambleModule->apply(bp_);
	localLayout->apply(bp_);

	// date
	bp_.suppress_date = latexModule->suppressDateCB->isChecked();
	bp_.use_refstyle  = latexModule->refstyleCB->isChecked();

	// biblio
	if (biblioModule->citeNatbibRB->isChecked())
		bp_.setCiteEngine("natbib");
	else if (biblioModule->citeJurabibRB->isChecked())
		bp_.setCiteEngine("jurabib");
	if (biblioModule->citeDefaultRB->isChecked()) {
		bp_.setCiteEngine("basic");
		bp_.setCiteEngineType(ENGINE_TYPE_DEFAULT);
	}
	else
	if (biblioModule->citeStyleCO->currentIndex())
		bp_.setCiteEngineType(ENGINE_TYPE_NUMERICAL);
	else
		bp_.setCiteEngineType(ENGINE_TYPE_AUTHORYEAR);

	bp_.use_bibtopic =
		biblioModule->bibtopicCB->isChecked();

	bp_.biblio_style = fromqstr(biblioModule->bibtexStyleLE->text());

	string const bibtex_command =
		fromqstr(biblioModule->bibtexCO->itemData(
			biblioModule->bibtexCO->currentIndex()).toString());
	string const bibtex_options =
		fromqstr(biblioModule->bibtexOptionsLE->text());
	if (bibtex_command == "default" || bibtex_options.empty())
		bp_.bibtex_command = bibtex_command;
	else
		bp_.bibtex_command = bibtex_command + " " + bibtex_options;

	if (biblioChanged_) {
		buffer().invalidateBibinfoCache();
		buffer().removeBiblioTempFiles();
	}

	// Indices
	indicesModule->apply(bp_);

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
				if (qt_(it->guiName()) == enc_gui &&
				    !it->unsafe()) {
					bp_.inputenc = it->name();
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

	bp_.quotes_language = (InsetQuotes::QuoteLanguage) langModule->quoteStyleCO->itemData(
		langModule->quoteStyleCO->currentIndex()).toInt();

	QString const langname = langModule->languageCO->itemData(
		langModule->languageCO->currentIndex()).toString();
	Language const * newlang = lyx::languages.getLanguage(fromqstr(langname));
	Cursor & cur = const_cast<BufferView *>(bufferview())->cursor();
	// If current cursor language was the document language, then update it too.
	if (cur.current_font.language() == bp_.language) {
		cur.current_font.setLanguage(newlang);
		cur.real_current_font.setLanguage(newlang);
	}
	bp_.language = newlang;

	QString const pack = langModule->languagePackageCO->itemData(
		langModule->languagePackageCO->currentIndex()).toString();
	if (pack == "custom")
		bp_.lang_package =
			fromqstr(langModule->languagePackageLE->text());
	else
		bp_.lang_package = fromqstr(pack);

	//color
	bp_.backgroundcolor = set_backgroundcolor;
	bp_.isbackgroundcolor = is_backgroundcolor;
	bp_.fontcolor = set_fontcolor;
	bp_.isfontcolor = is_fontcolor;
	bp_.notefontcolor = set_notefontcolor;
	bp_.boxbgcolor = set_boxbgcolor;

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
	bp_.graphics_driver =
		tex_graphics[latexModule->psdriverCO->currentIndex()];

	// text layout
	int idx = latexModule->classCO->currentIndex();
	if (idx >= 0) {
		string const classname = fromqstr(latexModule->classCO->getData(idx));
		bp_.setBaseClass(classname);
	}

	// Modules
	modulesToParams(bp_);

	// Math
	map<string, string> const & packages = BufferParams::auto_packages();
	for (map<string, string>::const_iterator it = packages.begin();
	     it != packages.end(); ++it) {
		QTableWidgetItem * item = mathsModule->packagesTW->findItems(toqstr(it->first), Qt::MatchExactly)[0];
		if (!item)
			continue;
		int row = mathsModule->packagesTW->row(item);
		QRadioButton * rb = (QRadioButton*)mathsModule->packagesTW->cellWidget(row, 1);
		if (rb->isChecked()) {
			bp_.use_package(it->first, BufferParams::package_auto);
			continue;
		}
		rb = (QRadioButton*)mathsModule->packagesTW->cellWidget(row, 2);
		if (rb->isChecked()) {
			bp_.use_package(it->first, BufferParams::package_on);
			continue;
		}
		rb = (QRadioButton*)mathsModule->packagesTW->cellWidget(row, 3);
		if (rb->isChecked())
			bp_.use_package(it->first, BufferParams::package_off);
	}

	// Page Layout
	if (pageLayoutModule->pagestyleCO->currentIndex() == 0)
		bp_.pagestyle = "default";
	else {
		QString style_gui = pageLayoutModule->pagestyleCO->currentText();
		for (size_t i = 0; i != pagestyles.size(); ++i)
			if (pagestyles[i].second == style_gui)
				bp_.pagestyle = pagestyles[i].first;
	}

	// Text Layout
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
	case 3: {
		string s = widgetToDoubleStr(textLayoutModule->lspacingLE);
		if (s.empty())
			bp_.spacing().set(Spacing::Single);
		else
			bp_.spacing().set(Spacing::Other, s);
		break;
		}
	}

	if (textLayoutModule->twoColumnCB->isChecked())
		bp_.columns = 2;
	else
		bp_.columns = 1;

	bp_.justification = textLayoutModule->justCB->isChecked();

	if (textLayoutModule->indentRB->isChecked()) {
		// if paragraphs are separated by an indentation
		bp_.paragraph_separation = BufferParams::ParagraphIndentSeparation;
		switch (textLayoutModule->indentCO->currentIndex()) {
		case 0:
			bp_.setIndentation(HSpace(HSpace::DEFAULT));
			break;
		case 1:	{
			HSpace indent = HSpace(
				widgetsToLength(textLayoutModule->indentLE,
				textLayoutModule->indentLengthCO)
				);
			bp_.setIndentation(indent);
			break;
			}
		default:
			// this should never happen
			bp_.setIndentation(HSpace(HSpace::DEFAULT));
			break;
		}
	} else {
		// if paragraphs are separated by a skip
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
			// this should never happen
			bp_.setDefSkip(VSpace(VSpace::MEDSKIP));
			break;
		}
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

	// Master/Child
	bp_.clearIncludedChildren();
	if (masterChildModule->includeonlyRB->isChecked()) {
		list<string>::const_iterator it = includeonlys_.begin();
		for (; it != includeonlys_.end() ; ++it) {
			bp_.addIncludedChildren(*it);
		}
	}
	bp_.maintain_unincluded_children =
		masterChildModule->maintainAuxCB->isChecked();

	// Float Placement
	bp_.float_placement = floatModule->get();

	// Listings
	// text should have passed validation
	bp_.listings_params =
		InsetListingsParams(fromqstr(listingsModule->listingsED->toPlainText())).params();

	// output
	bp_.default_output_format = fromqstr(outputModule->defaultFormatCO->itemData(
		outputModule->defaultFormatCO->currentIndex()).toString());

	bool const nontexfonts = fontModule->osFontsCB->isChecked();
	bp_.useNonTeXFonts = nontexfonts;

	bp_.output_sync = outputModule->outputsyncCB->isChecked();

	bp_.output_sync_macro = fromqstr(outputModule->synccustomCB->currentText());

	int mathfmt = outputModule->mathoutCB->currentIndex();
	if (mathfmt == -1)
		mathfmt = 0;
	BufferParams::MathOutput const mo =
		static_cast<BufferParams::MathOutput>(mathfmt);
	bp_.html_math_output = mo;
	bp_.html_be_strict = outputModule->strictCB->isChecked();
	bp_.html_css_as_file = outputModule->cssCB->isChecked();
	bp_.html_math_img_scale = outputModule->mathimgSB->value();
	bp_.display_pixel_ratio = theGuiApp()->pixelRatio();

	// fonts
	bp_.fonts_roman[nontexfonts] =
		fromqstr(fontModule->fontsRomanCO->
			itemData(fontModule->fontsRomanCO->currentIndex()).toString());
	bp_.fonts_roman[!nontexfonts] = fromqstr(fontModule->font_roman);

	bp_.fonts_sans[nontexfonts] =
		fromqstr(fontModule->fontsSansCO->
			itemData(fontModule->fontsSansCO->currentIndex()).toString());
	bp_.fonts_sans[!nontexfonts] = fromqstr(fontModule->font_sans);

	bp_.fonts_typewriter[nontexfonts] =
		fromqstr(fontModule->fontsTypewriterCO->
			itemData(fontModule->fontsTypewriterCO->currentIndex()).toString());
	bp_.fonts_typewriter[!nontexfonts] = fromqstr(fontModule->font_typewriter);

	bp_.fonts_math[nontexfonts] =
		fromqstr(fontModule->fontsMathCO->
			itemData(fontModule->fontsMathCO->currentIndex()).toString());
	bp_.fonts_math[!nontexfonts] = fromqstr(fontModule->font_math);

	QString const fontenc =
		fontModule->fontencCO->itemData(fontModule->fontencCO->currentIndex()).toString();
	if (fontenc == "custom")
		bp_.fontenc = fromqstr(fontModule->fontencLE->text());
	else
		bp_.fontenc = fromqstr(fontenc);

	bp_.fonts_cjk =
		fromqstr(fontModule->cjkFontLE->text());

	bp_.fonts_sans_scale[nontexfonts] = fontModule->scaleSansSB->value();
	bp_.fonts_sans_scale[!nontexfonts] = fontModule->font_sf_scale;

	bp_.fonts_typewriter_scale[nontexfonts] = fontModule->scaleTypewriterSB->value();
	bp_.fonts_typewriter_scale[!nontexfonts] = fontModule->font_tt_scale;

	bp_.fonts_expert_sc = fontModule->fontScCB->isChecked();

	bp_.fonts_old_figures = fontModule->fontOsfCB->isChecked();

	if (nontexfonts)
		bp_.fonts_default_family = "default";
	else
		bp_.fonts_default_family = GuiDocument::fontfamilies[
			fontModule->fontsDefaultCO->currentIndex()];

	if (fontModule->fontsizeCO->currentIndex() == 0)
		bp_.fontsize = "default";
	else
		bp_.fontsize =
			fromqstr(fontModule->fontsizeCO->currentText());

	// paper
	bp_.papersize = PAPER_SIZE(
		pageLayoutModule->papersizeCO->currentIndex());

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

	// margins
	bp_.use_geometry = !marginsModule->marginCB->isChecked();

	Ui::MarginsUi const * m = marginsModule;

	bp_.leftmargin = widgetsToLength(m->innerLE, m->innerUnit);
	bp_.topmargin = widgetsToLength(m->topLE, m->topUnit);
	bp_.rightmargin = widgetsToLength(m->outerLE, m->outerUnit);
	bp_.bottommargin = widgetsToLength(m->bottomLE, m->bottomUnit);
	bp_.headheight = widgetsToLength(m->headheightLE, m->headheightUnit);
	bp_.headsep = widgetsToLength(m->headsepLE, m->headsepUnit);
	bp_.footskip = widgetsToLength(m->footskipLE, m->footskipUnit);
	bp_.columnsep = widgetsToLength(m->columnsepLE, m->columnsepUnit);

	// branches
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

	// reset tracker
	nonModuleChanged_ = false;
}


void GuiDocument::paramsToDialog()
{
	// set the default unit
	Length::UNIT const default_unit = Length::defaultUnit();

	// preamble
	preambleModule->update(bp_, id());
	localLayout->update(bp_, id());

	// date
	latexModule->suppressDateCB->setChecked(bp_.suppress_date);
	latexModule->refstyleCB->setChecked(bp_.use_refstyle);

	// biblio
	string const cite_engine = bp_.citeEngine().list().front();

	biblioModule->citeDefaultRB->setChecked(
		cite_engine == "basic");

	biblioModule->citeJurabibRB->setChecked(
		cite_engine == "jurabib");

	biblioModule->citeNatbibRB->setChecked(
		cite_engine == "natbib");

	biblioModule->citeStyleCO->setCurrentIndex(
		bp_.citeEngineType() & ENGINE_TYPE_NUMERICAL);

	updateEngineType(documentClass().opt_enginetype(),
		bp_.citeEngineType());

	biblioModule->bibtopicCB->setChecked(
		bp_.use_bibtopic);

	biblioModule->bibtexStyleLE->setText(toqstr(bp_.biblio_style));

	string command;
	string options =
		split(bp_.bibtex_command, command, ' ');

	int const bpos = biblioModule->bibtexCO->findData(toqstr(command));
	if (bpos != -1) {
		biblioModule->bibtexCO->setCurrentIndex(bpos);
		biblioModule->bibtexOptionsLE->setText(toqstr(options).trimmed());
	} else {
		// We reset to default if we do not know the specified compiler
		// This is for security reasons
		biblioModule->bibtexCO->setCurrentIndex(
			biblioModule->bibtexCO->findData(toqstr("default")));
		biblioModule->bibtexOptionsLE->clear();
	}
	biblioModule->bibtexOptionsLE->setEnabled(
		biblioModule->bibtexCO->currentIndex() != 0);

	biblioChanged_ = false;

	// indices
	indicesModule->update(bp_, buffer().isReadonly());

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
				if (it->name() == bp_.inputenc &&
				    !it->unsafe()) {
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

	int const p = langModule->languagePackageCO->findData(toqstr(bp_.lang_package));
	if (p == -1) {
		langModule->languagePackageCO->setCurrentIndex(
			  langModule->languagePackageCO->findData("custom"));
		langModule->languagePackageLE->setText(toqstr(bp_.lang_package));
	} else {
		langModule->languagePackageCO->setCurrentIndex(p);
		langModule->languagePackageLE->clear();
	}

	//color
	if (bp_.isfontcolor) {
		colorModule->fontColorPB->setStyleSheet(
			colorButtonStyleSheet(rgb2qcolor(bp_.fontcolor)));
	}
	set_fontcolor = bp_.fontcolor;
	is_fontcolor = bp_.isfontcolor;

	colorModule->noteFontColorPB->setStyleSheet(
		colorButtonStyleSheet(rgb2qcolor(bp_.notefontcolor)));
	set_notefontcolor = bp_.notefontcolor;

	if (bp_.isbackgroundcolor) {
		colorModule->backgroundPB->setStyleSheet(
			colorButtonStyleSheet(rgb2qcolor(bp_.backgroundcolor)));
	}
	set_backgroundcolor = bp_.backgroundcolor;
	is_backgroundcolor = bp_.isbackgroundcolor;

	colorModule->boxBackgroundPB->setStyleSheet(
		colorButtonStyleSheet(rgb2qcolor(bp_.boxbgcolor)));
	set_boxbgcolor = bp_.boxbgcolor;

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
	int nitem = findToken(tex_graphics, bp_.graphics_driver);
	if (nitem >= 0)
		latexModule->psdriverCO->setCurrentIndex(nitem);
	updateModuleInfo();

	map<string, string> const & packages = BufferParams::auto_packages();
	for (map<string, string>::const_iterator it = packages.begin();
	     it != packages.end(); ++it) {
		QTableWidgetItem * item = mathsModule->packagesTW->findItems(toqstr(it->first), Qt::MatchExactly)[0];
		if (!item)
			continue;
		int row = mathsModule->packagesTW->row(item);
		switch (bp_.use_package(it->first)) {
			case BufferParams::package_off: {
				QRadioButton * rb = (QRadioButton*)mathsModule->packagesTW->cellWidget(row, 3);
				rb->setChecked(true);
				break;
			}
			case BufferParams::package_on: {
				QRadioButton * rb = (QRadioButton*)mathsModule->packagesTW->cellWidget(row, 2);
				rb->setChecked(true);
				break;
			}
			case BufferParams::package_auto: {
				QRadioButton * rb = (QRadioButton*)mathsModule->packagesTW->cellWidget(row, 1);
				rb->setChecked(true);
				break;
			}
		}
	}

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
		doubleToWidget(textLayoutModule->lspacingLE,
			bp_.spacing().getValueAsString());
	}
	setLSpacing(nitem);

	if (bp_.paragraph_separation == BufferParams::ParagraphIndentSeparation) {
		textLayoutModule->indentRB->setChecked(true);
		string indentation = bp_.getIndentation().asLyXCommand();
		int indent = 0;
		if (indentation != "default") {
			lengthToWidgets(textLayoutModule->indentLE,
			textLayoutModule->indentLengthCO,
			indentation, default_unit);
			indent = 1;
		}
		textLayoutModule->indentCO->setCurrentIndex(indent);
		setIndent(indent);
	} else {
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
				length, default_unit);
			break;
			}
		default:
			skip = 0;
			break;
		}
		textLayoutModule->skipCO->setCurrentIndex(skip);
		setSkip(skip);
	}

	textLayoutModule->twoColumnCB->setChecked(
		bp_.columns == 2);
	textLayoutModule->justCB->setChecked(bp_.justification);

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

	// Master/Child
	if (!bufferview() || !buffer().hasChildren()) {
		masterChildModule->childrenTW->clear();
		includeonlys_.clear();
		docPS->showPanel("Child Documents", false);
		if (docPS->isCurrentPanel("Child Documents"))
			docPS->setCurrentPanel("Document Class");
	} else {
		docPS->showPanel("Child Documents", true);
		masterChildModule->setEnabled(true);
		includeonlys_ = bp_.getIncludedChildren();
		updateIncludeonlys();
	}
	masterChildModule->maintainAuxCB->setChecked(
		bp_.maintain_unincluded_children);

	// Float Settings
	floatModule->set(bp_.float_placement);

	// ListingsSettings
	// break listings_params to multiple lines
	string lstparams =
		InsetListingsParams(bp_.listings_params).separatedParams();
	listingsModule->listingsED->setPlainText(toqstr(lstparams));

	// Fonts
	// some languages only work with polyglossia/XeTeX
	Language const * lang = lyx::languages.getLanguage(
		fromqstr(langModule->languageCO->itemData(
			langModule->languageCO->currentIndex()).toString()));
	bool const need_fontspec =
		lang->babel().empty() && !lang->polyglossia().empty();
	bool const os_fonts_available =
		bp_.baseClass()->outputType() == lyx::LATEX
		&& LaTeXFeatures::isAvailable("fontspec");
	fontModule->osFontsCB->setEnabled(os_fonts_available && !need_fontspec);
	fontModule->osFontsCB->setChecked(
		(os_fonts_available && bp_.useNonTeXFonts) || need_fontspec);
	updateFontsize(documentClass().opt_fontsize(),
			bp_.fontsize);

	QString font = toqstr(bp_.fontsRoman());
	int rpos = fontModule->fontsRomanCO->findData(font);
	if (rpos == -1) {
		rpos = fontModule->fontsRomanCO->count();
		fontModule->fontsRomanCO->addItem(font + qt_(" (not installed)"), font);
	}
	fontModule->fontsRomanCO->setCurrentIndex(rpos);
	fontModule->font_roman = toqstr(bp_.fonts_roman[!bp_.useNonTeXFonts]);

	font = toqstr(bp_.fontsSans());
	int spos = fontModule->fontsSansCO->findData(font);
	if (spos == -1) {
		spos = fontModule->fontsSansCO->count();
		fontModule->fontsSansCO->addItem(font + qt_(" (not installed)"), font);
	}
	fontModule->fontsSansCO->setCurrentIndex(spos);
	fontModule->font_sans = toqstr(bp_.fonts_sans[!bp_.useNonTeXFonts]);

	font = toqstr(bp_.fontsTypewriter());
	int tpos = fontModule->fontsTypewriterCO->findData(font);
	if (tpos == -1) {
		tpos = fontModule->fontsTypewriterCO->count();
		fontModule->fontsTypewriterCO->addItem(font + qt_(" (not installed)"), font);
	}
	fontModule->fontsTypewriterCO->setCurrentIndex(tpos);
	fontModule->font_typewriter = toqstr(bp_.fonts_typewriter[!bp_.useNonTeXFonts]);

	font = toqstr(bp_.fontsMath());
	int mpos = fontModule->fontsMathCO->findData(font);
	if (mpos == -1) {
		mpos = fontModule->fontsMathCO->count();
		fontModule->fontsMathCO->addItem(font + qt_(" (not installed)"), font);
	}
	fontModule->fontsMathCO->setCurrentIndex(mpos);
	fontModule->font_math = toqstr(bp_.fonts_math[!bp_.useNonTeXFonts]);

	if (bp_.useNonTeXFonts && os_fonts_available) {
		fontModule->fontencLA->setEnabled(false);
		fontModule->fontencCO->setEnabled(false);
		fontModule->fontencLE->setEnabled(false);
	} else {
		fontModule->fontencLA->setEnabled(true);
		fontModule->fontencCO->setEnabled(true);
		fontModule->fontencLE->setEnabled(true);
		romanChanged(rpos);
		sansChanged(spos);
		ttChanged(tpos);
	}

	if (!bp_.fonts_cjk.empty())
		fontModule->cjkFontLE->setText(
			toqstr(bp_.fonts_cjk));
	else
		fontModule->cjkFontLE->setText(QString());

	fontModule->fontScCB->setChecked(bp_.fonts_expert_sc);
	fontModule->fontOsfCB->setChecked(bp_.fonts_old_figures);
	fontModule->scaleSansSB->setValue(bp_.fontsSansScale());
	fontModule->font_sf_scale = bp_.fonts_sans_scale[!bp_.useNonTeXFonts];
	fontModule->scaleTypewriterSB->setValue(bp_.fontsTypewriterScale());
	fontModule->font_tt_scale = bp_.fonts_typewriter_scale[!bp_.useNonTeXFonts];

	int nn = findToken(GuiDocument::fontfamilies, bp_.fonts_default_family);
	if (nn >= 0)
		fontModule->fontsDefaultCO->setCurrentIndex(nn);

	if (bp_.fontenc == "global" || bp_.fontenc == "default") {
		fontModule->fontencCO->setCurrentIndex(
			fontModule->fontencCO->findData(toqstr(bp_.fontenc)));
		fontModule->fontencLE->setEnabled(false);
	} else {
		fontModule->fontencCO->setCurrentIndex(1);
		fontModule->fontencLE->setText(toqstr(bp_.fontenc));
	}

	// Output
	// This must be set _after_ fonts since updateDefaultFormat()
	// checks osFontsCB settings.
	// update combobox with formats
	updateDefaultFormat();
	int index = outputModule->defaultFormatCO->findData(toqstr(
		bp_.default_output_format));
	// set to default if format is not found
	if (index == -1)
		index = 0;
	outputModule->defaultFormatCO->setCurrentIndex(index);

	outputModule->outputsyncCB->setChecked(bp_.output_sync);
	outputModule->synccustomCB->setEditText(toqstr(bp_.output_sync_macro));

	outputModule->mathimgSB->setValue(bp_.html_math_img_scale);
	outputModule->mathoutCB->setCurrentIndex(bp_.html_math_output);
	outputModule->strictCB->setChecked(bp_.html_be_strict);
	outputModule->cssCB->setChecked(bp_.html_css_as_file);

	// paper
	bool const extern_geometry =
		documentClass().provides("geometry");
	int const psize = bp_.papersize;
	pageLayoutModule->papersizeCO->setCurrentIndex(psize);
	setCustomPapersize(!extern_geometry && psize == 1);
	pageLayoutModule->papersizeCO->setEnabled(!extern_geometry);

	bool const landscape =
		bp_.orientation == ORIENTATION_LANDSCAPE;
	pageLayoutModule->landscapeRB->setChecked(landscape);
	pageLayoutModule->portraitRB->setChecked(!landscape);
	pageLayoutModule->landscapeRB->setEnabled(!extern_geometry);
	pageLayoutModule->portraitRB->setEnabled(!extern_geometry);

	pageLayoutModule->facingPagesCB->setChecked(
		bp_.sides == TwoSides);

	lengthToWidgets(pageLayoutModule->paperwidthLE,
		pageLayoutModule->paperwidthUnitCO, bp_.paperwidth, default_unit);
	lengthToWidgets(pageLayoutModule->paperheightLE,
		pageLayoutModule->paperheightUnitCO, bp_.paperheight, default_unit);

	// margins
	Ui::MarginsUi * m = marginsModule;

	setMargins();

	lengthToWidgets(m->topLE, m->topUnit,
		bp_.topmargin, default_unit);

	lengthToWidgets(m->bottomLE, m->bottomUnit,
		bp_.bottommargin, default_unit);

	lengthToWidgets(m->innerLE, m->innerUnit,
		bp_.leftmargin, default_unit);

	lengthToWidgets(m->outerLE, m->outerUnit,
		bp_.rightmargin, default_unit);

	lengthToWidgets(m->headheightLE, m->headheightUnit,
		bp_.headheight, default_unit);

	lengthToWidgets(m->headsepLE, m->headsepUnit,
		bp_.headsep, default_unit);

	lengthToWidgets(m->footskipLE, m->footskipUnit,
		bp_.footskip, default_unit);

	lengthToWidgets(m->columnsepLE, m->columnsepUnit,
		bp_.columnsep, default_unit);

	// branches
	updateUnknownBranches();
	branchesModule->update(bp_);

	// PDF support
	PDFOptions const & pdf = bp_.pdfoptions();
	pdfSupportModule->use_hyperrefGB->setChecked(pdf.use_hyperref);
	if (bp_.documentClass().provides("hyperref"))
		pdfSupportModule->use_hyperrefGB->setTitle(qt_("C&ustomize Hyperref Options"));
	else
		pdfSupportModule->use_hyperrefGB->setTitle(qt_("&Use Hyperref Support"));
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

	// clear changed branches cache
	changedBranches_.clear();

	// reset tracker
	nonModuleChanged_ = false;
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


void GuiDocument::updateIncludeonlys()
{
	masterChildModule->childrenTW->clear();
	QString const no = qt_("No");
	QString const yes = qt_("Yes");

	if (includeonlys_.empty()) {
		masterChildModule->includeallRB->setChecked(true);
		masterChildModule->childrenTW->setEnabled(false);
		masterChildModule->maintainAuxCB->setEnabled(false);
	} else {
		masterChildModule->includeonlyRB->setChecked(true);
		masterChildModule->childrenTW->setEnabled(true);
		masterChildModule->maintainAuxCB->setEnabled(true);
	}
	ListOfBuffers children = buffer().getChildren();
	ListOfBuffers::const_iterator it  = children.begin();
	ListOfBuffers::const_iterator end = children.end();
	bool has_unincluded = false;
	bool all_unincluded = true;
	for (; it != end; ++it) {
		QTreeWidgetItem * item = new QTreeWidgetItem(masterChildModule->childrenTW);
		// FIXME Unicode
		string const name =
			to_utf8(makeRelPath(from_utf8((*it)->fileName().absFileName()),
							from_utf8(buffer().filePath())));
		item->setText(0, toqstr(name));
		item->setText(1, isChildIncluded(name) ? yes : no);
		if (!isChildIncluded(name))
			has_unincluded = true;
		else
			all_unincluded = false;
	}
	// Both if all childs are included and if none is included
	// is equal to "include all" (i.e., ommit \includeonly).
	// Thus, reset the GUI.
	if (!has_unincluded || all_unincluded) {
		masterChildModule->includeallRB->setChecked(true);
		masterChildModule->childrenTW->setEnabled(false);
		includeonlys_.clear();
	}
	// If all are included, we need to update again.
	if (!has_unincluded)
		updateIncludeonlys();
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
	string const classname = fromqstr(latexModule->classCO->getData(idx));
	if (!bp_.setBaseClass(classname)) {
		Alert::error(_("Error"), _("Unable to set document class."));
		return;
	}
	bp_.useClassDefaults();
	paramsToDialog();
}


void GuiDocument::setLayoutComboByIDString(string const & idString)
{
	if (!latexModule->classCO->set(toqstr(idString)))
		Alert::warning(_("Can't set layout!"),
			bformat(_("Unable to set layout for ID: %1$s"), from_utf8(idString)));
}


bool GuiDocument::isValid()
{
	return
		validateListingsParameters().isEmpty() &&
		localLayout->isValid() &&
		(
			// if we're asking for skips between paragraphs
			!textLayoutModule->skipRB->isChecked() ||
			// then either we haven't chosen custom
			textLayoutModule->skipCO->currentIndex() != 3 ||
			// or else a length has been given
			!textLayoutModule->skipLE->text().isEmpty()
		) &&
		(
			// if we're asking for indentation
			!textLayoutModule->indentRB->isChecked() ||
			// then either we haven't chosen custom
			textLayoutModule->indentCO->currentIndex() != 1 ||
			// or else a length has been given
			!textLayoutModule->indentLE->text().isEmpty()
		);
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
		LyXModule const * const mod = theModuleList[*it];
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
	BufferParams const & bp, FuncCode lfun, Buffer const * buf)
{
	ostringstream ss;
	ss << "\\begin_header\n";
	bp.writeFile(ss, buf);
	ss << "\\end_header\n";
	dialog.dispatch(FuncRequest(lfun, ss.str()));
}


void GuiDocument::dispatchParams()
{
	// We need a non-const buffer object.
	Buffer & buf = const_cast<BufferView *>(bufferview())->buffer();
	// There may be several undo records; group them (bug #8998)
	buf.undo().beginUndoGroup();

	// This must come first so that a language change is correctly noticed
	setLanguage();

	// Apply the BufferParams. Note that this will set the base class
	// and then update the buffer's layout.
	dispatch_bufferparams(*this, params(), LFUN_BUFFER_PARAMS_APPLY, &buffer());

	if (!params().master.empty()) {
		FileName const master_file = support::makeAbsPath(params().master,
			   support::onlyPath(buffer().absFileName()));
		if (isLyXFileName(master_file.absFileName())) {
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
		dispatch(FuncRequest(LFUN_INSET_FORALL,
			"Branch inset-toggle assign"));
	}
	// rename branches in the document
	executeBranchRenaming();
	// and clear changed branches cache
	changedBranches_.clear();

	// Generate the colours requested by indices.
	IndicesList & indiceslist = params().indiceslist();
	if (!indiceslist.empty()) {
		IndicesList::const_iterator it = indiceslist.begin();
		IndicesList::const_iterator const end = indiceslist.end();
		for (; it != end; ++it) {
			docstring const & current_index = it->shortcut();
			Index const * index = indiceslist.findShortcut(current_index);
			string const x11hexname = X11hexname(index->color());
			// display the new color
			docstring const str = current_index + ' ' + from_ascii(x11hexname);
			dispatch(FuncRequest(LFUN_SET_COLOR, str));
		}
	}
	// FIXME LFUN
	// If we used an LFUN, we would not need these two lines:
	BufferView * bv = const_cast<BufferView *>(bufferview());
	bv->processUpdateFlags(Update::Force | Update::FitCursor);

	// Don't forget to close the group. Note that it is important
	// to check that there is no early return in the method.
	buf.undo().endUndoGroup();
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
	dispatch_bufferparams(*this, params(), LFUN_BUFFER_SAVE_AS_DEFAULT, &buffer());
}


bool GuiDocument::providesOSF(QString const & font) const
{
	if (fontModule->osFontsCB->isChecked())
		// FIXME: we should check if the fonts really
		// have OSF support. But how?
		return true;
	return theLaTeXFonts().getLaTeXFont(
				qstring_to_ucs4(font)).providesOSF(ot1(),
								   completeFontset(),
								   noMathFont());
}


bool GuiDocument::providesSC(QString const & font) const
{
	if (fontModule->osFontsCB->isChecked())
		return false;
	return theLaTeXFonts().getLaTeXFont(
				qstring_to_ucs4(font)).providesSC(ot1(),
								  completeFontset(),
								  noMathFont());
}


bool GuiDocument::providesScale(QString const & font) const
{
	if (fontModule->osFontsCB->isChecked())
		return true;
	return theLaTeXFonts().getLaTeXFont(
				qstring_to_ucs4(font)).providesScale(ot1(),
								     completeFontset(),
								     noMathFont());
}


bool GuiDocument::providesNoMath(QString const & font) const
{
	if (fontModule->osFontsCB->isChecked())
		return false;
	return theLaTeXFonts().getLaTeXFont(
				qstring_to_ucs4(font)).providesNoMath(ot1(),
								      completeFontset());
}


bool GuiDocument::hasMonolithicExpertSet(QString const & font) const
{
	if (fontModule->osFontsCB->isChecked())
		return false;
	return theLaTeXFonts().getLaTeXFont(
				qstring_to_ucs4(font)).hasMonolithicExpertSet(ot1(),
									      completeFontset(),
									      noMathFont());
}


void GuiDocument::loadModuleInfo()
{
	moduleNames_.clear();
	LyXModuleList::const_iterator it  = theModuleList.begin();
	LyXModuleList::const_iterator end = theModuleList.end();
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
		if (it->category().substr(0, 8) != "Citation")
			moduleNames_.push_back(m);
	}
}


void GuiDocument::updateUnknownBranches()
{
	if (!bufferview())
		return;
	list<docstring> used_branches;
	buffer().getUsedBranches(used_branches);
	list<docstring>::const_iterator it = used_branches.begin();
	QStringList unknown_branches;
	for (; it != used_branches.end() ; ++it) {
		if (!buffer().params().branchlist().find(*it))
			unknown_branches.append(toqstr(*it));
	}
	branchesModule->setUnknownBranches(unknown_branches);
}


void GuiDocument::branchesRename(docstring const & oldname, docstring const & newname)
{
	map<docstring, docstring>::iterator it = changedBranches_.begin();
	for (; it != changedBranches_.end() ; ++it) {
		if (it->second == oldname) {
			// branch has already been renamed
			it->second = newname;
			return;
		}
	}
	// store new name
	changedBranches_[oldname] = newname;
}


void GuiDocument::executeBranchRenaming() const
{
	map<docstring, docstring>::const_iterator it = changedBranches_.begin();
	for (; it != changedBranches_.end() ; ++it) {
		docstring const arg = '"' + it->first + '"' + " " + '"' + it->second + '"';
		dispatch(FuncRequest(LFUN_BRANCHES_RENAME, arg));
	}
}


void GuiDocument::allPackagesAuto()
{
	allPackages(1);
}


void GuiDocument::allPackagesAlways()
{
	allPackages(2);
}


void GuiDocument::allPackagesNot()
{
	allPackages(3);
}


void GuiDocument::allPackages(int col)
{
	for (int row = 0; row < mathsModule->packagesTW->rowCount(); ++row) {
		QRadioButton * rb = (QRadioButton*)mathsModule->packagesTW->cellWidget(row, col);
		rb->setChecked(true);
	}
}


Dialog * createGuiDocument(GuiView & lv) { return new GuiDocument(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiDocument.cpp"
