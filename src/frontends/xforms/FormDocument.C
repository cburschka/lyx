/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 2000-2001 The LyX Team.
 *
 *           @author Jürgen Vigna
 *
 *======================================================*/

#include <config.h>

#include FORMS_H_LOCATION
#include XPM_H_LOCATION

#ifdef __GNUG_
#pragma implementation
#endif

#include "lyx_gui_misc.h"

#include "FormDocument.h"
#include "form_document.h"
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
#include "xforms_helpers.h" 

using Liason::setMinibuffer;
using SigC::slot;

FormDocument::FormDocument(LyXView * lv, Dialogs * d)
	: FormBaseBD(lv, d, _("Document Layout")), fbullet(0)
{
    // let the dialog be shown
    // This is a permanent connection so we won't bother
    // storing a copy because we won't be disconnecting.
    d->showDocument.connect(slot(this, &FormDocument::show));
}


void FormDocument::redraw()
{
	if( form() && form()->visible )
		fl_redraw_form( form() );
	else
		return;

	FL_FORM * outer_form = fl_get_active_folder(dialog_->tabbed_folder);
	if (outer_form && outer_form->visible)
		fl_redraw_form( outer_form );
}


FL_FORM * FormDocument::form() const
{
    if (dialog_.get()) return dialog_->form;
    return 0;
}


