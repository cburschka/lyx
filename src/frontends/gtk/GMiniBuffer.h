// -*- C++ -*-
/**
 * \file GMiniBuffer.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */


#ifndef GMINI_BUFFER_H
#define GMINI_BUFFER_H

#include "frontends/Timeout.h"

class ControlCommandBuffer;

class GMiniBuffer : public SigC::Object
{
public:
	GMiniBuffer(GView * view, ControlCommandBuffer & control);
	~GMiniBuffer();
	void message(std::string const & str);
	/// go into edit mode
	void editMode();
private:
	bool onKeyPress(GdkEventKey * event);
	bool onListKeyPress(GdkEventKey * event);
	void onCommit();
	bool onListFocusIn(GdkEventFocus * event);
	bool onFocusIn(GdkEventFocus * event);
	bool onFocusOut(GdkEventFocus * event);
	void focusTimeout();
	void onSelected();
	/// Are we in edit mode?
	bool isEditMode() const;
	/// reset buffer to stored input text
	void infoTimeout();
	/// go back to "at rest" message
	void idleTimeout();
	/// go into message mode
	void messageMode();
	/// show a temporary message whilst in edit mode
	void showInfo(Glib::ustring const & info, bool append = true);
	void setInput(Glib::ustring const & input);
	ControlCommandBuffer & controller_;
	GView * view_;
	Gtk::Entry entry_;
	/// info timer
	boost::scoped_ptr<Timeout> infoTimer_;
	boost::signals::connection infoCon_;
	/// idle timer
	boost::scoped_ptr<Timeout> idleTimer_;
	boost::signals::connection idleCon_;
	Glib::ustring storedInput_;
	/// are we showing an informational temporary message ?
	bool infoShown_;
	boost::scoped_ptr<Timeout> focusTimer_;

	Gtk::ScrolledWindow scrolledWindow_;
	Gtk::TreeModelColumn<Glib::ustring> listCol_;
	Gtk::TreeModel::ColumnRecord listCols_;
	Glib::RefPtr<Gtk::ListStore> listStore_;
	Gtk::TreeView listView_;
	Glib::RefPtr<Gtk::TreeSelection> listSel_;
};

#endif
