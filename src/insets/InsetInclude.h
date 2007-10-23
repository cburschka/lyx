// -*- C++ -*-
/**
 * \file InsetInclude.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Richard Heck (conversion to InsetCommand)
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_INCLUDE_H
#define INSET_INCLUDE_H

#include "BiblioInfo.h"
#include "InsetCommand.h"
#include "InsetCommandParams.h"
#include "RenderButton.h"
#include "MailInset.h"
#include "Counters.h"
#include "EmbeddedFiles.h"

#include "support/FileName.h"

#include <boost/scoped_ptr.hpp>

namespace lyx {

class Buffer;
class Dimension;
class LaTeXFeatures;
class RenderMonitoredPreview;


/// for including tex/lyx files
class InsetInclude : public InsetCommand {
public:
	///
	InsetInclude(InsetCommandParams const &);

	/// Override these InsetButton methods if Previewing
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	virtual DisplayType display() const;
	///
	InsetCode lyxCode() const { return INCLUDE_CODE; }
	/** Fills \c list
	 *  \param buffer the Buffer containing this inset.
	 *  \param list the list of labels in the child buffer.
	 */
	void getLabelList(Buffer const & buffer,
			  std::vector<docstring> & list) const;
	/** Fills \c keys
	 *  \param buffer the Buffer containing this inset.
	 *  \param keys the list of bibkeys in the child buffer.
	 *  \param it not used here
	 */
	virtual void fillWithBibKeys(Buffer const & buffer,
		BiblioInfo & keys, InsetIterator const & it) const;
	
	/** Update the cache with all bibfiles in use of the child buffer
	 *  (including bibfiles of grandchild documents).
	 *  Does nothing if the child document is not loaded to prevent
	 *  automatic loading of all child documents upon loading the master.
	 *  \param buffer the Buffer containing this inset.
	 */
	void updateBibfilesCache(Buffer const & buffer);
	/** Return the cache with all bibfiles in use of the child buffer
	 *  (including bibfiles of grandchild documents).
	 *  Return an empty vector if the child doc is not loaded.
	 *  \param buffer the Buffer containing this inset.
	 */
	std::vector<support::FileName> const &
		getBibfilesCache(Buffer const & buffer) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	int latex(Buffer const &, odocstream &,
		  OutputParams const &) const;
	///
	int plaintext(Buffer const &, odocstream &,
		      OutputParams const &) const;
	///
	int docbook(Buffer const &, odocstream &,
		    OutputParams const &) const;
	///
	void validate(LaTeXFeatures &) const;
	///
	void addPreview(graphics::PreviewLoader &) const;
	///
	void addToToc(TocList &, Buffer const &, ParConstIterator const &) const;
	///
	void updateLabels(Buffer const & buffer, ParIterator const &);
	/// child document can be embedded
	void registerEmbeddedFiles(Buffer const &, EmbeddedFiles &) const;
protected:
	InsetInclude(InsetInclude const &);
	///
	virtual void doDispatch(Cursor & cur, FuncRequest & cmd);
private:
	virtual Inset * clone() const;

	/** Slot receiving a signal that the external file has changed
	 *  and the preview should be regenerated.
	 */
	void fileChanged() const;

	/// set the parameters
	void set(InsetCommandParams const & params, Buffer const &);
	/// get the text displayed on the button
	docstring const getScreenLabel(Buffer const &) const;
	/// holds the entity name that defines the file location (SGML)
	docstring const include_label;

	/// The pointer never changes although *preview_'s contents may.
	boost::scoped_ptr<RenderMonitoredPreview> const preview_;

	/// cache
	mutable bool set_label_;
	mutable RenderButton button_;
	mutable docstring listings_label_;
};


/// return loaded Buffer or zero if the file loading did not proceed.
Buffer * loadIfNeeded(Buffer const & parent, InsetCommandParams const & params);

} // namespace lyx

#endif // INSETINCLUDE_H
