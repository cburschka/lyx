// -*- C++ -*-
/** \file
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 *  \author Lars Gullik Bjønnes
*/

#ifndef BUFFER_LIST_H
#define BUFFER_LIST_H

#include "LString.h"

#include <boost/utility.hpp>

#include <vector>

class Buffer;

/**
 * The class holds all all open buffers, and handles construction
 * and deletions of new ones.
 */
class BufferList : boost::noncopyable {
public:
	BufferList();

	/**
	   Loads a LyX file or...

	   \param filename The filename to read from.
	   \param tolastfiles Wether the file should be put in the
	   last opened files list or not.
	   \return The newly loaded LyX file.
	*/
	Buffer * loadLyXFile(string const & filename,
			     bool tolastfiles = true);

	/// write all buffers, asking the user
	bool qwriteAll();

	/// create a new buffer
	Buffer * newBuffer(string const & s, bool ronly = false);

	/// delete a buffer
	void release(Buffer * b);
 
	/// Close all open buffers.
	void closeAll();

	/// read the given file
	Buffer * readFile(string const &, bool ro);

	/// Make a new file (buffer) using a template
	Buffer * newFile(string const &, string, bool isNamed = false);
	/// returns a vector with all the buffers filenames
	std::vector<string> const getFileNames() const;

	/// FIXME
	void updateIncludedTeXfiles(string const &);

	/// emergency save for all buffers
	void emergencyWriteAll();

	/// close buffer. Returns false if cancelled by user
	bool close(Buffer * buf);

	/// return true if no buffers loaded
	bool empty() const;

	/// return head of buffer list if any
	Buffer * first();

	/// returns true if the buffer exists already
	bool exists(string const &) const;

	/// returns true if the buffer is loaded
	bool isLoaded(Buffer const * b) const;

	/// returns a pointer to the buffer with the given name.
	Buffer * getBuffer(string const &);
	/// returns a pointer to the buffer with the given number.
	Buffer * getBuffer(unsigned int);

	/// reset current author for all buffers
	void setCurrentAuthor(string const & name, string const & email);

private:
	/// ask to save a buffer on quit
	bool qwriteOne(Buffer * buf, string const & fname,
		       string & unsaved_list);

	typedef std::vector<Buffer *> BufferStorage;
 
	/// storage of all buffers
	BufferStorage bstore;

	/// save emergency file for the given buffer
	void emergencyWrite(Buffer * buf);
};

#endif // BUFFERLIST_H
