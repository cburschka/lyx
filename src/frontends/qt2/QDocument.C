/**
 * \file QDocument.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Kalle Dalheimer <kalle@klaralvdalens-datakonsult.se>
 */

#include <config.h>

#ifdef __GNUG_
#pragma implementation
#endif

#include "QDocument.h"
#include "QDocumentDialog.h"

#include "lyx_gui_misc.h"

#include "Dialogs.h"
#include "layout.h"
#include "tex-strings.h"
#include "bufferparams.h"
#include "insets/insetquotes.h"
#include "vspace.h"
#include "support/filetools.h"
#include "language.h"
#include "BufferView.h"
#include "buffer.h"
#include "Liason.h"
#include "CutAndPaste.h"
#include "bufferview_funcs.h"

#include <qcombobox.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qspinbox.h>

using Liason::setMinibuffer;
using SigC::slot;

typedef Qt2CB<ControlDocument, Qt2DB<QDocumentDialog> > base_class;

#if 0 
QDocument::QDocument(ControlDocument & c)
	: base_class(c, _("Document Settings"))
{
}

#else
QDocument::QDocument(LyXView *, Dialogs *)
{
}
#endif 


void QDocument::build_dialog()
{
	// the tabbed folder
	//dialog_.reset(new QDocumentDialog());

#if 0 
	// Manage the restore, ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->okPB);
	bc().setApply(dialog_->applyPB);
	bc().setCancel(dialog_->cancelPB);
	bc().setRestore(dialog_->restorePB);
	bc().refresh();

	// the document paper page
	QDocumentDialog * dialog = dialog_.get();
	dialog->papersize2CO->insertItem( _( "Default" ) );
	dialog->papersize2CO->insertItem( _( "Custom" ) );
	dialog->papersize2CO->insertItem( _( "USletter" ) );
	dialog->papersize2CO->insertItem( _( "USlegal" ) );
	dialog->papersize2CO->insertItem( _( "USexecutive" ) );
	dialog->papersize2CO->insertItem( _( "A3" ) );
	dialog->papersize2CO->insertItem( _( "A4" ) );
	dialog->papersize2CO->insertItem( _( "A5" ) );
	dialog->papersize2CO->insertItem( _( "B3" ) );
	dialog->papersize2CO->insertItem( _( "B4" ) );
	dialog->papersize2CO->insertItem( _( "B5" ) );

	dialog->paperPackageCO->insertItem( _( "None" ) );
	dialog->paperPackageCO->insertItem( _( "A4 small Margins (only portrait)" ) );
	dialog->paperPackageCO->insertItem( _( "A4 very small Margins (only portrait)" ) );
	dialog->paperPackageCO->insertItem( _( "A4 very wide margins (only portrait)" ) );

	bc().addReadOnly (dialog->paperPackageCO);
	bc().addReadOnly (dialog->orientationBG);
	bc().addReadOnly (dialog->portraitRB);
	bc().addReadOnly (dialog->landscapeRB);
	bc().addReadOnly (dialog->papersize2CO);
	bc().addReadOnly (dialog->useGeometryCB);
	bc().addReadOnly (dialog->customWidthED);
	bc().addReadOnly (dialog->customHeightED);
	bc().addReadOnly (dialog->topMarginED);
	bc().addReadOnly (dialog->bottomMarginED);
	bc().addReadOnly (dialog->leftMarginED);
	bc().addReadOnly (dialog->rightMarginED);
	bc().addReadOnly (dialog->headHeightED);
	bc().addReadOnly (dialog->headSepED);
	bc().addReadOnly (dialog->footSkipED);

	// the document page
	for (LyXTextClassList::const_iterator cit = textclasslist.begin();
		 cit != textclasslist.end(); ++cit) {
		docClassCO->insertItem( (*cit).description().c_str() );
	}

	dialog->docSpacingCO->insertItem( _( "Single" ) );
	dialog->docSpacingCO->insertItem( _( "OneHalf" ) );
	dialog->docSpacingCO->insertItem( _( "Double" ) );
	dialog->docSpacingCO->insertItem( _( "Other" ) );

	dialog->docFontSizeCO->insertItem( _( "default" ) );
	dialog->docFontSizeCO->insertItem( _( "10" ) );
	dialog->docFontSizeCO->insertItem( _( "11" ) );
	dialog->docFontSizeCO->insertItem( _( "12" ) );
	for (int n=0; tex_fonts[n][0]; ++n) {
		dialog->docFontsCO->insertItem( tex_fonts[n] );
	}

	dialog->docPagestyleCO->insertItem( _( "default" ) );
	dialog->docPagestyleCO->insertItem( _( "empty" ) );
	dialog->docPagestyleCO->insertItem( _( "plain" ) );
	dialog->docPagestyleCO->insertItem( _( "headings" ) );
	dialog->docPagestyleCO->insertItem( _( "fancy" ) );

	dialog->docSkipCO->insertItem( _( "Smallskip" ) );
	dialog->docSkipCO->insertItem( _( "Medskip" ) );
	dialog->docSkipCO->insertItem( _( "Bigskip" ) );
	dialog->docSkipCO->insertItem( _( "Length" ) );

	bc().addReadOnly (dialog->docIndentRB);
	bc().addReadOnly (dialog->docSkipRB);

	bc().addReadOnly (dialog->docPagestyleCO);
	bc().addReadOnly (dialog->docFontsCO);
	bc().addReadOnly (dialog->docFontsizeCO);
	bc().addReadOnly (dialog->docSidesOneRB);
	bc().addReadOnly (dialog->docSidesTwoRB);
	bc().addReadOnly (dialog->docColumnsOneRB);
	bc().addReadOnly (dialog->docColumnsTwoRB);
	bc().addReadOnly (dialog->docExtraED);
	bc().addReadOnly (dialog->docSkipED);
	bc().addReadOnly (dialog->docSkipCO);
	bc().addReadOnly (dialog->docSpacingCO);
	bc().addReadOnly (dialog->docSpacingED);

	// the document language page
	dialog->inputEncCO->insertItem( _( "default" ) );
	dialog->inputEncCO->insertItem( _( "auto" ) );
	dialog->inputEncCO->insertItem( _( "latin1" ) );
	dialog->inputEncCO->insertItem( _( "latin2" ) );
	dialog->inputEncCO->insertItem( _( "latin3" ) );
	dialog->inputEncCO->insertItem( _( "latin4" ) );
	dialog->inputEncCO->insertItem( _( "latin5" ) );
	dialog->inputEncCO->insertItem( _( "latin9" ) );
	dialog->inputEncCO->insertItem( _( "koi8-r" ) );
	dialog->inputEncCO->insertItem( _( "koi8-u" ) );
	dialog->inputEncCO->insertItem( _( "cp866" ) );
	dialog->inputEncCO->insertItem( _( "cp1251" ) );
	dialog->inputEncCO->insertItem( _( "iso88595" ) );

	for (Languages::const_iterator cit = languages.begin();
	 cit != languages.end(); ++cit) {
		dialog->languageCO->insertItem( (*cit).second.lang() );
	}

	dialog->quotesLanguageCO->insertItem( _( "``text''" ) );
	dialog->quotesLanguageCO->insertItem( _( "''text''" ) );
	dialog->quotesLanguageCO->insertItem( _( ",,text``" ) );
	dialog->quotesLanguageCO->insertItem( _( ",,text''" ) );
	dialog->quotesLanguageCO->insertItem( _( "«text»" ) );
	dialog->quotesLanguageCO->insertItem( _( "»text«" ) );

	bc().addReadOnly (dialog->inputEncCO);

	// the document options page
	dialog->secNumDepthSB->setRange( -2, 5 );
	dialog->tocDepthSB->setRange( -1, 5 );
	dialog->secNumDepthSB->setSteps( 1, 1 );
	dialog->tocDepthSB->setRange( 1, 1 );
	for (int n=0; tex_graphics[n][0]; ++n) {
		dialog->postscriptDriverCO->insertItem( tex_graphics[n] );
	}

	bc().addReadOnly (dialog->secNumDepthSB);
	bc().addReadOnly (dialog->tocDepthSB);
	bc().addReadOnly (dialog->useAmsMathCB);
	bc().addReadOnly (dialog->floatPlacementED);
	bc().addReadOnly (dialog->postscriptDriverCO);

	// the document bullets page
	dialog->bulletSizeCO->insertItem( _( "default" ) );
	dialog->bulletSizeCO->insertItem( _( "tiny" ) );
	dialog->bulletSizeCO->insertItem( _( "script" ) );
	dialog->bulletSizeCO->insertItem( _( "footnote" ) );
	dialog->bulletSizeCO->insertItem( _( "small" ) );
	dialog->bulletSizeCO->insertItem( _( "normal" ) );
	dialog->bulletSizeCO->insertItem( _( "large" ) );
	dialog->bulletSizeCO->insertItem( _( "Large" ) );
	dialog->bulletSizeCO->insertItem( _( "LARGE" ) );
	dialog->bulletSizeCO->insertItem( _( "huge" ) );
	dialog->bulletSizeCO->insertItem( _( "Huge" ) );

	dialog->bulletSizeCO->setCurrentItem( 0 );
	dialog->bulletLatexED->setMaxLength( 80 );

	bc().addReadOnly (dialog->bulletPanelBG);
	bc().addReadOnly (dialog->bulletSizeCO);
	bc().addReadOnly (dialog->bulletLatexED);
#endif 
}


