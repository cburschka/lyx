// -*- C++ -*-
/**
 * \file PreviewedInset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 *
 * lyx::graphics::PreviewedInset is an abstract base class that can help
 * insets to generate previews. The daughter class must instantiate two small
 * methods. The Inset would own an instance of this daughter class.
 */

#ifndef PREVIEWEDINSET_H
#define PREVIEWEDINSET_H

#include <boost/signals/trackable.hpp>
#include <boost/signals/connection.hpp>

class Buffer;
class BufferView;
class InsetOld;


namespace lyx {
namespace graphics {

class PreviewImage;
class PreviewLoader;

class PreviewedInset : public boost::signals::trackable {
public:
	/// a wrapper for Previews::activated()
	static bool activated();

	///
	PreviewedInset(InsetOld & inset);

	/** Find the PreviewLoader, add a LaTeX snippet to it and
	 *  start the loading process.
	 */
	void generatePreview(Buffer const &);

	/** Add a LaTeX snippet to the PreviewLoader but do not start the
	 *  loading process.
	 */
	void addPreview(PreviewLoader & ploader);

	/** Remove a snippet from the cache of previews.
	 *  Useful if previewing the contents of a file that has changed.
	 */
	void removePreview(Buffer const &);

	/// The preview has been generated and is ready to use.
	bool previewReady(Buffer const &) const;

	/// If !previewReady() returns 0.
	PreviewImage const * pimage() const;

protected:
	///
	virtual ~PreviewedInset() {}
	/// Allow the daughter classes to cast up to the parent inset.
	InsetOld const & inset() const;
	///
	BufferView * view() const;

private:
	/// This method is connected to the PreviewLoader::imageReady signal.
	void imageReady(PreviewImage const &) const;

	/// Does the owning inset want a preview?
	virtual bool previewWanted(Buffer const &) const = 0;
	/// a wrapper to Inset::latex
	virtual std::string const latexString(Buffer const &) const = 0;

	///
	InsetOld & inset_;
	///
	std::string snippet_;

	/// We don't own this. Cached for efficiency reasons.
	mutable PreviewImage const * pimage_;
	/** Store the connection to the preview loader so that we connect
	 *  only once.
	 */
	boost::signals::connection ploader_connection_;
};


inline
PreviewImage const * PreviewedInset::pimage() const
{
	return pimage_;
}


inline
InsetOld const & PreviewedInset::inset() const
{
	return inset_;
}

} // namespace graphics
} // namespace lyx

#endif // PREVIEWEDINSET_H
