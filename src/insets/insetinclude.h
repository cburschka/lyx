// -*- C++ -*-
/**
 * \file insetinclude.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_INCLUDE_H
#define INSET_INCLUDE_H

#include "inset.h"
#include "insetcommandparams.h"
#include "render_button.h"
#include <boost/scoped_ptr.hpp>

class Buffer;
class Dimension;
struct LaTeXFeatures;
class RenderMonitoredPreview;


/// for including tex/lyx files
class InsetInclude: public InsetOld {
public:
	///
	InsetInclude(InsetCommandParams const &);
	InsetInclude(InsetInclude const &);
	~InsetInclude();

	///
	virtual std::auto_ptr<InsetBase> clone() const;

	/// Override these InsetButton methods if Previewing
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	virtual bool display() const { return true; }

	/// get the parameters
	InsetCommandParams const & params(void) const;

	///
	InsetOld::Code lyxCode() const { return InsetOld::INCLUDE_CODE; }
	/** Fills \c list
	 *  \param buffer the Buffer containing this inset.
	 *  \param list the list of labels in the child buffer.
	 */
	void getLabelList(Buffer const & buffer,
			  std::vector<std::string> & list) const;
	/** Fills \c keys
	 *  \param buffer the Buffer containing this inset.
	 *  \param keys the list of bibkeys in the child buffer.
	 */
	///
	void fillWithBibKeys(Buffer const & buffer,
		std::vector<std::pair<std::string,std::string> > & keys) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	void write(Buffer const &, std::ostream &) const;
	///
	void read(Buffer const &, LyXLex &);
	///
	int latex(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	int plaintext(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	int linuxdoc(Buffer const &, std::ostream &,
		     OutputParams const &) const;
	///
	int docbook(Buffer const &, std::ostream &,
		    OutputParams const &) const;
	///
	void validate(LaTeXFeatures &) const;
	///
	void addPreview(lyx::graphics::PreviewLoader &) const;
protected:
	///
	virtual void priv_dispatch(LCursor & cur, FuncRequest const & cmd);
private:
	/// Slot receiving a signal that the preview is ready to display.
	void statusChanged() const;
	/** Slot receiving a signal that the external file has changed
	 *  and the preview should be regenerated.
	 */
	void fileChanged() const;

	friend class InsetIncludeMailer;

	/// set the parameters
	void set(InsetCommandParams const & params, Buffer const &);
	/// get the text displayed on the button
	std::string const getScreenLabel(Buffer const &) const;
	///
	void write(std::ostream &) const;
	///
	void read(LyXLex &);

	/// the parameters
	InsetCommandParams params_;
	/// holds the entity name that defines the file location (SGML)
	std::string const include_label;

	/// The pointer never changes although *preview_'s contents may.
	boost::scoped_ptr<RenderMonitoredPreview> const preview_;

	/// cache
	mutable bool set_label_;
	mutable RenderButton button_;
};


#include "mailinset.h"

class InsetIncludeMailer : public MailInset {
public:
	///
	InsetIncludeMailer(InsetInclude & inset);
	///
	virtual InsetBase & inset() const { return inset_; }
	///
	virtual std::string const & name() const { return name_; }
	///
	virtual std::string const inset2string(Buffer const &) const;
	///
	static void string2params(std::string const &, InsetCommandParams &);
	///
	static std::string const params2string(InsetCommandParams const &);
private:
	///
	static std::string const name_;
	///
	InsetInclude & inset_;
};

#endif // INSETINCLUDE_H
