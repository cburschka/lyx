/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Word Processor
 *
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-1999 The LyX Team. 
 *
 *           This file is Copyright 1996-1999
 *           Lars Gullik Bjønnes
 *
 * ====================================================== 
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include <sys/types.h>
#include <utime.h>
#include "bufferlist.h"
#include "lyx_main.h"
#include "minibuffer.h"
#include "support/FileInfo.h"
#include "support/filetools.h"
#include "lyx_gui_misc.h"
#include "lastfiles.h"
#include "debug.h"
#include "lyxrc.h"
#include "lyxscreen.h"
#include "lyxtext.h"
#include "lyx_cb.h"
#include "gettext.h"

extern BufferView * current_view;
extern MiniBuffer * minibuffer;
extern void SmallUpdate(signed char);
extern void BeforeChange();
extern int RunLinuxDoc(int, string const &);

//
// Class BufferStorage
//

void BufferStorage::release(Buffer * buf)
{
	for(Container::iterator it = container.begin();
	    it != container.end(); ++it) {
		if ((*it) == buf) {
			Buffer * tmpbuf = (*it);
			container.erase(it);
			delete tmpbuf;
			break;
		}
	}
}


Buffer * BufferStorage::newBuffer(string const & s,
				  LyXRC * lyxrc,
				  bool ronly)
{
	Buffer * tmpbuf = new Buffer(s, lyxrc, ronly);
	tmpbuf->params.useClassDefaults();
	lyxerr.debug() << "Assigning to buffer "
		       << container.size() + 1 << endl;
	container.push_back(tmpbuf);
	return tmpbuf;
}


//
// Class BufferList
//

BufferList::BufferList()
{
	_state = BufferList::OK;
}


bool BufferList::empty()
{
	return bstore.empty();
}


extern void MenuWrite(Buffer *);

bool BufferList::QwriteAll()
{
        bool askMoreConfirmation = false;
        string unsaved;
	for(BufferStorage::iterator it = bstore.begin();
	    it != bstore.end(); ++it) {
		if (!(*it)->isLyxClean()) {
			switch(AskConfirmation(_("Changes in document:"),
					       MakeDisplayPath((*it)->fileName(),
							       50),
					       _("Save document?"))) {
			case 1: // Yes
				MenuWrite((*it));
				break;
			case 2: // No
				askMoreConfirmation = true;
				unsaved += MakeDisplayPath((*it)->fileName(),
							   50);
				unsaved += "\n";
				break;
			case 3: // Cancel
				return false;
			}
		}
	}
        if (askMoreConfirmation &&
            lyxrc->exit_confirmation &&
            !AskQuestion(_("Some documents were not saved:"),
                         unsaved, _("Exit anyway?"))) {
                return false;
        }

        return true;
}


// Should probably be moved to somewhere else: BufferView? LyXView?
bool BufferList::write(Buffer * buf, bool makeBackup)
{
	minibuffer->Set(_("Saving document"),
			MakeDisplayPath(buf->fileName()), "...");

	// We don't need autosaves in the immediate future. (Asger)
	buf->resetAutosaveTimers();

	// make a backup
	if (makeBackup) {
		string s = buf->fileName() + '~';
		// Rename is the wrong way of making a backup,
		// this is the correct way.
		/* truss cp fil fil2:
		   lstat("LyXVC3.lyx", 0xEFFFF898)                 Err#2 ENOENT
		   stat("LyXVC.lyx", 0xEFFFF688)                   = 0
		   open("LyXVC.lyx", O_RDONLY)                     = 3
		   open("LyXVC3.lyx", O_WRONLY|O_CREAT|O_TRUNC, 0600) = 4
		   fstat(4, 0xEFFFF508)                            = 0
		   fstat(3, 0xEFFFF508)                            = 0
		   read(3, " # T h i s   f i l e   w".., 8192)     = 5579
		   write(4, " # T h i s   f i l e   w".., 5579)    = 5579
		   read(3, 0xEFFFD4A0, 8192)                       = 0
		   close(4)                                        = 0
		   close(3)                                        = 0
		   chmod("LyXVC3.lyx", 0100644)                    = 0
		   lseek(0, 0, SEEK_CUR)                           = 46440
		   _exit(0)
		*/

		// Should proabaly have some more error checking here.
		// Should be cleaned up in 0.13, at least a bit.
		// Doing it this way, also makes the inodes stay the same.
		// This is still not a very good solution, in particular we
		// might loose the owner of the backup.
		FileInfo finfo(buf->fileName());
		if (finfo.exist()) {
			mode_t fmode = finfo.getMode();
			struct utimbuf * times = new struct utimbuf;

			times->actime = finfo.getAccessTime();
			times->modtime = finfo.getModificationTime();
			long blksize = finfo.getBlockSize();
			lyxerr.debug() << "BlockSize: " << blksize << endl;
			FilePtr fin(buf->fileName(), FilePtr::read);
			FilePtr fout(s, FilePtr::truncate);
			if (fin() && fout()) {
				char * cbuf = new char[blksize+1];
				size_t c_read = 0;
				size_t c_write = 0;
				do {
					c_read = fread(cbuf, 1, blksize, fin);
					if (c_read != 0)
						c_write = 
							fwrite(cbuf, 1,
							       c_read, fout);
				} while (c_read);
				fin.close();
				fout.close();
				chmod(s.c_str(), fmode);
				
				if (utime(s.c_str(), times)) {
					lyxerr << "utime error." << endl;
				}
				delete [] cbuf;
			} else {
				lyxerr << "LyX was not able to make "
					"backupcopy. Beware." << endl;
			}
			delete[] times;
		}
	}
	
	if (buf->writeFile(buf->fileName(), false)) {
		buf->markLyxClean();

		minibuffer->Set(_("Document saved as"),
				MakeDisplayPath(buf->fileName()));

		// now delete the autosavefile
		string a = OnlyPath(buf->fileName());
		a += '#';
		a += OnlyFilename(buf->fileName());
		a += '#';
		FileInfo fileinfo(a);
		if (fileinfo.exist()) {
			if (remove(a.c_str()) != 0) {
				WriteFSAlert(_("Could not delete "
					       "auto-save file!"), a);
			}
		}
	} else {
		// Saving failed, so backup is not backup
		if (makeBackup) {
			string s = buf->fileName() + '~';
			rename(s.c_str(), buf->fileName().c_str());
		}
		minibuffer->Set(_("Save failed!"));
		return false;
	}

	return true;
}


