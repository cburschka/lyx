// -*- C++ -*-
/**
 *  \file GraphicsCache.h
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 * \author Baruch Even <baruch.even@writeme.com>
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 *
 *  grfx::GCache is the manager of the image cache.
 *  It is responsible for creating the grfx::GCacheItem's and maintaining them.
 *
 *  grfx::GCache is a singleton class. It is possible to have only one
 *  instance of it at any moment.
 */

#ifndef GRAPHICSCACHE_H
#define GRAPHICSCACHE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "GraphicsTypes.h"
#include <map>
#include <vector>
#include <boost/utility.hpp>

namespace grfx {

class GCache : boost::noncopyable {
public:

	/// This is a singleton class. Get the instance.
	static GCache & get();

	/** Which graphics formats can be loaded directly by the image loader.
	 *  Other formats can be loaded if a converter to a loadable format
	 *  can be defined.
	 */
	std::vector<string> loadableFormats() const;

	/// Add a graphics file to the cache.
	void add(string const & file);

	/** Remove a file from the cache.
	 *  Called from the InsetGraphics d-tor.
	 *  If we use reference counting, then this may become redundant.
	 */
	void remove(string const & file);

	/// Returns \c true if the file is in the cache.
	bool inCache(string const & file) const;

	/** Get the cache item associated with file.
	 *  Returns an empty container if there is no such item.
	 *
	 *  IMPORTANT: whatever uses an image must make a local copy of this
	 *  GraphicPtr. The boost::shared_ptr<>::use_count() function is
	 *  used to ascertain whether or not to remove the item from the cache
	 *  when remove(file) is called.
	 *
	 *  You have been warned!
	 */
	GraphicPtr const graphic(string const & file) const;

	/** Get the image associated with file.
	    If the image is not yet loaded, (or is not in the cache!) return
	    an empty container.
	 */
	ImagePtr const image(string const & file) const;

private:
	/** Make the c-tor, d-tor private so we can control how many objects
	 *  are instantiated.
	 */
	GCache();
	///
	~GCache();

	/** The cache contains one item per file, so use a map to find the
	 *  cache item quickly by filename.
	 *  Note that each cache item can have multiple views, potentially one
	 *  per inset that references the original file.
	 */
	typedef std::map<string, GraphicPtr> CacheType;

	/** Store a pointer to the cache so that we can forward declare
	 *  GCacheItem.
	 */
	CacheType * cache;
};

} // namespace grfx


#endif // GRAPHICSCACHE_H
