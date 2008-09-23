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

class FuncRequest;
class LyXErr;

/**
 * This class is a container for LyX actions. It associates a name to
 * most of them and describes some of their properties.
 */
class LyXAction {
public:
	/// category of an action, used in the Shortcuts dialog
	enum func_type {
		Hidden,  //< Not listed for configuration
		Edit,    //< Cursor and mouse movement, copy/paste etc
		Math,    //< Mathematics
		Buffer,  //< Buffer and window related
		Layout,  //< Font, Layout and textclass related
		System,  //< Lyx preference, server etc
	};

private:
	/// information for an action
	struct FuncInfo {
		/// the action name
		std::string name;
		/// the func_attrib values set
		unsigned int attrib;
		/// the category of this func
		func_type type;
	};

public:
	/// noncopyable
	LyXAction(LyXAction const &);
	void operator=(LyXAction const &);

	/// type for map between a function name and its action
	typedef std::map<std::string, FuncCode> func_map;
	/// type for map between an action and its info
	typedef std::map<FuncCode, FuncInfo> info_map;

	/// possible "permissions" for an action
	enum func_attrib {
		Noop = 0, //< nothing special about this func
		ReadOnly = 1, //< can be used in RO mode (perhaps this should change); no automatic markDirty
		NoBuffer = 2, //< Can be used when there is no document open
		Argument = 4, //< Requires argument
		NoUpdate = 8, //< Does not (usually) require update
		SingleParUpdate = 16 //< Usually only requires this par updated
	};

	LyXAction();

	/**
	 * Returns an pseudoaction from a string
	 * If you include arguments in func_name, a new pseudoaction
	 * will be created if needed.
	 */
	FuncRequest lookupFunc(std::string const & func_name) const;

	/// Return the name (and argument) associated with the given (pseudo) action
	std::string const getActionName(FuncCode action) const;

	func_type const getActionType(FuncCode action) const;

	/// True if the command has `flag' set
	bool funcHasFlag(FuncCode action, func_attrib flag) const;

	/// iterator across all real actions
	typedef func_map::const_iterator const_func_iterator;

	/// return an iterator to the start of all real actions
	const_func_iterator func_begin() const;

	/// return an iterator to the end of all real actions
	const_func_iterator func_end() const;

private:
	/// populate the action container with our actions
	void init();
	/// add the given action
	void newFunc(FuncCode, std::string const & name, unsigned int attrib, func_type type);

	/**
	 * This is a list of all the LyXFunc names with the
	 * coresponding action number. It is usually only used by the
	 * minibuffer or when assigning commands to keys during init.
	 */
	func_map lyx_func_map;

	/**
	 * This is a mapping from action number to an object holding
	 * info about this action. f.ex. command name (string),
	 * command attributes (ro)
	 */
	info_map lyx_info_map;
};

LyXErr & operator<<(LyXErr &, FuncCode);

/// singleton instance
extern LyXAction lyxaction;


} // namespace lyx

#endif // LYXACTION_H