void QDocument::apply()
{
	//if (!lv_->view()->available() || !dialog_.get())
	//	return;

#if 0 
	bool redo = class_apply();
	paper_apply();
	redo = language_apply() || redo;
	redo = options_apply() || redo;
	bullets_apply();

	if (redo) {
		lv_->view()->redoCurrentBuffer();
	}
	lv_->buffer()->markDirty();
	setMinibuffer(lv_, _("Document layout set"));
#endif 
}


void QDocument::cancel()
{
#if 0 
	// this avoids confusion when reopening
	BufferParams & param = lv_->buffer()->params;
	param.temp_bullets[0] = param.user_defined_bullets[0];
	param.temp_bullets[1] = param.user_defined_bullets[1];
	param.temp_bullets[2] = param.user_defined_bullets[2];
	param.temp_bullets[3] = param.user_defined_bullets[3];
	hide();
#endif 
}


void QDocument::update_contents()
{
	if (!dialog_.get())
		return;

#if 0 
	checkReadOnly();

	BufferParams const & params = lv_->buffer()->params;

	class_update(params);
	paper_update(params);
	language_update(params);
	options_update(params);
	bullets_update(params);
#endif 
}


#if 0
bool QDocument::input( FL_OBJECT * ob, long data )
{
	State cb = static_cast<State>( data );

	switch (cb) {
		case CHECKCHOICECLASS:
			CheckChoiceClass(ob, 0);
			break;
		case CHOICEBULLETSIZE:
			ChoiceBulletSize(ob, 0);
			break;
		case INPUTBULLETLATEX:
			InputBulletLaTeX(ob, 0);
			break;
		case BULLETDEPTH1:
		case BULLETDEPTH2:
		case BULLETDEPTH3:
		case BULLETDEPTH4:
			BulletDepth(ob, cb);
			break;
		case BULLETPANEL1:
		case BULLETPANEL2:
		case BULLETPANEL3:
		case BULLETPANEL4:
		case BULLETPANEL5:
		case BULLETPANEL6:
			BulletPanel(ob, cb);
			break;
		case BULLETBMTABLE:
			BulletBMTable(ob, 0);
			break;
		default:
			break;
	}

	switch (data) {
		case INPUT:
		case CHECKCHOICECLASS:
		case CHOICEBULLETSIZE:
		case INPUTBULLETLATEX:
		case BULLETBMTABLE:
			return CheckDocumentInput(ob, 0);
		default:
			break;
	}

	return true;
}


