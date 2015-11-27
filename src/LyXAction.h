// -*- C++ -*-
/**
 * \file LyXAction.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYXACTION_H
#define LYXACTION_H

#include "FuncCode.h"

#include <map>
#include <string>


namespace lyx {

// current LFUN format
static unsigned int const LFUN_FORMAT = 3;

class FuncRequest;
class LyXErr;

/**
 * This class is a container for LyX actions. It associates a name to
 * most of them and describes some of their properties.
 */
class LyXAction {
public:
	/// category of an action, used in the Shortcuts dialog
	enum FuncType {
		Hidden,  //< Not listed for configuration
		Edit,    //< Cursor and mouse movement, copy/paste etc
		Math,    //< Mathematics
		Buffer,  //< Buffer and window related
		Layout,  //< Font, Layout and textclass related
		System   //< Lyx preference, server etc
	};

private:
	/// information for an action
	struct FuncInfo {
		/// the action name
		std::string name;
		/// the FuncAttribs values set
		unsigned int attrib;
		/// the category of this func
		FuncType type;
	};
	/// type for map between a function name and its action
	typedef std::map<std::string, FuncCode> FuncMap;
	/// type for map between an action and its info
	typedef std::map<FuncCode, FuncInfo> InfoMap;

public:
	/// possible "permissions" for an action
	enum FuncAttribs {
		Noop = 0, //< Nothing special about this func
		ReadOnly = 1, //< Can be used in RO mode (perhaps this should change)
		NoBuffer = 2, //< Can be used when there is no document open
		Argument = 4, //< Requires argument
		NoUpdate = 8, //< Does not (usually) require update
		SingleParUpdate = 16, //< Usually only requires this par updated
		AtPoint = 32, //< dispatch first to inset at cursor if there is one
		NoInternal = 64 //< Cannot be used for internal, non-document Buffers
	};

	LyXAction();

	/**
	 * Creates a FuncRequest from a string of the form:
	 *   lyx-function [argument]
	 * where the argument is optional and "lyx-function" is in the form you'd
	 * enter it in the mini-buffer. 
	 */
	FuncRequest lookupFunc(std::string const & func_name) const;

	/// Return the command name associated with the given action
	/// Thus: getActionName(LFUN_ERT_INSERT) --> "ert-insert".
	std::string const getActionName(FuncCode action) const;
	///
	FuncType getActionType(FuncCode action) const;

	/// True if the command has `flag' set
	bool funcHasFlag(FuncCode action, FuncAttribs flag) const;

	/// iterator across all LFUNs
	typedef FuncMap::const_iterator const_iterator;

	/// return an iterator to the start of the list of LFUNs
	const_iterator func_begin() const;

	/// return an iterator to one past the end of the list of LFUNs
	const_iterator func_end() const;

private:
	/// noncopyable
	LyXAction(LyXAction const &);
	void operator=(LyXAction const &);

	/// populate the action container with our actions
	void init();
	/// add the given action
	void newFunc(FuncCode, std::string const & name, unsigned int attrib, FuncType type);

	/**
	 * This maps LyX function names to function codes, e.g.:
	 *   lyx_func_map["ert-insert"] == LFUN_ERT_INSERT
	 */
	FuncMap lyx_func_map;

	/**
	 * This maps function codes to objects holding info about the corresponding
	 * action. E.g., if
	 *   FuncInfo const & ert = lyx_info_map[LFUN_ERT_INSERT];
	 * then:
	 *   ert.name   == "ert-insert"'
	 *   ert.attrib == Noop
	 *   ert.type   == Edit
	 */
	InfoMap lyx_info_map;
};

LyXErr & operator<<(LyXErr &, FuncCode);

/// singleton instance
extern LyXAction lyxaction;


} // namespace lyx

#endif // LYXACTION_H
