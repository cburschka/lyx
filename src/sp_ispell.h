#ifndef SP_ISPELL_H
#define SP_ISPELL_H

#include <cstdio>

#include "sp_base.h"

class BufferParams;

#ifndef CXX_GLOBAL_CSTD
using std::FILE;
#endif

class ISpell : public SpellBase
{
  public:

    enum ActualSpellChecker {
		ASC_ISPELL,
		ASC_ASPELL
    };

    ISpell();

    ISpell(BufferParams const & params, string const & lang);

    ~ISpell();


    /// initialize spell checker
    void initialize(BufferParams const & params, string const & lang);

    bool alive();

    /// clean up after error
    void cleanUp();

    /// check word
    enum spellStatus check(string const & word);

    /// close spellchecker
    void close();

    void insert(string const & word);

    void accept(string const & word);

    /// store replacement
    void store(string const & mis, string const & cor);

    char const * nextMiss();

    char const * error();

  private:

    ActualSpellChecker actual_spell_checker;

    /// instream to communicate with ispell
    FILE * in;

    /// outstream to communicate with ispell
    FILE * out;

    /// spell error
    char const * error_;

    /// ?
    int isp_fd;

    ///
    char * str;
    ///
    spellStatus flag;
    ///
    char * b;
    ///
    char * e;

};

#endif
