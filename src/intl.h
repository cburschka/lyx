// -*- C++ -*-
/* International support for LyX


 */
#ifndef INTL_H
#define INTL_H

#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma interface
#endif

#include <boost/smart_ptr.hpp>
#include <sigc++/signal_system.h>
#include "LString.h"
#include "form1.h"
#include "trans_mgr.h"

class LyXText;
class Combox;

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

	// Get the Translation Manager
	TransManager & getTrans();
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
	boost::scoped_ptr<FD_KeyMap> fd_form_keymap;
	///
	boost::scoped_ptr<Combox> Language;
	///
	boost::scoped_ptr<Combox> Language2;
	///
	string & prim_lang;
	///
	string & sec_lang;
	///
	TransManager trans;
	/// Redraw connection.
	Connection r_;
};


inline
TransManager & Intl::getTrans()
{
	return trans;
}

#endif
