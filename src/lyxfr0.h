// -*- C++ -*-
#ifndef LYXFR0_H
#define LYXFR0_H

#ifdef __GNUG__
#pragma interface
#endif

#include FORMS_H_LOCATION


/**
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
class LyXFindReplace0 {
public:
	///
	LyXFindReplace0();
	///
	virtual ~LyXFindReplace0() {};	
	
	/// Initialize internal variables and dialog
	virtual void StartSearch();

	// Callbacks from form
	///
	virtual void SearchCancelCB();
	///
	virtual bool SearchCB(bool fForward) = 0;
	///
	virtual void SearchReplaceCB() = 0;
	///
	virtual void SearchReplaceAllCB() = 0;
protected:
	///
	bool		fCaseSensitive;
	/// Match complete word only.
	bool		fMatchWord;
	/// replace buttons enabled?
	bool		fReplaceEnabled;
	/// search string
	string		lsSearch;

	///
	void SetReplaceEnabled(bool fEnabled);
	///
	bool ReplaceEnabled() { return fReplaceEnabled; }

	/// Initialize search variables from Find & Replace form
	virtual void ReInitFromForm();

	// Return values
	///
	bool CaseSensitive() { return fCaseSensitive; }
	///
	bool MatchWord() { return fMatchWord; }
	///
	string const SearchString() { return lsSearch; }
	///
	void SetSearchString(string const &ls);
	///
	string const ReplaceString();

	///
	bool ValidSearchData() { return !lsSearch.empty(); }	 	
};

#endif
