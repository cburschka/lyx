// -*- C++ -*-
/**
 * \file FuncRequest.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FUNCREQUEST_H
#define FUNCREQUEST_H

#include "FuncCode.h"

#include "support/docstring.h"

#include "frontends/KeyModifier.h"
#include "frontends/mouse_state.h"


namespace lyx {

class LyXErr;

namespace frontend {
	class GuiView;
}

/**
 * This class encapsulates a LyX action and its argument
 * in order to pass it around easily.
 */
class FuncRequest
{
public:
	/// Where the request came from
	enum Origin {
		INTERNAL,
		MENU, // A menu entry
		TOOLBAR, // A toolbar icon
		KEYBOARD, // a keyboard binding
		COMMANDBUFFER,
		LYXSERVER,
		TOC
	};

	/// just for putting these things in std::container
	explicit FuncRequest(Origin o = INTERNAL);
	/// actions without extra argument
	explicit FuncRequest(FuncCode act, Origin o = INTERNAL);
	/// actions without extra argument
	FuncRequest(FuncCode act, int x, int y, mouse_button::state button,
		    KeyModifier modifier, Origin o = INTERNAL);
	/// actions with extra argument
	FuncRequest(FuncCode act, docstring const & arg,
		    Origin o = INTERNAL);
	/// actions with extra argument. FIXME: remove this
	FuncRequest(FuncCode act, std::string const & arg,
		    Origin o = INTERNAL);
	/// for changing requests a bit
	FuncRequest(FuncRequest const & cmd, docstring const & arg,
		    Origin o = INTERNAL);

	/// access the whole argument
	docstring const & argument() const { return argument_; }
	///
	FuncCode action() const { return action_ ; }
	///
	void setAction(FuncCode act) { action_ = act; }
	///
	Origin origin() const { return origin_; }
	///
	void setOrigin(Origin o) { origin_ = o; }
	///
	frontend::GuiView* view_origin() const { return view_origin_; }
	///
	void setViewOrigin(frontend::GuiView* o) { view_origin_ = o; }
	///
	int x() const { return x_; }
	///
	int y() const { return y_; }
	///
	void set_y(int y) { y_ = y; }
	///
	mouse_button::state button() const { return button_; }
	///
	KeyModifier modifier() const { return modifier_; }

	/// argument parsing, extract argument i as std::string
	std::string getArg(unsigned int i) const;
	/// argument parsing, extract argument i as std::string,
	/// eating all characters up to the end of the command line
	std::string getLongArg(unsigned int i) const;

	///
	static FuncRequest const unknown;
	///
	static FuncRequest const noaction;
	///
	bool allowAsync() const { return allow_async_; }
	///
	void allowAsync(bool allow_async) { allow_async_ = allow_async; }

private:
	/// the action
	FuncCode action_ = LFUN_NOACTION;
	/// the action's string argument
	docstring argument_;
	/// who initiated the action
	Origin origin_ = INTERNAL;
	/// to which view should be this command sent (see bug #11004)
	/// NULL=current view
	frontend::GuiView* view_origin_ = nullptr;
	/// the x coordinate of a mouse press
	int x_ = 0;
	/// the y coordinate of a mouse press
	int y_ = 0;
	/// some extra information (like button number)
	mouse_button::state button_ = mouse_button::none;
	///
	KeyModifier modifier_ = NoModifier;
	/// Commands should be run synchronously when they
	/// are launched via "command-sequence" or "repeat" or "buffer-forall"
	bool allow_async_ = true;
};


bool operator==(FuncRequest const & lhs, FuncRequest const & rhs);

std::ostream & operator<<(std::ostream &, FuncRequest const &);

LyXErr & operator<<(LyXErr &, FuncRequest const &);


} // namespace lyx

#endif // FUNCREQUEST_H
