/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "undo_funcs.h"
#include "lyxtext.h"
#include "BufferView.h"
#include "buffer.h"
#include "insets/inset.h"
#include "debug.h"
#include "support/LAssert.h"

#include "iterators.h"

#define DELETE_UNUSED_PARAGRAPHS 1
#ifdef DELETE_UNUSED_PARAGRAPHS
#include <vector>
#endif

using std::vector;


/// the flag used by FinishUndo();
bool undo_finished;
/// a flag
bool undo_frozen;


bool textUndo(BufferView * bv)
{
	// returns false if no undo possible
	Undo * undo = bv->buffer()->undostack.top();
	bv->buffer()->undostack.pop();
	if (undo) {
		finishUndo();
		if (!undo_frozen) {
			Paragraph * first = bv->buffer()->getParFromID(undo->number_of_before_par);
			if (first && first->next())
				first = first->next();
			else if (!first)
				first = firstUndoParagraph(bv, undo->number_of_inset_id);
			if (first) {
				bv->buffer()->redostack.push(
					createUndo(bv, undo->kind, first,
						   bv->buffer()->getParFromID(undo->number_of_behind_par)));
			}
		}
	}
	// now we can unlock the inset for saftey because the inset pointer could
	// be changed during the undo-function. Anyway if needed we have to lock
	// the right inset/position if this is requested.
	freezeUndo();
	bv->unlockInset(bv->theLockingInset());
	bool ret = textHandleUndo(bv, undo);
	unFreezeUndo();
	return ret;
}


bool textRedo(BufferView * bv)
{
	// returns false if no redo possible
	Undo * undo = bv->buffer()->redostack.top();
	bv->buffer()->redostack.pop();
	if (undo) {
		finishUndo();
		if (!undo_frozen) {
			Paragraph * first = bv->buffer()->getParFromID(undo->number_of_before_par);
			if (first && first->next())
				first = first->next();
			else if (!first)
				first = firstUndoParagraph(bv, undo->number_of_inset_id);
			if (first) {
				bv->buffer()->undostack.push(
					createUndo(bv, undo->kind, first,
						   bv->buffer()->getParFromID(undo->number_of_behind_par)));
			}
		}
	}
	// now we can unlock the inset for saftey because the inset pointer could
	// be changed during the undo-function. Anyway if needed we have to lock
	// the right inset/position if this is requested.
	freezeUndo();
	bv->unlockInset(bv->theLockingInset());
	bool ret = textHandleUndo(bv, undo);
	unFreezeUndo();
	return ret;
}


