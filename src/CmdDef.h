// -*- C++ -*-
/**
 * \file CmdDef.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bernhard Roider
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CMDDEF_H
#define CMDDEF_H

#include "FuncRequest.h"

#include "support/docstream.h"

#include <boost/shared_ptr.hpp>

#include <vector>
#include <deque>
#include <map>


namespace lyx {

/// Creates command definitions
class CmdDef {
private:
	/// information for a definition
	struct CmdDefInfo {
		CmdDefInfo(FuncRequest const & f): func(f), locked(false) {}
		/// the expanded FuncRequest
		FuncRequest func;
		/// to avoid recursive calls
		bool locked;
	};


	/// type for map between a macro name and its info
	typedef std::map<std::string, boost::shared_ptr<CmdDefInfo> > CmdDefMap;

public:

	/// Parse a def file
	bool read(std::string const & def_file);

	/**
	 * Look up a definition, lock it and return the
	 * associated action if it is not locked.
	 * @param name the name of the command
	 * @param func contains the action on success
	 * @return true if lock was successful
	 */
	bool lock(std::string const & name, FuncRequest & func);

	/// release a locked definition
	void release(std::string const & name);

private:

	/// possible reasons for not allowed definitions
	enum newCmdDefResult {
		CmdDefOk,
		CmdDefNameEmpty,
		CmdDefInvalid,
		CmdDefExists
	};

	/**
	 * Add a new command definition.
	 * @param name internal recursion level
	 */
	newCmdDefResult newCmdDef(std::string const & name, 
		std::string const & def);

	///
	CmdDefMap cmdDefMap;
};

/// Implementation is in LyX.cpp
extern CmdDef & theTopLevelCmdDef();


} // namespace lyx

#endif // CMDDEF_H