void QDocument::ComboInputCB(int, void * v, Combox * combox)
{
	QDocument * pre = static_cast<QDocument*>(v);
	if (combox == pre->combo_doc_class.get())
		pre->CheckChoiceClass(0, 0);
	pre->bc().valid(pre->CheckDocumentInput(0,0));
}

bool QDocument::class_apply()
{
	bool redo = false;
	BufferParams &params = lv_->buffer()->params;

	// If default skip is a "Length" but there's no text in the
	// input field, reset the kind to "Medskip", which is the default.
	if( ( dialog_->docSkipCO->currentItem() == 3 ) &&
		dialog_->docSkipED->text().isEmpty() )
		dialog_->docSkipCO->setCurrentItem( 1 );

	params.fonts = dialog_->docFontsCO->currentText();
	params.fontsize = dialog_->docFontSizeCO->currentText();
	params.pagestyle = dialog->docPagestyleCO->currentText();
	
	unsigned int const new_class = dialog_->docClassCO->currentItem();
	
	if (params.textclass != new_class) {
		// try to load new_class
		if (textclasslist.Load(new_class)) {
			// successfully loaded
			redo = true;
			setMinibuffer(lv_, _("Converting document to new document class..."));
			CutAndPaste cap;
			int ret = cap.SwitchLayoutsBetweenClasses(params.textclass, new_class,
								  lv_->buffer()->paragraph);		
			if (ret) {
				string s;
				if (ret==1) {
					s = _("One paragraph couldn't be converted");
				} else {
					s += tostr(ret);
					s += _(" paragraphs couldn't be converted");
				}
				WriteAlert(_("Conversion Errors!"),s,
					   _("into chosen document class"));
			}

			params.textclass = new_class;
		} else {
			// problem changing class -- warn user and retain old style
			WriteAlert(_("Conversion Errors!"),
				   _("Errors loading new document class."),
				   _("Reverting to original document class."));
			dialog_->docClassCO->setCurrentItem( params.textclass );
		}
	}
	BufferParams::PARSEP tmpsep = params.paragraph_separation;
	if( dialog_->_docIndentRB->isChecked() )
		params.paragraph_separation = BufferParams::PARSEP_INDENT;
	else
		params.paragraph_separation = BufferParams::PARSEP_SKIP;
	if (tmpsep != params.paragraph_separation)
		redo = true;

	VSpace tmpdefskip = params.getDefSkip();
	switch( dialog_->docSkipCO->currentItem() ) {
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
			params.setDefSkip
				(VSpace(LyXGlueLength(dialog_->docSkipED->text())));
			break;
		// DocumentDefskipCB assures that this never happens
		// ... so why is there code for it ?
		default:
			params.setDefSkip(VSpace(VSpace::MEDSKIP));
		break;
	}
	if (!(tmpdefskip == params.getDefSkip()))
		redo = true;

	if( dialog_->docColumnsTwo->isChecked() )
		params.columns = 2;
	else
		params.columns = 1;
	if( dialog_->docSidesTwo->isChecked() )
		params.sides = LyXTextClass::TwoSides;
	else
		params.sides = LyXTextClass::OneSide;

	Spacing tmpSpacing = params.spacing;
	switch( dialog_->docSpacingCO->currentItem() ) {
		case 0:
			lyxerr[Debug::INFO] << "Spacing: SINGLE\n";
			params.spacing.set(Spacing::Single);
			break;
		case 1:
			lyxerr[Debug::INFO] << "Spacing: ONEHALF\n";
			params.spacing.set(Spacing::Onehalf);
			break;
		case 2:
			lyxerr[Debug::INFO] << "Spacing: DOUBLE\n";
			params.spacing.set(Spacing::Double);
			break;
		case 3:
			lyxerr[Debug::INFO] << "Spacing: OTHER\n";
			params.spacing.set(Spacing::Other,
				   dialog_->docSpacingED->text() );
			break;
	}
	if (tmpSpacing != params.spacing)
		redo = true;

	params.options = docExtraED->text();

	return redo;
}