void FormDocument::build()
{
    int n;

    // the tabbed folder
    dialog_.reset(build_tabbed_document());

    // Manage the restore, ok, apply, restore and cancel/close buttons
    bc().setOK(dialog_->button_ok);
    bc().setApply(dialog_->button_apply);
    bc().setCancel(dialog_->button_cancel);
    bc().setRestore(dialog_->button_restore);

    // the document paper form
    paper_.reset(build_doc_paper());
    fl_addto_choice(paper_->choice_papersize2,
		    _(" Default | Custom | USletter | USlegal "
		      "| USexecutive | A3 | A4 | A5 | B3 | B4 | B5 "));
    fl_addto_choice(paper_->choice_paperpackage,
		    _(" None "
		      "| A4 small Margins (only portrait) "
		      "| A4 very small Margins (only portrait) "
		      "| A4 very wide margins (only portrait) "));
    fl_set_input_return(paper_->input_custom_width, FL_RETURN_CHANGED);
    fl_set_input_return(paper_->input_custom_height, FL_RETURN_CHANGED);
    fl_set_input_return(paper_->input_top_margin, FL_RETURN_CHANGED);
    fl_set_input_return(paper_->input_bottom_margin, FL_RETURN_CHANGED);
    fl_set_input_return(paper_->input_left_margin, FL_RETURN_CHANGED);
    fl_set_input_return(paper_->input_right_margin, FL_RETURN_CHANGED);
    fl_set_input_return(paper_->input_head_height, FL_RETURN_CHANGED);
    fl_set_input_return(paper_->input_head_sep, FL_RETURN_CHANGED);
    fl_set_input_return(paper_->input_foot_skip, FL_RETURN_CHANGED);

    bc().addReadOnly (paper_->choice_paperpackage);
    bc().addReadOnly (paper_->greoup_radio_orientation);
    bc().addReadOnly (paper_->radio_portrait);
    bc().addReadOnly (paper_->radio_landscape);
    bc().addReadOnly (paper_->choice_papersize2);
    bc().addReadOnly (paper_->push_use_geometry);
    bc().addReadOnly (paper_->input_custom_width);
    bc().addReadOnly (paper_->input_custom_height);
    bc().addReadOnly (paper_->input_top_margin);
    bc().addReadOnly (paper_->input_bottom_margin);
    bc().addReadOnly (paper_->input_left_margin);
    bc().addReadOnly (paper_->input_right_margin);
    bc().addReadOnly (paper_->input_head_height);
    bc().addReadOnly (paper_->input_head_sep);
    bc().addReadOnly (paper_->input_foot_skip);

    // the document class form
    class_.reset(build_doc_class());

    FL_OBJECT * obj;
    // The language is a combo-box and has to be inserted manually
    obj = class_->choice_doc_class;
    fl_deactivate_object(obj);
    fl_addto_form(class_->form);
    combo_doc_class.reset(new Combox(FL_COMBOX_DROPLIST));
    combo_doc_class->add(obj->x, obj->y, obj->w, obj->h, 400,
	 		 dialog_->tabbed_folder);
    combo_doc_class->shortcut("#C",1);
    combo_doc_class->setcallback(ComboInputCB, this);
    fl_end_form();
    for (LyXTextClassList::const_iterator cit = textclasslist.begin();
	 cit != textclasslist.end(); ++cit)
    {
	combo_doc_class->addto((*cit).description());
    }

    fl_addto_choice(class_->choice_doc_spacing,
		    _(" Single | OneHalf | Double | Other "));
    fl_addto_choice(class_->choice_doc_fontsize, "default|10|11|12");
   for (n=0; tex_fonts[n][0]; ++n) {
	fl_addto_choice(class_->choice_doc_fonts,tex_fonts[n]);
    }
    fl_addto_choice(class_->choice_doc_pagestyle,
		    "default|empty|plain|headings|fancy");
    fl_addto_choice(class_->choice_doc_skip,
		    _(" Smallskip | Medskip | Bigskip | Length "));
    fl_set_input_return(class_->input_doc_extra, FL_RETURN_CHANGED);
    fl_set_input_return(class_->input_doc_skip, FL_RETURN_CHANGED);
    fl_set_input_return(class_->input_doc_spacing, FL_RETURN_CHANGED);

    bc().addReadOnly (class_->radio_doc_indent);
    bc().addReadOnly (class_->radio_doc_skip);

    bc().addReadOnly (class_->choice_doc_pagestyle);
    bc().addReadOnly (class_->choice_doc_fonts);
    bc().addReadOnly (class_->choice_doc_fontsize);
    bc().addReadOnly (class_->radio_doc_sides_one);
    bc().addReadOnly (class_->radio_doc_sides_two);
    bc().addReadOnly (class_->radio_doc_columns_one);
    bc().addReadOnly (class_->radio_doc_columns_two);
    bc().addReadOnly (class_->input_doc_extra);
    bc().addReadOnly (class_->input_doc_skip);
    bc().addReadOnly (class_->choice_doc_skip);
    bc().addReadOnly (class_->choice_doc_spacing);
    bc().addReadOnly (class_->input_doc_spacing);

    // the document language form
    language_.reset(build_doc_language());
    fl_addto_choice(language_->choice_inputenc,
		    "default|auto|latin1|latin2|latin5|latin9"
		    "|koi8-r|koi8-u|cp866|cp1251|iso88595");

    // The language is a combo-box and has to be inserted manually
    obj = language_->choice_language;
    fl_deactivate_object(obj);
    fl_addto_form(language_->form);
    combo_language.reset(new Combox(FL_COMBOX_DROPLIST));
    combo_language->add(obj->x, obj->y, obj->w, obj->h, 400,
			dialog_->tabbed_folder);
    combo_language->shortcut("#L",1);
    combo_language->setcallback(ComboInputCB, this);
    fl_end_form();

    for (Languages::const_iterator cit = languages.begin();
	cit != languages.end(); ++cit) {
	combo_language->addto((*cit).second.lang());
    }

    fl_addto_choice(language_->choice_quotes_language,
		    _(" ``text'' | ''text'' | ,,text`` | ,,text'' |"
		      " «text» | »text« "));

    bc().addReadOnly (language_->choice_inputenc);

    // the document options form
    options_.reset(build_doc_options());
    fl_set_input_return(options_->input_float_placement, FL_RETURN_CHANGED);
    fl_set_counter_bounds(options_->slider_secnumdepth,-2,5);
    fl_set_counter_bounds(options_->slider_tocdepth,-1,5);
    fl_set_counter_step(options_->slider_secnumdepth,1,1);
    fl_set_counter_step(options_->slider_tocdepth,1,1);
    fl_set_counter_precision(options_->slider_secnumdepth, 0);
    fl_set_counter_precision(options_->slider_tocdepth, 0);
    for (n=0; tex_graphics[n][0]; ++n) {
	fl_addto_choice(options_->choice_postscript_driver, tex_graphics[n]);
    }

    bc().addReadOnly (options_->slider_secnumdepth);
    bc().addReadOnly (options_->slider_tocdepth);
    bc().addReadOnly (options_->check_use_amsmath);
    bc().addReadOnly (options_->input_float_placement);
    bc().addReadOnly (options_->choice_postscript_driver);

    // the document bullets form
    bullets_.reset(build_doc_bullet());
    fl_addto_choice(bullets_->choice_bullet_size,
		    _(" default | tiny | script | footnote | small |"
		      " normal | large | Large | LARGE | huge | Huge"));
    fl_set_choice(bullets_->choice_bullet_size, 1);
    fl_set_input_return(bullets_->input_bullet_latex, FL_RETURN_CHANGED);
    fl_set_input_maxchars(bullets_->input_bullet_latex, 80);

    bc().addReadOnly (bullets_->bmtable_bullet_panel);
    bc().addReadOnly (bullets_->choice_bullet_size);
    bc().addReadOnly (bullets_->input_bullet_latex);

    fl_addto_tabfolder(dialog_->tabbed_folder,_("Document"),
		       class_->form);
    fl_addto_tabfolder(dialog_->tabbed_folder,_("Paper"),
                       paper_->form);
    fl_addto_tabfolder(dialog_->tabbed_folder,_("Language"),
                       language_->form);
    fl_addto_tabfolder(dialog_->tabbed_folder,_("Extra"),
                       options_->form);
    fbullet = fl_addto_tabfolder(dialog_->tabbed_folder,_("Bullets"),
				 bullets_->form);
    if ((XpmVersion < 4) || (XpmVersion == 4 && XpmRevision < 7)) {
	    lyxerr << _("Your version of libXpm is older than 4.7.\n"
			"The `bullet' tab of the document dialog "
			"has been disabled") << '\n';
	fl_deactivate_object(fbullet);
	fl_set_object_lcol(fbullet, FL_INACTIVE);
    }
}


