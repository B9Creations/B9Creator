#ifndef DLGMATERIALSMANAGER_H
#define DLGMATERIALSMANAGER_H

#include <QDialog>
#include <QStyledItemDelegate>
#include "b9matcat.h"


class MaterialsTableItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    MaterialsTableItemDelegate(QObject *parent = 0);
    ~MaterialsTableItemDelegate();

    virtual QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    virtual void setModelData ( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const;

};


namespace Ui {
class DlgMaterialsManager;
}

class DlgMaterialsManager : public QDialog
{
    Q_OBJECT
    
public:
    explicit DlgMaterialsManager(B9MatCat *pCatalog, QWidget *parent = 0);
    ~DlgMaterialsManager();
    
public slots:

    void setXY(int iXY){fillData(m_pCatalog->getCurMatIndex(),iXY);}

private slots:

    void on_comboBoxMaterial_currentIndexChanged(int index);

    void on_comboBoxXY_currentIndexChanged(int index);

    void on_pushButtonDelete_clicked();

    void on_pushButtonAdd_clicked();

    void on_buttonBoxSaveCancel_accepted();

    void on_pushButtonDuplicate_clicked();

    void on_buttonBoxSaveCancel_rejected();

    void on_doubleSpinBox_valueChanged(double arg1);

private:
    Ui::DlgMaterialsManager *ui;
    void setUp();
    void fillData(int iMatIndex, int iXYIndex);
    void stuffData();
    void updateEnabledStates();
    void addMaterial(int iMatIndex);
    void removeMaterial(int iMatIndex);
    B9MatCat* m_pCatalog;
    bool m_bLoading;
};

#endif // DLGMATERIALSMANAGER_H
