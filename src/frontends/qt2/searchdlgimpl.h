#ifndef SEARCHDLGIMPL_H
#define SEARCHDLGIMPL_H

#include "searchdlg.h"
#include "FormSearch.h"

class QCloseEvent;

class SearchDlgImpl : public SearchDlg
{ 
    Q_OBJECT


 public:
    
   SearchDlgImpl(FormSearch * form, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    
   ~SearchDlgImpl();
   
   void setReadOnly(bool);
   
   void Replace(bool);
 
 protected:
   
   void closeEvent(QCloseEvent * e);

 private:
   
   FormSearch * form_;

 protected slots:
    
   void Find();
    
   void Replace() {
      Replace(false);
   };
    
   void ReplaceAll() {
      Replace(true);
   };
   
   void cancel_adaptor() {
      form_->close();
      hide();
   }

};

#endif // SEARCHDLGIMPL_H
