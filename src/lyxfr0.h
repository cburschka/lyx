// -*- C++ -*-
#ifndef LYXFR0_H
#define LYXFR0_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include FORMS_H_LOCATION
#include "form1.h"
#include <sigc++/signal_system.h>

class LyXFindReplace;

#ifdef SIGC_CXX_NAMESPACES
using SigC::Object;
using SigC::Connection;
#endif

/**
   The comments below are most likely not valied anymore since
   the rewrite of LyXFindReplace0 and LyXFindReplace1. (Lgb)
   
  LyXFindReplace0
 
  This is a virtual base class. It does not handle text specific
  items. It only manages the Find & Replace form and the variables.
  
  To be added:

  - Confirm replace lightbutton (checkbox)

  - A history list for search and replace strings.
  These 2 stringlists should match, so that you can replay previous replaces.
  If you select another search string, then the first choice should be the 
  replace string you have typed before.
  
  - regex searches (I'm working on that -- dnaber, 1999-02-24)

*/
class SearchForm : public Object {
public:
	///
	SearchForm();
	///
	~SearchForm();	
	
	///
	bool CaseSensitive() const {
		return fl_get_button(search_form->btnCaseSensitive);
	}

	///
	bool MatchWord() const {
		return fl_get_button(search_form->btnMatchWord);
	}

	///
	string const SearchString() const {
		return fl_get_input(search_form->input_search);
	}

	/// Initialize internal variables and dialog
	void StartSearch(LyXFindReplace *);

	///
	void replaceEnabled(bool fEnabled);
	///
	bool replaceEnabled() const { return fReplaceEnabled; }

	// Callbacks from form
	///
	void SearchCancelCB();

	// Return values
	///
	void SetSearchString(string const & ls);
	///
	string const ReplaceString() const;

	///
	bool ValidSearchData() const { return !(SearchString().empty()); }
private:
	/** Redraw the form (on receipt of a Signal indicating, for example,
	    that the xform colors have been re-mapped).
	*/
	void redraw();
	///
	FD_form_search * search_form;
	/// replace buttons enabled?
	bool fReplaceEnabled;
	/// Redraw connection.
	Connection r_;
};

#endif
