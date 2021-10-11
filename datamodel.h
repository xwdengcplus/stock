#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QAbstractTableModel>

class DataModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit DataModel(QObject *parent = nullptr);
    void SetHeader(QStringList header);
    void UpdateData(QList<QStringList> data);
protected:

    virtual int
    rowCount(const QModelIndex &parent = QModelIndex()) const;

    virtual int
    columnCount(const QModelIndex &parent = QModelIndex()) const;

    virtual QVariant
    data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    virtual QVariant
    headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;


signals:

private:
    QStringList m_header;
    QList<QStringList> m_data;

};

#endif // DATAMODEL_H
