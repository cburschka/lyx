/**
 * \file Previews.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Previews.h"
#include "PreviewLoader.h"

#include "Buffer.h"
#include "InsetIterator.h"
#include "LyXRC.h"

#include "insets/Inset.h"


namespace lyx {

namespace graphics {

LyXRC_PreviewStatus Previews::status()
{
	return lyxrc.preview;
}


Previews & Previews::get()
{
	static Previews singleton;
	return singleton;
}


class Previews::Impl {
public:
	///
	typedef boost::shared_ptr<PreviewLoader> PreviewLoaderPtr;
	///
	typedef std::map<Buffer const *, PreviewLoaderPtr> CacheType;
	///
	CacheType cache;
};


Previews::Previews()
	: pimpl_(new Impl)
{}


Previews::~Previews()
{}


PreviewLoader & Previews::loader(Buffer const & buffer) const
{
	Impl::CacheType::iterator it = pimpl_->cache.find(&buffer);

	if (it == pimpl_->cache.end()) {
		Impl::PreviewLoaderPtr ptr(new PreviewLoader(buffer));
		pimpl_->cache[&buffer] = ptr;
		return *ptr.get();
	}

	return *it->second.get();
}


void Previews::removeLoader(Buffer const & buffer) const
{
	Impl::CacheType::iterator it = pimpl_->cache.find(&buffer);

	if (it != pimpl_->cache.end())
		pimpl_->cache.erase(it);
}


void Previews::generateBufferPreviews(Buffer const & buffer) const
{
	PreviewLoader & ploader = loader(buffer);

	Inset & inset = buffer.inset();
	InsetIterator it = inset_iterator_begin(inset);
	InsetIterator const end = inset_iterator_end(inset);

	for (; it != end; ++it)
		it->addPreview(ploader);

	ploader.startLoading();
}

} // namespace graphics
} // namespace lyx