void QDocument::paper_apply()
{
	BufferParams & params = lv_->buffer()->params;

	params.papersize2 =
		static_cast<char>(dialog_->papersize2CO->currentItem());
	params.paperpackage =
		static_cast<char>(dialog_->paperPackageCO->currentItem() );
	params.use_geometry = dialog_->useGeometryCB->isChecked();
	if (dialog_->landscapeRB->isChecked() )
		params.orientation = BufferParams::ORIENTATION_LANDSCAPE;
	else
		params.orientation = BufferParams::ORIENTATION_PORTRAIT;
	params.paperwidth = dialog_->customWidthED->text();
	params.paperheight = dialog_->customHeightED->text();
	params.leftmargin = dialog_->leftMarginED->text();
	params.topmargin = dialog_->topMarginED->text();
	params.rightmargin = dialog_->rightMarginED->text();
	params.bottommargin = dialog_->bottomMarginED->text();
	params.headheight = dialog_->headHeightED->text();
	params.headsep = dialog_->headSepED->text();
	params.footskip = dialog_->footSkipED->text();
	lv_->buffer()->setPaperStuff();
}


bool QDocument::language_apply()
{
	BufferParams & params = lv_->buffer()->params;
	InsetQuotes::quote_language lga = InsetQuotes::EnglishQ;
	bool redo = false;

	switch (dialog_->quotesLanguageCO->currentItem() ) {
		case 0:
			lga = InsetQuotes::EnglishQ;
			break;
		case 1:
			lga = InsetQuotes::SwedishQ;
			break;
		case 2:
			lga = InsetQuotes::GermanQ;
			break;
		case 3:
			lga = InsetQuotes::PolishQ;
			break;
		case 4:
			lga = InsetQuotes::FrenchQ;
			break;
		case 5:
			lga = InsetQuotes::DanishQ;
			break;
	}
	params.quotes_language = lga;
	if (dialog_->singleRB->isChecked() )
		params.quotes_times = InsetQuotes::SingleQ;
	else
		params.quotes_times = InsetQuotes::DoubleQ;

	Language const * old_language = params.language;
	Language const * new_language =
	languages.getLanguage(dialog_->languageCO->currentItem() );
	if (!new_language)
		new_language = default_language;

	if (old_language != new_language
		&& old_language->RightToLeft() == new_language->RightToLeft()
		&& !lv_->buffer()->isMultiLingual())
		lv_->buffer()->ChangeLanguage(old_language, new_language);
 
	if (old_language != new_language)
		redo = true;

	params.language = new_language;
	params.inputenc = dialog_->inputEncCO->currentText();

	return redo;
}


