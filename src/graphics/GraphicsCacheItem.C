/*
 * \file GraphicsCacheItem.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Baruch Even <baruch.even@writeme.com>
 * \author Herbert Voss <voss@lyx.org>
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "graphics/GraphicsCache.h"
#include "graphics/GraphicsCacheItem.h"
#include "graphics/GraphicsImage.h"
#include "graphics/GraphicsParams.h"
#include "graphics/GraphicsConverter.h"
#include "insets/insetgraphics.h"
#include "BufferView.h"
#include "debug.h"
#include "gettext.h"
#include "lyx_main.h" // for global dispatch method
#include "support/LAssert.h"
#include "support/filetools.h"
#include "frontends/Alert.h"

// Very, Very UGLY!
extern BufferView * current_view;

using std::endl;


namespace grfx {

GCacheItem::GCacheItem(InsetGraphics const & inset, GParams const & params)
	: filename_(params.filename), zipped_(false),
	  remove_loaded_file_(false), status_(WaitingToLoad)
{
	ModifiedItemPtr item(new ModifiedItem(inset, params, image_));
	modified_images.push_back(item);
}


namespace {

typedef GCacheItem::ModifiedItemPtr ModifiedItemPtr;

class Compare_Params {
public:
	Compare_Params(GParams const & p) : p_(p) {}

	bool operator()(ModifiedItemPtr const & ptr)
	{
		if (!ptr.get())
			return false;
		return ptr->params() == p_;
	}

private:
	GParams const & p_;
};

class Find_Inset {
public:
	Find_Inset(InsetGraphics const & i) : i_(i) {}

	bool operator()(ModifiedItemPtr const & ptr)
	{
		if (!ptr.get())
			return false;
		return ptr->referencedBy(i_);
	}

private:
	InsetGraphics const & i_;
};

} // namespace anon


void GCacheItem::modify(InsetGraphics const & inset, GParams const & params)
{
	// Does this inset currently reference an existing ModifiedItem with
	// different params?
	// If so, remove the inset from the ModifiedItem's internal list
	// of insets
	ListType::iterator begin = modified_images.begin();
	ListType::iterator end   = modified_images.end();
	ListType::iterator it    = begin;
	while (it != end) {
		it = std::find_if(it, end, Find_Inset(inset));
		if (it == end)
			break;
		if ((*it)->params() != params) {
			(*it)->remove(inset);
			if ((*it)->empty())
				it = modified_images.erase(it);
		}
		++it;
	}

	// Is there an existing ModifiedItem with these params?
	// If so, add inset to the list of insets referencing this ModifiedItem
	begin = modified_images.begin();
	end   = modified_images.end();
	it = std::find_if(begin, end, Compare_Params(params));
	if (it != end) {
		(*it)->add(inset);
		return;
	}

	// If no ModifiedItem exists with these params, then create one.
	ModifiedItemPtr item(new ModifiedItem(inset, params, image_));
	modified_images.push_back(item);

	return;
}


void GCacheItem::remove(InsetGraphics const & inset)
{
	// search the list of ModifiedItems for one referenced by this inset.
	// If it is found, remove the reference.
	// If the ModifiedItem is now referenced by no insets, remove it.
	ListType::iterator begin = modified_images.begin();
	ListType::iterator end   = modified_images.end();
	ListType::iterator it = std::find_if(begin, end, Find_Inset(inset));

	if (it == end)
		return;

	(*it)->remove(inset);
	if ((*it)->empty()) {
		modified_images.clear();
	}
}


void GCacheItem::startLoading(InsetGraphics const & inset)
{
	if (status() != WaitingToLoad)
		return;

	// Check that the image is referenced by this inset
	ListType::const_iterator begin = modified_images.begin();
	ListType::const_iterator end   = modified_images.end();
	ListType::const_iterator it =
		std::find_if(begin, end, Find_Inset(inset));

	if (it == end)
		return;

	if ((*it)->params().display == GParams::NONE)
		return;

	convertToDisplayFormat();
}


bool GCacheItem::empty() const
{
	return modified_images.empty();
}


bool GCacheItem::referencedBy(InsetGraphics const & inset) const
{
	// Is one of the list of ModifiedItems referenced by this inset?
	ListType::const_iterator begin = modified_images.begin();
	ListType::const_iterator end   = modified_images.end();
	return std::find_if(begin, end, Find_Inset(inset)) != end;
}


string const & GCacheItem::filename() const
{
	return filename_;
}


ImagePtr const GCacheItem::image(InsetGraphics const & inset) const
{
	// find a ModifiedItem that is referenced by this inset.
	ListType::const_iterator begin = modified_images.begin();
	ListType::const_iterator end   = modified_images.end();
	ListType::const_iterator it =
		std::find_if(begin, end, Find_Inset(inset));

	// Someone's being daft.
	if (it == end)
		return ImagePtr();

	// We are expressly requested to not render the image
	if ((*it)->params().display == GParams::NONE)
		return ImagePtr();

	// If the original image has been loaded, return what's going on
	// in the ModifiedItem
	if (status() == Loaded)
		return (*it)->image();

	return ImagePtr();
}


ImageStatus GCacheItem::status(InsetGraphics const & inset) const
{
	// find a ModifiedItem that is referenced by this inset.
	ListType::const_iterator begin = modified_images.begin();
	ListType::const_iterator end   = modified_images.end();
	ListType::const_iterator it =
		std::find_if(begin, end, Find_Inset(inset));

	// Someone's being daft.
	if (it == end)
		return ErrorUnknown;

	if (status() == Loaded)
		return (*it)->status();

	return status();
}


// Called internally only. Use to ascertain the status of the loading of the
// original image. No scaling etc.
ImageStatus GCacheItem::status() const
{
	return status_;
}


void GCacheItem::setStatus(ImageStatus new_status)
{
	status_ = new_status;

	// Loop over all insets and tell the BufferView that it has changed.
	typedef ModifiedItem::ListType::const_iterator inset_iterator;

	ListType::const_iterator it  = modified_images.begin();
	ListType::const_iterator end = modified_images.end();
	for (; it != end; ++it) {
		inset_iterator it2  = (*it)->insets.begin();
		inset_iterator end2 = (*it)->insets.end();

		for (; it2 != end2; ++it2) {
			InsetGraphics * inset =
				const_cast<InsetGraphics *>(*it2);

			// Use of current_view is very, very Evil!!
			current_view->updateInset(inset, false);
		}
	}
}


void GCacheItem::changeDisplay(bool changed_background)
{
	ListType::iterator begin = modified_images.begin();
	ListType::iterator end   = modified_images.end();

	// The background has changed. Change all modified images.
	if (changed_background) {
		for (ListType::iterator it = begin; it != end; ++it) {
			(*it)->setPixmap();
		}
		return;
	}

	ListType temp_list;

	for (ListType::iterator it = begin; it != end; ++it) {
		// ModifiedItem::changeDisplay returns a full
		// ModifiedItemPtr if any of the insets have display=DEFAULT
		// and if that DEFAULT value has changed
		ModifiedItemPtr new_item = (*it)->changeDisplay();
		if (!new_item.get())
			continue;

		temp_list.push_back(new_item);

		// The original store may now be empty
		if ((*it)->insets.empty()) {
			it = modified_images.erase(it);
		}
	}

	if (temp_list.empty())
		return;

	// Recombine new_list and modified_images.
	begin = modified_images.begin();
	end   = modified_images.end();

	ListType::const_iterator tbegin = temp_list.begin();
	ListType::const_iterator tend   = temp_list.end();

	ListType append_list;

	for (ListType::const_iterator tit = tbegin; tit != tend; ++tit) {
		GParams const & params = (*tit)->params();
		ListType::iterator it =
			std::find_if(begin, end, Compare_Params(params));
		if (it == end)
			append_list.push_back(*tit);
		else
			(*it)->insets.merge((*tit)->insets);
	}

	if (append_list.empty())
		return;

	modified_images.splice(modified_images.end(), append_list);
}


void GCacheItem::imageConverted(string const & file_to_load)
{
	bool const success =
		(!file_to_load.empty() && IsFileReadable(file_to_load));

	string const text = success ? "succeeded" : "failed";
	lyxerr[Debug::GRAPHICS] << "Image conversion " << text << "." << endl;

	if (!success) {
		setStatus(ErrorConverting);

		if (zipped_)
			lyx::unlink(unzipped_filename_);

		return;
	}

	cc_.disconnect();

	// Do the actual image loading from file to memory.
	file_to_load_ = file_to_load;

	loadImage();
}


// This function gets called from the callback after the image has been
// converted successfully.
void GCacheItem::loadImage()
{
	setStatus(Loading);
	lyxerr[Debug::GRAPHICS] << "Loading image." << endl;

	// Connect a signal to this->imageLoaded and pass this signal to
	// GImage::loadImage.
	SignalLoadTypePtr on_finish;
	on_finish.reset(new SignalLoadType);
	cl_ = on_finish->connect(SigC::slot(this, &GCacheItem::imageLoaded));

	image_ = GImage::newImage();
	image_->load(file_to_load_, on_finish);
}


void GCacheItem::imageLoaded(bool success)
{
	string const text = success ? "succeeded" : "failed";
	lyxerr[Debug::GRAPHICS] << "Image loading " << text << "." << endl;

	// Clean up after loading.
	if (zipped_)
		lyx::unlink(unzipped_filename_);

	if (remove_loaded_file_ && unzipped_filename_ != file_to_load_)
		lyx::unlink(file_to_load_);

	cl_.disconnect();

	if (!success) {
		setStatus(ErrorLoading);
		return;
	}

	setStatus(Loaded);

	// Loop over the list of modified images and create them.
	ListType::iterator it  = modified_images.begin();
	ListType::iterator end = modified_images.end();
	for (; it != end; ++it) {
		(*it)->modify(image_);
	}
}


namespace {

string const findTargetFormat(string const & from)
{
	typedef GImage::FormatList FormatList;
	FormatList const & formats = GImage::loadableFormats();

	// There must be a format to load from.
	lyx::Assert(!formats.empty());

	// First ascertain if we can load directly with no conversion
	FormatList::const_iterator it1  = formats.begin();
	FormatList::const_iterator end = formats.end();
	for (; it1 != end; ++it1) {
		if (from == *it1)
			return *it1;
	}

	// So, we have to convert to a loadable format. Can we?
	grfx::GConverter const & graphics_converter = grfx::GConverter::get();

	FormatList::const_iterator it2  = formats.begin();
	for (; it2 != end; ++it2) {
		if (graphics_converter.isReachable(from, *it2))
			return *it2;
	}

	// Failed!
	return string();
}

} // anon namespace


void GCacheItem::convertToDisplayFormat()
{
	setStatus(Converting);
	string filename = filename_; // Make a local copy in case we unzip it
	string const displayed_filename = MakeDisplayPath(filename_);

	// First, check that the file exists!
	if (!IsFileReadable(filename)) {
		setStatus(ErrorNoFile);
		return;
	}

// maybe that other zip extensions also be useful, especially the
// ones that may be declared in texmf/tex/latex/config/graphics.cfg.
// for example:
/* -----------snip-------------
	  {\DeclareGraphicsRule{.pz}{eps}{.bb}{}%
	   \DeclareGraphicsRule{.eps.Z}{eps}{.eps.bb}{}%
	   \DeclareGraphicsRule{.ps.Z}{eps}{.ps.bb}{}%
	   \DeclareGraphicsRule{.ps.gz}{eps}{.ps.bb}{}%
	   \DeclareGraphicsRule{.eps.gz}{eps}{.eps.bb}{}}}%
   -----------snip-------------*/

	lyxerr[Debug::GRAPHICS]
		<< "Attempting to convert image file: " << displayed_filename
		<< "\nwith recognised extension: " << GetExtension(filename)
		<< "." << endl;

	zipped_ = zippedFile(filename);
	if (zipped_) {
		filename = unzipFile(filename);
		unzipped_filename_ = filename;
	}

	string const from = getExtFromContents(filename);
	string const to   = grfx::findTargetFormat(from);

	lyxerr[Debug::GRAPHICS]
		<< "The file contains " << from << " format data." << endl;

	if (to.empty()) {
		Alert::alert(_("Unable to convert file ") +
			     displayed_filename +
			     _(" to a loadable format."));
		setStatus(ErrorConverting);
		return;
	}

	if (from == to) {
		// No conversion needed!
		lyxerr[Debug::GRAPHICS] << "No conversion needed!" << endl;
		file_to_load_ = filename;
		loadImage();
		return;
	}

	lyxerr[Debug::GRAPHICS] << "Converting it to " << to << " format." << endl;

	// Take only the filename part of the file, without path or extension.
	string const temp = ChangeExtension(OnlyFilename(filename), string());

	// Add some stuff to create a uniquely named temporary file.
	// This file is deleted in loadImage after it is loaded into memory.
	string const to_file_base = lyx::tempName(string(), temp);
	remove_loaded_file_ = true;

	// Remove the temp file, we only want the name...
	lyx::unlink(to_file_base);

	// Connect a signal to this->imageConverted and pass this signal to
	// the graphics converter so that we can load the modified file
	// on completion of the conversion process.
	SignalConvertTypePtr on_finish;
	on_finish.reset(new SignalConvertType);
	cc_ = on_finish->connect(SigC::slot(this, &GCacheItem::imageConverted));

	GConverter & graphics_converter = GConverter::get();
	graphics_converter.convert(filename, to_file_base, from, to, on_finish);
}


