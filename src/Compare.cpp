/**
 * \file Compare.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Vincent van Ravesteijn
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Compare.h"

#include "Buffer.h"
#include "BufferParams.h"


using namespace std;
using namespace lyx::support;


namespace lyx {

/**
 * The implementation of the algorithm that does the comparison
 * between two documents.
 */
class Compare::Impl {
public:
	///
	Impl(Compare const & compare) 
		: abort_(false), compare_(compare)
	{}

	///
	~Impl() {}

	/// Set to true to abort the algorithm
	bool abort_;

private:
	/// The thread object, used to emit signals to the GUI
	Compare const & compare_;
};


Compare::Compare(Buffer const * new_buf, Buffer const * old_buf,
	Buffer * const dest_buf, CompareOptions const & options)
	: new_buffer(new_buf), old_buffer(old_buf), dest_buffer(dest_buf),
	  options_(options), pimpl_(new Impl(*this))
{
}


void Compare::run()
{
	if (!dest_buffer || !new_buffer || !old_buffer) {
		error();
		return;
	}

	// Copy the buffer params to the new buffer
	dest_buffer->params() = options_.settings_from_new
		? new_buffer->params() : old_buffer->params();
	
	// do the real work
	if (!doCompare())
		error();
	else
		finished(pimpl_->abort_);
	return;
}


void Compare::abort()
{
	pimpl_->abort_ = true;
	condition_.wakeOne();
	wait();
	pimpl_->abort_ = false;
}


int Compare::doCompare()
{
	return 0;
}


#include "moc_Compare.cpp"

} // namespace lyx
