/**
 * \file tabularcreatedlgimpl.h
 * Copyright 2001 LyX Team
 * see the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#ifndef TABULARCREATEDLGIMPL_H
#define TABULARCREATEDLGIMPL_H
#include "tabularcreatedlg.h"

class FormTabularCreate;

class TabularCreateDlgImpl : public InsertTabularDlg
{ 
    Q_OBJECT

public:
    TabularCreateDlgImpl(FormTabularCreate *form, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~TabularCreateDlgImpl();

protected slots:
   void insert_tabular();
   void cancel_adaptor();
   virtual void colsChanged(int);
   virtual void rowsChanged(int);
   
private:
   FormTabularCreate * form_;
};

#endif // TABULARCREATEDLGIMPL_H
