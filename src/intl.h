// -*- C++ -*-
/* International support for LyX


 */
#ifndef _INTL_H
#define _INTL_H

#ifdef __GNUG__
#pragma interface
#endif

#include "form1.h"

class LyXText;
class Combox;
class TransManager;
class LString;

/// default character set
#define DEFCHSET "iso8859-1"


/** The gui part and the non gui part should be split into two different
  classes. Probably should the gui class just have a pointer to the non
  gui class.
  */
class Intl
{
public:
	///
	Intl();
	///
	~Intl();
	
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
	int SetPrimary(LString const &);

	///
	int SetSecondary(LString const &);

	// insert correct stuff into paragraph
	//void TranslateAndInsert(char c, LyXText *text);

	/// initialize key mapper
	void InitKeyMapper(bool on);

	/// Get the Translation Manager
	inline TransManager *getTrans();
	///
	bool keymapon;
	///
	char *chsetcode;
private:
	///
	//int SelectCharset(char const *code);
	///
	void update();
	///
	static void LCombo(int i, void *); // callback
	///
	static void LCombo2(int i, void *); // callback
	///
	void Keymap(long code);
	///
	static void DispatchCallback(FL_OBJECT*,long);
	///
	bool primarykeymap;
	///
	int curkeymap;
	///
	int otherkeymap;
	
	///
	FD_KeyMap *fd_form_keymap;
	///
	Combox *Language;
	///
	Combox *Language2;
	///
	LString& prim_lang;
	///
	LString& sec_lang;
	///
	TransManager *trans;
};


TransManager* Intl::getTrans()
{
	return trans;
}

#endif
