/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#include "CutAndPaste.h"
#include "BufferView.h"
#include "buffer.h"
#include "lyxparagraph.h"
#include "insets/inseterror.h"
#include "lyx_gui_misc.h"
#include "lyxcursor.h"

#ifdef __GNUG__
#pragma implementation
#endif

using std::pair;

extern BufferView * current_view;

// Jürgen, note that this means that you cannot currently have a list
// of selections cut/copied. So IMHO later we should have a
// list/vector/deque that we could store
// struct selection_item {
//       LyXParagraph * buf;
//       LyXTextClassList::size_type textclass;
// };
// in and some method of choosing beween them (based on the first few chars
// in the selection probably.) This would be a nice feature and quite
// easy to implement. (Lgb)
//
// Sure but I just cleaned up this code for now with the same functionality
// as before. I also want to add a XClipboard function so that we can copy
// text from LyX to some other X-application in the form of ASCII or in the
// form of LaTeX (or Docbook depending on the document-class!). Think how nice
// it could be to select a math-inset do a "Copy to X-Clipboard as LaTeX" and
// then do a middle mouse button click in the application you want and have
// the whole formula there in LaTeX-Code. (Jug)

static LyXParagraph * buf = 0;
static LyXTextClassList::size_type textclass = 0;

// for now here this should be in another Cut&Paste Class!
// Jürgen, I moved this out of CutAndPaste since it does not operate on any
// member of the CutAndPaste class and in addition it was private.
// Perhaps it even should take a parameter? (Lgb)
static
void DeleteBuffer()
{
    if (!buf)
	return;
	
    LyXParagraph * tmppar;
	
    while (buf) {
	tmppar =  buf;
#ifndef NEW_INSETS
	buf = buf->next_;
#else
	buf = buf->next();
#endif
	delete tmppar;
    }
    buf = 0;
}


