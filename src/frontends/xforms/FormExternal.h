// -*- C++ -*-
/**
 * \file FormExternal.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORMEXTERNAL_H
#define FORMEXTERNAL_H

#include "FormDialogView.h"
#include <map>

namespace lyx {
namespace frontend {

class ControlExternal;
struct FD_external;
struct FD_external_file;
struct FD_external_lyxview;
struct FD_external_rotate;
struct FD_external_scale;
struct FD_external_crop;
struct FD_external_options;

/// The class for editing External insets via a dialog
class FormExternal
	: public FormController<ControlExternal, FormView<FD_external> > {
public:
	///
	FormExternal(Dialog &);

	typedef std::map<std::string, std::string> MapType;

	enum Tabs {
		FILETAB,
		LYXVIEWTAB,
		ROTATETAB,
		SCALETAB,
		CROPTAB,
		OPTIONSTAB
	};

	typedef std::map<Tabs, FL_OBJECT *> TabMap;

private:
	/// apply changes
	virtual void apply();

	/// build the dialog
	virtual void build();

	/// update the dialog
	virtual void update();

	/// Filter the inputs on callback from xforms
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);

	bool activateAspectratio() const;
	void getBB();
	void updateComboChange();
	void widthUnitChanged();

	MapType extra_;

	TabMap tabmap_;

	/// Real GUI implementation.
	boost::scoped_ptr<FD_external_file>    file_;
	boost::scoped_ptr<FD_external_lyxview> lyxview_;
	boost::scoped_ptr<FD_external_rotate>  rotate_;
	boost::scoped_ptr<FD_external_scale>   scale_;
	boost::scoped_ptr<FD_external_crop>    crop_;
	boost::scoped_ptr<FD_external_options> options_;
};

} // namespace frontend
} // namespace lyx

#endif // FORMEXTERNAL_H
