// -*- C++ -*-
#ifndef LYXFR1_H
#define LYXFR1_H

#ifdef __GNUG__
#pragma interface
#endif

#include FORMS_H_LOCATION
class 	LyXText;
class	LyXParagraph;

/**
  LyXFindReplace1"

  This class implements Find & Replace in LyXText texts. It is based on
  LyXFindReplace0, which implements the form related stuff. (see lyxfr0.h)
  */
class LyXFindReplace1 : public LyXFindReplace0 {
public:
	///
	LyXFindReplace1() : LyXFindReplace0() {};
	///
	~LyXFindReplace1() {};	

	/// Initialize internal variables and search form
	virtual void StartSearch();

	///
	virtual bool SearchCB(bool fForward);
	///
	virtual void SearchReplaceCB();
	///
	virtual void SearchReplaceAllCB();
protected:
	/// Set by IsSearchStringInText:
	int	iLenSelected;

	/// Direction we are searching:
	bool searchForward;
    
	///
	bool SearchForward(LyXText *lt);
	///
	bool SearchBackward(LyXText *lt); 

	/** Compares 2 char values. 
	return value is
	    \begin{itemize}
	    \item > 0 if ch1 > ch2
	    \item = 0 if ch1 == ch2
	    \item < 0 if ch1 < ch2
	    \end{itemize}
	*/
	int CompareChars(char chSearch, char chText);

	/// returns true if the specified string is at the specified position 
	bool IsSearchStringInText(LyXParagraph *par, int pos);
};
#endif
