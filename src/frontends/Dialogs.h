// -*- C++ -*-
/**
 * \file Dialogs.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef DIALOGS_H
#define DIALOGS_H

#include <boost/signal.hpp>
#include <boost/utility.hpp>

#include <map>

namespace lyx {

class Inset;
class LyXView;

namespace frontend { class Dialog; }

/** Container of all dialogs.
 */
class Dialogs : boost::noncopyable {
public:
	///
	Dialogs(LyXView &);

	/** Check the status of all visible dialogs and disable or reenable
	 *  them as appropriate.
	 *
	 *  Disabling is needed for example when a dialog is open and the
	 *  cursor moves to a position where the corresponding inset is not
	 *  allowed.
	 */
	void checkStatus();

	/// Are the tooltips on or off?
	static bool tooltipsEnabled();

	/// Hide all visible dialogs
	void hideAll() const;
	/// Hide any dialogs that require a buffer for them to operate
	void hideBufferDependent() const;
	/** Update visible, buffer-dependent dialogs
	    If the bool is true then a buffer change has occurred
	    else its still the same buffer.
	 */
	void updateBufferDependent(bool) const ;

	/** \param name == "about" etc; an identifier used to
	    launch a particular dialog.
	    \param data is a string encoding of the data used to populate
	    the dialog. Several of these dialogs do not need any data,
	    so it defaults to string().
	*/
	void show(std::string const & name, std::string const & data = std::string());

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
	void show(std::string const & name, std::string const & data, Inset * inset);

	/** \param name == "citation", "bibtex" etc; an identifier used
	    to update the contents of a particular dialog with \param data.
	    See the comments to 'show', above.
	*/
	void update(std::string const & name, std::string const & data);

	/// Is the dialog currently visible?
	bool visible(std::string const & name) const;

	/** All Dialogs of the given \param name will be closed if they are
	    connected to the given \param inset.
	*/
	void hide(std::string const & name, Inset * inset);
	///
	void disconnect(std::string const & name);
	///
	Inset * getOpenInset(std::string const & name) const;
private:
	///
	void redraw() const;
	///
	bool isValidName(std::string const & name) const;
	///
	frontend::Dialog * find_or_build(std::string const & name);
	///
	typedef boost::shared_ptr<frontend::Dialog> DialogPtr;
	///
	DialogPtr build(std::string const & name);

	///
	LyXView & lyxview_;
	///
	std::map<std::string, Inset *> open_insets_;

	///
	std::map<std::string, DialogPtr> dialogs_;

	/// flag against a race condition due to multiclicks in Qt frontend, see bug #1119
	bool in_show_;

	///
	boost::signals::connection connection_;
};

} // namespace lyx

#endif
