/**
 * \file chardlgimpl.h
 * Copyright 2001 LyX Team
 * see the file COPYING
 *
 * \author Edwin Leuven
 */

#ifndef CHARDLGIMPL_H
#define CHARDLGIMPL_H

#include "chardlg.h"
#include "FormCharacter.h"
#include <qevent.h>
#include <qcheckbox.h>
#include <qcombobox.h>

class LyXFont;
class FormCharacter;

class CharDlgImpl : public CharDlg
{ 
    Q_OBJECT


 public:
    
   CharDlgImpl(FormCharacter * form, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    
   ~CharDlgImpl();
   
   LyXFont getChar();
   
   int langItem() { 
      return lang->currentItem();
   };
   
   bool toggleAll() { 
      return toggleall->isChecked(); 
   };
   
   void setReadOnly(bool);
   
 protected:
   
   void closeEvent(QCloseEvent * e);


 private:
   
   FormCharacter * form_;
   

 protected slots:
   
   void apply_adaptor() {
      form_->apply();
   }
    
   void cancel_adaptor() {
      form_->close();
      hide();
   }
    
   void close_adaptor() {
      form_->apply();
      form_->close();
      hide();
   }

};

#endif // CHARDLGIMPL_H
