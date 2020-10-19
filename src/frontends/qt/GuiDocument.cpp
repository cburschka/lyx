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

#include "BulletsModule.h"
#include "CategorizedCombo.h"
#include "FancyLineEdit.h"
#include "GuiApplication.h"
#include "GuiBranches.h"
#include "GuiIndices.h"
#include "GuiSelectionManager.h"
#include "LaTeXHighlighter.h"
#include "Validator.h"

#include "LayoutFile.h"
#include "BranchList.h"
#include "buffer_funcs.h"
#include "Buffer.h"
#include "BufferView.h"
#include "CiteEnginesList.h"
#include "Color.h"
#include "ColorCache.h"
#include "Converter.h"
#include "Cursor.h"
#include "Encoding.h"
#include "FloatPlacement.h"
#include "Format.h"
#include "FuncRequest.h"
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
#include "Session.h"
#include "Spacing.h"
#include "TextClass.h"
#include "Undo.h"
#include "VSpace.h"

#include "insets/InsetListingsParams.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/Package.h"
#include "support/TempFile.h"

#include "frontends/alert.h"

#include <QAbstractItemModel>
#include <QButtonGroup>
#include <QColor>
#include <QColorDialog>
#include <QCloseEvent>
#include <QDirIterator>
#include <QFontDatabase>
#include <QHeaderView>
#include <QPixmap>
#include <QScrollBar>
#include <QTextBoundaryFinder>
#include <QTextCursor>

#include <algorithm>
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


char const * lst_packages[] =
{
	"Listings", "Minted", ""
};


vector<string> engine_types_;
vector<pair<string, QString> > pagestyles;

QMap<QString, QString> rmfonts_;
QMap<QString, QString> sffonts_;
QMap<QString, QString> ttfonts_;
QMap<QString, QString> mathfonts_;

enum EncodingSets {
	unicode = 0,
	legacy = 1,
	custom = 2
};

lyx::RGBColor set_backgroundcolor;
bool is_backgroundcolor;
lyx::RGBColor set_fontcolor;
bool is_fontcolor;
lyx::RGBColor set_notefontcolor;
lyx::RGBColor set_boxbgcolor;
bool forced_fontspec_activation;

} // anonymous namespace

