/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 2000 The LyX Team.
 *
 *           @author Jürgen Vigna, Kalle Dalheimer
 *
 *======================================================*/

#include <config.h>

#ifdef __GNUG_
#pragma implementation
#endif

#include "FormDocumentDialog.h"
#undef emit

#include "lyx_gui_misc.h"
#include "gettext.h"
//#include FORMS_H_LOCATION
#include XPM_H_LOCATION

#include "FormDocument.h"
//#include "xform_macros.h"
#include "Dialogs.h"
#include "layout.h"
#include "combox.h"
#include "tex-strings.h"
#include "bufferparams.h"
#include "insets/insetquotes.h"
#include "vspace.h"
#include "bmtable.h"
#include "support/filetools.h"
#include "language.h"
#include "LyXView.h"
#include "BufferView.h"
#include "buffer.h"
#include "Liason.h"
#include "CutAndPaste.h"
#include "bufferview_funcs.h"
#include "ButtonController.h"

#ifdef SIGC_CXX_NAMESPACES
using SigC::slot;
#endif

#ifdef CXX_WORKING_NAMESPACES
using Liason::setMinibuffer;
#endif

#define USE_CLASS_COMBO 1

#include <qcombobox.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qtoolbutton.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qbuttongroup.h>
#include <qlabel.h>

// Bullet images
#include "../../../lib/images/standard.xpm"
#include "../../../lib/images/psnfss1.xpm"
#include "../../../lib/images/psnfss2.xpm"
#include "../../../lib/images/psnfss3.xpm"
#include "../../../lib/images/psnfss4.xpm"
#include "../../../lib/images/amssymb.xpm"


FormDocument::FormDocument(LyXView * lv, Dialogs * d)
  : FormBaseBD( lv, d, _( "Document Layout" ), 
				new NoRepeatedApplyReadOnlyPolicy ),
  dialog_(0), lv_(lv), d_(d), u_(0), h_(0),
  status(POPUP_UNMODIFIED) ,
  currentBulletPanel(0),
  currentBulletDepth(0)
{
    // let the popup be shown
    // This is a permanent connection so we won't bother
    // storing a copy because we won't be disconnecting.
    d->showLayoutDocument.connect(slot(this, &FormDocument::show));

    // load the images
    standardpix = new QPixmap( standard );
    amssymbpix = new QPixmap( amssymb );
    psnfss1pix = new QPixmap( psnfss1 );
    psnfss2pix = new QPixmap( psnfss2 );
    psnfss3pix = new QPixmap( psnfss3 );
    psnfss4pix = new QPixmap( psnfss4 );
}


FormDocument::~FormDocument()
{
  delete standardpix;
  delete amssymbpix;
  delete psnfss1pix;
  delete psnfss2pix;
  delete psnfss3pix;
  delete psnfss4pix;
}


void FormDocument::build()
{
    int n;

    dialog_ = new FormDocumentDialog( this, 0, _( "Document Layout" ) );

    for (LyXTextClassList::const_iterator cit = textclasslist.begin();
	 cit != textclasslist.end(); ++cit)
    {
	dialog_->classesCO->insertItem((*cit).description().c_str());
    }

    for (n=0; tex_fonts[n][0]; ++n) {
      dialog_->fontsCO->insertItem( tex_fonts[n] );
    }

    for(Languages::const_iterator cit = languages.begin();
	cit != languages.end(); ++cit) {
	dialog_->languageCO->insertItem((*cit).second.lang().c_str());
    }

    for (n=0; tex_graphics[n][0]; ++n) {
      dialog_->psDriverCO->insertItem( tex_graphics[n] );
    }

    dialog_->bulletDepth1PB->setOn( true );
}


void FormDocument::apply()
{
    if (!lv_->view()->available() || !dialog_)
	return;

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
}


