// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team
 *
 *           This file is Copyright 2000
 *           Lars Gullik Bjønnes
 *
 * ====================================================== */

#ifndef TEXT_CACHE_H
#define TEXT_CACHE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "support/LOstream.h"
#include "lyxtext.h"

class Buffer;

// This is only the very first implemetation and use of the TextCache,
// operations on it needs to be put into a class or a namespace, that part
// is _NOT_ finished so don't bother to come with too many comments on it
// (unless you have some nice ideas on where/how to do it)
//
// I think we need a global TextCache that is common for all BufferViews,
// also the BufferList needs access to the TextCache. Please tell if you
// don't agree.
//
// Q. What are we caching?
// A. We are caching the screen representations (LyXText) of the
//    documents (Buffer,LyXParagraph) for specific BufferView widths.
// Q. Why the cache?
// A. It is not really needed, but it speeds things up a lot
//    when you have more than one document loaded at once since a total
//    rebreak (reformatting) need not be done when switching between
//    documents. When the cache is in function a document only needs to be
//    formatted upon loading and when the with of the BufferView changes.
//    Later it will also be unneccessary to reformat when having two
//    BufferViews of equal width with the same document, a simple copy
//    of the LyXText structure will do.
// Invariant for the TextCache:
//        - The buffer of the text  in the TextCache _must_ exists
//          in the bufferlist.
//        - For a text in the TextCache there _must not_ be an equivalent
//          text in any BufferView. (same buffer and width).
// Among others this mean:
//        - When a document is closed all trace of it must be removed from
//          the TextCache.
// Scenarios:
//    I believe there are only three possible scenarios where the two first
//    are also covered by the third.
//        - The simplest scenario is what we have now, a single
//          BufferView only.  
//          o Opening
//            Nothing to do with the TextCache is done when opening a file.
//          o Switching
//            We switch from buffer A to buffer B.
//            * A's text is cached in TextCache.
//            * We make a search for a text in TextCache that fits B
//              (same buffer and same width).
//          o Horizontal resize
//            If the BufferView's width (LyXView) is horizontally changed all
//            the entries in the TextCache are deleted. (This causes
//            reformat of all loaded documents when next viewed)
//          o Close
//            When a buffer is closed we don't have to do anything, because
//            to close a single buffer it is required to only exist in the
//            BufferView and not in the TextCache. Upon LFUN_QUIT we
//            don't really care since everything is deleted anyway.
//        - The next scenario is when we have several BufferViews (in one or
//          more LyXViews) of equal width.
//          o Opening
//            Nothing to do with the TextCache is done when opening a file.
//          o Switching
//            We switch from buffer A to buffer B.
//            * If A is in another Bufferview we do not put it into TextCache.
//              else we put A into TextCache.
//            * If B is viewed in another BufferView we make a copy of its
//              text and use that, else we search in TextCache for a match.
//              (same buffer same width)
//          o Horizontal resize
//            If the BufferView's width (LyXView) is horisontaly changed all
//            the entries in the TextCache is deleted. (This causes
//            reformat of all loaded documents when next viewed)
//          o Close
//        - The last scenario should cover both the previous ones, this time
//          we have several BufferViews (in one or more LyXViews) with no
//          limitations on width. (And if you wonder why the two other
//          senarios are needed... I used them to get to this one.)
//          o Opening
//            Nothing to do with the TextCache is done when opening a file.
//          o Switching
//            We switch from buffer A to buffer B.
//          o Horisontal rezize
//          o Close

/** This class is used to cache generated LyXText's.
    The LyXText's is used by the BufferView to visualize the contents
    of a buffer and its paragraphs. Instead of deleting the LyXText when
    we switch from one document to another we cache it here so that when
    we switch back we do not have to reformat. This makes switching very
    fast at the expense of a bit higher memory usage.
*/
class TextCache {
public:
	///
	typedef vector<LyXText*> Cache;
	///
	typedef LyXText * value_type;

	/** Returns a pointer to a LyXText that fits the provided buffer
	    and width. Of there is no match 0 is returned. */
	LyXText * findFit(Buffer * b, unsigned short p);
	/** Lists all the LyXText's currently in the cache.
	    Uses msg as header for the list. */
	void show(ostream & o, string const & msg);
	/// Gives info on a single LyXText (buffer and width)
	static void show(ostream & o, LyXText const *);
	/** Adds a LyXText to the cache iff its buffer is
	    present in bufferlist. */
	void add(LyXText *);
	/** Clears the cache. Deletes all LyXText's and releases
	    the allocated memory. */
	void clear();
	/// Removes all LyXText's that has buffer b from the TextCache
	void removeAllWithBuffer(Buffer * b);
private:
	/// The cache.
	Cache cache;
};

// bla bla
extern TextCache textcache;
#endif
