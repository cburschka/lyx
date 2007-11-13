// -*- C++ -*-
/**
 * \file LyXView.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYXVIEW_H
#define LYXVIEW_H

#include "frontends/Delegates.h"
#include "support/strfwd.h"

#include <vector>

namespace lyx {

namespace support { class FileName; }

class Buffer;
class BufferView;
class FuncRequest;
class Inset;
class ToolbarInfo;

namespace frontend {

class Dialogs;

/**
 * LyXView - main LyX window
 *
 * This class represents the main LyX window and provides
 * accessor functions to its content.
 *
 * The eventual intention is that LyX will support a number
 * of containing LyXViews. Currently a lot of code still
 * relies on there being a single top-level view.
 *
 * Additionally we would like to support multiple views
 * in a single LyXView.
 */
class LyXView
	: public GuiBufferViewDelegate, public GuiBufferDelegate
{
public:
	///
	LyXView(int id) : id_(id) {}
	///
	virtual ~LyXView() {}
	///
	int id() const { return id_; }
	///
	virtual void close() = 0;
	///
	virtual void setFocus() = 0;

	/**
	 * This is called after the concrete view has been created.
	 * We have to have the toolbar and the other stuff created
	 * before we can populate it with this call.
	 */
	virtual void init() = 0;

	enum Maximized {
		NotMaximized,
		VerticallyMaximized,
		HorizontallyMaximized,
		CompletelyMaximized
	};

	///
	virtual void setGeometry(
		unsigned int width,
		unsigned int height,
		int posx, int posy,
		int maximize,
		unsigned int iconSizeXY,
		const std::string & geometryArg) = 0;

	/// show busy cursor
	virtual void setBusy(bool) = 0;

	//@{ generic accessor functions

	/// \return the current buffer view.
	virtual BufferView * view() = 0;

	/// \return the buffer currently shown in this window
	virtual Buffer * buffer() = 0;
	virtual Buffer const * buffer() const = 0;
	/// set a buffer to the current workarea.
	virtual void setBuffer(Buffer * b) = 0; ///< \c Buffer to set.

	///
	virtual void openLayoutList() = 0;
	///
	virtual bool isToolbarVisible(std::string const & id) = 0;
	///
	virtual void showMiniBuffer(bool visible) = 0;
	virtual void openMenu(docstring const & name) = 0;

	/// get access to the dialogs
	virtual Dialogs & getDialogs() = 0;
	///
	virtual Dialogs const & getDialogs() const = 0;

	//@}

	/// load a buffer into the current workarea.
	virtual Buffer * loadLyXFile(support::FileName const &  name, ///< File to load.
		bool tolastfiles = true) = 0;  ///< append to the "Open recent" menu?

	/// updates the possible layouts selectable
	virtual void updateLayoutChoice(bool force) = 0;

	/// update the toolbar
	virtual void updateToolbars() = 0;
	/// get toolbar info
	virtual ToolbarInfo * getToolbarInfo(std::string const & name) = 0;
	/// toggle toolbar state
	virtual void toggleToolbarState(std::string const & name, bool allowauto) = 0;
	/// update the status bar
	virtual void updateStatusBar() = 0;

	/// display a message in the view
	virtual void message(docstring const &) = 0;

	/// clear any temporary message and replace with current status
	virtual void clearMessage() = 0;

	/// dispatch to current BufferView
	virtual void dispatch(FuncRequest const & cmd) = 0;

	/** redraw \c inset in all the BufferViews in which it is currently
	 *  visible. If successful return a pointer to the owning Buffer.
	 */
	virtual Buffer const * updateInset(Inset const *) = 0;

	/// returns true if this view has the focus.
	virtual bool hasFocus() const = 0;


	///
	virtual void restartCursor() = 0;
	
	//
	// GuiBufferDelegate
	//
	virtual void errors(std::string const &) = 0;

private:
	/// noncopyable
	LyXView(LyXView const &);
	void operator=(LyXView const &);

	///
	int id_;
};

} // namespace frontend
} // namespace lyx

#endif // LYXVIEW_H
