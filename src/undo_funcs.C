/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#include "undo_funcs.h"
#include "lyxtext.h"
#include "funcrequest.h"
#include "BufferView.h"
#include "buffer.h"
#include "insets/updatableinset.h"
#include "insets/insettext.h"
#include "debug.h"
#include "support/LAssert.h"
#include "iterators.h"


/// The flag used by FinishUndo().
bool undo_finished;
/// Whether actions are not added to the undo stacks.
bool undo_frozen;

namespace {


/**
 * Finish the undo operation in the case there was no entry
 * on the stack to perform.
 */
void finishNoUndo(BufferView * bv)
{
	freezeUndo();
	bv->unlockInset(bv->theLockingInset());
	finishUndo();
	bv->text->postPaint();
	unFreezeUndo();
}


// Returns false if no undo possible.
bool textHandleUndo(BufferView * bv, Undo & undo)
{
	Buffer * buf = bv->buffer();

	ParagraphList * plist = &buf->paragraphs;
/*
	ParIterator null = buf->par_iterator_end();

	for (ParIterator it = buf->par_iterator_begin(); it != null; ++it)
		if (it.plist().id() == undo.plist_id) {
			plist = &it.plist();
			break;
		}
*/

	// Set the right(new) inset-owner of the paragraph if there is any.
	UpdatableInset * inset =
		static_cast<UpdatableInset *>(buf->getInsetFromID(undo.inset_id));

	ParagraphList::iterator pit = undo.pars.begin();
	ParagraphList::iterator end = undo.pars.end();
	for ( ; pit != end; ++pit)
		pit->setInsetOwner(inset);

	//lyxerr << "\nhandle: inset_id: " << undo.inset_id << "\n";
	//lyxerr << "handle: inset: " << inset << "\n";
	//lyxerr << "handle: plist_id: " << undo.plist_id << "\n";
	lyxerr << "handle: undo.pars.size(): " << undo.pars.size() << "\n";
	lyxerr << "handle: first_offset:  " << undo.first_par_offset  << "\n";
	lyxerr << "handle: last_offset: " << undo.last_par_offset << "\n";

	// remove stuff between first and behind
	{
		ParagraphList::iterator first = plist->begin();
		advance(first, undo.first_par_offset);
		ParagraphList::iterator last = plist->begin();
		advance(last, plist->size() - undo.last_par_offset);
		lyxerr << "handle: first_id:  " << first->id()  << "\n";
		lyxerr << "handle: last_id: " << last->id() << "\n";
		lyxerr << "handle: remove: " << distance(first, last) + 1 << " pars\n";
		plist->erase(first, ++last);
		lyxerr << "handle: after remove\n";
	}

	// re-insert old stuff
	{
		ParagraphList::iterator first = plist->begin();
		advance(first, undo.first_par_offset);
		lyxerr << "handle: plist->size: " << plist->size() << "\n";
		lyxerr << "handle: offset: " << undo.first_par_offset << "\n";
		plist->insert(first, undo.pars.begin(), undo.pars.end());
		lyxerr << "handle: after insert\n";
	}

	/*
		// A memory optimization for edit:
		// Only layout information
		// is stored in the undo. So restore
		// the text informations.
		if (undo.kind == Undo::EDIT) {
			undo.pars[par]->setContentsFromPar(*deletelist.back());
			++par;
		}
	*/

	// redo Paragraphs  (should be handled outside undo...)
	{
		//LyXText * text = inset ? inset->getLyXText(bv) : bv->text;
		LyXText * text = bv->text;
		lyxerr << "handle: text: " << text << "\n";
		if (undo.first_par_offset) {
			ParagraphList::iterator redo = plist->begin();
			lyxerr << "handle: 1\n";
			advance(redo, undo.first_par_offset);
			lyxerr << "handle: 2\n";
			text->setCursorIntern(plist->begin(), 0);
		}
		lyxerr << "handle: 3\n";
		text->redoParagraphs(text->cursor, plist->end());
		lyxerr << "handle: after redo\n";

		if (inset) {
			FuncRequest cmd(bv, LFUN_INSET_EDIT, "left");
			inset->localDispatch(cmd);
		}
	}

	if (inset) {
		lyxerr << "fit cursor...\n";
		bv->fitCursor();
		bv->updateInset(inset);
	}

	// set cursor
	{
		LyXText * text = inset ? inset->getLyXText(bv) : bv->text;
		ParagraphList::iterator cursor = text->ownerParagraphs().begin();
		advance(cursor, undo.cursor_par_offset);
		text->setCursorIntern(cursor, undo.cursor_pos);
		// Clear any selection and set the selection
		// cursor for an evt. new selection.
		text->clearSelection();
		text->selection.cursor = text->cursor;
		text->updateCounters();
		lyxerr << "after setCursor\n";
	}


	finishUndo();
	bv->text->postPaint();

	lyxerr << "finished  textHandleUndo...\n";
	return true;
}


void createUndo(BufferView * bv, Undo::undo_kind kind,
	ParagraphList::iterator first, ParagraphList::iterator last,
	limited_stack<Undo> & stack)
{
	Buffer * buf = bv->buffer();

	ParagraphList * plist = 0;
	ParIterator null = buf->par_iterator_end();

	lyxerr << "\n";

	UpdatableInset * inset = first->inInset();
	LyXText * text = inset ? inset->getLyXText(bv) : bv->text;
	int const inset_id = inset ? inset->id() : -1;

#if 0
	// this is what we'd like to have in the end for small grained undo
	for (ParIterator it = buf->par_iterator_begin(); it != null; ++it) {
		if (it->id() == first->id()) {
			plist = &it.plist();
			break;
		}
	}

#else

	// and that's the big stick we wield now
	lyxerr << "create: first_id orig:   " << first->id()  << "\n";
	lyxerr << "create: last_id orig:    " << last->id()   << "\n";
	plist = &buf->paragraphs;
	// this is what we'd like to have in the end for small grained undo
	for (ParIterator it = buf->par_iterator_begin(); it != null; ++it) {
		if (it->id() == first->id())
			first = it.outerPar();
		if (it->id() == last->id())
			last = it.outerPar();
	}

#endif

	int const first_offset = std::distance(plist->begin(), first);
	int const last_offset  = std::distance(last, plist->end());

	if (last == plist->end()) {
		lyxerr << "*** createUndo: last == end  should not happen\n";
	}


	// Undo::EDIT and Undo::FINISH are
	// always finished. (no overlapping there)
	// overlapping only with insert and delete inside one paragraph:
	// Nobody wants all removed  character
	// appear one by one when undoing.
	// EDIT is special since only layout information, not the
	// contents of a paragaph are stored.
	if (!undo_finished && kind != Undo::EDIT && kind != Undo::FINISH) {
		// Check whether storing is needed.
		if (!buf->undostack.empty() &&
		    buf->undostack.top().kind == kind &&
		    buf->undostack.top().first_par_offset == first_offset &&
		    buf->undostack.top().last_par_offset == last_offset) {
			// No undo needed.
			return;
		}
	}

	// Create a new Undo.
	int const cursor_offset = std::distance
		(text->ownerParagraphs().begin(), text->cursor.par());

	//lyxerr << "create: plist_id:      " << plist->id()  << "\n";
	//lyxerr << "create: inset_id:      " << inset_id  << "\n";
	lyxerr << "create: first_id:      " << first->id()  << "\n";
	lyxerr << "create: last_id:       " << last->id()   << "\n";
	lyxerr << "create: first_offset:  " << first_offset  << "\n";
	lyxerr << "create: last_offset:   " << last_offset   << "\n";
	lyxerr << "create: cursor_offset: " << cursor_offset   << "\n";
	lyxerr << "create: cursor_pos:    " << text->cursor.pos() << "\n";

	stack.push(Undo(kind, inset_id, 0, //plist->id(),
		first_offset, last_offset,
		cursor_offset, text->cursor.pos(),
		ParagraphList()));

	ParagraphList & undo_pars = stack.top().pars;

	for (ParagraphList::iterator it = first; it != last; ++it) {
		undo_pars.push_back(*it);
		undo_pars.back().id(it->id());
	}
	undo_pars.push_back(*last);
	undo_pars.back().id(last->id());

	// A memory optimization: Just store the layout
	// information when only edit.
#warning Waste...
	//if (kind == Undo::EDIT)
	//	for (size_t i = 0, n = undo_pars.size(); i < n; ++i)
	//		undo_pars[i]->clearContents();

	undo_finished = false;
}


// Returns false if no undo possible.
bool textUndoOrRedo(BufferView * bv,
	limited_stack<Undo> & stack,
	limited_stack<Undo> & otherstack)
{
	if (stack.empty()) {
		finishNoUndo(bv);
		return false;
	}

	Undo undo = stack.top();
	stack.pop();
	finishUndo();

	if (!undo_frozen) {
		otherstack.push(undo);
		otherstack.top().pars.clear();
		Buffer * buf = bv->buffer();
		ParagraphList & plist = buf->paragraphs;
		lyxerr << "\nredo: first: " << undo.first_par_offset << "\n";
		lyxerr << "redo: last:  " << undo.last_par_offset << "\n";
		lyxerr << "redo: size:  " << plist.size() << "\n";
		if (undo.first_par_offset + undo.last_par_offset <= int(plist.size())) {
			ParagraphList::iterator first = plist.begin();
			advance(first, undo.first_par_offset);
			ParagraphList::iterator last = plist.begin();
			advance(last, plist.size() - undo.last_par_offset + 1);
			otherstack.top().pars.insert(otherstack.top().pars.begin(), first, last);
		}
	}

	// Now we can unlock the inset for safety because the inset
	// pointer could be changed during the undo-function. Anyway
	// if needed we have to lock the right inset/position if this
	// is requested.
	freezeUndo();
	bv->unlockInset(bv->theLockingInset());
	bool const ret = textHandleUndo(bv, undo);
	unFreezeUndo();
	return ret;
}

} // namespace anon


