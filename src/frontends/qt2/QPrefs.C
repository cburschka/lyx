/**
 * \file QPrefs.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "debug.h"
#include "support/lstrings.h"
#include "support/tostr.h"
#include "support/std_sstream.h"
#include <iomanip>

#include "ControlPrefs.h"
#include "QPrefsDialog.h"
#include "ui/QPrefAsciiModule.h"
#include "ui/QPrefDateModule.h"
#include "ui/QPrefKeyboardModule.h"
#include "ui/QPrefLatexModule.h"
#include "ui/QPrefScreenFontsModule.h"
#include "ui/QPrefColorsModule.h"
#include "ui/QPrefDisplayModule.h"
#include "ui/QPrefPathsModule.h"
#include "ui/QPrefSpellcheckerModule.h"
#include "ui/QPrefLanguageModule.h"
#include "ui/QPrefPrinterModule.h"
#include "ui/QPrefUIModule.h"
#include "ui/QPrefIdentityModule.h"
#include "lyx_gui.h"
#include "QPrefs.h"
#include "Qt2BC.h"
#include "lyxfont.h"
#include "frnt_lang.h"
#include "helper_funcs.h"
#include "qt_helpers.h"
#include "lcolorcache.h"

#include <boost/tuple/tuple.hpp>

#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include "qcoloritem.h"

#include "LColor.h"

using lyx::support::compare_no_case;
using lyx::support::strToDbl;

using std::distance;
using std::endl;
using std::setfill;
using std::setw;
using std::string;
using std::ostringstream;
using std::pair;
using std::vector;


typedef QController<ControlPrefs, QView<QPrefsDialog> > base_class;

QPrefs::QPrefs(Dialog & parent)
	: base_class(parent, _("LyX: Preferences"))
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


void QPrefs::build_dialog()
{
	dialog_.reset(new QPrefsDialog(this));

	bcview().setOK(dialog_->savePB);
	bcview().setApply(dialog_->applyPB);
	bcview().setCancel(dialog_->closePB);
	bcview().setRestore(dialog_->restorePB);

	QPrefLanguageModule * langmod(dialog_->languageModule);

	langmod->defaultLanguageCO->clear();
	// store the lang identifiers for later
	vector<frnt::LanguagePair> const langs = frnt::getLanguageData(false);
	lang_ = getSecond(langs);

	vector<frnt::LanguagePair>::const_iterator lit  = langs.begin();
	vector<frnt::LanguagePair>::const_iterator lend = langs.end();
	for (; lit != lend; ++lit) {
		langmod->defaultLanguageCO->insertItem(toqstr(lit->first));
	}

	QPrefSpellcheckerModule * spellmod(dialog_->spellcheckerModule);
	spellmod->spellCommandCO->insertItem(qt_("ispell"));
	spellmod->spellCommandCO->insertItem(qt_("aspell"));
	spellmod->spellCommandCO->insertItem(qt_("hspell"));
#ifdef USE_PSPELL
	spellmod->spellCommandCO->insertItem(qt_("pspell (library)"));
#else
#ifdef USE_ASPELL
	spellmod->spellCommandCO->insertItem(qt_("aspell (library)"));
#endif
#endif
}


void QPrefs::apply()
{
	LyXRC & rc(controller().rc());

	QPrefLanguageModule * langmod(dialog_->languageModule);

	// FIXME: remove rtl_support bool
	rc.rtl_support = langmod->rtlCB->isChecked();
	rc.mark_foreign_language = langmod->markForeignCB->isChecked();
	rc.language_auto_begin = langmod->autoBeginCB->isChecked();
	rc.language_auto_end = langmod->autoEndCB->isChecked();
	rc.language_use_babel = langmod->useBabelCB->isChecked();
	rc.language_global_options = langmod->globalCB->isChecked();
	rc.language_package = fromqstr(langmod->languagePackageED->text());
	rc.language_command_begin = fromqstr(langmod->startCommandED->text());
	rc.language_command_end = fromqstr(langmod->endCommandED->text());
	rc.default_language = lang_[langmod->defaultLanguageCO->currentItem()];

	QPrefUIModule * uimod(dialog_->uiModule);

	rc.ui_file = fromqstr(uimod->uiFileED->text());
	rc.bind_file = fromqstr(uimod->bindFileED->text());
	rc.cursor_follows_scrollbar = uimod->cursorFollowsCB->isChecked();
	rc.wheel_jump = uimod->wheelMouseSB->value();
	rc.autosave = uimod->autoSaveSB->value() * 60;
	rc.make_backup = uimod->autoSaveCB->isChecked();
	rc.num_lastfiles = uimod->lastfilesSB->value();


	QPrefKeyboardModule * keymod(dialog_->keyboardModule);

	// FIXME: can derive CB from the two EDs
	rc.use_kbmap = keymod->keymapCB->isChecked();
	rc.primary_kbmap = fromqstr(keymod->firstKeymapED->text());
	rc.secondary_kbmap = fromqstr(keymod->secondKeymapED->text());


	QPrefAsciiModule * ascmod(dialog_->asciiModule);

	rc.ascii_linelen = ascmod->asciiLinelengthSB->value();
	rc.ascii_roff_command = fromqstr(ascmod->asciiRoffED->text());


	QPrefDateModule * datemod(dialog_->dateModule);

	rc.date_insert_format = fromqstr(datemod->DateED->text());


	QPrefLatexModule * latexmod(dialog_->latexModule);

	rc.fontenc = fromqstr(latexmod->latexEncodingED->text());
	rc.chktex_command = fromqstr(latexmod->latexChecktexED->text());
	rc.bibtex_command = fromqstr(latexmod->latexBibtexED->text());
	rc.auto_reset_options = latexmod->latexAutoresetCB->isChecked();
	rc.view_dvi_paper_option = fromqstr(latexmod->latexDviPaperED->text());
	rc.default_papersize =
		static_cast<PAPER_SIZE>(latexmod->latexPaperSizeCO->currentItem());

	QPrefDisplayModule * displaymod(dialog_->displayModule);

	switch (displaymod->instantPreviewCO->currentItem()) {
	case 0:
		rc.preview = LyXRC::PREVIEW_OFF;
		break;
	case 1:
		rc.preview = LyXRC::PREVIEW_NO_MATH;
		break;
	case 2:
		rc.preview = LyXRC::PREVIEW_ON;
		break;
	}

	lyx::graphics::DisplayType dtype(lyx::graphics::ColorDisplay);

	switch (displaymod->displayGraphicsCO->currentItem()) {
	case 3:	dtype = lyx::graphics::NoDisplay; break;
	case 2:	dtype = lyx::graphics::ColorDisplay; break;
	case 1: dtype = lyx::graphics::GrayscaleDisplay;	break;
	case 0: dtype = lyx::graphics::MonochromeDisplay; break;
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

	QPrefPathsModule * pathsmod(dialog_->pathsModule);

	rc.document_path = fromqstr(pathsmod->workingDirED->text());
	rc.template_path = fromqstr(pathsmod->templateDirED->text());
	rc.backupdir_path = fromqstr(pathsmod->backupDirED->text());
	rc.tempdir_path = fromqstr(pathsmod->tempDirED->text());
	// FIXME: should be a checkbox only
	rc.lyxpipes = fromqstr(pathsmod->lyxserverDirED->text());


	QPrefSpellcheckerModule * spellmod(dialog_->spellcheckerModule);

	switch (spellmod->spellCommandCO->currentItem()) {
		case 0:
		case 1:
		case 2:
			rc.use_spell_lib = false;
			rc.isp_command = fromqstr(spellmod->spellCommandCO->currentText());
			break;
		case 3:
			rc.use_spell_lib = true;
			break;
	}

	// FIXME: remove isp_use_alt_lang
	rc.isp_alt_lang = fromqstr(spellmod->altLanguageED->text());
	rc.isp_use_alt_lang = !rc.isp_alt_lang.empty();
	// FIXME: remove isp_use_esc_chars
	rc.isp_esc_chars = fromqstr(spellmod->escapeCharactersED->text());
	rc.isp_use_esc_chars = !rc.isp_esc_chars.empty();
	// FIXME: remove isp_use_pers_dict
	rc.isp_pers_dict = fromqstr(spellmod->persDictionaryED->text());
	rc.isp_use_pers_dict = !rc.isp_pers_dict.empty();
	rc.isp_accept_compound = spellmod->compoundWordCB->isChecked();
	rc.isp_use_input_encoding = spellmod->inputEncodingCB->isChecked();


	QPrefPrinterModule * printmod(dialog_->printerModule);

	rc.print_adapt_output = printmod->printerAdaptCB->isChecked();
	rc.print_command = fromqstr(printmod->printerCommandED->text());
	rc.printer = fromqstr(printmod->printerNameED->text());

	rc.print_pagerange_flag = fromqstr(printmod->printerPageRangeED->text());
	rc.print_copies_flag = fromqstr(printmod->printerCopiesED->text());
	rc.print_reverse_flag = fromqstr(printmod->printerReverseED->text());
	rc.print_to_printer = fromqstr(printmod->printerToPrinterED->text());
	rc.print_file_extension = fromqstr(printmod->printerExtensionED->text());
	rc.print_spool_command = fromqstr(printmod->printerSpoolCommandED->text());
	rc.print_paper_flag = fromqstr(printmod->printerPaperTypeED->text());
	rc.print_evenpage_flag = fromqstr(printmod->printerEvenED->text());
	rc.print_oddpage_flag = fromqstr(printmod->printerOddED->text());
	rc.print_collcopies_flag = fromqstr(printmod->printerCollatedED->text());
	rc.print_landscape_flag = fromqstr(printmod->printerLandscapeED->text());
	rc.print_to_file = fromqstr(printmod->printerToFileED->text());
	rc.print_extra_options = fromqstr(printmod->printerExtraED->text());
	rc.print_spool_printerprefix = fromqstr(printmod->printerSpoolPrefixED->text());
	rc.print_paper_dimension_flag = fromqstr(printmod->printerPaperSizeED->text());


	QPrefIdentityModule * idmod(dialog_->identityModule);
	rc.user_name = fromqstr(idmod->nameED->text());
	rc.user_email = fromqstr(idmod->emailED->text());

	QPrefScreenFontsModule * fontmod(dialog_->screenfontsModule);

	LyXRC const oldrc(rc);

	boost::tie(rc.roman_font_name, rc.roman_font_foundry)
		= parseFontName(fromqstr(fontmod->screenRomanCO->currentText()));
	boost::tie(rc.sans_font_name, rc.sans_font_foundry) =
		parseFontName(fromqstr(fontmod->screenSansCO->currentText()));
	boost::tie(rc.typewriter_font_name, rc.typewriter_font_foundry) =
		parseFontName(fromqstr(fontmod->screenTypewriterCO->currentText()));

	rc.zoom = fontmod->screenZoomSB->value();
	rc.dpi = fontmod->screenDpiSB->value();
	rc.font_sizes[LyXFont::SIZE_TINY] = strToDbl(fromqstr(fontmod->screenTinyED->text()));
	rc.font_sizes[LyXFont::SIZE_SCRIPT] = strToDbl(fromqstr(fontmod->screenSmallestED->text()));
	rc.font_sizes[LyXFont::SIZE_FOOTNOTE] = strToDbl(fromqstr(fontmod->screenSmallerED->text()));
	rc.font_sizes[LyXFont::SIZE_SMALL] = strToDbl(fromqstr(fontmod->screenSmallED->text()));
	rc.font_sizes[LyXFont::SIZE_NORMAL] = strToDbl(fromqstr(fontmod->screenNormalED->text()));
	rc.font_sizes[LyXFont::SIZE_LARGE] = strToDbl(fromqstr(fontmod->screenLargeED->text()));
	rc.font_sizes[LyXFont::SIZE_LARGER] = strToDbl(fromqstr(fontmod->screenLargerED->text()));
	rc.font_sizes[LyXFont::SIZE_LARGEST] = strToDbl(fromqstr(fontmod->screenLargestED->text()));
	rc.font_sizes[LyXFont::SIZE_HUGE] = strToDbl(fromqstr(fontmod->screenHugeED->text()));
	rc.font_sizes[LyXFont::SIZE_HUGER] = strToDbl(fromqstr(fontmod->screenHugerED->text()));

	if (rc.font_sizes != oldrc.font_sizes
		|| rc.roman_font_name != oldrc.roman_font_name
		|| rc.sans_font_name != oldrc.sans_font_name
		|| rc.typewriter_font_name != oldrc.typewriter_font_name
		|| rc.zoom != oldrc.zoom || rc.dpi != oldrc.dpi) {
		controller().updateScreenFonts();
	}

	QPrefColorsModule * colmod(dialog_->colorsModule);

	unsigned int i;

	for (i = 0; i < colmod->lyxObjectsLB->count(); ++i) {
		QListBoxItem * ib(colmod->lyxObjectsLB->item(i));
		QColorItem * ci(static_cast<QColorItem*>(ib));

		LColor::color const col(dialog_->colors_[i]);
		QColor const & qcol(lcolorcache.get(col));

		// FIXME: dubious, but it's what xforms does
		if (qcol != ci->color()) {
			ostringstream ostr;

			ostr << '#' << std::setbase(16) << setfill('0')
			     << setw(2) << ci->color().red()
			     << setw(2) << ci->color().green()
			     << setw(2) << ci->color().blue();

			string newhex(ostr.str());
			controller().setColor(col, newhex);
		}
	}
}


// FIXME: move to helper_funcs.h
namespace {

template<class A>
typename std::vector<A>::size_type
findPos(std::vector<A> const & vec, A const & val)
{
	typedef typename std::vector<A>::const_iterator Cit;

	Cit it = std::find(vec.begin(), vec.end(), val);
	if (it == vec.end())
		return 0;
	return distance(vec.begin(), it);
}

void setComboxFont(QComboBox * cb, string const & family, string const & foundry)
{
	string const name = makeFontName(family, foundry);
	for (int i = 0; i < cb->count(); ++i) {
		if (fromqstr(cb->text(i)) == name) {
			cb->setCurrentItem(i);
			return;
		}
	}

	// Try matching without foundry name

	// We count in reverse in order to prefer the Xft foundry
	for (int i = cb->count() - 1; i >= 0; --i) {
		pair<string, string> tmp = parseFontName(fromqstr(cb->text(i)));
		if (compare_no_case(tmp.first, family) == 0) {
			cb->setCurrentItem(i);
			return;
		}
	}

	// family alone can contain e.g. "Helvetica [Adobe]"
	pair<string, string> tmpfam = parseFontName(family);

	// We count in reverse in order to prefer the Xft foundry
	for (int i = cb->count() - 1; i >= 0; --i) {
		pair<string, string> tmp = parseFontName(fromqstr(cb->text(i)));
		if (compare_no_case(tmp.first, tmpfam.first) == 0) {
			cb->setCurrentItem(i);
			return;
		}
	}

	// Bleh, default fonts, and the names couldn't be found. Hack
	// for bug 1063. Qt makes baby Jesus cry.

	QFont font;

	if (family == lyx_gui::roman_font_name()) {
		font.setStyleHint(QFont::Serif);
		font.setFamily(lyx_gui::roman_font_name().c_str());
	} else if (family == lyx_gui::sans_font_name()) {
		font.setStyleHint(QFont::SansSerif);
		font.setFamily(lyx_gui::sans_font_name().c_str());
	} else if (family == lyx_gui::typewriter_font_name()) {
		font.setStyleHint(QFont::TypeWriter);
		font.setFamily(lyx_gui::typewriter_font_name().c_str());
	} else {
		lyxerr << "FAILED to find the default font !"
			<< foundry << ", " << family << endl;
		return;
	}

	QFontInfo info(font);
	lyxerr << "Apparent font is " << info.family() << endl;

	for (int i = 0; i < cb->count(); ++i) {
		lyxerr << "Looking at " << fromqstr(cb->text(i)) << endl;
		if (compare_no_case(cb->text(i).latin1(), info.family().latin1()) == 0) {
			cb->setCurrentItem(i);
			return;
		}
	}

	lyxerr << "FAILED to find the font !"
		<< foundry << ", " << family << endl;
}

} // end namespace anon


void QPrefs::update_contents()
{
	LyXRC const & rc(controller().rc());

	QPrefLanguageModule * langmod(dialog_->languageModule);

	// FIXME: remove rtl_support bool
	langmod->rtlCB->setChecked(rc.rtl_support);
	langmod->markForeignCB->setChecked(rc.mark_foreign_language);
	langmod->autoBeginCB->setChecked(rc.language_auto_begin);
	langmod->autoEndCB->setChecked(rc.language_auto_end);
	langmod->useBabelCB->setChecked(rc.language_use_babel);
	langmod->globalCB->setChecked(rc.language_global_options);
	langmod->languagePackageED->setText(toqstr(rc.language_package));
	langmod->startCommandED->setText(toqstr(rc.language_command_begin));
	langmod->endCommandED->setText(toqstr(rc.language_command_end));

	int const pos = int(findPos(lang_, rc.default_language));
	langmod->defaultLanguageCO->setCurrentItem(pos);

	QPrefUIModule * uimod(dialog_->uiModule);

	uimod->uiFileED->setText(toqstr(rc.ui_file));
	uimod->bindFileED->setText(toqstr(rc.bind_file));
	uimod->cursorFollowsCB->setChecked(rc.cursor_follows_scrollbar);
	uimod->wheelMouseSB->setValue(rc.wheel_jump);
	// convert to minutes
	int mins(rc.autosave / 60);
	if (rc.autosave && !mins)
		mins = 1;
	uimod->autoSaveSB->setValue(mins);
	uimod->autoSaveCB->setChecked(rc.make_backup);
	uimod->lastfilesSB->setValue(rc.num_lastfiles);


	QPrefIdentityModule * idmod(dialog_->identityModule);
	idmod->nameED->setText(toqstr(rc.user_name));
	idmod->emailED->setText(toqstr(rc.user_email));


	QPrefKeyboardModule * keymod(dialog_->keyboardModule);

	// FIXME: can derive CB from the two EDs
	keymod->keymapCB->setChecked(rc.use_kbmap);
	// no idea why we need these. Fscking Qt.
	keymod->firstKeymapED->setEnabled(rc.use_kbmap);
	keymod->firstKeymapPB->setEnabled(rc.use_kbmap);
	keymod->firstKeymapLA->setEnabled(rc.use_kbmap);
	keymod->secondKeymapED->setEnabled(rc.use_kbmap);
	keymod->secondKeymapPB->setEnabled(rc.use_kbmap);
	keymod->secondKeymapLA->setEnabled(rc.use_kbmap);
	keymod->firstKeymapED->setText(toqstr(rc.primary_kbmap));
	keymod->secondKeymapED->setText(toqstr(rc.secondary_kbmap));


	QPrefAsciiModule * ascmod(dialog_->asciiModule);

	ascmod->asciiLinelengthSB->setValue(rc.ascii_linelen);
	ascmod->asciiRoffED->setText(toqstr(rc.ascii_roff_command));


	QPrefDateModule * datemod(dialog_->dateModule);

	datemod->DateED->setText(toqstr(rc.date_insert_format));


	QPrefLatexModule * latexmod(dialog_->latexModule);

	latexmod->latexEncodingED->setText(toqstr(rc.fontenc));
	latexmod->latexChecktexED->setText(toqstr(rc.chktex_command));
	latexmod->latexBibtexED->setText(toqstr(rc.bibtex_command));
	latexmod->latexAutoresetCB->setChecked(rc.auto_reset_options);
	latexmod->latexDviPaperED->setText(toqstr(rc.view_dvi_paper_option));
	latexmod->latexPaperSizeCO->setCurrentItem(rc.default_papersize);


	QPrefDisplayModule * displaymod(dialog_->displayModule);

	switch (rc.preview) {
	case LyXRC::PREVIEW_OFF:
		displaymod->instantPreviewCO->setCurrentItem(0);
		break;
	case LyXRC::PREVIEW_NO_MATH :
		displaymod->instantPreviewCO->setCurrentItem(1);
		break;
	case LyXRC::PREVIEW_ON :
		displaymod->instantPreviewCO->setCurrentItem(2);
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
	displaymod->displayGraphicsCO->setCurrentItem(item);


	QPrefPathsModule * pathsmod(dialog_->pathsModule);

	pathsmod->workingDirED->setText(toqstr(rc.document_path));
	pathsmod->templateDirED->setText(toqstr(rc.template_path));
	pathsmod->backupDirED->setText(toqstr(rc.backupdir_path));
	pathsmod->tempDirED->setText(toqstr(rc.tempdir_path));
	// FIXME: should be a checkbox only
	pathsmod->lyxserverDirED->setText(toqstr(rc.lyxpipes));


	QPrefSpellcheckerModule * spellmod(dialog_->spellcheckerModule);

	spellmod->spellCommandCO->setCurrentItem(0);

	if (rc.isp_command == "ispell") {
		spellmod->spellCommandCO->setCurrentItem(0);
	} else if (rc.isp_command == "aspell") {
		spellmod->spellCommandCO->setCurrentItem(1);
	} else if (rc.isp_command == "hspell") {
		spellmod->spellCommandCO->setCurrentItem(2);
	}

	if (rc.use_spell_lib) {
#if defined(USE_ASPELL) || defined(USE_PSPELL)
		spellmod->spellCommandCO->setCurrentItem(3);
#endif
	}

	// FIXME: remove isp_use_alt_lang
	spellmod->altLanguageED->setText(toqstr(rc.isp_alt_lang));
	// FIXME: remove isp_use_esc_chars
	spellmod->escapeCharactersED->setText(toqstr(rc.isp_esc_chars));
	// FIXME: remove isp_use_pers_dict
	spellmod->persDictionaryED->setText(toqstr(rc.isp_pers_dict));
	spellmod->compoundWordCB->setChecked(rc.isp_accept_compound);
	spellmod->inputEncodingCB->setChecked(rc.isp_use_input_encoding);


	QPrefPrinterModule * printmod(dialog_->printerModule);

	printmod->printerAdaptCB->setChecked(rc.print_adapt_output);
	printmod->printerCommandED->setText(toqstr(rc.print_command));
	printmod->printerNameED->setText(toqstr(rc.printer));

	printmod->printerPageRangeED->setText(toqstr(rc.print_pagerange_flag));
	printmod->printerCopiesED->setText(toqstr(rc.print_copies_flag));
	printmod->printerReverseED->setText(toqstr(rc.print_reverse_flag));
	printmod->printerToPrinterED->setText(toqstr(rc.print_to_printer));
	printmod->printerExtensionED->setText(toqstr(rc.print_file_extension));
	printmod->printerSpoolCommandED->setText(toqstr(rc.print_spool_command));
	printmod->printerPaperTypeED->setText(toqstr(rc.print_paper_flag));
	printmod->printerEvenED->setText(toqstr(rc.print_evenpage_flag));
	printmod->printerOddED->setText(toqstr(rc.print_oddpage_flag));
	printmod->printerCollatedED->setText(toqstr(rc.print_collcopies_flag));
	printmod->printerLandscapeED->setText(toqstr(rc.print_landscape_flag));
	printmod->printerToFileED->setText(toqstr(rc.print_to_file));
	printmod->printerExtraED->setText(toqstr(rc.print_extra_options));
	printmod->printerSpoolPrefixED->setText(toqstr(rc.print_spool_printerprefix));
	printmod->printerPaperSizeED->setText(toqstr(rc.print_paper_dimension_flag));


	QPrefScreenFontsModule * fontmod(dialog_->screenfontsModule);

	setComboxFont(fontmod->screenRomanCO, rc.roman_font_name,
			rc.roman_font_foundry);
	setComboxFont(fontmod->screenSansCO, rc.sans_font_name,
			rc.sans_font_foundry);
	setComboxFont(fontmod->screenTypewriterCO, rc.typewriter_font_name,
			rc.typewriter_font_foundry);

	dialog_->select_roman(fontmod->screenRomanCO->currentText());
	dialog_->select_sans(fontmod->screenSansCO->currentText());
	dialog_->select_typewriter(fontmod->screenTypewriterCO->currentText());

	fontmod->screenZoomSB->setValue(rc.zoom);
	fontmod->screenDpiSB->setValue(int(rc.dpi));
	fontmod->screenTinyED->setText(toqstr(tostr(rc.font_sizes[LyXFont::SIZE_TINY])));
	fontmod->screenSmallestED->setText(toqstr(tostr(rc.font_sizes[LyXFont::SIZE_SCRIPT])));
	fontmod->screenSmallerED->setText(toqstr(tostr(rc.font_sizes[LyXFont::SIZE_FOOTNOTE])));
	fontmod->screenSmallED->setText(toqstr(tostr(rc.font_sizes[LyXFont::SIZE_SMALL])));
	fontmod->screenNormalED->setText(toqstr(tostr(rc.font_sizes[LyXFont::SIZE_NORMAL])));
	fontmod->screenLargeED->setText(toqstr(tostr(rc.font_sizes[LyXFont::SIZE_LARGE])));
	fontmod->screenLargerED->setText(toqstr(tostr(rc.font_sizes[LyXFont::SIZE_LARGER])));
	fontmod->screenLargestED->setText(toqstr(tostr(rc.font_sizes[LyXFont::SIZE_LARGEST])));
	fontmod->screenHugeED->setText(toqstr(tostr(rc.font_sizes[LyXFont::SIZE_HUGE])));
	fontmod->screenHugerED->setText(toqstr(tostr(rc.font_sizes[LyXFont::SIZE_HUGER])));

	dialog_->updateFormats();

	dialog_->updateConverters();
}
