/**
 * \file GuiPrefs.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiPrefs.h"

#include "ColorCache.h"
#include "FileDialog.h"
#include "GuiApplication.h"
#include "GuiFontExample.h"
#include "GuiFontLoader.h"
#include "GuiKeySymbol.h"
#include "qt_helpers.h"

#include "BufferList.h"
#include "Color.h"
#include "ColorSet.h"
#include "ConverterCache.h"
#include "FontEnums.h"
#include "FuncRequest.h"
#include "KeyMap.h"
#include "KeySequence.h"
#include "Language.h"
#include "LyXAction.h"
#include "PanelStack.h"
#include "paper.h"
#include "Session.h"

#include "support/debug.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/foreach.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/Package.h"

#include "graphics/GraphicsTypes.h"

#include "frontends/alert.h"
#include "frontends/Application.h"
#include "frontends/FontLoader.h"

#include <QAbstractItemModel>
#include <QCheckBox>
#include <QColorDialog>
#include <QFontDatabase>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QPixmapCache>
#include <QPushButton>
#include <QSpinBox>
#include <QString>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QValidator>

#include <iomanip>
#include <sstream>
#include <algorithm>

using namespace Ui;

using namespace std;
using namespace lyx::support;
using namespace lyx::support::os;

namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////////
//
// Browser Helpers
//
/////////////////////////////////////////////////////////////////////

/** Launch a file dialog and return the chosen file.
	filename: a suggested filename.
	title: the title of the dialog.
	pattern: *.ps etc.
	dir1 = (name, dir), dir2 = (name, dir): extra buttons on the dialog.
*/
QString browseFile(QString const & filename,
	QString const & title,
	QStringList const & filters,
	bool save = false,
	QString const & label1 = QString(),
	QString const & dir1 = QString(),
	QString const & label2 = QString(),
	QString const & dir2 = QString(),
	QString const & fallback_dir = QString())
{
	QString lastPath = ".";
	if (!filename.isEmpty())
		lastPath = onlyPath(filename);
	else if(!fallback_dir.isEmpty())
		lastPath = fallback_dir;

	FileDialog dlg(title, LFUN_SELECT_FILE_SYNC);
	dlg.setButton2(label1, dir1);
	dlg.setButton2(label2, dir2);

	FileDialog::Result result;

	if (save)
		result = dlg.save(lastPath, filters, onlyFilename(filename));
	else
		result = dlg.open(lastPath, filters, onlyFilename(filename));

	return result.second;
}


/** Wrapper around browseFile which tries to provide a filename
*  relative to the user or system directory. The dir, name and ext
*  parameters have the same meaning as in the
*  support::LibFileSearch function.
*/
QString browseLibFile(QString const & dir,
	QString const & name,
	QString const & ext,
	QString const & title,
	QStringList const & filters)
{
	// FIXME UNICODE
	QString const label1 = qt_("System files|#S#s");
	QString const dir1 =
		toqstr(addName(package().system_support().absFilename(), fromqstr(dir)));

	QString const label2 = qt_("User files|#U#u");
	QString const dir2 =
		toqstr(addName(package().user_support().absFilename(), fromqstr(dir)));

	QString const result = browseFile(toqstr(
		libFileSearch(dir, name, ext).absFilename()),
		title, filters, false, dir1, dir2, QString(), QString(), dir1);

	// remove the extension if it is the default one
	QString noextresult;
	if (getExtension(result) == ext)
		noextresult = removeExtension(result);
	else
		noextresult = result;

	// remove the directory, if it is the default one
	QString const file = onlyFilename(noextresult);
	if (toqstr(libFileSearch(dir, file, ext).absFilename()) == result)
		return file;
	else
		return noextresult;
}


/** Launch a file dialog and return the chosen directory.
	pathname: a suggested pathname.
	title: the title of the dialog.
	dir1 = (name, dir), dir2 = (name, dir): extra buttons on the dialog.
*/
QString browseDir(QString const & pathname,
	QString const & title,
	QString const & label1 = QString(),
	QString const & dir1 = QString(),
	QString const & label2 = QString(),
	QString const & dir2 = QString())
{
	QString lastPath = ".";
	if (!pathname.isEmpty())
		lastPath = onlyPath(pathname);

	FileDialog dlg(title, LFUN_SELECT_FILE_SYNC);
	dlg.setButton1(label1, dir1);
	dlg.setButton2(label2, dir2);

	FileDialog::Result const result =
		dlg.opendir(lastPath, onlyFilename(pathname));

	return result.second;
}


} // namespace frontend


QString browseRelFile(QString const & filename, QString const & refpath,
	QString const & title, QStringList const & filters, bool save,
	QString const & label1, QString const & dir1,
	QString const & label2, QString const & dir2)
{
	QString const fname = makeAbsPath(filename, refpath);


	QString const outname =
		frontend::browseFile(fname, title, filters, save, label1, dir1, label2, dir2);

	QString const reloutname =
		toqstr(makeRelPath(qstring_to_ucs4(outname), qstring_to_ucs4(refpath)));

	if (reloutname.startsWith("../"))
		return outname;
	else
		return reloutname;
}



/////////////////////////////////////////////////////////////////////
//
// Helpers
//
/////////////////////////////////////////////////////////////////////

