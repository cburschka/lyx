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

#ifdef __GNUG__
#pragma interface
#endif

#include "undo.h"

class BufferView;
class Paragraph;

/// returns false if no undo possible
extern bool textUndo(BufferView *);
/// returns false if no redo possible
extern bool textRedo(BufferView *);
/// makes sure the next operation will be stored
extern void finishUndo();
/// this is dangerous and for internal use only
extern void freezeUndo();
/// this is dangerous and for internal use only
extern void unFreezeUndo();
/// FIXME
extern void setUndo(BufferView *, Undo::undo_kind kind,
		    Paragraph const * first, Paragraph const * behind);
/// FIXME
extern void setRedo(BufferView *, Undo::undo_kind kind,
		    Paragraph const * first, Paragraph const * behind);
/// FIXME
extern void setCursorParUndo(BufferView *);

// This is only used in one place. Need a nicer way.
/// is the undo frozen
extern bool undo_frozen;

#endif // UNDO_FUNCS_H
