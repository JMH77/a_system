#include "databasemanager.h"
#include "../config/configmanager.h"

databasemanager::databasemanager(configmanager *config)
    :m_configManager(config),
    m_lastError("")
{
    
}

databasemanager::~databasemanager() {
    if(m_db.isOpen()) {
        m_db.close();
    }
}

//建立数据库连接
bool databasemanager::connectDatabase(){
    // 步骤1：检查配置管理器
    if (!m_configManager) {
        m_lastError = "配置管理器未设置";
        qDebug() << m_lastError;
        return false;
    }

    if (!m_configManager->isInitialized()) {
        m_lastError = "配置管理器未初始化";
        qDebug() << m_lastError;
        return false;
    }

    // 步骤2：检查是否已经连接
    if (m_db.isOpen()) {
        qDebug() << "数据库已连接";
        return true;
    }

    // 步骤3：从配置管理器读取数据库配置
    QString dbType = m_configManager->getDbType();
    QMap<QString, QString> dbConfig = m_configManager->getDbConfig();

    //步骤4：根据数据库类型创建连接
    QString driverName;
    if(dbType.toUpper() == "SQLITE" || dbType.toUpper() == "QSQLITE"){
        driverName = "QSQLITE";
    } else if(dbType.toUpper() == "DM"){
        driverName = "QODBC";
    } else{
        m_lastError = QString("不支持的数据库类型: %1").arg(dbType);
        qDebug() << m_lastError;
        return false;
    }

    // 步骤5：创建数据库连接
    m_db = QSqlDatabase::addDatabase(driverName);

    // 步骤6：设置连接参数
    if (driverName == "QSQLITE") {
        // SQLite 只需要数据库文件路径
        QString dbName = dbConfig.value("DatabaseName", "learn1.db");
        m_db.setDatabaseName(dbName);
    } else {
        // 其他数据库需要更多参数
        m_db.setHostName(dbConfig.value("Host", "localhost"));
        m_db.setPort(dbConfig.value("Port", "5236").toInt());
        m_db.setDatabaseName(dbConfig.value("DatabaseName", ""));
        m_db.setUserName(dbConfig.value("UID", ""));
        m_db.setPassword(dbConfig.value("Password", ""));
    }

    // 步骤7：打开连接
    if (!m_db.open()) {
        m_lastError = QString("数据库连接失败: %1").arg(m_db.lastError().text());
        qDebug() << m_lastError;
        return false;
    }


    // 步骤8：连接成功
    qDebug() << "数据库连接成功";
    return true;
}



//判断数据库是否连接
bool databasemanager::isConnected() const{
    return m_db.isOpen();
}


//返回错误信息
QString databasemanager::getLastError() const{
    return m_lastError;
}


//断开连接
bool databasemanager::disconnected(){
    if (m_db.isOpen()) {
        m_db.close();
        qDebug() << "数据库连接已断开";
        return true;
    } else {
        m_lastError = "数据库未连接";
        qDebug() << m_lastError;
        return false;
    }
}





