bool QDocument::options_apply()
{
	BufferParams & params = lv_->buffer()->params;
	bool redo = false;

	params.graphicsDriver =
		dialog_->postscriptDriverCO->currentText();
	params.use_amsmath = dialog_->useAmsMathCB->isChecked();

	int tmpchar = int(dialog_->secNumDepth->value() );
	if (params.secnumdepth != tmpchar)
		redo = true;
	params.secnumdepth = tmpchar;

	params.tocdepth = int(dialog_->tocDepth->value() );

	params.float_placement =
		dialog_->floatPlacementED->text();

	return redo;
}


void QDocument::bullets_apply()
{
	/* update the bullet settings */
	BufferParams & param = lv_->buffer()->params;

	// a little bit of loop unrolling
	param.user_defined_bullets[0] = param.temp_bullets[0];
	param.user_defined_bullets[1] = param.temp_bullets[1];
	param.user_defined_bullets[2] = param.temp_bullets[2];
	param.user_defined_bullets[3] = param.temp_bullets[3];
}


void QDocument::class_update(BufferParams const & params)
{
	if (!class_.get())
		return;

	LyXTextClass const & tclass = textclasslist.TextClass(params.textclass);

	combo_doc_class->select_text(
				 textclasslist.DescOfClass(params.textclass));
	Qt2Helper::setCurrentComboItem( dialog_->docFonts, params.fonts.c_str() );
	dialog_->docFontSizeCO->clear();
	dialog_->docFontSizeCO->insertItem( _( "default" ) );
	dialog_->docFontSizeCO->insertItem( tclass.opt_fontsize().c_str());
	dialog_->docFontSizeCO->setCurrentItem( tokenPos(tclass.opt_fontsize(), '|', params.fontsize)+1);
	dialog_->docPagestyleCO->clear();
	dialog_->docPagestyleCO->insertItem( _( "default" ) );
	dialog_->docPagestyleCO->insertItem( tclass.opt_pagestyle().c_str());
	dialog_->docPagestyleCO->setCurrentItem( tokenPos(tclass.opt_pagestyle(), '|', params.pagestyle)+1);
	dialog_->docIndentRB->setChecked( false );
	dialog_->docSkipRB->setChecked( false );
	if (params.paragraph_separation == BufferParams::PARSEP_INDENT)
		dialog_->docIndentRB->setChecked( true );
	else
		dialog_->docSkipRB->setChecked( true );
	switch (params.getDefSkip().kind()) {
		case VSpace::SMALLSKIP:
			dialog_->docSkipCO->setCurrentItem( 0 );
			break;
		case VSpace::MEDSKIP:
			dialog_->docSkipCO->setCurrentItem( 1 );
			break;
		case VSpace::BIGSKIP:
			dialog_->docSkipCO->setCurrentItem( 2 );
			break;
		case VSpace::LENGTH:
			dialog_->docSkipCO->setCurrentItem( 3 );
			dialog_->docSkipED->setText( params.getDefSkip().asLyXCommand().c_str());
			break;
		default:
			dialog_->docSkipCO->setCurrentItem( 1 );
			break;
	}
	dialog_->docSidesOneRB->setChecked( false );
	dialog_->docSidesTwoRB->setChecked( false );
	if (params.sides == LyXTextClass::TwoSides)
		dialog_->docSidesTwo->setChecked( true );
	else
		dialog_->docSidesOne->setChecked( true );
	dialog_->docColumnsOneRB->setChecked( false );
	dialog_->docColumnsTwoRB->setChecked( false );
	if (params.columns == 2)
		dialog_->docColumnsTwoRB->setChecked( true );
	else
		dialog_->docColumnsOneRB->setChecked( true );
	dialog_->docSpacingED->setText( "" );
	switch (params.spacing.getSpace()) {
		case Spacing::Default: // nothing bad should happen with this
		case Spacing::Single:
			// \singlespacing
			dialog_->docSpacingCO->setCurrentItem( 0 );
			break;
		case Spacing::Onehalf:
			// \onehalfspacing
			dialog_->docSpacingCO->setCurrentItem( 1 );
			break;
		case Spacing::Double:
			// \doublespacing
			dialog_->docSpacingCO->setCurrentItem( 2 );
		break;
		case Spacing::Other:
		{
			dialog_->docSpacingCO->setCurrentItem( 3 );
			QString sval;
			sval.sprintf("%g",params.spacing.getValue());
			dialog_->docSpacingED->setText( sval );
			break;
		}
	}
	if (!params.options.empty())
		dialog_->docExtraED->setText(, params.options.c_str());
	else
		dialog_->docExtraED->setText( "" );
}


