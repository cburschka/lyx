/**
 *  \file Previews.C
 *  Read the file COPYING
 *
 * \author Angus Leeming 
 *
 * Full author contact details available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Previews.h"
#include "PreviewLoader.h"

#include "buffer.h"
#include "lyxrc.h"

#include "insets/inset.h"

#include "support/LAssert.h"

#include <map>

namespace grfx {

bool Previews::activated()
{
	return lyxrc.preview;
}


Previews & Previews::get()
{
	static Previews singleton;
	return singleton;
}


struct Previews::Impl {
	///
	typedef boost::shared_ptr<PreviewLoader> PreviewLoaderPtr;
	///
	typedef std::map<Buffer const *, PreviewLoaderPtr> CacheType;
	///
	CacheType cache;
};


Previews::Previews()
	: pimpl_(new Impl())
{}


Previews::~Previews()
{}


PreviewLoader & Previews::loader(Buffer const * buffer) const
{
	lyx::Assert(buffer);

	Impl::CacheType::iterator it = pimpl_->cache.find(buffer);

	if (it == pimpl_->cache.end()) {
		Impl::PreviewLoaderPtr ptr(new PreviewLoader(*buffer));
		pimpl_->cache[buffer] = ptr;
		return *ptr.get();
	}

	return *it->second.get();
}


void Previews::removeLoader(Buffer const * buffer) const
{
	if (!buffer)
		return;

	Impl::CacheType::iterator it = pimpl_->cache.find(buffer);

	if (it != pimpl_->cache.end())
		pimpl_->cache.erase(it);
}


void Previews::generateBufferPreviews(Buffer const & buffer) const
{
	PreviewLoader & ploader = loader(&buffer);

	Buffer::inset_iterator it  = buffer.inset_const_iterator_begin();
	Buffer::inset_iterator end = buffer.inset_const_iterator_end();

	for (; it != end; ++it)
		it->addPreview(ploader);

	ploader.startLoading();
}

} // namespace grfx
