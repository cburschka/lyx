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

#include <boost/signals/signal0.hpp>
#include <boost/signals/trackable.hpp>
#include <boost/signals/connection.hpp>

class Buffer;
class BufferView;


namespace lyx {
namespace graphics {

class PreviewImage;
class PreviewLoader;

} // namespace graphics
} // namespace lyx

class PreviewedInset : public boost::signals::trackable {
public:
	/// a wrapper for Previews::activated()
	static bool activated();

	///
	PreviewedInset();

	/** Find the PreviewLoader, add a LaTeX snippet to it and
	 *  start the loading process.
	 */
	void generatePreview(Buffer const &);

	/** Add a LaTeX snippet to the PreviewLoader but do not start the
	 *  loading process.
	 */
	void addPreview(lyx::graphics::PreviewLoader & ploader);

	/** Remove a snippet from the cache of previews.
	 *  Useful if previewing the contents of a file that has changed.
	 */
	void removePreview(Buffer const &);

	/// The preview has been generated and is ready to use.
	bool previewReady(Buffer const &) const;

	/// If the preview is not ready, returns 0.
	lyx::graphics::PreviewImage const * const pimage() const { return pimage_; }

	/// Connect and you'll be informed when the preview is ready.
	typedef boost::signal0<void>::slot_type slot_type;
	boost::signals::connection connect(slot_type const &);

protected:
	///
	virtual ~PreviewedInset() {}

private:
	/// This method is connected to the PreviewLoader::imageReady signal.
	void imageReady(lyx::graphics::PreviewImage const &) const;

	/// Does the owning inset want a preview?
	virtual bool previewWanted(Buffer const &) const = 0;
	/// a wrapper to Inset::latex
	virtual std::string const latexString(Buffer const &) const = 0;

	/// The thing that we're trying to generate a preview of.
	std::string snippet_;

	/// We don't own this. Cached for efficiency reasons.
	mutable lyx::graphics::PreviewImage const * pimage_;

	/** Store the connection to the preview loader so that we connect
	 *  only once.
	 */
	boost::signals::connection ploader_connection_;

	/// This signal is emitted when the preview is ready for display.
	boost::signal0<void> preview_ready_signal_;
};

#endif // PREVIEWEDINSET_H
