#ifndef AUTHMANAGER_H
#define AUTHMANAGER_H

#include <QString>
#include "userinfo.h"

class databasemanager;

class AuthManager
{
public:
    // 构造函数
    AuthManager(databasemanager *dbManager);
    
    // 检查用户名是否存在
    bool userExists(const QString &username);
    
    // 用户注册
    bool registerUser(const userinfo &user);
    
    // 用户登录验证
    bool login(const QString &username, const QString &password);
    
    // 获取错误信息
    QString getLastError() const;

private:
    // 密码加密
    QString hashPassword(const QString &password);
    
    // 密码验证
    bool verifyPassword(const QString &password, const QString &hash);
    
    // 成员变量
    databasemanager *m_dbManager;
    QString m_lastError;
};

#endif // AUTHMANAGER_H