void FormDocument::apply()
{
    if (!lv_->view()->available() || !dialog_.get())
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


void FormDocument::cancel()
{
    // this avoids confusion when reopening
    BufferParams & param = lv_->buffer()->params;
    param.temp_bullets[0] = param.user_defined_bullets[0];
    param.temp_bullets[1] = param.user_defined_bullets[1];
    param.temp_bullets[2] = param.user_defined_bullets[2];
    param.temp_bullets[3] = param.user_defined_bullets[3];
    hide();
}


void FormDocument::update()
{
    if (!dialog_.get())
        return;

    checkReadOnly();

    BufferParams const & params = lv_->buffer()->params;

    class_update(params);
    paper_update(params);
    language_update(params);
    options_update(params);
    bullets_update(params);
}


bool FormDocument::input( FL_OBJECT * ob, long data )
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


void FormDocument::ComboInputCB(int, void * v, Combox * combox)
{
    FormDocument * pre = static_cast<FormDocument*>(v);
    if (combox == pre->combo_doc_class.get())
	pre->CheckChoiceClass(0, 0);
    pre->bc().valid(pre->CheckDocumentInput(0,0));
}


bool FormDocument::class_apply()
{
	bool redo = false;
	BufferParams &params = lv_->buffer()->params;

	// If default skip is a "Length" but there's no text in the
	// input field, reset the kind to "Medskip", which is the default.
	if ((fl_get_choice (class_->choice_doc_skip) == 4) &&
	    !*(fl_get_input (class_->input_doc_skip))) {
		fl_set_choice (class_->choice_doc_skip, 2);
	}
	params.fonts = fl_get_choice_text(class_->choice_doc_fonts);
	params.fontsize = fl_get_choice_text(class_->choice_doc_fontsize);
	params.pagestyle = fl_get_choice_text(class_->choice_doc_pagestyle);

	unsigned int const new_class = combo_doc_class->get() - 1;
	
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
			combo_doc_class->select(int(params.textclass) + 1);
		}
	}
	BufferParams::PARSEP tmpsep = params.paragraph_separation;
	if (fl_get_button(class_->radio_doc_indent))
		params.paragraph_separation = BufferParams::PARSEP_INDENT;
	else
		params.paragraph_separation = BufferParams::PARSEP_SKIP;
	if (tmpsep != params.paragraph_separation)
		redo = true;
	
	VSpace tmpdefskip = params.getDefSkip();
	switch (fl_get_choice (class_->choice_doc_skip)) {
	case 1:
		params.setDefSkip(VSpace(VSpace::SMALLSKIP));
		break;
	case 2:
		params.setDefSkip(VSpace(VSpace::MEDSKIP));
		break;
    case 3:
	    params.setDefSkip(VSpace(VSpace::BIGSKIP));
	    break;
	case 4:
		params.setDefSkip
			(VSpace(LyXGlueLength(fl_get_input(class_->input_doc_skip))));
		break;
		// DocumentDefskipCB assures that this never happens
	default:
		params.setDefSkip(VSpace(VSpace::MEDSKIP));
		break;
	}
	if (!(tmpdefskip == params.getDefSkip()))
		redo = true;
	
	if (fl_get_button(class_->radio_doc_columns_two))
		params.columns = 2;
	else
		params.columns = 1;
	if (fl_get_button(class_->radio_doc_sides_two))
		params.sides = LyXTextClass::TwoSides;
	else
		params.sides = LyXTextClass::OneSide;
	
	Spacing tmpSpacing = params.spacing;
	switch (fl_get_choice(class_->choice_doc_spacing)) {
	case 1:
		lyxerr[Debug::INFO] << "Spacing: SINGLE\n";
		params.spacing.set(Spacing::Single);
		break;
	case 2:
		lyxerr[Debug::INFO] << "Spacing: ONEHALF\n";
		params.spacing.set(Spacing::Onehalf);
		break;
	case 3:
		lyxerr[Debug::INFO] << "Spacing: DOUBLE\n";
		params.spacing.set(Spacing::Double);
		break;
	case 4:
		lyxerr[Debug::INFO] << "Spacing: OTHER\n";
		params.spacing.set(Spacing::Other, 
				   fl_get_input(class_->input_doc_spacing));
		break;
	}
	if (tmpSpacing != params.spacing)
		redo = true;
	
	params.options = fl_get_input(class_->input_doc_extra);
	
	return redo;
}


