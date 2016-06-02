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

#include <boost/signals/trackable.hpp>


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
class InsetExternal : public Inset, public boost::signals::trackable
{
	// Disable assignment operator, since it is not used, and it is too
	// complicated to implement it consistently with the copy constructor
	InsetExternal & operator=(InsetExternal const &);
public:
	InsetExternal(Buffer *);
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
	void updatePreview();
	/// \returns the number of rows (\n's) of generated code.
	void latex(otexstream &, OutputParams const &) const;
	///
	std::string contextMenuName() const;
	///
	bool setMouseHover(BufferView const * bv, bool mouse_hover) const;
	///
	bool clickable(BufferView const &, int, int) const { return true; }
	///
	void addToToc(DocIterator const & di, bool output_active,
				  UpdateType utype) const;
private:
	///
	InsetExternal(InsetExternal const &);
	///
	InsetCode lyxCode() const { return EXTERNAL_CODE; }
	///
	bool hasSettings() const { return true; }
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(std::ostream &) const;
	///
	void read(Lexer & lex);
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	/// For now, this does nothing. Someone who knows about this
	/// should see what needs doing for XHTML output.
	docstring xhtml(XHTMLStream &, OutputParams const &) const;
	/// Update needed features for this inset.
	void validate(LaTeXFeatures & features) const;
	///
	void addPreview(DocIterator const &, graphics::PreviewLoader &) const;
	///
	bool showInsetDialog(BufferView * bv) const;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	Inset * clone() const { return new InsetExternal(*this); }
	/** This method is connected to the graphics loader, so we are
	 *  informed when the image has been loaded.
	 */
	void statusChanged() const;
	/** Slot receiving a signal that the external file has changed
	 *  and the preview should be regenerated.
	 */
	void fileChanged() const;

	/// The current params
	InsetExternalParams params_;
	/// The thing that actually draws the image on LyX's screen.
	unique_ptr<RenderBase> renderer_;
	/// changes color of the button when mouse enters/leaves this inset
	mutable std::map<BufferView const *, bool> mouse_hover_;
};

} // namespace lyx

#endif // INSET_EXTERNAL_H