void BufferList::closeAll()
{
	_state = BufferList::CLOSING;
	while (!bstore.empty()) {
		close(bstore.front());
	}
	_state = BufferList::OK;
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
        buf->InsetUnlock();
	
	if (buf->paragraph && !buf->isLyxClean() && !quitting) {
		ProhibitInput();
                switch(AskConfirmation(_("Changes in document:"),
				       MakeDisplayPath(buf->fileName(), 50),
				       _("Save document?"))){
		case 1: // Yes
			if (write(buf)) {
				lastfiles->newFile(buf->fileName());
			} else {
				AllowInput();
				return false;
			}
                        break;
		case 3: // Cancel
                        AllowInput();
                        return false;
                }
		AllowInput();
	}

	bstore.release(buf);
	return true;
}


void BufferList::makePup(int pup)
	/* This should be changed to return a char const *const
	   in the same way as for lastfiles.[hC]
	   */
{
	int ant = 0;
	for(BufferStorage::iterator it = bstore.begin();
	    it != bstore.end(); ++it) {
		string relbuf = MakeDisplayPath((*it)->fileName(), 30);
		fl_addtopup(pup, relbuf.c_str());
		++ant;
	}
	if (ant == 0) fl_addtopup(pup, _("No Documents Open!%t"));
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


void BufferList::updateInset(Inset * inset, bool mark_dirty)
{
	for (BufferStorage::iterator it = bstore.begin();
	     it != bstore.end(); ++it) {
		if ((*it)->getUser()
		    && (*it)->getUser()->text->UpdateInset(inset)) {
			if (mark_dirty)
				(*it)->markDirty();
			break;
		}
	}
}


int BufferList::unlockInset(UpdatableInset * inset)
{
	if (!inset) return 1;
	for(BufferStorage::iterator it = bstore.begin();
	    it != bstore.end(); ++it) {
		if ((*it)->the_locking_inset == inset) {
			(*it)->InsetUnlock();
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
			writefile += ChangeExtension((*it)->fileName(),
						     ".tex", true);
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
				   " document ")
			       << (*it)->fileName()
			       << _(" as...") << endl;
			
			for (int i = 0; i < 3 && !madeit; ++i) {
				string s;
				
				// We try to save three places:
				// 1) Same place as document.
				// 2) In HOME directory.
				// 3) In "/tmp" directory.
				if (i == 0) {
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
	Buffer * b = bstore.newBuffer(s, lyxrc, ronly);

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
				unlink(e.c_str());
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
					unlink(a.c_str());
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


bool BufferList::exists(string const & s)
{
	for (BufferStorage::iterator it = bstore.begin();
	     it != bstore.end(); ++it) {
		if ((*it)->fileName() == s)
			return true;
	}
	return false;
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


Buffer * BufferList::newFile(string const & name, string tname)
{
	/* get a free buffer */ 
	Buffer * b = bstore.newBuffer(name, lyxrc);

	// use defaults.lyx as a default template if it exists.
	if (tname.empty()) {
		tname = LibFileSearch("templates", "defaults.lyx");
	}
	if (!tname.empty() && IsLyXFilename(tname)){
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

	b->markDirty();
	b->setReadonly(false);
	
	return b;
}


Buffer * BufferList::loadLyXFile(string const & filename, bool tolastfiles)
{
	// make sure our path is absolute
	string s = MakeAbsPath(filename);

	// Is this done too early?
	// Is it LinuxDoc?
	if (IsSGMLFilename(s)) {
		FileInfo fi(s);
		if (fi.exist() && fi.readable()) {
			if (!RunLinuxDoc(-1, s)) {
				s = ChangeExtension (s, ".lyx", false);
			} else { // sgml2lyx failed
				WriteAlert(_("Error!"),
					   _("Could not convert file"), s);
				return 0;
			}
		} else {
			// just change the extension and it will be
			// handled like a regular lyx file that does
			// not exist.
			s = ChangeExtension(s, ".lyx", false);
		}
	}
	
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
		minibuffer->Set(_("File `")+MakeDisplayPath(s, 50)+
				_("' is read-only."));
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
			lyxerr << "Do you want to checkout?" << endl;
		}
		if (AskQuestion(_("Cannot open specified file:"), 
				MakeDisplayPath(s, 50),
				_("Create new document with this name?")))
			{
				// Find a free buffer
				b = newFile(s, string());
			}
		break;
	}

	if (b && tolastfiles)
		lastfiles->newFile(b->fileName());

	return b;
}