ModifiedItem::ModifiedItem(InsetGraphics const & new_inset,
			   GParams const &  new_params,
			   ImagePtr const & new_image)
	: status_(ScalingEtc)
{
	p_.reset(new GParams(new_params));
	insets.push_back(&new_inset);
	modify(new_image);
}


void ModifiedItem::add(InsetGraphics const & inset)
{
	insets.push_back(&inset);
	insets.sort();
}


void ModifiedItem::remove(InsetGraphics const & inset)
{
	ListType::iterator begin = insets.begin();
	ListType::iterator end   = insets.end();
	ListType::iterator it    = std::remove(begin, end, &inset);
	insets.erase(it, end);
}


bool ModifiedItem::referencedBy(InsetGraphics const & inset) const
{
	ListType::const_iterator begin = insets.begin();
	ListType::const_iterator end   = insets.end();
	return std::find(begin, end, &inset) != end;
}


ImagePtr const ModifiedItem::image() const
{
	if (modified_image_.get())
		return modified_image_;

	return original_image_;
}


void ModifiedItem::modify(ImagePtr const & new_image)
{
	if (!new_image.get())
		return;

	original_image_ = new_image;
	modified_image_.reset(original_image_->clone());

	if (params().display == GParams::NONE) {
		setStatus(Loaded);
		return;
	}

	setStatus(ScalingEtc);
	modified_image_->clip(params());
	modified_image_->rotate(params());
	modified_image_->scale(params());
	setPixmap();
}


