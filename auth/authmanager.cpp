#include "authmanager.h"
#include "../database/databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QDebug>
#include <QCryptographicHash>
#include <QList>

AuthManager::AuthManager(databasemanager *dbManager)
    : m_dbManager(dbManager)
    , m_lastError("")
{
}

// 检查用户名是否存在
bool AuthManager::userExists(const QString &username)
{
    if (!m_dbManager || !m_dbManager->isConnected()) {
        m_lastError = "数据库未连接";
        return false;
    }
    
    // 从 databasemanager 获取数据库连接（而不是创建新连接）
    QSqlDatabase db = m_dbManager->getDatabase();
    QSqlQuery query(db);
    
    query.prepare("SELECT COUNT(*) FROM NowUsers WHERE username = ?");
    query.addBindValue(username);
    
    if (!query.exec()) {
        m_lastError = QString("查询用户失败: %1").arg(query.lastError().text());
        query.finish();
        return false;
    }
    
    bool result = false;
    if (query.next()) {
        int count = query.value(0).toInt();
        result = count > 0;
    }
    
    // 确保查询完成
    query.finish();
    return result;
}

// 用户注册
bool AuthManager::registerUser(const userinfo &user)
{
    if (!m_dbManager || !m_dbManager->isConnected()) {
        m_lastError = "数据库未连接";
        return false;
    }
    
    userinfodata data = user.getUserData();
    
    // 检查用户名是否为空
    if (data.username.isEmpty()) {
        m_lastError = "用户名不能为空";
        return false;
    }
    
    // 加密密码
    QString passwordHash = hashPassword(data.password);
    
    // 插入数据库（从 databasemanager 获取连接）
    // 参考成功的代码，确保使用新的查询对象并提交事务
    QSqlDatabase db = m_dbManager->getDatabase();
    
    // 确保数据库连接有效
    if (!db.isOpen()) {
        m_lastError = "数据库连接未打开";
        qDebug() << m_lastError;
        return false;
    }
    
    // 先检查用户名是否存在（使用独立的查询对象，参考成功代码的方式）
    QSqlQuery checkQuery(db);
    checkQuery.prepare("SELECT COUNT(*) FROM NowUsers WHERE username = ?");
    checkQuery.addBindValue(data.username);
    if (checkQuery.exec() && checkQuery.next()) {
        if (checkQuery.value(0).toInt() > 0) {
            m_lastError = "用户名已存在";
            checkQuery.finish();
            return false;
        }
    }
    checkQuery.finish();
    
    // 创建全新的查询对象用于 INSERT（参考成功代码的方式）
    QSqlQuery query(db);
    
    // 准备 INSERT 语句（参考成功代码的格式）
    query.prepare("INSERT INTO NowUsers (username, password, email, name) VALUES (?, ?, ?, ?)");
    query.addBindValue(data.username);
    query.addBindValue(passwordHash);
    query.addBindValue(data.email);
    query.addBindValue(data.name);
    
    // 执行插入
    if (!query.exec()) {
        m_lastError = QString("注册失败: %1").arg(query.lastError().text());
        qDebug() << m_lastError;
        query.finish();
        return false;
    }
    
    // 确保查询完成
    query.finish();
    
    // 参考成功代码：达梦数据库需要显式提交事务
    if (!db.commit()) {
        m_lastError = QString("提交事务失败: %1").arg(db.lastError().text());
        qDebug() << m_lastError;
        return false;
    }
    
    qDebug() << "用户注册成功:" << data.username;
    return true;
}

