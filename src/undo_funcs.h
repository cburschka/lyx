// -*- C++ -*-
/**
 * \file undo_funcs.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author André Pönitz
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef UNDO_FUNCS_H
#define UNDO_FUNCS_H

#include "undo.h"
#include "ParagraphList_fwd.h"
#include "support/types.h"

class BufferView;
class Paragraph;

/// This will undo the last action - returns false if no undo possible
bool textUndo(BufferView *);

/// This will redo the last undo - returns false if no redo possible
bool textRedo(BufferView *);

/// Makes sure the next operation will be stored
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
void recordUndo(BufferView *, Undo::undo_kind kind,
	ParagraphList & plist, lyx::paroffset_type first, lyx::paroffset_type last);

/// Convienience: Prepare undo when change in a single paragraph.
void recordUndo(BufferView *, Undo::undo_kind kind,
	ParagraphList & plist, lyx::paroffset_type par);

/// Convienience: Prepare undo for the paragraph that contains the cursor
void recordUndo(BufferView *, Undo::undo_kind kind);

/// Are we avoiding tracking undos currently ?
extern bool undo_frozen;

#endif // UNDO_FUNCS_H