bool FormDocument::class_apply()
{
	bool redo = false;
	BufferParams &params = lv_->buffer()->params;

	// If default skip is a "Length" but there's no text in the
	// input field, reset the kind to "Medskip", which is the default.
	if( ( dialog_->defaultSkipCO->currentItem() == 4 ) &&
	    ( dialog_->defaultSkipED->text().isEmpty() ) )
	  dialog_->defaultSkipCO->setCurrentItem( 1 );
	params.fonts = dialog_->fontsCO->currentText();
	params.fontsize = dialog_->fontSizeCO->currentText();
	params.pagestyle = dialog_->pagestyleCO->currentText();

	unsigned int new_class = dialog_->classesCO->currentItem();
	if (params.textclass != new_class) {
		// try to load new_class
		if (textclasslist.Load(new_class)) {
			// successfully loaded
			redo = true;
			setMinibuffer(lv_, _("Converting document to new document class..."));
			CutAndPaste cap;
			int ret = cap.SwitchLayoutsBetweenClasses(
			    params.textclass, new_class,
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
			dialog_->classesCO->setCurrentItem( params.textclass );
		}
	}
	char tmpsep = params.paragraph_separation;
	if( dialog_->indentRB->isChecked() )
		params.paragraph_separation = BufferParams::PARSEP_INDENT;
	else
		params.paragraph_separation = BufferParams::PARSEP_SKIP;
	if (tmpsep != params.paragraph_separation)
		redo = true;
	
	VSpace tmpdefskip = params.getDefSkip();
	switch( dialog_->defaultSkipCO->currentItem() ) {
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
			(VSpace(LyXGlueLength(dialog_->defaultSkipED->text().latin1())));
		break;
		// DocumentDefskipCB assures that this never happens
	default:
		params.setDefSkip(VSpace(VSpace::MEDSKIP));
		break;
	}
	if (!(tmpdefskip == params.getDefSkip()))
		redo = true;
	
	if( dialog_->twoColumnsRB->isChecked() )
		params.columns = 2;
	else
		params.columns = 1;
	if( dialog_->twoSidesRB->isChecked() )
		params.sides = LyXTextClass::TwoSides;
	else
		params.sides = LyXTextClass::OneSide;
	
	Spacing tmpSpacing = params.spacing;
	switch( dialog_->spacingCO->currentItem() ) {
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
				   dialog_->defaultSkipED->text().latin1() );
		break;
	}
	if (tmpSpacing != params.spacing)
		redo = true;
	
	params.options = dialog_->extraOptionsED->text();
	
	return redo;
}


void FormDocument::paper_apply()
{
    BufferParams & params = lv_->buffer()->params;
    
    params.papersize2 = dialog_->papersizeCO->currentItem();
    params.paperpackage = dialog_->specialCO->currentItem();
    params.use_geometry = dialog_->CheckBox1->isChecked();
    if( dialog_->landscapeRB->isChecked() )
	params.orientation = BufferParams::ORIENTATION_LANDSCAPE;
    else
	params.orientation = BufferParams::ORIENTATION_PORTRAIT;
    params.paperwidth = dialog_->customWidthED->text();
    params.paperheight = dialog_->customHeightED->text();
    params.leftmargin = dialog_->marginLeftED->text();
    params.topmargin = dialog_->marginTopED->text();
    params.rightmargin = dialog_->marginRightED->text();
    params.bottommargin = dialog_->marginBottomED->text();
    params.headheight = dialog_->headheightED->text();
    params.headsep = dialog_->headsepED->text();
    params.footskip = dialog_->footskipED->text();
    lv_->buffer()->setPaperStuff();
}


