/**
 * \file frontends/Dialogs.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 *
 * Common to all frontends' Dialogs
 */

#include <config.h>

#include "Dialogs.h"
#include "controllers/Dialog.h"
#include <boost/signals/signal2.hpp>
#include <boost/bind.hpp>


// Note that static boost signals break some compilers, so this wrapper
// initialises the signal dynamically when it is first invoked.
template<typename Signal>
class BugfixSignal {
public:
	Signal & operator()() { return thesignal(); }
	Signal const & operator()() const { return thesignal(); }

private:
	Signal & thesignal() const
	{
		if (!signal_.get())
			signal_.reset(new Signal);
		return *signal_;
	}

	mutable boost::scoped_ptr<Signal> signal_;
};


boost::signal0<void> & Dialogs::redrawGUI()
{
	static BugfixSignal<boost::signal0<void> > thesignal;
	return thesignal();
}


namespace {

BugfixSignal<boost::signal2<void, string const &, InsetBase*> > hideSignal;

}


void Dialogs::hide(string const & name, InsetBase* inset)
{
	hideSignal()(name, inset);
}


Dialogs::Dialogs(LyXView & lyxview)
	: lyxview_(lyxview)
{
	// Connect signals
	redrawGUI().connect(boost::bind(&Dialogs::redraw, this));
	hideSignal().connect(boost::bind(&Dialogs::hideSlot, this, _1, _2));

	// All this is slated to go
	init_pimpl();
	// reduce the number of connections needed in
	// dialogs by a simple connection here.
	hideAllSignal.connect(hideBufferDependentSignal);
}


Dialog * Dialogs::find(string const & name)
{
	if (!isValidName(name))
		return 0;

	std::map<string, DialogPtr>::iterator it =
		dialogs_.find(name);

	if (it == dialogs_.end()) {
		dialogs_[name] = DialogPtr(build(name));
		return dialogs_[name].get();
	}

	return it->second.get();
}


void Dialogs::show(string const & name, string const & data)
{
	Dialog * dialog = find(name);
	if (!dialog)
		return;

	// FIXME! Should check that the dialog is NOT an inset dialog.
	dialog->show(data);
}


void Dialogs::show(string const & name, string const & data, InsetBase * inset)
{
	Dialog * dialog = find(name);
	if (!dialog)
		return;

	// FIXME! Should check that the dialog IS an inset dialog.
	dialog->show(data);
	open_insets_[name] = inset;
}


bool Dialogs::visible(string const & name) const
{
	std::map<string, DialogPtr>::const_iterator it =
		dialogs_.find(name);
	if (it == dialogs_.end())
		return false;
	return it->second.get()->isVisible();
}


void Dialogs::update(string const & name, string const & data)
{
	Dialog * dialog = find(name);
	if (!dialog)
		return;

	if (dialog->isVisible())
		dialog->update(data);
}


void Dialogs::hideSlot(string const & name, InsetBase * inset)
{
	Dialog * dialog = find(name);
	if (!dialog)
		return;

	if (inset && inset != getOpenInset(name))
		return;

	if (dialog->isVisible())
		dialog->hide();
	open_insets_[name] = 0;
}


void Dialogs::disconnect(string const & name)
{
	if (!isValidName(name))
		return;

	open_insets_[name] = 0;
}


InsetBase * Dialogs::getOpenInset(string const & name) const
{
	if (!isValidName(name))
		return 0;

	std::map<string, InsetBase *>::const_iterator it =
		open_insets_.find(name);
	return it == open_insets_.end() ? 0 : it->second;
}


void Dialogs::hideAll() const
{
	std::map<string, DialogPtr>::const_iterator it  = dialogs_.begin();
	std::map<string, DialogPtr>::const_iterator end = dialogs_.end();

	for(; it != end; ++it) {
		it->second->hide();
	}
	hideAllSignal();
}


void Dialogs::hideBufferDependent() const
{
	std::map<string, DialogPtr>::const_iterator it  = dialogs_.begin();
	std::map<string, DialogPtr>::const_iterator end = dialogs_.end();

	for(; it != end; ++it) {
		Dialog * dialog =  it->second.get();
		if (dialog->controller().isBufferDependent())
			dialog->hide();
	}
	hideBufferDependentSignal();
}


void Dialogs::updateBufferDependent(bool switched) const
{
	std::map<string, DialogPtr>::const_iterator it  = dialogs_.begin();
	std::map<string, DialogPtr>::const_iterator end = dialogs_.end();

	for(; it != end; ++it) {
		Dialog * dialog =  it->second.get();
		if (switched && dialog->controller().isBufferDependent()) {
			dialog->hide();
		} else {
			// A bit clunky, but the dialog will request
			// that the kernel provides it with the necessary
			// data.
			dialog->RestoreButton();
		}
	}
	updateBufferDependentSignal(switched);
}


void Dialogs::redraw() const
{
	std::map<string, DialogPtr>::const_iterator it  = dialogs_.begin();
	std::map<string, DialogPtr>::const_iterator end = dialogs_.end();

	for(; it != end; ++it) {
		it->second->redraw();
	}
}
