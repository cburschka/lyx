// -*- C++ -*-
/**
 * \file InsetCaptionable.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 * \author Guillaume Munch
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_CAPTIONABLE_H
#define INSET_CAPTIONABLE_H

#include "InsetCollapsible.h"


namespace lyx {

class InsetCaption;

class InsetCaptionable : public InsetCollapsible
{
public:
	InsetCaptionable(Buffer * buffer, std::string const & type = "senseless")
		: InsetCollapsible(buffer), caption_type_(type) {}
	///
	std::string const & captionType() const { return caption_type_; }
	///
	docstring floatName(std::string const & type) const;
	///
protected:
	///
	InsetCaption const * getCaptionInset() const;
	///
	docstring getCaptionText(OutputParams const &) const;
	///
	docstring getCaptionHTML(OutputParams const &) const;
	///
	docstring getCaptionDocBook(OutputParams const &) const;
	///
	virtual void setCaptionType(std::string const & type);
	/// are our captions subcaptions?
	virtual bool hasSubCaptions(ParIterator const &) const { return false; }
	///
	void addToToc(DocIterator const & di, bool output_active,
				  UpdateType utype, TocBackend & backend) const override;
	/// Update the counters of this inset and of its contents
	void updateBuffer(ParIterator const &, UpdateType, bool const deleted = false) override;
	///
	bool insetAllowed(InsetCode) const override;
private:
	///
	std::string caption_type_;
};


} // namespace lyx

#endif // INSET_CAPTIONABLE_H
