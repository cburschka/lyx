/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Word Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team. 
 *
 *           This file is Copyright 1996-2000
 *           Lars Gullik Bjønnes
 *
 * ====================================================== 
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include <algorithm>

#include "bufferlist.h"
#include "lyx_main.h"
#include "minibuffer.h"
#include "support/FileInfo.h"
#include "support/filetools.h"
#include "lyx_gui_misc.h"
#include "lastfiles.h"
#include "debug.h"
#include "lyxrc.h"
#include "lyxtext.h"
#include "lyx_cb.h"
#include "bufferview_funcs.h"
#include "gettext.h"
#include "LyXView.h"
#include "vc-backend.h"
#include "TextCache.h"

extern BufferView * current_view; // called too many times in this file...

using std::vector;
using std::find;
using std::endl;

//
// Class BufferStorage
//

void BufferStorage::release(Buffer * buf)
{
	Container::iterator it = find(container.begin(), container.end(), buf);
	if (it != container.end()) {
		// Make sure that we don't store a LyXText in
		// the textcache that points to the buffer
		// we just deleted.
		Buffer * tmp = (*it);
		container.erase(it);
		textcache.removeAllWithBuffer(tmp);
		delete tmp;
	}
}


Buffer * BufferStorage::newBuffer(string const & s, bool ronly)
{
	Buffer * tmpbuf = new Buffer(s, ronly);
	tmpbuf->params.useClassDefaults();
	lyxerr.debug() << "Assigning to buffer "
		       << container.size() << endl;
	container.push_back(tmpbuf);
	return tmpbuf;
}


//
// Class BufferList
//

BufferList::BufferList()
	: state_(BufferList::OK)
{}


bool BufferList::empty() const
{
	return bstore.empty();
}


extern bool MenuWrite(Buffer *);
extern bool MenuWriteAs(Buffer *);

bool BufferList::QwriteAll()
{
        bool askMoreConfirmation = false;
        string unsaved;
	for(BufferStorage::iterator it = bstore.begin();
	    it != bstore.end(); ++it) {
		if (!(*it)->isLyxClean()) {
			string fname;
			if ((*it)->isUnnamed())
				fname = OnlyFilename((*it)->fileName());
			else
				fname = MakeDisplayPath((*it)->fileName(), 50);
			bool reask = true;
			while(reask) {
				switch(AskConfirmation(_("Changes in document:"),
						       fname,
						       _("Save document?"))) {
				case 1: // Yes
					if ((*it)->isUnnamed())
						reask = !MenuWriteAs((*it));
					else {
						reask = !MenuWrite((*it));
					}
					break;
				case 2: // No
					// if we crash after this we could
					// have no autosave file but I guess
					// this is really inprobable (Jug)
					if ((*it)->isUnnamed()) {
						removeAutosaveFile((*it)->fileName());
					}
					askMoreConfirmation = true;
					unsaved += MakeDisplayPath(fname, 50);
					unsaved += "\n";
					reask = false;
					break;
				case 3: // Cancel
					return false;
				}
			}
		}
	}
        if (askMoreConfirmation &&
            lyxrc.exit_confirmation &&
            !AskQuestion(_("Some documents were not saved:"),
                         unsaved, _("Exit anyway?"))) {
                return false;
        }

        return true;
}


void BufferList::closeAll()
{
	state_ = BufferList::CLOSING;
	// Since we are closing we can just as well delete all
	// in the textcache this will also speed the closing/quiting up a bit.
	textcache.clear();
	
	while (!bstore.empty()) {
		close(bstore.front());
	}
	state_ = BufferList::OK;
}


void BufferList::resize()
{
	for(BufferStorage::iterator it = bstore.begin();
	    it != bstore.end(); ++it) {
		(*it)->resize();
	}
}


bool BufferList::close(Buffer * buf)
{
	// CHECK
	// Trace back why we need to use buf->getUser here.
	// Perhaps slight rewrite is in order? (Lgb)
	
        if (buf->getUser()) buf->getUser()->insetUnlock();
	if (buf->paragraph && !buf->isLyxClean() && !quitting) {
		if (buf->getUser())
			ProhibitInput(buf->getUser());
		string fname;
		if (buf->isUnnamed())
			fname = OnlyFilename(buf->fileName());
		else
			fname = MakeDisplayPath(buf->fileName(), 50);
		bool reask = true;
		while(reask) {
			switch(AskConfirmation(_("Changes in document:"),
					       fname,
					       _("Save document?"))){
			case 1: // Yes
				if (buf->isUnnamed())
					reask = !MenuWriteAs(buf);
				else if (buf->save()) {
					lastfiles->newFile(buf->fileName());
				} else {
					if (buf->getUser())
						AllowInput(buf->getUser());
					return false;
				}
				break;
			case 2:
				if (buf->isUnnamed()) {
					removeAutosaveFile(buf->fileName());
				}
				reask = false;
				break;
			case 3: // Cancel
				if (buf->getUser())
					AllowInput(buf->getUser());
				return false;
			}
		}
		if (buf->getUser())
			AllowInput(buf->getUser());
	}

	bstore.release(buf);
	return true;
}