void finishUndo()
{
	// Makes sure the next operation will be stored.
	undo_finished = true;
}


void freezeUndo()
{
	// This is dangerous and for internal use only.
	undo_frozen = true;
}


void unFreezeUndo()
{
	// This is dangerous and for internal use only.
	undo_frozen = false;
}


bool textUndo(BufferView * bv)
{
	return textUndoOrRedo(bv, bv->buffer()->undostack,
			      bv->buffer()->redostack);
}


bool textRedo(BufferView * bv)
{
	return textUndoOrRedo(bv, bv->buffer()->redostack,
			      bv->buffer()->undostack);
}


void setUndo(BufferView * bv, Undo::undo_kind kind)
{
	setUndo(bv, kind, bv->text->cursor.par());
}


void setUndo(BufferView * bv, Undo::undo_kind kind,
	     ParagraphList::iterator first)
{
	setUndo(bv, kind, first, first);
}


void setUndo(BufferView * bv, Undo::undo_kind kind,
	     ParagraphList::iterator first, ParagraphList::iterator last)
{
	if (!undo_frozen) {
		createUndo(bv, kind, first, last, bv->buffer()->undostack);
		bv->buffer()->redostack.clear();
	}
}


void setCursorParUndo(BufferView * bv)
{
	setUndo(bv, Undo::FINISH, bv->text->cursor.par());
}
