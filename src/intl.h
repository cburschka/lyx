// -*- C++ -*-
/* International support for LyX


 */
#ifndef INTL_H
#define INTL_H

#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma interface
#endif

#include <sigc++/signal_system.h>
#include "LString.h"
#include "form1.h"

#if 1
#include "trans_mgr.h"
#endif

class LyXText;
class Combox;

#if 0
class TransManager;
#endif

#ifdef SIGC_CXX_NAMESPACES
using SigC::Object;
using SigC::Connection;
#endif

/// default character set
#define DEFCHSET "iso8859-1"


/** The gui part and the non gui part should be split into two different
  classes. Probably should the gui class just have a pointer to the non
  gui class.
  */
class Intl : public Object {
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
	int SetPrimary(string const &);

	///
	int SetSecondary(string const &);

	/// initialize key mapper
	void InitKeyMapper(bool on);

#if 0
	/// Get the Translation Manager
	TransManager * getTrans();
#else
	// Get the Translation Manager
	TransManager & getTrans();
#endif
	///
	bool keymapon;
	///
	bool primarykeymap;
	///
	char * chsetcode;
	///
	static void DispatchCallback(FL_OBJECT *, long);
private:
	/** Redraw the form (on receipt of a Signal indicating, for example,
	    that the xform colors have been re-mapped).
	*/
	void redraw();
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
#if 0
	///
	TransManager * trans;
#else
	///
	TransManager trans;
#endif
	/// Redraw connection.
	Connection r_;
};


#if 0
inline
TransManager * Intl::getTrans()
{
	return trans;
}
#else
inline
TransManager & Intl::getTrans()
{
	return trans;
}
#endif

#endif
