// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team
 *
 *           This file is Copyright 1996-2001
 *           Lars Gullik Bjønnes
 *
 * ====================================================== */

#ifndef BUFFER_LIST_H
#define BUFFER_LIST_H

#ifdef __GNUG__
#pragma interface
#endif

#include "buffer.h"
#include "debug.h"
#include <boost/utility.hpp>

/** A class to hold all the buffers in a structure
  The point of this class is to hide from bufferlist what kind
  of structure the buffers are stored in. Should be no concern for
  bufferlist if the buffers is in a array or in a linked list.

  This class should ideally be enclosed inside class BufferList, but that
  gave me an "internal gcc error".
  */
class BufferStorage : boost::noncopyable {
public:
	///
	typedef std::vector<Buffer *> Container;
	///
	typedef Container::iterator iterator;
	///
	typedef Container::const_iterator const_iterator;
	///
	typedef Container::size_type size_type;
	///
	bool empty() const { return container.empty(); }
	///
	void release(Buffer * buf);
	///
	Buffer * newBuffer(string const & s, bool = false);
	///
	Container::iterator begin() { return container.begin(); }
	///
	Container::iterator end() { return container.end(); }
	///
	Container::const_iterator begin() const { return container.begin(); }
	///
	Container::const_iterator end() const { return container.end(); }
	///
	Buffer * front() { return container.front(); }
	///
	Buffer * operator[](int c) { return container[c]; }
	///
	size_type size() const { return container.size(); }
private:
	///
	Container container;
};


/** The class govern all open buffers.
 */
class BufferList : boost::noncopyable {
public:
	///
 	BufferList();

	/// state info
	enum list_state {
		///
		OK,
		///
		CLOSING
	};

	/// returns the state of the bufferlist
	list_state getState() const { return state_; }
	
	/** loads a LyX file or...
	    If the optional argument tolastfiles is false (default is
            true), the file name will not be added to the last opened
	    files list
	*/  
	Buffer * loadLyXFile(string const & filename, 
			     bool tolastfiles = true);
	
	///
	bool empty() const;

	///
        bool QwriteAll();

	/// Close all open buffers.
	void closeAll();

	/// Read a file into a buffer readonly or not.
	Buffer * readFile(string const &, bool ro);

	/// Make a new file (buffer) using a template
	Buffer * newFile(string const &, string, bool isNamed = false);
	/// returns a vector with all the buffers filenames
	std::vector<string> const getFileNames() const;

	///
	int unlockInset(UpdatableInset *);

	///
	void updateIncludedTeXfiles(string const &);

	///
	void emergencyWriteAll();
	
	/** Close buffer.
	    @param buf the buffer that should be closed
	    @return #false# if operation was canceled
	  */
	bool close(Buffer * buf);

	///
	Buffer * first();
	
	/// returns true if the buffer exists already
	bool exists(string const &) const;

	/// returns true if the buffer is loaded
	bool isLoaded(Buffer const * b) const;
	
	/// returns a pointer to the buffer with the given name.
	Buffer * getBuffer(string const &);
	/// returns a pointer to the buffer with the given number.
	Buffer * getBuffer(unsigned int);

private:
	///
	BufferStorage bstore;
	
	///
	list_state state_;
	///
	void emergencyWrite(Buffer * buf);
};

#endif
