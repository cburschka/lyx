#ifndef SP_PSPELL_H
#define SP_PSPELL_H

#include "sp_base.h"

class PspellManager;
class PspellStringEmulation;
class PspellCanHaveError;

class BufferParams;


class PSpell : public SpellBase
{
  public:

    PSpell();
    
    PSpell(BufferParams const & params, string const & lang);
    
    ~PSpell();

    /// initialize spell checker
    void initialize(BufferParams const & params, string const & lang);

    bool alive() { return true; }

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
     
    void sigchldhandler(pid_t pid, int *status);

    char const * nextMiss();

    char const * error();
   
  private:

    PspellManager * sc;
    
    PspellStringEmulation * els;

    PspellCanHaveError * spell_error_object;

    spellStatus flag;

    const char * error_;
   
};
   
#endif


