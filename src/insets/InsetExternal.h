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
#include "ExternalTransforms.h"
#include "EmbeddedFiles.h"

#include "support/FileName.h"
#include "support/Translator.h"

#include <boost/scoped_ptr.hpp>
#include <boost/signals/trackable.hpp>


/** No two InsetExternalParams variables can have the same temporary file.
 *  This class has copy-semantics but the copy constructor
 *  and assignment operator simply call the default constructor.
 *  Use of this class enables us to use the compiler-generated
 *  copy constructor and assignment operator for the
 *  InsetExternalParams class.
 */
namespace lyx {

namespace external {

class TempName {
public:
	TempName();
	TempName(TempName const &);
	~TempName();
	TempName & operator=(TempName const &);
	support::FileName const & operator()() const { return tempname_; }
private:
	support::FileName tempname_;
};

/// How is the image to be displayed on the LyX screen?
enum DisplayType {
	DefaultDisplay,
	MonochromeDisplay,
	GrayscaleDisplay,
	ColorDisplay,
	PreviewDisplay,
	NoDisplay
};


/// The translator between the Display enum and corresponding lyx string.
Translator<DisplayType, std::string> const & displayTranslator();

} // namespace external


/// hold parameters settable from the GUI
class InsetExternalParams {
public:
	InsetExternalParams();

	void write(Buffer const &, std::ostream &) const;
	bool read(Buffer const &, Lexer &);

	/// The name of the tempfile used for manipulations.
	support::FileName const & tempname() const { return tempname_(); }

	/// The template currently in use.
	void settemplate(std::string const &);
	std::string const & templatename() const { return templatename_; }

	/// The external file.
	EmbeddedFile filename;
	/// How the inset is to be displayed by LyX.
	external::DisplayType display;
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
public:
	InsetExternal(Buffer &);
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
	///
	void setBuffer(Buffer & buffer);
	/// \returns the number of rows (\n's) of generated code.
	int latex(odocstream &, OutputParams const &) const;

private:
	///
	InsetExternal(InsetExternal const &);
	///
	InsetCode lyxCode() const { return EXTERNAL_CODE; }
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(std::ostream &) const;
	///
	void read(Lexer & lex);
	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	/// Update needed features for this inset.
	void validate(LaTeXFeatures & features) const;
	///
	void addPreview(graphics::PreviewLoader &) const;
	///
	void edit(Cursor & cur, bool front, EntryDirection entry_from);
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	/// external file can be embedded
	void registerEmbeddedFiles(EmbeddedFileList &) const;
	///
	void updateEmbeddedFile(EmbeddedFile const &);
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
	boost::scoped_ptr<RenderBase> renderer_;
};

} // namespace lyx

#endif // INSET_EXTERNAL_H
