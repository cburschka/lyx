// -*- C++ -*-
/**
 * \file Kernel.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef KERNEL_H
#define KERNEL_H


#include "LString.h"


class Buffer;
class BufferView;
class FuncRequest;
class LyXView;


/** \c Kernel is a wrapper making the LyX kernel available to the dialog.
 * (Ie, it provides an interface to the Model part of the Model-Controller-
 *  View split.
 *  In an ideal world, it will shrink as more info is passed to the
 *  Dialog::show() and update() methods.
 */
class Kernel {
public:
	/// \param lv is the access point for the dialog to the LyX kernel.
	Kernel(LyXView & lv);

	/** This method is the primary prupose of the class. It provides
	    the "gateway" by which the dialog can send a request (of a
	    change in the data, for more information) to the kernel,
	    encoded as \param fr.
	    \param verbose Set to true if the completed action should
	    be displayed in the minibuffer.
	 */
	void dispatch(FuncRequest const & fr, bool verbose = false) const;

	/** The dialog has received a request from the user
	    (who pressed the "Restore" buuton) to update  contents.
	    It must, therefore, ask the kernel to provide this information.
	    \param name is used as an identifier by the kernel
	    when the information is posted.
	 */
	void updateDialog(string const & name) const;

	/** A request from the Controller that future changes to the data
	 *  stored by the dialog are not applied to the inset currently
	 *  connected to the dialog. Instead, they will be used to generate
	 *  a new inset at the cursor position.
	 */
	void disconnect(string const & name) const;

	//@{
	/// Simple wrapper functions to Buffer methods.
	bool isBufferAvailable() const;
	bool isBufferReadonly() const;
	//@}

	//@{
	/** \enum DocTypes is used to flag the different kinds of buffer
	 *  without making the kernel header files available to the
	 *  dialog's Controller or View.
	 */
	enum DocTypes {
		LATEX,
		LITERATE,
		LINUXDOC,
		DOCBOOK
	};
	/// The type of the current buffer.
	DocTypes docType() const;
	//@}


	//@{
	/** Unpleasantly public internals of the LyX kernel.
	 *  We should aim to reduce/remove these from the interface.
	 */
	LyXView & lyxview() { return lyxview_; }
	LyXView const & lyxview() const { return lyxview_; }

	Buffer * buffer();
	Buffer const * buffer() const;

	BufferView * bufferview();
	BufferView const * bufferview() const;
	//@}

private:
	LyXView & lyxview_;
};


#endif // KERNEL_H
