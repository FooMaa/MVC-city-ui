#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QShortcut>
#include <QDialog>
#include <QSortFilterProxyModel>

#include "datamodellist.h"
#include "datamodeltree.h"
#include "datamodeltable.h"
#include "connector.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class Widget;
    class Dialog;
}
QT_END_NAMESPACE

class Dialog;

class Widget : public QWidget
{
    Q_OBJECT

    QObject *for_repeat_objects;
    QShortcut *shortcut_for_table;

    Dialog *dialog_window;

    DataModelTree *model_tree;
    DataModelList *model_list;
    DataModelTable *model_table;
    QSortFilterProxyModel *model_proxy_sort;

    QModelIndex current_index_list;
    QModelIndex current_index_tree;


public:
    Widget(QWidget *parent = nullptr);  
    ~Widget();
    template <typename CHILD, typename PARENT> void insertObjectInTree();


public slots:
    void slot_insertValueFromLineEdit();
    void slot_insertToLineFromTree();
    void slot_insertToLineFromList();
    void slot_clickSort();
    void slot_seeText();
    void slot_realiseTable();
    void slot_listChangeAfterDrop();
    void slot_treeChangeAfterDrop();
    void slot_changeNameAfterDrop();
    void slot_treeChangeAfterAdd();
    void slot_changeNameAfterAdd();


private:
    Ui::Widget *ui;
};

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr, QString name_object = "", int type_changing = 0);
    ~Dialog();
    QString getTextFromLineEdit();


private:
    Ui::Dialog *ui;
};

#endif // WIDGET_H
