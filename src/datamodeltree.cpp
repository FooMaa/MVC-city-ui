#include "headers/datamodeltree.h"
#include <QDebug>

DataModelTree::DataModelTree(QObject *parent)
    : QAbstractItemModel(parent)
{
//
}

int DataModelTree::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 1;
}

int DataModelTree::rowCount(const QModelIndex &parent) const
{
    if (objByIndex<Singleton>(parent) == nullptr)
    {
          return rootItem.children().count();
    }

    return static_cast<QObject*>(parent.internalPointer())->children().size();
}

QModelIndex DataModelTree::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    if (objByIndex<Singleton>(parent) == nullptr)
    {
          return createIndex(row, column, rootItem.children().at(row));
    }

    return createIndex(row, column, static_cast<QObject*>(parent.internalPointer())->children()[row]);

}

QModelIndex DataModelTree::parent(const QModelIndex &child) const
{

    if (objByIndex<Singleton>(child)->parent() == &rootItem)
    {
        return QModelIndex();
    }

    int row = 0;

    if (objByIndex<Singleton>(child)->parent() != nullptr)
    {
        QObject *children = objByIndex<QObject>(child);
        QObject *father = children->parent();
        QObject *grandfather = father->parent();

        if (grandfather == nullptr)
        {
           grandfather = &rootItem;
        }

        row = grandfather->children().indexOf(father);
        return createIndex(row, 0, father);
    }

    return QModelIndex();
}



QVariant DataModelTree::data(const QModelIndex &index, int role) const
{

    if (!index.isValid())
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole)
    {
        //qDebug()<<static_cast<General*>(index.internalPointer())->name_object;
        return objByIndex<QObject>(index)->property(QString("objectName").toUtf8());
    }

    return QVariant();
}

QVariant DataModelTree::headerData(int section, Qt::Orientation orientation, int role) const{
    Q_UNUSED(section);

    if (orientation== Qt::Orientation::Horizontal && role==Qt::DisplayRole)
    {
        return QString("TreeView");
    }

    return QVariant();
}

static const char tree_mimetype[] = "application/x-treenode";

QStringList DataModelTree::mimeTypes() const
{
    return QStringList() << tree_mimetype;
}

QMimeData *DataModelTree::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimedata = new QMimeData;
    QByteArray  data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    QList <General*> objects;
    QList <QModelIndex> indexes_for_count;

    foreach(const QModelIndex &index, indexes)
    {
        General *object = objByIndex<General>(index);

        if (!objects.contains(object))
        {
            objects << object;
            indexes_for_count << index;
        }

    }

    stream << QCoreApplication::applicationPid();
    stream << objects.count();

    foreach (const General *object, objects)
    {
        stream << object->id_for_drop.toByteArray();
    }

    mimedata->setData(tree_mimetype, data);

    return mimedata;
}

bool DataModelTree::dropMimeData(const QMimeData *mimedata, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_ASSERT(action == Qt::MoveAction);
    Q_UNUSED(column);

    if (!mimedata->hasFormat(tree_mimetype))
    {
        return false;
    }

    QByteArray data = mimedata->data(tree_mimetype);
    QDataStream stream(&data, QIODevice::ReadOnly);
    qint64 senderPid;

    stream >> senderPid;

    if (senderPid != QCoreApplication::applicationPid())
    {
        return false;
    }

    QObject *parent_object = objByIndex<QObject>(parent);

    if (parent_object == nullptr)
    {
        parent_object = &rootItem;
    }

    Q_ASSERT(parent_object);

    int count;
    stream >> count;

    if(row == -1)
    {
        if(parent.isValid())
        {
            row=0;
        }
        else
        {
            row=rowCount(parent);
        }
    }

    for (int i =0 ; i < count ; i++)
    {
        QByteArray id;
        stream >> id;

        QUuid id_drop = QUuid(id);

        QList <QObject*> childrens = rootItem.children();
        General *main_object = seeObjectForDrop(childrens, id_drop);

        if (main_object == nullptr)
        {
            return false;
        }

        QObject *children = dynamic_cast<QObject*>(main_object);

        if (children->parent()->children().indexOf(children) < row && parent_object == children->parent())
        {
            row--;
        }

        if (seeParent(children) == seeParent(parent_object) + 1)
        {
            children->setParent(parent_object);
        }
        else
        {

          if (seeParent(children) == 4 && seeParent(parent_object) + 1 == 3)
          {
            children->setObjectName("Бомж");
            children->setParent(parent_object);
          }

          if (children->objectName() == "Бомж" && seeParent(parent_object) + 1 == 4)
          {
              children->setObjectName("People из бомжа");
              children->setParent(parent_object);
          }

        }

        row++;
    }

    emit signal_dragAndDrop();
    emit layoutChanged();

    return true;
}

Qt::DropActions DataModelTree::supportedDropActions() const
{
    return Qt::MoveAction;
}

Qt::DropActions DataModelTree::supportedDragActions() const
{
    return Qt::MoveAction;
}

Qt::ItemFlags DataModelTree::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::ItemIsDropEnabled;
    }

    return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

int DataModelTree::seeParent(QObject *child, QModelIndex index)
{
    if (child == nullptr)
    {
        child = objByIndex<QObject>(index);
    }

    int amount_parent = 0;

    while( child != &rootItem && child != nullptr)
    {
        child = child->parent();
        amount_parent++;
    }

    return amount_parent;
}

General *DataModelTree::seeObjectForDrop(QList<QObject *> childrens, QUuid id_drop)
{
    General *main;

    for (int i = 0; i < childrens.size(); i++)
    {
        General *object_to_check = dynamic_cast<General*>(childrens[i]);

        if (object_to_check->id_for_drop == id_drop)
        {
            main = object_to_check;
            return object_to_check;
        }

    }

    for (int i = 0; i < childrens.size(); i++)
    {
        main = seeObjectForDrop(childrens[i]->children(), id_drop);

        if (main != nullptr)
        {
            return main;
        }

    }

    return nullptr;
}






