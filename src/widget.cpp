#include "headers/widget.h"
#include "ui_widget.h"
#include "ui_dialog.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    model_tree = new DataModelTree(this);
    model_list = new DataModelList(this);
    model_table = new DataModelTable(0, this);
    model_proxy_sort = new QSortFilterProxyModel(this);


    ui->view_tree->setModel(model_tree);
    model_proxy_sort->setSourceModel(model_list);
    ui->view_list->setModel(model_proxy_sort);
    ui->view_table->setModel(model_table);
    ui->view_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    shortcut_for_table = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_0), this);

    ui->view_tree->setCurrentIndex(ui->view_tree->rootIndex());

    connect(ui->btn_add, &QPushButton::clicked, this, &Widget::slot_insertValueFromLineEdit);
    connect(ui->btn_sort, &QPushButton::clicked, this, &Widget::slot_clickSort);
    connect(ui->view_tree, &QTreeView::doubleClicked, this, &Widget::slot_insertToLineFromTree);
    connect(ui->view_list, &QListView::doubleClicked, this, &Widget::slot_insertToLineFromList);
    connect(ui->searching, &QLineEdit::textChanged, this, &Widget::slot_seeText);
    connect(shortcut_for_table, &QShortcut::activated, this, &Widget::slot_realiseTable);
    connect(model_tree, &DataModelTree::signal_dragAndDrop, this, &Widget::slot_listChangeAfterDrop);
    connect(model_tree, &DataModelTree::signal_dragAndDrop, this, &Widget::slot_treeChangeAfterDrop);
    connect(model_tree, &DataModelTree::signal_addItem, this, &Widget::slot_treeChangeAfterAdd);
}

void Widget::slot_insertValueFromLineEdit()
{
    if (ui->view_tree->currentIndex() == ui->view_tree->rootIndex())
    {
        City *new_item = new City(model_tree->rootItem);

        Q_UNUSED(new_item)
    }
    else
    {

         switch (model_tree->seeParent(nullptr, ui->view_tree->currentIndex()))
         {
              case type_Street:
                insertObjectInTree<Street, City>();
                break;

              case type_Home:
                insertObjectInTree<Home, Street>();
                break;

              case type_People:
                insertObjectInTree<People, Home>();
                break;
          }

          if (ui->view_tree->currentIndex() == current_index_tree)
          {
              model_list->addValue(ui->view_tree->currentIndex());
          }

    }

   model_tree->layoutChanged();
   model_tree->signal_addItem();
}

void Widget::slot_insertToLineFromTree()
{
    current_index_tree = ui->view_tree->currentIndex();
    model_list->addValue(current_index_tree);
    current_index_list = QModelIndex();
    ui->searching->clear();
}

void Widget::slot_insertToLineFromList()
{
    current_index_list = model_proxy_sort->mapToSource(ui->view_list->currentIndex());
    model_list->addValue(current_index_list);
    current_index_tree = QModelIndex();
    ui->searching->clear();
}

void Widget::slot_clickSort()
{
    if (ui->btn_sort->isChecked())
    {
        model_proxy_sort->sort(0, Qt::AscendingOrder);
    }
    else
    {
        model_proxy_sort->sort(0, Qt::DescendingOrder);
    }
}

void Widget::slot_seeText()
{
    model_proxy_sort->setFilterRegExp(QRegExp(ui->searching->text(), Qt::CaseInsensitive, QRegExp::FixedString));
    model_proxy_sort->layoutChanged();
}

void Widget::slot_realiseTable()
{
    model_table->row_Count = model_proxy_sort->rowCount();

    QObjectList objects_from_list;

    for (int j = 0; j < model_proxy_sort->rowCount(); j++)
    {

        for (int i = 0; i < model_list->data_list.length(); i++)
        {

           if (model_proxy_sort->index(0, 0).internalPointer() != nullptr)
           {

                 if (model_proxy_sort->data(model_proxy_sort->index(j, 0)).toString()
                          == model_list->data(model_list->index(i, 0), Qt::DisplayRole).toString())
                 {
                     objects_from_list.append(static_cast<QObject*>(model_list->index(i, 0).internalPointer()));
                 }

           }

        }

    }
    qDebug()<<objects_from_list;
    model_table->takeObjects(objects_from_list);
    model_table->setAddress(model_tree->rootItem);
    model_table->layoutChanged();
}

