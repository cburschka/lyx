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
/// Whilst undo is frozen, all actions do not get added
/// to the undo stack
void freezeUndo();
/// Track undos again
void unFreezeUndo();
/// FIXME
void setUndo(BufferView *, Undo::undo_kind kind,
		    ParagraphList::iterator first, ParagraphList::iterator last);
void setUndo(BufferView *, Undo::undo_kind kind,
		    ParagraphList::iterator first);
/// FIXME
void setCursorParUndo(BufferView *);

/// Are we avoiding tracking undos currently ?
extern bool undo_frozen;

#endif // UNDO_FUNCS_H
