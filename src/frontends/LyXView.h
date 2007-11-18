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

namespace lyx {

namespace support { class FileName; }

class Buffer;
class BufferView;
class FuncRequest;
class Inset;

namespace frontend {

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
	virtual bool isToolbarVisible(std::string const & id) = 0;

	//@}

	/// load a buffer into the current workarea.
	virtual Buffer * loadLyXFile(support::FileName const &  name, ///< File to load.
		bool tolastfiles = true) = 0;  ///< append to the "Open recent" menu?

	/// updates the possible layouts selectable
	virtual void updateLayoutChoice(bool force) = 0;
	/// update the toolbar
	virtual void updateToolbars() = 0;
	/// update the status bar
	virtual void updateStatusBar() = 0;
	/// display a message in the view
	virtual void message(docstring const &) = 0;

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


	//
	// This View's Dialogs
	//
	
	/// Hide all visible dialogs
	virtual void hideAll() const = 0;

	/** \param name == "bibtex", "citation" etc; an identifier used to
	    launch a particular dialog.
	    \param data is a string representation of the Inset contents.
	    It is often little more than the output from Inset::write.
	    It is passed to, and parsed by, the frontend dialog.
	    Several of these dialogs do not need any data,
	    so it defaults to string().
	    \param inset ownership is _not_ passed to the frontend dialog.
	    It is stored internally and used by the kernel to ascertain
	    what to do with the FuncRequest dispatched from the frontend
	    dialog on 'Apply'; should it be used to create a new inset at
	    the current cursor position or modify an existing, 'open' inset?
	*/
	virtual void showDialog(std::string const & name,
		std::string const & data, Inset * inset = 0) = 0;

	/** \param name == "citation", "bibtex" etc; an identifier used
	    to update the contents of a particular dialog with \param data.
	    See the comments to 'show', above.
	*/
	virtual void updateDialog(std::string const & name, std::string const & data) = 0;

	/// Is the dialog currently visible?
	virtual bool isDialogVisible(std::string const & name) const = 0;

	/** All Dialogs of the given \param name will be closed if they are
	    connected to the given \param inset.
	*/
	virtual void hideDialog(std::string const & name, Inset * inset) = 0;
	///
	virtual void disconnectDialog(std::string const & name) = 0;
	///
	virtual Inset * getOpenInset(std::string const & name) const = 0;

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
