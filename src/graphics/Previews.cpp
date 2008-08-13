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

using namespace std;

namespace lyx {

namespace graphics {

LyXRC_PreviewStatus Previews::status()
{
	return lyxrc.preview;
}


namespace {
typedef boost::shared_ptr<PreviewLoader> PreviewLoaderPtr;
///
typedef map<Buffer const *, PreviewLoaderPtr> LyxCacheType;
///
static LyxCacheType preview_cache_;
}


PreviewLoader & Previews::loader(Buffer const & buffer) const
{
	LyxCacheType::iterator it = preview_cache_.find(&buffer);

	if (it == preview_cache_.end()) {
		PreviewLoaderPtr ptr(new PreviewLoader(buffer));
		preview_cache_[&buffer] = ptr;
		return *ptr.get();
	}

	return *it->second.get();
}


void Previews::removeLoader(Buffer const & buffer) const
{
	LyxCacheType::iterator it = preview_cache_.find(&buffer);

	if (it != preview_cache_.end())
		preview_cache_.erase(it);
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