void FormDocument::paper_apply()
{
    BufferParams & params = lv_->buffer()->params;
    
    params.papersize2 =
	    static_cast<char>(fl_get_choice(paper_->choice_papersize2)-1);
    params.paperpackage =
	    static_cast<char>(fl_get_choice(paper_->choice_paperpackage)-1);
    params.use_geometry = fl_get_button(paper_->push_use_geometry);
    if (fl_get_button(paper_->radio_landscape))
	params.orientation = BufferParams::ORIENTATION_LANDSCAPE;
    else
	params.orientation = BufferParams::ORIENTATION_PORTRAIT;
    params.paperwidth = fl_get_input(paper_->input_custom_width);
    params.paperheight = fl_get_input(paper_->input_custom_height);
    params.leftmargin = fl_get_input(paper_->input_left_margin);
    params.topmargin = fl_get_input(paper_->input_top_margin);
    params.rightmargin = fl_get_input(paper_->input_right_margin);
    params.bottommargin = fl_get_input(paper_->input_bottom_margin);
    params.headheight = fl_get_input(paper_->input_head_height);
    params.headsep = fl_get_input(paper_->input_head_sep);
    params.footskip = fl_get_input(paper_->input_foot_skip);
    lv_->buffer()->setPaperStuff();
}


bool FormDocument::language_apply()
{
    BufferParams & params = lv_->buffer()->params;
    InsetQuotes::quote_language lga = InsetQuotes::EnglishQ;
    bool redo = false;

    switch (fl_get_choice(language_->choice_quotes_language) - 1) {
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
    if (fl_get_button(language_->radio_single))   
	params.quotes_times = InsetQuotes::SingleQ;
    else
	params.quotes_times = InsetQuotes::DoubleQ;

    Language const * old_language = params.language;
    Language const * new_language = 
	    languages.getLanguage(combo_language->getline());
    if (!new_language)
	new_language = default_language;

    if (old_language != new_language
	&& old_language->RightToLeft() == new_language->RightToLeft()
	&& !lv_->buffer()->isMultiLingual())
	lv_->buffer()->ChangeLanguage(old_language, new_language);
    if (old_language != new_language) {
	redo = true;
    }
    params.language = new_language;
    params.inputenc = fl_get_choice_text(language_->choice_inputenc);

    return redo;
}


bool FormDocument::options_apply()
{
    BufferParams & params = lv_->buffer()->params;
    bool redo = false;

    params.graphicsDriver =
	fl_get_choice_text(options_->choice_postscript_driver);
    params.use_amsmath = fl_get_button(options_->check_use_amsmath);

    int tmpchar = int(fl_get_counter_value(options_->slider_secnumdepth));
    if (params.secnumdepth != tmpchar)
	redo = true;
    params.secnumdepth = tmpchar;
   
    params.tocdepth = int(fl_get_counter_value(options_->slider_tocdepth));

    params.float_placement =
	fl_get_input(options_->input_float_placement);

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


void FormDocument::class_update(BufferParams const & params)
{
    if (!class_.get())
        return;

    LyXTextClass const & tclass = textclasslist.TextClass(params.textclass);

    combo_doc_class->select_text(
	textclasslist.DescOfClass(params.textclass));
    fl_set_choice_text(class_->choice_doc_fonts, params.fonts.c_str());
    fl_clear_choice(class_->choice_doc_fontsize);
    fl_addto_choice(class_->choice_doc_fontsize, "default");
    fl_addto_choice(class_->choice_doc_fontsize,
		    tclass.opt_fontsize().c_str());
    fl_set_choice(class_->choice_doc_fontsize,
		  tokenPos(tclass.opt_fontsize(), '|', params.fontsize)+2);
    fl_clear_choice(class_->choice_doc_pagestyle);
    fl_addto_choice(class_->choice_doc_pagestyle, "default");
    fl_addto_choice(class_->choice_doc_pagestyle,
		    tclass.opt_pagestyle().c_str());
    fl_set_choice(class_->choice_doc_pagestyle,
		  tokenPos(tclass.opt_pagestyle(), '|', params.pagestyle)+2);
    fl_set_button(class_->radio_doc_indent, 0);
    fl_set_button(class_->radio_doc_skip, 0);
    if (params.paragraph_separation == BufferParams::PARSEP_INDENT)
	fl_set_button(class_->radio_doc_indent, 1);
    else
	fl_set_button(class_->radio_doc_skip, 1);
    switch (params.getDefSkip().kind()) {
    case VSpace::SMALLSKIP: 
	fl_set_choice (class_->choice_doc_skip, 1);
	break;
    case VSpace::MEDSKIP: 
	fl_set_choice (class_->choice_doc_skip, 2);
	break;
    case VSpace::BIGSKIP: 
	fl_set_choice (class_->choice_doc_skip, 3);
	break;
    case VSpace::LENGTH: 
	fl_set_choice (class_->choice_doc_skip, 4);
	fl_set_input (class_->input_doc_skip,
		      params.getDefSkip().asLyXCommand().c_str());
	break;
    default:
	fl_set_choice (class_->choice_doc_skip, 2);
	break;
    }
    fl_set_button(class_->radio_doc_sides_one, 0);
    fl_set_button(class_->radio_doc_sides_two, 0);
    if (params.sides == LyXTextClass::TwoSides)
	fl_set_button(class_->radio_doc_sides_two, 1);
    else
	fl_set_button(class_->radio_doc_sides_one, 1);
    fl_set_button(class_->radio_doc_columns_one, 0);
    fl_set_button(class_->radio_doc_columns_two, 0);
    if (params.columns == 2)
	fl_set_button(class_->radio_doc_columns_two, 1);
    else
	fl_set_button(class_->radio_doc_columns_one, 1);
    fl_set_input(class_->input_doc_spacing, "");
    switch (params.spacing.getSpace()) {
    case Spacing::Default: // nothing bad should happen with this
    case Spacing::Single:
	// \singlespacing
	fl_set_choice(class_->choice_doc_spacing, 1);
	break;
    case Spacing::Onehalf:
	// \onehalfspacing
	fl_set_choice(class_->choice_doc_spacing, 2);
	break;
    case Spacing::Double:
	// \doublespacing
	fl_set_choice(class_->choice_doc_spacing, 3);
	break;
    case Spacing::Other:
	{
	    fl_set_choice(class_->choice_doc_spacing, 4);
	    char sval[20];
	    sprintf(sval,"%g",params.spacing.getValue()); 
	    fl_set_input(class_->input_doc_spacing, sval);
	    break;
	}
    }
    if (!params.options.empty())
	fl_set_input(class_->input_doc_extra, params.options.c_str());
    else
	fl_set_input(class_->input_doc_extra, "");
}


void FormDocument::language_update(BufferParams const & params)
{
    if (!language_.get())
        return;

    combo_language->select_text(params.language->lang());
    fl_set_choice_text(language_->choice_inputenc, params.inputenc.c_str());
    fl_set_choice(language_->choice_quotes_language, params.quotes_language + 1);
    fl_set_button(language_->radio_single, 0);
    fl_set_button(language_->radio_double, 0);
    if (params.quotes_times == InsetQuotes::SingleQ)
	fl_set_button(language_->radio_single, 1);
    else
	fl_set_button(language_->radio_double, 1);
}


void FormDocument::options_update(BufferParams const & params)
{
    if (!options_.get())
        return;

    fl_set_choice_text(options_->choice_postscript_driver,
		       params.graphicsDriver.c_str());
    fl_set_button(options_->check_use_amsmath, params.use_amsmath);
    fl_set_counter_value(options_->slider_secnumdepth, params.secnumdepth);
    fl_set_counter_value(options_->slider_tocdepth, params.tocdepth);
    if (!params.float_placement.empty())
	fl_set_input(options_->input_float_placement,
		     params.float_placement.c_str());
    else
	fl_set_input(options_->input_float_placement, "");
}


void FormDocument::paper_update(BufferParams const & params)
{
    if (!paper_.get())
        return;

    fl_set_choice(paper_->choice_papersize2, params.papersize2 + 1);
    fl_set_choice(paper_->choice_paperpackage, params.paperpackage + 1);
    fl_set_button(paper_->push_use_geometry, params.use_geometry);
    fl_set_button(paper_->radio_portrait, 0);
    fl_set_button(paper_->radio_landscape, 0);
    if (params.orientation == BufferParams::ORIENTATION_LANDSCAPE)
	fl_set_button(paper_->radio_landscape, 1);
    else
	fl_set_button(paper_->radio_portrait, 1);
    fl_set_input(paper_->input_custom_width, params.paperwidth.c_str());
    fl_set_input(paper_->input_custom_height, params.paperheight.c_str());
    fl_set_input(paper_->input_left_margin, params.leftmargin.c_str());
    fl_set_input(paper_->input_top_margin, params.topmargin.c_str());
    fl_set_input(paper_->input_right_margin, params.rightmargin.c_str());
    fl_set_input(paper_->input_bottom_margin, params.bottommargin.c_str());
    fl_set_input(paper_->input_head_height, params.headheight.c_str());
    fl_set_input(paper_->input_head_sep, params.headsep.c_str());
    fl_set_input(paper_->input_foot_skip, params.footskip.c_str());
    fl_set_focus_object(paper_->form, paper_->choice_papersize2);
}


void FormDocument::bullets_update(BufferParams const & params)
{
    if (!bullets_.get() || ((XpmVersion<4) || (XpmVersion==4 && XpmRevision<7)))
        return;

    bool const isLinuxDoc = lv_->buffer()->isLinuxDoc();
    setEnabled(fbullet, !isLinuxDoc);

    if (isLinuxDoc) return;

    fl_set_button(bullets_->radio_bullet_depth_1, 1);
    fl_set_input(bullets_->input_bullet_latex,
		 params.user_defined_bullets[0].getText().c_str());
    fl_set_choice(bullets_->choice_bullet_size,
		  params.user_defined_bullets[0].getSize() + 2);
}


void FormDocument::checkReadOnly()
{
    if (bc().readOnly(lv_->buffer()->isReadonly())) {
	combo_doc_class->deactivate();
	combo_language->deactivate();
	fl_set_object_label(dialog_->text_warning,
			    _("Document is read-only."
			      " No changes to layout permitted."));
	fl_show_object(dialog_->text_warning);
    } else {
	combo_doc_class->activate();
	combo_language->activate();
	fl_hide_object(dialog_->text_warning);
    }	
}


void FormDocument::checkMarginValues()
{
	bool const not_empty =
		strlen(fl_get_input(paper_->input_top_margin)) ||
		strlen(fl_get_input(paper_->input_bottom_margin)) ||
		strlen(fl_get_input(paper_->input_left_margin)) ||
		strlen(fl_get_input(paper_->input_right_margin)) ||
		strlen(fl_get_input(paper_->input_head_height)) ||
		strlen(fl_get_input(paper_->input_head_sep)) ||
		strlen(fl_get_input(paper_->input_foot_skip)) ||
		strlen(fl_get_input(paper_->input_custom_width)) ||
		strlen(fl_get_input(paper_->input_custom_height));
	if (not_empty)
		fl_set_button(paper_->push_use_geometry, 1);
}


bool FormDocument::CheckDocumentInput(FL_OBJECT * ob, long)
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
    else  if (fl_get_choice(class_->choice_doc_spacing) != 4)
	fl_set_input (class_->input_doc_spacing, "");
    return ok;
}


void FormDocument::ChoiceBulletSize(FL_OBJECT * ob, long /*data*/ )
{
    BufferParams & param = lv_->buffer()->params;

    // convert from 1-6 range to -1-4 
    param.temp_bullets[current_bullet_depth].setSize(fl_get_choice(ob) - 2);
    fl_set_input(bullets_->input_bullet_latex,
		 param.temp_bullets[current_bullet_depth].getText().c_str());
}


void FormDocument::InputBulletLaTeX(FL_OBJECT *, long)
{
    BufferParams & param = lv_->buffer()->params;

    param.temp_bullets[current_bullet_depth].
	setText(fl_get_input(bullets_->input_bullet_latex));
}


void FormDocument::BulletDepth(FL_OBJECT * ob, State cb)
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


void FormDocument::BulletPanel(FL_OBJECT * /*ob*/, State cb)
{
    /* Here we have to change the background pixmap to that selected */
    /* by the user. (eg. standard.xpm, psnfss1.xpm etc...)           */
    
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


void FormDocument::BulletBMTable(FL_OBJECT * ob, long /*data*/ )
{
    /* handle the user input by setting the current bullet depth's pixmap */
    /* to that extracted from the current chosen position of the BMTable  */
    /* Don't forget to free the button's old pixmap first.                */

    BufferParams & param = lv_->buffer()->params;
    int bmtable_button = fl_get_bmtable(ob);

    /* try to keep the button held down till another is pushed */
    /*  fl_set_bmtable(ob, 1, bmtable_button); */
    param.temp_bullets[current_bullet_depth].setFont(current_bullet_panel);
    param.temp_bullets[current_bullet_depth].setCharacter(bmtable_button);
    fl_set_input(bullets_->input_bullet_latex,
		 param.temp_bullets[current_bullet_depth].getText().c_str());
}


void FormDocument::CheckChoiceClass(FL_OBJECT * ob, long)
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


void FormDocument::UpdateLayoutDocument(BufferParams const & params)
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
