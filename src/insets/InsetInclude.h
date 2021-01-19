// -*- C++ -*-
/**
 * \file InsetInclude.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Richard Kimberly Heck (conversion to InsetCommand)
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_INCLUDE_H
#define INSET_INCLUDE_H

#include "InsetCommand.h"

#include "support/unique_ptr.h"


namespace lyx {

class Buffer;
class Dimension;
class InsetCommandParams;
class InsetLabel;
class LaTeXFeatures;
class RenderMonitoredPreview;

namespace support {
	class FileNameList;
}

/// for including tex/lyx files
class InsetInclude : public InsetCommand {
	// Disable assignment operator, since it is not used, and cannot be
	// implemented consistently with the copy constructor, because
	// include_label is const.
	InsetInclude & operator=(InsetInclude const &);
public:
	///
	InsetInclude(Buffer * buf, InsetCommandParams const &);
	///
	~InsetInclude();

	///
	void setChildBuffer(Buffer * buffer);

	/// \return loaded Buffer or zero if the file loading did not proceed.
	Buffer * loadIfNeeded() const;

	/** Update the cache with all bibfiles in use of the child buffer
	 *  (including bibfiles of grandchild documents).
	 *  Does nothing if the child document is not loaded to prevent
	 *  automatic loading of all child documents upon loading the master.
	 *  \param buffer the Buffer containing this inset.
	 */
	void updateBibfilesCache();

	///
	void updateCommand();
	///
	void write(std::ostream &) const override;

	/// \name Public functions inherited from Inset class
	//@{
	///
	void setBuffer(Buffer & buffer) override;
	///
	bool isLabeled() const override { return true; }
	///
	bool inheritFont() const override;
	/// Override these InsetButton methods if Previewing
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	RowFlags rowFlags() const override;
	///
	InsetCode lyxCode() const override { return INCLUDE_CODE; }
	///
	docstring layoutName() const override;
	/** Fills \c key
	 *  \param keys the list of bibkeys in the child buffer.
	 *  \param it not used here
	 */
	void collectBibKeys(InsetIterator const &, support::FileNameList &) const override;
	///
	bool hasSettings() const override { return true; }
	///
	void latex(otexstream &, OutputParams const &) const override;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const override;
	///
	void docbook(XMLStream &, OutputParams const &) const override;
	///
	docstring xhtml(XMLStream &, OutputParams const &) const override;
	///
	void validate(LaTeXFeatures &) const override;
	///
	void addPreview(DocIterator const &, graphics::PreviewLoader &) const override;
	///
	void addToToc(DocIterator const & di, bool output_active,
				  UpdateType utype, TocBackend & backend) const override;
	///
	void updateBuffer(ParIterator const &, UpdateType, bool const deleted = false) override;
	///
	std::string contextMenuName() const override;
	//@}

	/// \name Static public methods obligated for InsetCommand derived classes
	//@{
	///
	static ParamInfo const & findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "include"; }
	///
	static bool isCompatibleCommand(std::string const & s);
	///
	bool needsCProtection(bool const maintext = false,
			      bool const fragile = false) const override;
	//@}

protected:
	///
	InsetInclude(InsetInclude const &);

private:
	/** Slot receiving a signal that the external file has changed
	 *  and the preview should be regenerated.
	 */
	void fileChanged() const;
	/// launch external application
	void editIncluded(std::string const & file);
	///
	bool isChildIncluded() const;
	/// check whether the included file exist
	bool includedFileExist() const;
	/// \return True if there is a recursive include
	/// Also issues appropriate warning, etc
	bool checkForRecursiveInclude(Buffer const * cbuf, bool silent = false) const;

	/// \name Private functions inherited from Inset class
	//@{
	Inset * clone() const override { return new InsetInclude(*this); }
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const override;
	//@}

	/// \name Private functions inherited from InsetCommand class
	//@{
	/// set the parameters
	// FIXME:InsetCommmand::setParams is not virtual
	void setParams(InsetCommandParams const & params);
	/// get the text displayed on the button
	docstring screenLabel() const override;
	//@}

	/// holds the entity name that defines the file location (XML)
	docstring const include_label;

	/// The pointer never changes although *preview_'s contents may.
	unique_ptr<RenderMonitoredPreview> const preview_;

	///
	mutable bool failedtoload_;
	/// cache
	mutable docstring listings_label_;
	InsetLabel * label_;
	mutable Buffer * child_buffer_;
	mutable bool file_exist_;
	mutable bool recursion_error_;
};


} // namespace lyx

#endif // INSET_INCLUDE_H
