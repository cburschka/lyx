// -*- C++ -*-
/** \file
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 *  \author Lars Gullik Bjønnes
*/

#ifndef BUFFER_LIST_H
#define BUFFER_LIST_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

#include <boost/utility.hpp>

#include <vector>

class Buffer;
class UpdatableInset;

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
	/**
	   Is the container empty or not.
	   \return True if the container is empty, False otherwise.
	 */
	bool empty() const { return container.empty(); }
	/**
	   Releases the passed buffer from the storage and deletes
	   all resources.
	   \param buf The buffer to release.
	 */
	void release(Buffer * buf);
	/**
	   \param s The name of the file to base the buffer on.
	   \param ronly If the buffer should be created read only of not.
	   \return The newly created buffer.
	 */
	Buffer * newBuffer(string const & s, bool ronly = false);
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
	/**
	   What is the size of the container.
	   \return The size of the container.
	 */
	size_type size() const { return container.size(); }
private:
	///
	Container container;
};


/**
   The class holds all all open buffers, and handles construction
   and deletions of new ones.
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

	/**
	   Loads a LyX file or...

	   \param filename The filename to read from.
	   \param tolastfiles Wether the file should be put in the
	   last opened files list or not.
	   \return The newly loaded LyX file.
	*/
	Buffer * loadLyXFile(string const & filename,
			     bool tolastfiles = true);

	///
	bool empty() const;

	///
	bool qwriteAll();

	/// Close all open buffers.
	void closeAll();

	/**
	   Read a file into a buffer readonly or not.
	   \return
	*/
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

	/**
	   Close buffer.
	   \param buf the buffer that should be closed
	   \return #false# if operation was canceled
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
	/// ask to save a buffer on quit
	bool qwriteOne(Buffer * buf, string const & fname,
		       string & unsaved_list);

	///
	BufferStorage bstore;

	///
	list_state state_;
	///
	void emergencyWrite(Buffer * buf);
};

#endif
