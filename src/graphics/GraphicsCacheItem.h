// -*- C++ -*-
/*
 * \file GraphicsCacheItem.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Baruch Even <baruch.even@writeme.com>
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 *
 * The graphics cache is a container of GCacheItems. Each GCacheItem, defined
 * here represents a separate image file. However, each file can be viewed in
 * different ways (different sizes, rotations etc), so each GCacheItem itself
 * contains a list of ModifiedItems, also defined here. Each ModifiedItem
 * has a GParams variable that defines the way it will be viewed. It also
 * contains a list of the graphics insets that refer to it, so calls through
 * the GCache to GCacheItem ultimately return the loading status and image
 * for that particular graphics inset.
 *
 * The graphics cache supports fully asynchronous:
 * file conversion to a loadable format;
 * file loading.
 *
 * Whether you get that, of course, depends on grfx::GConverter and on the
 * grfx::GImage-derived image class.
 *
 * Image modification (scaling, rotation etc) is blocking.
 */

#ifndef GRAPHICSCACHEITEM_H
#define GRAPHICSCACHEITEM_H

#ifdef __GNUG__
#pragma interface
#endif

#include "GraphicsTypes.h" 
#include <list>
#include "LString.h"
#include <boost/utility.hpp>
#include <boost/smart_ptr.hpp>
#include <sigc++/signal_system.h>

class InsetGraphics;

namespace grfx {

class GParams;
class ModifiedItem;
 
/// A grfx::GCache item holder.
class GCacheItem : boost::noncopyable, public SigC::Object {
public:
	/// the GCacheItem contains data of this type.
	typedef boost::shared_ptr<ModifiedItem> ModifiedItemPtr;

	///
	GCacheItem(InsetGraphics const &, GParams const &);

	/// The params have changed (but still refer to this file).
	void modify(InsetGraphics const &, GParams const &);

	/// Remove the reference to this inset.
	void remove(InsetGraphics const &);

	/// It's in the cache. Now start the loading process.
	void startLoading(InsetGraphics const &);

	/// Is the cache item referenced by any insets at all?
	bool empty() const;

	/// The name of the original image file.
	string const & filename() const;
	
	/// Is this image file referenced by this inset?
	bool referencedBy(InsetGraphics const &) const;

	/** Returns the image referenced by this inset (or an empty container
	 *  if it's not yet loaded.
	 */
	ImagePtr const image(InsetGraphics const &) const;

	/// The loading status of the image referenced by this inset.
	ImageStatus status(InsetGraphics const &) const;

	/** If (changed_background == true), then the background color of the
	 *  graphics inset has changed. Update all images.
	 *  Else, the preferred display type has changed.
	 *  Update the view of all insets whose display type is DEFAULT.
	 */
	void changeDisplay(bool changed_background);

private:
	/** Start the image conversion process, checking first that it is
	 *  necessary. If it is necessary, then a conversion task is started.
	 *  GCacheItem asumes that the conversion is asynchronous and so
	 *  passes a Signal to the converting routine. When the conversion
	 *  is finished, this Signal is emitted, returning the converted
	 *  file to this->imageConverted.
	 *
	 *  If no file conversion is needed, then convertToDisplayFormat() calls
	 *  loadImage() directly.
	 *
	 *  convertToDisplayFormat() will set the loading status flag as
	 *  approriate through calls to setStatus().
	 */
	void convertToDisplayFormat();

	/** Load the image into memory. This is called either from
	 *  convertToDisplayFormat() direct or from imageConverted().
	 */
	void loadImage();

	/** Get a notification when the image conversion is done.
	 *  Connected to a signal on_finish_ which is passed to
	 *  GConverter::convert.
	 */
	void imageConverted(string const & file_to_load);

	/** Get a notification when the image loading is done.
	 *  Connected to a signal on_finish_ which is passed to
	 *  GImage::loadImage.
	 */
	void imageLoaded(bool);

	/// How far have we got in loading the original, unmodified image?
	ImageStatus status() const; 

	/** Sets the status of the loading process. Also notifies
	 *  listeners that the status has chacnged.
	 */
	void setStatus(ImageStatus new_status);

	/// The filename we refer too.
	string filename_;
	/// Is the file compressed?
	bool zipped_;
	/// If so, store the uncompressed file in this temporary file.
	string unzipped_filename_;
	/// What file are we trying to load?
	string file_to_load_;
	/** Should we delete the file after loading? True if the file is
	 *  the result of a conversion process.
	 */
	bool remove_loaded_file_;

	/// The original, unmodified image and its loading status.
	ImagePtr image_;
	///
	ImageStatus status_;

	/** A SignalLoadTypePtr is connected to this->imageLoaded and
	 *  then passed to ImagePtr::load.
	 *  When the image has been loaded, the signal is emitted.
	 *
	 *  We pass a shared_ptr because it is eminently possible for the
	 *  ModifiedItem to be destructed before the loading is complete and
	 *  the signal must remain in scope. It doesn't matter if the slot
	 *  disappears, SigC takes care of that.
	 */
	typedef SigC::Signal1<void, bool> SignalLoadType;
	///
	typedef boost::shared_ptr<SignalLoadType> SignalLoadTypePtr;

	/// The connection of the signal passed to ImagePtr::loadImage.
	SigC::Connection cl_;

	/** A SignalConvertTypePtr is connected to this->imageConverted and
	 *  then passed to GConverter::convert.
	 *  When the image has been converted to a loadable format, the signal
	 *  is emitted, returning the name of the loadable file to
	 *  imageConverted.
	 */
	typedef SigC::Signal1<void, string const &> SignalConvertType;
	///
	typedef boost::shared_ptr<SignalConvertType> SignalConvertTypePtr;

	/// The connection of the signal passed to GConverter::convert.
	SigC::Connection cc_;

	/// The list of all modified images.
	typedef std::list<ModifiedItemPtr> ListType;
	///
	ListType modified_images;
};


///
class ModifiedItem {
public:
	///
	ModifiedItem(InsetGraphics const &, GParams const &, ImagePtr const &);

	///
	GParams const & params() { return *p_.get(); }

	/// Add inset to the list of insets.
	void add(InsetGraphics const & inset);

	/// Remove inset from the list of insets.
	void remove(InsetGraphics const & inset);

	///
	bool empty() const { return insets.empty(); }

	/// Is this ModifiedItem referenced by inset?
	bool referencedBy(InsetGraphics const & inset) const;

	///
	ImagePtr const image() const;

	/// How far have we got in loading the modified image?
	ImageStatus status() const { return status_; }

	/** Called from GCacheItem once the raw image is loaded.
	 *  Modifies the image in accord with p_.
	 */
	void modify(ImagePtr const &);

	/// Updates the pixmap.
	void setPixmap();

	/** changeDisplay returns a full ModifiedItemPtr if any of the
	 *  insets have display=DEFAULT and if that DEFAULT value has
	 *  changed.
	 *  If this occurs, then this has these insets removed.
	 */
	boost::shared_ptr<ModifiedItem> changeDisplay();

	///
	typedef std::list<InsetGraphics const *> ListType;

	/// Make these accessible for changeDisplay.
	ListType insets;

private:
	/** Sets the status of the loading process. Also notifies
	 *  listeners that the status has changed.
	 */
	void setStatus(ImageStatus new_status);

	/// The original and modified images and its loading status.
	ImagePtr original_image_;
	///
	ImagePtr modified_image_;
	///
	ImageStatus status_;
	/// 
	boost::shared_ptr<GParams> p_;
};

} // namespace grfx

#endif // GRAPHICSCACHEITEM_H