void ModifiedItem::setPixmap()
{
	if (!modified_image_.get())
		return;

	if (params().display == GParams::NONE) {
		setStatus(Loaded);
		return;
	}

	bool const success = modified_image_->setPixmap(params());

	if (success) {
		setStatus(Loaded);
	} else {
		modified_image_.reset();
		setStatus(ErrorScalingEtc);
	}
}


void ModifiedItem::setStatus(ImageStatus new_status)
{
	status_ = new_status;

	// Tell the BufferView that the inset has changed.
	// Very, Very Ugly!!
	ListType::const_iterator it  = insets.begin();
	ListType::const_iterator end = insets.end();
	for (; it != end; ++it) {
		InsetGraphics * inset = const_cast<InsetGraphics *>(*it);
		current_view->updateInset(inset, false);
	}
}


namespace {

struct Params_Changed {

	Params_Changed(GParams const & p) : p_(p) {}

	bool operator()(InsetGraphics const * inset)
	{
		string const path = OnlyPath(p_.filename);
		return GParams(inset->params(), path) != p_;
	}

private:
	GParams p_;
};

} // namespace anon

// changeDisplay returns an initialised ModifiedItem if any of the insets
// have display == DEFAULT and if that DEFAULT value has changed.
// If this occurs, then (this) has these insets removed.
ModifiedItemPtr ModifiedItem::changeDisplay()
{
	// Loop over the list of insets. Compare the updated params for each
	// with params(). If different, move into a new list.
	ListType::iterator begin = insets.begin();
	ListType::iterator end   = insets.end();
	ListType::iterator it =
		std::remove_if(begin, end, Params_Changed(params()));

	if (it == end) {
		// No insets have changed params
		return ModifiedItemPtr();
	}

	// it -> end have params that are changed. Move to the new list.
	ListType new_insets;
	new_insets.insert(new_insets.begin(), it, end);
	insets.erase(it, end);

	// Create a new ModifiedItem with these new params. Note that
	// the only params that have changed are the display ones,
	// so we don't need to crop, rotate, scale.
	string const path = OnlyPath(p_->filename);

	ModifiedItemPtr new_item(new ModifiedItem(*this));
	new_item->insets = new_insets;
	*(new_item->p_)  = GParams((*new_insets.begin())->params(), path);

	new_item->setPixmap();
	return new_item;
}

} // namespace grfx
