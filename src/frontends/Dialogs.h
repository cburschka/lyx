// -*- C++ -*-
/**
 * \file Dialogs.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 * \author Allan Rae
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef DIALOGS_H
#define DIALOGS_H

#include <boost/utility.hpp>
#include <boost/signals/signal0.hpp>
#include <boost/signals/signal1.hpp>

#include "support/std_string.h"


class Dialog;
class InsetBase;
class LyXView;

/** Container of all dialogs and signals a LyXView needs or uses to access them
    The list of dialog signals isn't comprehensive but should be a good guide
    for any future additions.  Remember don't go overboard -- think minimal.
 */
class Dialogs : boost::noncopyable
{
public:
	///
	Dialogs(LyXView &);
	///
	~Dialogs();

	/** Redraw all visible dialogs because, for example, the GUI colours
	 *  have been re-mapped.
	 *
	 *  Note that static boost signals break some compilers, so we return a
	 *  reference to some hidden magic ;-)
	 */
	static boost::signal0<void> & redrawGUI();

	/// Toggle tooltips on/off in all dialogs.
	static void toggleTooltips();

	/// Are the tooltips on or off?
	static bool tooltipsEnabled();

	/// Signals slated to go
	//@{
	boost::signal0<void> hideAllSignal;
	boost::signal0<void> hideBufferDependentSignal;
	boost::signal1<void, bool> updateBufferDependentSignal;
	//@}

	/// Hide all visible dialogs
	void hideAll() const;
	/// Hide any dialogs that require a buffer for them to operate
	void hideBufferDependent() const;
	/** Update visible, buffer-dependent dialogs
	    If the bool is true then a buffer change has occurred
	    else its still the same buffer.
	 */
	void updateBufferDependent(bool) const ;

	/**@name Dialog Access Signals.
	   Put into some sort of alphabetical order */
	//@{
	///
	void showDocument();
	/// show all forked child processes
	void showForks();
	///
	void showPreamble();
	///
	void showPreferences();
	///
	void showPrint();
	///
	void showSearch();
	///
	void showSendto();
	/// bring up the spellchecker
	void showSpellchecker();
	//@}

	/** \param name == "about" etc; an identifier used to
	    launch a particular dialog.
	    \param data is a string encoding of the data used to populate
	    the dialog. Several of these dialogs do not need any data,
	    so it defaults to string().
	*/
	void show(string const & name, string const & data = string());

	/** \param name == "bibtex", "citation" etc; an identifier used to
	    launch a particular dialog.
	    \param data is a string representation of the Inset contents.
	    It is often little more than the output from Inset::write.
	    It is passed to, and parsed by, the frontend dialog.
	    \param inset is _not_ passed to the frontend dialog.
	    It is stored internally and used by the kernel to ascertain
	    what to do with the FuncRequest dispatched from the frontend
	    dialog on 'Apply'; should it be used to create a new inset at
	    the current cursor position or modify an existing, 'open' inset?
	*/
	void show(string const & name, string const & data, InsetBase * inset);

	/** \param name == "citation", "bibtex" etc; an identifier used
	    to update the contents of a particular dialog with \param data .
	    See the comments to 'show', above.
	*/
	void update(string const & name, string const & data);

	/// is the dialog currently visible?
	bool visible(string const & name) const;

	/** All Dialogs of the given \param name will be closed if they are
	    connected to the given \param inset.
	*/
	static void hide(string const & name, InsetBase * inset);
	///
	void disconnect(string const & name);
	///
	InsetBase * getOpenInset(string const & name) const;
private:
	///
	void hideSlot(string const & name, InsetBase * inset);
	///
	void redraw() const;
	///
	bool isValidName(string const & name) const;
	///
	Dialog * find_or_build(string const & name);
	///
	Dialog * build(string const & name);

	///
	LyXView & lyxview_;
	///
	std::map<string, InsetBase *> open_insets_;

	///
	typedef boost::shared_ptr<Dialog> DialogPtr;
	///
	std::map<string, DialogPtr> dialogs_;

	/// the stuff below is slated to go...
	void init_pimpl();
	///
	class Impl;
	///
	Impl * pimpl_;
};

#endif