void QDocument::language_update(BufferParams const & params)
{
	if (!language_.get())
		return;

	Qt2Helper::setCurrentItemText( dialog_->languageCO, params.language->lang() );
	Qt2Helper::setCurrentItemText( dialog_->inputEncCO, params.inputenc.c_str() );
	dialog_->quotesLanguageCO->setCurrentItem( params.quotes_language );
	dialog_->singleRB->setChecked( false );
	dialog_->doubleRB->setChecked( false );
	if (params.quotes_times == InsetQuotes::SingleQ)
		dialog_->singleRB->setChecked( true );
	else
		dialog_->doubleRB->setChecked( true );
}


void QDocument::options_update(BufferParams const & params)
{
	if (!options_.get())
		return;

	Qt2Helper::setCurrentItemText( dialog_->postscriptDriverCO,
				   params.graphicsDriver.c_str());
	dialog_->useAmsMathCB->setChecked( params.use_amsmath );
	dialog_->secNumDepthSB->setValue( params.secnumdepth );
	dialog_->setTocDepthSB->setValue( params.tocdepth );
	if (!params.float_placement.empty())
		dialog_->floatPlacementED->setText( params.float_placement.c_str());
	else
		dialog_->floatPlacementED->setText( "" );
}


void QDocument::paper_update(BufferParams const & params)
{
	if (!paper_.get())
		return;

	dialog_->papersize2CO->setCurrentItem( params.papersize2 );
	dialog_->paperPackageCO->setCurrentItem( params.paperpackage );
	dialog_->useGeometryCB->setChecked( params.use_geometry );
	dialog_->portraitRB->setChecked( false );
	dialog_->landscapeRB->setChecked( false );
	if (params.orientation == BufferParams::ORIENTATION_LANDSCAPE)
		dialog_->landscapeRB->setChecked( true );
	else
		dialog_->portraitRB->setChecked( true );
	dialog_->customWidthED->setText( params.paperwidth.c_str());
	dialog_->customHeightED->setText( params.paperheight.c_str());
	dialog_->leftMarginED->setText( params.leftmargin.c_str());
	dialog_->topMarginED->setText( params.topmargin.c_str());
	dialog_->rightMarginED->setText( params.rightmargin.c_str());
	dialog_->bottomMarginED->setText( params.bottommargin.c_str());
	dialog_->headHeightED->setText( params.headheight.c_str());
	dialog_->headSepED->setText( params.headsep.c_str());
	dialog_->footSkipED->setText( params.footskip.c_str());
	dialog_->papersize2CO->setFocus();
}


void QDocument::bullets_update(BufferParams const & params)
{
	bool const isLinuxDoc = lv_->buffer()->isLinuxDoc();
	setEnabled(fbullet, !isLinuxDoc);

	if (isLinuxDoc) return;

	dialog_->bulletDepth1RB->setChecked( true );
	dialog_->bulletLatexED->setText( params.user_defined_bullets[0].getText().c_str());
	dialog_->bulletSizeCO->setCurrentItem( params.user_defined_bullets[0].getSize() + 1);
}


void QDocument::checkReadOnly()
{
	if (bc().readOnly(lv_->buffer()->isReadonly())) {
		dialog_->docClassCO->setEnabled( false );
		dialog_->languageCO->setEnabled( false );
		dialog_->warningLA->setText( _("Document is read-only."
						   " No changes to layout permitted."));
		dialog_->warningLA->show();
	} else {
		dialog_->docClassCO->setEnabled( true );
		dialog_->languageCO->setEnabled( true );
		dialog_->warningLA->hide();
	}
}


void QDocument::checkMarginValues()
{
	bool const not_empty =
		!dialog_->topMarginED->text().isEmpty() ||
		!dialog_->bottomMarginED->text().isEmpty() ||
		!dialog_->leftMarginED->text().isEmpty() ||
		!dialog_->rightMarginED->text().isEmpty() ||
		!dialog_->headHeightED->text().isEmpty() ||
		!dialog_->headSepED->text().isEmpty() ||
		!dialog_->footSkipED->text().isEmpty() ||
		!dialog_->customWidthED->text().isEmpty() ||
		!dialog_->customHeightED->text().isEmpty();
	if (not_empty)
		dialog_->useGeometryCB->setChecked( true );
}