bool textHandleUndo(BufferView * bv, Undo * undo)
{
	// returns false if no undo possible
	bool result = false;
	if (undo) {
		Paragraph * before =
			bv->buffer()->getParFromID(undo->number_of_before_par);
		Paragraph * behind =
			bv->buffer()->getParFromID(undo->number_of_behind_par);
		Paragraph * tmppar;
		Paragraph * tmppar2;
		Paragraph * tmppar5;

		// if there's no before take the beginning
		// of the document for redoing
		if (!before) {
			LyXText * t = bv->text;
			int num = undo->number_of_inset_id;
			if (undo->number_of_inset_id >= 0) {
				Inset * in = bv->buffer()->getInsetFromID(num);
				if (in) {
					t = in->getLyXText(bv);
				} else {
					num = -1;
				}
			}
			t->setCursorIntern(bv, firstUndoParagraph(bv, num), 0);
		}

		// replace the paragraphs with the undo informations

		Paragraph * tmppar3 = undo->par;
		undo->par = 0;	/* otherwise the undo destructor would
				   delete the paragraph */

		// get last undo par and set the right(new) inset-owner of the
		// paragraph if there is any. This is not needed if we don't have
		// a paragraph before because then in is automatically done in the
		// function which assigns the first paragraph to an InsetText. (Jug)
		Paragraph * tmppar4 = tmppar3;
		if (tmppar4) {
			Inset * in = 0;
			if (before)
				in = before->inInset();
			else if (undo->number_of_inset_id >= 0)
				in = bv->buffer()->getInsetFromID(undo->number_of_inset_id);
			tmppar4->setInsetOwner(in);
			while (tmppar4->next()) {
				tmppar4 = tmppar4->next();
				tmppar4->setInsetOwner(in);
			}
		}

		// now remove the old text if there is any
#ifdef DELETE_UNUSED_PARAGRAPHS
		vector<Paragraph *> vvpar;
#endif
		if (before != behind || (!behind && !before)) {
			if (before)
				tmppar5 = before->next();
			else
				tmppar5 = firstUndoParagraph(bv, undo->number_of_inset_id);
			tmppar2 = tmppar3;
			while (tmppar5 && tmppar5 != behind) {
#ifdef DELETE_UNUSED_PARAGRAPHS
				vvpar.push_back(tmppar5);
#endif
				tmppar = tmppar5;
				tmppar5 = tmppar5->next();
				// a memory optimization for edit:
				// Only layout information
				// is stored in the undo. So restore
				// the text informations.
				if (undo->kind == Undo::EDIT) {
					tmppar2->setContentsFromPar(tmppar);
#ifndef DELETE_UNUSED_PARAGRAPHS
					tmppar->clearContents();
#endif
					tmppar2 = tmppar2->next();
				}
			}
		}

		// put the new stuff in the list if there is one
		if (tmppar3) {
			if (before)
				before->next(tmppar3);
			else
				bv->text->ownerParagraph(firstUndoParagraph(bv, undo->number_of_inset_id)->id(),
							 tmppar3);

			tmppar3->previous(before);
		} else {
			// We enter here on DELETE undo operations where we have to
			// substitue the second paragraph with the first if the removed
			// one is the first!
			if (!before && behind) {
				bv->text->ownerParagraph(firstUndoParagraph(bv, undo->number_of_inset_id)->id(),
							 behind);
				tmppar3 = behind;
			}
		}
		if (tmppar4) {
			tmppar4->next(behind);
			if (behind)
				behind->previous(tmppar4);
		}


		// Set the cursor for redoing
		if (before) {
			Inset * it = before->inInset();
			if (it)
				it->getLyXText(bv)->setCursorIntern(bv, before, 0);
			else
				bv->text->setCursorIntern(bv, before, 0);
		}

		Paragraph * endpar = 0;
		// calculate the endpar for redoing the paragraphs.
		if (behind)
			endpar = behind->next();

		tmppar = bv->buffer()->getParFromID(undo->number_of_cursor_par);
		UpdatableInset* it = 0;
		if (tmppar3)
			it = static_cast<UpdatableInset*>(tmppar3->inInset());
		if (it) {
			it->getLyXText(bv)->redoParagraphs(bv,
							   it->getLyXText(bv)->cursor,
							   endpar);
			if (tmppar) {
				it = static_cast<UpdatableInset*>(tmppar->inInset());
				LyXText * t;
				if (it) {
					it->edit(bv);
					t = it->getLyXText(bv);
				} else {
					t = bv->text;
				}
				t->setCursorIntern(bv, tmppar, undo->cursor_pos);
				// clear any selection and set the selection cursor
				// for an evt. new selection.
				t->clearSelection();
				t->selection.cursor = t->cursor;
				t->updateCounters(bv, t->cursor.row());
				bv->fitCursor();
			}
			bv->updateInset(it, false);
			bv->text->setCursorIntern(bv, bv->text->cursor.par(),
						  bv->text->cursor.pos());
		} else {
			bv->text->redoParagraphs(bv, bv->text->cursor, endpar);
			if (tmppar) {
				LyXText * t;
				Inset * it = tmppar->inInset();
				if (it) {
					it->edit(bv);
					t = it->getLyXText(bv);
				} else {
					t = bv->text;
				}
				t->setCursorIntern(bv, tmppar, undo->cursor_pos);
				// clear any selection and set the selection cursor
				// for an evt. new selection.
				t->clearSelection();
				t->selection.cursor = t->cursor;
				t->updateCounters(bv, t->cursor.row());
			}
		}
		result = true;
		delete undo;
#ifdef DELETE_UNUSED_PARAGRAPHS
		// And here it's save enough to delete all removed paragraphs
		vector<Paragraph *>::iterator pit = vvpar.begin();
		if (pit != vvpar.end()) {
#if 0
			lyxerr << endl << "PARS BEFORE:";
			ParIterator end = bv->buffer()->par_iterator_end();
			ParIterator it = bv->buffer()->par_iterator_begin();
			for (; it != end; ++it)
				lyxerr << (*it)->previous() << "<- " << (*it) << " ->" << (*it)->next() << endl;
			lyxerr << "DEL: ";
#endif
			for(;pit != vvpar.end(); ++pit) {
//				lyxerr << *pit << " ";
				(*pit)->previous(0);
				(*pit)->next(0);
				delete (*pit);
			}
#if 0
			lyxerr << endl << "PARS AFTER:";
			end = bv->buffer()->par_iterator_end();
			it = bv->buffer()->par_iterator_begin();
			for (; it != end; ++it)
				lyxerr << (*it)->previous() << "<- " << (*it) << " ->" << (*it)->next() << endl;
#endif
		}
#endif
	}
	finishUndo();
	bv->text->status(bv, LyXText::NEED_MORE_REFRESH);
	return result;
}


