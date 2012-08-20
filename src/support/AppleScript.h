// -*- C++ -*-
/**
 * \file AppleScript.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Benjamin Piwowarski
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_SUPPORT_APPLESCRIPT_H
#define LYX_SUPPORT_APPLESCRIPT_H

#ifdef __cplusplus 
extern "C" {
#endif
    /// What is returned by applescript_execute_command
    typedef struct {
        int code;
        char *message;
    } LyXFunctionResult;

    LyXFunctionResult applescript_execute_command(const char *cmd, const char *args);

    /// Sets up apple script support
    void setupApplescript();
#ifdef __cplusplus 
}
#endif

#endif