#ifdef I_SUPPOSE_THIS_NEEDS_TO_FIXED
bool QDocument::CheckDocumentInput(FL_OBJECT * ob, long)
{
	string str;
	int val;
	bool ok = true;
	char const * input;

	checkMarginValues();
	if (ob == paper_->choice_papersize2) {
		val = fl_get_choice(paper_->choice_papersize2)-1;
		if (val == BufferParams::VM_PAPER_DEFAULT) {
			fl_set_button(paper_->push_use_geometry, 0);
			checkMarginValues();
		} else {
			if ((val != BufferParams::VM_PAPER_USLETTER) &&
				(val != BufferParams::VM_PAPER_USLEGAL) &&
				(val != BufferParams::VM_PAPER_USEXECUTIVE) &&
				(val != BufferParams::VM_PAPER_A4) &&
				(val != BufferParams::VM_PAPER_A5) &&
				(val != BufferParams::VM_PAPER_B5)) {
					fl_set_button(paper_->push_use_geometry, 1);
				}
			fl_set_choice(paper_->choice_paperpackage,
				  BufferParams::PACKAGE_NONE + 1);
		}
	} else if (ob == paper_->choice_paperpackage) {
		val = fl_get_choice(paper_->choice_paperpackage)-1;
		if (val != BufferParams::PACKAGE_NONE) {
			fl_set_choice(paper_->choice_papersize2,
				  BufferParams::VM_PAPER_DEFAULT + 1);
			fl_set_button(paper_->push_use_geometry, 0);
		}
	} else if (ob == class_->input_doc_spacing) {
		input = fl_get_input(class_->input_doc_spacing);
		if (!*input) {
			fl_set_choice (class_->choice_doc_spacing, 1);
		} else {
			fl_set_choice(class_->choice_doc_spacing, 4);
		}
	}
	// this has to be all out of if/elseif because it has to deactivate
	// the document buttons and so the whole stuff has to be tested again.
	str = fl_get_input(paper_->input_custom_width);
	ok = ok && (str.empty() || isValidLength(str));
	str = fl_get_input(paper_->input_custom_height);
	ok = ok && (str.empty() || isValidLength(str));
	str = fl_get_input(paper_->input_left_margin);
	ok = ok && (str.empty() || isValidLength(str));
	str = fl_get_input(paper_->input_right_margin);
	ok = ok && (str.empty() || isValidLength(str));
	str = fl_get_input(paper_->input_top_margin);
	ok = ok && (str.empty() || isValidLength(str));
	str = fl_get_input(paper_->input_bottom_margin);
	ok = ok && (str.empty() || isValidLength(str));
	str = fl_get_input(paper_->input_head_height);
	ok = ok && (str.empty() || isValidLength(str));
	str = fl_get_input(paper_->input_head_sep);
	ok = ok && (str.empty() || isValidLength(str));
	str = fl_get_input(paper_->input_foot_skip);
	ok = ok && (str.empty() || isValidLength(str));
	// "Synchronize" the choice and the input field, so that it
	// is impossible to commit senseless data.
	input = fl_get_input (class_->input_doc_skip);
	if (ob == class_->input_doc_skip) {
		if (!*input) {
			fl_set_choice (class_->choice_doc_skip, 2);
		} else if (isValidGlueLength (input)) {
			fl_set_choice (class_->choice_doc_skip, 4);
		} else {
			fl_set_choice(class_->choice_doc_skip, 4);
			ok = false;
		}
	} else {
		if (*input && !isValidGlueLength(input))
			ok = false;
	}
	if ((fl_get_choice(class_->choice_doc_skip) == 4) && !*input)
		ok = false;
	else if (fl_get_choice(class_->choice_doc_skip) != 4)
		fl_set_input (class_->input_doc_skip, "");

	input = fl_get_input(class_->input_doc_spacing);
	if ((fl_get_choice(class_->choice_doc_spacing) == 4) && !*input)
		ok = false;
	else if (fl_get_choice(class_->choice_doc_spacing) != 4)
		fl_set_input (class_->input_doc_spacing, "");
	return ok;
}


void QDocument::ChoiceBulletSize(FL_OBJECT * ob, long /*data*/ )
{
	BufferParams & param = lv_->buffer()->params;

	// convert from 1-6 range to -1-4
	param.temp_bullets[current_bullet_depth].setSize(fl_get_choice(ob) - 2);
	fl_set_input(bullets_->input_bullet_latex,
		 param.temp_bullets[current_bullet_depth].getText().c_str());
}


void QDocument::InputBulletLaTeX(FL_OBJECT *, long)
{
	BufferParams & param = lv_->buffer()->params;

	param.temp_bullets[current_bullet_depth].
	setText(fl_get_input(bullets_->input_bullet_latex));
}


