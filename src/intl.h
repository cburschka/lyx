// -*- C++ -*-
/* International support for LyX


 */
#ifndef INTL_H
#define INTL_H

#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "form1.h"

class LyXText;
class Combox;
class TransManager;


/// default character set
#define DEFCHSET "iso8859-1"


/** The gui part and the non gui part should be split into two different
  classes. Probably should the gui class just have a pointer to the non
  gui class.
  */
class Intl {
public:
	///
	Intl();
	
	/// show key mapping dialog
	void MenuKeymap(); 
	///
	void KeyMapOn(bool on);
	///
	void KeyMapPrim();
	///
	void KeyMapSec();

	/// turn on/off key mappings, status in keymapon
	void ToggleKeyMap();

	///
	int SetPrimary(string const &);

	///
	int SetSecondary(string const &);

	/// initialize key mapper
	void InitKeyMapper(bool on);

	/// Get the Translation Manager
	TransManager * getTrans();
	///
	bool keymapon;
	///
	bool primarykeymap;
	///
	char * chsetcode;
	///
	static void DispatchCallback(FL_OBJECT *, long);
private:
	///
	void update();
	///
	static void LCombo(int i, void *, Combox *); // callback
	///
	void Keymap(long code);
	///
	int curkeymap;
	///
	int otherkeymap;
	
	///
	FD_KeyMap * fd_form_keymap;
	///
	Combox * Language;
	///
	Combox * Language2;
	///
	string & prim_lang;
	///
	string & sec_lang;
	///
	TransManager * trans;
};


inline
TransManager * Intl::getTrans()
{
	return trans;
}

#endif