namespace lyx {

namespace {
// used when sorting the textclass list.
class less_textclass_avail_desc
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

} // namespace

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
	ModuleSelectionManager(QObject * parent,
						   QTreeView * availableLVarg,
						   QTreeView * selectedLVarg,
						   QPushButton * addPBarg,
						   QPushButton * delPBarg,
						   QPushButton * upPBarg,
						   QPushButton * downPBarg,
						   QStandardItemModel * availableModelarg,
						   GuiIdListModel * selectedModelarg,
	                       GuiDocument const * container)
		: GuiSelectionManager(parent, availableLVarg, selectedLVarg, addPBarg, delPBarg,
							  upPBarg, downPBarg, availableModelarg, selectedModelarg),
		  container_(container)
		{}
	///
	void updateProvidedModules(LayoutModuleList const & pm)
			{ provided_modules_ = pm.list(); }
	///
	void updateExcludedModules(LayoutModuleList const & em)
			{ excluded_modules_ = em.list(); }
private:
	///
	void updateAddPB() override;
	///
	void updateUpPB() override;
	///
	void updateDownPB() override;
	///
	void updateDelPB() override;
	/// returns availableModel as a GuiIdListModel
	QStandardItemModel * getAvailableModel()
	{
		return dynamic_cast<QStandardItemModel *>(availableModel);
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
			availableLV->selectionModel()->selectedRows(0);

	// disable if there aren't any modules (?), if none of them is chosen
	// in the dialog, or if the chosen one is already selected for use.
	if (arows == 0 || avail_sels.isEmpty() || isSelected(avail_sels.first())) {
		addPB->setEnabled(false);
		return;
	}

	QModelIndex const & idx = availableLV->selectionModel()->currentIndex();

	if (getAvailableModel()->itemFromIndex(idx)->hasChildren()) {
		// This is a category header
		addPB->setEnabled(false);
		return;
	}

	string const modname = fromqstr(getAvailableModel()->data(idx, Qt::UserRole).toString());

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

	// determine whether immediately succeeding element requires this one
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

PreambleModule::PreambleModule(QWidget * parent)
	: UiWidget<Ui::PreambleUi>(parent), current_id_(nullptr)
{
	// This is not a memory leak. The object will be destroyed
	// with this.
	// @ is letter in the LyX user preamble
	(void) new LaTeXHighlighter(preambleTE->document(), true);
	preambleTE->setFont(guiApp->typewriterSystemFont());
	preambleTE->setWordWrapMode(QTextOption::NoWrap);
	setFocusProxy(preambleTE);
	connect(preambleTE, SIGNAL(textChanged()), this, SIGNAL(changed()));
	connect(findLE, SIGNAL(textEdited(const QString &)), this, SLOT(checkFindButton()));
	connect(findButtonPB, SIGNAL(clicked()), this, SLOT(findText()));
	connect(editPB, SIGNAL(clicked()), this, SLOT(editExternal()));
	connect(findLE, SIGNAL(returnPressed()), this, SLOT(findText()));
	checkFindButton();
	// https://stackoverflow.com/questions/13027091/how-to-override-tab-width-in-qt
	const int tabStop = 4;
	QFontMetrics metrics(preambleTE->currentFont());
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
	// horizontalAdvance() is available starting in 5.11.0
	// setTabStopDistance() is available starting in 5.10.0
	preambleTE->setTabStopDistance(tabStop * metrics.horizontalAdvance(' '));
#else
	preambleTE->setTabStopWidth(tabStop * metrics.width(' '));
#endif
}


void PreambleModule::checkFindButton()
{
	findButtonPB->setEnabled(!findLE->text().isEmpty());
}


void PreambleModule::findText()
{
	bool const found = preambleTE->find(findLE->text());
	if (!found) {
		// wrap
		QTextCursor qtcur = preambleTE->textCursor();
		qtcur.movePosition(QTextCursor::Start);
		preambleTE->setTextCursor(qtcur);
		preambleTE->find(findLE->text());
	}
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
		cur = preambleTE->textCursor();
		cur.setPosition(it->second.first);
		preambleTE->setTextCursor(cur);
		preambleTE->verticalScrollBar()->setValue(it->second.second);
	}
}


void PreambleModule::apply(BufferParams & params)
{
	params.preamble = qstring_to_ucs4(preambleTE->document()->toPlainText());
}


void PreambleModule::closeEvent(QCloseEvent * e)
{
	// Save the coords before closing.
	QTextCursor cur = preambleTE->textCursor();
	preamble_coords_[current_id_] =
		make_pair(cur.position(), preambleTE->verticalScrollBar()->value());
	e->accept();
}


void PreambleModule::editExternal() {
	if (!current_id_)
		return;

	if (tempfile_) {
		preambleTE->setReadOnly(false);
		FileName const tempfilename = tempfile_->name();
		docstring const s = tempfilename.fileContents("UTF-8");
		preambleTE->document()->setPlainText(toqstr(s));
		tempfile_.reset();
		editPB->setText(qt_("&Edit"));
		changed();
		return;
	}

	string const format =
		current_id_->params().documentClass().outputFormat();
	string const ext = theFormats().extension(format);
	tempfile_.reset(new TempFile("preamble_editXXXXXX." + ext));
	FileName const tempfilename = tempfile_->name();
	string const name = tempfilename.toFilesystemEncoding();
	ofdocstream os(name.c_str());
	os << qstring_to_ucs4(preambleTE->document()->toPlainText());
	os.close();
	preambleTE->setReadOnly(true);
	theFormats().edit(*current_id_, tempfilename, format);
	editPB->setText(qt_("&End Edit"));
	changed();
}

/////////////////////////////////////////////////////////////////////
//
// LocalLayout
//
/////////////////////////////////////////////////////////////////////


LocalLayout::LocalLayout(QWidget * parent)
	: UiWidget<Ui::LocalLayoutUi>(parent), current_id_(nullptr), validated_(false)
{
	locallayoutTE->setFont(guiApp->typewriterSystemFont());
	locallayoutTE->setWordWrapMode(QTextOption::NoWrap);
	connect(locallayoutTE, SIGNAL(textChanged()), this, SLOT(textChanged()));
	connect(validatePB, SIGNAL(clicked()), this, SLOT(validatePressed()));
	connect(convertPB, SIGNAL(clicked()), this, SLOT(convertPressed()));
	connect(editPB, SIGNAL(clicked()), this, SLOT(editExternal()));
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
	docstring const layout =
		qstring_to_ucs4(locallayoutTE->document()->toPlainText());
	params.setLocalLayout(layout, false);
}


void LocalLayout::hideConvert()
{
	convertPB->setEnabled(false);
	convertLB->setText("");
	convertPB->hide();
	convertLB->hide();
}


void LocalLayout::textChanged()
{
	// Flashy red bold text
	static const QString ivpar("<p style=\"color: #c00000; font-weight: bold; text-align:left\">"
				   "%1</p>");
	static const QString message = ivpar.arg(qt_("Validation required!"));
	string const layout =
		fromqstr(locallayoutTE->document()->toPlainText().trimmed());

	if (layout.empty()) {
		validated_ = true;
		validatePB->setEnabled(false);
		validLB->setText("");
		hideConvert();
		changed();
	} else if (!validatePB->isEnabled()) {
		// if that's already enabled, we shouldn't need to do anything.
		validated_ = false;
		validLB->setText(message);
		validatePB->setEnabled(true);
		hideConvert();
		changed();
	}
}


void LocalLayout::convert() {
	string const layout =
		fromqstr(locallayoutTE->document()->toPlainText().trimmed());
	string const newlayout = TextClass::convert(layout);
	if (!newlayout.empty())
		locallayoutTE->setPlainText(toqstr(newlayout));
	validate();
}


void LocalLayout::convertPressed() {
	convert();
	hideConvert();
	changed();
}


void LocalLayout::validate() {
	// Bold text
	static const QString vpar("<p style=\"font-weight: bold; text-align:left\">%1</p>");
	// Flashy red bold text
	static const QString ivpar("<p style=\"color: #c00000; font-weight: bold; text-align:left\">"
	                           "%1</p>");
	string const layout =
		fromqstr(locallayoutTE->document()->toPlainText().trimmed());
	if (!layout.empty()) {
		TextClass::ReturnValues const ret = TextClass::validate(layout);
		validated_ = (ret == TextClass::OK) || (ret == TextClass::OK_OLDFORMAT);
		validatePB->setEnabled(false);
		validLB->setText(validated_ ? vpar.arg(qt_("Layout is valid!"))
		                            : ivpar.arg(qt_("Layout is invalid!")));
		if (ret == TextClass::OK_OLDFORMAT) {
			convertPB->show();
			// Testing conversion to LYXFILE_LAYOUT_FORMAT at this point
			// already.
			if (TextClass::convert(layout).empty()) {
				// Conversion failed. If LAYOUT_FORMAT > LYXFILE_LAYOUT_FORMAT,
				// then maybe the layout is still valid, but its format is more
				// recent than LYXFILE_LAYOUT_FORMAT. However, if LAYOUT_FORMAT
				// == LYXFILE_LAYOUT_FORMAT then something is definitely wrong.
				convertPB->setEnabled(false);
				const QString text = (LAYOUT_FORMAT == LYXFILE_LAYOUT_FORMAT)
					? ivpar.arg(qt_("Conversion to current format impossible!"))
					: vpar.arg(qt_("Conversion to current stable format "
					               "impossible."));
				convertLB->setText(text);
			} else {
				convertPB->setEnabled(true);
				convertLB->setText(qt_("Convert to current format"));
			}
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


void LocalLayout::editExternal() {
	if (!current_id_)
		return;

	if (tempfile_) {
		locallayoutTE->setReadOnly(false);
		FileName const tempfilename = tempfile_->name();
		docstring const s = tempfilename.fileContents("UTF-8");
		locallayoutTE->document()->setPlainText(toqstr(s));
		tempfile_.reset();
		editPB->setText(qt_("&Edit"));
		changed();
		return;
	}

	string const format =
		current_id_->params().documentClass().outputFormat();
	string const ext = theFormats().extension(format);
	tempfile_.reset(new TempFile("preamble_editXXXXXX." + ext));
	FileName const tempfilename = tempfile_->name();
	string const name = tempfilename.toFilesystemEncoding();
	ofdocstream os(name.c_str());
	os << qstring_to_ucs4(locallayoutTE->document()->toPlainText());
	os.close();
	locallayoutTE->setReadOnly(true);
	theFormats().edit(*current_id_, tempfilename, format);
	editPB->setText(qt_("&End Edit"));
	validatePB->setEnabled(false);
	hideConvert();
	changed();
}

/////////////////////////////////////////////////////////////////////
//
// DocumentDialog
//
/////////////////////////////////////////////////////////////////////


GuiDocument::GuiDocument(GuiView & lv)
	: GuiDialog(lv, "document", qt_("Document Settings")),
	  biblioChanged_(false), nonModuleChanged_(false),
	  modulesChanged_(false), shellescapeChanged_(false)
{
	setupUi(this);

	connect(buttonBox, SIGNAL(clicked(QAbstractButton *)),
		this, SLOT(slotButtonBox(QAbstractButton *)));

	connect(savePB, SIGNAL(clicked()), this, SLOT(saveDefaultClicked()));
	connect(defaultPB, SIGNAL(clicked()), this, SLOT(useDefaultsClicked()));

	// Manage the restore, ok, apply, restore and cancel/close buttons
	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setOK(buttonBox->button(QDialogButtonBox::Ok));
	bc().setApply(buttonBox->button(QDialogButtonBox::Apply));
	bc().setCancel(buttonBox->button(QDialogButtonBox::Cancel));
	bc().setRestore(buttonBox->button(QDialogButtonBox::Reset));


	// text layout
	textLayoutModule = new UiWidget<Ui::TextLayoutUi>(this);
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

	connect(textLayoutModule->tableStyleCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));

	textLayoutModule->lspacingLE->setValidator(new QDoubleValidator(
		textLayoutModule->lspacingLE));
	textLayoutModule->indentLE->setValidator(new LengthValidator(
		textLayoutModule->indentLE));
	textLayoutModule->skipLE->setValidator(new LengthValidator(
		textLayoutModule->skipLE));

	textLayoutModule->indentCO->addItem(qt_("Default"));
	textLayoutModule->indentCO->addItem(qt_("Custom"));
	textLayoutModule->skipCO->addItem(qt_("Half line height"), VSpace::HALFLINE);
	textLayoutModule->skipCO->addItem(qt_("Line height"), VSpace::FULLLINE);
	textLayoutModule->skipCO->addItem(qt_("SmallSkip"), VSpace::SMALLSKIP);
	textLayoutModule->skipCO->addItem(qt_("MedSkip"), VSpace::MEDSKIP);
	textLayoutModule->skipCO->addItem(qt_("BigSkip"), VSpace::BIGSKIP);
	textLayoutModule->skipCO->addItem(qt_("Custom"), VSpace::LENGTH);
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

	textLayoutModule->tableStyleCO->addItem(qt_("Default"), toqstr("default"));
	getTableStyles();


	// master/child handling
	masterChildModule = new UiWidget<Ui::MasterChildUi>(this);

	connect(masterChildModule->childrenTW, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
		this, SLOT(includeonlyClicked(QTreeWidgetItem *, int)));
	connect(masterChildModule->includeonlyRB, SIGNAL(toggled(bool)),
		masterChildModule->childrenTW, SLOT(setEnabled(bool)));
	connect(masterChildModule->includeonlyRB, SIGNAL(toggled(bool)),
		masterChildModule->maintainGB, SLOT(setEnabled(bool)));
	connect(masterChildModule->includeallRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(masterChildModule->includeonlyRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(masterChildModule->maintainCRNoneRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(masterChildModule->maintainCRMostlyRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(masterChildModule->maintainCRStrictRB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	masterChildModule->childrenTW->setColumnCount(2);
	masterChildModule->childrenTW->headerItem()->setText(0, qt_("Child Document"));
	masterChildModule->childrenTW->headerItem()->setText(1, qt_("Include to Output"));
#if (QT_VERSION > 0x050000)
        masterChildModule->childrenTW->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        masterChildModule->childrenTW->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
#else
        masterChildModule->childrenTW->header()->setResizeMode(0, QHeaderView::ResizeToContents);
        masterChildModule->childrenTW->header()->setResizeMode(1, QHeaderView::ResizeToContents);
#endif

	// Formats
	outputModule = new UiWidget<Ui::OutputUi>(this);

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
	connect(outputModule->tableoutCB, SIGNAL(currentIndexChanged(int)),
		this, SLOT(change_adaptor()));

	connect(outputModule->shellescapeCB, SIGNAL(stateChanged(int)),
		this, SLOT(shellescapeChanged()));
	connect(outputModule->outputsyncCB, SIGNAL(toggled(bool)),
		this, SLOT(setOutputSync(bool)));
	connect(outputModule->synccustomCB, SIGNAL(editTextChanged(QString)),
		this, SLOT(change_adaptor()));
	outputModule->synccustomCB->addItem("");
	outputModule->synccustomCB->addItem("\\synctex=1");
	outputModule->synccustomCB->addItem("\\synctex=-1");
	outputModule->synccustomCB->addItem("\\usepackage[active]{srcltx}");

	outputModule->synccustomCB->setValidator(new NoNewLineValidator(
		outputModule->synccustomCB));

	connect(outputModule->saveTransientPropertiesCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(outputModule->postponeFragileCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));


	// language & quote
	// this must precede font, since fonts depend on this
	langModule = new UiWidget<Ui::LanguageUi>(this);
	connect(langModule->languageCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(langModule->languageCO, SIGNAL(activated(int)),
		this, SLOT(languageChanged(int)));
	connect(langModule->encodingCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(langModule->encodingCO, SIGNAL(activated(int)),
		this, SLOT(encodingSwitched(int)));
	connect(langModule->unicodeEncodingCO, SIGNAL(activated(int)),
 		this, SLOT(change_adaptor()));
	connect(langModule->autoEncodingCO, SIGNAL(activated(int)),
 		this, SLOT(change_adaptor()));
	connect(langModule->customEncodingCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(langModule->quoteStyleCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(langModule->languagePackageCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(langModule->languagePackageLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(langModule->languagePackageCO, SIGNAL(currentIndexChanged(int)),
		this, SLOT(languagePackageChanged(int)));
	connect(langModule->dynamicQuotesCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));

	langModule->languagePackageLE->setValidator(new NoNewLineValidator(
		langModule->languagePackageLE));

	QAbstractItemModel * language_model = guiApp->languageModel();
	// FIXME: it would be nice if sorting was enabled/disabled via a checkbox.
	language_model->sort(0);
	langModule->languageCO->setModel(language_model);
	langModule->languageCO->setModelColumn(0);

	langModule->encodingCO->addItem(qt_("Unicode (utf8)"));
	langModule->encodingCO->addItem(qt_("Traditional (auto-selected)"));
	langModule->encodingCO->addItem(qt_("Custom"));
	langModule->encodingCO->setItemData(EncodingSets::unicode,
		qt_("Select Unicode (utf8) encoding."), Qt::ToolTipRole);
	langModule->encodingCO->setItemData(EncodingSets::legacy,
		qt_("Use language-dependent traditional encodings."), Qt::ToolTipRole);
	langModule->encodingCO->setItemData(EncodingSets::custom,
		qt_("Select a custom, document-wide encoding."), Qt::ToolTipRole);

	// basic Unicode encodings: keep order
	const QStringList utf8_base_encodings = {"utf8", "utf8-plain", "utf8x"};
	for (auto const & i : utf8_base_encodings) {
		langModule->unicodeEncodingCO->addItem(
					qt_(encodings.fromLyXName(fromqstr(i))->guiName()), i);
	}
	langModule->unicodeEncodingCO->setItemData(0,
		qt_("Standard Unicode support by the ``inputenc'' package."),
		Qt::ToolTipRole);
	langModule->unicodeEncodingCO->setItemData(1,
		qt_("Use UTF-8 'as-is': do not load any supporting packages, "
			"do not convert any characters to LaTeX macros. "
			"For use with non-TeX fonts (XeTeX/LuaTeX) or custom preamble code."),
		Qt::ToolTipRole);
	langModule->unicodeEncodingCO->setItemData(2,
		qt_("Load ``inputenc'' with option 'utf8x' "
			"for extended Unicode support by the ``ucs'' package."),
		Qt::ToolTipRole);
	langModule->autoEncodingCO->addItem(qt_("Language Default"), toqstr("auto-legacy"));
	langModule->autoEncodingCO->addItem(qt_("Language Default (no inputenc)"), toqstr("auto-legacy-plain"));
	langModule->autoEncodingCO->setItemData(0,
		qt_("Use the traditional default encoding of the text language. Switch encoding "
			"if a text part is set to a language with different default."),
		Qt::ToolTipRole);
	langModule->autoEncodingCO->setItemData(1,
		qt_("Do not load the 'inputenc' package. Switch encoding if required "
			"but do not write input encoding switch commands to the source."),
		Qt::ToolTipRole);
	// sort encodings
	QMap<QString,QString> encodingmap;
	QMap<QString,QString> encodingmap_utf8;
	for (auto const & encvar : encodings) {
		if (encvar.unsafe() ||encvar.guiName().empty()
		    || utf8_base_encodings.contains(toqstr(encvar.name())))
			continue;
		if (std::string(encvar.name()).find("utf8") == 0)
			encodingmap_utf8.insert(qt_(encvar.guiName()), toqstr(encvar.name()));
		else
			encodingmap.insert(qt_(encvar.guiName()), toqstr(encvar.name()));
	}
	for (auto const & i : encodingmap_utf8.keys()) {
		langModule->unicodeEncodingCO->addItem(i, encodingmap_utf8.value(i));
	}
	for (auto const & i : encodingmap.keys()) {
		langModule->customEncodingCO->addItem(i, encodingmap.value(i));
	}
	// equalise the width of encoding selectors
	langModule->autoEncodingCO->setMinimumSize(
		langModule->unicodeEncodingCO->minimumSizeHint());
	langModule->customEncodingCO->setMinimumSize(
		langModule->unicodeEncodingCO->minimumSizeHint());

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


	// fonts
	fontModule = new FontModule(this);
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
	connect(fontModule->microtypeCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(fontModule->dashesCB, SIGNAL(clicked()),
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
	connect(fontModule->fontSansOsfCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(fontModule->fontTypewriterOsfCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(fontModule->fontspecRomanLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(fontModule->fontspecSansLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(fontModule->fontspecTypewriterLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));

	fontModule->fontencLE->setValidator(new NoNewLineValidator(
		fontModule->fontencLE));
	fontModule->cjkFontLE->setValidator(new NoNewLineValidator(
		fontModule->cjkFontLE));
	fontModule->fontspecRomanLE->setValidator(new NoNewLineValidator(
		fontModule->fontspecRomanLE));
	fontModule->fontspecSansLE->setValidator(new NoNewLineValidator(
		fontModule->fontspecSansLE));
	fontModule->fontspecTypewriterLE->setValidator(new NoNewLineValidator(
		fontModule->fontspecTypewriterLE));

	updateFontlist();

	fontModule->fontsizeCO->addItem(qt_("Default"));
	fontModule->fontsizeCO->addItem(qt_("10"));
	fontModule->fontsizeCO->addItem(qt_("11"));
	fontModule->fontsizeCO->addItem(qt_("12"));

	fontModule->fontencCO->addItem(qt_("Automatic[[encoding]]"), QString("auto"));
	fontModule->fontencCO->addItem(qt_("Class Default"), QString("default"));
	fontModule->fontencCO->addItem(qt_("Custom"), QString("custom"));

	for (int n = 0; GuiDocument::fontfamilies_gui[n][0]; ++n)
		fontModule->fontsDefaultCO->addItem(
			qt_(GuiDocument::fontfamilies_gui[n]));

	if (!LaTeXFeatures::isAvailable("fontspec"))
		fontModule->osFontsCB->setToolTip(
			qt_("Use OpenType and TrueType fonts directly (requires XeTeX or LuaTeX)\n"
			    "You need to install the package \"fontspec\" to use this feature"));


	// page layout
	pageLayoutModule = new UiWidget<Ui::PageLayoutUi>(this);
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
	marginsModule = new UiWidget<Ui::MarginsUi>(this);
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
	marginsModule->topLE->setValidator(new LengthValidator(
		marginsModule->topLE));
	marginsModule->bottomLE->setValidator(new LengthValidator(
		marginsModule->bottomLE));
	marginsModule->innerLE->setValidator(new LengthValidator(
		marginsModule->innerLE));
	marginsModule->outerLE->setValidator(new LengthValidator(
		marginsModule->outerLE));
	marginsModule->headsepLE->setValidator(new LengthValidator(
		marginsModule->headsepLE));
	marginsModule->headheightLE->setValidator(new LengthValidator(
		marginsModule->headheightLE));
	marginsModule->footskipLE->setValidator(new LengthValidator(
		marginsModule->footskipLE));
	marginsModule->columnsepLE->setValidator(new LengthValidator(
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


	// color
	colorModule = new UiWidget<Ui::ColorUi>(this);
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

	// change tracking
	changesModule = new UiWidget<Ui::ChangeTrackingUi>(this);
	connect(changesModule->trackChangesCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(changesModule->outputChangesCB, SIGNAL(toggled(bool)),
		this, SLOT(outputChangesToggled(bool)));
	connect(changesModule->changeBarsCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));

	// numbering
	numberingModule = new UiWidget<Ui::NumberingUi>(this);
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
	connect(numberingModule->linenoCB, SIGNAL(toggled(bool)),
		this, SLOT(linenoToggled(bool)));
	connect(numberingModule->linenoCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(numberingModule->linenoLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));


	// biblio
	biblioModule = new UiWidget<Ui::BiblioUi>(this);
	connect(biblioModule->citeEngineCO, SIGNAL(activated(int)),
		this, SLOT(citeEngineChanged(int)));
	connect(biblioModule->citeStyleCO, SIGNAL(activated(int)),
		this, SLOT(citeStyleChanged()));
	connect(biblioModule->bibtopicCB, SIGNAL(clicked()),
		this, SLOT(biblioChanged()));
	connect(biblioModule->bibunitsCO, SIGNAL(activated(int)),
		this, SLOT(biblioChanged()));
	connect(biblioModule->bibtexCO, SIGNAL(activated(int)),
		this, SLOT(bibtexChanged(int)));
	connect(biblioModule->bibtexOptionsLE, SIGNAL(textChanged(QString)),
		this, SLOT(biblioChanged()));
	connect(biblioModule->citePackageOptionsLE, SIGNAL(textChanged(QString)),
		this, SLOT(biblioChanged()));
	connect(biblioModule->defaultBiblioCO, SIGNAL(activated(int)),
		this, SLOT(biblioChanged()));
	connect(biblioModule->defaultBiblioCO, SIGNAL(editTextChanged(QString)),
		this, SLOT(biblioChanged()));
	connect(biblioModule->defaultBiblioCO, SIGNAL(editTextChanged(QString)),
		this, SLOT(updateResetDefaultBiblio()));
	connect(biblioModule->biblatexBbxCO, SIGNAL(activated(int)),
		this, SLOT(biblioChanged()));
	connect(biblioModule->biblatexBbxCO, SIGNAL(editTextChanged(QString)),
		this, SLOT(biblioChanged()));
	connect(biblioModule->biblatexBbxCO, SIGNAL(editTextChanged(QString)),
		this, SLOT(updateResetDefaultBiblio()));
	connect(biblioModule->biblatexCbxCO, SIGNAL(activated(int)),
		this, SLOT(biblioChanged()));
	connect(biblioModule->biblatexCbxCO, SIGNAL(editTextChanged(QString)),
		this, SLOT(biblioChanged()));
	connect(biblioModule->biblatexCbxCO, SIGNAL(editTextChanged(QString)),
		this, SLOT(updateResetDefaultBiblio()));
	connect(biblioModule->rescanBibliosPB, SIGNAL(clicked()),
		this, SLOT(rescanBibFiles()));
	connect(biblioModule->resetDefaultBiblioPB, SIGNAL(clicked()),
		this, SLOT(resetDefaultBibfile()));
	connect(biblioModule->resetCbxPB, SIGNAL(clicked()),
		this, SLOT(resetDefaultCbxBibfile()));
	connect(biblioModule->resetBbxPB, SIGNAL(clicked()),
		this, SLOT(resetDefaultBbxBibfile()));
	connect(biblioModule->matchBbxPB, SIGNAL(clicked()),
		this, SLOT(matchBiblatexStyles()));

	biblioModule->citeEngineCO->clear();
	for (LyXCiteEngine const & cet : theCiteEnginesList) {
		biblioModule->citeEngineCO->addItem(qt_(cet.getName()), toqstr(cet.getID()));
		int const i = biblioModule->citeEngineCO->findData(toqstr(cet.getID()));
		biblioModule->citeEngineCO->setItemData(i, qt_(cet.getDescription()),
							Qt::ToolTipRole);
	}

	biblioModule->bibtexOptionsLE->setValidator(new NoNewLineValidator(
		biblioModule->bibtexOptionsLE));
	biblioModule->defaultBiblioCO->lineEdit()->setValidator(new NoNewLineValidator(
		biblioModule->defaultBiblioCO->lineEdit()));
	biblioModule->citePackageOptionsLE->setValidator(new NoNewLineValidator(
		biblioModule->citePackageOptionsLE));

	// NOTE: we do not provide "custom" here for security reasons!
	biblioModule->bibtexCO->clear();
	biblioModule->bibtexCO->addItem(qt_("Default"), QString("default"));
	for (auto const & alts : lyxrc.bibtex_alternatives) {
		QString const command = toqstr(alts).left(toqstr(alts).indexOf(" "));
		biblioModule->bibtexCO->addItem(command, command);
	}


	// indices
	indicesModule = new GuiIndices;
	connect(indicesModule, SIGNAL(changed()),
		this, SLOT(change_adaptor()));


	// maths
	mathsModule = new UiWidget<Ui::MathsUi>(this);
	QStringList headers;
	headers << qt_("Package") << qt_("Load automatically")
		<< qt_("Load always") << qt_("Do not load");
	mathsModule->packagesTW->setHorizontalHeaderLabels(headers);
	setSectionResizeMode(mathsModule->packagesTW->horizontalHeader(), QHeaderView::Stretch);
	map<string, string> const & packages = BufferParams::auto_packages();
	mathsModule->packagesTW->setRowCount(packages.size());
	int packnum = 0;
	for (auto const & pkgvar : packages) {
		docstring const package = from_ascii(pkgvar.first);
		QString autoTooltip = qt_(pkgvar.second);
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

		// Pack the buttons in a layout in order to get proper alignment
		QWidget * autoRBWidget = new QWidget();
		QHBoxLayout * autoRBLayout = new QHBoxLayout(autoRBWidget);
		autoRBLayout->addWidget(autoRB);
		autoRBLayout->setAlignment(Qt::AlignCenter);
		autoRBLayout->setContentsMargins(0, 0, 0, 0);
		autoRBWidget->setLayout(autoRBLayout);

		QWidget * alwaysRBWidget = new QWidget();
		QHBoxLayout * alwaysRBLayout = new QHBoxLayout(alwaysRBWidget);
		alwaysRBLayout->addWidget(alwaysRB);
		alwaysRBLayout->setAlignment(Qt::AlignCenter);
		alwaysRBLayout->setContentsMargins(0, 0, 0, 0);
		alwaysRBWidget->setLayout(alwaysRBLayout);

		QWidget * neverRBWidget = new QWidget();
		QHBoxLayout * neverRBLayout = new QHBoxLayout(neverRBWidget);
		neverRBLayout->addWidget(neverRB);
		neverRBLayout->setAlignment(Qt::AlignCenter);
		neverRBLayout->setContentsMargins(0, 0, 0, 0);
		neverRBWidget->setLayout(neverRBLayout);

		QTableWidgetItem * pack = new QTableWidgetItem(toqstr(package));
		mathsModule->packagesTW->setItem(packnum, 0, pack);
		mathsModule->packagesTW->setCellWidget(packnum, 1, autoRBWidget);
		mathsModule->packagesTW->setCellWidget(packnum, 2, alwaysRBWidget);
		mathsModule->packagesTW->setCellWidget(packnum, 3, neverRBWidget);

		connect(autoRB, SIGNAL(clicked()),
		        this, SLOT(change_adaptor()));
		connect(alwaysRB, SIGNAL(clicked()),
		        this, SLOT(change_adaptor()));
		connect(neverRB, SIGNAL(clicked()),
		        this, SLOT(change_adaptor()));
		++packnum;
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
	connect(mathsModule->MathNumberingPosCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));

	connect(mathsModule->MathIndentCB, SIGNAL(toggled(bool)),
		this, SLOT(change_adaptor()));
	connect(mathsModule->MathIndentCB, SIGNAL(toggled(bool)),
		this, SLOT(allowMathIndent()));
	connect(mathsModule->MathIndentCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(mathsModule->MathIndentCO, SIGNAL(activated(int)),
		this, SLOT(enableMathIndent(int)));
	connect(mathsModule->MathIndentLE, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(mathsModule->MathIndentLengthCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));


	mathsModule->MathIndentCO->addItem(qt_("Default"));
	mathsModule->MathIndentCO->addItem(qt_("Custom"));
	mathsModule->MathIndentLE->setValidator(new LengthValidator(
		mathsModule->MathIndentLE));
	// initialize the length validator
	bc().addCheckedLineEdit(mathsModule->MathIndentLE);
	mathsModule->MathNumberingPosCO->addItem(qt_("Left"));
	mathsModule->MathNumberingPosCO->addItem(qt_("Default"));
	mathsModule->MathNumberingPosCO->addItem(qt_("Right"));
	mathsModule->MathNumberingPosCO->setCurrentIndex(1);


	// latex class
	latexModule = new UiWidget<Ui::LaTeXUi>(this);
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

	for (auto const & cvar : classList) {
		LayoutFile const & tc = bcl[cvar];
		bool const available = tc.isTeXClassAvailable();
		docstring const guiname = translateIfPossible(from_utf8(tc.description()));
		// tooltip sensu "KOMA-Script Article [Class 'scrartcl']"
		QString tooltip = toqstr(bformat(_("%1$s [Class '%2$s']"), guiname, from_utf8(tc.latexname())));
		if (!available) {
			docstring const output_type = _("LaTeX");
			tooltip += '\n' + toqstr(bformat(_("Class not found by LyX. "
							   "Please check if you have the matching %1$s class "
							   "and all required packages (%2$s) installed."),
							 output_type, from_utf8(tc.prerequisites(", "))));
		}
		latexModule->classCO->addItemSort(toqstr(tc.name()),
						  toqstr(guiname),
						  toqstr(translateIfPossible(from_utf8(tc.category()))),
						  tooltip,
						  true, true, true, available);
	}


	// branches
	branchesModule = new GuiBranches(this);
	connect(branchesModule, SIGNAL(changed()),
		this, SLOT(change_adaptor()));
	connect(branchesModule, SIGNAL(renameBranches(docstring const &, docstring const &)),
		this, SLOT(branchesRename(docstring const &, docstring const &)));
	connect(branchesModule, SIGNAL(okPressed()), this, SLOT(slotOK()));
	updateUnknownBranches();


	// preamble
	preambleModule = new PreambleModule(this);
	connect(preambleModule, SIGNAL(changed()),
		this, SLOT(change_adaptor()));

	localLayout = new LocalLayout(this);
	connect(localLayout, SIGNAL(changed()),
		this, SLOT(change_adaptor()));


	// bullets
	bulletsModule = new BulletsModule(this);
	connect(bulletsModule, SIGNAL(changed()),
		this, SLOT(change_adaptor()));


	// Modules
	modulesModule = new UiWidget<Ui::ModulesUi>(this);
	modulesModule->availableLV->header()->setVisible(false);
	setSectionResizeMode(modulesModule->availableLV->header(), QHeaderView::ResizeToContents);
	modulesModule->availableLV->header()->setStretchLastSection(false);
	modulesModule->selectedLV->header()->setVisible(false);
	setSectionResizeMode(modulesModule->selectedLV->header(), QHeaderView::ResizeToContents);
	modulesModule->selectedLV->header()->setStretchLastSection(false);
	selectionManager =
		new ModuleSelectionManager(this, modulesModule->availableLV,
		                           modulesModule->selectedLV,
		                           modulesModule->addPB,
		                           modulesModule->deletePB,
		                           modulesModule->upPB,
		                           modulesModule->downPB,
					   availableModel(), selectedModel(), this);
	connect(selectionManager, SIGNAL(updateHook()),
		this, SLOT(updateModuleInfo()));
	connect(selectionManager, SIGNAL(selectionChanged()),
		this, SLOT(modulesChanged()));
	// The filter bar
	filter_ = new FancyLineEdit(this);
	filter_->setButtonPixmap(FancyLineEdit::Right, getPixmap("images/", "editclear", "svgz,png"));
	filter_->setButtonVisible(FancyLineEdit::Right, true);
	filter_->setButtonToolTip(FancyLineEdit::Right, qt_("Clear text"));
	filter_->setAutoHideButton(FancyLineEdit::Right, true);
	filter_->setPlaceholderText(qt_("All avail. modules"));
	modulesModule->moduleFilterBarL->addWidget(filter_, 0);
	modulesModule->findModulesLA->setBuddy(filter_);

	connect(filter_, SIGNAL(rightButtonClicked()),
		this, SLOT(resetModuleFilter()));
	connect(filter_, SIGNAL(textEdited(QString)),
		this, SLOT(moduleFilterChanged(QString)));
	connect(filter_, SIGNAL(returnPressed()),
		this, SLOT(moduleFilterPressed()));
#if (QT_VERSION < 0x050000)
	connect(filter_, SIGNAL(downPressed()),
		modulesModule->availableLV, SLOT(setFocus()));
#else
	connect(filter_, &FancyLineEdit::downPressed,
		modulesModule->availableLV, [=](){ focusAndHighlight(modulesModule->availableLV); });
#endif


	// PDF support
	pdfSupportModule = new UiWidget<Ui::PDFSupportUi>(this);
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
	connect(pdfSupportModule->bookmarksopenGB, SIGNAL(toggled(bool)),
		this, SLOT(bookmarksopenChanged(bool)));
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
	listingsModule = new UiWidget<Ui::ListingsSettingsUi>(this);
	connect(listingsModule->listingsED, SIGNAL(textChanged()),
		this, SLOT(change_adaptor()));
	connect(listingsModule->bypassCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(listingsModule->bypassCB, SIGNAL(clicked()),
		this, SLOT(setListingsMessage()));
	connect(listingsModule->packageCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(listingsModule->packageCO, SIGNAL(activated(int)),
		this, SLOT(listingsPackageChanged(int)));
	connect(listingsModule->listingsED, SIGNAL(textChanged()),
		this, SLOT(setListingsMessage()));
	listingsModule->listingsTB->setPlainText(
		qt_("Input listings parameters below. Enter ? for a list of parameters."));

	for (int i = 0; lst_packages[i][0]; ++i)
            listingsModule->packageCO->addItem(lst_packages[i]);


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
	docPS->addPanel(changesModule, N_("Change Tracking"));
	docPS->addPanel(numberingModule, N_("Numbering & TOC"));
	docPS->addPanel(biblioModule, N_("Bibliography"));
	docPS->addPanel(indicesModule, N_("Indexes"));
	docPS->addPanel(pdfSupportModule, N_("PDF Properties"));
	docPS->addPanel(mathsModule, N_("Math Options"));
	docPS->addPanel(floatModule, N_("Float Settings"));
	docPS->addPanel(listingsModule, N_("Listings[[inset]]"));
	docPS->addPanel(bulletsModule, N_("Bullets"));
	docPS->addPanel(branchesModule, N_("Branches"));
	docPS->addPanel(outputModule, N_("Formats[[output]]"));
	docPS->addPanel(preambleModule, N_("LaTeX Preamble"));
	docPS->setCurrentPanel("Document Class");
// FIXME: hack to work around resizing bug in Qt >= 4.2
// bug verified with Qt 4.2.{0-3} (JSpitzm)
#if QT_VERSION >= 0x040200
	docPS->updateGeometry();
#endif
}


void GuiDocument::onBufferViewChanged()
{
	if (isVisibleView())
		initialiseParams("");
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


void GuiDocument::shellescapeChanged()
{
	shellescapeChanged_ = true;
	changed();
}

void GuiDocument::bookmarksopenChanged(bool state)
{
	pdfSupportModule->bookmarksopenlevelSB->setEnabled(state);
	pdfSupportModule->bookmarksopenlevelLA->setEnabled(state);
}


void GuiDocument::slotApply()
{
	bool only_shellescape_changed = !nonModuleChanged_ && !modulesChanged_;
	bool wasclean = buffer().isClean();
	GuiDialog::slotApply();
	if (wasclean && only_shellescape_changed)
		buffer().markClean();
	modulesChanged_ = false;
}


void GuiDocument::slotOK()
{
	bool only_shellescape_changed = !nonModuleChanged_ && !modulesChanged_;
	bool wasclean = buffer().isClean();
	GuiDialog::slotOK();
	if (wasclean && only_shellescape_changed)
		buffer().markClean();
	modulesChanged_ = false;
}


void GuiDocument::slotButtonBox(QAbstractButton * button)
{
	switch (buttonBox->standardButton(button)) {
	case QDialogButtonBox::Ok:
		slotOK();
		break;
	case QDialogButtonBox::Apply:
		slotApply();
		break;
	case QDialogButtonBox::Cancel:
		slotClose();
		break;
	case QDialogButtonBox::Reset:
	case QDialogButtonBox::RestoreDefaults:
		slotRestore();
		break;
	default:
		break;
	}
}


void GuiDocument::filterModules(QString const & str)
{
	updateAvailableModules();
	if (str.isEmpty())
		return;

	modules_av_model_.clear();
	list<modInfoStruct> modInfoList = getModuleInfo();
	// Sort names according to the locale
	modInfoList.sort([](modInfoStruct const & a, modInfoStruct const & b) {
			return 0 < b.name.localeAwareCompare(a.name);
		});

	QIcon user_icon(getPixmap("images/", "lyxfiles-user", "svgz,png"));
	QIcon system_icon(getPixmap("images/", "lyxfiles-system", "svgz,png"));

	int i = 0;
	for (modInfoStruct const & m : modInfoList) {
		if (m.name.contains(str, Qt::CaseInsensitive) || contains(m.id, fromqstr(str))) {
			QStandardItem * item = new QStandardItem();
			item->setData(m.name, Qt::DisplayRole);
			item->setData(toqstr(m.id), Qt::UserRole);
			item->setData(m.description, Qt::ToolTipRole);
			if (m.local)
				item->setIcon(user_icon);
			else
				item->setIcon(system_icon);
			modules_av_model_.insertRow(i, item);
			++i;
		}
	}
}


void GuiDocument::moduleFilterChanged(const QString & text)
{
	if (!text.isEmpty()) {
		filterModules(filter_->text());
		return;
	}
	filterModules(filter_->text());
	filter_->setFocus();
}


void GuiDocument::moduleFilterPressed()
{
	filterModules(filter_->text());
}


void GuiDocument::resetModuleFilter()
{
	filter_->setText(QString());
	filterModules(filter_->text());
}


void GuiDocument::includeonlyClicked(QTreeWidgetItem * item, int)
{
	if (item == nullptr)
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
	if (listingsModule->bypassCB->isChecked())
		return QString();
	string const package =
	    lst_packages[listingsModule->packageCO->currentIndex()];
	string params = fromqstr(listingsModule->listingsED->toPlainText());
	InsetListingsParams lstparams(params);
	lstparams.setMinted(package == "Minted");
	return toqstr(lstparams.validate());
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
		// listingsModule->listingsTB->setTextColor("black");
		listingsModule->listingsTB->setPlainText(
			qt_("Input listings parameters below. "
		            "Enter ? for a list of parameters."));
	} else {
		isOK = false;
		// listingsModule->listingsTB->setTextColor("red");
		listingsModule->listingsTB->setPlainText(msg);
	}
}


void GuiDocument::listingsPackageChanged(int index)
{
        string const package = lst_packages[index];
        if (package == "Minted" && lyxrc.pygmentize_command.empty()) {
                Alert::warning(_("Pygments driver command not found!"),
                    _("The driver command necessary to use the minted package\n"
                      "(pygmentize) has not been found. Make sure you have\n"
                      "the python-pygments module installed or, if the driver\n"
                      "is named differently, to add the following line to the\n"
                      "document preamble:\n\n"
                      "\\AtBeginDocument{\\renewcommand{\\MintedPygmentize}{driver}}\n\n"
                      "where 'driver' is name of the driver command."));
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
	VSpace::VSpaceKind kind =
		VSpace::VSpaceKind(textLayoutModule->skipCO->itemData(item).toInt());
	bool const enable = (kind == VSpace::LENGTH);
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

void GuiDocument::allowMathIndent() {
	// only disable when not checked, checked does not always allow enabling
	if (!mathsModule->MathIndentCB->isChecked()) {
		mathsModule->MathIndentLE->setEnabled(false);
		mathsModule->MathIndentLengthCO->setEnabled(false);
	}
	if (mathsModule->MathIndentCB->isChecked()
	    && mathsModule->MathIndentCO->currentIndex() == 1) {
			mathsModule->MathIndentLE->setEnabled(true);
			mathsModule->MathIndentLengthCO->setEnabled(true);
	}
	isValid();
}

void GuiDocument::enableMathIndent(int item)
{
	bool const enable = (item == 1);
	mathsModule->MathIndentLE->setEnabled(enable);
	mathsModule->MathIndentLengthCO->setEnabled(enable);
	isValid();
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


void GuiDocument::updateQuoteStyles(bool const set)
{
	Language const * lang = lyx::languages.getLanguage(
		fromqstr(langModule->languageCO->itemData(
			langModule->languageCO->currentIndex()).toString()));

	InsetQuotesParams::QuoteStyle def = bp_.getQuoteStyle(lang->quoteStyle());

	langModule->quoteStyleCO->clear();

	bool has_default = false;
	for (int i = 0; i < quoteparams.stylescount(); ++i) {
		InsetQuotesParams::QuoteStyle qs = InsetQuotesParams::QuoteStyle(i);
		if (qs == InsetQuotesParams::DynamicQuotes)
			continue;
		bool const langdef = (qs == def);
		if (langdef) {
			// add the default style on top
			langModule->quoteStyleCO->insertItem(0,
				toqstr(quoteparams.getGuiLabel(qs, langdef)), qs);
			has_default = true;
		}
		else
			langModule->quoteStyleCO->addItem(
				toqstr(quoteparams.getGuiLabel(qs, langdef)), qs);
	}
	if (set && has_default)
		// (re)set to the default style
		langModule->quoteStyleCO->setCurrentIndex(0);
}


void GuiDocument::languageChanged(int i)
{
	// some languages only work with Polyglossia
	Language const * lang = lyx::languages.getLanguage(
		fromqstr(langModule->languageCO->itemData(i).toString()));
	if (lang->babel().empty() && !lang->polyglossia().empty()
		&& lang->required() != "CJK" && lang->required() != "japanese") {
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
	updateQuoteStyles(true);
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

	// non-tex fonts override the "\inputencoding" option with "utf8-plain"
	langModule->encodingCO->setEnabled(tex_fonts);
	inputencodingToDialog();

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


void GuiDocument::encodingSwitched(int i)
{
	bool const tex_fonts = !fontModule->osFontsCB->isChecked();
	langModule->unicodeEncodingCO->setEnabled(tex_fonts);
	langModule->customEncodingCO->setEnabled(tex_fonts);
	langModule->autoEncodingCO->setEnabled(tex_fonts);
	langModule->unicodeEncodingCO->setVisible(i == EncodingSets::unicode);
	langModule->autoEncodingCO->setVisible(i == EncodingSets::legacy);
	langModule->customEncodingCO->setVisible(i == EncodingSets::custom);
	if (tex_fonts)
		langModule->unicodeEncodingCO->setItemText(1, qt_("Direct (No inputenc)"));
	else
		langModule->unicodeEncodingCO->setItemText(1, qt_("Direct (XeTeX/LuaTeX)"));
}

void GuiDocument::inputencodingToDialog()
{
	QString inputenc = toqstr(bp_.inputenc);
	int p;
	if (fontModule->osFontsCB->isChecked()) { // non-tex fonts require utf8-plain
		langModule->encodingCO->setCurrentIndex(EncodingSets::unicode);
		langModule->unicodeEncodingCO->setCurrentIndex(
			langModule->unicodeEncodingCO->findData("utf8-plain"));
	} else if (inputenc.startsWith("utf8")) {
		langModule->encodingCO->setCurrentIndex(EncodingSets::unicode);
		p = langModule->unicodeEncodingCO->findData(inputenc);
		if (p == -1)
			p = 0;
		langModule->unicodeEncodingCO->setCurrentIndex(p);
		langModule->autoEncodingCO->setCurrentIndex(0);
		langModule->customEncodingCO->setCurrentIndex(0);
	} else if (inputenc.startsWith("auto")) {
		langModule->encodingCO->setCurrentIndex(EncodingSets::legacy);
		p = langModule->autoEncodingCO->findData(inputenc);
		if (p == -1)
			p = 0;
		langModule->unicodeEncodingCO->setCurrentIndex(0);
		langModule->autoEncodingCO->setCurrentIndex(p);
		langModule->customEncodingCO->setCurrentIndex(0);
	} else {
		langModule->encodingCO->setCurrentIndex(EncodingSets::custom);
		p = langModule->customEncodingCO->findData(inputenc);
		if (p == -1) {
			p = 0;
			langModule->encodingCO->setCurrentIndex(EncodingSets::unicode);
		}
		langModule->unicodeEncodingCO->setCurrentIndex(0);
		langModule->autoEncodingCO->setCurrentIndex(0);
		langModule->customEncodingCO->setCurrentIndex(p);
	}
	encodingSwitched(langModule->encodingCO->currentIndex());
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


void GuiDocument::updateExtraOpts()
{
	bool const tex_fonts = !fontModule->osFontsCB->isChecked();
	QString font;
	if (tex_fonts)
		font = fontModule->fontsRomanCO->itemData(
				fontModule->fontsRomanCO->currentIndex()).toString();
	bool const rm_opts = providesExtraOpts(font);
	if (tex_fonts)
		font = fontModule->fontsSansCO->itemData(
				fontModule->fontsSansCO->currentIndex()).toString();
	bool const sf_opts = providesExtraOpts(font);
	if (tex_fonts)
		font = fontModule->fontsTypewriterCO->itemData(
				fontModule->fontsTypewriterCO->currentIndex()).toString();
	bool const tt_opts = providesExtraOpts(font);
	fontModule->fontspecRomanLA->setEnabled(!tex_fonts || rm_opts);
	fontModule->fontspecRomanLE->setEnabled(!tex_fonts || rm_opts);
	fontModule->fontspecSansLA->setEnabled(!tex_fonts || sf_opts);
	fontModule->fontspecSansLE->setEnabled(!tex_fonts || sf_opts);
	fontModule->fontspecTypewriterLA->setEnabled(!tex_fonts || tt_opts);
	fontModule->fontspecTypewriterLE->setEnabled(!tex_fonts || tt_opts);
}


void GuiDocument::updateFontOptions()
{
	bool const tex_fonts = !fontModule->osFontsCB->isChecked();
	QString font;
	if (tex_fonts)
		font = fontModule->fontsSansCO->itemData(
				fontModule->fontsSansCO->currentIndex()).toString();
	bool scalable = providesScale(font);
	fontModule->scaleSansSB->setEnabled(scalable);
	fontModule->scaleSansLA->setEnabled(scalable);
	fontModule->fontSansOsfCB->setEnabled(providesOSF(font));
	if (tex_fonts)
		font = fontModule->fontsTypewriterCO->itemData(
				fontModule->fontsTypewriterCO->currentIndex()).toString();
	scalable = providesScale(font);
	fontModule->scaleTypewriterSB->setEnabled(scalable);
	fontModule->scaleTypewriterLA->setEnabled(scalable);
	fontModule->fontTypewriterOsfCB->setEnabled(providesOSF(font));
	if (tex_fonts)
		font = fontModule->fontsRomanCO->itemData(
				fontModule->fontsRomanCO->currentIndex()).toString();
	fontModule->fontScCB->setEnabled(providesSC(font));
	fontModule->fontOsfCB->setEnabled(providesOSF(font));
	updateExtraOpts();
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
	int const i = langModule->languageCO->currentIndex();
	if (i == -1)
		return false;
	QString const langname = langModule->languageCO->itemData(i).toString();
	Language const * newlang = lyx::languages.getLanguage(fromqstr(langname));
	return (fontenc == "default"
		|| (fontenc == "auto" && newlang->fontenc(buffer().params()) == "OT1")
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
	// reset the filters of the CategorizedCombos
	fontModule->fontsRomanCO->resetFilter();
	fontModule->fontsSansCO->resetFilter();
	fontModule->fontsTypewriterCO->resetFilter();
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
		for (auto const & family : families) {
			fontModule->fontsRomanCO->addItem(family, family);
			fontModule->fontsSansCO->addItem(family, family);
			fontModule->fontsTypewriterCO->addItem(family, family);
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
	updateExtraOpts();
	updateMathFonts(font);
}


void GuiDocument::sansChanged(int item)
{
	if (fontModule->osFontsCB->isChecked())
		return;
	QString const font =
		fontModule->fontsSansCO->itemData(item).toString();
	bool const scalable = providesScale(font);
	fontModule->scaleSansSB->setEnabled(scalable);
	fontModule->scaleSansLA->setEnabled(scalable);
	fontModule->fontSansOsfCB->setEnabled(providesOSF(font));
	updateExtraOpts();
}


void GuiDocument::ttChanged(int item)
{
	if (fontModule->osFontsCB->isChecked())
		return;
	QString const font =
		fontModule->fontsTypewriterCO->itemData(item).toString();
	bool scalable = providesScale(font);
	fontModule->scaleTypewriterSB->setEnabled(scalable);
	fontModule->scaleTypewriterLA->setEnabled(scalable);
	fontModule->fontTypewriterOsfCB->setEnabled(providesOSF(font));
	updateExtraOpts();
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

	for (auto const & pagestyle : pagestyles)
		if (pagestyle.first == sel)
			nn = pageLayoutModule->pagestyleCO->findText(pagestyle.second);

	if (nn > 0)
		pageLayoutModule->pagestyleCO->setCurrentIndex(nn);
}


void GuiDocument::browseLayout()
{
	QString const label1 = qt_("Lay&outs");
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
		qt_("D&ocuments"), toqstr(lyxrc.document_path));

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

	if (buttonBox->button(QDialogButtonBox::Apply)->isEnabled()) {
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
	if (!bp_.setBaseClass(classname, buffer().layoutPos())) {
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


void GuiDocument::checkPossibleCiteEngines()
{
	// Check if the class provides a specific engine,
	// and if so, enforce this.
	string force_engine;
	if (documentClass().provides("natbib")
	    || documentClass().provides("natbib-internal"))
		force_engine = "natbib";
	else if (documentClass().provides("jurabib"))
		force_engine = "jurabib";
	else if (documentClass().provides("biblatex"))
		force_engine = "biblatex";
	else if (documentClass().provides("biblatex-natbib"))
		force_engine = "biblatex-natbib";

	if (!force_engine.empty())
		biblioModule->citeEngineCO->setCurrentIndex(
			biblioModule->citeEngineCO->findData(toqstr(force_engine)));
	biblioModule->citeEngineCO->setEnabled(force_engine.empty());
}


void GuiDocument::rescanBibFiles()
{
	if (isBiblatex())
		rescanTexStyles("bbx cbx");
	else
		rescanTexStyles("bst");
}


void GuiDocument::resetDefaultBibfile(string const & which)
{
	QString const engine =
		biblioModule->citeEngineCO->itemData(
				biblioModule->citeEngineCO->currentIndex()).toString();

	CiteEngineType const cet =
		CiteEngineType(biblioModule->citeStyleCO->itemData(
							  biblioModule->citeStyleCO->currentIndex()).toInt());

	updateDefaultBiblio(theCiteEnginesList[fromqstr(engine)]->getDefaultBiblio(cet), which);
}


void GuiDocument::resetDefaultBbxBibfile()
{
	resetDefaultBibfile("bbx");
}


void GuiDocument::resetDefaultCbxBibfile()
{
	resetDefaultBibfile("cbx");
}


void GuiDocument::citeEngineChanged(int n)
{
	QString const engine =
		biblioModule->citeEngineCO->itemData(n).toString();

	vector<string> const engs =
		theCiteEnginesList[fromqstr(engine)]->getEngineType();

	updateCiteStyles(engs);
	updateEngineDependends();
	resetDefaultBibfile();
	biblioChanged();
}


void GuiDocument::updateEngineDependends()
{
	bool const biblatex = isBiblatex();

	// These are only useful with BibTeX
	biblioModule->defaultBiblioCO->setEnabled(!biblatex);
	biblioModule->bibtexStyleLA->setEnabled(!biblatex);
	biblioModule->resetDefaultBiblioPB->setEnabled(!biblatex);
	biblioModule->bibtopicCB->setEnabled(!biblatex);

	// These are only useful with Biblatex
	biblioModule->biblatexBbxCO->setEnabled(biblatex);
	biblioModule->biblatexBbxLA->setEnabled(biblatex);
	biblioModule->biblatexCbxCO->setEnabled(biblatex);
	biblioModule->biblatexCbxLA->setEnabled(biblatex);
	biblioModule->resetBbxPB->setEnabled(biblatex);
	biblioModule->resetCbxPB->setEnabled(biblatex);
	biblioModule->matchBbxPB->setEnabled(biblatex);

	// These are useful with biblatex, jurabib and natbib
	QString const engine =
		biblioModule->citeEngineCO->itemData(
				biblioModule->citeEngineCO->currentIndex()).toString();
	LyXCiteEngine const * ce = theCiteEnginesList[fromqstr(engine)];

	bool const citepack = ce->required("biblatex.sty") || ce->required("jurabib.sty")
			|| ce->required("natbib.sty");
	biblioModule->citePackageOptionsLE->setEnabled(citepack);
	biblioModule->citePackageOptionsL->setEnabled(citepack);
}


void GuiDocument::citeStyleChanged()
{
	QString const engine =
		biblioModule->citeEngineCO->itemData(
				biblioModule->citeEngineCO->currentIndex()).toString();
	QString const currentDef = isBiblatex() ?
		biblioModule->biblatexBbxCO->currentText()
		: biblioModule->defaultBiblioCO->currentText();
	if (theCiteEnginesList[fromqstr(engine)]->isDefaultBiblio(fromqstr(currentDef)))
		resetDefaultBibfile();

	biblioChanged();
}


void GuiDocument::bibtexChanged(int n)
{
	biblioModule->bibtexOptionsLE->setEnabled(
		biblioModule->bibtexCO->itemData(n).toString() != "default");
	biblioChanged();
}


void GuiDocument::updateCiteStyles(vector<string> const & engs, CiteEngineType const & sel)
{
	biblioModule->citeStyleCO->clear();

	vector<string>::const_iterator it  = engs.begin();
	vector<string>::const_iterator end = engs.end();
	for (; it != end; ++it) {
		if (*it == "default")
			biblioModule->citeStyleCO->addItem(qt_("Basic numerical"),
							   ENGINE_TYPE_DEFAULT);
		else if (*it == "authoryear")
			biblioModule->citeStyleCO->addItem(qt_("Author-year"),
							   ENGINE_TYPE_AUTHORYEAR);
		else if (*it == "numerical")
			biblioModule->citeStyleCO->addItem(qt_("Author-number"),
							   ENGINE_TYPE_NUMERICAL);
	}
	int i = biblioModule->citeStyleCO->findData(sel);
	if (biblioModule->citeStyleCO->findData(sel) == -1)
		i = 0;
	biblioModule->citeStyleCO->setCurrentIndex(i);

	biblioModule->citationStyleL->setEnabled(engs.size() > 1);
	biblioModule->citeStyleCO->setEnabled(engs.size() > 1);
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

	updateCiteStyles(engine_types_, sel);
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
	updateSelectedModules();

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

	if (buttonBox->button(QDialogButtonBox::Apply)->isEnabled()
	    && (nonModuleChanged_ || shellescapeChanged_)) {
		int const ret = Alert::prompt(_("Unapplied changes"),
				_("Some changes in the dialog were not yet applied.\n"
				"If you do not apply now, they will be lost after this action."),
				1, 1, _("&Apply"), _("&Dismiss"));
		if (ret == 0)
			applyView();
	}

	modulesChanged_ = true;
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
	bool category = false;
	if (focus_on_selected) {
		lv = modulesModule->selectedLV;
		category = true;
	} else
		lv = modulesModule->availableLV;
	if (lv->selectionModel()->selectedIndexes().isEmpty()) {
		modulesModule->infoML->document()->clear();
		return;
	}
	QModelIndex const & idx = lv->selectionModel()->currentIndex();

	if (!focus_on_selected
	    && modules_av_model_.itemFromIndex(idx)->hasChildren()) {
		// This is a category header
		modulesModule->infoML->document()->clear();
		return;
	}

	string const modName = focus_on_selected ?
				modules_sel_model_.getIDString(idx.row())
			      : fromqstr(modules_av_model_.data(idx, Qt::UserRole).toString());
	docstring desc = getModuleDescription(modName);

	LayoutModuleList const & provmods = bp_.baseClass()->providedModules();
	if (std::find(provmods.begin(), provmods.end(), modName) != provmods.end()) {
		if (!desc.empty())
			desc += "\n";
		desc += _("Module provided by document class.");
	}

	if (category) {
		docstring cat = getModuleCategory(modName);
		if (!cat.empty()) {
			if (!desc.empty())
				desc += "\n";
			desc += bformat(_("<p><b>Category:</b> %1$s.</p>"),
					translateIfPossible(cat));
		}
	}

	vector<string> pkglist = getPackageList(modName);
	docstring pkgdesc = formatStrVec(pkglist, _("and"));
	if (!pkgdesc.empty()) {
		if (!desc.empty())
			desc += "\n";
		desc += bformat(_("<p><b>Package(s) required:</b> %1$s.</p>"), pkgdesc);
	}

	pkglist = getRequiredList(modName);
	if (!pkglist.empty()) {
		vector<string> const reqdescs = idsToNames(pkglist);
		pkgdesc = formatStrVec(reqdescs, _("or"));
		if (!desc.empty())
			desc += "\n";
		desc += bformat(_("<p><b>Modules required:</b> %1$s.</p>"), pkgdesc);
	}

	pkglist = getExcludedList(modName);
	if (!pkglist.empty()) {
		vector<string> const reqdescs = idsToNames(pkglist);
		pkgdesc = formatStrVec(reqdescs, _( "and"));
		if (!desc.empty())
			desc += "\n";
		desc += bformat(_("<p><b>Modules excluded:</b> %1$s.</p>"), pkgdesc);
	}

	if (!desc.empty())
		desc += "\n";
	desc += bformat(_("<p><b>Filename:</b> <tt>%1$s.module</tt>.</p>"), from_utf8(modName));

	if (!isModuleAvailable(modName)) {
		if (!desc.empty())
			desc += "\n";
		desc += _("<p><font color=red><b>WARNING: Some required packages are unavailable!</b></font></p>");
	}

	modulesModule->infoML->document()->setHtml(toqstr(desc));
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
	QTreeWidgetItem * item = nullptr;

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


void GuiDocument::getTableStyles()
{
	// We look for lyx files in the subdirectory dir of
	//   1) user_lyxdir
	//   2) build_lyxdir (if not empty)
	//   3) system_lyxdir
	// in this order. Files with a given sub-hierarchy will
	// only be listed once.
	// We also consider i18n subdirectories and store them separately.
	QStringList dirs;

	// The three locations to look at.
	string const user = addPath(package().user_support().absFileName(), "tabletemplates");
	string const build = addPath(package().build_support().absFileName(), "tabletemplates");
	string const system = addPath(package().system_support().absFileName(), "tabletemplates");

	dirs << toqstr(user)
	     << toqstr(build)
	     << toqstr(system);

	for (int i = 0; i < dirs.size(); ++i) {
		QString const dir = dirs.at(i);
		QDirIterator it(dir, QDir::Files, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			QString fn = QFileInfo(it.next()).fileName();
			if (!fn.endsWith(".lyx") || fn.contains("_1x"))
				continue;
			QString data = fn.left(fn.lastIndexOf(".lyx"));
			QString guiname = data;
			guiname = toqstr(translateIfPossible(qstring_to_ucs4(guiname.replace('_', ' '))));
			QString relpath = toqstr(makeRelPath(qstring_to_ucs4(fn),
							     qstring_to_ucs4(dir)));
			if (textLayoutModule->tableStyleCO->findData(data) == -1)
				textLayoutModule->tableStyleCO->addItem(guiname, data);
		}
	}
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
		param_copy.setBaseClass(classname, buffer().layoutPos());
		param_copy.makeDocumentClass(true);
	}
	outputModule->defaultFormatCO->blockSignals(true);
	outputModule->defaultFormatCO->clear();
	outputModule->defaultFormatCO->addItem(qt_("Default"),
				QVariant(QString("default")));
	FormatList const & formats =
				param_copy.exportableFormats(true);
	for (Format const * f : formats)
		outputModule->defaultFormatCO->addItem
			(toqstr(translateIfPossible(f->prettyname())),
			 QVariant(toqstr(f->name())));
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
	string const engine =
		fromqstr(biblioModule->citeEngineCO->itemData(
				biblioModule->citeEngineCO->currentIndex()).toString());
	bp_.setCiteEngine(engine);

	CiteEngineType const style = CiteEngineType(biblioModule->citeStyleCO->itemData(
		biblioModule->citeStyleCO->currentIndex()).toInt());
	if (theCiteEnginesList[engine]->hasEngineType(style))
		bp_.setCiteEngineType(style);
	else
		bp_.setCiteEngineType(ENGINE_TYPE_DEFAULT);

	bp_.splitbib(biblioModule->bibtopicCB->isChecked());

	bp_.multibib = fromqstr(biblioModule->bibunitsCO->itemData(
				biblioModule->bibunitsCO->currentIndex()).toString());

	bp_.setDefaultBiblioStyle(fromqstr(biblioModule->defaultBiblioCO->currentText()));

	bp_.biblatex_bibstyle = fromqstr(biblioModule->biblatexBbxCO->currentText());
	bp_.biblatex_citestyle = fromqstr(biblioModule->biblatexCbxCO->currentText());
	bp_.biblio_opts = fromqstr(biblioModule->citePackageOptionsLE->text());

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
	switch (langModule->encodingCO->currentIndex()) {
		case EncodingSets::unicode: {
			if (!fontModule->osFontsCB->isChecked())
				bp_.inputenc = fromqstr(langModule->unicodeEncodingCO->itemData(
					langModule->unicodeEncodingCO->currentIndex()).toString());
			break;
		}
		case EncodingSets::legacy: {
			bp_.inputenc = "auto-legacy";
			bp_.inputenc = fromqstr(langModule->autoEncodingCO->itemData(
				langModule->autoEncodingCO->currentIndex()).toString());
			break;
		}
		case EncodingSets::custom: {
			bp_.inputenc = fromqstr(langModule->customEncodingCO->itemData(
				langModule->customEncodingCO->currentIndex()).toString());
			break;
		}
		default:
			// this should never happen
			bp_.inputenc = "utf8";
	}
	bp_.quotes_style = (InsetQuotesParams::QuoteStyle) langModule->quoteStyleCO->itemData(
		langModule->quoteStyleCO->currentIndex()).toInt();
	bp_.dynamic_quotes = langModule->dynamicQuotesCB->isChecked();

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
	bp_.use_lineno = numberingModule->linenoCB->isChecked();
	bp_.lineno_opts = fromqstr(numberingModule->linenoLE->text());

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
		bp_.setBaseClass(classname, buffer().layoutPos());
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

		QRadioButton * rb =
			(QRadioButton*)mathsModule->packagesTW->cellWidget(row, 1)->layout()->itemAt(0)->widget();
		if (rb->isChecked()) {
			bp_.use_package(it->first, BufferParams::package_auto);
			continue;
		}
		rb = (QRadioButton*)mathsModule->packagesTW->cellWidget(row, 2)->layout()->itemAt(0)->widget();
		if (rb->isChecked()) {
			bp_.use_package(it->first, BufferParams::package_on);
			continue;
		}
		rb = (QRadioButton*)mathsModule->packagesTW->cellWidget(row, 3)->layout()->itemAt(0)->widget();
		if (rb->isChecked())
			bp_.use_package(it->first, BufferParams::package_off);
	}
	// if math is indented
	bp_.is_math_indent = mathsModule->MathIndentCB->isChecked();
	if (bp_.is_math_indent) {
		// if formulas are indented
		switch (mathsModule->MathIndentCO->currentIndex()) {
		case 0:
			bp_.setMathIndent(Length());
			break;
		case 1: {
			Length mathindent(widgetsToLength(mathsModule->MathIndentLE,
			                                  mathsModule->MathIndentLengthCO));
			bp_.setMathIndent(mathindent);
			break;
		}
		default:
			// this should never happen
			bp_.setMathIndent(Length());
			break;
		}
	}
	switch (mathsModule->MathNumberingPosCO->currentIndex()) {
		case 0:
			bp_.math_numbering_side = BufferParams::LEFT;
			break;
		case 1:
			bp_.math_numbering_side = BufferParams::DEFAULT;
			break;
		case 2:
			bp_.math_numbering_side = BufferParams::RIGHT;
			break;
		default:
			// this should never happen
			bp_.math_numbering_side = BufferParams::DEFAULT;
			break;
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
			bp_.setParIndent(Length());
			break;
		case 1: {
			Length parindent(widgetsToLength(textLayoutModule->indentLE,
			                                 textLayoutModule->indentLengthCO));
			bp_.setParIndent(parindent);
			break;
		}
		default:
			// this should never happen
			bp_.setParIndent(Length());
			break;
		}
	} else {
		// if paragraphs are separated by a skip
		bp_.paragraph_separation = BufferParams::ParagraphSkipSeparation;
		VSpace::VSpaceKind spacekind =
			VSpace::VSpaceKind(textLayoutModule->skipCO->itemData(textLayoutModule->skipCO->currentIndex()).toInt());
		switch (spacekind) {
		case VSpace::SMALLSKIP:
		case VSpace::MEDSKIP:
		case VSpace::BIGSKIP:
		case VSpace::HALFLINE:
		case VSpace::FULLLINE:
			bp_.setDefSkip(VSpace(spacekind));
			break;
		case VSpace::LENGTH: {
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
	bp_.tablestyle = fromqstr(textLayoutModule->tableStyleCO->itemData(
				      textLayoutModule->tableStyleCO->currentIndex()).toString());

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
	if (masterChildModule->maintainCRNoneRB->isChecked())
		bp_.maintain_unincluded_children =
			BufferParams::CM_None;
	else if (masterChildModule->maintainCRMostlyRB->isChecked())
		bp_.maintain_unincluded_children =
			BufferParams::CM_Mostly;
	else
		bp_.maintain_unincluded_children =
			BufferParams::CM_Strict;
	updateIncludeonlyDisplay();

	// Float Settings
	bp_.float_placement = floatModule->getPlacement();
	bp_.float_alignment = floatModule->getAlignment();

	// Listings
	// text should have passed validation
	idx = listingsModule->packageCO->currentIndex();
	bp_.use_minted = string(lst_packages[idx]) == "Minted";
	bp_.listings_params =
		InsetListingsParams(fromqstr(listingsModule->listingsED->toPlainText())).params();

	// Formats
	bp_.default_output_format = fromqstr(outputModule->defaultFormatCO->itemData(
		outputModule->defaultFormatCO->currentIndex()).toString());

	bool const nontexfonts = fontModule->osFontsCB->isChecked();
	bp_.useNonTeXFonts = nontexfonts;

	bp_.shell_escape = outputModule->shellescapeCB->isChecked();
	if (!bp_.shell_escape)
	    theSession().shellescapeFiles().remove(buffer().absFileName());
	else if (!theSession().shellescapeFiles().find(buffer().absFileName()))
	    theSession().shellescapeFiles().insert(buffer().absFileName());
	Buffer & buf = const_cast<Buffer &>(buffer());
	buf.params().shell_escape = bp_.shell_escape;

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

	int tablefmt = outputModule->tableoutCB->currentIndex();
	if (tablefmt == -1)
		tablefmt = 0;
	BufferParams::TableOutput const to =
			static_cast<BufferParams::TableOutput>(tablefmt);
	bp_.docbook_table_output = to;

	bp_.save_transient_properties =
		outputModule->saveTransientPropertiesCB->isChecked();
	bp_.postpone_fragile_content =
		outputModule->postponeFragileCB->isChecked();

	// fonts
	bp_.fonts_roman[nontexfonts] =
		fromqstr(fontModule->fontsRomanCO->
			itemData(fontModule->fontsRomanCO->currentIndex()).toString());
	bp_.fonts_roman[!nontexfonts] = fromqstr(fontModule->font_roman);
	bp_.font_roman_opts = fromqstr(fontModule->fontspecRomanLE->text());

	bp_.fonts_sans[nontexfonts] =
		fromqstr(fontModule->fontsSansCO->
			itemData(fontModule->fontsSansCO->currentIndex()).toString());
	bp_.fonts_sans[!nontexfonts] = fromqstr(fontModule->font_sans);
	bp_.font_sans_opts = fromqstr(fontModule->fontspecSansLE->text());

	bp_.fonts_typewriter[nontexfonts] =
		fromqstr(fontModule->fontsTypewriterCO->
			itemData(fontModule->fontsTypewriterCO->currentIndex()).toString());
	bp_.fonts_typewriter[!nontexfonts] = fromqstr(fontModule->font_typewriter);
	bp_.font_typewriter_opts = fromqstr(fontModule->fontspecTypewriterLE->text());

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

	bp_.use_microtype = fontModule->microtypeCB->isChecked();
	bp_.use_dash_ligatures = !fontModule->dashesCB->isChecked();

	bp_.fonts_sans_scale[nontexfonts] = fontModule->scaleSansSB->value();
	bp_.fonts_sans_scale[!nontexfonts] = fontModule->font_sf_scale;

	bp_.fonts_typewriter_scale[nontexfonts] = fontModule->scaleTypewriterSB->value();
	bp_.fonts_typewriter_scale[!nontexfonts] = fontModule->font_tt_scale;

	bp_.fonts_expert_sc = fontModule->fontScCB->isChecked();

	bp_.fonts_roman_osf = fontModule->fontOsfCB->isChecked();
	bp_.fonts_sans_osf = fontModule->fontSansOsfCB->isChecked();
	bp_.fonts_typewriter_osf = fontModule->fontTypewriterOsfCB->isChecked();

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

	// change tracking
	bp_.track_changes = changesModule->trackChangesCB->isChecked();
	bp_.output_changes = changesModule->outputChangesCB->isChecked();
	bool const cb_switched_off = (bp_.change_bars
				      && !changesModule->changeBarsCB->isChecked());
	bp_.change_bars = changesModule->changeBarsCB->isChecked();
	if (cb_switched_off)
		// if change bars have been switched off,
		// we need to ditch the aux file
		buffer().requireFreshStart(true);

	// reset trackers
	nonModuleChanged_ = false;
	shellescapeChanged_ = false;
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
	string const cite_engine = bp_.citeEngine();

	biblioModule->citeEngineCO->setCurrentIndex(
		biblioModule->citeEngineCO->findData(toqstr(cite_engine)));

	updateEngineType(documentClass().opt_enginetype(),
		bp_.citeEngineType());

	checkPossibleCiteEngines();

	biblioModule->citeStyleCO->setCurrentIndex(
		biblioModule->citeStyleCO->findData(bp_.citeEngineType()));

	biblioModule->bibtopicCB->setChecked(bp_.splitbib());

	biblioModule->bibunitsCO->clear();
	biblioModule->bibunitsCO->addItem(qt_("No"), QString());
	if (documentClass().hasLaTeXLayout("part"))
		biblioModule->bibunitsCO->addItem(qt_("per part"), toqstr("part"));
	if (documentClass().hasLaTeXLayout("chapter"))
		biblioModule->bibunitsCO->addItem(qt_("per chapter"), toqstr("chapter"));
	if (documentClass().hasLaTeXLayout("section"))
		biblioModule->bibunitsCO->addItem(qt_("per section"), toqstr("section"));
	if (documentClass().hasLaTeXLayout("subsection"))
		biblioModule->bibunitsCO->addItem(qt_("per subsection"), toqstr("subsection"));
	biblioModule->bibunitsCO->addItem(qt_("per child document"), toqstr("child"));

	int const mbpos = biblioModule->bibunitsCO->findData(toqstr(bp_.multibib));
	if (mbpos != -1)
		biblioModule->bibunitsCO->setCurrentIndex(mbpos);
	else
		biblioModule->bibunitsCO->setCurrentIndex(0);

	updateEngineDependends();

	if (isBiblatex()) {
		updateDefaultBiblio(bp_.biblatex_bibstyle, "bbx");
		updateDefaultBiblio(bp_.biblatex_citestyle, "cbx");
	} else
		updateDefaultBiblio(bp_.defaultBiblioStyle());

	biblioModule->citePackageOptionsLE->setText(toqstr(bp_.biblio_opts));

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
	// We may be called when there is no Buffer, e.g., when
	// the last view has just been closed.
	bool const isReadOnly = isBufferAvailable() ? buffer().isReadonly() : false;
	indicesModule->update(bp_, isReadOnly);

	// language & quotes
	int const pos = langModule->languageCO->findData(toqstr(
		bp_.language->lang()));
	langModule->languageCO->setCurrentIndex(pos);

	updateQuoteStyles();

	langModule->quoteStyleCO->setCurrentIndex(
		langModule->quoteStyleCO->findData(bp_.quotes_style));
	langModule->dynamicQuotesCB->setChecked(bp_.dynamic_quotes);

	// LaTeX input encoding: set after the fonts (see below)

	int p = langModule->languagePackageCO->findData(toqstr(bp_.lang_package));
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

	numberingModule->linenoCB->setChecked(bp_.use_lineno);
	numberingModule->linenoLE->setEnabled(bp_.use_lineno);
	numberingModule->linenoLA->setEnabled(bp_.use_lineno);
	numberingModule->linenoLE->setText(toqstr(bp_.lineno_opts));

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

	// math
	mathsModule->MathIndentCB->setChecked(bp_.is_math_indent);
	if (bp_.is_math_indent) {
		Length const mathindent = bp_.getMathIndent();
		int indent = 0;
		if (!mathindent.empty()) {
			lengthToWidgets(mathsModule->MathIndentLE,
			                mathsModule->MathIndentLengthCO,
			                mathindent, default_unit);
			indent = 1;
		}
		mathsModule->MathIndentCO->setCurrentIndex(indent);
		enableMathIndent(indent);
	}
	switch(bp_.math_numbering_side) {
	case BufferParams::LEFT:
		mathsModule->MathNumberingPosCO->setCurrentIndex(0);
		break;
	case BufferParams::DEFAULT:
		mathsModule->MathNumberingPosCO->setCurrentIndex(1);
		break;
	case BufferParams::RIGHT:
		mathsModule->MathNumberingPosCO->setCurrentIndex(2);
	}

	map<string, string> const & packages = BufferParams::auto_packages();
	for (map<string, string>::const_iterator it = packages.begin();
	     it != packages.end(); ++it) {
		QTableWidgetItem * item = mathsModule->packagesTW->findItems(toqstr(it->first), Qt::MatchExactly)[0];
		if (!item)
			continue;
		int row = mathsModule->packagesTW->row(item);
		switch (bp_.use_package(it->first)) {
			case BufferParams::package_off: {
				QRadioButton * rb =
					(QRadioButton*)mathsModule->packagesTW->cellWidget(row, 3)->layout()->itemAt(0)->widget();
				rb->setChecked(true);
				break;
			}
			case BufferParams::package_on: {
				QRadioButton * rb =
					(QRadioButton*)mathsModule->packagesTW->cellWidget(row, 2)->layout()->itemAt(0)->widget();
				rb->setChecked(true);
				break;
			}
			case BufferParams::package_auto: {
				QRadioButton * rb =
					(QRadioButton*)mathsModule->packagesTW->cellWidget(row, 1)->layout()->itemAt(0)->widget();
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
	int ts = textLayoutModule->tableStyleCO->findData(toqstr(bp_.tablestyle));
	if (ts != -1)
		textLayoutModule->tableStyleCO->setCurrentIndex(ts);

	if (bp_.paragraph_separation == BufferParams::ParagraphIndentSeparation) {
		textLayoutModule->indentRB->setChecked(true);
		string parindent = bp_.getParIndent().asString();
		int indent = 0;
		if (!parindent.empty()) {
			lengthToWidgets(textLayoutModule->indentLE,
			                textLayoutModule->indentLengthCO,
			                parindent, default_unit);
			indent = 1;
		}
		textLayoutModule->indentCO->setCurrentIndex(indent);
		setIndent(indent);
	} else {
		textLayoutModule->skipRB->setChecked(true);
		VSpace::VSpaceKind skip = bp_.getDefSkip().kind();
		textLayoutModule->skipCO->setCurrentIndex(textLayoutModule->skipCO->findData(skip));
		if (skip == VSpace::LENGTH) {
			string const length = bp_.getDefSkip().asLyXCommand();
			lengthToWidgets(textLayoutModule->skipLE,
				textLayoutModule->skipLengthCO,
				length, default_unit);
		}
		setSkip(textLayoutModule->skipCO->currentIndex());
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
		updateIncludeonlyDisplay();
	}
	switch (bp_.maintain_unincluded_children) {
	case BufferParams::CM_None:
		masterChildModule->maintainCRNoneRB->setChecked(true);
		break;
	case BufferParams::CM_Mostly:
		masterChildModule->maintainCRMostlyRB->setChecked(true);
		break;
	case BufferParams::CM_Strict:
	default:
		masterChildModule->maintainCRStrictRB->setChecked(true);
		break;
	}

	// Float Settings
	floatModule->setPlacement(bp_.float_placement);
	floatModule->setAlignment(bp_.float_alignment);

	// ListingsSettings
	// break listings_params to multiple lines
	string lstparams =
		InsetListingsParams(bp_.listings_params).separatedParams();
	listingsModule->listingsED->setPlainText(toqstr(lstparams));
	int nn = findToken(lst_packages, bp_.use_minted ? "Minted" : "Listings");
	if (nn >= 0)
		listingsModule->packageCO->setCurrentIndex(nn);

	// Fonts
	// some languages only work with Polyglossia (which requires non-TeX fonts)
	Language const * lang = lyx::languages.getLanguage(
		fromqstr(langModule->languageCO->itemData(
			langModule->languageCO->currentIndex()).toString()));
	bool const need_fontspec =
		lang->babel().empty() && !lang->polyglossia().empty()
		&& lang->required() != "CJK" && lang->required() != "japanese";
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

	fontModule->microtypeCB->setChecked(bp_.use_microtype);
	fontModule->dashesCB->setChecked(!bp_.use_dash_ligatures);

	fontModule->fontScCB->setChecked(bp_.fonts_expert_sc);
	fontModule->fontOsfCB->setChecked(bp_.fonts_roman_osf);
	fontModule->fontSansOsfCB->setChecked(bp_.fonts_sans_osf);
	fontModule->fontTypewriterOsfCB->setChecked(bp_.fonts_typewriter_osf);
	fontModule->scaleSansSB->setValue(bp_.fontsSansScale());
	fontModule->font_sf_scale = bp_.fonts_sans_scale[!bp_.useNonTeXFonts];
	fontModule->scaleTypewriterSB->setValue(bp_.fontsTypewriterScale());
	fontModule->font_tt_scale = bp_.fonts_typewriter_scale[!bp_.useNonTeXFonts];
	if (!bp_.font_roman_opts.empty())
		fontModule->fontspecRomanLE->setText(
			toqstr(bp_.font_roman_opts));
	else
		fontModule->fontspecRomanLE->setText(QString());
	if (!bp_.font_sans_opts.empty())
		fontModule->fontspecSansLE->setText(
			toqstr(bp_.font_sans_opts));
	else
		fontModule->fontspecSansLE->setText(QString());
	if (!bp_.font_typewriter_opts.empty())
		fontModule->fontspecTypewriterLE->setText(
			toqstr(bp_.font_typewriter_opts));
	else
		fontModule->fontspecTypewriterLE->setText(QString());

	nn = findToken(GuiDocument::fontfamilies, bp_.fonts_default_family);
	if (nn >= 0)
		fontModule->fontsDefaultCO->setCurrentIndex(nn);

	if (bp_.fontenc == "auto" || bp_.fontenc == "default") {
		fontModule->fontencCO->setCurrentIndex(
			fontModule->fontencCO->findData(toqstr(bp_.fontenc)));
		fontModule->fontencLE->setEnabled(false);
	} else {
		fontModule->fontencCO->setCurrentIndex(
					fontModule->fontencCO->findData("custom"));
		fontModule->fontencLE->setText(toqstr(bp_.fontenc));
	}

	// LaTeX input encoding
	// Set after fonts because non-tex fonts override "\inputencoding".
	inputencodingToDialog();

	// Formats
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

	outputModule->shellescapeCB->setChecked(bp_.shell_escape);
	outputModule->outputsyncCB->setChecked(bp_.output_sync);
	outputModule->synccustomCB->setEditText(toqstr(bp_.output_sync_macro));
	outputModule->synccustomCB->setEnabled(bp_.output_sync);
	outputModule->synccustomLA->setEnabled(bp_.output_sync);

	outputModule->mathimgSB->setValue(bp_.html_math_img_scale);
	outputModule->mathoutCB->setCurrentIndex(bp_.html_math_output);
	outputModule->strictCB->setChecked(bp_.html_be_strict);
	outputModule->cssCB->setChecked(bp_.html_css_as_file);

	outputModule->tableoutCB->setCurrentIndex(bp_.docbook_table_output);

	outputModule->saveTransientPropertiesCB
		->setChecked(bp_.save_transient_properties);
	outputModule->postponeFragileCB
		->setChecked(bp_.postpone_fragile_content);

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
	pdfSupportModule->bookmarksopenlevelSB->setEnabled(pdf.bookmarksopen);
	pdfSupportModule->bookmarksopenlevelLA->setEnabled(pdf.bookmarksopen);

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

	// change tracking
	changesModule->trackChangesCB->setChecked(bp_.track_changes);
	changesModule->outputChangesCB->setChecked(bp_.output_changes);
	changesModule->changeBarsCB->setChecked(bp_.change_bars);
	changesModule->changeBarsCB->setEnabled(bp_.output_changes);

	// Make sure that the bc is in the INITIAL state
	if (bc().policy().buttonStatus(ButtonPolicy::RESTORE))
		bc().restore();

	// clear changed branches cache
	changedBranches_.clear();

	// reset trackers
	nonModuleChanged_ = false;
	shellescapeChanged_ = false;
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
	list<modInfoStruct> modInfoList = getModuleInfo();
	// Sort names according to the locale
	modInfoList.sort([](modInfoStruct const & a, modInfoStruct const & b) {
			return 0 < b.name.localeAwareCompare(a.name);
		});
	QIcon user_icon(getPixmap("images/", "lyxfiles-user", "svgz,png"));
	QIcon system_icon(getPixmap("images/", "lyxfiles-system", "svgz,png"));
	int i = 0;
	QFont catfont;
	catfont.setBold(true);
	QBrush unavbrush;
	unavbrush.setColor(Qt::gray);
	for (modInfoStruct const & m : modInfoList) {
		QStandardItem * item = new QStandardItem();
		QStandardItem * catItem;
		QString const catname = m.category;
		QList<QStandardItem *> fcats = modules_av_model_.findItems(catname, Qt::MatchExactly);
		if (!fcats.empty())
			catItem = fcats.first();
		else {
			catItem = new QStandardItem();
			catItem->setText(catname);
			catItem->setFont(catfont);
			modules_av_model_.insertRow(i, catItem);
			++i;
		}
		item->setEditable(false);
		catItem->setEditable(false);
		item->setData(m.name, Qt::DisplayRole);
		if (m.missingreqs)
			item->setForeground(unavbrush);
		item->setData(toqstr(m.id), Qt::UserRole);
		item->setData(m.description, Qt::ToolTipRole);
		if (m.local)
			item->setIcon(user_icon);
		else
			item->setIcon(system_icon);
		catItem->appendRow(item);
	}
	modules_av_model_.sort(0);
}


void GuiDocument::updateSelectedModules()
{
	modules_sel_model_.clear();
	list<modInfoStruct> const selModList = getSelectedModules();
	int i = 0;
	for (modInfoStruct const & m : selModList) {
		modules_sel_model_.insertRow(i, m.name, m.id, m.description);
		++i;
	}
}


void GuiDocument::updateIncludeonlyDisplay()
{
	if (includeonlys_.empty()) {
		masterChildModule->includeallRB->setChecked(true);
		masterChildModule->childrenTW->setEnabled(false);
		masterChildModule->maintainGB->setEnabled(false);
	} else {
		masterChildModule->includeonlyRB->setChecked(true);
		masterChildModule->childrenTW->setEnabled(true);
		masterChildModule->maintainGB->setEnabled(true);
	}
}


void GuiDocument::updateIncludeonlys()
{
	masterChildModule->childrenTW->clear();
	QString const no = qt_("No");
	QString const yes = qt_("Yes");

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
	// Both if all children are included and if none is included
	// is equal to "include all" (i.e., omit \includeonly).
	if (!has_unincluded || all_unincluded)
		includeonlys_.clear();
}


bool GuiDocument::isBiblatex() const
{
	QString const engine =
		biblioModule->citeEngineCO->itemData(
				biblioModule->citeEngineCO->currentIndex()).toString();

	// this can happen if the cite engine is unknown, which can happen
	// if one is using a file that came from someone else, etc. in that
	// case, we crash if we proceed.
	if (engine.isEmpty())
	    return false;

	return theCiteEnginesList[fromqstr(engine)]->getCiteFramework() == "biblatex";
}


void GuiDocument::updateDefaultBiblio(string const & style,
				      string const & which)
{
	QString const bibstyle = toqstr(style);
	biblioModule->defaultBiblioCO->clear();

	int item_nr = -1;

	if (isBiblatex()) {
		if (which != "cbx") {
			// First the bbx styles
			biblioModule->biblatexBbxCO->clear();
			QStringList str = texFileList("bbxFiles.lst");
			// test whether we have a valid list, otherwise run rescan
			if (str.isEmpty()) {
				rescanTexStyles("bbx");
				str = texFileList("bbxFiles.lst");
			}
			for (int i = 0; i != str.size(); ++i)
				str[i] = onlyFileName(str[i]);
			// sort on filename only (no path)
			str.sort();

			for (int i = 0; i != str.count(); ++i) {
				QString item = changeExtension(str[i], "");
				if (item == bibstyle)
					item_nr = i;
				biblioModule->biblatexBbxCO->addItem(item);
			}

			if (item_nr == -1 && !bibstyle.isEmpty()) {
				biblioModule->biblatexBbxCO->addItem(bibstyle);
				item_nr = biblioModule->biblatexBbxCO->count() - 1;
			}

			if (item_nr != -1)
				biblioModule->biblatexBbxCO->setCurrentIndex(item_nr);
			else
				biblioModule->biblatexBbxCO->clearEditText();
		}

		if (which != "bbx") {
			// now the cbx styles
			biblioModule->biblatexCbxCO->clear();
			QStringList str = texFileList("cbxFiles.lst");
			// test whether we have a valid list, otherwise run rescan
			if (str.isEmpty()) {
				rescanTexStyles("cbx");
				str = texFileList("cbxFiles.lst");
			}
			for (int i = 0; i != str.size(); ++i)
				str[i] = onlyFileName(str[i]);
			// sort on filename only (no path)
			str.sort();

			for (int i = 0; i != str.count(); ++i) {
				QString item = changeExtension(str[i], "");
				if (item == bibstyle)
					item_nr = i;
				biblioModule->biblatexCbxCO->addItem(item);
			}

			if (item_nr == -1 && !bibstyle.isEmpty()) {
				biblioModule->biblatexCbxCO->addItem(bibstyle);
				item_nr = biblioModule->biblatexCbxCO->count() - 1;
			}

			if (item_nr != -1)
				biblioModule->biblatexCbxCO->setCurrentIndex(item_nr);
			else
				biblioModule->biblatexCbxCO->clearEditText();
		}
	} else {// BibTeX
		biblioModule->biblatexBbxCO->clear();
		biblioModule->biblatexCbxCO->clear();
		QStringList str = texFileList("bstFiles.lst");
		// test whether we have a valid list, otherwise run rescan
		if (str.isEmpty()) {
			rescanTexStyles("bst");
			str = texFileList("bstFiles.lst");
		}
		for (int i = 0; i != str.size(); ++i)
			str[i] = onlyFileName(str[i]);
		// sort on filename only (no path)
		str.sort();

		for (int i = 0; i != str.count(); ++i) {
			QString item = changeExtension(str[i], "");
			if (item == bibstyle)
				item_nr = i;
			biblioModule->defaultBiblioCO->addItem(item);
		}

		if (item_nr == -1 && !bibstyle.isEmpty()) {
			biblioModule->defaultBiblioCO->addItem(bibstyle);
			item_nr = biblioModule->defaultBiblioCO->count() - 1;
		}

		if (item_nr != -1)
			biblioModule->defaultBiblioCO->setCurrentIndex(item_nr);
		else
			biblioModule->defaultBiblioCO->clearEditText();
	}

	updateResetDefaultBiblio();
}


void GuiDocument::updateResetDefaultBiblio()
{
	QString const engine =
		biblioModule->citeEngineCO->itemData(
				biblioModule->citeEngineCO->currentIndex()).toString();
	CiteEngineType const cet =
		CiteEngineType(biblioModule->citeStyleCO->itemData(
							  biblioModule->citeStyleCO->currentIndex()).toInt());

	string const defbib = theCiteEnginesList[fromqstr(engine)]->getDefaultBiblio(cet);
	if (isBiblatex()) {
		QString const bbx = biblioModule->biblatexBbxCO->currentText();
		QString const cbx = biblioModule->biblatexCbxCO->currentText();
		biblioModule->resetCbxPB->setEnabled(defbib != fromqstr(cbx));
		biblioModule->resetBbxPB->setEnabled(defbib != fromqstr(bbx));
		biblioModule->matchBbxPB->setEnabled(bbx != cbx && !cbx.isEmpty()
			&& biblioModule->biblatexBbxCO->findText(cbx) != -1);
	} else
		biblioModule->resetDefaultBiblioPB->setEnabled(
			defbib != fromqstr(biblioModule->defaultBiblioCO->currentText()));
}


void GuiDocument::matchBiblatexStyles()
{
	updateDefaultBiblio(fromqstr(biblioModule->biblatexCbxCO->currentText()), "bbx");
	biblioChanged();
}


void GuiDocument::updateContents()
{
	// Nothing to do here as the document settings is not cursor dependent.
	return;
}


void GuiDocument::useClassDefaults()
{
	if (buttonBox->button(QDialogButtonBox::Apply)->isEnabled()) {
		int const ret = Alert::prompt(_("Unapplied changes"),
				_("Some changes in the dialog were not yet applied.\n"
				  "If you do not apply now, they will be lost after this action."),
				1, 1, _("&Apply"), _("&Dismiss"));
		if (ret == 0)
			applyView();
	}

	int idx = latexModule->classCO->currentIndex();
	string const classname = fromqstr(latexModule->classCO->getData(idx));
	if (!bp_.setBaseClass(classname, buffer().layoutPos())) {
		Alert::error(_("Error"), _("Unable to set document class."));
		return;
	}
	bp_.useClassDefaults();
	paramsToDialog();
	changed();
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
		!localLayout->editing() &&
		!preambleModule->editing() &&
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
		) &&
		(
			// if we're asking for math indentation
			!mathsModule->MathIndentCB->isChecked() ||
			// then either we haven't chosen custom
			mathsModule->MathIndentCO->currentIndex() != 1 ||
			// or else a length has been given
			!mathsModule->MathIndentLE->text().isEmpty()
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
	return view? &view->buffer() : nullptr;
}


list<GuiDocument::modInfoStruct> const & GuiDocument::getModuleInfo()
{
	return moduleNames_;
}


list<GuiDocument::modInfoStruct> const
GuiDocument::makeModuleInfo(LayoutModuleList const & mods)
{
	list<modInfoStruct> mInfo;
	for (string const & name : mods) {
		modInfoStruct m;
		LyXModule const * const mod = theModuleList[name];
		if (mod)
			m = modInfo(*mod);
		else {
			m.id = name;
			m.name = toqstr(name + " (") + qt_("Not Found") + toqstr(")");
			m.local = false;
			m.missingreqs = true;
		}
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
	// This handles undo groups automagically
	UndoGroupHelper ugh(&buf);

	// This must come first so that a language change is correctly noticed
	setLanguage();

	// We need to load the master before we formally update the params,
	// since otherwise we run updateBuffer, etc, before the child's master
	// has been set.
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

	// Apply the BufferParams. Note that this will set the base class
	// and then update the buffer's layout.
	dispatch_bufferparams(*this, params(), LFUN_BUFFER_PARAMS_APPLY, &buffer());

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


bool GuiDocument::providesExtraOpts(QString const & font) const
{
	if (fontModule->osFontsCB->isChecked())
		return true;
	return theLaTeXFonts().getLaTeXFont(
				qstring_to_ucs4(font)).providesMoreOptions(ot1(),
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


//static
GuiDocument::modInfoStruct GuiDocument::modInfo(LyXModule const & mod)
{
	// FIXME Unicode: docstrings would be better for these parameters but this
	// change requires a lot of others
	modInfoStruct m;
	m.id = mod.getID();
	QString const guiname = toqstr(translateIfPossible(from_utf8(mod.getName())));
	m.missingreqs = !isModuleAvailable(mod.getID());
	if (m.missingreqs) {
		m.name = QString(qt_("%1 (missing req.)")).arg(guiname);
	} else
		m.name = guiname;
	m.category = mod.category().empty() ? qt_("Miscellaneous")
					    : toqstr(translateIfPossible(from_utf8(mod.category())));
	QString desc = toqstr(translateIfPossible(from_utf8(mod.getDescription())));
	// Find the first sentence of the description
	QTextBoundaryFinder bf(QTextBoundaryFinder::Sentence, desc);
	int pos = bf.toNextBoundary();
	if (pos > 0)
		desc.truncate(pos);
	m.local = mod.isLocal();
	QString const mtype = m.local ? qt_("personal module") : qt_("distributed module");
	QString modulename = QString(qt_("<b>Module name:</b> <i>%1</i> (%2)")).arg(toqstr(m.id)).arg(mtype);
	// Tooltip is the desc followed by the module name and the type
	m.description = QString("%1%2")
		.arg(desc.isEmpty() ? QString() : QString("<p>%1</p>").arg(desc),
		     modulename);
	if (m.missingreqs)
		m.description += QString("<p>%1</p>").arg(qt_("<b>Note:</b> Some requirements for this module are missing!"));
	return m;
}


void GuiDocument::loadModuleInfo()
{
	moduleNames_.clear();
	for (LyXModule const & mod : theModuleList)
		moduleNames_.push_back(modInfo(mod));
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
		QRadioButton * rb =
			(QRadioButton*)mathsModule->packagesTW->cellWidget(row, col)->layout()->itemAt(0)->widget();
		rb->setChecked(true);
	}
}


void GuiDocument::linenoToggled(bool on)
{
	numberingModule->linenoLE->setEnabled(on);
	numberingModule->linenoLA->setEnabled(on);
}


void GuiDocument::outputChangesToggled(bool on)
{
	changesModule->changeBarsCB->setEnabled(on);
	change_adaptor();
}

void GuiDocument::setOutputSync(bool on)
{
	outputModule->synccustomCB->setEnabled(on);
	outputModule->synccustomLA->setEnabled(on);
	change_adaptor();
}



Dialog * createGuiDocument(GuiView & lv) { return new GuiDocument(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiDocument.cpp"
