// -*- C++ -*-
/**
 * \file render_preview.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 *
 * lyx::graphics::RenderPreview is an abstract base class that can help
 * insets to generate previews. The daughter class must instantiate two small
 * methods. The Inset would own an instance of this daughter class.
 */

#ifndef RENDER_PREVIEW_H
#define RENDER_PREVIEW_H

#include "render_base.h"

#include "support/FileMonitor.h"

#include <boost/signals/signal0.hpp>
#include <boost/signals/trackable.hpp>
#include <boost/signals/connection.hpp>

class Buffer;
class BufferView;
class MetricsInfo;
class PainterInfo;

namespace lyx {
namespace graphics {

class PreviewImage;
class PreviewLoader;

} // namespace graphics
} // namespace lyx


class RenderPreview : public RenderBase, public boost::signals::trackable {
public:
	/// a wrapper for Previews::activated()
	static bool activated();

	RenderPreview();
	RenderPreview(RenderPreview const &);
	std::auto_ptr<RenderBase> clone() const;

	/// Compute the size of the object, returned in dim
	void metrics(MetricsInfo &, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;

	/** Find the PreviewLoader and add a LaTeX snippet to it.
	 *  Do not start the loading process.
	 */
	void addPreview(std::string const & latex_snippet, Buffer const &);

	/** Add a LaTeX snippet to the PreviewLoader.
	 *  Do not start the loading process.
	 */
	void addPreview(std::string const & latex_snippet,
			lyx::graphics::PreviewLoader & ploader);

	/// Begin the loading process.
	void startLoading(Buffer const & buffer) const;

	/** Remove a snippet from the cache of previews.
	 *  Useful if previewing the contents of a file that has changed.
	 */
	void removePreview(Buffer const &);

	/// The preview has been generated and is ready to use.
	bool previewReady() const;

	/// Connect and you'll be informed when the preview is ready.
	typedef boost::signal0<void>::slot_type slot_type;
	boost::signals::connection connect(slot_type const &);

	/// equivalent to dynamic_cast
	virtual RenderPreview * asPreview() { return this; }

private:
	/// Not implemented.
	void operator=(RenderPreview const &);

	/// This method is connected to the PreviewLoader::imageReady signal.
	void imageReady(lyx::graphics::PreviewImage const &);

	/// The thing that we're trying to generate a preview of.
	std::string snippet_;

	/// We don't own this. Cached for efficiency reasons.
	lyx::graphics::PreviewImage const * pimage_;

	/** Store the connection to the preview loader so that we connect
	 *  only once.
	 */
	boost::signals::connection ploader_connection_;

	/// This signal is emitted when the preview is ready for display.
	boost::signal0<void> preview_ready_signal_;
};


class RenderMonitoredPreview : public RenderPreview {
public:
	RenderMonitoredPreview() : monitor_(std::string(), 2000) {}
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void setAbsFile(std::string const & file);
	///
	bool monitoring() const { return monitor_.monitoring(); }
	void startMonitoring() const { monitor_.start(); }
	void stopMonitoring() const { monitor_.stop(); }

	/// Connect and you'll be informed when the file changes.
	boost::signals::connection fileChanged(slot_type const &);

	/// equivalent to dynamic_cast
	virtual RenderMonitoredPreview * asMonitoredPreview() { return this; }

private:
	///
	mutable lyx::support::FileMonitor monitor_;
};

#endif // RENDERPREVIEW_H