// 用户登录验证
bool AuthManager::login(const QString &username, const QString &password)
{
    if (!m_dbManager || !m_dbManager->isConnected()) {
        m_lastError = "数据库未连接";
        return false;
    }
    
    if (username.isEmpty() || password.isEmpty()) {
        m_lastError = "用户名或密码不能为空";
        return false;
    }
    
    // 查询用户信息（从 databasemanager 获取连接）
    QSqlDatabase db = m_dbManager->getDatabase();
    QSqlQuery query(db);
    
    query.prepare("SELECT password FROM NowUsers WHERE username = ?");
    query.addBindValue(username);
    
    if (!query.exec()) {
        m_lastError = QString("查询用户失败: %1").arg(query.lastError().text());
        query.finish();
        return false;
    }
    
    if (!query.next()) {
        m_lastError = "用户名不存在";
        query.finish();
        return false;
    }
    
    // 获取存储的密码哈希
    QString storedHash = query.value(0).toString();
    query.finish();
    
    // 验证密码
    if (!verifyPassword(password, storedHash)) {
        m_lastError = "密码错误";
        return false;
    }
    
    qDebug() << "用户登录成功:" << username;
    return true;
}

// 获取用户的功能权限列表
QList<int> AuthManager::getUserFunctionPermissions(const QString &username) const
{
    QList<int> permissions;
    
    if (!m_dbManager || !m_dbManager->isConnected()) {
        return permissions;
    }
    
    QSqlDatabase db = m_dbManager->getDatabase();
    
    // 先检查用户是否是管理员（role_type=1）
    QSqlQuery roleQuery(db);
    roleQuery.prepare("SELECT role_type FROM NowUsers WHERE username = ?");
    roleQuery.addBindValue(username);
    bool isAdmin = false;
    if (roleQuery.exec() && roleQuery.next()) {
        int roleType = roleQuery.value(0).toInt();
        isAdmin = (roleType == 1);
    }
    roleQuery.finish();
    
    // 如果是管理员，返回所有权限
    if (isAdmin) {
        return QList<int>() << 1 << 2 << 3 << 4 << 5;
    }
    
    // 查询普通用户的功能权限
    // 先获取userid
    QSqlQuery userQuery(db);
    userQuery.prepare("SELECT userid FROM NowUsers WHERE username = ?");
    userQuery.addBindValue(username);
    int userId = -1;
    if (userQuery.exec() && userQuery.next()) {
        userId = userQuery.value(0).toInt();
    }
    userQuery.finish();
    
    if (userId <= 0) {
        return permissions;
    }
    
    // 查询该用户启用的功能权限
    QSqlQuery permQuery(db);
    permQuery.prepare("SELECT function_id FROM NowUsersPermissions WHERE userid = ? AND enabled = 1");
    permQuery.addBindValue(userId);
    
    if (permQuery.exec()) {
        while (permQuery.next()) {
            permissions.append(permQuery.value(0).toInt());
        }
    }
    permQuery.finish();
    
    return permissions;
}

// 检查用户是否有指定功能的权限
bool AuthManager::hasFunctionPermission(const QString &username, int functionId) const
{
    QList<int> permissions = getUserFunctionPermissions(username);
    return permissions.contains(functionId);
}

// 获取所有用户列表
QList<userinfo> AuthManager::getAllUsers() const
{
    QList<userinfo> users;
    
    if (!m_dbManager || !m_dbManager->isConnected()) {
        return users;
    }
    
    QSqlDatabase db = m_dbManager->getDatabase();
    QSqlQuery query(db);
    
    query.prepare("SELECT username, email, name FROM NowUsers ORDER BY username");
    
    if (query.exec()) {
        while (query.next()) {
            userinfodata data;
            data.username = query.value(0).toString();
            data.email = query.value(1).toString();
            data.name = query.value(2).toString();
            
            userinfo user;
            user.setUserData(data);
            users.append(user);
        }
    }
    query.finish();
    
    return users;
}

// 获取数据库管理器
databasemanager* AuthManager::getDatabaseManager() const
{
    return m_dbManager;
}

// 获取错误信息
QString AuthManager::getLastError() const
{
    return m_lastError;
}

// 密码加密（使用 MD5）
QString AuthManager::hashPassword(const QString &password)
{
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Md5);
    return hash.toHex();
}

// 密码验证
bool AuthManager::verifyPassword(const QString &password, const QString &hash)
{
    QString computedHash = hashPassword(password);
    return computedHash == hash;
}
