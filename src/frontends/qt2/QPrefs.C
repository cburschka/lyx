/**
 * \file QPrefs.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "support/lstrings.h"
#include "support/os.h"
#include "Lsstream.h"
#include <iomanip>

#include "ControlPrefs.h"
#include "QPrefsDialog.h"
#include "ui/QPrefAsciiModule.h"
#include "ui/QPrefDateModule.h"
#include "ui/QPrefKeyboardModule.h"
#include "ui/QPrefLatexModule.h"
#include "ui/QPrefScreenFontsModule.h"
#include "ui/QPrefColorsModule.h"
#if defined(__CYGWIN__) || defined(__CYGWIN32__)
#include "ui/QPrefCygwinPathModule.h"	
#endif
#include "ui/QPrefDisplayModule.h"
#include "ui/QPrefPathsModule.h"
#include "ui/QPrefSpellcheckerModule.h"
#include "ui/QPrefConvertersModule.h"
#include "ui/QPrefFileformatsModule.h"
#include "ui/QPrefLanguageModule.h"
#include "ui/QPrefPrinterModule.h"
#include "ui/QPrefUIModule.h"
#include "QPrefs.h"
#include "Qt2BC.h"
#include "lyxrc.h"
#include "frnt_lang.h"
#include "helper_funcs.h"
#include "qt_helpers.h"
#include "debug.h"

#include <boost/tuple/tuple.hpp>

#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qlabel.h>
#include "qcoloritem.h"

using std::vector;
using std::pair;
using std::ostringstream;
using std::setfill;
using std::setw;
using std::endl;

typedef Qt2CB<ControlPrefs, Qt2DB<QPrefsDialog> > base_class;


QPrefs::QPrefs()
	: base_class(qt_("Preferences"))
{
}


void QPrefs::build_dialog()
{
	dialog_.reset(new QPrefsDialog(this));

	bc().setOK(dialog_->savePB);
	bc().setApply(dialog_->applyPB);
	bc().setCancel(dialog_->closePB);
	bc().setRestore(dialog_->restorePB);

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
}


namespace {

string const internal_path(QString const & input)
{
	return os::internal_path(fromqstr(input));
}

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

	rc.ui_file = internal_path(uimod->uiFileED->text());
	rc.bind_file = internal_path(uimod->bindFileED->text());
	rc.cursor_follows_scrollbar = uimod->cursorFollowsCB->isChecked();
	rc.wheel_jump = uimod->wheelMouseSB->value();
	rc.autosave = uimod->autoSaveSB->value() * 60;
	rc.make_backup = uimod->autoSaveCB->isChecked();
	rc.num_lastfiles = uimod->lastfilesSB->value();


	QPrefKeyboardModule * keymod(dialog_->keyboardModule);

	// FIXME: can derive CB from the two EDs
	rc.use_kbmap = keymod->keymapCB->isChecked();
	rc.primary_kbmap = internal_path(keymod->firstKeymapED->text());
	rc.secondary_kbmap = internal_path(keymod->secondKeymapED->text());


	QPrefAsciiModule * ascmod(dialog_->asciiModule);

	rc.ascii_linelen = ascmod->asciiLinelengthSB->value();
	rc.ascii_roff_command = fromqstr(ascmod->asciiRoffED->text());


	QPrefDateModule * datemod(dialog_->dateModule);

	rc.date_insert_format = fromqstr(datemod->DateED->text());

#if defined(__CYGWIN__) || defined(__CYGWIN32__)
	QPrefCygwinPathModule * cygwinmod(dialog_->cygwinpathModule);
	rc.cygwin_path_fix = cygwinmod->pathCB->isChecked();
#endif

	QPrefLatexModule * latexmod(dialog_->latexModule);

	rc.fontenc = fromqstr(latexmod->latexEncodingED->text());
	rc.chktex_command = fromqstr(latexmod->latexChecktexED->text());
	rc.auto_reset_options = latexmod->latexAutoresetCB->isChecked();
	rc.view_dvi_paper_option = fromqstr(latexmod->latexDviPaperED->text());
	rc.default_papersize =
		static_cast<BufferParams::PAPER_SIZE>(latexmod->latexPaperSizeCO->currentItem());


	QPrefDisplayModule * displaymod(dialog_->displayModule);

	rc.preview = displaymod->previewCB->isChecked();

	grfx::DisplayType dtype(grfx::ColorDisplay);

	switch (displaymod->displayGraphicsCO->currentItem()) {
		case 3:	dtype = grfx::NoDisplay; break;
		case 2:	dtype = grfx::ColorDisplay; break;
		case 1: dtype = grfx::GrayscaleDisplay;	break;
		case 0: dtype = grfx::MonochromeDisplay; break;
	}
	rc.display_graphics = dtype;

#ifdef WITH_WARNINGS
#warning FIXME!! The graphics cache no longer has a changeDisplay method.
#endif
#if 0
	if (old_value != rc.display_graphics) {
		grfx::GCache & gc = grfx::GCache::get();
		gc.changeDisplay();
	}
#endif
	QPrefPathsModule * pathsmod(dialog_->pathsModule);

	rc.document_path = internal_path(pathsmod->workingDirED->text());
	rc.template_path = internal_path(pathsmod->templateDirED->text());
	rc.backupdir_path = internal_path(pathsmod->backupDirED->text());
	rc.use_tempdir = pathsmod->tempDirCB->isChecked();
	rc.tempdir_path = internal_path(pathsmod->tempDirED->text());
	rc.path_prefix = fromqstr(pathsmod->pathPrefixED->text());
	// FIXME: should be a checkbox only
	rc.lyxpipes = internal_path(pathsmod->lyxserverDirED->text());

	QPrefSpellcheckerModule * spellmod(dialog_->spellcheckerModule);

	rc.isp_command = fromqstr(spellmod->spellCommandCO->currentText());

	// FIXME: remove isp_use_alt_lang
	rc.isp_alt_lang = fromqstr(spellmod->altLanguageED->text());
	rc.isp_use_alt_lang = !rc.isp_alt_lang.empty();
	// FIXME: remove isp_use_esc_chars
	rc.isp_esc_chars = fromqstr(spellmod->escapeCharactersED->text());
	rc.isp_use_esc_chars = !rc.isp_esc_chars.empty();
	// FIXME: remove isp_use_pers_dict
	rc.isp_pers_dict = internal_path(spellmod->persDictionaryED->text());
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
	rc.print_to_file = internal_path(printmod->printerToFileED->text());
	rc.print_extra_options = fromqstr(printmod->printerExtraED->text());
	rc.print_spool_printerprefix = fromqstr(printmod->printerSpoolPrefixED->text());
	rc.print_paper_dimension_flag = fromqstr(printmod->printerPaperSizeED->text());


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

	controller().setFormats(formats_);
	controller().setConverters(converters_);

	QPrefColorsModule * colmod(dialog_->colorsModule);

	unsigned int i;

	for (i = 0; i < colmod->lyxObjectsLB->count(); ++i) {
		QListBoxItem * ib(colmod->lyxObjectsLB->item(i));
		QColorItem * ci(static_cast<QColorItem*>(ib));

		LColor::color const col(dialog_->colors_[i]);
		QColor const qcol(toqstr(lcolor.getX11Name(col)));

		// FIXME: dubious, but it's what xforms does
		if (qcol != ci->color()) {
			ostringstream ostr;
			
			ostr << '#' << std::setbase(16) << setfill('0')
			     << setw(2) << ci->color().red()
			     << setw(2) << ci->color().green()
			     << setw(2) << ci->color().blue();

			string newhex(STRCONV(ostr.str()));
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
	typename std::vector<A>::const_iterator it =
		std::find(vec.begin(), vec.end(), val);
	if (it == vec.end())
		return 0;
	return std::distance(vec.begin(), it);
}

void setComboxFont(QComboBox * cb, string const & family,
		string const & foundry, QFont::StyleHint hint)
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

	// Try the hint
	QFont font;
	font.setStyleHint(hint);
	QFontInfo fi(font);
	// The combobox stores only the font name; the foundry info is
	// discarded. So extract the font name from fi.family().
	pair<string, string> tmp = parseFontName(fi.family());
	string const & default_font_name = tmp.first;

	for (int i = cb->count() - 1; i >= 0; --i) {
		if (cb->text(i) == default_font_name) {
			cb->setCurrentItem(i);
			return;
		}
	}
}

}


namespace {

QString const external_path(string const & input)
{
	return toqstr(os::external_path(input));
}

}


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

	uimod->uiFileED->setText(external_path(rc.ui_file));
	uimod->bindFileED->setText(external_path(rc.bind_file));
	uimod->cursorFollowsCB->setChecked(rc.cursor_follows_scrollbar);
	uimod->wheelMouseSB->setValue(rc.wheel_jump);
	// convert to minutes
	int mins(rc.autosave / 60);
	if (rc.autosave && !mins)
		mins = 1;
	uimod->autoSaveSB->setValue(mins);
	uimod->autoSaveCB->setChecked(rc.make_backup);
	uimod->lastfilesSB->setValue(rc.num_lastfiles);


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
	keymod->firstKeymapED->setText(external_path(rc.primary_kbmap));
	keymod->secondKeymapED->setText(external_path(rc.secondary_kbmap));


	QPrefAsciiModule * ascmod(dialog_->asciiModule);

	ascmod->asciiLinelengthSB->setValue(rc.ascii_linelen);
	ascmod->asciiRoffED->setText(toqstr(rc.ascii_roff_command));


	QPrefDateModule * datemod(dialog_->dateModule);

	datemod->DateED->setText(toqstr(rc.date_insert_format));

#if defined(__CYGWIN__) || defined(__CYGWIN32__)
	QPrefCygwinPathModule * cygwinmod(dialog_->cygwinpathModule);
	cygwinmod->pathCB->setChecked(rc.cygwin_path_fix);
#endif

	QPrefLatexModule * latexmod(dialog_->latexModule);

	latexmod->latexEncodingED->setText(toqstr(rc.fontenc));
	latexmod->latexChecktexED->setText(toqstr(rc.chktex_command));
	latexmod->latexAutoresetCB->setChecked(rc.auto_reset_options);
	latexmod->latexDviPaperED->setText(toqstr(rc.view_dvi_paper_option));
	latexmod->latexPaperSizeCO->setCurrentItem(rc.default_papersize);


	QPrefDisplayModule * displaymod(dialog_->displayModule);

	displaymod->previewCB->setChecked(rc.preview);

	int item = 2;

	switch (rc.display_graphics) {
		case grfx::NoDisplay:		item = 3; break;
		case grfx::ColorDisplay:	item = 2; break;
		case grfx::GrayscaleDisplay:	item = 1; break;
		case grfx::MonochromeDisplay:	item = 0; break;
		default: break;
	}
	displaymod->displayGraphicsCO->setCurrentItem(item);


	QPrefPathsModule * pathsmod(dialog_->pathsModule);

	pathsmod->workingDirED->setText(external_path(rc.document_path));
	pathsmod->templateDirED->setText(external_path(rc.template_path));
	pathsmod->backupDirED->setText(external_path(rc.backupdir_path));
	pathsmod->tempDirCB->setChecked(rc.use_tempdir);
	pathsmod->tempDirED->setText(external_path(rc.tempdir_path));
	pathsmod->pathPrefixED->setText(toqstr(rc.path_prefix));
	// FIXME: should be a checkbox only
	pathsmod->lyxserverDirED->setText(external_path(rc.lyxpipes));

	QPrefSpellcheckerModule * spellmod(dialog_->spellcheckerModule);

	QString const tmp = qt_(rc.isp_command);
	for (int i = 0; i < spellmod->spellCommandCO->count(); ++i) {
		if (spellmod->spellCommandCO->text(i) == tmp) {
			spellmod->spellCommandCO->setCurrentItem(i);
			break;
		}
	}
	// FIXME: remove isp_use_alt_lang
	spellmod->altLanguageED->setText(toqstr(rc.isp_alt_lang));
	// FIXME: remove isp_use_esc_chars
	spellmod->escapeCharactersED->setText(toqstr(rc.isp_esc_chars));
	// FIXME: remove isp_use_pers_dict
	spellmod->persDictionaryED->setText(external_path(rc.isp_pers_dict));
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
	printmod->printerToFileED->setText(external_path(rc.print_to_file));
	printmod->printerExtraED->setText(toqstr(rc.print_extra_options));
	printmod->printerSpoolPrefixED->setText(toqstr(rc.print_spool_printerprefix));
	printmod->printerPaperSizeED->setText(toqstr(rc.print_paper_dimension_flag));


	QPrefScreenFontsModule * fontmod(dialog_->screenfontsModule);

	setComboxFont(fontmod->screenRomanCO, rc.roman_font_name,
			rc.roman_font_foundry, QFont::Serif);
	setComboxFont(fontmod->screenSansCO, rc.sans_font_name,
			rc.sans_font_foundry, QFont::SansSerif);
	setComboxFont(fontmod->screenTypewriterCO, rc.typewriter_font_name,
			rc.typewriter_font_foundry, QFont::TypeWriter);

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

	formats_ = formats;

	dialog_->updateFormats();

	converters_ = converters;

	dialog_->updateConverters();
}
