/**
 * \file FormForks.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 * \date 2001-10-22
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "FormForks.h"
#include "ControlForks.h"
#include "form_forks.h"
#include "Tooltips.h"
#include "helper_funcs.h"
#include "xforms_helpers.h"
#include "gettext.h"
#include "support/lstrings.h"

using std::vector;
using std::find;
using std::find_if;

typedef FormCB<ControlForks, FormDB<FD_form_forks> > base_class;

FormForks::FormForks(ControlForks & c)
	: base_class(c, _("Child processes"))
{}


void FormForks::build() {
	dialog_.reset(build_forks());

	// It appears that the browsers aren't initialised properly.
	// This fudge fixes tings.
	fl_add_browser_line(dialog_->browser_children, " ");
	fl_add_browser_line(dialog_->browser_kill, " ");
	fl_clear_browser(dialog_->browser_children);
	fl_clear_browser(dialog_->browser_kill);

        // Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);
	bc().invalid();

	// Set up the tooltip mechanism
	string str = N_("All currently running child processes forked by LyX.");
	tooltips().initTooltip(dialog_->browser_children, str);

	str = N_("A list of all child processes to kill.");
	tooltips().initTooltip(dialog_->browser_kill, str);

	str = N_("Add all processes to the list of processes to kill.");
	tooltips().initTooltip(dialog_->button_all, str);

	str = N_("Add the currently selected child process to the list of processes to kill.");
	tooltips().initTooltip(dialog_->button_add, str);

	str = N_("Remove the currently selected item from the list of processes to kill.");
	tooltips().initTooltip(dialog_->button_remove, str);
}


void FormForks::update()
{
	if (!form())
		return;

	string const current_pid_str =
		getSelectedStringFromBrowser(dialog_->browser_kill);
	pid_t const current_pid = strToInt(current_pid_str);

	vector<pid_t> pids = controller().getPIDs();

	// No child processes.
	if (pids.empty()) {
		if (fl_get_browser_maxline(dialog_->browser_kill) > 0)
			fl_clear_browser(dialog_->browser_kill);
		if (fl_get_browser_maxline(dialog_->browser_children) > 0)
			fl_clear_browser(dialog_->browser_children);

		setEnabled(dialog_->browser_children, false);
		setEnabled(dialog_->browser_kill,     false);
		setEnabled(dialog_->button_all,       false);
		setEnabled(dialog_->button_add,       false);
		setEnabled(dialog_->button_remove,    false);

		return;
	}

	// Remove any processes from the kill browser that aren't in the
	// vector of existing PIDs.
	for (int i = 1; i <= fl_get_browser_maxline(dialog_->browser_kill);
	     ++i) {
		string const pid_str =
			getStringFromBrowser(dialog_->browser_kill, i);
		pid_t const pid = strToInt(pid_str);
		vector<pid_t>::const_iterator it =
			find(pids.begin(), pids.end(), pid);
		if (it == pids.end())
			fl_delete_browser_line(dialog_->browser_kill, i);
	}

	// Build the children browser from scratch.
	if (fl_get_browser_maxline(dialog_->browser_children) > 0)
		fl_clear_browser(dialog_->browser_children);
	int i = 1;
	for (vector<pid_t>::const_iterator it = pids.begin();
	     it != pids.end(); ++it) {
		string const pid_str = tostr(*it);
		string const command = controller().getCommand(*it);
		string const line = pid_str + '\t' + command;

		fl_add_browser_line(dialog_->browser_children, line.c_str());

		if (*it == current_pid)
			fl_select_browser_line(dialog_->browser_children, i);
		++i;
	}

	setEnabled(dialog_->browser_children, true);
	setEnabled(dialog_->button_all,       true);
	setEnabled(dialog_->button_add,       true);
}


void FormForks::apply()
{
	// Get the list of all processes to kill.
	vector<string> const kill_vec =
		getVectorFromBrowser(dialog_->browser_kill);

	if (kill_vec.empty())
		return;

	// Remove these items from the vector of child processes.
	for (int i = 1; i <= fl_get_browser_maxline(dialog_->browser_children);
	     ++i) {
		string const selection =
			getStringFromBrowser(dialog_->browser_children, i);
		string pid_str;
		split(selection, pid_str, '\t');

		vector<string>::const_iterator it =
			find(kill_vec.begin(), kill_vec.end(), pid_str);

		if (it != kill_vec.end())
			fl_delete_browser_line(dialog_->browser_children, i);
	}

	// Clear the kill browser and deactivate appropriately.
	fl_clear_browser(dialog_->browser_kill);
	setEnabled(dialog_->browser_kill,  false);
	setEnabled(dialog_->button_remove, false);

	// Pass these pids to the controller for destruction.
	for (vector<string>::const_iterator it = kill_vec.begin();
	     it != kill_vec.end(); ++it) {
		pid_t const pid = strToInt(*it);
		controller().kill(pid);
	}

}


ButtonPolicy::SMInput FormForks::input(FL_OBJECT * ob, long)
{
	ButtonPolicy::SMInput activate = ButtonPolicy::SMI_NOOP;

	if (ob == dialog_->browser_children) {
		activate = input_browser_children();
		
	} else if (ob == dialog_->browser_kill) {
		activate = input_browser_kill();

	} else if (ob == dialog_->button_all) {
		activate = input_button_all();

	} else if (ob == dialog_->button_add) {
		activate = input_button_add();

	} else if (ob == dialog_->button_remove) {
		activate = input_button_remove();
	}

	return activate;
}

ButtonPolicy::SMInput FormForks::input_browser_children()
{
	// Selected an item in the browser containing a list of all child
	// processes.

	// 1. Highlight this item in the browser of processes to kill
	//    if it is already there.

	// 2. If it is there, enable the remove button so that it can
	//    be removed from this list, if so desired.

	// 3. If it isn't there, activate the add button so that it can
	//    be added to this list if so desired.

	string const selection =
		getSelectedStringFromBrowser(dialog_->browser_children);
	string pid_str;
	split(selection, pid_str, '\t');

	vector<string> const kill_vec = 
		getVectorFromBrowser(dialog_->browser_kill);

	vector<string>::const_iterator it =
		find(kill_vec.begin(), kill_vec.end(), pid_str);

	fl_deselect_browser(dialog_->browser_kill);
	if (it != kill_vec.end()) {
		int const n = int(it - kill_vec.begin());
		fl_select_browser_line(dialog_->browser_kill, n+1);
		fl_set_browser_topline(dialog_->browser_kill, n+1);
	}

	setEnabled(dialog_->button_remove, it != kill_vec.end());
	setEnabled(dialog_->button_add,    it == kill_vec.end());

	return ButtonPolicy::SMI_NOOP;
}


namespace {

class FindPID {
public:
	FindPID(string const & pid) : pid_(pid) {}
	bool operator()(string const & line)
	{
		if (line.empty())
			return false;

		string pid_str;
		split(line, pid_str, '\t');
		return pid_str == pid_;
	}

private:
	string pid_;
};
 
} // namespace anon


ButtonPolicy::SMInput FormForks::input_browser_kill()
{
	// Selected an item in the browser containing a list of processes
	// to kill.

	// 1. Highlight this item in the browser of all child processes.

	// 2. Enable the remove button so that it can removed from this list,
	//    if so desired.

	// 3. Disable the add button.

	string const pid_str =
		getSelectedStringFromBrowser(dialog_->browser_kill);

	// Find this string in the list of all child processes
	vector<string> const child_vec =
		getVectorFromBrowser(dialog_->browser_children);

	vector<string>::const_iterator it =
		find_if(child_vec.begin(), child_vec.end(), FindPID(pid_str));

	fl_deselect_browser(dialog_->browser_children);
	if (it != child_vec.end()) {
		int const n = int(it - child_vec.begin());
		fl_select_browser_line(dialog_->browser_children, n+1);
		fl_set_browser_topline(dialog_->browser_children, n+1);
	}
		
	setEnabled(dialog_->button_remove, true);
	setEnabled(dialog_->button_add,    false);

	return ButtonPolicy::SMI_NOOP;
}


namespace {

vector<string> const getPIDvector(FL_OBJECT * ob)
{
	vector<string> vec = getVectorFromBrowser(ob);
	if (vec.empty())
		return vec;

	for (vector<string>::iterator it = vec.begin(); it != vec.end(); ++it) {
		string pid_str;
		split(*it, pid_str, '\t');
		*it = pid_str;
	}

	return vec;
}
 
} // namespace anon


ButtonPolicy::SMInput FormForks::input_button_all()
{
	// Pressed the "All" button.

	// 1. Check that the browser of processes to kill doesn't already
	//    contain the entire list.

	// 2. If it doesn't, copy the PIDs of all child processes into the
	//    browser of processes to kill.

	// 3. Deactivate the "children" browser and the "add" and "all" buttons

	// 4. Activate the "kill" browser and the "remove" button"

	ButtonPolicy::SMInput activate = ButtonPolicy::SMI_NOOP;

	vector<string> const pid_vec = getPIDvector(dialog_->browser_children);

	// to resolve a warning about comparison between signed and unsigned.
	int const pid_vec_size = int(pid_vec.size());
	
	if (fl_get_browser_maxline(dialog_->browser_kill) != pid_vec_size) {
		activate = ButtonPolicy::SMI_VALID;

		fl_clear_browser(dialog_->browser_kill);
		for (vector<string>::const_iterator it = pid_vec.begin();
		     it != pid_vec.end(); ++it) {
			fl_add_browser_line(dialog_->browser_kill, it->c_str());
		}

		if (fl_get_browser_maxline(dialog_->browser_kill) >= 1)
			fl_set_browser_topline(dialog_->browser_kill, 1);
	}

	setEnabled(dialog_->browser_children, false);
	setEnabled(dialog_->button_add,       false);
	setEnabled(dialog_->button_all,       false);
	setEnabled(dialog_->browser_kill,     true);
	setEnabled(dialog_->button_remove,    true);

	return activate;
}


ButtonPolicy::SMInput FormForks::input_button_add()
{
	// Pressed the "Add" button.

	// 1. Copy the PID of the selected item in the browser of all child
	//    processes over into the browser of processes to kill.

	// 2. Activate the "kill" browser and the "remove" button.

	// 3. Deactivate the "add" button.

	string const selection =
		getSelectedStringFromBrowser(dialog_->browser_children);
	string pid_str;
	split(selection, pid_str, '\t');

	vector<string> const kill_vec = 
		getVectorFromBrowser(dialog_->browser_kill);

	vector<string>::const_iterator it =
		find(kill_vec.begin(), kill_vec.end(), pid_str);

	if (it == kill_vec.end()) {
		fl_add_browser_line(dialog_->browser_kill, pid_str.c_str());
		int const n = fl_get_browser_maxline(dialog_->browser_kill);
		fl_select_browser_line(dialog_->browser_kill, n);
	}

	setEnabled(dialog_->browser_kill,  true);
	setEnabled(dialog_->button_remove, true);
	setEnabled(dialog_->button_add,    false);

	return ButtonPolicy::SMI_VALID;
}


ButtonPolicy::SMInput FormForks::input_button_remove()
{
	// Pressed the "Remove" button.

	// 1. Remove the selected item in the browser of processes to kill.

	// 2. Activate the "add" button and "all" buttons.

	// 3. Deactivate the "remove" button.

	int const sel = fl_get_browser(dialog_->browser_kill);
	fl_delete_browser_line(dialog_->browser_kill, sel);

	setEnabled(dialog_->button_add,    true);
	setEnabled(dialog_->button_all,    true);
	setEnabled(dialog_->button_remove, false);

	return ButtonPolicy::SMI_VALID;
}
