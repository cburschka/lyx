// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef UNDO_FUNCS_H
#define UNDO_FUNCS_H

#include "undo.h"
#include "ParagraphList.h"

class BufferView;
class Paragraph;

/// returns false if no undo possible
bool textUndo(BufferView *);

/// returns false if no redo possible
bool textRedo(BufferView *);

/// makes sure the next operation will be stored
void finishUndo();

/**
 * Whilst undo is frozen, all actions do not get added
 * to the undo stack
 */
void freezeUndo();

/// Track undos again
void unFreezeUndo();

/**
 * Record undo information - call with the first paragraph that will be changed
 * and the last paragraph that will be changed. So we give an inclusive
 * range.
 * This is called before you make the changes to the paragraph, and it
 * will record the original information of the paragraphs in the undo stack.
 */
void setUndo(BufferView *, Undo::undo_kind kind,
		    ParagraphList::iterator first, ParagraphList::iterator last);
/// Convienience: Prepare undo when change in a single paragraph.
void setUndo(BufferView *, Undo::undo_kind kind,
		    ParagraphList::iterator first);

/// Convienience: Prepare undo for the paragraph that contains the cursor
void setUndo(BufferView *, Undo::undo_kind kind);

/// Convienience: Prepare and finish undo for the paragraph that contains the cursor
void setCursorParUndo(BufferView *);

/// Are we avoiding tracking undos currently ?
extern bool undo_frozen;

#endif // UNDO_FUNCS_H
