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

#ifdef __GNUG__
#pragma implementation
#endif

#include "support/lstrings.h"
 
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

#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qcombobox.h> 
#include <qlistbox.h>
 
typedef Qt2CB<ControlPrefs, Qt2DB<QPrefsDialog> > base_class;


QPrefs::QPrefs()
	: base_class(_("LyX: Preferences"))
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
		langmod->defaultLanguageCO->insertItem(lit->first.c_str());
	}
}


void QPrefs::apply()
{
	LyXRC & rc(controller().rc());

	// do something ... 

	QPrefLanguageModule * langmod(dialog_->languageModule);
 
	// FIXME: remove rtl_support bool
	rc.rtl_support = langmod->rtlCB->isChecked();
	rc.mark_foreign_language = langmod->markForeignCB->isChecked(); 
	rc.language_auto_begin = langmod->autoBeginCB->isChecked(); 
	rc.language_auto_end = langmod->autoEndCB->isChecked(); 
	rc.language_use_babel = langmod->useBabelCB->isChecked();
	rc.language_global_options = langmod->globalCB->isChecked();
	rc.language_package = langmod->languagePackageED->text().latin1();
	rc.language_command_begin = langmod->startCommandED->text().latin1();
	rc.language_command_end = langmod->endCommandED->text().latin1();
        rc.default_language = lang_[langmod->defaultLanguageCO->currentItem()];

	QPrefUIModule * uimod(dialog_->uiModule); 

	rc.bind_file = uimod->uiFileED->text().latin1();
	rc.ui_file = uimod->bindFileED->text().latin1();
	rc.cursor_follows_scrollbar = uimod->cursorFollowsCB->isChecked(); 
	rc.wheel_jump = uimod->wheelMouseSB->value();
	rc.autosave = uimod->autoSaveSB->value() * 60;
	rc.make_backup = uimod->autoSaveCB->isChecked();
	rc.num_lastfiles = uimod->lastfilesSB->value();
 
 
	QPrefKeyboardModule * keymod(dialog_->keyboardModule);

	// FIXME: can derive CB from the two EDs 
	rc.use_kbmap = keymod->keymapCB->isChecked();
	rc.primary_kbmap = keymod->firstKeymapED->text().latin1();
	rc.secondary_kbmap = keymod->secondKeymapED->text().latin1();
 

	QPrefAsciiModule * ascmod(dialog_->asciiModule);

	rc.ascii_linelen = ascmod->asciiLinelengthSB->value();
	rc.ascii_roff_command = ascmod->asciiRoffED->text().latin1();


	QPrefDateModule * datemod(dialog_->dateModule);

	rc.date_insert_format = datemod->DateED->text().latin1();


	QPrefLatexModule * latexmod(dialog_->latexModule);

	rc.fontenc = latexmod->latexEncodingED->text().latin1();
	rc.chktex_command = latexmod->latexChecktexED->text().latin1(); 
	rc.auto_reset_options = latexmod->latexAutoresetCB->isChecked();
	rc.view_dvi_paper_option = latexmod->latexDviPaperED->text().latin1();
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
 
	rc.document_path = pathsmod->workingDirED->text().latin1();
	rc.template_path = pathsmod->templateDirED->text().latin1();
	rc.backupdir_path = pathsmod->backupDirED->text().latin1();
	rc.use_tempdir = pathsmod->tempDirCB->isChecked();
	rc.tempdir_path = pathsmod->tempDirED->text().latin1();
	// FIXME: should be a checkbox only
	rc.lyxpipes = pathsmod->lyxserverDirED->text().latin1();


	QPrefSpellcheckerModule * spellmod(dialog_->spellcheckerModule);

	if (spellmod->spellCommandCO->currentItem() == 0)
		rc.isp_command = "ispell";
	else
		rc.isp_command = "aspell";
 
	// FIXME: remove isp_use_alt_lang
	rc.isp_alt_lang = spellmod->altLanguageED->text().latin1();
	rc.isp_use_alt_lang = !rc.isp_alt_lang.empty(); 
	// FIXME: remove isp_use_esc_chars 
	rc.isp_esc_chars = spellmod->escapeCharactersED->text().latin1();
	rc.isp_use_esc_chars = !rc.isp_esc_chars.empty();
	// FIXME: remove isp_use_pers_dict 
	rc.isp_pers_dict = spellmod->persDictionaryED->text().latin1();
	rc.isp_use_pers_dict = !rc.isp_pers_dict.empty();
	rc.isp_accept_compound = spellmod->compoundWordCB->isChecked();
	rc.isp_use_input_encoding = spellmod->inputEncodingCB->isChecked();


	QPrefPrinterModule * printmod(dialog_->printerModule);

	rc.print_adapt_output = printmod->printerAdaptCB->isChecked();
	rc.print_command = printmod->printerCommandED->text().latin1();
	rc.printer = printmod->printerNameED->text().latin1();

	rc.print_pagerange_flag = printmod->printerPageRangeED->text().latin1();
	rc.print_copies_flag = printmod->printerCopiesED->text().latin1();
	rc.print_reverse_flag = printmod->printerReverseED->text().latin1();
	rc.print_to_printer = printmod->printerToPrinterED->text().latin1();
	rc.print_file_extension = printmod->printerExtensionED->text().latin1();
	rc.print_file_extension = printmod->printerSpoolCommandED->text().latin1();
	rc.print_paper_flag = printmod->printerPaperTypeED->text().latin1();
	rc.print_evenpage_flag = printmod->printerEvenED->text().latin1();
	rc.print_oddpage_flag = printmod->printerOddED->text().latin1();
	rc.print_collcopies_flag = printmod->printerCollatedED->text().latin1();
	rc.print_landscape_flag = printmod->printerLandscapeED->text().latin1();
	rc.print_to_file = printmod->printerToFileED->text().latin1();
	rc.print_extra_options = printmod->printerExtraED->text().latin1();
	rc.print_spool_printerprefix = printmod->printerSpoolPrefixED->text().latin1();
	rc.print_paper_dimension_flag = printmod->printerPaperSizeED->text().latin1();


	QPrefScreenFontsModule * fontmod(dialog_->screenfontsModule);

	LyXRC const oldrc(rc);
 
	rc.roman_font_name = fontmod->screenRomanED->text().latin1();
	rc.sans_font_name = fontmod->screenSansED->text().latin1();
	rc.typewriter_font_name = fontmod->screenTypewriterED->text().latin1();
	rc.zoom = fontmod->screenZoomSB->value();
	rc.dpi = fontmod->screenDpiSB->value();
	rc.font_sizes[LyXFont::SIZE_TINY] = strToDbl(fontmod->screenTinyED->text().latin1());
	rc.font_sizes[LyXFont::SIZE_SCRIPT] = strToDbl(fontmod->screenSmallestED->text().latin1());
	rc.font_sizes[LyXFont::SIZE_FOOTNOTE] = strToDbl(fontmod->screenSmallerED->text().latin1());
	rc.font_sizes[LyXFont::SIZE_SMALL] = strToDbl(fontmod->screenSmallED->text().latin1());
	rc.font_sizes[LyXFont::SIZE_NORMAL] = strToDbl(fontmod->screenNormalED->text().latin1());
	rc.font_sizes[LyXFont::SIZE_LARGE] = strToDbl(fontmod->screenLargeED->text().latin1());
	rc.font_sizes[LyXFont::SIZE_LARGER] = strToDbl(fontmod->screenLargerED->text().latin1());
	rc.font_sizes[LyXFont::SIZE_LARGEST] = strToDbl(fontmod->screenLargestED->text().latin1());
	rc.font_sizes[LyXFont::SIZE_HUGE] = strToDbl(fontmod->screenHugeED->text().latin1());
	rc.font_sizes[LyXFont::SIZE_HUGER] = strToDbl(fontmod->screenHugerED->text().latin1());

	if (rc.font_sizes != oldrc.font_sizes
		|| rc.roman_font_name != oldrc.roman_font_name
		|| rc.sans_font_name != oldrc.sans_font_name
		|| rc.typewriter_font_name != oldrc.typewriter_font_name
		|| rc.zoom != oldrc.zoom || rc.dpi != oldrc.dpi) {
		controller().updateScreenFonts();
	}
 
	// FIXME: here we read new converters/ formats

	controller().setConverters(converters_);
	controller().setFormats(formats_); 

	// FIXME: controller().setColor(lc, hexname) 
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
	langmod->languagePackageED->setText(rc.language_package.c_str());
	langmod->startCommandED->setText(rc.language_command_begin.c_str());
	langmod->endCommandED->setText(rc.language_command_end.c_str());

	int const pos = int(findPos(lang_, rc.default_language));
	langmod->defaultLanguageCO->setCurrentItem(pos);

	QPrefUIModule * uimod(dialog_->uiModule); 

	uimod->uiFileED->setText(rc.bind_file.c_str());
	uimod->bindFileED->setText(rc.ui_file.c_str());
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
	keymod->firstKeymapED->setText(rc.primary_kbmap.c_str());
	keymod->secondKeymapED->setText(rc.secondary_kbmap.c_str());
 

	QPrefAsciiModule * ascmod(dialog_->asciiModule);

	ascmod->asciiLinelengthSB->setValue(rc.ascii_linelen);
	ascmod->asciiRoffED->setText(rc.ascii_roff_command.c_str());


	QPrefDateModule * datemod(dialog_->dateModule);

	datemod->DateED->setText(rc.date_insert_format.c_str());


	QPrefLatexModule * latexmod(dialog_->latexModule);

	latexmod->latexEncodingED->setText(rc.fontenc.c_str());
	latexmod->latexChecktexED->setText(rc.chktex_command.c_str()); 
	latexmod->latexAutoresetCB->setChecked(rc.auto_reset_options);
	latexmod->latexDviPaperED->setText(rc.view_dvi_paper_option.c_str());
	latexmod->latexPaperSizeCO->setCurrentItem(rc.default_papersize);


	QPrefDisplayModule * displaymod(dialog_->displayModule);

	displaymod->previewCB->setChecked(rc.preview);
 
	int item = 2;
 
	switch (rc.display_graphics) {
		case grfx::NoDisplay:		item = 3; break;
		case grfx::ColorDisplay:	item = 2; break;
		case grfx::GrayscaleDisplay:	item = 1; break;
		case grfx::MonochromeDisplay:	item = 0; break;
	}
	displaymod->displayGraphicsCO->setCurrentItem(item);
 

	QPrefPathsModule * pathsmod(dialog_->pathsModule);
 
	pathsmod->workingDirED->setText(rc.document_path.c_str());
	pathsmod->templateDirED->setText(rc.template_path.c_str());
	pathsmod->backupDirED->setText(rc.backupdir_path.c_str());
	pathsmod->tempDirCB->setChecked(rc.use_tempdir);
	pathsmod->tempDirED->setText(rc.tempdir_path.c_str());
	// FIXME: should be a checkbox only
	pathsmod->lyxserverDirED->setText(rc.lyxpipes.c_str());


	QPrefSpellcheckerModule * spellmod(dialog_->spellcheckerModule);

	item = (rc.isp_command == "ispell") ? 0 : 1;
	spellmod->spellCommandCO->setCurrentItem(item);
	// FIXME: remove isp_use_alt_lang
	spellmod->altLanguageED->setText(rc.isp_alt_lang.c_str());
	// FIXME: remove isp_use_esc_chars 
	spellmod->escapeCharactersED->setText(rc.isp_esc_chars.c_str());
	// FIXME: remove isp_use_pers_dict 
	spellmod->persDictionaryED->setText(rc.isp_pers_dict.c_str());
	spellmod->compoundWordCB->setChecked(rc.isp_accept_compound);
	spellmod->inputEncodingCB->setChecked(rc.isp_use_input_encoding);


	QPrefPrinterModule * printmod(dialog_->printerModule);

	printmod->printerAdaptCB->setChecked(rc.print_adapt_output);
	printmod->printerCommandED->setText(rc.print_command.c_str());
	printmod->printerNameED->setText(rc.printer.c_str());

	printmod->printerPageRangeED->setText(rc.print_pagerange_flag.c_str());
	printmod->printerCopiesED->setText(rc.print_copies_flag.c_str());
	printmod->printerReverseED->setText(rc.print_reverse_flag.c_str());
	printmod->printerToPrinterED->setText(rc.print_to_printer.c_str());
	printmod->printerExtensionED->setText(rc.print_file_extension.c_str());
	printmod->printerSpoolCommandED->setText(rc.print_file_extension.c_str());
	printmod->printerPaperTypeED->setText(rc.print_paper_flag.c_str());
	printmod->printerEvenED->setText(rc.print_evenpage_flag.c_str());
	printmod->printerOddED->setText(rc.print_oddpage_flag.c_str());
	printmod->printerCollatedED->setText(rc.print_collcopies_flag.c_str());
	printmod->printerLandscapeED->setText(rc.print_landscape_flag.c_str());
	printmod->printerToFileED->setText(rc.print_to_file.c_str());
	printmod->printerExtraED->setText(rc.print_extra_options.c_str());
	printmod->printerSpoolPrefixED->setText(rc.print_spool_printerprefix.c_str());
	printmod->printerPaperSizeED->setText(rc.print_paper_dimension_flag.c_str());


	QPrefScreenFontsModule * fontmod(dialog_->screenfontsModule);

	fontmod->screenRomanED->setText(rc.roman_font_name.c_str());
	fontmod->screenSansED->setText(rc.sans_font_name.c_str());
	fontmod->screenTypewriterED->setText(rc.typewriter_font_name.c_str());
	fontmod->screenZoomSB->setValue(rc.zoom);
	fontmod->screenDpiSB->setValue(int(rc.dpi));
	fontmod->screenTinyED->setText(tostr(rc.font_sizes[LyXFont::SIZE_TINY]).c_str());
	fontmod->screenSmallestED->setText(tostr(rc.font_sizes[LyXFont::SIZE_SCRIPT]).c_str());
	fontmod->screenSmallerED->setText(tostr(rc.font_sizes[LyXFont::SIZE_FOOTNOTE]).c_str());
	fontmod->screenSmallED->setText(tostr(rc.font_sizes[LyXFont::SIZE_SMALL]).c_str());
	fontmod->screenNormalED->setText(tostr(rc.font_sizes[LyXFont::SIZE_NORMAL]).c_str());
	fontmod->screenLargeED->setText(tostr(rc.font_sizes[LyXFont::SIZE_LARGE]).c_str());
	fontmod->screenLargerED->setText(tostr(rc.font_sizes[LyXFont::SIZE_LARGER]).c_str());
	fontmod->screenLargestED->setText(tostr(rc.font_sizes[LyXFont::SIZE_LARGEST]).c_str());
	fontmod->screenHugeED->setText(tostr(rc.font_sizes[LyXFont::SIZE_HUGE]).c_str());
	fontmod->screenHugerED->setText(tostr(rc.font_sizes[LyXFont::SIZE_HUGER]).c_str());

	formats_ = formats;

	dialog_->updateFormats();
 
	converters_ = converters;

	dialog_->updateConverters();
 
 
	// FIXME: populate colors
}
