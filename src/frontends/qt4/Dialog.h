// -*- C++ -*-
/**
 * \file Dialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef DIALOG_H
#define DIALOG_H

#include "FuncCode.h"

#include "insets/InsetCode.h"

#include "support/strfwd.h"

#include <QString>

class QWidget;

namespace lyx {

class Buffer;
class BufferView;
class FuncRequest;
class Inset;

namespace frontend {

class GuiView;

/** \enum KernelDocType used to flag the different kinds of buffer
 *  without making the kernel header files available to the
 *  dialog's Controller or View.
 */
enum KernelDocType
{
	LATEX,
	LITERATE,
	DOCBOOK
};

/** \c Dialog collects the different parts of a Model-Controller-View
 *  split of a generic dialog together.
 */
class Dialog 
{
public:
	/// \param lv is the access point for the dialog to the LyX kernel.
	/// \param name is the identifier given to the dialog by its parent
	/// container.
	/// \param title is the window title used for decoration.
	Dialog(GuiView & lv, QString const & name, QString const & title);

	virtual ~Dialog();

	virtual QWidget * asQWidget() = 0;
	virtual QWidget const * asQWidget() const = 0;

	/// Session key.
	/**
	 * This key must be used for any session setting.
	 **/
	QString sessionKey() const;

	/// Save session settings.
	/**
	 * This default implementation saves the geometry state.
	 * Reimplement to save more settings.
	 **/
	virtual void saveSession() const;

	/// Restore session settings.
	/**
	 * This default implementation restores the geometry state.
	 * Reimplement to restore more settings.
	 **/
	virtual void restoreSession();

	/** \name Container Access
	 *  These methods are publicly accessible because they are invoked
	 *  by the parent container acting on commands from the LyX kernel.
	 */
	//@{
	/// \param data is a string encoding of the data to be displayed.
	/// It is passed to the Controller to be translated into a useable form.
	virtual void showData(std::string const & data);
	//@}

	/// \return inset at current cursor location.
	Inset const * inset(InsetCode code) const;

	/** Check whether we may apply our data.
	 *
	 *  The buttons are disabled if not and (re-)enabled if yes.
	 */
	virtual void checkStatus();

	/** When applying, it's useful to know whether the dialog is about
	 *  to close or not (no point refreshing the display for example).
	 */
	virtual bool isClosing() const { return false; }

	/** \c View part
	 *  of a Model-Controller-View split of a generic dialog.
	 *  These few methods are all that a generic dialog needs of a
	 *  view.
	 */
	//@{
	/** A request to modify the data structures stored by the
	 *  accompanying Controller in preparation for their dispatch to
	 *  the LyX kernel.
	 */
	virtual void applyView() = 0;

	/// Hide the dialog from sight
	void hideView();

	/// Prepare dialog and display it.
	void showView();

	/// Prepare dialog before view.
	void prepareView();

	/// Decide wether the dialog should grab thekeyboard focus when shown.
	/// This method defaults to true, override if a different behaviour
	/// is wanted.
	virtual bool wantInitialFocus() const { return true; }

	/// Update the display of the dialog whilst it is still visible.
	virtual void updateView() = 0;

	// Default Implementation does nothing.
	// Each dialog has to choose what control to enable or disable.
	virtual void enableView(bool /*enable*/) {}

	/// \return true if the dialog is visible.
	virtual bool isVisibleView() const;
	//@}

	/// Dialog identifier.
	QString name() const { return name_; }

	//@{
	/** Enable the controller to initialise its data structures.
	 *  \param data is a string encoding of the parameters to be displayed.
	 *  \return true if the translation was successful.
	 */
	virtual bool initialiseParams(std::string const & data) = 0;

	/// Enable the controller to clean up its data structures.
	virtual void clearParams() = 0;

	/// Enable the Controller to dispatch its data back to the LyX kernel.
	virtual void dispatchParams() = 0;

	/** \return true if the dialog should be updated when the
	 *  buffer has changed.
	 */
	virtual bool isBufferDependent() const = 0;

	/** \return true if the dialog should be shown only when
	 *  a buffer is open.
	 */
	virtual bool needBufferOpen() const = 0;

	/** \return true if the dialog can apply data also
	 *  for ReadOnly buffers.
	 *  This has to be distinguished from isBufferDependent()
	 */
	virtual bool canApplyToReadOnly() const { return false; }

	/** The lfun that is sent for applying the data.
	 *
	 * This method is used by the default implementation of canApply()
	 * for buffer dependent dialogs that send one lfun when applying the
	 * data.
	 * It should be used in dispatchParams(), too for consistency reasons.
	 *  \returns the lfun that is sent for applying the data.
	 */
	virtual FuncCode getLfun() const { return LFUN_INSET_APPLY; }

	/** Check whether we may apply our data.
	 *
	 * The default implementation works for all dialogs that send one
	 * lfun when applying the data. Dialogs that send none or more than
	 * one lfun need to reimplement it.
	 *  \returns whether the data can be applied or not.
	 */
	virtual bool canApply() const;

	/** \return true if the kernel should disconnect the dialog from
	 *  a particular inset after the data has been applied to it.
	 *  Clearly this makes sense only for dialogs modifying the contents
	 *  of an inset :-)
	 *  In practise, only a very few dialogs (e.g. the citation dialog)
	 *  return true.
	 */
	virtual bool disconnectOnApply() const { return false; }

	//@}

	/** \c Kernel part: a wrapper making the LyX kernel available to the dialog.
	 * (Ie, it provides an interface to the Model part of the Model-Controller-
	 *  View split.
	 *  In an ideal world, it will shrink as more info is passed to the
	 *  Dialog::show() and Dialog::update() methods.
	 */


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
	void updateDialog() const;

	/** A request from the Controller that future changes to the data
	 *  stored by the dialog are not applied to the inset currently
	 *  connected to the dialog. Instead, they will be used to generate
	 *  a new inset at the cursor position.
	 */
	void disconnect() const;

	/** \name Kernel Wrappers
	 *  Simple wrapper functions to Buffer methods.
	 */
	//@{
	bool isBufferAvailable() const;
	bool isBufferReadonly() const;
	QString bufferFilePath() const;
	//@}

	/// The type of the current buffer.
	KernelDocType docType() const;

	/** \name Kernel Nasties
	 *  Unpleasantly public internals of the LyX kernel.
	 *  We should aim to reduce/remove these from the interface.
	 */
	//@{
	GuiView const & lyxview() const { return lyxview_; }
	/// Current buffer
	Buffer const & buffer() const;
	/// Main document buffer
	Buffer const & documentBuffer() const;
	/// Current BufferView
	BufferView const * bufferview() const;
	//@}

protected:
	///
	void setTitle(QString const & title) { title_ = title; }
	///
	virtual void apply();

private:
	/** The Dialog's name is the means by which a dialog identifies
	 *  itself to the GuiView.
	 */
	QString const name_;
	///
	QString title_;
	///
	GuiView & lyxview_;

	/// intentionally unimplemented, therefore uncopiable
	Dialog(Dialog const &);
	void operator=(Dialog const &);
};


} // namespace frontend
} // namespace lyx

#endif // DIALOG_H
