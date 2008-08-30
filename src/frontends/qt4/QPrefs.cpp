/**
 * \file QPrefs.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QPrefs.h"

#include "Qt2BC.h"
#include "qt_helpers.h"
#include "GuiApplication.h"
#include "GuiFontLoader.h"

#include "ConverterCache.h"
#include "Session.h"
#include "debug.h"
#include "Color.h"
#include "Font.h"
#include "PanelStack.h"
#include "QFontExample.h"
#include "gettext.h"

#include "support/lstrings.h"
#include "support/os.h"

#include "controllers/ControlPrefs.h"
#include "controllers/frontend_helpers.h"

#include "frontends/alert.h"
#include "frontends/Application.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QFontDatabase>
#include <QLineEdit>
#include <QPixmapCache>
#include <QPushButton>
#include <QSpinBox>
#include <QString>
#include <QValidator>
#include <QCloseEvent>

#include <boost/tuple/tuple.hpp>
#include <iomanip>
#include <sstream>

using namespace Ui;

using lyx::support::compare_ascii_no_case;
using lyx::support::os::external_path;
using lyx::support::os::external_path_list;
using lyx::support::os::internal_path;
using lyx::support::os::internal_path_list;
using lyx::support::split;

using std::endl;
using std::string;
using std::pair;
using std::vector;


namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////////
//
// Helpers
//
/////////////////////////////////////////////////////////////////////

string const LookAndFeel = N_("Look and feel");
string const LanguageSettings = N_("Language settings");
string const Outputs = N_("Outputs");


// FIXME: move to frontend_helpers.h

template<class A>
static size_t findPos_helper(std::vector<A> const & vec, A const & val)
{
	typedef typename std::vector<A>::const_iterator Cit;

	Cit it = std::find(vec.begin(), vec.end(), val);
	if (it == vec.end())
		return 0;
	return std::distance(vec.begin(), it);
}


static void setComboxFont(QComboBox * cb, string const & family,
	string const & foundry)
{
	string const name = makeFontName(family, foundry);
	for (int i = 0; i < cb->count(); ++i) {
		if (fromqstr(cb->itemText(i)) == name) {
			cb->setCurrentIndex(i);
			return;
		}
	}

	// Try matching without foundry name

	// We count in reverse in order to prefer the Xft foundry
	for (int i = cb->count() - 1; i >= 0; --i) {
		pair<string, string> tmp = parseFontName(fromqstr(cb->itemText(i)));
		if (compare_ascii_no_case(tmp.first, family) == 0) {
			cb->setCurrentIndex(i);
			return;
		}
	}

	// family alone can contain e.g. "Helvetica [Adobe]"
	pair<string, string> tmpfam = parseFontName(family);

	// We count in reverse in order to prefer the Xft foundry
	for (int i = cb->count() - 1; i >= 0; --i) {
		pair<string, string> tmp = parseFontName(fromqstr(cb->itemText(i)));
		if (compare_ascii_no_case(tmp.first, tmpfam.first) == 0) {
			cb->setCurrentIndex(i);
			return;
		}
	}

	// Bleh, default fonts, and the names couldn't be found. Hack
	// for bug 1063.

	QFont font;
	font.setKerning(false);

	if (family == theApp()->romanFontName()) {
		font.setStyleHint(QFont::Serif);
		font.setFamily(family.c_str());
	} else if (family == theApp()->sansFontName()) {
		font.setStyleHint(QFont::SansSerif);
		font.setFamily(family.c_str());
	} else if (family == theApp()->typewriterFontName()) {
		font.setStyleHint(QFont::TypeWriter);
		font.setFamily(family.c_str());
	} else {
		lyxerr << "FAILED to find the default font: '"
		       << foundry << "', '" << family << '\''<< endl;
		return;
	}

	QFontInfo info(font);
	pair<string, string> tmp = parseFontName(fromqstr(info.family()));
	string const & default_font_name = tmp.first;
	lyxerr << "Apparent font is " << default_font_name << endl;

	for (int i = 0; i < cb->count(); ++i) {
		lyxerr << "Looking at " << fromqstr(cb->itemText(i)) << endl;
		if (compare_ascii_no_case(fromqstr(cb->itemText(i)),
				    default_font_name) == 0) {
			cb->setCurrentIndex(i);
			return;
		}
	}

	lyxerr << "FAILED to find the font: '"
	       << foundry << "', '" << family << '\'' <<endl;
}


/////////////////////////////////////////////////////////////////////
//
// PrefPlaintext
//
/////////////////////////////////////////////////////////////////////

PrefPlaintext::PrefPlaintext(QWidget * parent)
	: PrefModule(_(Outputs), _("Plain text"), 0, parent)
{
	setupUi(this);
	connect(plaintextLinelengthSB, SIGNAL(valueChanged(int)),
		this, SIGNAL(changed()));
	connect(plaintextRoffED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
}


void PrefPlaintext::apply(LyXRC & rc) const
{
	rc.plaintext_linelen = plaintextLinelengthSB->value();
	rc.plaintext_roff_command = fromqstr(plaintextRoffED->text());
}


void PrefPlaintext::update(LyXRC const & rc)
{
	plaintextLinelengthSB->setValue(rc.plaintext_linelen);
	plaintextRoffED->setText(toqstr(rc.plaintext_roff_command));
}


/////////////////////////////////////////////////////////////////////
//
// PrefDate
//
/////////////////////////////////////////////////////////////////////

PrefDate::PrefDate(QWidget * parent)
	: PrefModule(_(Outputs), _("Date format"), 0, parent)
{
	setupUi(this);
	connect(DateED, SIGNAL(textChanged(const QString &)),
		this, SIGNAL(changed()));
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
// PrefKeyboard
//
/////////////////////////////////////////////////////////////////////

PrefKeyboard::PrefKeyboard(QPrefs * form, QWidget * parent)
	: PrefModule(_(LookAndFeel), _("Keyboard"), form, parent)
{
	setupUi(this);

	connect(keymapCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(firstKeymapED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(secondKeymapED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
}


void PrefKeyboard::apply(LyXRC & rc) const
{
	// FIXME: can derive CB from the two EDs
	rc.use_kbmap = keymapCB->isChecked();
	rc.primary_kbmap = internal_path(fromqstr(firstKeymapED->text()));
	rc.secondary_kbmap = internal_path(fromqstr(secondKeymapED->text()));
}


void PrefKeyboard::update(LyXRC const & rc)
{
	// FIXME: can derive CB from the two EDs
	keymapCB->setChecked(rc.use_kbmap);
	firstKeymapED->setText(toqstr(external_path(rc.primary_kbmap)));
	secondKeymapED->setText(toqstr(external_path(rc.secondary_kbmap)));
}


QString PrefKeyboard::testKeymap(QString keymap)
{
	return toqstr(form_->controller().browsekbmap(from_utf8(internal_path(fromqstr(keymap)))));
}


void PrefKeyboard::on_firstKeymapPB_clicked(bool)
{
	QString const file = testKeymap(firstKeymapED->text());
	if (!file.isEmpty())
		firstKeymapED->setText(file);
}


void PrefKeyboard::on_secondKeymapPB_clicked(bool)
{
	QString const file = testKeymap(secondKeymapED->text());
	if (!file.isEmpty())
		secondKeymapED->setText(file);
}


void PrefKeyboard::on_keymapCB_toggled(bool keymap)
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
// PrefLatex
//
/////////////////////////////////////////////////////////////////////

PrefLatex::PrefLatex(QPrefs * form, QWidget * parent)
	: PrefModule(_(Outputs), _("LaTeX"), form, parent)
{
	setupUi(this);
	connect(latexEncodingED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(latexChecktexED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(latexBibtexED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(latexIndexED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(latexAutoresetCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(latexDviPaperED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(latexPaperSizeCO, SIGNAL(activated(int)),
		this, SIGNAL(changed()));
}


void PrefLatex::apply(LyXRC & rc) const
{
	rc.fontenc = fromqstr(latexEncodingED->text());
	rc.chktex_command = fromqstr(latexChecktexED->text());
	rc.bibtex_command = fromqstr(latexBibtexED->text());
	rc.index_command = fromqstr(latexIndexED->text());
	rc.nomencl_command = fromqstr(latexNomenclED->text());
	rc.auto_reset_options = latexAutoresetCB->isChecked();
	rc.view_dvi_paper_option = fromqstr(latexDviPaperED->text());
	rc.default_papersize =
		form_->controller().toPaperSize(latexPaperSizeCO->currentIndex());
}


void PrefLatex::update(LyXRC const & rc)
{
	latexEncodingED->setText(toqstr(rc.fontenc));
	latexChecktexED->setText(toqstr(rc.chktex_command));
	latexBibtexED->setText(toqstr(rc.bibtex_command));
	latexIndexED->setText(toqstr(rc.index_command));
	latexNomenclED->setText(toqstr(rc.nomencl_command));
	latexAutoresetCB->setChecked(rc.auto_reset_options);
	latexDviPaperED->setText(toqstr(rc.view_dvi_paper_option));
	latexPaperSizeCO->setCurrentIndex(
		form_->controller().fromPaperSize(rc.default_papersize));
}


/////////////////////////////////////////////////////////////////////
//
// PrefScreenFonts
//
/////////////////////////////////////////////////////////////////////

PrefScreenFonts::PrefScreenFonts(QPrefs * form, QWidget * parent)
	: PrefModule(_(LookAndFeel), _("Screen fonts"), form, parent)
{
	setupUi(this);

	connect(screenRomanCO, SIGNAL(activated(const QString&)),
		this, SLOT(select_roman(const QString&)));
	connect(screenSansCO, SIGNAL(activated(const QString&)),
		this, SLOT(select_sans(const QString&)));
	connect(screenTypewriterCO, SIGNAL(activated(const QString&)),
		this, SLOT(select_typewriter(const QString&)));

	QFontDatabase fontdb;
	QStringList families(fontdb.families());
	for (QStringList::Iterator it = families.begin(); it != families.end(); ++it) {
		screenRomanCO->addItem(*it);
		screenSansCO->addItem(*it);
		screenTypewriterCO->addItem(*it);
	}
	connect(screenRomanCO, SIGNAL(activated(const QString&)),
		this, SIGNAL(changed()));
	connect(screenSansCO, SIGNAL(activated(const QString&)),
		this, SIGNAL(changed()));
	connect(screenTypewriterCO, SIGNAL(activated(const QString&)),
		this, SIGNAL(changed()));
	connect(screenZoomSB, SIGNAL(valueChanged(int)),
		this, SIGNAL(changed()));
	connect(screenDpiSB, SIGNAL(valueChanged(int)),
		this, SIGNAL(changed()));
	connect(screenTinyED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(screenSmallestED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(screenSmallerED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(screenSmallED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(screenNormalED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(screenLargeED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(screenLargerED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(screenLargestED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(screenHugeED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(screenHugerED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));

	screenTinyED->setValidator(new QDoubleValidator(
		screenTinyED));
	screenSmallestED->setValidator(new QDoubleValidator(
		screenSmallestED));
	screenSmallerED->setValidator(new QDoubleValidator(
		screenSmallerED));
	screenSmallED->setValidator(new QDoubleValidator(
		screenSmallED));
	screenNormalED->setValidator(new QDoubleValidator(
		screenNormalED));
	screenLargeED->setValidator(new QDoubleValidator(
		screenLargeED));
	screenLargerED->setValidator(new QDoubleValidator(
		screenLargerED));
	screenLargestED->setValidator(new QDoubleValidator(
		screenLargestED));
	screenHugeED->setValidator(new QDoubleValidator(
		screenHugeED));
	screenHugerED->setValidator(new QDoubleValidator(
		screenHugerED));
}


void PrefScreenFonts::apply(LyXRC & rc) const
{
	LyXRC const oldrc(rc);

	boost::tie(rc.roman_font_name, rc.roman_font_foundry)
		= parseFontName(fromqstr(screenRomanCO->currentText()));
	boost::tie(rc.sans_font_name, rc.sans_font_foundry) =
		parseFontName(fromqstr(screenSansCO->currentText()));
	boost::tie(rc.typewriter_font_name, rc.typewriter_font_foundry) =
		parseFontName(fromqstr(screenTypewriterCO->currentText()));

	rc.zoom = screenZoomSB->value();
	rc.dpi = screenDpiSB->value();
	rc.font_sizes[Font::SIZE_TINY] = fromqstr(screenTinyED->text());
	rc.font_sizes[Font::SIZE_SCRIPT] = fromqstr(screenSmallestED->text());
	rc.font_sizes[Font::SIZE_FOOTNOTE] = fromqstr(screenSmallerED->text());
	rc.font_sizes[Font::SIZE_SMALL] = fromqstr(screenSmallED->text());
	rc.font_sizes[Font::SIZE_NORMAL] = fromqstr(screenNormalED->text());
	rc.font_sizes[Font::SIZE_LARGE] = fromqstr(screenLargeED->text());
	rc.font_sizes[Font::SIZE_LARGER] = fromqstr(screenLargerED->text());
	rc.font_sizes[Font::SIZE_LARGEST] = fromqstr(screenLargestED->text());
	rc.font_sizes[Font::SIZE_HUGE] = fromqstr(screenHugeED->text());
	rc.font_sizes[Font::SIZE_HUGER] = fromqstr(screenHugerED->text());

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
		form_->controller().updateScreenFonts();
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
	screenTinyED->setText(toqstr(rc.font_sizes[Font::SIZE_TINY]));
	screenSmallestED->setText(toqstr(rc.font_sizes[Font::SIZE_SCRIPT]));
	screenSmallerED->setText(toqstr(rc.font_sizes[Font::SIZE_FOOTNOTE]));
	screenSmallED->setText(toqstr(rc.font_sizes[Font::SIZE_SMALL]));
	screenNormalED->setText(toqstr(rc.font_sizes[Font::SIZE_NORMAL]));
	screenLargeED->setText(toqstr(rc.font_sizes[Font::SIZE_LARGE]));
	screenLargerED->setText(toqstr(rc.font_sizes[Font::SIZE_LARGER]));
	screenLargestED->setText(toqstr(rc.font_sizes[Font::SIZE_LARGEST]));
	screenHugeED->setText(toqstr(rc.font_sizes[Font::SIZE_HUGE]));
	screenHugerED->setText(toqstr(rc.font_sizes[Font::SIZE_HUGER]));
}

void PrefScreenFonts::select_roman(const QString& name)
{
	screenRomanFE->set(QFont(name), name);
}


void PrefScreenFonts::select_sans(const QString& name)
{
	screenSansFE->set(QFont(name), name);
}


void PrefScreenFonts::select_typewriter(const QString& name)
{
	screenTypewriterFE->set(QFont(name), name);
}


/////////////////////////////////////////////////////////////////////
//
// PrefColors
//
/////////////////////////////////////////////////////////////////////

PrefColors::PrefColors(QPrefs * form, QWidget * parent)
	: PrefModule(_(LookAndFeel), _("Colors"), form, parent)
{
	setupUi(this);

	// FIXME: all of this initialization should be put into the controller.
	// See http://www.mail-archive.com/lyx-devel@lists.lyx.org/msg113301.html
	// for some discussion of why that is not trivial.
	QPixmap icon(32, 32);
	for (int i = 0; i < Color::ignore; ++i) {
		Color::color lc = static_cast<Color::color>(i);
		if (lc == Color::none
			|| lc == Color::black
			|| lc == Color::white
			|| lc == Color::red
			|| lc == Color::green
			|| lc == Color::blue
			|| lc == Color::cyan
			|| lc == Color::magenta
			|| lc == Color::yellow
			|| lc == Color::inherit
			|| lc == Color::ignore) continue;

		lcolors_.push_back(lc);
	}
	lcolors_ = frontend::getSortedColors(lcolors_);
	vector<Color_color>::const_iterator cit = lcolors_.begin();
	vector<Color_color>::const_iterator const end = lcolors_.end();
	for (; cit != end; ++cit)
	{
		// This is not a memory leak:
		/*QListWidgetItem * newItem =*/ new QListWidgetItem(QIcon(icon),
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
	for (unsigned int i = 0; i < lcolors_.size(); ++i) {
		if (curcolors_[i] != newcolors_[i]) {
			form_->controller().setColor(lcolors_[i], fromqstr(newcolors_[i]));
		}
	}
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
	if (row < 0) return;

	QString const color = newcolors_[row];
	QColor c(QColorDialog::getColor(QColor(color), qApp->focusWidget()));

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
// PrefCygwinPath
//
/////////////////////////////////////////////////////////////////////

PrefCygwinPath::PrefCygwinPath(QWidget * parent)
	: PrefModule(_(Outputs), _("Paths"), 0, parent)
{
	setupUi(this);
	connect(pathCB, SIGNAL(clicked()), this, SIGNAL(changed()));
}


void PrefCygwinPath::apply(LyXRC & rc) const
{
	rc.windows_style_tex_paths = pathCB->isChecked();
}


void PrefCygwinPath::update(LyXRC const & rc)
{
	pathCB->setChecked(rc.windows_style_tex_paths);
}


/////////////////////////////////////////////////////////////////////
//
// PrefDisplay
//
/////////////////////////////////////////////////////////////////////

PrefDisplay::PrefDisplay(QWidget * parent)
	: PrefModule(_(LookAndFeel), _("Graphics"), 0, parent)
{
	setupUi(this);
	connect(instantPreviewCO, SIGNAL(activated(int)),
		this, SIGNAL(changed()));
	connect(displayGraphicsCO, SIGNAL(activated(int)),
		this, SIGNAL(changed()));
}


void PrefDisplay::apply(LyXRC & rc) const
{
	switch (instantPreviewCO->currentIndex()) {
	case 0: rc.preview = LyXRC::PREVIEW_OFF; break;
	case 1:	rc.preview = LyXRC::PREVIEW_NO_MATH; break;
	case 2:	rc.preview = LyXRC::PREVIEW_ON;	break;
	}

	lyx::graphics::DisplayType dtype;
	switch (displayGraphicsCO->currentIndex()) {
	case 3:	dtype = lyx::graphics::NoDisplay; break;
	case 2:	dtype = lyx::graphics::ColorDisplay; break;
	case 1: dtype = lyx::graphics::GrayscaleDisplay;	break;
	case 0: dtype = lyx::graphics::MonochromeDisplay; break;
	default: dtype = lyx::graphics::GrayscaleDisplay;
	}
	rc.display_graphics = dtype;

#ifdef WITH_WARNINGS
#warning FIXME!! The graphics cache no longer has a changeDisplay method.
#endif
#if 0
	if (old_value != rc.display_graphics) {
		lyx::graphics::GCache & gc = lyx::graphics::GCache::get();
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

	int item = 2;
	switch (rc.display_graphics) {
		case lyx::graphics::NoDisplay:		item = 3; break;
		case lyx::graphics::ColorDisplay:	item = 2; break;
		case lyx::graphics::GrayscaleDisplay:	item = 1; break;
		case lyx::graphics::MonochromeDisplay:	item = 0; break;
		default: break;
	}
	displayGraphicsCO->setCurrentIndex(item);
}


/////////////////////////////////////////////////////////////////////
//
// PrefPaths
//
/////////////////////////////////////////////////////////////////////

PrefPaths::PrefPaths(QPrefs * form, QWidget * parent)
	: PrefModule(docstring(), _("Paths"), form, parent)
{
	setupUi(this);
	connect(templateDirPB, SIGNAL(clicked()), this, SLOT(select_templatedir()));
	connect(tempDirPB, SIGNAL(clicked()), this, SLOT(select_tempdir()));
	connect(backupDirPB, SIGNAL(clicked()), this, SLOT(select_backupdir()));
	connect(workingDirPB, SIGNAL(clicked()), this, SLOT(select_workingdir()));
	connect(lyxserverDirPB, SIGNAL(clicked()), this, SLOT(select_lyxpipe()));
	connect(workingDirED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(templateDirED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(backupDirED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(tempDirED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(lyxserverDirED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(pathPrefixED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
}


void PrefPaths::apply(LyXRC & rc) const
{
	rc.document_path = internal_path(fromqstr(workingDirED->text()));
	rc.template_path = internal_path(fromqstr(templateDirED->text()));
	rc.backupdir_path = internal_path(fromqstr(backupDirED->text()));
	rc.tempdir_path = internal_path(fromqstr(tempDirED->text()));
	rc.path_prefix = internal_path_list(fromqstr(pathPrefixED->text()));
	// FIXME: should be a checkbox only
	rc.lyxpipes = internal_path(fromqstr(lyxserverDirED->text()));
}


void PrefPaths::update(LyXRC const & rc)
{
	workingDirED->setText(toqstr(external_path(rc.document_path)));
	templateDirED->setText(toqstr(external_path(rc.template_path)));
	backupDirED->setText(toqstr(external_path(rc.backupdir_path)));
	tempDirED->setText(toqstr(external_path(rc.tempdir_path)));
	pathPrefixED->setText(toqstr(external_path_list(rc.path_prefix)));
	// FIXME: should be a checkbox only
	lyxserverDirED->setText(toqstr(external_path(rc.lyxpipes)));
}


void PrefPaths::select_templatedir()
{
	docstring file(form_->controller().browsedir(
		from_utf8(internal_path(fromqstr(templateDirED->text()))),
		_("Select a document templates directory")));
	if (!file.empty())
		templateDirED->setText(toqstr(file));
}


void PrefPaths::select_tempdir()
{
	docstring file(form_->controller().browsedir(
		from_utf8(internal_path(fromqstr(tempDirED->text()))),
		_("Select a temporary directory")));
	if (!file.empty())
		tempDirED->setText(toqstr(file));
}


void PrefPaths::select_backupdir()
{
	docstring file(form_->controller().browsedir(
		from_utf8(internal_path(fromqstr(backupDirED->text()))),
		_("Select a backups directory")));
	if (!file.empty())
		backupDirED->setText(toqstr(file));
}


void PrefPaths::select_workingdir()
{
	docstring file(form_->controller().browsedir(
		from_utf8(internal_path(fromqstr(workingDirED->text()))),
		_("Select a document directory")));
	if (!file.empty())
		workingDirED->setText(toqstr(file));
}


void PrefPaths::select_lyxpipe()
{
	docstring file(form_->controller().browse(
		from_utf8(internal_path(fromqstr(lyxserverDirED->text()))),
		_("Give a filename for the LyX server pipe")));
	if (!file.empty())
		lyxserverDirED->setText(toqstr(file));
}


/////////////////////////////////////////////////////////////////////
//
// PrefSpellchecker
//
/////////////////////////////////////////////////////////////////////

PrefSpellchecker::PrefSpellchecker(QPrefs * form, QWidget * parent)
	: PrefModule(_(LanguageSettings), _("Spellchecker"), form, parent)
{
	setupUi(this);

	connect(persDictionaryPB, SIGNAL(clicked()), this, SLOT(select_dict()));
#if defined (USE_ISPELL)
	connect(spellCommandCO, SIGNAL(activated(int)),
		this, SIGNAL(changed()));
#else
	spellCommandCO->setEnabled(false);
#endif
	connect(altLanguageED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(escapeCharactersED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(persDictionaryED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(compoundWordCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(inputEncodingCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));

	spellCommandCO->addItem(qt_("ispell"));
	spellCommandCO->addItem(qt_("aspell"));
	spellCommandCO->addItem(qt_("hspell"));
#ifdef USE_PSPELL
	spellCommandCO->addItem(qt_("pspell (library)"));
#else
#ifdef USE_ASPELL
	spellCommandCO->addItem(qt_("aspell (library)"));
#endif
#endif
}


void PrefSpellchecker::apply(LyXRC & rc) const
{
	switch (spellCommandCO->currentIndex()) {
		case 0:
		case 1:
		case 2:
			rc.use_spell_lib = false;
			rc.isp_command = fromqstr(spellCommandCO->currentText());
			break;
		case 3:
			rc.use_spell_lib = true;
			break;
	}

	// FIXME: remove isp_use_alt_lang
	rc.isp_alt_lang = fromqstr(altLanguageED->text());
	rc.isp_use_alt_lang = !rc.isp_alt_lang.empty();
	// FIXME: remove isp_use_esc_chars
	rc.isp_esc_chars = fromqstr(escapeCharactersED->text());
	rc.isp_use_esc_chars = !rc.isp_esc_chars.empty();
	// FIXME: remove isp_use_pers_dict
	rc.isp_pers_dict = internal_path(fromqstr(persDictionaryED->text()));
	rc.isp_use_pers_dict = !rc.isp_pers_dict.empty();
	rc.isp_accept_compound = compoundWordCB->isChecked();
	rc.isp_use_input_encoding = inputEncodingCB->isChecked();
}


void PrefSpellchecker::update(LyXRC const & rc)
{
	spellCommandCO->setCurrentIndex(0);

	if (rc.isp_command == "ispell") {
		spellCommandCO->setCurrentIndex(0);
	} else if (rc.isp_command == "aspell") {
		spellCommandCO->setCurrentIndex(1);
	} else if (rc.isp_command == "hspell") {
		spellCommandCO->setCurrentIndex(2);
	}

	if (rc.use_spell_lib) {
#if defined(USE_ASPELL) || defined(USE_PSPELL)
		spellCommandCO->setCurrentIndex(3);
#endif
	}

	// FIXME: remove isp_use_alt_lang
	altLanguageED->setText(toqstr(rc.isp_alt_lang));
	// FIXME: remove isp_use_esc_chars
	escapeCharactersED->setText(toqstr(rc.isp_esc_chars));
	// FIXME: remove isp_use_pers_dict
	persDictionaryED->setText(toqstr(external_path(rc.isp_pers_dict)));
	compoundWordCB->setChecked(rc.isp_accept_compound);
	inputEncodingCB->setChecked(rc.isp_use_input_encoding);
}


void PrefSpellchecker::select_dict()
{
	docstring file(form_->controller().browsedict(
		from_utf8(internal_path(fromqstr(persDictionaryED->text())))));
	if (!file.empty())
		persDictionaryED->setText(toqstr(file));
}



/////////////////////////////////////////////////////////////////////
//
// PrefConverters
//
/////////////////////////////////////////////////////////////////////


PrefConverters::PrefConverters(QPrefs * form, QWidget * parent)
	: PrefModule(docstring(), _("Converters"), form, parent)
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
	connect(converterFromCO, SIGNAL(activated(const QString&)),
		this, SLOT(converter_changed()));
	connect(converterToCO, SIGNAL(activated(const QString&)),
		this, SLOT(converter_changed()));
	connect(converterED, SIGNAL(textChanged(const QString&)),
		this, SLOT(converter_changed()));
	connect(converterFlagED, SIGNAL(textChanged(const QString&)),
		this, SLOT(converter_changed()));
	connect(converterNewPB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(converterRemovePB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(converterModifyPB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(maxAgeLE, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));

	maxAgeLE->setValidator(new QDoubleValidator(maxAgeLE));
	converterDefGB->setFocusProxy(convertersLW);
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
		if (item.size() > 0)
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
	Format const & from(form_->formats().get(converterFromCO->currentIndex()));
	Format const & to(form_->formats().get(converterToCO->currentIndex()));
	int const sel = form_->converters().getNumber(from.name(), to.name());
	bool const known = !(sel < 0);
	bool const valid = !(converterED->text().isEmpty()
		|| from.name() == to.name());

	int const cnr = convertersLW->currentItem()->type();
	Converter const & c(form_->converters().get(cnr));
	string const old_command = c.command;
	string const old_flag = c.flags;
	string const new_command(fromqstr(converterED->text()));
	string const new_flag(fromqstr(converterFlagED->text()));

	bool modified = ((old_command != new_command) || (old_flag != new_flag));

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
	Format const & from(form_->formats().get(converterFromCO->currentIndex()));
	Format const & to(form_->formats().get(converterToCO->currentIndex()));
	string const flags = fromqstr(converterFlagED->text());
	string const command = fromqstr(converterED->text());

	Converter const * old = form_->converters().getConverter(from.name(), to.name());
	form_->converters().add(from.name(), to.name(), command, flags);
	if (!old) {
		form_->converters().updateLast(form_->formats());
	}

	updateGui();

	// Remove all files created by this converter from the cache, since
	// the modified converter might create different files.
	ConverterCache::get().remove_all(from.name(), to.name());
}


void PrefConverters::remove_converter()
{
	Format const & from(form_->formats().get(converterFromCO->currentIndex()));
	Format const & to(form_->formats().get(converterToCO->currentIndex()));
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
// PrefCopiers
//
/////////////////////////////////////////////////////////////////////

PrefCopiers::PrefCopiers(QPrefs * form, QWidget * parent)
	: PrefModule(docstring(), _("Copiers"), form, parent)
{
	setupUi(this);

	connect(copierNewPB, SIGNAL(clicked()), this, SLOT(new_copier()));
	connect(copierRemovePB, SIGNAL(clicked()), this, SLOT(remove_copier()));
	connect(copierModifyPB, SIGNAL(clicked()), this, SLOT(modify_copier()));
	connect(AllCopiersLW, SIGNAL(currentRowChanged(int)),
		this, SLOT(switch_copierLB(int)));
	connect(copierFormatCO, SIGNAL(activated(int)),
		this, SLOT(switch_copierCO(int)));
	connect(copierNewPB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(copierRemovePB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(copierModifyPB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(copierFormatCO, SIGNAL(activated(const QString &)),
		this, SLOT(copiers_changed()));
	connect(copierED, SIGNAL(textChanged(const QString &)),
		this, SLOT(copiers_changed()));
}


void PrefCopiers::apply(LyXRC & /*rc*/) const
{
}


void PrefCopiers::update(LyXRC const & /*rc*/)
{
	update();
}


void PrefCopiers::update()
{
	// The choice widget
	// save current selection
	QString current = copierFormatCO->currentText();
	copierFormatCO->clear();

	for (Formats::const_iterator it = form_->formats().begin(),
		     end = form_->formats().end();
	     it != end; ++it) {
		copierFormatCO->addItem(qt_(it->prettyname()));
	}

	// The browser widget
	AllCopiersLW->clear();

	for (Movers::const_iterator it = form_->movers().begin(),
		     end = form_->movers().end();
	     it != end; ++it) {
		std::string const & command = it->second.command();
		if (command.empty())
			continue;
		QString const pretty = 
			toqstr(translateIfPossible(form_->formats().prettyName(it->first)));
		AllCopiersLW->addItem(pretty);
	}
	AllCopiersLW->sortItems(Qt::AscendingOrder);

	// restore selection
	if (!current.isEmpty()) {
		QList<QListWidgetItem *> item =
			AllCopiersLW->findItems(current, Qt::MatchExactly);
		if (item.size()>0)
			AllCopiersLW->setCurrentItem(item.at(0));
	}
	// select first element if restoring failed
	if (AllCopiersLW->currentRow() == -1)
		AllCopiersLW->setCurrentRow(0);
}


namespace {

class SamePrettyName {
public:
	SamePrettyName(docstring const & n) : pretty_name_(n) {}

	bool operator()(Format const & fmt) const {
		return _(fmt.prettyname()) == pretty_name_;
	}

private:
	docstring const pretty_name_;
};


Format const * getFormat(docstring const & prettyname)
{
	Formats::const_iterator it = formats.begin();
	Formats::const_iterator const end = formats.end();
	it = std::find_if(it, end, SamePrettyName(prettyname));
	return it == end ? 0 : &*it;
}

} // namespace anon


void PrefCopiers::switch_copierLB(int row)
{
	if (row < 0)
		return;

	docstring const browser_text =
		qstring_to_ucs4(AllCopiersLW->currentItem()->text());
	Format const * fmt = getFormat(browser_text);
	if (fmt == 0)
		return;

	QString const gui_name = qt_(fmt->prettyname());
	QString const command = toqstr(form_->movers().command(fmt->name()));

	copierED->clear();
	int const combo_size = copierFormatCO->count();
	for (int i = 0; i < combo_size; ++i) {
		QString const text = copierFormatCO->itemText(i);
		if (text == gui_name) {
			copierFormatCO->setCurrentIndex(i);
			copierED->setText(command);
			break;
		}
	}
	updateButtons();
}


void PrefCopiers::switch_copierCO(int row)
{
	if (row<0)
		return;

	docstring const combo_text =
		qstring_to_ucs4(copierFormatCO->currentText());
	Format const * fmt = getFormat(combo_text);
	if (fmt == 0)
		return;

	QString const command = toqstr(form_->movers().command(fmt->name()));
	copierED->setText(command);

	QListWidgetItem * const index = AllCopiersLW->currentItem();
	if (index >= 0)
		AllCopiersLW->setItemSelected(index, false);

	QString const gui_name = qt_(fmt->prettyname());
	int const browser_size = AllCopiersLW->count();
	for (int i = 0; i < browser_size; ++i) {
		QString const text = AllCopiersLW->item(i)->text();
		if (text == gui_name) {
			QListWidgetItem * item = AllCopiersLW->item(i);
			AllCopiersLW->setItemSelected(item, true);
			break;
		}
	}
}


void PrefCopiers::copiers_changed()
{
	updateButtons();
}


void PrefCopiers::updateButtons()
{
	QString selected = copierFormatCO->currentText();

	bool known = false;
	for (int i = 0; i < AllCopiersLW->count(); ++i) {
		if (AllCopiersLW->item(i)->text() == selected)
			known = true;
	}

	bool const valid = !copierED->text().isEmpty();

	Format const * fmt = getFormat(qstring_to_ucs4(selected));
	string const old_command = form_->movers().command(fmt->name());
	string const new_command(fromqstr(copierED->text()));

	bool modified = (old_command != new_command);

	copierModifyPB->setEnabled(valid && known && modified);
	copierNewPB->setEnabled(valid && !known);
	copierRemovePB->setEnabled(known);
}


void PrefCopiers::new_copier()
{
	docstring const combo_text =
		qstring_to_ucs4(copierFormatCO->currentText());
	Format const * fmt = getFormat(combo_text);
	if (fmt == 0)
		return;

	string const command = fromqstr(copierED->text());
	if (command.empty())
		return;

	form_->movers().set(fmt->name(), command);

	update();
	int const last = AllCopiersLW->count() - 1;
	AllCopiersLW->setCurrentRow(last);

	updateButtons();
}


void PrefCopiers::modify_copier()
{
	docstring const combo_text =
		qstring_to_ucs4(copierFormatCO->currentText());
	Format const * fmt = getFormat(combo_text);
	if (fmt == 0)
		return;

	string const command = fromqstr(copierED->text());
	form_->movers().set(fmt->name(), command);

	update();
	updateButtons();
}


void PrefCopiers::remove_copier()
{
	docstring const combo_text =
		qstring_to_ucs4(copierFormatCO->currentText());
	Format const * fmt = getFormat(combo_text);
	if (fmt == 0)
		return;

	string const & fmt_name = fmt->name();
	form_->movers().set(fmt_name, string());

	update();
	updateButtons();
}



/////////////////////////////////////////////////////////////////////
//
// PrefFileformats
//
/////////////////////////////////////////////////////////////////////

PrefFileformats::PrefFileformats(QPrefs * form, QWidget * parent)
	: PrefModule(docstring(), _("File formats"), form, parent)
{
	setupUi(this);

	connect(formatNewPB, SIGNAL(clicked()),
		this, SLOT(new_format()));
	connect(formatRemovePB, SIGNAL(clicked()),
		this, SLOT(remove_format()));
	connect(formatModifyPB, SIGNAL(clicked()),
		this, SLOT(modify_format()));
	connect(formatsLW, SIGNAL(currentRowChanged(int)),
		this, SLOT(switch_format(int)));
	connect(formatED, SIGNAL(textChanged(const QString&)),
		this, SLOT(fileformat_changed()));
	connect(guiNameED, SIGNAL(textChanged(const QString&)),
		this, SLOT(fileformat_changed()));
	connect(shortcutED, SIGNAL(textChanged(const QString&)),
		this, SLOT(fileformat_changed()));
	connect(extensionED, SIGNAL(textChanged(const QString&)),
		this, SLOT(fileformat_changed()));
	connect(viewerED, SIGNAL(textChanged(const QString&)),
		this, SLOT(fileformat_changed()));
	connect(editorED, SIGNAL(textChanged(const QString&)),
		this, SLOT(fileformat_changed()));
	connect(documentCB, SIGNAL(clicked()),
		this, SLOT(fileformat_changed()));
	connect(vectorCB, SIGNAL(clicked()),
		this, SLOT(fileformat_changed()));
	connect(formatNewPB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(formatRemovePB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(formatModifyPB, SIGNAL(clicked()),
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


void PrefFileformats::apply(LyXRC & /*rc*/) const
{
}


void PrefFileformats::update(LyXRC const & /*rc*/)
{
	update();
}


void PrefFileformats::update()
{
	// save current selection
	QString current = guiNameED->text();

	// update listwidget with formats
	formatsLW->blockSignals(true);
	formatsLW->clear();
	Formats::const_iterator cit = form_->formats().begin();
	Formats::const_iterator end = form_->formats().end();
	for (; cit != end; ++cit) {
		new QListWidgetItem(qt_(cit->prettyname()),
				   formatsLW,
				   form_->formats().getNumber(cit->name()) );
	}
	formatsLW->sortItems(Qt::AscendingOrder);
	formatsLW->blockSignals(false);

	// restore selection
	if (!current.isEmpty()) {
		QList<QListWidgetItem *>  item = formatsLW->findItems(current, Qt::MatchExactly);
		if (item.size()>0)
			formatsLW->setCurrentItem(item.at(0));
	}
	// select first element if restoring failed
	if (formatsLW->currentRow() == -1)
		formatsLW->setCurrentRow(0);
}


void PrefFileformats::switch_format(int nr)
{
	int const ftype = formatsLW->item(nr)->type();
	Format const f = form_->formats().get(ftype);

	formatED->setText(toqstr(f.name()));
	extensionED->setText(toqstr(f.extension()));
	viewerED->setText(toqstr(f.viewer()));
	editorED->setText(toqstr(f.editor()));
	documentCB->setChecked((f.documentFormat()));
	vectorCB->setChecked((f.vectorFormat()));

	// l10n
	guiNameED->setText(qt_(f.prettyname()));
	shortcutED->setText(toqstr(l10n_shortcut(f.prettyname(), f.shortcut())));

	updateButtons();
}


void PrefFileformats::fileformat_changed()
{
	updateButtons();
}


void PrefFileformats::updateButtons()
{
	QString const format = formatED->text();
	QString const gui_name = guiNameED->text();
	int const sel = form_->formats().getNumber(fromqstr(format));
	bool gui_name_known = false;
	int where = sel;
	for (int i = 0; i < formatsLW->count(); ++i) {
		if (formatsLW->item(i)->text() == gui_name) {
			gui_name_known = true;
			where = formatsLW->item(i)->type();
		}
	}

	// assure that a gui name cannot be chosen twice
	bool known_otherwise = gui_name_known && (where != sel);

	// Also check untranslated prettynames
	Formats::const_iterator cit = form_->formats().begin();
	Formats::const_iterator end = form_->formats().end();
	for (; cit != end; ++cit) {
		if (toqstr(cit->prettyname()) == gui_name
		    && qt_(cit->prettyname()) != gui_name)
			known_otherwise = true;
	}

	bool const known = !(sel < 0);
	bool const valid = (!formatED->text().isEmpty()
		&& !guiNameED->text().isEmpty());

	int const ftype = formatsLW->currentItem()->type();
	Format const & f(form_->formats().get(ftype));
	string const old_pretty(to_utf8(_(f.prettyname())));
	string const old_shortcut(l10n_shortcut(f.prettyname(), f.shortcut()));
	string const old_extension(f.extension());
	string const old_viewer(f.viewer());
	string const old_editor(f.editor());
	bool const old_document(f.documentFormat());
	bool const old_vector(f.vectorFormat());

	string const new_pretty(fromqstr(gui_name));
	string const new_shortcut(fromqstr(shortcutED->text()));
	string const new_extension(fromqstr(extensionED->text()));
	string const new_viewer(fromqstr(viewerED->text()));
	string const new_editor(fromqstr(editorED->text()));
	bool const new_document(documentCB->isChecked());
	bool const new_vector(vectorCB->isChecked());

	bool modified = ((old_pretty != new_pretty) || (old_shortcut != new_shortcut)
		|| (old_extension != new_extension) || (old_viewer != new_viewer)
		|| old_editor != new_editor || old_document != new_document
		|| old_vector != new_vector);

	formatModifyPB->setEnabled(valid && known && modified && !known_otherwise);
	formatNewPB->setEnabled(valid && !known && !gui_name_known);
	formatRemovePB->setEnabled(known);
}


void PrefFileformats::new_format()
{
	string const name = fromqstr(formatED->text());
	string prettyname = fromqstr(guiNameED->text());
	string const extension = fromqstr(extensionED->text());
	string shortcut = fromqstr(shortcutED->text());
	string const viewer = fromqstr(viewerED->text());
	string const editor = fromqstr(editorED->text());

	// handle translations
	Formats::const_iterator cit = form_->formats().begin();
	Formats::const_iterator end = form_->formats().end();
	for (; cit != end; ++cit) {
		if (_(cit->prettyname()) == from_utf8(prettyname)) {
			prettyname = cit->prettyname();
			if (shortcut == l10n_shortcut(prettyname, cit->shortcut()))
				shortcut == cit->shortcut();
		}
	}

	int flags = Format::none;
	if (documentCB->isChecked())
		flags |= Format::document;
	if (vectorCB->isChecked())
		flags |= Format::vector;

	form_->formats().add(name, extension, prettyname, shortcut, viewer,
			     editor, flags);
	form_->formats().sort();
	form_->converters().update(form_->formats());

	update();
	updateButtons();
	formatsChanged();
}


void PrefFileformats::modify_format()
{
	int const current_item = formatsLW->currentItem()->type();
	Format const & oldformat = form_->formats().get(current_item);
	form_->formats().erase(oldformat.name());

	new_format();
}


void PrefFileformats::remove_format()
{
	int const nr = formatsLW->currentItem()->type();
	string const current_text = form_->formats().get(nr).name();
	if (form_->converters().formatIsUsed(current_text)) {
		Alert::error(_("Format in use"),
			     _("Cannot remove a Format used by a Converter. "
					    "Remove the converter first."));
		return;
	}

	form_->formats().erase(current_text);
	form_->converters().update(form_->formats());

	update();
	updateButtons();
	formatsChanged();
}


/////////////////////////////////////////////////////////////////////
//
// PrefLanguage
//
/////////////////////////////////////////////////////////////////////

PrefLanguage::PrefLanguage(QWidget * parent)
	: PrefModule(_(LanguageSettings), _("Language"), 0, parent)
{
	setupUi(this);

	connect(rtlCB, SIGNAL(clicked()),
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
	connect(languagePackageED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(startCommandED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(endCommandED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(defaultLanguageCO, SIGNAL(activated(int)),
		this, SIGNAL(changed()));

	defaultLanguageCO->clear();

	// store the lang identifiers for later
	std::vector<LanguagePair> const langs = frontend::getLanguageData(false);
	lang_ = getSecond(langs);

	std::vector<LanguagePair>::const_iterator lit  = langs.begin();
	std::vector<LanguagePair>::const_iterator lend = langs.end();
	for (; lit != lend; ++lit) {
		defaultLanguageCO->addItem(toqstr(lit->first));
	}
}


void PrefLanguage::apply(LyXRC & rc) const
{
	// FIXME: remove rtl_support bool
	rc.rtl_support = rtlCB->isChecked();
	rc.mark_foreign_language = markForeignCB->isChecked();
	rc.language_auto_begin = autoBeginCB->isChecked();
	rc.language_auto_end = autoEndCB->isChecked();
	rc.language_use_babel = useBabelCB->isChecked();
	rc.language_global_options = globalCB->isChecked();
	rc.language_package = fromqstr(languagePackageED->text());
	rc.language_command_begin = fromqstr(startCommandED->text());
	rc.language_command_end = fromqstr(endCommandED->text());
	rc.default_language = lang_[defaultLanguageCO->currentIndex()];
}


void PrefLanguage::update(LyXRC const & rc)
{
	// FIXME: remove rtl_support bool
	rtlCB->setChecked(rc.rtl_support);
	markForeignCB->setChecked(rc.mark_foreign_language);
	autoBeginCB->setChecked(rc.language_auto_begin);
	autoEndCB->setChecked(rc.language_auto_end);
	useBabelCB->setChecked(rc.language_use_babel);
	globalCB->setChecked(rc.language_global_options);
	languagePackageED->setText(toqstr(rc.language_package));
	startCommandED->setText(toqstr(rc.language_command_begin));
	endCommandED->setText(toqstr(rc.language_command_end));

	int const pos = int(findPos_helper(lang_, rc.default_language));
	defaultLanguageCO->setCurrentIndex(pos);
}


/////////////////////////////////////////////////////////////////////
//
// PrefPrinter
//
/////////////////////////////////////////////////////////////////////

PrefPrinter::PrefPrinter(QWidget * parent)
	: PrefModule(_(Outputs), _("Printer"), 0, parent)
{
	setupUi(this);

	connect(printerAdaptCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(printerCommandED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(printerNameED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(printerPageRangeED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(printerCopiesED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(printerReverseED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(printerToPrinterED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(printerExtensionED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(printerSpoolCommandED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(printerPaperTypeED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(printerEvenED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(printerOddED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(printerCollatedED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(printerLandscapeED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(printerToFileED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(printerExtraED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(printerSpoolPrefixED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(printerPaperSizeED, SIGNAL(textChanged(const QString&)),
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

PrefUserInterface::PrefUserInterface(QPrefs * form, QWidget * parent)
	: PrefModule(_(LookAndFeel), _("User interface"), form, parent)
{
	setupUi(this);

	connect(autoSaveCB, SIGNAL(toggled(bool)),
		autoSaveLA, SLOT(setEnabled(bool)));
	connect(autoSaveCB, SIGNAL(toggled(bool)),
		autoSaveSB, SLOT(setEnabled(bool)));
	connect(autoSaveCB, SIGNAL(toggled(bool)),
		TextLabel1, SLOT(setEnabled(bool)));
	connect(uiFilePB, SIGNAL(clicked()),
		this, SLOT(select_ui()));
	connect(bindFilePB, SIGNAL(clicked()),
		this, SLOT(select_bind()));
	connect(uiFileED, SIGNAL(textChanged(const QString &)),
		this, SIGNAL(changed()));
	connect(bindFileED, SIGNAL(textChanged(const QString &)),
		this, SIGNAL(changed()));
	connect(restoreCursorCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(loadSessionCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(loadWindowSizeCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(loadWindowLocationCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(windowWidthSB, SIGNAL(valueChanged(int)),
		this, SIGNAL(changed()));
	connect(windowHeightSB, SIGNAL(valueChanged(int)),
		this, SIGNAL(changed()));
	connect(cursorFollowsCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(autoSaveSB, SIGNAL(valueChanged(int)),
		this, SIGNAL(changed()));
	connect(autoSaveCB, SIGNAL(clicked()),
		this, SIGNAL(changed()));
	connect(lastfilesSB, SIGNAL(valueChanged(int)),
		this, SIGNAL(changed()));
	connect(pixmapCacheCB, SIGNAL(toggled(bool)),
		this, SIGNAL(changed()));
	lastfilesSB->setMaximum(maxlastfiles);
}


void PrefUserInterface::apply(LyXRC & rc) const
{
	rc.ui_file = internal_path(fromqstr(uiFileED->text()));
	rc.bind_file = internal_path(fromqstr(bindFileED->text()));
	rc.use_lastfilepos = restoreCursorCB->isChecked();
	rc.load_session = loadSessionCB->isChecked();
	if (loadWindowSizeCB->isChecked()) {
		rc.geometry_width = 0;
		rc.geometry_height = 0;
	} else {
		rc.geometry_width = windowWidthSB->value();
		rc.geometry_height = windowHeightSB->value();
	}
	rc.geometry_xysaved = loadWindowLocationCB->isChecked();
	rc.cursor_follows_scrollbar = cursorFollowsCB->isChecked();
	rc.autosave = autoSaveSB->value() * 60;
	rc.make_backup = autoSaveCB->isChecked();
	rc.num_lastfiles = lastfilesSB->value();
	rc.use_pixmap_cache = pixmapCacheCB->isChecked();
}


void PrefUserInterface::update(LyXRC const & rc)
{
	uiFileED->setText(toqstr(external_path(rc.ui_file)));
	bindFileED->setText(toqstr(external_path(rc.bind_file)));
	restoreCursorCB->setChecked(rc.use_lastfilepos);
	loadSessionCB->setChecked(rc.load_session);
	bool loadWindowSize = rc.geometry_width == 0 && rc.geometry_height == 0;
	loadWindowSizeCB->setChecked(loadWindowSize);
	if (!loadWindowSize) {
		windowWidthSB->setValue(rc.geometry_width);
		windowHeightSB->setValue(rc.geometry_height);
	}
	loadWindowLocationCB->setChecked(rc.geometry_xysaved);
	cursorFollowsCB->setChecked(rc.cursor_follows_scrollbar);
	// convert to minutes
	int mins(rc.autosave / 60);
	if (rc.autosave && !mins)
		mins = 1;
	autoSaveSB->setValue(mins);
	autoSaveCB->setChecked(rc.make_backup);
	lastfilesSB->setValue(rc.num_lastfiles);
	pixmapCacheCB->setChecked(rc.use_pixmap_cache);
#if (QT_VERSION < 0x040200) || defined(Q_WS_X11)
	pixmapCacheGB->setEnabled(false);
#endif
}



void PrefUserInterface::select_ui()
{
	docstring const name =
		from_utf8(internal_path(fromqstr(uiFileED->text())));
	docstring file(form_->controller().browseUI(name));
	if (!file.empty())
		uiFileED->setText(toqstr(file));
}


void PrefUserInterface::select_bind()
{
	docstring const name =
		from_utf8(internal_path(fromqstr(bindFileED->text())));
	docstring file(form_->controller().browsebind(name));
	if (!file.empty())
		bindFileED->setText(toqstr(file));
}


void PrefUserInterface::on_loadWindowSizeCB_toggled(bool loadwindowsize)
{
	windowWidthLA->setDisabled(loadwindowsize);
	windowHeightLA->setDisabled(loadwindowsize);
	windowWidthSB->setDisabled(loadwindowsize);
	windowHeightSB->setDisabled(loadwindowsize);
}


PrefIdentity::PrefIdentity(QWidget * parent)
: PrefModule(docstring(), _("Identity"), 0, parent)
{
	setupUi(this);

	connect(nameED, SIGNAL(textChanged(const QString&)),
		this, SIGNAL(changed()));
	connect(emailED, SIGNAL(textChanged(const QString&)),
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
// QPrefsDialog
//
/////////////////////////////////////////////////////////////////////

QPrefsDialog::QPrefsDialog(QPrefs * form)
	: form_(form)
{
	setupUi(this);
	QDialog::setModal(false);

	connect(savePB, SIGNAL(clicked()), form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), form, SLOT(slotClose()));
	connect(restorePB, SIGNAL(clicked()), form, SLOT(slotRestore()));

	add(new PrefUserInterface(form_));
	add(new PrefScreenFonts(form_));
	add(new PrefColors(form_));
	add(new PrefDisplay);
	add(new PrefKeyboard(form_));

	add(new PrefPaths(form_));

	add(new PrefIdentity);

	add(new PrefLanguage);
	add(new PrefSpellchecker(form_));

	add(new PrefPrinter);
	add(new PrefDate);
	add(new PrefPlaintext);
	add(new PrefLatex(form_));

#if defined(__CYGWIN__) || defined(_WIN32)
	add(new PrefCygwinPath);
#endif


	PrefConverters * converters = new PrefConverters(form_);
	PrefFileformats * formats = new PrefFileformats(form_);
	connect(formats, SIGNAL(formatsChanged()),
			converters, SLOT(updateGui()));
	add(converters);
	add(formats);

	add(new PrefCopiers(form_));


	prefsPS->setCurrentPanel(_("User interface"));
// FIXME: hack to work around resizing bug in Qt >= 4.2
// bug verified with Qt 4.2.{0-3} (JSpitzm)
#if QT_VERSION >= 0x040200
	prefsPS->updateGeometry();
#endif

	form_->bcview().setOK(savePB);
	form_->bcview().setApply(applyPB);
	form_->bcview().setCancel(closePB);
	form_->bcview().setRestore(restorePB);
}


void QPrefsDialog::add(PrefModule * module)
{
	BOOST_ASSERT(module);

//	if (module->category().empty())
//		prefsPS->addPanel(module, module->title());
//	else
		prefsPS->addPanel(module, module->title(), module->category());

	connect(module, SIGNAL(changed()), this, SLOT(change_adaptor()));

	modules_.push_back(module);
}

void QPrefsDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QPrefsDialog::change_adaptor()
{
	form_->changed();
}


void QPrefsDialog::apply(LyXRC & rc) const
{
	size_t end = modules_.size();
	for (size_t i = 0; i != end; ++i)
		modules_[i]->apply(rc);
}


void QPrefsDialog::updateRc(LyXRC const & rc)
{
	size_t const end = modules_.size();
	for (size_t i = 0; i != end; ++i)
		modules_[i]->update(rc);
}


/////////////////////////////////////////////////////////////////////
//
// QPrefs
//
/////////////////////////////////////////////////////////////////////

typedef QController<ControlPrefs, QView<QPrefsDialog> > PrefsBase;

QPrefs::QPrefs(Dialog & parent)
	: PrefsBase(parent, _("Preferences"))
{
}

Converters & QPrefs::converters()
{
	return controller().converters();
}

Formats & QPrefs::formats()
{
	return controller().formats();
}

Movers & QPrefs::movers()
{
	return controller().movers();
}

void QPrefs::build_dialog()
{
	dialog_.reset(new QPrefsDialog(this));
}

void QPrefs::apply()
{
	dialog_->apply(controller().rc());
}

void QPrefs::update_contents()
{
	dialog_->updateRc(controller().rc());
}

} // namespace frontend
} // namespace lyx

#include "QPrefs_moc.cpp"