vector<string> BufferList::getFileNames() const
{
	vector<string> nvec;
	for(BufferStorage::const_iterator cit = bstore.begin();
	    cit != bstore.end(); ++cit) {
		nvec.push_back((*cit)->fileName());
	}
	return nvec;
}


Buffer * BufferList::first()
{
	if (bstore.empty()) return 0;
	return bstore.front();
}


Buffer * BufferList::getBuffer(int choice)
{
	if (choice >= bstore.size()) return 0;
	return bstore[choice];
}


int BufferList::unlockInset(UpdatableInset * inset)
{
	if (!inset) return 1;
	for(BufferStorage::iterator it = bstore.begin();
	    it != bstore.end(); ++it) {
		if ((*it)->getUser()
		    && (*it)->getUser()->the_locking_inset == inset) {
			(*it)->getUser()->insetUnlock();
			return 0;
		}
	}
	return 1;
}


void BufferList::updateIncludedTeXfiles(string const & mastertmpdir)
{
	for(BufferStorage::iterator it = bstore.begin();
	    it != bstore.end(); ++it) {
		if (!(*it)->isDepClean(mastertmpdir)) {
			string writefile = mastertmpdir;
			writefile += '/';
			writefile += (*it)->getLatexName();
			(*it)->makeLaTeXFile(writefile, mastertmpdir,
					     false, true);
			(*it)->markDepClean(mastertmpdir);
		}
	}
}


void BufferList::emergencyWriteAll()
{
	for (BufferStorage::iterator it = bstore.begin();
	     it != bstore.end(); ++it) {
		if (!(*it)->isLyxClean()) {
			bool madeit = false;
			
			lyxerr <<_("lyx: Attempting to save"
				   " document ");
			if ((*it)->isUnnamed())
			    lyxerr << OnlyFilename((*it)->fileName());
			else
			    lyxerr << (*it)->fileName();
			lyxerr << _(" as...") << endl;
			
			for (int i = 0; i < 3 && !madeit; ++i) {
				string s;
				
				// We try to save three places:
				// 1) Same place as document.
				// 2) In HOME directory.
				// 3) In "/tmp" directory.
				if (i == 0) {
					if ((*it)->isUnnamed())
						continue;
					s = (*it)->fileName();
				} else if (i == 1) {
					s = AddName(GetEnvPath("HOME"),
						    (*it)->fileName());
				} else {
					// MakeAbsPath to prepend the current
					// drive letter on OS/2
					s = AddName(MakeAbsPath("/tmp/"),
						    (*it)->fileName());
				}
				s += ".emergency";
				
				lyxerr << "  " << i + 1 << ") " << s << endl;
				
				if ((*it)->writeFile(s, true)) {
					(*it)->markLyxClean();
					lyxerr << _("  Save seems successful. "
						    "Phew.") << endl;
					madeit = true;
				} else if (i != 2) {
					lyxerr << _("  Save failed! Trying...")
					       << endl;
				} else {
					lyxerr << _("  Save failed! Bummer. "
						    "Document is lost.")
					       << endl;
				}
			}
		}
	}
}


Buffer * BufferList::readFile(string const & s, bool ronly)
{
	Buffer * b = bstore.newBuffer(s, ronly);

	string ts = s;
	string e = OnlyPath(s);
	string a = e;
	// File information about normal file
	FileInfo fileInfo2(s);

	// Check if emergency save file exists and is newer.
	e += OnlyFilename(s) + ".emergency";
	FileInfo fileInfoE(e);

	bool use_emergency = false;

	if (fileInfoE.exist() && fileInfo2.exist()) {
		if (fileInfoE.getModificationTime()
		    > fileInfo2.getModificationTime()) {
			if (AskQuestion(_("An emergency save of this document exists!"),
					MakeDisplayPath(s, 50),
					_("Try to load that instead?"))) {
				ts = e;
				// the file is not saved if we load the
				// emergency file.
				b->markDirty();
				use_emergency = true;
			} else {
				// Here, we should delete the emergency save
				::unlink(e.c_str());
			}
		}
	}

	if (!use_emergency) {
		// Now check if autosave file is newer.
		a += '#';
		a += OnlyFilename(s);
		a += '#';
		FileInfo fileInfoA(a);
		if (fileInfoA.exist() && fileInfo2.exist()) {
			if (fileInfoA.getModificationTime()
			    > fileInfo2.getModificationTime()) {
				if (AskQuestion(_("Autosave file is newer."),
						MakeDisplayPath(s, 50),
						_("Load that one instead?"))) {
					ts = a;
					// the file is not saved if we load the
					// autosave file.
					b->markDirty();
				} else {
					// Here, we should delete the autosave
					::unlink(a.c_str());
				}
			}
		}
	}
	// not sure if this is the correct place to begin LyXLex
	LyXLex lex(0, 0);
	lex.setFile(ts);
	if (b->readFile(lex))
		return b;
	else {
		bstore.release(b);
		return 0;
	}
}


