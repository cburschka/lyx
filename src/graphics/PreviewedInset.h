// -*- C++ -*-
/**
 *  \file PreviewedInset.h
 *  Read the file COPYING
 *
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
 *
 *  grfx::PreviewedInset is an abstract base class that can help insets to
 *  generate previews. The daughter class must instantiate three small
 *  methods. The Inset would own an instance of this daughter class.
 */

#ifndef PREVIEWEDINSET_H
#define PREVIEWEDINSET_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include <boost/weak_ptr.hpp>
#include <boost/signals/trackable.hpp>
#include <boost/signals/connection.hpp>

class Inset;
class BufferView;

namespace grfx {

class PreviewImage;
class PreviewLoader;

class PreviewedInset : public boost::signals::trackable {
public:
	/// a wrapper for Previews::activated()
	static bool activated();

	///
	PreviewedInset(Inset & inset) : inset_(inset), pimage_(0) {}
	///
	virtual ~PreviewedInset() {}

	/** Find the PreviewLoader, add a LaTeX snippet to it and
	 *  start the loading process.
	 */
	void generatePreview();

	/** Add a LaTeX snippet to the PreviewLoader but do not start the
	 *  loading process.
	 */
	void addPreview(PreviewLoader & ploader);

	/** Remove a snippet from the cache of previews.
	 *  Useful if previewing the contents of a file that has changed.
	 */
	void removePreview();

	/// The preview has been generated and is ready to use.
	bool previewReady() const;

	/// If !previewReady() returns 0.
	PreviewImage const * pimage() const { return pimage_; }

	///
	void setView(BufferView *);

protected:
	/// Allow the daughter classes to cast up to the parent inset.
	Inset * inset() const { return &inset_; }
	///
	BufferView * view() const { return view_.get(); }

private:
	/** This method is connected to the grfx::PreviewLoader::imageReady
	 *  signal.
	 */
	void imageReady(PreviewImage const &) const;

	/// Does the owning inset want a preview?
	virtual bool previewWanted() const = 0;
	/// a wrapper to Inset::latex
	virtual string const latexString() const = 0;

	///
	Inset & inset_;
	///
	string snippet_;
	///
	boost::weak_ptr<BufferView> view_;
	
	/// We don't own this. Cached for efficiency reasons.
	mutable PreviewImage const * pimage_;
	///
	boost::signals::connection connection_;
};

} // namespace grfx


#endif // PREVIEWEDINSET_H
