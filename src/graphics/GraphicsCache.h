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

#include "GraphicsTypes.h"
#include <map>
#include "LString.h"
#include <boost/utility.hpp>

class InsetGraphics;

namespace grfx {

class GCacheItem;

class GCache : boost::noncopyable {
public:

	/// This is a singleton class. Get the instance.
	static GCache & get();

	///
	~GCache();

	/// Add a file to the cache (or modify an existing image).
	void update(InsetGraphics const &, string const & filepath);

	/** Remove the data associated with this inset.
	 *  Called from the InsetGraphics d-tor.
	 */
	void remove(InsetGraphics const &);

	/** No processing of the image will take place until this call is
	 *  received.
	 */
	void startLoading(InsetGraphics const &);

	/** If (changed_background == true), then the background color of the
	 *  graphics inset has changed. Update all images.
	 *  Else, the preferred display type has changed.
	 *  Update the view of all insets whose display type is DEFAULT.
	 */
	void changeDisplay(bool changed_background = false);

	/// Get the image referenced by a particular inset.
	ImagePtr const image(InsetGraphics const &) const;

	/// How far have we got in loading the image?
	ImageStatus status(InsetGraphics const &) const;

	// Used to ascertain the Bounding Box of non (e)ps files.
	unsigned int raw_width(string const & filename) const;
	///
	unsigned int raw_height(string const & filename) const;
	
private:
	/** Make the c-tor private so we can control how many objects
	 *  are instantiated.
	 */
	GCache();

	/// The cache contains data of this type.
	typedef boost::shared_ptr<GCacheItem> CacheItemType;

	/** The cache contains one item per file, so use a map to find the
	 *  cache item quickly by filename.
	 *  Note that each cache item can have multiple views, potentially one
	 *  per inset that references the original file.
	 */
	typedef std::map<string, CacheItemType> CacheType;

	/// Search the cache by inset.
	CacheType::const_iterator find(InsetGraphics const &) const;
	///
	CacheType::iterator find(InsetGraphics const &);

	/** Store a pointer to the cache so that we can forward declare
	 *  GCacheItem.
	 */
	CacheType * cache;
};

} // namespace grfx


#endif // GRAPHICSCACHE_H
