// -*- C++ -*-
/**
 * \file LyXAction.h
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 */

#ifndef LYXACTION_H
#define LYXACTION_H

#ifdef __GNUG__
#pragma interface
#endif

#include <map>

#include "funcrequest.h"
#include <boost/utility.hpp>

/**
 * This class is a container for LyX actions. It also
 * stores and managers "pseudo-actions". Pseudo-actions
 * are not part of the kb_action enum, but are created
 * dynamically, for encapsulating a real action and an
 * argument. They are used for things like the menus.
 */
class LyXAction : boost::noncopyable {
private:
	/// information for an action
	struct func_info {
		/// the action name
		string name;
		/// the func_attrib values set
		unsigned int attrib;
		/// the help text for this action
		string helpText;
	};

public:
	/// type for map between a function name and its action
	typedef std::map<string, kb_action> func_map;
	/// type for map between an action and its info
	typedef std::map<kb_action, func_info> info_map;
	/// type for a map between a pseudo-action and its stored action/arg
	typedef std::map<unsigned int, FuncRequest> pseudo_map;
	/// map from argument to pseudo-action
	typedef std::map<string, unsigned int> arg_item;
	/// map from an action to all its dependent pseudo-actions
	typedef std::map<kb_action, arg_item> arg_map;

	/// possible "permissions" for an action
	enum func_attrib {
		Noop = 0, //< nothing special about this func
		ReadOnly = 1, //< can be used in RO mode (perhaps this should change)
		NoBuffer = 2, //< Can be used when there is no document open
		Argument = 4 //< Requires argument
	};

	LyXAction();

	/**
	 * Returns an pseudoaction from a string
	 * If you include arguments in func_name, a new pseudoaction
	 * will be created if needed.
	 */
	int LookupFunc(string const & func_name);

	/// Returns a pseudo-action given an action and its argument.
	int getPseudoAction(kb_action action, string const & arg);

	/**
	 * Given a pseudo-action, return the real action and
	 * associated argument
	 */
	FuncRequest retrieveActionArg(int pseudo) const;

	/// Search for an existent pseudoaction, return -1 if it doesn't exist.
	int searchActionArg(kb_action action, string const & arg) const;

	/// Return the name (and argument) associated with the given (pseudo) action
	string const getActionName(int action) const;

	/// Return one line help text associated with (pseudo)action
	string const helpText(int action) const;

	/// True if the command has `flag' set
	bool funcHasFlag(kb_action action, func_attrib flag) const;

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
	void newFunc(kb_action, string const & name,
		     string const & helpText, unsigned int attrib);

	/**
	 * This is a list of all the LyXFunc names with the
	 * coresponding action number. It is usually only used by the
	 * minibuffer or when assigning commands to keys during init.
	 */
	func_map lyx_func_map;

	/**
	 * This is a mapping from action number to an object holding
	 * info about this action. f.ex. helptext, command name (string),
	 * command attributes (ro)
	 */
	info_map lyx_info_map;

	/**
	 * A mapping from the automatically created pseudo action number
	 * to the real action and its argument.
	 */
	pseudo_map lyx_pseudo_map;

	/**
	 * A (multi) mapping from the lyx action to all the generated
	 * pseudofuncs and the arguments the action should use.
	 */
	arg_map lyx_arg_map;
};

/// singleton instance
extern LyXAction lyxaction;
 
#endif // LYXACTION_H