void Widget::slot_listChangeAfterDrop()
{
    if (current_index_list.internalPointer() != nullptr
            || current_index_tree.internalPointer() != nullptr)
    {

        if (current_index_list.internalPointer() != nullptr)
        {
            model_list->addValue(current_index_list);
        }
        else
        {
            model_list->addValue(current_index_tree);
        }

      model_proxy_sort->layoutChanged();
      ui->searching->clear();
    }

}

void Widget::slot_treeChangeAfterDrop()
{
    QObject *object = static_cast<QObject*>(ui->view_tree->currentIndex().internalPointer());
    QString name_object = object->objectName();
    General *general_object = static_cast<General*>(ui->view_tree->currentIndex().internalPointer());
    QString name_object_by_general = general_object->name_object;

    QObjectList childrens = object->parent()->children();

    int counter_all_common = 0;
    for (int i = 0; i < childrens.size(); i++)
    {

        if (childrens[i]->objectName() == name_object)
        {
           counter_all_common++;
           for_repeat_objects = childrens[i];
        }

    }

    if (counter_all_common > 1)
    {
        dialog_window = new Dialog(this, name_object, Drop);
        dialog_window->show();
    }

}

void Widget::slot_treeChangeAfterAdd()
{
    QObject *object = static_cast<QObject*>(ui->view_tree->currentIndex().internalPointer());
    General *general_object = static_cast<General*>(ui->view_tree->currentIndex().internalPointer());

    Q_UNUSED(general_object);

    if (object != nullptr)
    {
        QObjectList  childrens = object->children();
        for_repeat_objects = new QObject();

        int counter_all_common = 1;

        for (int i = 0; i < childrens.size(); i++)
        {
            QString current_name = childrens[i]->objectName();

            for (int j = i + 1; j < childrens.size(); j++)
            {

               if (current_name == childrens[j]->objectName())
               {
                   counter_all_common++;
                   for_repeat_objects = childrens[j];
               }

            }

         }

         if (counter_all_common > 1)
         {
             dialog_window = new Dialog(this, for_repeat_objects->objectName(), Add);
             dialog_window->show();
         }

       }

}

void Widget::slot_changeNameAfterAdd()
{
    QObject *object = for_repeat_objects;
    QModelIndex index = ui->view_tree->currentIndex();
    General *general_object = static_cast<General*>(object);

    QString sending_name = dialog_window->getTextFromLineEdit();

    if (sending_name != "")
    {
        object->setObjectName(sending_name);
        general_object->name_object = sending_name;

        model_list->layoutChanged();
        model_proxy_sort->layoutChanged();

        delete dialog_window;

        model_tree->signal_addItem();
    }
    else
    {
        delete dialog_window;

        model_tree->signal_addItem();
    }

    ui->view_tree->setCurrentIndex(index);

}

void Widget::slot_changeNameAfterDrop()
{
    QObject *object = static_cast<QObject*>(ui->view_tree->currentIndex().internalPointer());
    QModelIndex index = ui->view_tree->currentIndex();
    General *general_object = static_cast<General*>(ui->view_tree->currentIndex().internalPointer());

    QString sending_name = dialog_window->getTextFromLineEdit();

    if (sending_name != "")
    {
        object->setObjectName(sending_name);
        general_object->name_object = sending_name;

        model_list->layoutChanged();

        delete dialog_window;

        model_tree->signal_dragAndDrop();
    }
    else
    {
        delete dialog_window;

        model_tree->signal_dragAndDrop();
    }

    ui->view_tree->setCurrentIndex(index);

}


template <typename CHILD, typename PARENT>
void Widget::insertObjectInTree()
{
    CHILD *new_item = new CHILD(model_tree->objByIndex<PARENT>(ui->view_tree->currentIndex()));

    Q_UNUSED(new_item);
}

Widget::~Widget()
{

}


//Interface for Dialog to changing name object

Dialog::Dialog(QWidget *parent, QString name_object, int type_changing)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    ui->please_change->setText("Change name object, names cannot be repeated! ("
                                                          + name_object + ")");
    if (type_changing == Drop)
    {
        connect(ui->btn_change_text, SIGNAL(clicked()), parent, SLOT(slot_changeNameAfterDrop()));
    }
    else if (type_changing == Add)
    {
        connect(ui->btn_change_text, SIGNAL(clicked()), parent, SLOT(slot_changeNameAfterAdd()));
    }

}

Dialog::~Dialog()
{
    delete ui;
}

QString Dialog::getTextFromLineEdit()
{
    return ui->name_to_change->text();
}


