// -*- C++ -*-
/**
 * \file ControlExternal.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLEXTERNAL_H
#define CONTROLEXTERNAL_H

#include "Dialog.h"

#include "support/types.h"

#include <boost/scoped_ptr.hpp>

#include <string>
#include <vector>


class InsetExternalParams;

namespace lyx {
namespace external {
class Template;
class RotationDataType;
} // namespace external
} // namespace lyx


class ControlExternal : public Dialog::Controller {
public:
	///
	ControlExternal(Dialog &);
	///
	virtual bool initialiseParams(std::string const & data);
	/// clean-up on hide.
	virtual void clearParams();
	/// clean-up on hide.
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }

	///
	InsetExternalParams const & params() const;
	///
	void setParams(InsetExternalParams const &);

	///
	void editExternal();
	///
	std::vector<std::string> const getTemplates() const;
	///
	int getTemplateNumber(std::string const &) const;
	///
	lyx::external::Template getTemplate(int) const;
	///
	std::string const browse(std::string const & input_file,
				 std::string const & tempalate_name) const;

	/// Read the Bounding Box from a eps or ps-file
	std::string const readBB(std::string const & file);
	///
	void bbChanged(bool val) { bb_changed_ = val; }
	bool bbChanged() const { return bb_changed_; }
private:
	///
	boost::scoped_ptr<InsetExternalParams> params_;
	bool bb_changed_;
};


namespace lyx {
namespace external {

std::vector<RotationDataType> const & all_origins();
std::string const origin_gui_str(lyx::size_type i);

} // namespace external
} // namespace lyx

#endif // CONTROLEXTERNAL_H
