// -*- C++ -*-
/**
 * \file bufferlist.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BUFFER_LIST_H
#define BUFFER_LIST_H

#include <boost/utility.hpp>

#include <string>
#include <vector>

class Buffer;
class LatexRunParams;

/**
 * The class holds all all open buffers, and handles construction
 * and deletions of new ones.
 */
class BufferList : boost::noncopyable {
public:
	BufferList();

	/// write all buffers, asking the user, returns false if cancelled
	bool quitWriteAll();

	/// create a new buffer
	Buffer * newBuffer(std::string const & s, bool ronly = false);

	/// delete a buffer
	void release(Buffer * b);

	/// Close all open buffers.
	void closeAll();

	/// returns a vector with all the buffers filenames
	std::vector<std::string> const getFileNames() const;

	/// FIXME
	void updateIncludedTeXfiles(std::string const &, LatexRunParams const &);

	/// emergency save for all buffers
	void emergencyWriteAll();

	/// close buffer. Returns false if cancelled by user
	bool close(Buffer * buf, bool ask);

	/// return true if no buffers loaded
	bool empty() const;

	/// return head of buffer list if any
	Buffer * first();

	/// returns true if the buffer exists already
	bool exists(std::string const &) const;

	/// returns true if the buffer is loaded
	bool isLoaded(Buffer const * b) const;

	/// returns a pointer to the buffer with the given name.
	Buffer * getBuffer(std::string const &);
	/// returns a pointer to the buffer with the given number.
	Buffer * getBuffer(unsigned int);
	/// returns a pointer to the buffer whose temppath matches the string
	Buffer * BufferList::getBufferFromTmp(std::string const &);

	/// reset current author for all buffers
	void setCurrentAuthor(std::string const & name, std::string const & email);

private:
	/// ask to save a buffer on quit, returns false if should cancel
	bool quitWriteBuffer(Buffer * buf);

	typedef std::vector<Buffer *> BufferStorage;

	/// storage of all buffers
	BufferStorage bstore;

	/// save emergency file for the given buffer
	void emergencyWrite(Buffer * buf);
};

#endif // BUFFERLIST_H
