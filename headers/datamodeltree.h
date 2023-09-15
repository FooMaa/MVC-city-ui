#ifndef DATAMODELTREE_H
#define DATAMODELTREE_H

#include <QAbstractItemModel>
#include <QStringList>
#include <QObjectList>
#include <QMimeData>
#include <QDataStream>
#include <QCoreApplication>

#include "singleton.h"

class DataModelTree : public QAbstractItemModel
{
  Q_OBJECT
public:
    Singleton &rootItem = Singleton::get_instance();

    DataModelTree(QObject *parent);

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *mimedata, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    Qt::DropActions supportedDropActions() const;
    Qt::DropActions supportedDragActions() const;
    Qt::ItemFlags flags(const QModelIndex &index) const;


    template <typename OBJECT, typename PARENT> void addValue(OBJECT *item, const QModelIndex &parentIdx);
    template <typename OBJECT> OBJECT *objByIndex(const QModelIndex &index) const;
    int seeParent(QObject *child = nullptr, QModelIndex index = QModelIndex());
    General *seeObjectForDrop(QList<QObject*> childrens, QUuid id_drop);

signals:
    void signal_dragAndDrop();
    void signal_addItem();
};


template <typename OBJECT,typename PARENT>
void DataModelTree::addValue(OBJECT *item, const QModelIndex &parentIdx)
{
    beginInsertRows(parentIdx, rowCount(parentIdx), rowCount(parentIdx));

    if (objByIndex<PARENT>(parentIdx) == nullptr)
    {
        item->setParent(&rootItem);
    }
    else
    {
        item->setObjectName(item->names_obj);
    }

    endInsertRows();
    emit layoutChanged();
}

template <typename OBJECT>
OBJECT *DataModelTree::objByIndex(const QModelIndex &index) const
{
    if (!index.isValid())
    {
       return 0;
    }

   return static_cast<OBJECT*>(index.internalPointer());
}

#endif // DATAMODELTREE_H
