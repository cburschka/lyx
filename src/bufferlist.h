// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team
 *
 *           This file is Copyright 1996
 *           Lars Gullik Bjønnes
 *
 * ======================================================*/

#ifndef BUFFER_LIST_H
#define BUFFER_LIST_H

#ifdef __GNUG__
#pragma interface
#endif

#include "buffer.h"
#include "debug.h"

/** A class to hold all the buffers in a structure
  The point of this class is to hide from bufferlist what kind
  of structure the buffers are stored in. Should be no concern for
  bufferlist if the buffers is in a array or in a linked list.

  This class should ideally be enclosed inside class BufferList, but that
  gave me an "internal gcc error".
  */
class BufferStorage {
public:
	///
	BufferStorage();
	///
	bool isEmpty();
	///
	void release(Buffer* buf);
	///
	Buffer* newBuffer(string const &s, LyXRC *, bool =false);
private:
	enum {
		/** The max number of buffers there are possible to have
		    loaded at the same time. (this only applies when we use an
		    array)
		*/
		NUMBER_OF_BUFFERS = 50
	};
	
	/** The Bufferlist is currently implemented as a static array.
	  The buffers are new'ed and deleted as reqested.
	  */
	Buffer *buffer[NUMBER_OF_BUFFERS];
	///
	friend class BufferStorage_Iter;
};

/// An Iterator class for BufferStorage
class BufferStorage_Iter {
public:
	///
	BufferStorage_Iter(BufferStorage const & bs)
	{ cs =& bs; index = 0;}
	/// next
	Buffer* operator() ();
	///
	Buffer* operator[] (int a);
private:
	///
	const BufferStorage *cs;
	///
	unsigned char index;
};



/** The class governing all the open buffers
  This class governs all the currently open buffers. Currently all the buffer
  are located in a static array, soon this will change and we will have a
  linked list instead.
 */
class BufferList {
public:
	///
 	BufferList();

	///
	~BufferList();

	/// state info
	enum list_state {
		///
		OK,
		///
		CLOSING
	};

	/// returns the state of the bufferlist
	list_state getState() { return _state; }
	
	/** loads a LyX file or...
	  If the optional argument tolastfiles is false (default is
            true), the file name will not be added to the last opened
	    files list
	    */  
	Buffer* loadLyXFile(string const & filename, 
			    bool tolastfiles = true);
	
	///
	bool isEmpty();

	/// Saves buffer. Returns false if unsuccesful.
	bool write(Buffer *, bool makeBackup = true);

	///
        bool QwriteAll();

	/// Close all open buffers.
	void closeAll();

	///
	void resize();

	/// Read a file into a buffer readonly or not.
	Buffer* readFile(string const &, bool ro);

	/// Make a new file (buffer) using a template
	Buffer* newFile(string const &, string);

	/** This one must be moved to some other place.
	 */
	void makePup(int);

	///** Later with multiple frames this should not be here.
	// */
	//Buffer* switchBuffer(Buffer *from, int);

	///
	void updateInset(Inset*, bool = true);

	///
	int unlockInset(UpdatableInset*);

	///
	void updateIncludedTeXfiles(string const &);

	///
	void emergencyWriteAll();

	/** closes buffer
	  Returns false if operation was canceled
	  */
	bool close(Buffer *);

	///
	Buffer* first();
	
	/// returns true if the buffer exists already
	bool exists(string const &);

	/// returns a pointer to the buffer with the given name.
	Buffer* getBuffer(string const &);
	/// returns a pointer to the buffer with the given number.
	Buffer* getBuffer(int);

private:
	///
	BufferStorage bstore;
	
	///
	list_state _state;
};

#endif
