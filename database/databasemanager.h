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

    //初始化用户表
    bool initUserTable();
    
    //初始化用户权限表
    bool initUserPermissionsTable();
    
    //初始化工单主表
    bool initWorkOrderTable();
    
    //初始化工单备件消耗表
    bool initWorkOrderSpareTable();
    
    //初始化工单检测报表表
    bool initWorkOrderReportTable();
    
    //初始化工单操作日志表
    bool initWorkOrderLogTable();
    
    //扩展用户表，添加工单角色字段
    bool addWorkOrderRoleToUserTable();
    
    //获取数据库连接（供其他模块使用）
    QSqlDatabase getDatabase() const;
    
    //获取数据库类型（只支持达梦数据库）
    QString getDbType() const;

private:
    QSqlDatabase m_db;
    configmanager *m_configManager;
    QString m_lastError;
};

#endif // DATABASEMANAGER_H