bool CutAndPaste::cutSelection(LyXParagraph * startpar, LyXParagraph ** endpar,
			       int start, int & end, char tc, bool doclear)
{
#ifndef NEW_INSETS
    if (!startpar || (start > startpar->Last()))
	return false;
#else
    if (!startpar || (start > startpar->size()))
	return false;
#endif

    DeleteBuffer();

    textclass = tc;

    if (!(*endpar) ||
#ifndef NEW_INSETS
	(startpar->ParFromPos(start) ==
	 (*endpar)->ParFromPos(end))
#else
	(startpar == (*endpar))
#endif
	   ) {
	// only within one paragraph
	buf = new LyXParagraph;
	LyXParagraph::size_type i = start;
#ifndef NEW_INSETS
	if (end > startpar->Last())
	    end = startpar->Last();
#else
	if (end > startpar->size())
	    end = startpar->size();
#endif
	for (; i < end; ++i) {
	    startpar->CopyIntoMinibuffer(*current_view->buffer(), start);
	    startpar->Erase(start);

#ifndef NEW_INSETS
	    buf->InsertFromMinibuffer(buf->Last());
#else
	    buf->InsertFromMinibuffer(buf->size());
#endif
	}
	end = start-1;
    } else {
	// more than one paragraph
	(*endpar)->BreakParagraphConservative(current_view->buffer()->params,
					      end);
	*endpar = (*endpar)->next();
	end = 0;
   
	startpar->BreakParagraphConservative(current_view->buffer()->params,
					     start);

	// store the selection
#ifndef NEW_INSETS
	buf = startpar->ParFromPos(start)->next_;
#else
	buf = startpar->next();
#endif
	buf->previous(0);
	(*endpar)->previous()->next(0);

	// cut the selection
#ifndef NEW_INSETS
	startpar->ParFromPos(start)->next(*endpar);
	
	(*endpar)->previous(startpar->ParFromPos(start));
#else
	startpar->next(*endpar);
	
	(*endpar)->previous(startpar);
#endif

#ifndef NEW_INSETS
	// care about footnotes
	if (buf->footnoteflag) {
	    LyXParagraph * tmppar = buf;
	    while (tmppar){
		tmppar->footnoteflag = LyXParagraph::NO_FOOTNOTE;
		tmppar = tmppar->next_;
	    }
	}
#endif

	// the cut selection should begin with standard layout
	buf->Clear(); 
   
	// paste the paragraphs again, if possible
	if (doclear)
	    startpar->next()->StripLeadingSpaces(textclass);
#ifndef NEW_INSETS
	if (startpar->FirstPhysicalPar()->HasSameLayout(startpar->next()) ||
	    !startpar->next()->Last()) {
#else
	if (startpar->HasSameLayout(startpar->next()) ||
	    !startpar->next()->size()) {
#endif
#ifndef NEW_INSETS
	    startpar->ParFromPos(start)->PasteParagraph(current_view->buffer()->params);
#else
	    startpar->PasteParagraph(current_view->buffer()->params);
#endif
	    (*endpar) = startpar; // this because endpar gets deleted here!
	}
    }
    return true;
}


bool CutAndPaste::copySelection(LyXParagraph * startpar, LyXParagraph * endpar,
				int start, int end, char tc)
{
#ifndef NEW_INSETS
    if (!startpar || (start > startpar->Last()))
	return false;
#else
    if (!startpar || (start > startpar->size()))
	return false;
#endif

    DeleteBuffer();

    textclass = tc;

    if (!(endpar) ||
#ifndef NEW_INSETS
	(startpar->ParFromPos(start) ==
		       (endpar)->ParFromPos(end))
#else
	(startpar == endpar)
#endif
	   ) {
	// only within one paragraph
	buf = new LyXParagraph;
	LyXParagraph::size_type i = start;
#ifndef NEW_INSETS
	if (end > startpar->Last())
	    end = startpar->Last();
#else
	if (end > startpar->size())
	    end = startpar->size();
#endif
	for (; i < end; ++i) {
	    startpar->CopyIntoMinibuffer(*current_view->buffer(), i);
#ifndef NEW_INSETS
	    buf->InsertFromMinibuffer(buf->Last());
#else
	    buf->InsertFromMinibuffer(buf->size());
#endif
	}
    } else {
	// copy more than one paragraph
	// clone the paragraphs within the selection
#ifndef NEW_INSETS
	LyXParagraph * tmppar = startpar->ParFromPos(start);
#else
	LyXParagraph * tmppar = startpar;
#endif
	buf = tmppar->Clone();
	LyXParagraph * tmppar2 = buf;
     
#ifndef NEW_INSETS
	while (tmppar != endpar->ParFromPos(end)
	       && tmppar->next_) {
		tmppar = tmppar->next_;
		tmppar2->next(tmppar->Clone());
		tmppar2->next_->previous(tmppar2);
		tmppar2 = tmppar2->next_;
	}
	tmppar2->next(0);
#else
	while (tmppar != endpar
	       && tmppar->next()) {
		tmppar = tmppar->next();
		tmppar2->next(tmppar->Clone());
		tmppar2->next()->previous(tmppar2);
		tmppar2 = tmppar2->next();
	}
	tmppar2->next(0);
#endif

#ifndef NEW_INSETS
	// care about footnotes
	if (buf->footnoteflag) {
	    tmppar = buf;
	    while (tmppar) {
		tmppar->footnoteflag = LyXParagraph::NO_FOOTNOTE;
		tmppar = tmppar->next_;
	    }
	}
#endif
	
	// the buf paragraph is too big
#ifndef NEW_INSETS
	LyXParagraph::size_type tmpi2 = startpar->PositionInParFromPos(start);
#else
	LyXParagraph::size_type tmpi2 = start;
#endif
	for (; tmpi2; --tmpi2)
	    buf->Erase(0);
	
	// now tmppar 2 is too big, delete all after end
#ifndef NEW_INSETS
	tmpi2 = endpar->PositionInParFromPos(end);
#else
	tmpi2 = end;
#endif
	while (tmppar2->size() > tmpi2) {
	    tmppar2->Erase(tmppar2->size() - 1);
	}
    }
    return true;
}


bool CutAndPaste::pasteSelection(LyXParagraph ** par, LyXParagraph ** endpar,
				 int & pos, char tc)
{
    if (!checkPastePossible(*par, pos))
	return false;

#ifndef NEW_INSETS
    if (pos > (*par)->Last())
	pos = (*par)->Last();
#else
    if (pos > (*par)->size())
	pos = (*par)->size();
#endif
    
    LyXParagraph * tmpbuf;
    LyXParagraph * tmppar = *par;
    int tmppos = pos;

    // There are two cases: cutbuffer only one paragraph or many
#ifndef NEW_INSETS
    if (!buf->next_) {
#else
    if (!buf->next()) {
#endif
	// only within a paragraph
	tmpbuf = buf->Clone();
	// Some provisions should be done here for checking
	// if we are inserting at the beginning of a
	// paragraph. If there are a space at the beginning
	// of the text to insert and we are inserting at
	// the beginning of the paragraph the space should
	// be removed.
	while (buf->size()) {
		// This is an attempt to fix the
		// "never insert a space at the
		// beginning of a paragraph" problem.
		if (!tmppos && buf->IsLineSeparator(0)) {
			buf->Erase(0);
		} else {
			buf->CutIntoMinibuffer(current_view->buffer()->params, 0);
			buf->Erase(0);
			if (tmppar->InsertFromMinibuffer(tmppos))
				++tmppos;
		}
	}
	delete buf;
	buf = tmpbuf;
	*endpar = tmppar->next();
	pos = tmppos;
    } else {
	// many paragraphs

	// make a copy of the simple cut_buffer
	tmpbuf = buf;
	LyXParagraph * simple_cut_clone = tmpbuf->Clone();
	LyXParagraph * tmpbuf2 = simple_cut_clone;
#ifndef NEW_INSETS
	if ((*par)->footnoteflag) {
		tmpbuf->footnoteflag = (*par)->footnoteflag;
		tmpbuf->footnotekind = (*par)->footnotekind;
	}
	while (tmpbuf->next_) {
		tmpbuf = tmpbuf->next_;
		tmpbuf2->next(tmpbuf->Clone());
		tmpbuf2->next_->previous(tmpbuf2);
		tmpbuf2 = tmpbuf2->next_;
		if ((*par)->footnoteflag){
			tmpbuf->footnoteflag = (*par)->footnoteflag;
			tmpbuf->footnotekind = (*par)->footnotekind;
		}
	}
#else
	while (tmpbuf->next()) {
		tmpbuf = tmpbuf->next();
		tmpbuf2->next(tmpbuf->Clone());
		tmpbuf2->next()->previous(tmpbuf2);
		tmpbuf2 = tmpbuf2->next();
	}
#endif
	
	// make sure there is no class difference
	SwitchLayoutsBetweenClasses(textclass, tc, buf);
	
	// make the buf exactly the same layout than
	// the cursor paragraph
	buf->MakeSameLayout(*par);
	
	// find the end of the buffer
	LyXParagraph * lastbuffer = buf;
	while (lastbuffer->next())
	    lastbuffer = lastbuffer->next();
	
	bool paste_the_end = false;
	
#ifndef NEW_INSETS
	// open the paragraph for inserting the buf
	// if necessary
	if (((*par)->Last() > pos) || !(*par)->next()) {
	    (*par)->BreakParagraphConservative(current_view->buffer()->params,
					       pos);
	    paste_the_end = true;
	}
	// set the end for redoing later
	*endpar = (*par)->ParFromPos(pos)->next_->next();
	
	// paste it!
	lastbuffer->ParFromPos(lastbuffer->Last())->next(
	    (*par)->ParFromPos(pos)->next_);
	(*par)->ParFromPos(pos)->next()->previous(
	    lastbuffer->ParFromPos(lastbuffer->Last()));
	
	(*par)->ParFromPos(pos)->next(buf);
	buf->previous((*par)->ParFromPos(pos));
	
	if ((*par)->ParFromPos(pos)->next() == lastbuffer)
	    lastbuffer = *par;
	
	(*par)->ParFromPos(pos)->PasteParagraph(current_view->buffer()->params);
	// store the new cursor position
	*par = lastbuffer;
	pos = lastbuffer->Last();
	// maybe some pasting
	if (lastbuffer->next() && paste_the_end) {
	    if (lastbuffer->next()->HasSameLayout(lastbuffer)) {
		lastbuffer->ParFromPos(lastbuffer->Last())->PasteParagraph(current_view->buffer()->params);
	    } else if (!lastbuffer->next()->Last()) {
		lastbuffer->next()->MakeSameLayout(lastbuffer);
		lastbuffer->ParFromPos(lastbuffer->Last())->PasteParagraph(current_view->buffer()->params);
	    } else if (!lastbuffer->Last()) {
		lastbuffer->MakeSameLayout(lastbuffer->next_);
		lastbuffer->ParFromPos(lastbuffer->Last())->PasteParagraph(current_view->buffer()->params);
	    } else
		lastbuffer->next()->StripLeadingSpaces(tc);
	}
	// restore the simple cut buffer
	buf = simple_cut_clone;
    }
#else
	// open the paragraph for inserting the buf
	// if necessary
	if (((*par)->size() > pos) || !(*par)->next()) {
	    (*par)->BreakParagraphConservative(current_view->buffer()->params,
					       pos);
	    paste_the_end = true;
	}
	// set the end for redoing later
	*endpar = (*par)->next()->next();
	
	// paste it!
	lastbuffer->next((*par)->next());
	(*par)->next()->previous(lastbuffer);
	
	(*par)->next(buf);
	buf->previous(*par);
	
	if ((*par)->next() == lastbuffer)
	    lastbuffer = *par;
	
	(*par)->PasteParagraph(current_view->buffer()->params);
	// store the new cursor position
	*par = lastbuffer;
	pos = lastbuffer->size();
	// maybe some pasting
	if (lastbuffer->next() && paste_the_end) {
	    if (lastbuffer->next()->HasSameLayout(lastbuffer)) {
		lastbuffer->PasteParagraph(current_view->buffer()->params);
	    } else if (!lastbuffer->next()->size()) {
		lastbuffer->next()->MakeSameLayout(lastbuffer);
		lastbuffer->PasteParagraph(current_view->buffer()->params);
	    } else if (!lastbuffer->size()) {
		lastbuffer->MakeSameLayout(lastbuffer->next());
		lastbuffer->PasteParagraph(current_view->buffer()->params);
	    } else
		lastbuffer->next()->StripLeadingSpaces(tc);
	}
	// restore the simple cut buffer
	buf = simple_cut_clone;
    }
#endif

    return true;
}


int CutAndPaste::nrOfParagraphs()
{
	if (!buf) return 0;

	int n = 1;
	LyXParagraph * tmppar = buf;
#ifndef NEW_INSETS
	while(tmppar->next_) {
		++n;
		tmppar = tmppar->next_;
	}
#else
	while(tmppar->next()) {
		++n;
		tmppar = tmppar->next();
	}
#endif
	return n;
}


int CutAndPaste::SwitchLayoutsBetweenClasses(LyXTextClassList::size_type c1,
					     LyXTextClassList::size_type c2,
					     LyXParagraph * par)
{
    int ret = 0;
    if (!par || c1 == c2)
	return ret;
#ifndef NEW_INSETS
    par = par->FirstPhysicalPar();
#endif
    while (par) {
	string name = textclasslist.NameOfLayout(c1, par->layout);
	int lay = 0;
	pair<bool, LyXTextClass::LayoutList::size_type> pp =
	    textclasslist.NumberOfLayout(c2, name);
	if (pp.first) {
	    lay = pp.second;
	} else { // layout not found
	    // use default layout "Standard" (0)
	    lay = 0;
	}
	par->layout = lay;
	
	if (name != textclasslist.NameOfLayout(c2, par->layout)) {
	    ++ret;
	    string s = _("Layout had to be changed from\n")
		    + name + _(" to ")
		    + textclasslist.NameOfLayout(c2, par->layout)
		    + _("\nbecause of class conversion from\n")
		    + textclasslist.NameOfClass(c1) + _(" to ")
		    + textclasslist.NameOfClass(c2);
	    InsetError * new_inset = new InsetError(s);
	    par->InsertInset(0, new_inset);
	}
#ifndef NEW_INSETS
	par = par->next_;
#else
	par = par->next();
#endif
    }
    return ret;
}


bool CutAndPaste::checkPastePossible(LyXParagraph * par, int)
{
    if (!buf) return false;

#ifndef NEW_INSETS
    // be carefull with footnotes in footnotes
    if (par->footnoteflag != LyXParagraph::NO_FOOTNOTE) {
	// check whether the cut_buffer includes a footnote
	LyXParagraph * tmppar = buf;
	while (tmppar && tmppar->footnoteflag == LyXParagraph::NO_FOOTNOTE)
	    tmppar = tmppar->next_;
      
	if (tmppar) {
	    WriteAlert(_("Impossible operation"),
		       _("Can't paste float into float!"),
		       _("Sorry."));
	    return false;
	}
    }
#endif
    return true;
}
