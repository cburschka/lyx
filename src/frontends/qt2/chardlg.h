/****************************************************************************
** Form interface generated from reading ui file 'chardlg.ui'
**
** Created: Thu Mar 1 12:56:14 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef CHARDLG_H
#define CHARDLG_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QPushButton;

class CharDlg : public QDialog
{ 
    Q_OBJECT

public:
    CharDlg( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~CharDlg();

    QGroupBox* sizeGB;
    QLabel* sizeL;
    QComboBox* size;
    QGroupBox* miscGB;
    QLabel* always_toggledL;
    QComboBox* misc;
    QGroupBox* charGB;
    QLabel* familyL;
    QComboBox* family;
    QLabel* seriesL;
    QLabel* langL;
    QComboBox* shape;
    QComboBox* color;
    QLabel* shapeL;
    QComboBox* series;
    QLabel* colorL;
    QComboBox* lang;
    QCheckBox* toggleall;
    QPushButton* applyPB;
    QPushButton* okPB;
    QPushButton* cancelPB;

protected slots:
    virtual void apply_adaptor();
    virtual void cancel_adaptor();
    virtual void close_adaptor();

protected:
    QGridLayout* CharDlgLayout;
    QHBoxLayout* sizeGBLayout;
    QHBoxLayout* miscGBLayout;
    QGridLayout* charGBLayout;
    QHBoxLayout* Layout9;
};

#endif // CHARDLG_H
