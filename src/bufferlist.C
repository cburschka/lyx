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
 *======================================================
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
#include "error.h"
#include "lyxrc.h"
#include "lyxscreen.h"
#include "lyxtext.h"
#include "lyx_cb.h"
#include "gettext.h"

extern BufferView *current_view;
extern MiniBuffer *minibuffer;
extern void SmallUpdate(signed char);
extern void BeforeChange();
extern int RunLinuxDoc(int, string const &);

//
// Class BufferStorage
//

BufferStorage::BufferStorage()
{
	// Initialize the buffer array
	for (int i=NUMBER_OF_BUFFERS-1; i >=0; i--) {
		buffer[i] = 0;
	}	
}


bool BufferStorage::isEmpty()
{
	for (int i=NUMBER_OF_BUFFERS-1; i >=0; i--) {
		if (buffer[i]) return false;
	}
	return true;
}


void BufferStorage::release(Buffer* buf)
{
	int i=0;
	for (i=0; i<NUMBER_OF_BUFFERS; i++)
		if (buffer[i] == buf) break;
	Buffer *tmpbuf = buffer[i];
	buffer[i] = 0;
	delete tmpbuf;
}


Buffer* BufferStorage::newBuffer(string const &s,
				 LyXRC *lyxrc,
				 bool ronly)
{
	int i=0;
	while (i < NUMBER_OF_BUFFERS - 1
	       && buffer[i]) i++;
	buffer[i] = new Buffer(s, lyxrc, ronly);
	buffer[i]->params.useClassDefaults();
	lyxerr.debug(string("Assigning to buffer ") + tostr(i), Error::ANY);
	return buffer[i];
}


//
// Class BufferStrorage_Iter
//

Buffer* BufferStorage_Iter::operator() ()
{
	int i=0;
	for (i=index; i < BufferStorage::NUMBER_OF_BUFFERS; i++) {
		if (cs->buffer[i]) {
			index = i+1;
			return cs->buffer[i];
		}
	}
	return 0;	
}


Buffer* BufferStorage_Iter::operator[] (int a)
{
	// a is >=1
	if (a<=0) return 0;
	
	int i=0;
	while (a--) {
		while(!cs->buffer[i++]);
	}
	if (i-1 < BufferStorage::NUMBER_OF_BUFFERS)
		return cs->buffer[i-1];
	return 0;	
}


//
// Class BufferList
//
BufferList::BufferList()
{
	_state = BufferList::OK;
}


BufferList::~BufferList()
{
	// I am sure something should be done here too.
}


bool BufferList::isEmpty()
{
	return bstore.isEmpty();
}

extern void MenuWrite(Buffer*);