namespace frontend {

string const catLookAndFeel = N_("Look & Feel");
string const catEditing = N_("Editing");
string const catLanguage = N_("Language Settings");
string const catOutput = N_("Output");
string const catFiles = N_("File Handling");

static void parseFontName(QString const & mangled0,
	string & name, string & foundry)
{
	string mangled = fromqstr(mangled0);
	size_t const idx = mangled.find('[');
	if (idx == string::npos || idx == 0) {
		name = mangled;
		foundry.clear();
	} else {
		name = mangled.substr(0, idx - 1),
		foundry = mangled.substr(idx + 1, mangled.size() - idx - 2);
	}
}


static void setComboxFont(QComboBox * cb, string const & family,
	string const & foundry)
{
	QString fontname = toqstr(family);
	if (!foundry.empty())
		fontname += " [" + toqstr(foundry) + ']';

	for (int i = 0; i != cb->count(); ++i) {
		if (cb->itemText(i) == fontname) {
			cb->setCurrentIndex(i);
			return;
		}
	}

	// Try matching without foundry name

	// We count in reverse in order to prefer the Xft foundry
	for (int i = cb->count(); --i >= 0;) {
		string name, foundry;
		parseFontName(cb->itemText(i), name, foundry);
		if (compare_ascii_no_case(name, family) == 0) {
			cb->setCurrentIndex(i);
			return;
		}
	}

	// family alone can contain e.g. "Helvetica [Adobe]"
	string tmpname, tmpfoundry;
	parseFontName(toqstr(family), tmpname, tmpfoundry);

	// We count in reverse in order to prefer the Xft foundry
	for (int i = cb->count(); --i >= 0; ) {
		string name, foundry;
		parseFontName(cb->itemText(i), name, foundry);
		if (compare_ascii_no_case(name, foundry) == 0) {
			cb->setCurrentIndex(i);
			return;
		}
	}

	// Bleh, default fonts, and the names couldn't be found. Hack
	// for bug 1063.

	QFont font;
	font.setKerning(false);

	QString const font_family = toqstr(family);
	if (font_family == guiApp->romanFontName()) {
		font.setStyleHint(QFont::Serif);
		font.setFamily(font_family);
	} else if (font_family == guiApp->sansFontName()) {
		font.setStyleHint(QFont::SansSerif);
		font.setFamily(font_family);
	} else if (font_family == guiApp->typewriterFontName()) {
		font.setStyleHint(QFont::TypeWriter);
		font.setFamily(font_family);
	} else {
		LYXERR0("FAILED to find the default font: '"
		       << foundry << "', '" << family << '\'');
		return;
	}

	QFontInfo info(font);
	string default_font_name, dummyfoundry;
	parseFontName(info.family(), default_font_name, dummyfoundry);
	LYXERR0("Apparent font is " << default_font_name);

	for (int i = 0; i < cb->count(); ++i) {
		LYXERR0("Looking at " << cb->itemText(i));
		if (compare_ascii_no_case(fromqstr(cb->itemText(i)),
				    default_font_name) == 0) {
			cb->setCurrentIndex(i);
			return;
		}
	}

	LYXERR0("FAILED to find the font: '"
	       << foundry << "', '" << family << '\'');
}



/////////////////////////////////////////////////////////////////////
//
// PrefPlaintext
//
/////////////////////////////////////////////////////////////////////

PrefPlaintext::PrefPlaintext(GuiPreferences * form)
	: PrefModule(qt_(catOutput), qt_("Plain text"), form)
{
	setupUi(this);
	connect(plaintextLinelengthSB, SIGNAL(valueChanged(int)),
		this, SIGNAL(changed()));
}


void PrefPlaintext::apply(LyXRC & rc) const
{
	rc.plaintext_linelen = plaintextLinelengthSB->value();
}


void PrefPlaintext::update(LyXRC const & rc)
{
	plaintextLinelengthSB->setValue(rc.plaintext_linelen);
}


/////////////////////////////////////////////////////////////////////
//
// StrftimeValidator
//
/////////////////////////////////////////////////////////////////////

class StrftimeValidator : public QValidator
{
public:
	StrftimeValidator(QWidget *);
	QValidator::State validate(QString & input, int & pos) const;
};


StrftimeValidator::StrftimeValidator(QWidget * parent)
	: QValidator(parent)
{
}


QValidator::State StrftimeValidator::validate(QString & input, int & /*pos*/) const
{
	if (is_valid_strftime(fromqstr(input)))
		return QValidator::Acceptable;
	else
		return QValidator::Intermediate;
}


/////////////////////////////////////////////////////////////////////
//
// PrefDate
//
/////////////////////////////////////////////////////////////////////

PrefDate::PrefDate(GuiPreferences * form)
	: PrefModule(qt_(catOutput), qt_("Date format"), form)
{
	setupUi(this);
	DateED->setValidator(new StrftimeValidator(DateED));
	connect(DateED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
}


void PrefDate::on_DateED_textChanged(const QString &)
{
	QString t = DateED->text();
	int p = 0;
	bool valid = DateED->validator()->validate(t, p)
		     == QValidator::Acceptable;
	setValid(DateLA, valid);
}


void PrefDate::apply(LyXRC & rc) const
{
	rc.date_insert_format = fromqstr(DateED->text());
}


void PrefDate::update(LyXRC const & rc)
{
	DateED->setText(toqstr(rc.date_insert_format));
}


/////////////////////////////////////////////////////////////////////
//
// PrefInput
//
/////////////////////////////////////////////////////////////////////

PrefInput::PrefInput(GuiPreferences * form)
	: PrefModule(qt_(catEditing), qt_("Keyboard/Mouse"), form)
{
	setupUi(this);

	connect(keymapCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(firstKeymapED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(secondKeymapED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(mouseWheelSpeedSB, SIGNAL(valueChanged(double)),
		this, SIGNAL(changed()));
}


void PrefInput::apply(LyXRC & rc) const
{
	// FIXME: can derive CB from the two EDs
	rc.use_kbmap = keymapCB->isChecked();
	rc.primary_kbmap = internal_path(fromqstr(firstKeymapED->text()));
	rc.secondary_kbmap = internal_path(fromqstr(secondKeymapED->text()));
	rc.mouse_wheel_speed = mouseWheelSpeedSB->value();
}


void PrefInput::update(LyXRC const & rc)
{
	// FIXME: can derive CB from the two EDs
	keymapCB->setChecked(rc.use_kbmap);
	firstKeymapED->setText(toqstr(external_path(rc.primary_kbmap)));
	secondKeymapED->setText(toqstr(external_path(rc.secondary_kbmap)));
	mouseWheelSpeedSB->setValue(rc.mouse_wheel_speed);
}


QString PrefInput::testKeymap(QString const & keymap)
{
	return form_->browsekbmap(internalPath(keymap));
}


void PrefInput::on_firstKeymapPB_clicked(bool)
{
	QString const file = testKeymap(firstKeymapED->text());
	if (!file.isEmpty())
		firstKeymapED->setText(file);
}


void PrefInput::on_secondKeymapPB_clicked(bool)
{
	QString const file = testKeymap(secondKeymapED->text());
	if (!file.isEmpty())
		secondKeymapED->setText(file);
}


void PrefInput::on_keymapCB_toggled(bool keymap)
{
	firstKeymapLA->setEnabled(keymap);
	secondKeymapLA->setEnabled(keymap);
	firstKeymapED->setEnabled(keymap);
	secondKeymapED->setEnabled(keymap);
	firstKeymapPB->setEnabled(keymap);
	secondKeymapPB->setEnabled(keymap);
}


/////////////////////////////////////////////////////////////////////
//
// PrefCompletion
//
/////////////////////////////////////////////////////////////////////

PrefCompletion::PrefCompletion(GuiPreferences * form)
	: PrefModule(qt_(catEditing), qt_("Input Completion"), form)
{
	setupUi(this);

	connect(inlineDelaySB, SIGNAL(valueChanged(double)),
		this, SIGNAL(changed()));
	connect(inlineMathCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(inlineTextCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(inlineDotsCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(popupDelaySB, SIGNAL(valueChanged(double)),
		this, SIGNAL(changed()));
	connect(popupMathCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(popupTextCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(popupAfterCompleteCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(cursorTextCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
}


void PrefCompletion::apply(LyXRC & rc) const
{
	rc.completion_inline_delay = inlineDelaySB->value();
	rc.completion_inline_math = inlineMathCB->isChecked();
	rc.completion_inline_text = inlineTextCB->isChecked();
	rc.completion_inline_dots = inlineDotsCB->isChecked() ? 13 : -1;
	rc.completion_popup_delay = popupDelaySB->value();
	rc.completion_popup_math = popupMathCB->isChecked();
	rc.completion_popup_text = popupTextCB->isChecked();
	rc.completion_cursor_text = cursorTextCB->isChecked();
	rc.completion_popup_after_complete =
		popupAfterCompleteCB->isChecked();
}


void PrefCompletion::update(LyXRC const & rc)
{
	inlineDelaySB->setValue(rc.completion_inline_delay);
	inlineMathCB->setChecked(rc.completion_inline_math);
	inlineTextCB->setChecked(rc.completion_inline_text);
	inlineDotsCB->setChecked(rc.completion_inline_dots != -1);
	popupDelaySB->setValue(rc.completion_popup_delay);
	popupMathCB->setChecked(rc.completion_popup_math);
	popupTextCB->setChecked(rc.completion_popup_text);
	cursorTextCB->setChecked(rc.completion_cursor_text);
	popupAfterCompleteCB->setChecked(rc.completion_popup_after_complete);
}



/////////////////////////////////////////////////////////////////////
//
// PrefLatex
//
/////////////////////////////////////////////////////////////////////

PrefLatex::PrefLatex(GuiPreferences * form)
	: PrefModule(qt_(catOutput), qt_("LaTeX"), form)
{
	setupUi(this);
	connect(latexEncodingED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(latexChecktexED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(latexBibtexED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(latexJBibtexED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(latexIndexED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(latexJIndexED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(latexAutoresetCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(latexDviPaperED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(latexPaperSizeCO, SIGNAL(activated(int)),
		this, SIGNAL(changed()));

#if defined(__CYGWIN__) || defined(_WIN32)
	pathCB->setVisible(true);
	connect(pathCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
#else
	pathCB->setVisible(false);
#endif
}


void PrefLatex::apply(LyXRC & rc) const
{
	rc.fontenc = fromqstr(latexEncodingED->text());
	rc.chktex_command = fromqstr(latexChecktexED->text());
	rc.bibtex_command = fromqstr(latexBibtexED->text());
	rc.jbibtex_command = fromqstr(latexJBibtexED->text());
	rc.index_command = fromqstr(latexIndexED->text());
	rc.index_command = fromqstr(latexJIndexED->text());
	rc.nomencl_command = fromqstr(latexNomenclED->text());
	rc.auto_reset_options = latexAutoresetCB->isChecked();
	rc.view_dvi_paper_option = fromqstr(latexDviPaperED->text());
	rc.default_papersize =
		form_->toPaperSize(latexPaperSizeCO->currentIndex());
#if defined(__CYGWIN__) || defined(_WIN32)
	rc.windows_style_tex_paths = pathCB->isChecked();
#endif
}


void PrefLatex::update(LyXRC const & rc)
{
	latexEncodingED->setText(toqstr(rc.fontenc));
	latexChecktexED->setText(toqstr(rc.chktex_command));
	latexBibtexED->setText(toqstr(rc.bibtex_command));
	latexJBibtexED->setText(toqstr(rc.jbibtex_command));
	latexIndexED->setText(toqstr(rc.index_command));
	latexJIndexED->setText(toqstr(rc.jindex_command));
	latexNomenclED->setText(toqstr(rc.nomencl_command));
	latexAutoresetCB->setChecked(rc.auto_reset_options);
	latexDviPaperED->setText(toqstr(rc.view_dvi_paper_option));
	latexPaperSizeCO->setCurrentIndex(
		form_->fromPaperSize(rc.default_papersize));
#if defined(__CYGWIN__) || defined(_WIN32)
	pathCB->setChecked(rc.windows_style_tex_paths);
#endif
}


/////////////////////////////////////////////////////////////////////
//
// PrefScreenFonts
//
/////////////////////////////////////////////////////////////////////

PrefScreenFonts::PrefScreenFonts(GuiPreferences * form)
	: PrefModule(qt_(catLookAndFeel), qt_("Screen fonts"), form)
{
	setupUi(this);

	connect(screenRomanCO, SIGNAL(activated(QString)),
		this, SLOT(select_roman(QString)));
	connect(screenSansCO, SIGNAL(activated(QString)),
		this, SLOT(select_sans(QString)));
	connect(screenTypewriterCO, SIGNAL(activated(QString)),
		this, SLOT(select_typewriter(QString)));

	QFontDatabase fontdb;
	QStringList families(fontdb.families());
	for (QStringList::Iterator it = families.begin(); it != families.end(); ++it) {
		screenRomanCO->addItem(*it);
		screenSansCO->addItem(*it);
		screenTypewriterCO->addItem(*it);
	}
	connect(screenRomanCO, SIGNAL(activated(QString)),
		this, SIGNAL(changed()));
	connect(screenSansCO, SIGNAL(activated(QString)),
		this, SIGNAL(changed()));
	connect(screenTypewriterCO, SIGNAL(activated(QString)),
		this, SIGNAL(changed()));
	connect(screenZoomSB, SIGNAL(valueChanged(int)),
		this, SIGNAL(changed()));
	connect(screenDpiSB, SIGNAL(valueChanged(int)),
		this, SIGNAL(changed()));
	connect(screenTinyED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(screenSmallestED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(screenSmallerED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(screenSmallED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(screenNormalED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(screenLargeED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(screenLargerED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(screenLargestED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(screenHugeED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(screenHugerED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(pixmapCacheCB, SIGNAL(toggled(bool)),
		this, SIGNAL(changed()));

	screenTinyED->setValidator(new QDoubleValidator(screenTinyED));
	screenSmallestED->setValidator(new QDoubleValidator(screenSmallestED));
	screenSmallerED->setValidator(new QDoubleValidator(screenSmallerED));
	screenSmallED->setValidator(new QDoubleValidator(screenSmallED));
	screenNormalED->setValidator(new QDoubleValidator(screenNormalED));
	screenLargeED->setValidator(new QDoubleValidator(screenLargeED));
	screenLargerED->setValidator(new QDoubleValidator(screenLargerED));
	screenLargestED->setValidator(new QDoubleValidator(screenLargestED));
	screenHugeED->setValidator(new QDoubleValidator(screenHugeED));
	screenHugerED->setValidator(new QDoubleValidator(screenHugerED));
}


void PrefScreenFonts::apply(LyXRC & rc) const
{
	LyXRC const oldrc = rc;

	parseFontName(screenRomanCO->currentText(),
		rc.roman_font_name, rc.roman_font_foundry);
	parseFontName(screenSansCO->currentText(),
		rc.sans_font_name, rc.sans_font_foundry);
	parseFontName(screenTypewriterCO->currentText(),
		rc.typewriter_font_name, rc.typewriter_font_foundry);

	rc.zoom = screenZoomSB->value();
	rc.dpi = screenDpiSB->value();
	rc.font_sizes[FONT_SIZE_TINY] = fromqstr(screenTinyED->text());
	rc.font_sizes[FONT_SIZE_SCRIPT] = fromqstr(screenSmallestED->text());
	rc.font_sizes[FONT_SIZE_FOOTNOTE] = fromqstr(screenSmallerED->text());
	rc.font_sizes[FONT_SIZE_SMALL] = fromqstr(screenSmallED->text());
	rc.font_sizes[FONT_SIZE_NORMAL] = fromqstr(screenNormalED->text());
	rc.font_sizes[FONT_SIZE_LARGE] = fromqstr(screenLargeED->text());
	rc.font_sizes[FONT_SIZE_LARGER] = fromqstr(screenLargerED->text());
	rc.font_sizes[FONT_SIZE_LARGEST] = fromqstr(screenLargestED->text());
	rc.font_sizes[FONT_SIZE_HUGE] = fromqstr(screenHugeED->text());
	rc.font_sizes[FONT_SIZE_HUGER] = fromqstr(screenHugerED->text());
	rc.use_pixmap_cache = pixmapCacheCB->isChecked();

	if (rc.font_sizes != oldrc.font_sizes
		|| rc.roman_font_name != oldrc.roman_font_name
		|| rc.sans_font_name != oldrc.sans_font_name
		|| rc.typewriter_font_name != oldrc.typewriter_font_name
		|| rc.zoom != oldrc.zoom || rc.dpi != oldrc.dpi) {
		// The global QPixmapCache is used in GuiPainter to cache text
		// painting so we must reset it in case any of the above
		// parameter is changed.
		QPixmapCache::clear();
		guiApp->fontLoader().update();
		form_->updateScreenFonts();
	}
}


void PrefScreenFonts::update(LyXRC const & rc)
{
	setComboxFont(screenRomanCO, rc.roman_font_name,
			rc.roman_font_foundry);
	setComboxFont(screenSansCO, rc.sans_font_name,
			rc.sans_font_foundry);
	setComboxFont(screenTypewriterCO, rc.typewriter_font_name,
			rc.typewriter_font_foundry);

	select_roman(screenRomanCO->currentText());
	select_sans(screenSansCO->currentText());
	select_typewriter(screenTypewriterCO->currentText());

	screenZoomSB->setValue(rc.zoom);
	screenDpiSB->setValue(rc.dpi);
	screenTinyED->setText(toqstr(rc.font_sizes[FONT_SIZE_TINY]));
	screenSmallestED->setText(toqstr(rc.font_sizes[FONT_SIZE_SCRIPT]));
	screenSmallerED->setText(toqstr(rc.font_sizes[FONT_SIZE_FOOTNOTE]));
	screenSmallED->setText(toqstr(rc.font_sizes[FONT_SIZE_SMALL]));
	screenNormalED->setText(toqstr(rc.font_sizes[FONT_SIZE_NORMAL]));
	screenLargeED->setText(toqstr(rc.font_sizes[FONT_SIZE_LARGE]));
	screenLargerED->setText(toqstr(rc.font_sizes[FONT_SIZE_LARGER]));
	screenLargestED->setText(toqstr(rc.font_sizes[FONT_SIZE_LARGEST]));
	screenHugeED->setText(toqstr(rc.font_sizes[FONT_SIZE_HUGE]));
	screenHugerED->setText(toqstr(rc.font_sizes[FONT_SIZE_HUGER]));

	pixmapCacheCB->setChecked(rc.use_pixmap_cache);
#if defined(Q_WS_X11)
	pixmapCacheCB->setEnabled(false);
#endif

}


void PrefScreenFonts::select_roman(const QString & name)
{
	screenRomanFE->set(QFont(name), name);
}


void PrefScreenFonts::select_sans(const QString & name)
{
	screenSansFE->set(QFont(name), name);
}


void PrefScreenFonts::select_typewriter(const QString & name)
{
	screenTypewriterFE->set(QFont(name), name);
}


/////////////////////////////////////////////////////////////////////
//
// PrefColors
//
/////////////////////////////////////////////////////////////////////

namespace {

struct ColorSorter
{
	bool operator()(ColorCode lhs, ColorCode rhs) const {
		return 
			compare_no_case(lcolor.getGUIName(lhs), lcolor.getGUIName(rhs)) < 0;
	}
};

} // namespace anon

PrefColors::PrefColors(GuiPreferences * form)
	: PrefModule(qt_(catLookAndFeel), qt_("Colors"), form)
{
	setupUi(this);

	// FIXME: all of this initialization should be put into the controller.
	// See http://www.mail-archive.com/lyx-devel@lists.lyx.org/msg113301.html
	// for some discussion of why that is not trivial.
	QPixmap icon(32, 32);
	for (int i = 0; i < Color_ignore; ++i) {
		ColorCode lc = static_cast<ColorCode>(i);
		if (lc == Color_none
			|| lc == Color_black
			|| lc == Color_white
			|| lc == Color_red
			|| lc == Color_green
			|| lc == Color_blue
			|| lc == Color_cyan
			|| lc == Color_magenta
			|| lc == Color_yellow
			|| lc == Color_inherit
			|| lc == Color_ignore) continue;

		lcolors_.push_back(lc);
	}
	sort(lcolors_.begin(), lcolors_.end(), ColorSorter());
	vector<ColorCode>::const_iterator cit = lcolors_.begin();
	vector<ColorCode>::const_iterator const end = lcolors_.end();
	for (; cit != end; ++cit) {
			(void) new QListWidgetItem(QIcon(icon),
			toqstr(lcolor.getGUIName(*cit)), lyxObjectsLW);
	}
	curcolors_.resize(lcolors_.size());
	newcolors_.resize(lcolors_.size());
	// End initialization

	connect(colorChangePB, SIGNAL(clicked()),
		this, SLOT(change_color()));
	connect(lyxObjectsLW, SIGNAL(itemSelectionChanged()),
		this, SLOT(change_lyxObjects_selection()));
	connect(lyxObjectsLW, SIGNAL(itemActivated(QListWidgetItem*)),
		this, SLOT(change_color()));
}


void PrefColors::apply(LyXRC & /*rc*/) const
{
	for (unsigned int i = 0; i < lcolors_.size(); ++i)
		if (curcolors_[i] != newcolors_[i])
			form_->setColor(lcolors_[i], newcolors_[i]);
}


void PrefColors::update(LyXRC const & /*rc*/)
{
	for (unsigned int i = 0; i < lcolors_.size(); ++i) {
		QColor color = QColor(guiApp->colorCache().get(lcolors_[i]));
		QPixmap coloritem(32, 32);
		coloritem.fill(color);
		lyxObjectsLW->item(i)->setIcon(QIcon(coloritem));
		newcolors_[i] = curcolors_[i] = color.name();
	}
	change_lyxObjects_selection();
}


void PrefColors::change_color()
{
	int const row = lyxObjectsLW->currentRow();

	// just to be sure
	if (row < 0)
		return;

	QString const color = newcolors_[row];
	QColor c = QColorDialog::getColor(QColor(color), qApp->focusWidget());

	if (c.isValid() && c.name() != color) {
		newcolors_[row] = c.name();
		QPixmap coloritem(32, 32);
		coloritem.fill(c);
		lyxObjectsLW->currentItem()->setIcon(QIcon(coloritem));
		// emit signal
		changed();
	}
}

void PrefColors::change_lyxObjects_selection()
{
	colorChangePB->setDisabled(lyxObjectsLW->currentRow() < 0);
}


/////////////////////////////////////////////////////////////////////
//
// PrefDisplay
//
/////////////////////////////////////////////////////////////////////

PrefDisplay::PrefDisplay(GuiPreferences * form)
	: PrefModule(qt_(catLookAndFeel), qt_("Graphics"), form)
{
	setupUi(this);
	connect(displayGraphicsCB, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
	connect(instantPreviewCO, SIGNAL(activated(int)), this, SIGNAL(changed()));
}


void PrefDisplay::apply(LyXRC & rc) const
{
	switch (instantPreviewCO->currentIndex()) {
		case 0: rc.preview = LyXRC::PREVIEW_OFF; break;
		case 1:	rc.preview = LyXRC::PREVIEW_NO_MATH; break;
		case 2:	rc.preview = LyXRC::PREVIEW_ON;	break;
	}

	rc.display_graphics = displayGraphicsCB->isChecked();

	// FIXME!! The graphics cache no longer has a changeDisplay method.
#if 0
	if (old_value != rc.display_graphics) {
		graphics::GCache & gc = graphics::GCache::get();
		gc.changeDisplay();
	}
#endif
}


void PrefDisplay::update(LyXRC const & rc)
{
	switch (rc.preview) {
	case LyXRC::PREVIEW_OFF:
		instantPreviewCO->setCurrentIndex(0);
		break;
	case LyXRC::PREVIEW_NO_MATH :
		instantPreviewCO->setCurrentIndex(1);
		break;
	case LyXRC::PREVIEW_ON :
		instantPreviewCO->setCurrentIndex(2);
		break;
	}

	displayGraphicsCB->setChecked(rc.display_graphics);
	instantPreviewCO->setEnabled(rc.display_graphics);
}


/////////////////////////////////////////////////////////////////////
//
// PrefPaths
//
/////////////////////////////////////////////////////////////////////

PrefPaths::PrefPaths(GuiPreferences * form)
	: PrefModule(QString(), qt_("Paths"), form)
{
	setupUi(this);
	connect(exampleDirPB, SIGNAL(clicked()), this, SLOT(select_exampledir()));
	connect(templateDirPB, SIGNAL(clicked()), this, SLOT(select_templatedir()));
	connect(tempDirPB, SIGNAL(clicked()), this, SLOT(select_tempdir()));
	connect(backupDirPB, SIGNAL(clicked()), this, SLOT(select_backupdir()));
	connect(workingDirPB, SIGNAL(clicked()), this, SLOT(select_workingdir()));
	connect(lyxserverDirPB, SIGNAL(clicked()), this, SLOT(select_lyxpipe()));
	connect(thesaurusDirPB, SIGNAL(clicked()), this, SLOT(select_thesaurusdir()));
	connect(workingDirED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(exampleDirED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(templateDirED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(backupDirED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(tempDirED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(lyxserverDirED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(thesaurusDirED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(pathPrefixED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
}


void PrefPaths::apply(LyXRC & rc) const
{
	rc.document_path = internal_path(fromqstr(workingDirED->text()));
	rc.example_path = internal_path(fromqstr(exampleDirED->text()));
	rc.template_path = internal_path(fromqstr(templateDirED->text()));
	rc.backupdir_path = internal_path(fromqstr(backupDirED->text()));
	rc.tempdir_path = internal_path(fromqstr(tempDirED->text()));
	rc.thesaurusdir_path = internal_path(fromqstr(thesaurusDirED->text()));
	rc.path_prefix = internal_path_list(fromqstr(pathPrefixED->text()));
	// FIXME: should be a checkbox only
	rc.lyxpipes = internal_path(fromqstr(lyxserverDirED->text()));
}


void PrefPaths::update(LyXRC const & rc)
{
	workingDirED->setText(toqstr(external_path(rc.document_path)));
	exampleDirED->setText(toqstr(external_path(rc.example_path)));
	templateDirED->setText(toqstr(external_path(rc.template_path)));
	backupDirED->setText(toqstr(external_path(rc.backupdir_path)));
	tempDirED->setText(toqstr(external_path(rc.tempdir_path)));
	thesaurusDirED->setText(toqstr(external_path(rc.thesaurusdir_path)));
	pathPrefixED->setText(toqstr(external_path_list(rc.path_prefix)));
	// FIXME: should be a checkbox only
	lyxserverDirED->setText(toqstr(external_path(rc.lyxpipes)));
}


void PrefPaths::select_exampledir()
{
	QString file = browseDir(internalPath(exampleDirED->text()),
		qt_("Select directory for example files"));
	if (!file.isEmpty())
		exampleDirED->setText(file);
}


void PrefPaths::select_templatedir()
{
	QString file = browseDir(internalPath(templateDirED->text()),
		qt_("Select a document templates directory"));
	if (!file.isEmpty())
		templateDirED->setText(file);
}


void PrefPaths::select_tempdir()
{
	QString file = browseDir(internalPath(tempDirED->text()),
		qt_("Select a temporary directory"));
	if (!file.isEmpty())
		tempDirED->setText(file);
}


void PrefPaths::select_backupdir()
{
	QString file = browseDir(internalPath(backupDirED->text()),
		qt_("Select a backups directory"));
	if (!file.isEmpty())
		backupDirED->setText(file);
}


void PrefPaths::select_workingdir()
{
	QString file = browseDir(internalPath(workingDirED->text()),
		qt_("Select a document directory"));
	if (!file.isEmpty())
		workingDirED->setText(file);
}


void PrefPaths::select_thesaurusdir()
{
	QString file = browseDir(internalPath(thesaurusDirED->text()),
		qt_("Set the path to the thesaurus dictionaries"));
	if (!file.isEmpty())
		thesaurusDirED->setText(file);
}


void PrefPaths::select_lyxpipe()
{
	QString file = form_->browse(internalPath(lyxserverDirED->text()),
		qt_("Give a filename for the LyX server pipe"));
	if (!file.isEmpty())
		lyxserverDirED->setText(file);
}


/////////////////////////////////////////////////////////////////////
//
// PrefSpellchecker
//
/////////////////////////////////////////////////////////////////////

PrefSpellchecker::PrefSpellchecker(GuiPreferences * form)
	: PrefModule(qt_(catLanguage), qt_("Spellchecker"), form)
{
	setupUi(this);

	connect(persDictionaryPB, SIGNAL(clicked()), this, SLOT(select_dict()));

	connect(altLanguageED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(escapeCharactersED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(persDictionaryED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(compoundWordCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(inputEncodingCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(spellcheckContinuouslyCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
}


void PrefSpellchecker::apply(LyXRC & rc) const
{
	// FIXME: remove spellchecker_use_alt_lang
	rc.spellchecker_alt_lang = fromqstr(altLanguageED->text());
	rc.spellchecker_use_alt_lang = !rc.spellchecker_alt_lang.empty();
	// FIXME: remove spellchecker_use_esc_chars
	rc.spellchecker_esc_chars = fromqstr(escapeCharactersED->text());
	rc.spellchecker_use_esc_chars = !rc.spellchecker_esc_chars.empty();
	// FIXME: remove spellchecker_use_pers_dict
	rc.spellchecker_pers_dict = internal_path(fromqstr(persDictionaryED->text()));
	rc.spellchecker_use_pers_dict = !rc.spellchecker_pers_dict.empty();
	rc.spellchecker_accept_compound = compoundWordCB->isChecked();
	rc.spellchecker_use_input_encoding = inputEncodingCB->isChecked();
	rc.spellcheck_continuously = spellcheckContinuouslyCB->isChecked();
}


void PrefSpellchecker::update(LyXRC const & rc)
{
	// FIXME: remove spellchecker_use_alt_lang
	altLanguageED->setText(toqstr(rc.spellchecker_alt_lang));
	// FIXME: remove spellchecker_use_esc_chars
	escapeCharactersED->setText(toqstr(rc.spellchecker_esc_chars));
	// FIXME: remove spellchecker_use_pers_dict
	persDictionaryED->setText(toqstr(external_path(rc.spellchecker_pers_dict)));
	compoundWordCB->setChecked(rc.spellchecker_accept_compound);
	inputEncodingCB->setChecked(rc.spellchecker_use_input_encoding);
	spellcheckContinuouslyCB->setChecked(rc.spellcheck_continuously);
}


void PrefSpellchecker::select_dict()
{
	QString file = form_->browsedict(internalPath(persDictionaryED->text()));
	if (!file.isEmpty())
		persDictionaryED->setText(file);
}



/////////////////////////////////////////////////////////////////////
//
// PrefConverters
//
/////////////////////////////////////////////////////////////////////


PrefConverters::PrefConverters(GuiPreferences * form)
	: PrefModule(qt_(catFiles), qt_("Converters"), form)
{
	setupUi(this);

	connect(converterNewPB, SIGNAL(clicked()),
		this, SLOT(update_converter()));
	connect(converterRemovePB, SIGNAL(clicked()),
		this, SLOT(remove_converter()));
	connect(converterModifyPB, SIGNAL(clicked()),
		this, SLOT(update_converter()));
	connect(convertersLW, SIGNAL(currentRowChanged(int)),
		this, SLOT(switch_converter()));
	connect(converterFromCO, SIGNAL(activated(QString)),
		this, SLOT(converter_changed()));
	connect(converterToCO, SIGNAL(activated(QString)),
		this, SLOT(converter_changed()));
	connect(converterED, SIGNAL(textEdited(QString)),
		this, SLOT(converter_changed()));
	connect(converterFlagED, SIGNAL(textEdited(QString)),
		this, SLOT(converter_changed()));
	connect(converterNewPB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(converterRemovePB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(converterModifyPB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(maxAgeLE, SIGNAL(textEdited(QString)),
		this, SIGNAL(changed()));

	maxAgeLE->setValidator(new QDoubleValidator(maxAgeLE));
	//converterDefGB->setFocusProxy(convertersLW);
}


void PrefConverters::apply(LyXRC & rc) const
{
	rc.use_converter_cache = cacheCB->isChecked();
	rc.converter_cache_maxage = int(maxAgeLE->text().toDouble() * 86400.0);
}


void PrefConverters::update(LyXRC const & rc)
{
	cacheCB->setChecked(rc.use_converter_cache);
	QString max_age;
	max_age.setNum(double(rc.converter_cache_maxage) / 86400.0, 'g', 6);
	maxAgeLE->setText(max_age);
	updateGui();
}


void PrefConverters::updateGui()
{
	form_->formats().sort();
	form_->converters().update(form_->formats());
	// save current selection
	QString current = converterFromCO->currentText()
		+ " -> " + converterToCO->currentText();

	converterFromCO->clear();
	converterToCO->clear();

	Formats::const_iterator cit = form_->formats().begin();
	Formats::const_iterator end = form_->formats().end();
	for (; cit != end; ++cit) {
		converterFromCO->addItem(qt_(cit->prettyname()));
		converterToCO->addItem(qt_(cit->prettyname()));
	}

	// currentRowChanged(int) is also triggered when updating the listwidget
	// block signals to avoid unnecessary calls to switch_converter()
	convertersLW->blockSignals(true);
	convertersLW->clear();

	Converters::const_iterator ccit = form_->converters().begin();
	Converters::const_iterator cend = form_->converters().end();
	for (; ccit != cend; ++ccit) {
		QString const name =
			qt_(ccit->From->prettyname()) + " -> " + qt_(ccit->To->prettyname());
		int type = form_->converters().getNumber(ccit->From->name(), ccit->To->name());
		new QListWidgetItem(name, convertersLW, type);
	}
	convertersLW->sortItems(Qt::AscendingOrder);
	convertersLW->blockSignals(false);

	// restore selection
	if (!current.isEmpty()) {
		QList<QListWidgetItem *> const item =
			convertersLW->findItems(current, Qt::MatchExactly);
		if (!item.isEmpty())
			convertersLW->setCurrentItem(item.at(0));
	}

	// select first element if restoring failed
	if (convertersLW->currentRow() == -1)
		convertersLW->setCurrentRow(0);

	updateButtons();
}


void PrefConverters::switch_converter()
{
	int const cnr = convertersLW->currentItem()->type();
	Converter const & c(form_->converters().get(cnr));
	converterFromCO->setCurrentIndex(form_->formats().getNumber(c.from));
	converterToCO->setCurrentIndex(form_->formats().getNumber(c.to));
	converterED->setText(toqstr(c.command));
	converterFlagED->setText(toqstr(c.flags));

	updateButtons();
}


void PrefConverters::converter_changed()
{
	updateButtons();
}


void PrefConverters::updateButtons()
{
	Format const & from = form_->formats().get(converterFromCO->currentIndex());
	Format const & to = form_->formats().get(converterToCO->currentIndex());
	int const sel = form_->converters().getNumber(from.name(), to.name());
	bool const known = sel >= 0;
	bool const valid = !(converterED->text().isEmpty()
		|| from.name() == to.name());

	int const cnr = convertersLW->currentItem()->type();
	Converter const & c = form_->converters().get(cnr);
	string const old_command = c.command;
	string const old_flag = c.flags;
	string const new_command = fromqstr(converterED->text());
	string const new_flag = fromqstr(converterFlagED->text());

	bool modified = (old_command != new_command || old_flag != new_flag);

	converterModifyPB->setEnabled(valid && known && modified);
	converterNewPB->setEnabled(valid && !known);
	converterRemovePB->setEnabled(known);

	maxAgeLE->setEnabled(cacheCB->isChecked());
	maxAgeLA->setEnabled(cacheCB->isChecked());
}


// FIXME: user must
// specify unique from/to or it doesn't appear. This is really bad UI
// this is why we can use the same function for both new and modify
void PrefConverters::update_converter()
{
	Format const & from = form_->formats().get(converterFromCO->currentIndex());
	Format const & to = form_->formats().get(converterToCO->currentIndex());
	string const flags = fromqstr(converterFlagED->text());
	string const command = fromqstr(converterED->text());

	Converter const * old =
		form_->converters().getConverter(from.name(), to.name());
	form_->converters().add(from.name(), to.name(), command, flags);

	if (!old)
		form_->converters().updateLast(form_->formats());

	updateGui();

	// Remove all files created by this converter from the cache, since
	// the modified converter might create different files.
	ConverterCache::get().remove_all(from.name(), to.name());
}


void PrefConverters::remove_converter()
{
	Format const & from = form_->formats().get(converterFromCO->currentIndex());
	Format const & to = form_->formats().get(converterToCO->currentIndex());
	form_->converters().erase(from.name(), to.name());

	updateGui();

	// Remove all files created by this converter from the cache, since
	// a possible new converter might create different files.
	ConverterCache::get().remove_all(from.name(), to.name());
}


void PrefConverters::on_cacheCB_stateChanged(int state)
{
	maxAgeLE->setEnabled(state == Qt::Checked);
	maxAgeLA->setEnabled(state == Qt::Checked);
	changed();
}


/////////////////////////////////////////////////////////////////////
//
// FormatValidator
//
/////////////////////////////////////////////////////////////////////

class FormatValidator : public QValidator
{
public:
	FormatValidator(QWidget *, Formats const & f);
	void fixup(QString & input) const;
	QValidator::State validate(QString & input, int & pos) const;
private:
	virtual QString toString(Format const & format) const = 0;
	int nr() const;
	Formats const & formats_;
};


FormatValidator::FormatValidator(QWidget * parent, Formats const & f)
	: QValidator(parent), formats_(f)
{
}


void FormatValidator::fixup(QString & input) const
{
	Formats::const_iterator cit = formats_.begin();
	Formats::const_iterator end = formats_.end();
	for (; cit != end; ++cit) {
		QString const name = toString(*cit);
		if (distance(formats_.begin(), cit) == nr()) {
			input = name;
			return;
		}
	}
}


QValidator::State FormatValidator::validate(QString & input, int & /*pos*/) const
{
	Formats::const_iterator cit = formats_.begin();
	Formats::const_iterator end = formats_.end();
	bool unknown = true;
	for (; unknown && cit != end; ++cit) {
		QString const name = toString(*cit);
		if (distance(formats_.begin(), cit) != nr())
			unknown = name != input;
	}

	if (unknown && !input.isEmpty())
		return QValidator::Acceptable;
	else
		return QValidator::Intermediate;
}


int FormatValidator::nr() const
{
	QComboBox * p = qobject_cast<QComboBox *>(parent());
	return p->itemData(p->currentIndex()).toInt();
}


/////////////////////////////////////////////////////////////////////
//
// FormatNameValidator
//
/////////////////////////////////////////////////////////////////////

class FormatNameValidator : public FormatValidator
{
public:
	FormatNameValidator(QWidget * parent, Formats const & f)
		: FormatValidator(parent, f)
	{}
private:
	QString toString(Format const & format) const
	{
		return toqstr(format.name());
	}
};


/////////////////////////////////////////////////////////////////////
//
// FormatPrettynameValidator
//
/////////////////////////////////////////////////////////////////////

class FormatPrettynameValidator : public FormatValidator
{
public:
	FormatPrettynameValidator(QWidget * parent, Formats const & f)
		: FormatValidator(parent, f)
	{}
private:
	QString toString(Format const & format) const
	{
		return qt_(format.prettyname());
	}
};


/////////////////////////////////////////////////////////////////////
//
// PrefFileformats
//
/////////////////////////////////////////////////////////////////////

PrefFileformats::PrefFileformats(GuiPreferences * form)
	: PrefModule(qt_(catFiles), qt_("File formats"), form)
{
	setupUi(this);
	formatED->setValidator(new FormatNameValidator(formatsCB, form_->formats()));
	formatsCB->setValidator(new FormatPrettynameValidator(formatsCB, form_->formats()));

	connect(documentCB, SIGNAL(clicked()),
		this, SLOT(setFlags()));
	connect(vectorCB, SIGNAL(clicked()),
		this, SLOT(setFlags()));
	connect(formatsCB->lineEdit(), SIGNAL(editingFinished()),
		this, SLOT(updatePrettyname()));
	connect(formatsCB->lineEdit(), SIGNAL(textEdited(QString)),
		this, SIGNAL(changed()));
	connect(defaultFormatCB, SIGNAL(activated(QString)),
		this, SIGNAL(changed()));
}


namespace {

string const l10n_shortcut(string const prettyname, string const shortcut)
{
	if (shortcut.empty())
		return string();

	string l10n_format =
		to_utf8(_(prettyname + '|' + shortcut));
	return split(l10n_format, '|');
}

}; // namespace anon


void PrefFileformats::apply(LyXRC & rc) const
{
	QString const default_format = defaultFormatCB->itemData(
		defaultFormatCB->currentIndex()).toString();
	rc.default_view_format = fromqstr(default_format);
}


void PrefFileformats::update(LyXRC const & rc)
{
	bool const init = defaultFormatCB->currentText().isEmpty();
	updateView();
	if (init) {
		int const pos = defaultFormatCB->findData(toqstr(
		rc.default_view_format));
		defaultFormatCB->setCurrentIndex(pos);
	}
}


void PrefFileformats::updateView()
{
	QString const current = formatsCB->currentText();
	QString const current_def = defaultFormatCB->currentText();

	// update comboboxes with formats
	formatsCB->blockSignals(true);
	defaultFormatCB->blockSignals(true);
	formatsCB->clear();
	defaultFormatCB->clear();
	form_->formats().sort();
	Formats::const_iterator cit = form_->formats().begin();
	Formats::const_iterator end = form_->formats().end();
	for (; cit != end; ++cit) {
		formatsCB->addItem(qt_(cit->prettyname()),
				QVariant(form_->formats().getNumber(cit->name())));
		if (form_->converters().isReachable("latex", cit->name())
		    || form_->converters().isReachable("pdflatex", cit->name()))
			defaultFormatCB->addItem(qt_(cit->prettyname()),
					QVariant(toqstr(cit->name())));
	}

	// restore selection
	int item = formatsCB->findText(current, Qt::MatchExactly);
	formatsCB->setCurrentIndex(item < 0 ? 0 : item);
	on_formatsCB_currentIndexChanged(item < 0 ? 0 : item);
	item = defaultFormatCB->findText(current_def, Qt::MatchExactly);
	defaultFormatCB->setCurrentIndex(item < 0 ? 0 : item);
	formatsCB->blockSignals(false);
	defaultFormatCB->blockSignals(false);
}


void PrefFileformats::on_formatsCB_currentIndexChanged(int i)
{
	int const nr = formatsCB->itemData(i).toInt();
	Format const f = form_->formats().get(nr);

	formatED->setText(toqstr(f.name()));
	copierED->setText(toqstr(form_->movers().command(f.name())));
	extensionED->setText(toqstr(f.extension()));
	shortcutED->setText(
		toqstr(l10n_shortcut(f.prettyname(), f.shortcut())));
	viewerED->setText(toqstr(f.viewer()));
	editorED->setText(toqstr(f.editor()));
	documentCB->setChecked((f.documentFormat()));
	vectorCB->setChecked((f.vectorFormat()));
}


void PrefFileformats::setFlags()
{
	int flags = Format::none;
	if (documentCB->isChecked())
		flags |= Format::document;
	if (vectorCB->isChecked())
		flags |= Format::vector;
	currentFormat().setFlags(flags);
	changed();
}


void PrefFileformats::on_copierED_textEdited(const QString & s)
{
	string const fmt = fromqstr(formatED->text());
	form_->movers().set(fmt, fromqstr(s));
	changed();
}


void PrefFileformats::on_extensionED_textEdited(const QString & s)
{
	currentFormat().setExtension(fromqstr(s));
	changed();
}

void PrefFileformats::on_viewerED_textEdited(const QString & s)
{
	currentFormat().setViewer(fromqstr(s));
	changed();
}


void PrefFileformats::on_editorED_textEdited(const QString & s)
{
	currentFormat().setEditor(fromqstr(s));
	changed();
}


void PrefFileformats::on_shortcutED_textEdited(const QString & s)
{
	string const new_shortcut = fromqstr(s);
	if (new_shortcut == l10n_shortcut(currentFormat().prettyname(),
					  currentFormat().shortcut()))
		return;
	currentFormat().setShortcut(new_shortcut);
	changed();
}


void PrefFileformats::on_formatED_editingFinished()
{
	string const newname = fromqstr(formatED->displayText());
	if (newname == currentFormat().name())
		return;

	currentFormat().setName(newname);
	changed();
}


void PrefFileformats::on_formatED_textChanged(const QString &)
{
	QString t = formatED->text();
	int p = 0;
	bool valid = formatED->validator()->validate(t, p) == QValidator::Acceptable;
	setValid(formatLA, valid);
}


void PrefFileformats::on_formatsCB_editTextChanged(const QString &)
{
	QString t = formatsCB->currentText();
	int p = 0;
	bool valid = formatsCB->validator()->validate(t, p) == QValidator::Acceptable;
	setValid(formatsLA, valid);
}


void PrefFileformats::updatePrettyname()
{
	QString const newname = formatsCB->currentText();
	if (newname == qt_(currentFormat().prettyname()))
		return;

	currentFormat().setPrettyname(fromqstr(newname));
	formatsChanged();
	updateView();
	changed();
}


Format & PrefFileformats::currentFormat()
{
	int const i = formatsCB->currentIndex();
	int const nr = formatsCB->itemData(i).toInt();
	return form_->formats().get(nr);
}


void PrefFileformats::on_formatNewPB_clicked()
{
	form_->formats().add("", "", "", "", "", "", Format::none);
	updateView();
	formatsCB->setCurrentIndex(0);
	formatsCB->setFocus(Qt::OtherFocusReason);
}


void PrefFileformats::on_formatRemovePB_clicked()
{
	int const i = formatsCB->currentIndex();
	int const nr = formatsCB->itemData(i).toInt();
	string const current_text = form_->formats().get(nr).name();
	if (form_->converters().formatIsUsed(current_text)) {
		Alert::error(_("Format in use"),
			     _("Cannot remove a Format used by a Converter. "
					    "Remove the converter first."));
		return;
	}

	form_->formats().erase(current_text);
	formatsChanged();
	updateView();
	on_formatsCB_editTextChanged(formatsCB->currentText());
	changed();
}


/////////////////////////////////////////////////////////////////////
//
// PrefLanguage
//
/////////////////////////////////////////////////////////////////////

PrefLanguage::PrefLanguage(GuiPreferences * form)
	: PrefModule(qt_(catLanguage), qt_("Language"), form)
{
	setupUi(this);

	connect(rtlGB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(visualCursorRB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(logicalCursorRB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(markForeignCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(autoBeginCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(autoEndCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(useBabelCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(globalCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(languagePackageED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(startCommandED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(endCommandED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(uiLanguageCO, SIGNAL(activated(int)),
		this, SIGNAL(changed()));

	uiLanguageCO->clear();

	QAbstractItemModel * language_model = guiApp->languageModel();
	// FIXME: it would be nice if sorting was enabled/disabled via a checkbox.
	language_model->sort(0);

	// FIXME: This is wrong, we need filter this list based on the available
	// translation.
	uiLanguageCO->blockSignals(true);
	uiLanguageCO->addItem(qt_("Default"), toqstr("auto"));
	for (int i = 0; i != language_model->rowCount(); ++i) {
		QModelIndex index = language_model->index(i, 0);
		uiLanguageCO->addItem(index.data(Qt::DisplayRole).toString(),
			index.data(Qt::UserRole).toString());
	}
	uiLanguageCO->blockSignals(false);
}


void PrefLanguage::on_uiLanguageCO_currentIndexChanged(int)
{
	 QMessageBox::information(this, qt_("LyX needs to be restarted!"),
		 qt_("The change of user interface language will be fully "
		 "effective only after a restart."));
}


void PrefLanguage::apply(LyXRC & rc) const
{
	// FIXME: remove rtl_support bool
	rc.rtl_support = rtlGB->isChecked();
	rc.visual_cursor = rtlGB->isChecked() && visualCursorRB->isChecked();
	rc.mark_foreign_language = markForeignCB->isChecked();
	rc.language_auto_begin = autoBeginCB->isChecked();
	rc.language_auto_end = autoEndCB->isChecked();
	rc.language_use_babel = useBabelCB->isChecked();
	rc.language_global_options = globalCB->isChecked();
	rc.language_package = fromqstr(languagePackageED->text());
	rc.language_command_begin = fromqstr(startCommandED->text());
	rc.language_command_end = fromqstr(endCommandED->text());
	rc.gui_language = fromqstr(
		uiLanguageCO->itemData(uiLanguageCO->currentIndex()).toString());
}


void PrefLanguage::update(LyXRC const & rc)
{
	// FIXME: remove rtl_support bool
	rtlGB->setChecked(rc.rtl_support);
	if (rc.visual_cursor)
		visualCursorRB->setChecked(true);
	else
		logicalCursorRB->setChecked(true);
	markForeignCB->setChecked(rc.mark_foreign_language);
	autoBeginCB->setChecked(rc.language_auto_begin);
	autoEndCB->setChecked(rc.language_auto_end);
	useBabelCB->setChecked(rc.language_use_babel);
	globalCB->setChecked(rc.language_global_options);
	languagePackageED->setText(toqstr(rc.language_package));
	startCommandED->setText(toqstr(rc.language_command_begin));
	endCommandED->setText(toqstr(rc.language_command_end));

	int pos = uiLanguageCO->findData(toqstr(rc.gui_language));
	uiLanguageCO->blockSignals(true);
	uiLanguageCO->setCurrentIndex(pos);
	uiLanguageCO->blockSignals(false);
}


/////////////////////////////////////////////////////////////////////
//
// PrefPrinter
//
/////////////////////////////////////////////////////////////////////

PrefPrinter::PrefPrinter(GuiPreferences * form)
	: PrefModule(qt_(catOutput), qt_("Printer"), form)
{
	setupUi(this);

	connect(printerAdaptCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(printerCommandED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(printerNameED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(printerPageRangeED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(printerCopiesED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(printerReverseED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(printerToPrinterED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(printerExtensionED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(printerSpoolCommandED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(printerPaperTypeED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(printerEvenED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(printerOddED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(printerCollatedED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(printerLandscapeED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(printerToFileED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(printerExtraED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(printerSpoolPrefixED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(printerPaperSizeED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
}


void PrefPrinter::apply(LyXRC & rc) const
{
	rc.print_adapt_output = printerAdaptCB->isChecked();
	rc.print_command = fromqstr(printerCommandED->text());
	rc.printer = fromqstr(printerNameED->text());

	rc.print_pagerange_flag = fromqstr(printerPageRangeED->text());
	rc.print_copies_flag = fromqstr(printerCopiesED->text());
	rc.print_reverse_flag = fromqstr(printerReverseED->text());
	rc.print_to_printer = fromqstr(printerToPrinterED->text());
	rc.print_file_extension = fromqstr(printerExtensionED->text());
	rc.print_spool_command = fromqstr(printerSpoolCommandED->text());
	rc.print_paper_flag = fromqstr(printerPaperTypeED->text());
	rc.print_evenpage_flag = fromqstr(printerEvenED->text());
	rc.print_oddpage_flag = fromqstr(printerOddED->text());
	rc.print_collcopies_flag = fromqstr(printerCollatedED->text());
	rc.print_landscape_flag = fromqstr(printerLandscapeED->text());
	rc.print_to_file = internal_path(fromqstr(printerToFileED->text()));
	rc.print_extra_options = fromqstr(printerExtraED->text());
	rc.print_spool_printerprefix = fromqstr(printerSpoolPrefixED->text());
	rc.print_paper_dimension_flag = fromqstr(printerPaperSizeED->text());
}


void PrefPrinter::update(LyXRC const & rc)
{
	printerAdaptCB->setChecked(rc.print_adapt_output);
	printerCommandED->setText(toqstr(rc.print_command));
	printerNameED->setText(toqstr(rc.printer));

	printerPageRangeED->setText(toqstr(rc.print_pagerange_flag));
	printerCopiesED->setText(toqstr(rc.print_copies_flag));
	printerReverseED->setText(toqstr(rc.print_reverse_flag));
	printerToPrinterED->setText(toqstr(rc.print_to_printer));
	printerExtensionED->setText(toqstr(rc.print_file_extension));
	printerSpoolCommandED->setText(toqstr(rc.print_spool_command));
	printerPaperTypeED->setText(toqstr(rc.print_paper_flag));
	printerEvenED->setText(toqstr(rc.print_evenpage_flag));
	printerOddED->setText(toqstr(rc.print_oddpage_flag));
	printerCollatedED->setText(toqstr(rc.print_collcopies_flag));
	printerLandscapeED->setText(toqstr(rc.print_landscape_flag));
	printerToFileED->setText(toqstr(external_path(rc.print_to_file)));
	printerExtraED->setText(toqstr(rc.print_extra_options));
	printerSpoolPrefixED->setText(toqstr(rc.print_spool_printerprefix));
	printerPaperSizeED->setText(toqstr(rc.print_paper_dimension_flag));
}


/////////////////////////////////////////////////////////////////////
//
// PrefUserInterface
//
/////////////////////////////////////////////////////////////////////

PrefUserInterface::PrefUserInterface(GuiPreferences * form)
	: PrefModule(qt_(catLookAndFeel), qt_("User interface"), form)
{
	setupUi(this);

	connect(autoSaveCB, SIGNAL(toggled(bool)),
		autoSaveSB, SLOT(setEnabled(bool)));
	connect(autoSaveCB, SIGNAL(toggled(bool)),
		TextLabel1, SLOT(setEnabled(bool)));
	connect(openDocumentsInTabsCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(uiFilePB, SIGNAL(clicked()),
		this, SLOT(select_ui()));
	connect(uiFileED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(restoreCursorCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(loadSessionCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(allowGeometrySessionCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(autoSaveSB, SIGNAL(valueChanged(int)),
		this, SIGNAL(changed()));
	connect(autoSaveCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(lastfilesSB, SIGNAL(valueChanged(int)),
		this, SIGNAL(changed()));
	connect(tooltipCB, SIGNAL(toggled(bool)),
		this, SIGNAL(changed()));
	lastfilesSB->setMaximum(maxlastfiles);
}


void PrefUserInterface::apply(LyXRC & rc) const
{
	rc.ui_file = internal_path(fromqstr(uiFileED->text()));
	rc.use_lastfilepos = restoreCursorCB->isChecked();
	rc.load_session = loadSessionCB->isChecked();
	rc.allow_geometry_session = allowGeometrySessionCB->isChecked();
	rc.autosave = autoSaveSB->value() * 60;
	rc.make_backup = autoSaveCB->isChecked();
	rc.num_lastfiles = lastfilesSB->value();
	rc.use_tooltip = tooltipCB->isChecked();
	rc.open_buffers_in_tabs = openDocumentsInTabsCB->isChecked();
}


void PrefUserInterface::update(LyXRC const & rc)
{
	uiFileED->setText(toqstr(external_path(rc.ui_file)));
	restoreCursorCB->setChecked(rc.use_lastfilepos);
	loadSessionCB->setChecked(rc.load_session);
	allowGeometrySessionCB->setChecked(rc.allow_geometry_session);
	// convert to minutes
	int mins(rc.autosave / 60);
	if (rc.autosave && !mins)
		mins = 1;
	autoSaveSB->setValue(mins);
	autoSaveCB->setChecked(rc.make_backup);
	lastfilesSB->setValue(rc.num_lastfiles);
	tooltipCB->setChecked(rc.use_tooltip);
	openDocumentsInTabsCB->setChecked(rc.open_buffers_in_tabs);
}


void PrefUserInterface::select_ui()
{
	QString file = form_->browseUI(internalPath(uiFileED->text()));
	if (!file.isEmpty())
		uiFileED->setText(file);
}


void PrefUserInterface::on_clearSessionPB_clicked()
{
	guiApp->clearSession();
}



/////////////////////////////////////////////////////////////////////
//
// PrefEdit
//
/////////////////////////////////////////////////////////////////////

PrefEdit::PrefEdit(GuiPreferences * form)
	: PrefModule(qt_(catEditing), qt_("Control"), form)
{
	setupUi(this);

	connect(cursorFollowsCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(scrollBelowCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(sortEnvironmentsCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(groupEnvironmentsCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(macroEditStyleCO, SIGNAL(activated(int)),
		this, SIGNAL(changed()));
	connect(fullscreenLimitGB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(fullscreenWidthSB, SIGNAL(valueChanged(int)),
		this, SIGNAL(changed()));
	connect(toggleTabbarCB, SIGNAL(toggled(bool)),
		this, SIGNAL(changed()));
	connect(toggleScrollbarCB, SIGNAL(toggled(bool)),
		this, SIGNAL(changed()));
	connect(toggleToolbarsCB, SIGNAL(toggled(bool)),
		this, SIGNAL(changed()));
}


void PrefEdit::apply(LyXRC & rc) const
{
	rc.cursor_follows_scrollbar = cursorFollowsCB->isChecked();
	rc.scroll_below_document = scrollBelowCB->isChecked();
	rc.sort_layouts = sortEnvironmentsCB->isChecked();
	rc.group_layouts = groupEnvironmentsCB->isChecked();
	switch (macroEditStyleCO->currentIndex()) {
		case 0: rc.macro_edit_style = LyXRC::MACRO_EDIT_INLINE_BOX; break;
		case 1:	rc.macro_edit_style = LyXRC::MACRO_EDIT_INLINE; break;
		case 2:	rc.macro_edit_style = LyXRC::MACRO_EDIT_LIST;	break;
	}
	rc.full_screen_toolbars = toggleToolbarsCB->isChecked();
	rc.full_screen_scrollbar = toggleScrollbarCB->isChecked();
	rc.full_screen_tabbar = toggleTabbarCB->isChecked();
	rc.full_screen_width = fullscreenWidthSB->value();
	rc.full_screen_limit = fullscreenLimitGB->isChecked();
}


void PrefEdit::update(LyXRC const & rc)
{
	cursorFollowsCB->setChecked(rc.cursor_follows_scrollbar);
	scrollBelowCB->setChecked(rc.scroll_below_document);
	sortEnvironmentsCB->setChecked(rc.sort_layouts);
	groupEnvironmentsCB->setChecked(rc.group_layouts);
	macroEditStyleCO->setCurrentIndex(rc.macro_edit_style);
	toggleScrollbarCB->setChecked(rc.full_screen_scrollbar);
	toggleToolbarsCB->setChecked(rc.full_screen_toolbars);
	toggleTabbarCB->setChecked(rc.full_screen_tabbar);
	fullscreenWidthSB->setValue(rc.full_screen_width);
	fullscreenLimitGB->setChecked(rc.full_screen_limit);
}


/////////////////////////////////////////////////////////////////////
//
// PrefShortcuts
//
/////////////////////////////////////////////////////////////////////


GuiShortcutDialog::GuiShortcutDialog(QWidget * parent) : QDialog(parent)
{
	Ui::shortcutUi::setupUi(this);
	QDialog::setModal(true);
}


PrefShortcuts::PrefShortcuts(GuiPreferences * form)
	: PrefModule(qt_(catEditing), qt_("Shortcuts"), form)
{
	setupUi(this);

	shortcutsTW->setColumnCount(2);
	shortcutsTW->headerItem()->setText(0, qt_("Function"));
	shortcutsTW->headerItem()->setText(1, qt_("Shortcut"));
	shortcutsTW->setSortingEnabled(true);
	// Multi-selection can be annoying.
	// shortcutsTW->setSelectionMode(QAbstractItemView::MultiSelection);

	connect(bindFilePB, SIGNAL(clicked()),
		this, SLOT(select_bind()));
	connect(bindFileED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(removePB, SIGNAL(clicked()),
		this, SIGNAL(changed()));

	shortcut_ = new GuiShortcutDialog(this);
	shortcut_bc_.setPolicy(ButtonPolicy::OkCancelPolicy);
	shortcut_bc_.setOK(shortcut_->okPB);
	shortcut_bc_.setCancel(shortcut_->cancelPB);

	connect(shortcut_->okPB, SIGNAL(clicked()),
		shortcut_, SLOT(accept()));
	connect(shortcut_->okPB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(shortcut_->cancelPB, SIGNAL(clicked()),
		shortcut_, SLOT(reject()));
	connect(shortcut_->clearPB, SIGNAL(clicked()),
		this, SLOT(shortcut_clearPB_pressed()));
	connect(shortcut_->removePB, SIGNAL(clicked()),
		this, SLOT(shortcut_removePB_pressed()));
	connect(shortcut_->okPB, SIGNAL(clicked()),
		this, SLOT(shortcut_okPB_pressed()));
	connect(shortcut_->cancelPB, SIGNAL(clicked()),
		this, SLOT(shortcut_cancelPB_pressed()));
}


void PrefShortcuts::apply(LyXRC & rc) const
{
	rc.bind_file = internal_path(fromqstr(bindFileED->text()));
	// write user_bind and user_unbind to .lyx/bind/user.bind
	FileName bind_dir(addPath(package().user_support().absFilename(), "bind"));
	if (!bind_dir.exists() && !bind_dir.createDirectory(0777)) {
		lyxerr << "LyX could not create the user bind directory '"
		       << bind_dir << "'. All user-defined key bindings will be lost." << endl;
		return;
	}
	if (!bind_dir.isDirWritable()) {
		lyxerr << "LyX could not write to the user bind directory '"
		       << bind_dir << "'. All user-defined key bindings will be lost." << endl;
		return;
	}
	FileName user_bind_file(bind_dir.absFilename() + "/user.bind");
	user_unbind_.write(user_bind_file.toFilesystemEncoding(), false, true);
	user_bind_.write(user_bind_file.toFilesystemEncoding(), true, false);
	// immediately apply the keybindings. Why this is not done before?
	// The good thing is that the menus are updated automatically.
	theTopLevelKeymap().clear();
	theTopLevelKeymap().read("site");
	theTopLevelKeymap().read(rc.bind_file);
	theTopLevelKeymap().read("user");
}


void PrefShortcuts::update(LyXRC const & rc)
{
	bindFileED->setText(toqstr(external_path(rc.bind_file)));
	//
	system_bind_.clear();
	user_bind_.clear();
	user_unbind_.clear();
	system_bind_.read("site");
	system_bind_.read(rc.bind_file);
	// \unbind in user.bind is added to user_unbind_
	user_bind_.read("user", &user_unbind_);
	updateShortcutsTW();
}


void PrefShortcuts::updateShortcutsTW()
{
	shortcutsTW->clear();

	editItem_ = new QTreeWidgetItem(shortcutsTW);
	editItem_->setText(0, qt_("Cursor, Mouse and Editing functions"));
	editItem_->setFlags(editItem_->flags() & ~Qt::ItemIsSelectable);

	mathItem_ = new QTreeWidgetItem(shortcutsTW);
	mathItem_->setText(0, qt_("Mathematical Symbols"));
	mathItem_->setFlags(mathItem_->flags() & ~Qt::ItemIsSelectable);

	bufferItem_ = new QTreeWidgetItem(shortcutsTW);
	bufferItem_->setText(0, qt_("Document and Window"));
	bufferItem_->setFlags(bufferItem_->flags() & ~Qt::ItemIsSelectable);

	layoutItem_ = new QTreeWidgetItem(shortcutsTW);
	layoutItem_->setText(0, qt_("Font, Layouts and Textclasses"));
	layoutItem_->setFlags(layoutItem_->flags() & ~Qt::ItemIsSelectable);

	systemItem_ = new QTreeWidgetItem(shortcutsTW);
	systemItem_->setText(0, qt_("System and Miscellaneous"));
	systemItem_->setFlags(systemItem_->flags() & ~Qt::ItemIsSelectable);

	// listBindings(unbound=true) lists all bound and unbound lfuns
	// Items in this list is tagged by its source.
	KeyMap::BindingList bindinglist = system_bind_.listBindings(true,
		KeyMap::System);
	KeyMap::BindingList user_bindinglist = user_bind_.listBindings(false,
		KeyMap::UserBind);
	KeyMap::BindingList user_unbindinglist = user_unbind_.listBindings(false,
		KeyMap::UserUnbind);
	bindinglist.insert(bindinglist.end(), user_bindinglist.begin(),
			user_bindinglist.end());
	bindinglist.insert(bindinglist.end(), user_unbindinglist.begin(),
			user_unbindinglist.end());

	KeyMap::BindingList::const_iterator it = bindinglist.begin();
	KeyMap::BindingList::const_iterator it_end = bindinglist.end();
	for (; it != it_end; ++it)
		insertShortcutItem(it->request, it->sequence, KeyMap::ItemType(it->tag));

	shortcutsTW->sortItems(0, Qt::AscendingOrder);
	QList<QTreeWidgetItem*> items = shortcutsTW->selectedItems();
	removePB->setEnabled(!items.isEmpty() && !items[0]->text(1).isEmpty());
	modifyPB->setEnabled(!items.isEmpty());

	shortcutsTW->resizeColumnToContents(0);
}


void PrefShortcuts::setItemType(QTreeWidgetItem * item, KeyMap::ItemType tag)
{
	item->setData(0, Qt::UserRole, QVariant(tag));
	QFont font;

	switch (tag) {
	case KeyMap::System:
		break;
	case KeyMap::UserBind:
		font.setBold(true);
		break;
	case KeyMap::UserUnbind:
		font.setStrikeOut(true);
		break;
	// this item is not displayed now.
	case KeyMap::UserExtraUnbind:
		font.setStrikeOut(true);
		break;
	}

	item->setFont(1, font);
}


QTreeWidgetItem * PrefShortcuts::insertShortcutItem(FuncRequest const & lfun,
		KeySequence const & seq, KeyMap::ItemType tag)
{
	FuncCode action = lfun.action;
	string const action_name = lyxaction.getActionName(action);
	QString const lfun_name = toqstr(from_utf8(action_name)
			+ ' ' + lfun.argument());
	QString const shortcut = toqstr(seq.print(KeySequence::ForGui));
	KeyMap::ItemType item_tag = tag;

	QTreeWidgetItem * newItem = 0;
	// for unbind items, try to find an existing item in the system bind list
	if (tag == KeyMap::UserUnbind) {
		QList<QTreeWidgetItem*> const items = shortcutsTW->findItems(lfun_name,
			Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive), 0);
		for (int i = 0; i < items.size(); ++i) {
			if (items[i]->text(1) == shortcut)
				newItem = items[i];
				break;
			}
		// if not found, this unbind item is KeyMap::UserExtraUnbind
		// Such an item is not displayed to avoid confusion (what is
		// unmatched removed?).
		if (!newItem) {
			item_tag = KeyMap::UserExtraUnbind;
			return 0;
		}
	}
	if (!newItem) {
		switch(lyxaction.getActionType(action)) {
		case LyXAction::Hidden:
			return 0;
		case LyXAction::Edit:
			newItem = new QTreeWidgetItem(editItem_);
			break;
		case LyXAction::Math:
			newItem = new QTreeWidgetItem(mathItem_);
			break;
		case LyXAction::Buffer:
			newItem = new QTreeWidgetItem(bufferItem_);
			break;
		case LyXAction::Layout:
			newItem = new QTreeWidgetItem(layoutItem_);
			break;
		case LyXAction::System:
			newItem = new QTreeWidgetItem(systemItem_);
			break;
		default:
			// this should not happen
			newItem = new QTreeWidgetItem(shortcutsTW);
		}
	}

	newItem->setText(0, lfun_name);
	newItem->setText(1, shortcut);
	// record BindFile representation to recover KeySequence when needed.
	newItem->setData(1, Qt::UserRole, toqstr(seq.print(KeySequence::BindFile)));
	setItemType(newItem, item_tag);
	return newItem;
}


void PrefShortcuts::on_shortcutsTW_itemSelectionChanged()
{
	QList<QTreeWidgetItem*> items = shortcutsTW->selectedItems();
	removePB->setEnabled(!items.isEmpty() && !items[0]->text(1).isEmpty());
	modifyPB->setEnabled(!items.isEmpty());
	if (items.isEmpty())
		return;

	KeyMap::ItemType tag = 
		static_cast<KeyMap::ItemType>(items[0]->data(0, Qt::UserRole).toInt());
	if (tag == KeyMap::UserUnbind)
		removePB->setText(qt_("Res&tore"));
	else
		removePB->setText(qt_("Remo&ve"));
}


void PrefShortcuts::on_shortcutsTW_itemDoubleClicked()
{
	modifyShortcut();
}


void PrefShortcuts::modifyShortcut()
{
	QTreeWidgetItem * item = shortcutsTW->currentItem();
	if (item->flags() & Qt::ItemIsSelectable) {
		shortcut_->lfunLE->setText(item->text(0));
		save_lfun_ = item->text(0);
		shortcut_->shortcutWG->setText(item->text(1));
		KeySequence seq;
		seq.parse(fromqstr(item->data(1, Qt::UserRole).toString()));
		shortcut_->shortcutWG->setKeySequence(seq);
		shortcut_->shortcutWG->setFocus();
		shortcut_->exec();
	}
}


void PrefShortcuts::removeShortcut()
{
	// it seems that only one item can be selected, but I am
	// removing all selected items anyway.
	QList<QTreeWidgetItem*> items = shortcutsTW->selectedItems();
	for (int i = 0; i < items.size(); ++i) {
		string shortcut = fromqstr(items[i]->data(1, Qt::UserRole).toString());
		string lfun = fromqstr(items[i]->text(0));
		FuncRequest func = lyxaction.lookupFunc(lfun);
		KeyMap::ItemType tag = 
			static_cast<KeyMap::ItemType>(items[i]->data(0, Qt::UserRole).toInt());

		switch (tag) {
		case KeyMap::System: {
			// for system bind, we do not touch the item
			// but add an user unbind item
			user_unbind_.bind(shortcut, func);
			setItemType(items[i], KeyMap::UserUnbind);
			removePB->setText(qt_("Res&tore"));
			break;
		}
		case KeyMap::UserBind: {
			// for user_bind, we remove this bind
			QTreeWidgetItem * parent = items[i]->parent();
			int itemIdx = parent->indexOfChild(items[i]);
			parent->takeChild(itemIdx);
			if (itemIdx > 0)
				shortcutsTW->scrollToItem(parent->child(itemIdx - 1));
			else
				shortcutsTW->scrollToItem(parent);
			user_bind_.unbind(shortcut, func);
			break;
		}
		case KeyMap::UserUnbind: {
			// for user_unbind, we remove the unbind, and the item
			// become KeyMap::System again.
			user_unbind_.unbind(shortcut, func);
			setItemType(items[i], KeyMap::System);
			removePB->setText(qt_("Remo&ve"));
			break;
		}
		case KeyMap::UserExtraUnbind: {
			// for user unbind that is not in system bind file,
			// remove this unbind file
			QTreeWidgetItem * parent = items[i]->parent();
			parent->takeChild(parent->indexOfChild(items[i]));
			user_unbind_.unbind(shortcut, func);
		}
		}
	}
}


void PrefShortcuts::select_bind()
{
	QString file = form_->browsebind(internalPath(bindFileED->text()));
	if (!file.isEmpty()) {
		bindFileED->setText(file);
		system_bind_ = KeyMap();
		system_bind_.read(fromqstr(file));
		updateShortcutsTW();
	}
}


void PrefShortcuts::on_modifyPB_pressed()
{
	modifyShortcut();
}


void PrefShortcuts::on_newPB_pressed()
{
	shortcut_->lfunLE->clear();
	shortcut_->shortcutWG->reset();
	save_lfun_ = QString();
	shortcut_->exec();
}


void PrefShortcuts::on_removePB_pressed()
{
	removeShortcut();
}


void PrefShortcuts::on_searchLE_textEdited()
{
	if (searchLE->text().isEmpty()) {
		// show all hidden items
		QTreeWidgetItemIterator it(shortcutsTW, QTreeWidgetItemIterator::Hidden);
		while (*it)
			shortcutsTW->setItemHidden(*it++, false);
		return;
	}
	// search both columns
	QList<QTreeWidgetItem *> matched = shortcutsTW->findItems(searchLE->text(),
		Qt::MatchFlags(Qt::MatchContains | Qt::MatchRecursive), 0);
	matched += shortcutsTW->findItems(searchLE->text(),
		Qt::MatchFlags(Qt::MatchContains | Qt::MatchRecursive), 1);

	// hide everyone (to avoid searching in matched QList repeatedly
	QTreeWidgetItemIterator it(shortcutsTW, QTreeWidgetItemIterator::Selectable);
	while (*it)
		shortcutsTW->setItemHidden(*it++, true);
	// show matched items
	for (int i = 0; i < matched.size(); ++i) {
		shortcutsTW->setItemHidden(matched[i], false);
        shortcutsTW->setItemExpanded(matched[i]->parent(), true);
	}
}


docstring makeCmdString(FuncRequest const & f)
{
	docstring actionStr = from_ascii(lyxaction.getActionName(f.action));
	if (!f.argument().empty())
		actionStr += " " + f.argument();
	return actionStr;
}


void PrefShortcuts::shortcut_okPB_pressed()
{
	QString const new_lfun = shortcut_->lfunLE->text();
	FuncRequest func = lyxaction.lookupFunc(fromqstr(new_lfun));

	if (func.action == LFUN_UNKNOWN_ACTION) {
		Alert::error(_("Failed to create shortcut"),
			_("Unknown or invalid LyX function"));
		return;
	}

	KeySequence k = shortcut_->shortcutWG->getKeySequence();
	if (k.length() == 0) {
		Alert::error(_("Failed to create shortcut"),
			_("Invalid or empty key sequence"));
		return;
	}

	// check to see if there's been any change
	FuncRequest oldBinding = system_bind_.getBinding(k);
	if (oldBinding.action == LFUN_UNKNOWN_ACTION)
		oldBinding = user_bind_.getBinding(k);
	if (oldBinding == func) {
		docstring const actionStr = makeCmdString(func);
		Alert::error(_("Failed to create shortcut"),
			bformat(_("Shortcut `%1$s' is already bound to:\n%2$s"), 
			k.print(KeySequence::ForGui), actionStr));
		return;
	}
	
	// make sure this key isn't already bound---and, if so, not unbound
	FuncCode const unbind = user_unbind_.getBinding(k).action;
	if (oldBinding.action != LFUN_UNKNOWN_ACTION && unbind == LFUN_UNKNOWN_ACTION)
	{
		// FIXME Perhaps we should offer to over-write the old shortcut?
		// If so, we'll need to remove it from our list, etc.
		docstring const actionStr = makeCmdString(oldBinding);
		Alert::error(_("Failed to create shortcut"),
			bformat(_("Shortcut `%1$s' is already bound to:\n%2$s\n"
			  "You need to remove that binding before creating a new one."), 
			k.print(KeySequence::ForGui), actionStr));
		return;
	}

	if (!save_lfun_.isEmpty() && new_lfun == save_lfun_)
		// real modification of the lfun's shortcut,
		// so remove the previous one
		removeShortcut();

	QTreeWidgetItem * item = insertShortcutItem(func, k, KeyMap::UserBind);
	if (item) {
		user_bind_.bind(&k, func);
		shortcutsTW->sortItems(0, Qt::AscendingOrder);
		shortcutsTW->setItemExpanded(item->parent(), true);
		shortcutsTW->scrollToItem(item);
	} else {
		Alert::error(_("Failed to create shortcut"),
			_("Can not insert shortcut to the list"));
		return;
	}
}


void PrefShortcuts::shortcut_cancelPB_pressed()
{
	shortcut_->shortcutWG->reset();
}


void PrefShortcuts::shortcut_clearPB_pressed()
{
	shortcut_->shortcutWG->reset();
}


void PrefShortcuts::shortcut_removePB_pressed()
{
	shortcut_->shortcutWG->removeFromSequence();
}


/////////////////////////////////////////////////////////////////////
//
// PrefIdentity
//
/////////////////////////////////////////////////////////////////////

PrefIdentity::PrefIdentity(GuiPreferences * form)
	: PrefModule(QString(), qt_("Identity"), form)
{
	setupUi(this);

	connect(nameED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
	connect(emailED, SIGNAL(textChanged(QString)),
		this, SIGNAL(changed()));
}


void PrefIdentity::apply(LyXRC & rc) const
{
	rc.user_name = fromqstr(nameED->text());
	rc.user_email = fromqstr(emailED->text());
}


void PrefIdentity::update(LyXRC const & rc)
{
	nameED->setText(toqstr(rc.user_name));
	emailED->setText(toqstr(rc.user_email));
}



/////////////////////////////////////////////////////////////////////
//
// GuiPreferences
//
/////////////////////////////////////////////////////////////////////

GuiPreferences::GuiPreferences(GuiView & lv)
	: GuiDialog(lv, "prefs", qt_("Preferences")), update_screen_font_(false)
{
	setupUi(this);

	QDialog::setModal(false);

	connect(savePB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(restorePB, SIGNAL(clicked()), this, SLOT(slotRestore()));

	addModule(new PrefUserInterface(this));
	addModule(new PrefEdit(this));
	addModule(new PrefShortcuts(this));
	addModule(new PrefScreenFonts(this));
	addModule(new PrefColors(this));
	addModule(new PrefDisplay(this));
	addModule(new PrefInput(this));
	addModule(new PrefCompletion(this));

	addModule(new PrefPaths(this));

	addModule(new PrefIdentity(this));

	addModule(new PrefLanguage(this));
	addModule(new PrefSpellchecker(this));

	addModule(new PrefPrinter(this));
	PrefDate * dateFormat = new PrefDate(this);
	addModule(dateFormat);
	addModule(new PrefPlaintext(this));
	addModule(new PrefLatex(this));

	PrefConverters * converters = new PrefConverters(this);
	PrefFileformats * formats = new PrefFileformats(this);
	connect(formats, SIGNAL(formatsChanged()),
			converters, SLOT(updateGui()));
	addModule(converters);
	addModule(formats);

	prefsPS->setCurrentPanel(qt_("User interface"));
// FIXME: hack to work around resizing bug in Qt >= 4.2
// bug verified with Qt 4.2.{0-3} (JSpitzm)
#if QT_VERSION >= 0x040200
	prefsPS->updateGeometry();
#endif

	bc().setPolicy(ButtonPolicy::PreferencesPolicy);
	bc().setOK(savePB);
	bc().setApply(applyPB);
	bc().setCancel(closePB);
	bc().setRestore(restorePB);

	// initialize the strftime validator
	bc().addCheckedLineEdit(dateFormat->DateED);
}


void GuiPreferences::addModule(PrefModule * module)
{
	LASSERT(module, return);
	if (module->category().isEmpty())
		prefsPS->addPanel(module, module->title());
	else
		prefsPS->addPanel(module, module->title(), module->category());
	connect(module, SIGNAL(changed()), this, SLOT(change_adaptor()));
	modules_.push_back(module);
}


void GuiPreferences::change_adaptor()
{
	changed();
}


void GuiPreferences::apply(LyXRC & rc) const
{
	size_t end = modules_.size();
	for (size_t i = 0; i != end; ++i)
		modules_[i]->apply(rc);
}


void GuiPreferences::updateRc(LyXRC const & rc)
{
	size_t const end = modules_.size();
	for (size_t i = 0; i != end; ++i)
		modules_[i]->update(rc);
}


void GuiPreferences::applyView()
{
	apply(rc());
}

bool GuiPreferences::initialiseParams(string const &)
{
	rc_ = lyxrc;
	formats_ = lyx::formats;
	converters_ = theConverters();
	converters_.update(formats_);
	movers_ = theMovers();
	colors_.clear();
	update_screen_font_ = false;
	
	updateRc(rc_);
	// Make sure that the bc is in the INITIAL state  
	if (bc().policy().buttonStatus(ButtonPolicy::RESTORE))  
		bc().restore();  

	return true;
}


void GuiPreferences::dispatchParams()
{
	ostringstream ss;
	rc_.write(ss, true);
	dispatch(FuncRequest(LFUN_LYXRC_APPLY, ss.str()));
	// FIXME: these need lfuns
	// FIXME UNICODE
	theBufferList().setCurrentAuthor(from_utf8(rc_.user_name), from_utf8(rc_.user_email));

	lyx::formats = formats_;

	theConverters() = converters_;
	theConverters().update(lyx::formats);
	theConverters().buildGraph();

	theMovers() = movers_;

	vector<string>::const_iterator it = colors_.begin();
	vector<string>::const_iterator const end = colors_.end();
	for (; it != end; ++it)
		dispatch(FuncRequest(LFUN_SET_COLOR, *it));
	colors_.clear();

	if (update_screen_font_) {
		dispatch(FuncRequest(LFUN_SCREEN_FONT_UPDATE));
		update_screen_font_ = false;
	}

	// The Save button has been pressed
	if (isClosing())
		dispatch(FuncRequest(LFUN_PREFERENCES_SAVE));
}


void GuiPreferences::setColor(ColorCode col, QString const & hex)
{
	colors_.push_back(lcolor.getLyXName(col) + ' ' + fromqstr(hex));
}


void GuiPreferences::updateScreenFonts()
{
	update_screen_font_ = true;
}


QString GuiPreferences::browsebind(QString const & file) const
{
	return browseLibFile("bind", file, "bind", qt_("Choose bind file"),
			     QStringList(qt_("LyX bind files (*.bind)")));
}


QString GuiPreferences::browseUI(QString const & file) const
{
	return browseLibFile("ui", file, "ui", qt_("Choose UI file"),
			     QStringList(qt_("LyX UI files (*.ui)")));
}


QString GuiPreferences::browsekbmap(QString const & file) const
{
	return browseLibFile("kbd", file, "kmap", qt_("Choose keyboard map"),
			     QStringList(qt_("LyX keyboard maps (*.kmap)")));
}


QString GuiPreferences::browsedict(QString const & file) const
{
	return browseFile(file, qt_("Choose personal dictionary"),
		QStringList(qt_("*.pws")));
}


QString GuiPreferences::browse(QString const & file,
	QString const & title) const
{
	return browseFile(file, title, QStringList(), true);
}


// We support less paper sizes than the document dialog
// Therefore this adjustment is needed.
PAPER_SIZE GuiPreferences::toPaperSize(int i) const
{
	switch (i) {
	case 0:
		return PAPER_DEFAULT;
	case 1:
		return PAPER_USLETTER;
	case 2:
		return PAPER_USLEGAL;
	case 3:
		return PAPER_USEXECUTIVE;
	case 4:
		return PAPER_A3;
	case 5:
		return PAPER_A4;
	case 6:
		return PAPER_A5;
	case 7:
		return PAPER_B5;
	default:
		// should not happen
		return PAPER_DEFAULT;
	}
}


int GuiPreferences::fromPaperSize(PAPER_SIZE papersize) const
{
	switch (papersize) {
	case PAPER_DEFAULT:
		return 0;
	case PAPER_USLETTER:
		return 1;
	case PAPER_USLEGAL:
		return 2;
	case PAPER_USEXECUTIVE:
		return 3;
	case PAPER_A3:
		return 4;
	case PAPER_A4:
		return 5;
	case PAPER_A5:
		return 6;
	case PAPER_B5:
		return 7;
	default:
		// should not happen
		return 0;
	}
}


Dialog * createGuiPreferences(GuiView & lv) { return new GuiPreferences(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiPrefs.cpp"