void QDocument::BulletDepth(FL_OBJECT * ob, State cb)
{
	/* Should I do the following:
	 *  1. change to the panel that the current bullet belongs in 
	 *  2. show that bullet as selected
	 *  3. change the size setting to the size of the bullet in Q.
	 *  4. display the latex equivalent in the latex box
	 *
	 * I'm inclined to just go with 3 and 4 at the moment and
	 * maybe try to support the others later
	 */ 
	BufferParams & param = lv_->buffer()->params;

	int data = 0;
	if (cb == BULLETDEPTH1 )
		data = 0;
	else if (cb == BULLETDEPTH2 )
		data = 1;
	else if (cb == BULLETDEPTH3 )
		data = 2;
	else if (cb == BULLETDEPTH4 )
		data = 3;

	switch (fl_get_button_numb(ob)) {
		case 3:
			// right mouse button resets to default
			param.temp_bullets[data] = ITEMIZE_DEFAULTS[data];
		default:
			current_bullet_depth = data;
			fl_set_input(bullets_->input_bullet_latex,
					 param.temp_bullets[data].getText().c_str());
			fl_set_choice(bullets_->choice_bullet_size,
					  param.temp_bullets[data].getSize() + 2);
	}
}


void QDocument::BulletPanel(FL_OBJECT * /*ob*/, State cb)
{
	/* Here we have to change the background pixmap to that selected */
	/* by the user. (eg. standard.xpm, psnfss1.xpm etc...)		   */

	int data = 0;
	if (cb == BULLETPANEL1 )
		data = 0;
	else if (cb == BULLETPANEL2 )
		data = 1;
	else if (cb == BULLETPANEL3 )
		data = 2;
	else if (cb == BULLETPANEL4 )
		data = 3;
	else if (cb == BULLETPANEL5 )
		data = 4;
	else if (cb == BULLETPANEL6 )
		data = 5;

	if (data != current_bullet_panel) {
		fl_freeze_form(bullets_->form);
		current_bullet_panel = data;

		/* free the current pixmap */
		fl_free_bmtable_pixmap(bullets_->bmtable_bullet_panel);
		string new_panel;
		switch (cb) {
			/* display the new one */
			case BULLETPANEL1 :
				new_panel = "standard";
				break;
			case BULLETPANEL2 :
				new_panel = "amssymb";
				break;
			case BULLETPANEL3 :
			new_panel = "psnfss1";
			break;
			case BULLETPANEL4 :
				new_panel = "psnfss2";
				break;
			case BULLETPANEL5 :
				new_panel = "psnfss3";
				break;
			case BULLETPANEL6 :
				new_panel = "psnfss4";
				break;
			default :
				/* something very wrong happened */
				// play it safe for now but should be an exception
				current_bullet_panel = 0;  // standard panel
				new_panel = "standard";
			break;
		}
		new_panel += ".xpm";
		fl_set_bmtable_pixmap_file(bullets_->bmtable_bullet_panel, 6, 6,
					   LibFileSearch("images", new_panel).c_str());
		fl_redraw_object(bullets_->bmtable_bullet_panel);
		fl_unfreeze_form(bullets_->form);
	}
}


void QDocument::BulletBMTable(FL_OBJECT * ob, long /*data*/ )
{
	/* handle the user input by setting the current bullet depth's pixmap */
	/* to that extracted from the current chosen position of the BMTable  */
	/* Don't forget to free the button's old pixmap first.				*/

	BufferParams & param = lv_->buffer()->params;
	int bmtable_button = fl_get_bmtable(ob);

	/* try to keep the button held down till another is pushed */
	/*  fl_set_bmtable(ob, 1, bmtable_button); */
	param.temp_bullets[current_bullet_depth].setFont(current_bullet_panel);
	param.temp_bullets[current_bullet_depth].setCharacter(bmtable_button);
	fl_set_input(bullets_->input_bullet_latex,
		 param.temp_bullets[current_bullet_depth].getText().c_str());
}


void QDocument::CheckChoiceClass(FL_OBJECT * ob, long)
{
	if (!ob)
		ob = class_->choice_doc_class;

	ProhibitInput(lv_->view());

	unsigned int tc = combo_doc_class->get() - 1;
	if (textclasslist.Load(tc)) {
		// we use a copy of the bufferparams because we do not
		// want to modify them yet.
		BufferParams params = lv_->buffer()->params;

		if (params.textclass != tc
			&& AskQuestion(_("Should I set some parameters to"),
				   _("the defaults of this document class?"))) {
			params.textclass = tc;
			params.useClassDefaults();
			UpdateLayoutDocument(params);
		}
	} else {
		// unable to load new style
		WriteAlert(_("Conversion Errors!"),
			   _("Unable to switch to new document class."),
			   _("Reverting to original document class."));
		combo_doc_class->select(int(lv_->buffer()->params.textclass) + 1);
	}
	AllowInput(lv_->view());
}
#endif

void QDocument::UpdateLayoutDocument(BufferParams const & params)
{
	if (!dialog_.get())
		return;

	checkReadOnly();
	class_update(params);
	paper_update(params);
	language_update(params);
	options_update(params);
	bullets_update(params);
}
#endif // 0 