bool BufferList::QwriteAll()
{
        bool askMoreConfirmation = false;
        string unsaved;
	BufferStorage_Iter biter(bstore);
	Buffer *b=0;
	while ((b=biter())) {
		if (!b->isLyxClean()) {
			switch(AskConfirmation(_("Changes in document:"),
					       MakeDisplayPath(b->filename,50),
					       _("Save document?"))) {
			case 1: // Yes
				MenuWrite(b);
				break;
			case 2: // No
				askMoreConfirmation = true;
				unsaved += MakeDisplayPath(b->filename,50);
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
bool BufferList::write(Buffer *buf, bool makeBackup)
{
	minibuffer->Set(_("Saving document"),
			MakeDisplayPath(buf->filename),"...");

	// We don't need autosaves in the immediate future. (Asger)
	buf->resetAutosaveTimers();

	// make a backup
	if (makeBackup) {
		string s = buf->filename + '~';
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
		FileInfo finfo(buf->filename);
		if (finfo.exist()) {
			mode_t fmode = finfo.getMode();

			struct utimbuf *times =
				(struct utimbuf*)new char[sizeof(struct utimbuf)];
			times->actime = finfo.getAccessTime();
			times->modtime = finfo.getModificationTime();
			long blksize = finfo.getBlockSize();
			lyxerr.debug(string("BlockSize: ") + tostr(blksize));
			FilePtr fin(buf->filename,FilePtr::read);
			FilePtr fout(s,FilePtr::truncate);
			if (fin() && fout()) {
				char * cbuf = new char[blksize+1];
				size_t c_read = 0;
				size_t c_write = 0;
				do {
					c_read = fread(cbuf, 1, blksize, fin);
					if (c_read != 0)
						c_write =
							fwrite(cbuf, 1, c_read, fout);
				} while (c_read);
				fin.close();
				fout.close();
				chmod(s.c_str(), fmode);
				
				if (utime(s.c_str(), times)) {
					lyxerr.print("utime error.");
				}
				delete [] cbuf;
			} else {
				lyxerr.print("LyX was not able to make backupcopy. Beware.");
			}
			delete[] times;
		}
	}
	
	if (buf->writeFile(buf->filename,false)) {
		buf->markLyxClean();

		minibuffer->Set(_("Document saved as"),
				MakeDisplayPath(buf->filename));

		// now delete the autosavefile
		string a = OnlyPath(buf->filename);
		a += '#';
		a += OnlyFilename(buf->filename);
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
			string s = buf->filename + '~';
			rename(s.c_str(), buf->filename.c_str());
		}
		minibuffer->Set(_("Save failed!"));
		return false;
	}

	return true;
}


void BufferList::closeAll()
{
	_state = BufferList::CLOSING;
	
	BufferStorage_Iter biter(bstore);
	Buffer *b=0;
	while ((b=biter())) {
		close(b);
	}
	_state = BufferList::OK;
}


void BufferList::resize()
{
	BufferStorage_Iter biter(bstore);
	Buffer *b=0;
	while ((b=biter())) {
		b->resize();
	}
}


bool BufferList::close(Buffer *buf)
{
        buf->InsetUnlock();
	
	if (buf->paragraph && !buf->isLyxClean() && !quitting) {
		ProhibitInput();
                switch(AskConfirmation(_("Changes in document:"),
                              MakeDisplayPath(buf->filename,50),
                                      _("Save document?"))){
		case 1: // Yes
			if (write(buf)) {
				lastfiles->newFile(buf->filename);
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
	int ant=0;
	BufferStorage_Iter biter(bstore);
	Buffer *b=0;
	while ((b=biter())) {
		string relbuf = MakeDisplayPath(b->filename,30);
		fl_addtopup(pup, relbuf.c_str());
		ant++;
	}
	if (ant == 0) fl_addtopup(pup,_("No Documents Open!%t"));
}


Buffer* BufferList::first()
{
	BufferStorage_Iter biter(bstore);
	return biter();
}


Buffer* BufferList::getBuffer(int choice)
{
	BufferStorage_Iter biter(bstore);
	Buffer *b=0;
	b = biter[choice];
	    
	// Be careful, this could be 0.    
	return b;
}


void BufferList::updateInset(Inset *inset, bool mark_dirty)
{
	BufferStorage_Iter biter(bstore);
	Buffer *b=0;
	while ((b=biter())) {
		if (b->text && b->text->UpdateInset(inset)) {
			if (mark_dirty)
				b->markDirty();
			break;
		}
	}
}


int BufferList::unlockInset(UpdatableInset *inset)
{
	if (!inset) return 1;
	
	BufferStorage_Iter biter(bstore);
	Buffer *b=0;
	while ((b=biter())) {
		if (b->the_locking_inset == inset) {
			b->InsetUnlock();
			return 0;
		}
	}
	return 1;
}


void BufferList::updateIncludedTeXfiles(string const & mastertmpdir)
{
	BufferStorage_Iter biter(bstore);
	Buffer *b=0;
	while ((b=biter())) {
		if (!b->isDepClean(mastertmpdir)) {
			string writefile = mastertmpdir;
			writefile += '/';
			writefile += ChangeExtension(b->getFileName(), ".tex", true);
			b->makeLaTeXFile(writefile,mastertmpdir,false,true);
			b->markDepClean(mastertmpdir);
		}
	}
}


void BufferList::emergencyWriteAll()
{
	BufferStorage_Iter biter(bstore);
	Buffer *b=0;
	while ((b=biter())) {
		if (!b->isLyxClean()) {
			bool madeit=false;
			
			lyxerr.print(_("lyx: Attempting to save"
				      " document ")
				      + b->filename
				      + _(" as..."));
			
			for (int i=0; i<3 && !madeit; i++) {
				string s;
				
				// We try to save three places:
				// 1) Same place as document.
				// 2) In HOME directory.
				// 3) In "/tmp" directory.
				if (i==0) {
					s = b->filename;
				} else if (i==1) {
					s = AddName(GetEnvPath("HOME"),
						    b->filename);
				} else { // MakeAbsPath to prepend the current drive letter on OS/2
					s = AddName(MakeAbsPath("/tmp/"),
						    b->filename);
				}
				s += ".emergency";
				
				lyxerr.print(string("  ") + tostr(i+1) + ") " + s);
				
				if (b->writeFile(s,true)) {
					b->markLyxClean();
					lyxerr.print(_("  Save seems successful. Phew."));
					madeit = true;
				} else if (i != 2) {
					lyxerr.print(_("  Save failed! Trying..."));
				} else {
					lyxerr.print(_("  Save failed! Bummer. Document is lost."));
				}
			}
		}
	}
}


Buffer* BufferList::readFile(string const & s, bool ronly)
{
	Buffer *b = bstore.newBuffer(s, lyxrc, ronly);

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
					MakeDisplayPath(s,50),
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
						MakeDisplayPath(s,50),
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
	BufferStorage_Iter biter(bstore);
	Buffer *b=0;
	while ((b=biter())) {
		if (b->filename == s)
			return true;
	}
	return false;
}


Buffer* BufferList::getBuffer(string const &s)
{
	BufferStorage_Iter biter(bstore);
	Buffer *b=0;
	while ((b=biter())) {
		if (b->filename ==s)
			return b;
	}
	return 0;
}


Buffer* BufferList::newFile(string const & name, string tname)
{
	/* get a free buffer */ 
	Buffer *b = bstore.newBuffer(name, lyxrc);

	// use defaults.lyx as a default template if it exists.
	if (tname.empty()) {
		tname = LibFileSearch("templates", "defaults.lyx");
	}
	if (!tname.empty() && IsLyXFilename(tname)){
		bool templateok = false;
		LyXLex lex(0,0);
		lex.setFile(tname);
		if (lex.IsOK()) {
			if (b->readFile(lex)) {
				templateok = true;
			}
		}
		if (!templateok) {
			WriteAlert(_("Error!"),_("Unable to open template"), 
				   MakeDisplayPath(tname));
			// no template, start with empty buffer
			b->paragraph = new LyXParagraph();
		}
	}
	else {  // start with empty buffer
		b->paragraph = new LyXParagraph();
	}

	b->markDirty();
	b->setReadonly(false);
	
	return b;
}


Buffer* BufferList::loadLyXFile(string const & filename, bool tolastfiles)
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
					   _("Could not convert file"),s);
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
				MakeDisplayPath(s,50),
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
	Buffer *b=0;
	bool ro = false;
	switch (IsFileWriteable(s)) {
	case 0:
		minibuffer->Set(_("File `")+MakeDisplayPath(s,50)+
				_("' is read-only."));
		ro = true;
		// Fall through
	case 1:
		b=readFile(s, ro);
		if (b) {
			b->lyxvc.file_found_hook(s);
		}
		break; //fine- it's r/w
	case -1:
		// Here we probably should run
		if (LyXVC::file_not_found_hook(s)) {
			// Ask if the file should be checked out for
			// viewing/editing, if so: load it.
			lyxerr.print("Do you want to checkout?");
		}
		if (AskQuestion(_("Cannot open specified file:"), 
				MakeDisplayPath(s,50),
				_("Create new document with this name?")))
	    	{
			// Find a free buffer
			b = newFile(s,string());
	    	}
		break;
	}

	if (b && tolastfiles)
		lastfiles->newFile(b->getFileName());

	return b;
}
