// -*- C++ -*-
#ifndef LYXFR1_H
#define LYXFR1_H

#ifdef __GNUG__
#pragma interface
#endif

#include "lyxparagraph.h"
#include "lyxfr0.h"

class LyXText;

/**
  LyXFindReplace"

  This class implements Find & Replace in LyXText texts. It is based on
  LyXFindReplace0, which implements the form related stuff. (see lyxfr0.h)
  */
class LyXFindReplace {
public:
	///
	LyXFindReplace();
	///
	~LyXFindReplace();

	/// Initialize internal variables and search form
	void StartSearch(BufferView *);

	///
	bool SearchCB(bool fForward);
	///
	void SearchReplaceCB();
	///
	void SearchReplaceAllCB();
private:
	/// Set by IsSearchStringInText:
	mutable int iLenSelected;

	/// Direction we are searching:
	bool searchForward;
    
	///
	bool SearchForward(BufferView *);
	///
	bool SearchBackward(BufferView *); 

	/** Compares 2 char values. 
	return value is
	    \begin{itemize}
	    \item > 0 if ch1 > ch2
	    \item = 0 if ch1 == ch2
	    \item < 0 if ch1 < ch2
	    \end{itemize}
	*/
	int CompareChars(char chSearch, char chText) const;
	/// returns true if the specified string is at the specified position 
	bool IsSearchStringInText(LyXParagraph * par,
				  LyXParagraph::size_type pos) const;
	///
	SearchForm SF;
	///
	BufferView * bv;
};
#endif
