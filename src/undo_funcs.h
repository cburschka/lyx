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
class LyXCursor;

/// returns false if no undo possible
extern bool textUndo(BufferView *);
/// returns false if no redo possible
extern bool textRedo(BufferView *);
/// used by TextUndo/TextRedo
extern bool textHandleUndo(BufferView *, Undo * undo);
/// makes sure the next operation will be stored
extern void finishUndo();
/// this is dangerous and for internal use only
extern void freezeUndo();
/// this is dangerous and for internal use only
extern void unFreezeUndo();
///
extern void setUndo(BufferView *, Undo::undo_kind kind,
		    Paragraph const * first, Paragraph const * behind);
///
extern void setRedo(BufferView *, Undo::undo_kind kind,
		    Paragraph const * first, Paragraph const * behind);
///
extern Undo * createUndo(BufferView *, Undo::undo_kind kind,
			 Paragraph const * first, Paragraph const * behind);
/// for external use in lyx_cb.C
extern void setCursorParUndo(BufferView *);

// returns a pointer to the very first Paragraph depending of where we are
// so it will return the first paragraph of the buffer or the first paragraph
// of the textinset we're in.
extern Paragraph * firstUndoParagraph(BufferView *, int inset_arg);

///
extern LyXCursor const & undoCursor(BufferView * bv);

/// the flag used by FinishUndo();
extern bool undo_finished;
/// a flag
extern bool undo_frozen;

#endif