void finishUndo()
{
	// makes sure the next operation will be stored
	undo_finished = true;
}


void freezeUndo()
{
	// this is dangerous and for internal use only
	undo_frozen = true;
}


void unFreezeUndo()
{
	// this is dangerous and for internal use only
	undo_frozen = false;
}


void setUndo(BufferView * bv, Undo::undo_kind kind,
	     Paragraph const * first, Paragraph const * behind)
{
	if (!undo_frozen) {
		bv->buffer()->undostack.push(createUndo(bv, kind, first, behind));
		bv->buffer()->redostack.clear();
	}
}


void setRedo(BufferView * bv, Undo::undo_kind kind,
	     Paragraph const * first, Paragraph const * behind)
{
	bv->buffer()->redostack.push(createUndo(bv, kind, first, behind));
}


Undo * createUndo(BufferView * bv, Undo::undo_kind kind,
		  Paragraph const * first, Paragraph const * behind)
{
	lyx::Assert(first);

	int before_number = -1;
	int behind_number = -1;
	int inset_id = -1;

	if (first->previous())
		before_number = first->previous()->id();
	if (behind)
		behind_number = behind->id();
	if (first->inInset())
		inset_id = first->inInset()->id();

	// Undo::EDIT  and Undo::FINISH are
	// always finished. (no overlapping there)
	// overlapping only with insert and delete inside one paragraph:
	// Nobody wants all removed  character
	// appear one by one when undoing.
	// EDIT is special since only layout information, not the
	// contents of a paragaph are stored.
	if (!undo_finished && (kind != Undo::EDIT) && (kind != Undo::FINISH)) {
		// check wether storing is needed
		if (!bv->buffer()->undostack.empty() &&
		    bv->buffer()->undostack.top()->kind == kind &&
		    bv->buffer()->undostack.top()->number_of_before_par == before_number &&
		    bv->buffer()->undostack.top()->number_of_behind_par == behind_number) {
			// no undo needed
			return 0;
		}
	}
	// create a new Undo
	Paragraph * undopar;

	Paragraph * start = const_cast<Paragraph *>(first);
	Paragraph * end = 0;

	if (behind)
		end = const_cast<Paragraph*>(behind->previous());
	else {
		end = start;
		while (end->next())
			end = end->next();
	}
	if (start && end && (start != end->next()) &&
	    ((before_number != behind_number) ||
		 ((before_number < 0) && (behind_number < 0))))
	{
		Paragraph * tmppar = start;
		Paragraph * tmppar2 = new Paragraph(*tmppar, true);

		// a memory optimization: Just store the layout information
		// when only edit
		if (kind == Undo::EDIT) {
			tmppar2->clearContents();
		}

		undopar = tmppar2;

		while (tmppar != end && tmppar->next()) {
			tmppar = tmppar->next();
#if 0
			tmppar2->next(new Paragraph(*tmppar, true));
#else
			Paragraph * ptmp = new Paragraph(*tmppar, true);
			tmppar2->next(ptmp);
#endif
			// a memory optimization: Just store the layout
			// information when only edit
			if (kind == Undo::EDIT) {
				tmppar2->clearContents();
			}
			tmppar2->next()->previous(tmppar2);

			tmppar2 = tmppar2->next();
		}
		tmppar2->next(0);
	} else
		undopar = 0; // nothing to replace (undo of delete maybe)

	int cursor_par = undoCursor(bv).par()->id();
	int cursor_pos =  undoCursor(bv).pos();

	Undo * undo = new Undo(kind, inset_id,
			       before_number, behind_number,
			       cursor_par, cursor_pos, undopar);

	undo_finished = false;
	return undo;
}


void setCursorParUndo(BufferView * bv)
{
	setUndo(bv, Undo::FINISH, bv->text->cursor.par(),
		bv->text->cursor.par()->next());
}


Paragraph * firstUndoParagraph(BufferView * bv, int inset_id)
{
	Inset * inset = bv->buffer()->getInsetFromID(inset_id);
	if (inset) {
		Paragraph * result = inset->getFirstParagraph(0);
		if (result)
			return result;
	}
	return bv->text->ownerParagraph();
}


LyXCursor const & undoCursor(BufferView * bv)
{
	if (bv->theLockingInset())
		return bv->theLockingInset()->cursor(bv);
	return bv->text->cursor;
}