bool BufferList::exists(string const & s) const
{
	for (BufferStorage::const_iterator cit = bstore.begin();
	     cit != bstore.end(); ++cit) {
		if ((*cit)->fileName() == s)
			return true;
	}
	return false;
}


bool BufferList::isLoaded(Buffer const * b) const
{
	BufferStorage::const_iterator cit =
		find(bstore.begin(), bstore.end(), b);
	return cit != bstore.end();
}


Buffer * BufferList::getBuffer(string const & s)
{
	for(BufferStorage::iterator it = bstore.begin();
	    it != bstore.end(); ++it) {
		if ((*it)->fileName() == s)
			return (*it);
	}
	return 0;
}


Buffer * BufferList::newFile(string const & name, string tname, bool isNamed)
{
	// get a free buffer
	Buffer * b = bstore.newBuffer(name);

	// use defaults.lyx as a default template if it exists.
	if (tname.empty()) {
		tname = LibFileSearch("templates", "defaults.lyx");
	}
	if (!tname.empty() && IsLyXFilename(tname)) {
		bool templateok = false;
		LyXLex lex(0, 0);
		lex.setFile(tname);
		if (lex.IsOK()) {
			if (b->readFile(lex)) {
				templateok = true;
			}
		}
		if (!templateok) {
			WriteAlert(_("Error!"), _("Unable to open template"), 
				   MakeDisplayPath(tname));
			// no template, start with empty buffer
			b->paragraph = new LyXParagraph;
		}
	}
	else {  // start with empty buffer
		b->paragraph = new LyXParagraph;
	}

#warning Why mark a new document dirty? I deactivate this (Jug)
	if (!lyxrc.new_ask_filename) {
//		b->markDirty();
		if (!isNamed)
			b->setUnnamed();
	}
	b->setReadonly(false);
	
	return b;
}


Buffer * BufferList::loadLyXFile(string const & filename, bool tolastfiles)
{
	// make sure our path is absolute
	string s = MakeAbsPath(filename);

	// file already open?
	if (exists(s)) {
		if (AskQuestion(_("Document is already open:"), 
				MakeDisplayPath(s, 50),
				_("Do you want to reload that document?"))) {
			// Reload is accomplished by closing and then loading
			if (!close(getBuffer(s))) {
				return 0;
			}
			// Fall through to new load. (Asger)
		} else {
			// Here, we pretend that we just loaded the 
			// open document
			return getBuffer(s);
		}
	}
	Buffer * b = 0;
	bool ro = false;
	switch (IsFileWriteable(s)) {
	case 0:
#if 0
		current_view->owner()->getMiniBuffer()->
			Set(_("File `") + MakeDisplayPath(s, 50) +
			    _("' is read-only."));
#endif
		ro = true;
		// Fall through
	case 1:
		b = readFile(s, ro);
		if (b) {
			b->lyxvc.file_found_hook(s);
		}
		break; //fine- it's r/w
	case -1:
		// Here we probably should run
		if (LyXVC::file_not_found_hook(s)) {
			// Ask if the file should be checked out for
			// viewing/editing, if so: load it.
		        if (AskQuestion(_("Do you want to retrieve file under version control?"))) {
				// How can we know _how_ to do the checkout?
				// With the current VC support it has to be,
				// a RCS file since CVS do not have special ,v files.
				RCS::retrive(s);
				return loadLyXFile(filename, tolastfiles);
			}
		}
		if (AskQuestion(_("Cannot open specified file:"), 
				MakeDisplayPath(s, 50),
				_("Create new document with this name?")))
			{
				// Find a free buffer
				b = newFile(s, string(), true);
			}
		break;
	}

	if (b && tolastfiles)
		lastfiles->newFile(b->fileName());

	return b;
}
