#ifndef TEX2LYX_H
#define TEX2LYX_H

#include "texparser.h"

#include <iosfwd>
#include <string>
#include <vector>


void parse_preamble(Parser & p, std::ostream & os);

void parse_text(Parser & p, std::ostream & os, unsigned flags, bool outer);

void parse_table(Parser & p, std::ostream & os, unsigned flags);

void parse_math(Parser & p, std::ostream & os, unsigned flags, mode_type mode);

void handle_tabular(Parser & p, std::ostream & os);

// Helper
std::string parse_text(Parser & p, unsigned flags, const bool outer);

void handle_comment(Parser & p);
std::string const trim(std::string const & a, char const * p = " \t\n\r");

void split(std::string const & s, std::vector<std::string> & result, char delim = ',');
std::string join(std::vector<std::string> const & input, char const * delim);

bool is_math_env(std::string const & name);
char const ** is_known(string const & str, char const ** what);

// Access to environment stack
std::string curr_env();
void active_environments_push(std::string const & name);
void active_environments_pop();
bool active_environments_empty();

#endif
