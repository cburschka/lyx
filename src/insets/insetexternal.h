// -*- C++ -*-
/**
 * \file insetexternal.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_EXTERNAL_H
#define INSET_EXTERNAL_H

#include "inset.h"
#include "ExternalTransforms.h"

#include "support/filename.h"
#include "support/translator.h"

#include <boost/scoped_ptr.hpp>
#include <boost/signals/trackable.hpp>


/** No two InsetExternalParams variables can have the same temporary file.
 *  This struct has copy-semantics but the copy constructor
 *  and assignment operator simply call the default constructor.
 *  Use of this struct enables us to use the compiler-generated
 *  copy constructor and assignment operator for the
 *  InsetExternalParams struct.
 */
namespace lyx {
namespace external {

struct TempName {
	TempName();
	TempName(TempName const &);
	~TempName();
	TempName & operator=(TempName const &);
	std::string const & operator()() const { return tempname_; }
private:
	std::string tempname_;
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
} // namespace lyx


/// hold parameters settable from the GUI
struct InsetExternalParams {
	InsetExternalParams();

	void write(Buffer const &, std::ostream &) const;
	bool read(Buffer const &, LyXLex &);

	/// The name of the tempfile used for manipulations.
	std::string const & tempname() const { return tempname_(); }

	/// The template currently in use.
	void settemplate(std::string const &);
	std::string const & templatename() const { return templatename_; }

	/// The external file.
	lyx::support::FileName filename;
	/// How the inset is to be displayed by LyX.
	lyx::external::DisplayType display;
	/// The scale of the displayed graphic (if shown).
	unsigned int lyxscale;

	lyx::external::ClipData     clipdata;
	lyx::external::ExtraData    extradata;
	lyx::external::ResizeData   resizedata;
	lyx::external::RotationData rotationdata;

	/** if \c true, simply output the filename, maybe wrapped in a
	 *  box, rather than generate and display the image etc.
	 */
	bool draft;

private:
	lyx::external::TempName tempname_;
	std::string templatename_;
};


class RenderBase;

///
class InsetExternal : public InsetOld, public boost::signals::trackable
{
public:
	InsetExternal();
	///
	InsetExternal(InsetExternal const &);
	///
	virtual ~InsetExternal();
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	virtual InsetOld::Code lyxCode() const { return EXTERNAL_CODE; }
	///
	virtual EDITABLE editable() const { return IS_EDITABLE; }

	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	virtual void write(Buffer const &, std::ostream &) const;
	///
	virtual void read(Buffer const &, LyXLex & lex);

	/// \returns the number of rows (\n's) of generated code.
	virtual int latex(Buffer const &, std::ostream &,
			  OutputParams const &) const;
	///
	virtual int plaintext(Buffer const &, std::ostream &,
			  OutputParams const &) const;
	///
	virtual int linuxdoc(Buffer const &, std::ostream &,
			     OutputParams const &) const;
	///
	virtual int docbook(Buffer const &, std::ostream &,
			    OutputParams const &) const;

	/// Update needed features for this inset.
	virtual void validate(LaTeXFeatures & features) const;

	///
	InsetExternalParams const & params() const;
	void setParams(InsetExternalParams const &, Buffer const &);
	///
	void addPreview(lyx::graphics::PreviewLoader &) const;
	///
	void edit(LCursor & cur, bool);

protected:
	///
	virtual
	DispatchResult
	priv_dispatch(LCursor & cur, FuncRequest const & cmd);
private:
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


#include "mailinset.h"

class InsetExternalMailer : public MailInset {
public:
	///
	InsetExternalMailer(InsetExternal & inset);
	///
	virtual InsetBase & inset() const { return inset_; }
	///
	virtual std::string const & name() const { return name_; }
	///
	virtual std::string const inset2string(Buffer const &) const;
	///
	static void string2params(std::string const &, Buffer const &,
				  InsetExternalParams &);
	///
	static std::string const params2string(InsetExternalParams const &,
					       Buffer const &);
private:
	///
	static std::string const name_;
	///
	InsetExternal & inset_;
};

#endif
