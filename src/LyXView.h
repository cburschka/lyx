// -*- C++ -*-

#ifndef LYXVIEW_BASE_H
#define LYXVIEW_BASE_H

#ifdef __GNUG__
#pragma interface
#endif

#include <boost/utility.hpp>
#include <sigc++/signal_system.h>

#include "LString.h"
#include "frontends/Timeout.h"
#include "layout.h"

class Buffer;
class Toolbar;
class MiniBuffer;
class Intl;
class Menubar;

class BufferView;
class Dialogs;
class LyXFunc;

///
class LyXView : public SigC::Object, boost::noncopyable {
public:
	///
	LyXView();
	///
	virtual ~LyXView();
	///
	virtual void init() = 0;
	///
	virtual void setPosition(int, int) = 0;
	///
	virtual void show(int, int, string const &) = 0;
	/// Redraw the main form.
	virtual void redraw() = 0;

	/// Resize all BufferViews in this LyXView (because the width changed)
	void resize();

	/// returns the buffer currently shown in the main form.
	Buffer * buffer() const;

	///
	BufferView * view() const;

	/// return a pointer to the toolbar
	Toolbar * getToolbar() const;

	/// sets the layout in the toolbar layout combox
	void setLayout(LyXTextClass::size_type layout);

	/// update the toolbar
	void updateToolbar();

	/// return a pointer to the lyxfunc
	LyXFunc * getLyXFunc() const;

	/// return a pointer to the minibuffer
	MiniBuffer * getMiniBuffer() const;

	///
	void message(string const &);
	///
	void messagePush(string const & str);
	///
	void messagePop();
	
	///
	Menubar * getMenubar() const;

	///
	void updateMenubar();

	///
	Intl * getIntl() const;

	///
	Dialogs * getDialogs() { return dialogs_; }

	///
	void updateLayoutChoice();

	/// Updates the title of the window
	void updateWindowTitle();

	/// Show state (toolbar and font in minibuffer)
	void showState();

	/// Reset autosave timer
	void resetAutosaveTimer();
	///
	virtual void prohibitInput() const = 0;
	///
	virtual void allowInput() const = 0;
protected:
	///
	Menubar * menubar;
	/// 
	Toolbar * toolbar;
	/** This is supposed to be a pointer or a list of pointers to the
	   BufferViews currently being shown in the LyXView. So far
	   this is not used, but that should change pretty soon. (Lgb) */
	BufferView * bufferview;
	/// 
	MiniBuffer * minibuffer;
	///
	Intl * intl;
	///
	Timeout autosave_timeout;
	/// A callback
	void AutoSave();
	///
	void invalidateLayoutChoice();
private:
	/// 
	LyXFunc * lyxfunc;
	///
	Dialogs * dialogs_;
	///
	virtual void setWindowTitle(string const &) = 0;
	/** The last textclass layout list in the layout choice selector
	  This should probably be moved to the toolbar, but for now it's
	here. (Asger) */
	int last_textclass;
};
#endif
