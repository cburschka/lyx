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

#include <vector>


namespace lyx {

class Buffer;
class OutputParams;

namespace support {
class FileName;
class FileNameList;
}

/**
 * The class holds all all open buffers, and handles construction
 * and deletions of new ones.
 */
class BufferList {
public:
	typedef std::vector<Buffer *>::iterator iterator;
	typedef std::vector<Buffer *>::const_iterator const_iterator;

public:
	BufferList();
	~BufferList();

	iterator begin();
	const_iterator begin() const;

	iterator end();
	const_iterator end() const;

	/// create a new buffer
	/// \return 0 if the Buffer creation is not possible for whatever reason.
	Buffer * newBuffer(std::string const & s, bool ronly = false);

	/// delete a buffer
	void release(Buffer * b);

	/// Release \p child if it really is a child and is not used elsewhere.
	/// \return true is the file was closed.
	bool releaseChild(Buffer * parent, Buffer * child);

	/// Close all open buffers.
	void closeAll();

	/// returns a vector with all the buffers filenames
	support::FileNameList const & fileNames() const;

	/// FIXME
	void updateIncludedTeXfiles(std::string const &, OutputParams const &);

	/// emergency save for all buffers
	void emergencyWriteAll();

	/// save emergency file for the given buffer
	/**
	  * \return a status message towards the user.
	  */
	docstring emergencyWrite(Buffer * buf);

	/// return true if no buffers loaded
	bool empty() const;

	/// return head of buffer list if any
	Buffer * first();

	/// return back of buffer list if any
	Buffer * last();

	/// returns true if the buffer exists already
	bool exists(support::FileName const &) const;

	/// returns true if the buffer is loaded
	bool isLoaded(Buffer const * b) const;

	/// return index of named buffer in buffer list
	int bufferNum(support::FileName const & name) const;
	/// returns a pointer to the buffer with the given name.
	Buffer * getBuffer(support::FileName const &) const;
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
	/// noncopiable
	BufferList(BufferList const &);
	void operator=(BufferList const &);

	typedef std::vector<Buffer *> BufferStorage;

	/// storage of all buffers
	BufferStorage bstore;
	/// storage of all internal buffers used for cut&paste, etc.
	BufferStorage binternal;
};

/// Implementation is in LyX.cpp
extern BufferList & theBufferList();


} // namespace lyx

#endif // BUFFERLIST_H