bool FormDocument::language_apply()
{
    BufferParams & params = lv_->buffer()->params;
    InsetQuotes::quote_language lga = InsetQuotes::EnglishQ;
    bool redo = false;

    switch( dialog_->quoteStyleTypeCO->currentItem() ) {
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
    if( dialog_->quoteStyleSingleRB->isChecked() )
	params.quotes_times = InsetQuotes::SingleQ;
    else
	params.quotes_times = InsetQuotes::DoubleQ;

    Language const * old_language = params.language;
    Language const * new_language =
	  languages.getLanguage( dialog_->languageCO->currentText().latin1() );
	if( !new_language )
	  new_language = default_language;

    if (old_language != new_language
	&& old_language->RightToLeft() == new_language->RightToLeft()
	&& !lv_->buffer()->isMultiLingual())
	lv_->buffer()->ChangeLanguage(old_language, new_language);
    if (old_language != new_language) {
	redo = true;
    }
    params.language = new_language;
    params.inputenc = dialog_->encodingCO->currentText();

    return redo;
}


bool FormDocument::options_apply()
{
    BufferParams & params = lv_->buffer()->params;
    bool redo = false;

    params.graphicsDriver =
      dialog_->psDriverCO->currentText();
    params.use_amsmath = dialog_->amsMathCB->isChecked();

    int tmpchar = dialog_->sectionNumberDepthSB->value();
    if (params.secnumdepth != tmpchar)
	redo = true;
    params.secnumdepth = tmpchar;
   
    params.tocdepth = dialog_->tocDepthSB->value();

    params.float_placement =
      dialog_->floatPlacementED->text();

    return redo;
}


void FormDocument::bullets_apply()
{
    /* update the bullet settings */
    BufferParams & param = lv_->buffer()->params;
    
    // a little bit of loop unrolling
    param.user_defined_bullets[0] = param.temp_bullets[0];
    param.user_defined_bullets[1] = param.temp_bullets[1];
    param.user_defined_bullets[2] = param.temp_bullets[2];
    param.user_defined_bullets[3] = param.temp_bullets[3];
}


void FormDocument::cancel()
{
    // this avoids confusion when reopening
    BufferParams & param = lv_->buffer()->params;
    param.temp_bullets[0] = param.user_defined_bullets[0];
    param.temp_bullets[1] = param.user_defined_bullets[1];
    param.temp_bullets[2] = param.user_defined_bullets[2];
    param.temp_bullets[3] = param.user_defined_bullets[3];
}


void FormDocument::update()
{
    if (!dialog_)
        return;

    checkReadOnly();

    BufferParams const & params = lv_->buffer()->params;

    class_update(params);
    paper_update(params);
    language_update(params);
    options_update(params);
    bullets_update(params);
}


void FormDocument::class_update(BufferParams const & params)
{
    LyXTextClass const & tclass = textclasslist.TextClass(params.textclass);

    for( int i = 0; i < dialog_->classesCO->count(); i++ )
      if( dialog_->classesCO->text( i ) == textclasslist.DescOfClass( params.textclass ).c_str() )
	dialog_->classesCO->setCurrentItem( i );

    for( int i = 0; i < dialog_->fontsCO->count(); i++ )
      if( dialog_->fontsCO->text( i ) == params.fonts.c_str() )
	dialog_->fontsCO->setCurrentItem( i );

    dialog_->fontSizeCO->clear();
    dialog_->fontSizeCO->insertItem( _("default") );
    dialog_->fontSizeCO->insertStringList( QStringList::split( "|", tclass.opt_fontsize().c_str() ) );
    for( int i = 0; i < dialog_->fontSizeCO->count(); i++ )
      if( dialog_->fontSizeCO->text( i ) ==
	  tokenPos(tclass.opt_fontsize(), '|', params.fontsize)+2)
	dialog_->fontSizeCO->setCurrentItem( i );

    dialog_->pagestyleCO->clear();
    dialog_->pagestyleCO->insertItem( _("default") );
    dialog_->pagestyleCO->insertStringList( QStringList::split( "|", tclass.opt_pagestyle().c_str() ) );
    for( int i = 0; i < dialog_->pagestyleCO->count(); i++ )
      if( dialog_->pagestyleCO->text( i ) ==
	  tokenPos(tclass.opt_pagestyle(), '|', params.pagestyle)+2)
	dialog_->pagestyleCO->setCurrentItem( i );

    if (params.paragraph_separation == BufferParams::PARSEP_INDENT) {
      dialog_->indentRB->setChecked( true );
      dialog_->skipRB->setChecked( false );
    } else {
      dialog_->skipRB->setChecked( true );
      dialog_->indentRB->setChecked( false );
    }
    switch (params.getDefSkip().kind()) {
    case VSpace::SMALLSKIP: 
      dialog_->defaultSkipCO->setCurrentItem( 0 );
	break;
    case VSpace::MEDSKIP: 
      dialog_->defaultSkipCO->setCurrentItem( 1 );
	break;
    case VSpace::BIGSKIP: 
      dialog_->defaultSkipCO->setCurrentItem( 2 );
	break;
    case VSpace::LENGTH: 
      dialog_->defaultSkipCO->setCurrentItem( 3 );
      dialog_->defaultSkipED->setText( params.getDefSkip().asLyXCommand().c_str() );
	break;
    default:
      dialog_->defaultSkipCO->setCurrentItem( 1 );
	break;
    }

    if( params.sides == 2 ) {
      dialog_->twoSidesRB->setChecked( true );
      dialog_->oneSideRB->setChecked( false );
    } else {
      dialog_->twoSidesRB->setChecked( false );
      dialog_->oneSideRB->setChecked( true );
    }

    if( params.columns == 2 ) {
      dialog_->twoColumnsRB->setChecked( true );
      dialog_->oneColumnRB->setChecked( false );
    } else {
      dialog_->twoColumnsRB->setChecked( false );
      dialog_->oneColumnRB->setChecked( true );
    }

    dialog_->spacingED->setText( "" );
    switch (params.spacing.getSpace()) {
    case Spacing::Default: // nothing bad should happen with this
    case Spacing::Single:
      // \singlespacing
      dialog_->spacingCO->setCurrentItem( 0 );
      break;
    case Spacing::Onehalf:
      // \onehalfspacing
      dialog_->spacingCO->setCurrentItem( 1 );
      break;
    case Spacing::Double:
      // \doublespacing
      dialog_->spacingCO->setCurrentItem( 2 );
      break;
    case Spacing::Other:
      {
	dialog_->spacingCO->setCurrentItem( 3 );
	QString sval;
	sval.sprintf("%g",params.spacing.getValue()); 
	dialog_->spacingED->setText( sval );
	break;
      }
    }
    if (!params.options.empty())
      dialog_->extraOptionsED->setText( params.options.c_str());
    else
      dialog_->extraOptionsED->setText( "" );
}


void FormDocument::language_update(BufferParams const & params)
{
    for( int i = 0; i < dialog_->languageCO->count(); i++ )
      if( dialog_->languageCO->text( i ) == params.language->lang().c_str() )
	dialog_->languageCO->setCurrentItem( i );
    

    for( int i = 0; i < dialog_->encodingCO->count(); i++ )
      if( dialog_->encodingCO->text( i ) == params.inputenc.c_str() )
	dialog_->encodingCO->setCurrentItem( i );

    dialog_->quoteStyleTypeCO->setCurrentItem( params.quotes_language );

    if( params.quotes_times == InsetQuotes::SingleQ ) {
      dialog_->quoteStyleSingleRB->setChecked( true );
      dialog_->quoteStyleDoubleRB->setChecked( false );
    } else {
      dialog_->quoteStyleSingleRB->setChecked( false );
      dialog_->quoteStyleDoubleRB->setChecked( true );
    }
}


void FormDocument::options_update(BufferParams const & params)
{
    for( int i = 0; i < dialog_->psDriverCO->count(); i++ )
      if( dialog_->psDriverCO->text( i ) == params.graphicsDriver.c_str() )
	dialog_->psDriverCO->setCurrentItem( i );

    dialog_->amsMathCB->setChecked( params.use_amsmath );
    dialog_->sectionNumberDepthSB->setValue( params.secnumdepth );
    dialog_->tocDepthSB->setValue( params.tocdepth );

    if (!params.float_placement.empty())
      dialog_->floatPlacementED->setText( params.float_placement.c_str());
    else
      dialog_->floatPlacementED->setText( "" );
}


void FormDocument::paper_update(BufferParams const & params)
{
    dialog_->papersizeCO->setCurrentItem( params.papersize2 );
    dialog_->specialCO->setCurrentItem( params.paperpackage );
    dialog_->CheckBox1->setChecked( params.use_geometry );
    
    if (params.orientation == BufferParams::ORIENTATION_LANDSCAPE) {
      dialog_->landscapeRB->setChecked( true );
      dialog_->portraitRB->setChecked( false );
    } else {
      dialog_->landscapeRB->setChecked( false );
      dialog_->portraitRB->setChecked( true );
    }

    dialog_->customWidthED->setText( params.paperwidth.c_str());
    dialog_->customHeightED->setText( params.paperheight.c_str());
    dialog_->marginLeftED->setText( params.leftmargin.c_str());
    dialog_->marginTopED->setText( params.topmargin.c_str());
    dialog_->marginRightED->setText( params.rightmargin.c_str());
    dialog_->marginBottomED->setText( params.bottommargin.c_str());
    dialog_->headheightED->setText( params.headheight.c_str());
    dialog_->headsepED->setText( params.headsep.c_str());
    dialog_->footskipED->setText( params.footskip.c_str());
    dialog_->papersizeCO->setFocus();
}


void FormDocument::bullets_update(BufferParams const & params)
{
    if (lv_->buffer()->isLinuxDoc()) {
      dialog_->bulletTypeBG->setEnabled( false );
      dialog_->bulletDepth1PB->setEnabled( false );
      dialog_->bulletDepth2PB->setEnabled( false );
      dialog_->bulletDepth3PB->setEnabled( false );
      dialog_->bulletDepth4PB->setEnabled( false );
      dialog_->bulletSizeCO->setEnabled( false );
      dialog_->latexED->setEnabled( false );
      return;
    } else {
      dialog_->bulletTypeBG->setEnabled( true );
      dialog_->bulletDepth1PB->setEnabled( true );
      dialog_->bulletDepth2PB->setEnabled( true );
      dialog_->bulletDepth3PB->setEnabled( true );
      dialog_->bulletDepth4PB->setEnabled( true );
      dialog_->bulletSizeCO->setEnabled( true );
      dialog_->latexED->setEnabled( true );
    }
    if (lv_->buffer()->isReadonly()) {
      dialog_->bulletTypeBG->setEnabled( false );
      dialog_->bulletSizeCO->setEnabled( false );
      dialog_->latexED->setEnabled( false );
    } else {
      dialog_->bulletTypeBG->setEnabled( true );
      dialog_->bulletSizeCO->setEnabled( true );
      dialog_->latexED->setEnabled( true );
    }

    dialog_->bulletDepth1PB->setOn( true );
    dialog_->latexED->setText( params.user_defined_bullets[0].getText().c_str());
    dialog_->bulletSizeCO->setCurrentItem( params.user_defined_bullets[0].getSize() + 1);
    dialog_->bulletStandardPB->setOn( false );
    dialog_->bulletMathsPB->setOn( false );
    dialog_->bulletDing1PB->setOn( false );
    dialog_->bulletDing2PB->setOn( false );
    dialog_->bulletDing3PB->setOn( false );
    dialog_->bulletDing4PB->setOn( false );
    switch( params.user_defined_bullets[0].getFont() ) {
    case 0:
      dialog_->bulletStandardPB->setOn( true );
      break;
    case 1:
      dialog_->bulletMathsPB->setOn( true );
      break;
    case 2:
      dialog_->bulletDing1PB->setOn( true );
      break;
    case 3:
      dialog_->bulletDing2PB->setOn( true );
      break;
    case 4:
      dialog_->bulletDing3PB->setOn( true );
      break;
    case 5:
      dialog_->bulletDing4PB->setOn( true );
      break;
    default:
      // should not happen
      dialog_->bulletStandardPB->setOn( true );
    }
}


void FormDocument::free()
{
    if (dialog_) {
        hide();
        delete dialog_;
        dialog_ = 0;
    }
}


void FormDocument::checkReadOnly()
{
    if (bc_.readOnly(lv_->buffer()->isReadonly())) {
      dialog_->classesCO->setEnabled( false );
      dialog_->languageCO->setEnabled( false );
      dialog_->warningLA->setText( _("Document is read-only."
				     " No changes to layout permitted."));
      dialog_->warningLA->show();
    } else {
      dialog_->classesCO->setEnabled( true );
      dialog_->languageCO->setEnabled( true );
      dialog_->warningLA->hide();
    }	
}


void FormDocument::checkMarginValues()
{
  const int allEmpty = 
    dialog_->marginTopED->text().isEmpty() &&
    dialog_->marginBottomED->text().isEmpty() &&
    dialog_->marginLeftED->text().isEmpty() &&
    dialog_->marginRightED->text().isEmpty() &&
    dialog_->headheightED->text().isEmpty() &&
    dialog_->headsepED->text().isEmpty() &&
    dialog_->footskipED->text().isEmpty() &&
    dialog_->customWidthED->text().isEmpty() &&
    dialog_->customHeightED->text().isEmpty();
    if (!allEmpty)
      dialog_->CheckBox1->setChecked( true );
}

bool FormDocument::checkDocumentInput(QWidget* w)
{
    string str;
    char val;
    bool ok = true;
    QString input;
    
    checkMarginValues();
    if (w == dialog_->papersizeCO) {
	val = dialog_->papersizeCO->currentItem();

	if (val == BufferParams::VM_PAPER_DEFAULT) {
	  dialog_->CheckBox1->setChecked( false );
	    checkMarginValues();
	} else {
	    if ((val != BufferParams::VM_PAPER_USLETTER) &&
		(val != BufferParams::VM_PAPER_USLEGAL) &&
		(val != BufferParams::VM_PAPER_USEXECUTIVE) &&
		(val != BufferParams::VM_PAPER_A4) &&
		(val != BufferParams::VM_PAPER_A5) &&
		(val != BufferParams::VM_PAPER_B5)) {
	      dialog_->CheckBox1->setChecked( true );
	    }
	    dialog_->specialCO->setCurrentItem( BufferParams::PACKAGE_NONE );
	}
    } else if (w == dialog_->specialCO ) {
      val = dialog_->specialCO->currentItem();
      if (val != BufferParams::PACKAGE_NONE) {
	dialog_->papersizeCO->setCurrentItem( BufferParams::VM_PAPER_DEFAULT );
	dialog_->CheckBox1->setChecked( false );
      }
    } else if (w == dialog_->spacingED) {
	input = dialog_->spacingED->text();
	if (input.isEmpty()) {
	  dialog_->spacingCO->setCurrentItem( 0 );
	} else {
	  dialog_->spacingCO->setCurrentItem( 3 );
	}
    }
    // this has to be all out of if/elseif because it has to deactivate
    // the document buttons and so the whole stuff has to be tested again.
    str = dialog_->customWidthED->text();
    ok = ok && (str.empty() || isValidLength(str));
    str = dialog_->customHeightED->text();
    ok = ok && (str.empty() || isValidLength(str));
    str = dialog_->marginLeftED->text();
    ok = ok && (str.empty() || isValidLength(str));
    str = dialog_->marginRightED->text();
    ok = ok && (str.empty() || isValidLength(str));
    str = dialog_->marginTopED->text();
    ok = ok && (str.empty() || isValidLength(str));
    str = dialog_->marginBottomED->text();
    ok = ok && (str.empty() || isValidLength(str));
    str = dialog_->headheightED->text();
    ok = ok && (str.empty() || isValidLength(str));
    str = dialog_->headsepED->text();
    ok = ok && (str.empty() || isValidLength(str));
    str = dialog_->footskipED->text();
    ok = ok && (str.empty() || isValidLength(str));
    // "Synchronize" the choice and the input field, so that it
    // is impossible to commit senseless data.
    input = dialog_->defaultSkipED->text();
    if (w == dialog_->defaultSkipED) {
	if (input.isEmpty()) {
	  dialog_->defaultSkipCO->setCurrentItem( 1 );
	} else if (isValidGlueLength (input.latin1())) {
	  dialog_->defaultSkipCO->setCurrentItem( 3 );
	} else {
	  dialog_->defaultSkipCO->setCurrentItem( 3 );
	    ok = false;
	}
    } else {
	if (!input.isEmpty() && !isValidGlueLength(input.latin1()))
	    ok = false;
    }
    if(( dialog_->defaultSkipCO->currentItem() == 3 ) && input.isEmpty() )
	ok = false;
    else if( dialog_->defaultSkipCO->currentItem() != 3 )
      dialog_->defaultSkipED->setText( "" );

    input = dialog_->spacingED->text();
    if( ( dialog_->spacingCO->currentItem() == 3 ) && input.isEmpty() )
	ok = false;
    else if( dialog_->spacingCO->currentItem() != 3 )
      dialog_->spacingED->setText( "" );
    return ok;
}


void FormDocument::choiceBulletSize()
{
    BufferParams & param = lv_->buffer()->params;

    // convert from 0-5 range to -1-4 
    param.temp_bullets[currentBulletDepth].setSize(dialog_->bulletSizeCO->currentItem() - 1);
    dialog_->latexED->setText( param.temp_bullets[currentBulletDepth].getText().c_str());
}


void FormDocument::inputBulletLaTeX()
{
    BufferParams & param = lv_->buffer()->params;

    param.temp_bullets[currentBulletDepth].
	setText(dialog_->latexED->text().latin1() );
}


void FormDocument::bulletDepth(int depth)
{
    /* Should I do the following:                                 */
    /*  1. change to the panel that the current bullet belongs in */
    /*  2. show that bullet as selected                           */
    /*  3. change the size setting to the size of the bullet in Q.*/
    /*  4. display the latex equivalent in the latex box          */
    /*                                                            */
    /* I'm inclined to just go with 3 and 4 at the moment and     */
    /* maybe try to support the others later                      */
    BufferParams & param = lv_->buffer()->params;

    // reset on right mouse button not supported in KLyX
    // right mouse button resets to default
    param.temp_bullets[depth] = ITEMIZE_DEFAULTS[depth];
    dialog_->latexED->setText( param.temp_bullets[depth].getText().c_str());
    dialog_->bulletSizeCO->setCurrentItem( param.temp_bullets[depth].getSize() + 1 );

    currentBulletDepth = depth;
}


// PENDING(kalle) Call this.
void FormDocument::setBulletPics()
{
  QPixmap* currentpix = 0;
  if( dialog_->bulletStandardPB->isOn() ) {
    currentpix = standardpix;
    currentBulletPanel = 0;
  } else if( dialog_->bulletMathsPB->isOn() ) {
    currentpix = amssymbpix;
    currentBulletPanel = 1;
  } else if( dialog_->bulletDing1PB->isOn() ) {
    currentpix = psnfss1pix;
    currentBulletPanel = 2;
  } else if( dialog_->bulletDing2PB->isOn() ) {
    currentpix = psnfss2pix;
    currentBulletPanel = 3;
  } else if( dialog_->bulletDing3PB->isOn() ) {
    currentpix = psnfss3pix;
    currentBulletPanel = 4;
  } else if( dialog_->bulletDing4PB->isOn() ) {
    currentpix = psnfss4pix;
    currentBulletPanel = 5;
  }

  for( int x = 0; x < 6; x++ )
    for( int y = 0; y < 6; y++ ) {
      bulletpics[x*6+y].resize( 44, 29 );
      bitBlt( &bulletpics[x*6+y], 0, 0, currentpix, x*44, y*29, 44, 29 );
    }

  // This is disgusting, but the only way since the designer does not
  // support widget arrays.
  dialog_->bullet00PB->setPixmap( bulletpics[0] );
  dialog_->bullet01PB->setPixmap( bulletpics[1] );
  dialog_->bullet02PB->setPixmap( bulletpics[2] );
  dialog_->bullet03PB->setPixmap( bulletpics[3] );
  dialog_->bullet04PB->setPixmap( bulletpics[4] );
  dialog_->bullet05PB->setPixmap( bulletpics[5] );
  dialog_->bullet10PB->setPixmap( bulletpics[6] );
  dialog_->bullet11PB->setPixmap( bulletpics[7] );
  dialog_->bullet12PB->setPixmap( bulletpics[8] );
  dialog_->bullet13PB->setPixmap( bulletpics[9] );
  dialog_->bullet14PB->setPixmap( bulletpics[10] );
  dialog_->bullet15PB->setPixmap( bulletpics[11] );
  dialog_->bullet20PB->setPixmap( bulletpics[12] );
  dialog_->bullet21PB->setPixmap( bulletpics[13] );
  dialog_->bullet22PB->setPixmap( bulletpics[14] );
  dialog_->bullet23PB->setPixmap( bulletpics[15] );
  dialog_->bullet24PB->setPixmap( bulletpics[16] );
  dialog_->bullet25PB->setPixmap( bulletpics[17] );
  dialog_->bullet30PB->setPixmap( bulletpics[18] );
  dialog_->bullet31PB->setPixmap( bulletpics[19] );
  dialog_->bullet32PB->setPixmap( bulletpics[20] );
  dialog_->bullet33PB->setPixmap( bulletpics[21] );
  dialog_->bullet34PB->setPixmap( bulletpics[22] );
  dialog_->bullet35PB->setPixmap( bulletpics[23] );
  dialog_->bullet40PB->setPixmap( bulletpics[24] );
  dialog_->bullet41PB->setPixmap( bulletpics[25] );
  dialog_->bullet42PB->setPixmap( bulletpics[26] );
  dialog_->bullet43PB->setPixmap( bulletpics[27] );
  dialog_->bullet44PB->setPixmap( bulletpics[28] );
  dialog_->bullet45PB->setPixmap( bulletpics[29] );
  dialog_->bullet50PB->setPixmap( bulletpics[30] );
  dialog_->bullet51PB->setPixmap( bulletpics[31] );
  dialog_->bullet52PB->setPixmap( bulletpics[32] );
  dialog_->bullet53PB->setPixmap( bulletpics[33] );
  dialog_->bullet54PB->setPixmap( bulletpics[34] );
  dialog_->bullet55PB->setPixmap( bulletpics[35] );
}


void FormDocument::bulletBMTable(int button)
{
    /* handle the user input by setting the current bullet depth's pixmap */
    /* to that extracted from the current chosen position of the BMTable  */
    /* Don't forget to free the button's old pixmap first.                */

    BufferParams & param = lv_->buffer()->params;

    /* try to keep the button held down till another is pushed */
    /*  fl_set_bmtable(ob, 1, bmtable_button); */
    param.temp_bullets[currentBulletDepth].setFont(currentBulletPanel);
    param.temp_bullets[currentBulletDepth].setCharacter(button);
    dialog_->latexED->setText( param.temp_bullets[currentBulletDepth].getText().c_str());
}


void FormDocument::checkChoiceClass(QComboBox* cb)
{
    if (!cb)
	cb = dialog_->classesCO;

    ProhibitInput(lv_->view());
    int tc;
    string tct;

    tc = cb->currentItem();
    tct = cb->currentText();

    if (textclasslist.Load(tc)) {
	if (AskQuestion(_("Should I set some parameters to"), tct,
			_("the defaults of this document class?"))) {
	    BufferParams & params = lv_->buffer()->params;

	    params.textclass = tc;
	    params.useClassDefaults();
	    UpdateLayoutDocument(params);
	}
    } else {
	// unable to load new style
	WriteAlert(_("Conversion Errors!"),
		   _("Unable to switch to new document class."),
		   _("Reverting to original document class."));

	dialog_->classesCO->setCurrentItem(
					   lv_->buffer()->params.textclass );
    }
    AllowInput(lv_->view());
}


void FormDocument::UpdateLayoutDocument(BufferParams const & params)
{
    if (!dialog_)
        return;

    checkReadOnly();
    class_update(params);
    paper_update(params);
    language_update(params);
    options_update(params);
    bullets_update(params);
}
