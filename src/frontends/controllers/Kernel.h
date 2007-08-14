// -*- C++ -*-
/**
 * \file Kernel.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef KERNEL_H
#define KERNEL_H

#include <string>

namespace lyx {

class Buffer;
class BufferView;
class FuncRequest;

namespace frontend {

class LyXView;

/** \c Kernel is a wrapper making the LyX kernel available to the dialog.
 * (Ie, it provides an interface to the Model part of the Model-Controller-
 *  View split.
 *  In an ideal world, it will shrink as more info is passed to the
 *  Dialog::show() and Dialog::update() methods.
 */
class Kernel {
public:
	/// \param lv is the access point for the dialog to the LyX kernel.
	Kernel(LyXView & lv);

	/** This method is the primary purpose of the class. It provides
	 *  the "gateway" by which the dialog can send a request (of a
	 *  change in the data, for more information) to the kernel.
	 *  \param fr is the encoding of the request.
	 */
	void dispatch(FuncRequest const & fr) const;

	/** The dialog has received a request from the user
	 *  (who pressed the "Restore" button) to update contents.
	 *  It must, therefore, ask the kernel to provide this information.
	 *  \param name is used to identify the dialog to the kernel.
	 */
	void updateDialog(std::string const & name) const;

	/** A request from the Controller that future changes to the data
	 *  stored by the dialog are not applied to the inset currently
	 *  connected to the dialog. Instead, they will be used to generate
	 *  a new inset at the cursor position.
	 *  \param name is used to identify the dialog to the kernel.
	 */
	void disconnect(std::string const & name) const;

	/** \name Kernel Wrappers
	 *  Simple wrapper functions to Buffer methods.
	 */
	//@{
	bool isBufferAvailable() const;
	bool isBufferReadonly() const;
	std::string const bufferFilepath() const;
	//@}

	/** \enum DocType used to flag the different kinds of buffer
	 *  without making the kernel header files available to the
	 *  dialog's Controller or View.
	 */
	enum DocType {
		LATEX,
		LITERATE,
		DOCBOOK
	};
	/// The type of the current buffer.
	DocType docType() const;

	/** \name Kernel Nasties
	 *  Unpleasantly public internals of the LyX kernel.
	 *  We should aim to reduce/remove these from the interface.
	 */
	//@{
	LyXView & lyxview() { return lyxview_; }
	LyXView const & lyxview() const { return lyxview_; }

	Buffer & buffer();
	Buffer const & buffer() const;

	BufferView * bufferview();
	BufferView const * bufferview() const;
	//@}

private:
	LyXView & lyxview_;
};


/** \c KernelDocType is a wrapper for Kernel::DocType.
 *  It can be forward-declared and passed as a function argument without
 *  having to expose Kernel.h.
 */
class KernelDocType {
public:
	KernelDocType(Kernel::DocType val) : val_(val) {}
	operator Kernel::DocType() const { return val_; }
private:
	Kernel::DocType val_;
};

} // namespace frontend
} // namespace lyx

#endif // KERNEL_H
