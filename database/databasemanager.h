#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QString>

class configmanager;

class databasemanager
{
public:
    databasemanager(configmanager *config = nullptr);
    ~databasemanager();

    //建立数据库连接
    bool connectDatabase();

    //检查数据库是否已连接
    bool isConnected() const;

    //返回错误信息
    QString getLastError() const;

    //断开连接
    bool disconnected();


private:
    QSqlDatabase m_db;
    configmanager *m_configManager;
    QString m_lastError;
};

#endif // DATABASEMANAGER_H
