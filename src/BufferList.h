// -*- C++ -*-
/**
 * \file BufferList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BUFFER_LIST_H
#define BUFFER_LIST_H

#include "support/docstring.h"

#include <boost/noncopyable.hpp>

#include <vector>


namespace lyx {

class Buffer;
class OutputParams;

/**
 * The class holds all all open buffers, and handles construction
 * and deletions of new ones.
 */
class BufferList : boost::noncopyable {
public:
	typedef std::vector<Buffer *>::iterator iterator;
	typedef std::vector<Buffer *>::const_iterator const_iterator;

public:
	BufferList();

	iterator begin();
	const_iterator begin() const;

	iterator end();
	const_iterator end() const;

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
	void updateIncludedTeXfiles(std::string const &, OutputParams const &);

	/// emergency save for all buffers
	void emergencyWriteAll();

	/// close buffer. Returns false if cancelled by user
	bool close(Buffer * buf, bool ask);

	/// return true if no buffers loaded
	bool empty() const;

	/// return head of buffer list if any
	Buffer * first();

	/// return back of buffer list if any
	Buffer * last();

	/// returns true if the buffer exists already
	bool exists(std::string const &) const;

	/// returns true if the buffer is loaded
	bool isLoaded(Buffer const * b) const;

	/// returns a pointer to the buffer with the given name.
	Buffer * getBuffer(std::string const &);
	/// returns a pointer to the buffer with the given number.
	Buffer * getBuffer(unsigned int);
	/// returns a pointer to the buffer whose temppath matches the string
	Buffer * getBufferFromTmp(std::string const &);

	/** returns a pointer to the buffer that follows argument in
	 * buffer list. The buffer following the last in list is the
	 * first one.
	 */
	Buffer * next(Buffer const *) const;

	/** returns a pointer to the buffer that precedes argument in
	 * buffer list. The buffer preceding the first in list is the
	 * last one.
	 */
	Buffer * previous(Buffer const *) const;

	/// reset current author for all buffers
	void setCurrentAuthor(docstring const & name, docstring const & email);

private:
	/// ask to save a buffer on quit, returns false if should cancel
	bool quitWriteBuffer(Buffer * buf);

	typedef std::vector<Buffer *> BufferStorage;

	/// storage of all buffers
	BufferStorage bstore;

	/// save emergency file for the given buffer
	void emergencyWrite(Buffer * buf);
};

/// Implementation is in LyX.cpp
extern BufferList & theBufferList();


} // namespace lyx

#endif // BUFFERLIST_H
