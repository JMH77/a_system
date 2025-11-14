#ifndef DATABASEMANGER_H
#define DATABASEMANGER_H
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QString>

class databasemanger
{
public:
    databasemanger();
    bool connectDatabase();
    bool disconnectDatabase();
    bool createTable();

private:
    QSqlDatabase m_db;
};

#endif // DATABASEMANGER_H
