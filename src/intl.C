/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

/*
 *	International support for LyX
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif
 
#include FORMS_H_LOCATION
#include "intl.h"
#include "form1.h"
#include "tex-strings.h"
#include "combox.h"
#include "lyx_gui_misc.h" // CancelCloseBoxCB
#include "debug.h"
#include "lyxrc.h"
#include "trans_mgr.h"
#include "support/lstrings.h"
#include "language.h"
#include "frontends/Dialogs.h" // redrawGUI

#ifdef SIGC_CXX_NAMESPACES
using SigC::slot;
#endif

using std::endl;

// a wrapper around the callback static member.
extern "C" void C_Intl_DispatchCallback(FL_OBJECT * ob, long code);


Intl::Intl()
	: prim_lang(lyxrc.primary_kbmap), 
	sec_lang(lyxrc.secondary_kbmap),
	trans(new TransManager)
{
	keymapon = lyxrc.use_kbmap;
	chsetcode = 0;
	primarykeymap = false;
	curkeymap = 0;
	otherkeymap = 0;
	r_ = Dialogs::redrawGUI.connect(slot(this, &Intl::redraw));
}

Intl::~Intl()
{
	r_.disconnect();
	delete trans;
}


void Intl::redraw()
{
	if (fd_form_keymap && fd_form_keymap->KeyMap->visible)
		fl_redraw_form(fd_form_keymap->KeyMap);
}


int Intl::SetPrimary(string const & lang)
{
	if (lyxerr.debugging(Debug::KBMAP))
		lyxerr << "Primary: `" << lang << "'" << endl;
	return trans->SetPrimary(lang);
}


int Intl::SetSecondary(string const & lang)
{
	if (lyxerr.debugging(Debug::KBMAP))
		lyxerr << "Secondary: `" << lang << "'" << endl;
	return trans->SetSecondary(lang);
}


void Intl::update()
{
	int off = 0;
	int prim = 0;
	int sec = 0;
	
	if (!keymapon) {
		off = 1;
	} else {
		if (primarykeymap) {
			prim = 1;
		} else {
			sec = 1;
		}
	}
	
	fl_set_button(fd_form_keymap->KeyOffBtn, off);
	fl_set_button(fd_form_keymap->KeyOnBtn, prim);
	fl_set_button(fd_form_keymap->KeyOnBtn2, sec);
}


void Intl::KeyMapOn(bool on)
	/* turn on/off key mappings, status in keymapon */
{
	keymapon = on;

	if (!fd_form_keymap) return;
	
	fl_set_button(fd_form_keymap->KeyOffBtn, 0);
	fl_set_button(fd_form_keymap->KeyOnBtn, 0);
	fl_set_button(fd_form_keymap->KeyOnBtn2, 0);

	if (on) {
		if (primarykeymap) {
			KeyMapPrim();
		} else {
			KeyMapSec();
		}
	} else {
		fl_set_button(fd_form_keymap->KeyOffBtn, 1);
		fl_hide_object(fd_form_keymap->KeymapErr);
		trans->DisableKeymap();
	}
}


void Intl::ToggleKeyMap()
{
	if (keymapon && primarykeymap) {
		KeyMapSec();
	} else if (keymapon) {
		KeyMapOn(false);
	} else	
		KeyMapPrim();
}


void Intl::KeyMapPrim()
{
	fl_set_button(fd_form_keymap->KeyOffBtn, 0);
	fl_set_button(fd_form_keymap->KeyOnBtn, 1);
	fl_set_button(fd_form_keymap->KeyOnBtn2, 0);

	/* read text from choice */
	int const i = Language->get();

	string p;
	if (i == otherkeymap)
		p = fl_get_input(fd_form_keymap->OtherKeymap);
	else
		p = Language->getline();

	curkeymap = i;

	if (p.empty() || trans->SetPrimary(p)) {
		// error selecting keymap
		fl_show_object(fd_form_keymap->KeymapErr);
		update();
	} else {
		// no error
		trans->EnablePrimary();
		keymapon = true;
		primarykeymap = true;
		fl_hide_object(fd_form_keymap->KeymapErr);
	}
}


void Intl::KeyMapSec()
{
	fl_set_button(fd_form_keymap->KeyOffBtn, 0);
	fl_set_button(fd_form_keymap->KeyOnBtn, 0);
	fl_set_button(fd_form_keymap->KeyOnBtn2, 1);

	/* read text from choice */
	int const i = Language2->get();

	string p;
	if (i == otherkeymap)
		p = fl_get_input(fd_form_keymap->OtherKeymap2);
	else
		p = Language2->getline();
	curkeymap = i;

	if (p.empty() || trans->SetSecondary(p)) {
		// error selecting keymap
		fl_show_object(fd_form_keymap->KeymapErr);
		update();
	} else {
		// no error
		trans->EnableSecondary();
		keymapon = true;
		primarykeymap = false;
		fl_hide_object(fd_form_keymap->KeymapErr);
	}
}

void Intl::LCombo(int, void * v, Combox * combox)
{
	Intl * itl = static_cast<Intl*>(v);
	if (combox == itl->Language)
	    itl->Keymap(23);
	else if (combox == itl->Language2)
	    itl->Keymap(43);
	return;
}

