// -*- C++ -*-
/**
 * \file InsetExternal.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_EXTERNAL_H
#define INSET_EXTERNAL_H

#include "Inset.h"

#include "ExternalTemplate.h"

#include "support/FileName.h"
#include "support/unique_ptr.h"


namespace lyx {

namespace external {

/** No two InsetExternalParams variables can have the same temporary file.
 *  This class has copy-semantics but the copy constructor
 *  and assignment operator simply call the default constructor.
 *  Use of this class enables us to use the compiler-generated
 *  copy constructor and assignment operator for the
 *  InsetExternalParams class.
 */
class TempName {
public:
	TempName();
	TempName(TempName const &);
	~TempName();
	TempName & operator=(TempName const &);
	support::FileName operator()() const;
private:
	support::FileName tempname_;
};

} // namespace external


/// hold parameters settable from the GUI
class InsetExternalParams {
public:
	InsetExternalParams();

	void write(Buffer const &, std::ostream &) const;
	bool read(Buffer const &, Lexer &);

	/// The name of the tempfile used for manipulations.
	support::FileName tempname() const { return tempname_(); }

	/// The template currently in use.
	void settemplate(std::string const &);
	std::string const & templatename() const { return templatename_; }

	/// The external file.
	support::DocFileName filename;
	/// If the inset is to be displayed by LyX.
	bool display;
	/// If the inset is to use the preview mechanism.
	PreviewMode preview_mode;
	/// The scale of the displayed graphic (if shown).
	unsigned int lyxscale;

	external::ClipData     clipdata;
	external::ExtraData    extradata;
	external::ResizeData   resizedata;
	external::RotationData rotationdata;

	/** if \c true, simply output the filename, maybe wrapped in a
	 *  box, rather than generate and display the image etc.
	 */
	bool draft;

private:
	external::TempName tempname_;
	std::string templatename_;
};


class RenderBase;

///
class InsetExternal : public Inset
{
	// Disable assignment operator, since it is not used, and it is too
	// complicated to implement it consistently with the copy constructor
	InsetExternal & operator=(InsetExternal const &);
public:
	explicit InsetExternal(Buffer *);
	///
	~InsetExternal();
	///
	static void string2params(std::string const &, Buffer const &,
				  InsetExternalParams &);
	///
	static std::string params2string(InsetExternalParams const &,
					       Buffer const &);
	///
	InsetExternalParams const & params() const;
	///
	void setParams(InsetExternalParams const &);
	/// Update not loaded previews
	void updatePreview() const;
	/// \returns the number of rows (\n's) of generated code.
	void latex(otexstream &, OutputParams const &) const override;
	///
	std::string contextMenuName() const override;
	///
	bool setMouseHover(BufferView const * bv, bool mouse_hover) const override;
	///
	bool clickable(BufferView const &, int, int) const override { return true; }
	///
	void addToToc(DocIterator const & di, bool output_active,
				  UpdateType utype, TocBackend & backend) const override;
	///
	InsetCode lyxCode() const override { return EXTERNAL_CODE; }
	///
	bool hasSettings() const override { return true; }

	///
	int topOffset(BufferView const *) const override { return 0; }
	///
	int bottomOffset(BufferView const *) const override { return 0; }
	///
	int leftOffset(BufferView const *) const override { return 0; }
	///
	int rightOffset(BufferView const *) const override { return 0; }

	///
	void metrics(MetricsInfo &, Dimension &) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void write(std::ostream &) const override;
	///
	void read(Lexer & lex) override;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const override;
	///
	void generateXML(XMLStream &, OutputParams const &, std::string const &) const;
	///
	void docbook(XMLStream &, OutputParams const &) const override;
	/// For now, this does nothing. Someone who knows about this
	/// should see what needs doing for XHTML output.
	docstring xhtml(XMLStream &, OutputParams const &) const override;
	/// Update needed features for this inset.
	void validate(LaTeXFeatures & features) const override;
	///
	void addPreview(DocIterator const &, graphics::PreviewLoader &) const override;
	///
	bool showInsetDialog(BufferView * bv) const override;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const override;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	Inset * clone() const override { return new InsetExternal(*this); }
	/** Slot receiving a signal that the external file has changed
	 *  and the preview should be regenerated.
	 */
	void fileChanged() const;

private:
	///
	InsetExternal(InsetExternal const &);

	/// Is this inset using (instant or graphics) preview?
	bool isPreviewed() const;
	/// Do we have the right renderer (button, graphic or monitored preview)?
	bool isRendererValid() const;

	/// The current params
	InsetExternalParams params_;
	/// The thing that actually draws the image on LyX's screen.
	mutable unique_ptr<RenderBase> renderer_;
	/// changes color of the button when mouse enters/leaves this inset
	mutable std::map<BufferView const *, bool> mouse_hover_;
};

} // namespace lyx

#endif // INSET_EXTERNAL_H
