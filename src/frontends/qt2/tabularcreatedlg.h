/****************************************************************************
** Form interface generated from reading ui file 'tabularcreatedlg.ui'
**
** Created: Tue Feb 6 01:41:06 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef INSERTTABULARDLG_H
#define INSERTTABULARDLG_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class EmptyTable;
class QLabel;
class QPushButton;
class QSpinBox;

class InsertTabularDlg : public QDialog
{ 
    Q_OBJECT

public:
    InsertTabularDlg( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~InsertTabularDlg();

    QLabel* rowsL;
    QSpinBox* rows;
    QLabel* columnsL;
    QSpinBox* columns;
    EmptyTable* table;
    QPushButton* insertPB;
    QPushButton* cancelPB;

protected slots:
    virtual void insert_tabular();
    virtual void cancel_adaptor();

protected:
    QVBoxLayout* InsertTabularDlgLayout;
    QHBoxLayout* Layout1;
    QHBoxLayout* Layout2;
};

#endif // INSERTTABULARDLG_H
