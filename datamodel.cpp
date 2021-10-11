#include "datamodel.h"
#include <QColor>
DataModel::DataModel(QObject *parent) : QAbstractTableModel(parent)
{

}

void DataModel::SetHeader(QStringList header)
{
    m_header = header;
}

void DataModel::UpdateData(QList<QStringList> data)
{
    beginRemoveRows({}, 0, m_data.size()-1);
    m_data.clear();
    endRemoveRows();

    beginInsertRows({}, 0, data.size()-1);
    m_data.append(data);
    endInsertRows();
}


int DataModel::rowCount(const QModelIndex &parent) const
{
    return m_data.size();
}

int DataModel::columnCount(const QModelIndex &parent) const
{
    return m_header.size();
}

QVariant DataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    if (m_data.size() == 0) return QVariant();

    if (role == Qt::TextAlignmentRole) {
        if (index.column() == 0)
            return int(Qt::AlignLeft | Qt::AlignVCenter);
        else
            return int(Qt::AlignRight | Qt::AlignVCenter);
    } else  if(role == Qt::DisplayRole) {
        if (index.row() >= m_data.size() || index.column() >= m_data.at(0).size()) {
            return QVariant();
        } else {
            if (index.column() == 1) {
                return m_data.at(index.row()).at(index.column())+"%";
            } else {
                return m_data.at(index.row()).at(index.column());
            }
        }
    } else if (role == Qt::TextColorRole && index.column() == 1) {
        if (m_data.at(index.row()).at(index.column()).contains("-"))
            return QColor(Qt::darkGreen);
        else
            return QColor(Qt::red);
    } else if (role == Qt::BackgroundRole && (index.row() %2 ==0)) {
        return QColor(244,245,245);
    } else {
        return QVariant();
    }
}

QVariant DataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            if (m_header.size() > section)
            {
                return m_header[section];
            }
            return QVariant();
        }
    }
    return QVariant();
}