void Intl::DispatchCallback(FL_OBJECT * ob, long code)
{
	if (ob && (code == 0)) {
		fl_hide_form(ob->form);
		return;
	}
	if (!ob || !(ob->u_vdata))
		return;
	
	Intl * itl = static_cast<Intl *>(ob->u_vdata);

	if (itl!= 0) itl->Keymap(code);
}


extern "C" void C_Intl_DispatchCallback(FL_OBJECT * ob, long code)
{
	Intl::DispatchCallback(ob, code);
}


void Intl::InitKeyMapper(bool on)
	/* initialize key mapper */
{
	lyxerr[Debug::INIT] << "Initializing key mappings..." << endl;

	if (prim_lang.empty() && sec_lang.empty())
		keymapon = false; 
	else
		keymapon = on;

	Language = new Combox(FL_COMBOX_DROPLIST);
	Language2 = new Combox(FL_COMBOX_DROPLIST);
	Language->setcallback(LCombo, this);
	Language2->setcallback(LCombo, this);

	fd_form_keymap = create_form_KeyMap();

	// Add the Intl* pointer
	fd_form_keymap->AcceptChset->u_vdata =
		fd_form_keymap->Charset->u_vdata =
		fd_form_keymap->Accept->u_vdata =
		fd_form_keymap->OtherKeymap->u_vdata =
		fd_form_keymap->KeyOnBtn->u_vdata =
		fd_form_keymap->KeyOffBtn->u_vdata =
		fd_form_keymap->KeyOnBtn2->u_vdata = this;

	// add the callbacks.
	fl_set_object_callback(fd_form_keymap->AcceptChset,
			       C_Intl_DispatchCallback, 27);
	fl_set_object_callback(fd_form_keymap->Charset,
			       C_Intl_DispatchCallback, 26);
	fl_set_object_callback(fd_form_keymap->Accept,
			       C_Intl_DispatchCallback, 0);

	fl_set_object_callback(fd_form_keymap->KeyOnBtn,
			       C_Intl_DispatchCallback, 23);
	fl_set_object_callback(fd_form_keymap->KeyOffBtn,
			       C_Intl_DispatchCallback, 3);
	fl_set_object_callback(fd_form_keymap->KeyOnBtn2,
			       C_Intl_DispatchCallback, 43);
	
	// Make sure pressing the close box does not kill LyX. (RvdK)
	fl_set_form_atclose(fd_form_keymap->KeyMap, CancelCloseBoxCB, 0);

	fl_hide_object(fd_form_keymap->KeymapErr);
	fl_hide_object(fd_form_keymap->ChsetErr);
	fl_set_input(fd_form_keymap->Charset, lyxrc.font_norm.c_str());

	// Adds two comboxes to the keyboard map
	fl_addto_form(fd_form_keymap->KeyMap);
	Language->add(120, 30, 160, 30, 300);	// Primary
	Language2->add(120, 110, 160, 30, 300);	// Secondary
	fl_end_form();

	int n = 0;
	// Default is not in the language map
#ifdef DO_USE_DEFAULT_LANGUAGE
	Language->addto("default");
	Language2->addto("default");
	++n;
#endif
	for (Languages::const_iterator cit = languages.begin();
	     cit != languages.end(); ++cit) {
		Language->addto((*cit).second.lang());
		Language2->addto((*cit).second.lang());
		++n;
	}

	Language->addto(_("other..."));
	Language2->addto(_("other..."));
	otherkeymap = n + 1;
	if (!Language->select_text(prim_lang)) {
		Language->select(n + 1);
		fl_set_input(fd_form_keymap->OtherKeymap, prim_lang.c_str());
	}
	else 
		trans->SetPrimary(prim_lang);

	if (!Language2->select_text(sec_lang)) {
		Language2->select(n + 1);
		fl_set_input(fd_form_keymap->OtherKeymap2, sec_lang.c_str());
	}
	else
		trans->SetSecondary(sec_lang);

	KeyMapOn(keymapon);
	if (keymapon)
		Keymap(23); // turn primary on

	trans->setCharset(lyxrc.font_norm);
}


void Intl::Keymap(long code)
{
	if (lyxerr.debugging(Debug::KBMAP))
		lyxerr << "KeyMap callback: " << code << endl;

	// Did you wonder if it is possible to write spagetti code with
	// other constructs thatn goto's? Well here we have a nice small
	// spagetti example using a switch... (Lgb)
	switch (code) {
	case 0:
		/* cancel/hide */
		fl_hide_form(fd_form_keymap->KeyMap);
		break;
	case 3:
	case 23:
	case 43:
		if (code == 3) {
			KeyMapOn(false);
			return;
		}
		code -= 19;	// change to language change type code
		
	case 4: // 4 and 24 will never be called directly, they will only be
	case 24: // called through 3, 23, 43 (lgb)
		if (code == 4) {
			KeyMapPrim();
		} else {
			KeyMapSec();
		}
		break;
	case 27:	/* set new font norm */
		char const * p = fl_get_input(fd_form_keymap->Charset);
		if (trans->setCharset(p))
			fl_show_object(fd_form_keymap->ChsetErr);
		else
			fl_hide_object(fd_form_keymap->ChsetErr);
		break;
	}
}


void Intl::MenuKeymap()
{
	if (fd_form_keymap->KeyMap->visible) {
		fl_raise_form(fd_form_keymap->KeyMap);
	} else fl_show_form(fd_form_keymap->KeyMap,
			    FL_PLACE_MOUSE | FL_FREE_SIZE, FL_TRANSIENT,
			    _("Key Mappings"));
}
