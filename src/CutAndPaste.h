// -*- C++ -*-
/**
 * \file CutAndPaste.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CUTANDPASTE_H
#define CUTANDPASTE_H

#include "DocumentClassPtr.h"

#include "support/docstring.h"

#include "frontends/Clipboard.h"

#include <vector>

using lyx::frontend::Clipboard;

namespace lyx {

class DocumentClass;
class ErrorList;
class InsetText;
class Cursor;
class ParagraphList;

namespace cap {

/// Get all elements of the cut buffer in plain text format.
std::vector<docstring> availableSelections(Buffer const *);
/// Get the number of available elements in the cut buffer.
size_type numberOfSelections();
/// Get the sel_index-th element of the cut buffer in plain text format.
docstring selection(size_t sel_index, DocumentClassConstPtr docclass);

/**
 * Replace using the font of the first selected character and select
 * the new string. Does handle undo.
 */
void replaceSelectionWithString(Cursor & cur, docstring const & str);
/// If a selection exists, delete it without pushing it to the cut buffer.
/// Does handle undo.
void replaceSelection(Cursor & cur);

/**
 * Cut the current selection and possibly push it to the cut buffer and
 * system clipboard.
 * Does handle undo. Calls saveSelection.
 * \param doclear If this is true: Delete leading spaces in paragraphs before
 *                they get merged.
 * \param realcut If this is true: Push the selection to the cut buffer and
 *                system clipboard. Set this to false to only delete the
 *                selection.
 */
void cutSelection(Cursor & cur, bool doclear = true, bool realcut = true);
/// Push the current selection to the cut buffer and the system clipboard.
void copySelection(Cursor const & cur);
///
void copyInset(Cursor const & cur, Inset * inset, docstring const & plaintext);
/**
 * Push the current selection to the cut buffer and the system clipboard.
 * \param plaintext plain text version of the selection for the system
 *        clipboard
 */
void copySelection(Cursor const & cur, docstring const & plaintext);
/// Push the selection buffer to the cut buffer.
void copySelectionToStack();
/// Store the current selection in the internal selection buffer
void saveSelection(Cursor const & cur);
/// Is a selection available in our selection buffer?
bool selection();
/// Clear our selection buffer
void clearSelection();
/// Clear our cut stack.
void clearCutStack();
/// Paste the current selection at \p cur
/// Does handle undo. Does only work in text, not mathed.
void pasteSelection(Cursor & cur, ErrorList &);
/// Replace the current selection with the clipboard contents as text
/// (internal or external: which is newer).
/// Does handle undo. Does only work in text, not mathed.
/// \p asParagraphs is only considered if plain text is pasted.
bool pasteClipboardText(Cursor & cur, ErrorList & errorList, bool asParagraphs,
	Clipboard::TextType preferedType = Clipboard::LyXOrPlainTextType);
/// Replace the current selection with the clipboard contents as graphic.
/// Does handle undo. Does only work in text, not mathed.
void pasteClipboardGraphics(Cursor & cur, ErrorList & errorList,
	Clipboard::GraphicsType preferedType = Clipboard::AnyGraphicsType);
/// Replace the current selection with cut buffer \c sel_index
/// Does handle undo. Does only work in text, not mathed.
bool pasteFromStack(Cursor & cur, ErrorList & errorList, size_t sel_index);
/// Paste the clipboard as simple text, removing any formatting
void pasteSimpleText(Cursor & cur, bool asParagraphs);

/// Paste the paragraph list \p parlist at the position given by \p cur.
/// Does not handle undo. Does only work in text, not mathed.
void pasteParagraphList(Cursor & cur, ParagraphList const & parlist,
			DocumentClassConstPtr textclass, ErrorList & errorList);


/** Needed to switch between different classes. This works
 *  for a list of paragraphs beginning with the specified par.
 *  It changes layouts and character styles.
 */
void switchBetweenClasses(DocumentClassConstPtr c1,
			DocumentClassConstPtr c2, InsetText & in, ErrorList &);

/// Get the current selection as a string. Does not change the selection.
/// Does only work if the whole selection is in mathed.
docstring grabSelection(Cursor const & cur);
/// Erase the current selection.
/// Does not handle undo. Does only work if the whole selection is in mathed.
/// Calls saveSelection.
void eraseSelection(Cursor & cur);
/// Reduce the selected text in mathed to only one cell. If it spans multiple
/// cells, the cursor is moved the end of the current cell and the anchor to the
/// start. If the selection is inside only one cell, nothing is done. Return
/// true if the selection now does not span multiple cells anymore.
bool reduceSelectionToOneCell(Cursor & cur);
/// Returns true if multiple cells are selected in mathed.
bool multipleCellsSelected(Cursor const & cur);
/// Erase the selection and return it as a string.
/// Does not handle undo. Does only work if the whole selection is in mathed.
docstring grabAndEraseSelection(Cursor & cur);
// other selection methods
/// Erase the selection if one exists.
/// Does not handle undo. Does only work if the whole selection is in mathed.
void selDel(Cursor & cur);
/// Clear or delete the selection if one exists, depending on lyxrc setting.
/// Does not handle undo. Does only work if the whole selection is in mathed.
void selClearOrDel(Cursor & cur);
/// Calculate rectangular region of cell between \c i1 and \c i2.
void region(CursorSlice const & i1, CursorSlice const & i2,
    Inset::row_type & r1, Inset::row_type & r2,
    Inset::col_type & c1, Inset::col_type & c2);
/** Tabular has its own paste stack for multiple cells
 *  but it needs to know whether there is a more recent
 *  ordinary paste. Therefore which one is newer.
 */
//FIXME: this is a workaround for bug 1919. Replace this by
//an all-for-one-paste mechanism in 1.7
/// store whether tabular or ordinary paste stack is newer
void dirtyTabularStack(bool b);
/// is the tabular paste stack newer than the ordinary one?
bool tabularStackDirty();
} // namespace cap
} // namespce lyx

#endif
