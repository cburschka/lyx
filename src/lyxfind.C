#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "lyxtext.h"
#include "lyxfind.h"
#include "LyXView.h"
#include "lyx_gui_misc.h"
#include "support/textutils.h"
#include "support/lstrings.h"
#include "BufferView.h"
#include "buffer.h"
#include "gettext.h"

int LyXReplace(BufferView * bv,
	       string const & searchstr,
	       string const & replacestr,
	       bool const & casesens,
	       bool const & matchwrd,
	       bool const & forward,
	       bool const & replaceall)
{
   int replace_count = 0;
 
   if (!bv->available() || bv->buffer()->isReadonly()) 
     return replace_count;
   
   // CutSelection cannot cut a single space, so we have to stop
   // in order to avoid endless loop :-(
   if (searchstr.length() == 0
       || (searchstr.length() == 1 && searchstr[0] == ' ')) {
      WriteAlert(_("Sorry!"), _("You cannot replace a single space, "
				"nor an empty character."));
      return replace_count;
   }
   // now we can start searching for the first 
   // start at top if replaceall
   bool fw = forward;
   if (replaceall) {
      bv->text->clearSelection(bv);
      bv->text->cursorTop(bv);
      // override search direction because we search top to bottom
      fw = true;
   }

   // if nothing selected or selection does not equal search string
   // search and select next occurance and return if no replaceall
   if (searchstr!=bv->text->selectionAsString(bv->buffer())) {
      LyXFind(bv, searchstr, casesens, matchwrd, fw);
      if (!replaceall)
	return replace_count;
   }
   
   bool found;
   do {
      bv->hideCursor();
      bv->update(bv->text, BufferView::SELECT|BufferView::FITCUR);
      bv->toggleSelection(false);
      bv->text->replaceSelectionWithString(bv, replacestr);
      bv->text->setSelectionOverString(bv, replacestr);
      bv->update(bv->text, BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
      ++replace_count;
      found = LyXFind(bv, searchstr, casesens, matchwrd, fw);
   } while (replaceall && found);
   
   if (bv->focus())
     bv->showCursor();

   return replace_count;
}

bool LyXFind(BufferView * bv,
	     string const & searchstr,
	     bool const & casesens,
	     bool const & matchwrd,
	     bool const & forward)
{
   bool found = false;
   
   if (!bv->available() || searchstr.empty())
     return found;

   bv->hideCursor();
   bv->update(bv->text, BufferView::SELECT|BufferView::FITCUR);
   
   LyXText * ltCur = bv->text;
   if (ltCur->selection.set())
     ltCur->cursor = forward ? ltCur->selection.end
	     : ltCur->selection.start;

   if (forward 
       ? SearchForward(bv, searchstr, casesens, matchwrd)
       : SearchBackward(bv, searchstr, casesens, matchwrd)) {
      bv->update(bv->text, BufferView::SELECT|BufferView::FITCUR);
      bv->toggleSelection();
      bv->text->clearSelection(bv);
      bv->text->setSelectionOverString(bv, searchstr);
      bv->toggleSelection(false);
      found = true;
   };
   
   if (bv->focus())
     bv->showCursor();
   
   return found;
}


// returns true if the specified string is at the specified position
bool IsStringInText(Paragraph * par, Paragraph::size_type pos,
		    string const & str, bool const & cs,
		    bool const & mw)
{
	if (!par)
		return false;
   
	string::size_type size = str.length();
	Paragraph::size_type i = 0;
	while (((pos + i) < par->size())
	       && (string::size_type(i) < size)
	       && (cs ? (str[i] == par->getChar(pos + i))
		   : (toupper(str[i]) == toupper(par->getChar(pos + i)))))
	{
		++i;
	}
	if (size == string::size_type(i)) {
	  // if necessary, check whether string matches word
	  if (!mw || 
	      (mw && ((pos <= 0 || !IsLetterCharOrDigit(par->getChar(pos - 1)))
		      && (pos + Paragraph::size_type(size) >= par->size()
			  || !IsLetterCharOrDigit(par->getChar(pos + size))))
	       ))
	    return true;
	}
	return false;
}

// forward search:
// if the string can be found: return true and set the cursor to
// the new position, cs = casesensitive, mw = matchword
bool SearchForward(BufferView * bv, string const & str,
		   bool const & cs, bool const & mw)
{
	Paragraph * par = bv->text->cursor.par();
	Paragraph::size_type pos = bv->text->cursor.pos();
   
	while (par && !IsStringInText(par, pos, str, cs, mw)) {
		if (pos < par->size() - 1)
			++pos;
		else {
			pos = 0;
			par = par->next();
		}
	}
	if (par) {
		bv->text->setCursor(bv, par, pos);
		return true;
	}
	else
		return false;
}


// backward search:
// if the string can be found: return true and set the cursor to
// the new position, cs = casesensitive, mw = matchword
bool SearchBackward(BufferView * bv, string const & str,
		    bool const & cs, bool const & mw)
{
	Paragraph * par = bv->text->cursor.par();
	Paragraph::size_type pos = bv->text->cursor.pos();

	do {
		if (pos > 0)
			--pos;
		else {
			// We skip empty paragraphs (Asger)
			do {
				par = par->previous();
				if (par)
					pos = par->size() - 1;
			} while (par && pos < 0);
		}
	} while (par && !IsStringInText(par, pos, str, cs, mw));
  
	if (par) {
		bv->text->setCursor(bv, par, pos);
		return true;
	} else
		return false;
}

