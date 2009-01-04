// -*- C++ -*-
/**
 * \file Clipboard.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BASE_CLIPBOARD_H
#define BASE_CLIPBOARD_H

#include "Cursor.h"

#include "support/FileName.h"
#include "support/strfwd.h"

using lyx::support::FileName;

namespace lyx {
namespace frontend {

/**
 * A Clipboard class manages the clipboard.
 */
class Clipboard
{
public:
	virtual ~Clipboard() {}

	enum GraphicsType {
		PdfGraphicsType,
		PngGraphicsType,
		JpegGraphicsType,
		LinkBackGraphicsType,
		EmfGraphicsType,
		WmfGraphicsType,
		AnyGraphicsType,
	};

	/**
	 * Get the system clipboard contents. The format is as written in
	 * .lyx files (may even be an older version than ours if it comes
	 * from an older LyX).
	 * Does not convert plain text to LyX if only plain text is available.
	 * This should be called when the user requests to paste from the
	 * clipboard.
	 */
	virtual std::string const getAsLyX() const = 0;
	/// Get the contents of the window system clipboard in plain text format.
	virtual docstring const getAsText() const = 0;
	/// Get the contents of the window system clipboard as graphics file.
	virtual FileName getAsGraphics(Cursor const & cur, GraphicsType type) const = 0;
	
	/**
	 * Fill the system clipboard. The format of \p lyx is as written in
	 * .lyx files, the format of \p text is plain text.
	 * We put the clipboard contents in LyX format and plain text into
	 * the system clipboard if supported, so that it is useful for other
	 * applications as well as other instances of LyX.
	 * This should be called when the user requests to cut or copy to
	 * the clipboard.
	 */
	virtual void put(std::string const & lyx, docstring const & text) = 0;

	/// Does the clipboard contain LyX contents?
	virtual bool hasLyXContents() const = 0;
	/// Does the clipboard contain text contents?
	virtual bool hasTextContents() const = 0;
	/// Does the clipboard contain graphics contents of a certain type?
	virtual bool hasGraphicsContents(GraphicsType type = AnyGraphicsType) const = 0;
	/// state of clipboard.
	/// \returns true if the system clipboard has been set within LyX
	/// (document contents, dialogs count as external here).
	virtual bool isInternal() const = 0;
	/// \returns true if the OS has the concept of clipboard ownership,
	/// which is crucial for our concept of internal clipboard.
	virtual bool hasInternal() const = 0;
	/// Is the clipboard empty?
	/// \returns true if both the LyX and the plaintext versions of the
	/// clipboard are empty, and no supported graphics format is available.
	virtual bool empty() const = 0;
};

} // namespace frontend

/// Implementation is in Application.cpp
extern frontend::Clipboard & theClipboard();

} // namespace lyx


#endif // BASE_CLIPBOARD_H
