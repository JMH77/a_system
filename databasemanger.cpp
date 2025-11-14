#include "databasemanger.h"

databasemanger::databasemanger() {}

//建立数据库连接
bool databasemanger::connectDatabase()
{
    //检查是否已经有连接
     if(m_db.isOpen())
     {
        qDebug() << "数据库已连接";
        return true;
     }

     //创建连接
     m_db = QSqlDatabase::addDatabase("QSQLITE");
     m_db.setDatabaseName("learn1.db");
     if(!m_db.open())
     {
        qDebug() << "数据库连接失败" << m_db.lastError().text();
        return false;
     }
     
     //连接成功
     qDebug() << "数据库连接成功";
     return true;
}

//断开数据库连接
bool databasemanger::disconnectDatabase()
{
    if(m_db.isOpen())
    {
        m_db.close();
        qDebug() << "数据库成功断开";
        return true;
    }
    else
    {
        qDebug() << "数据库未连接";
        return false;
    }
}

//创建表
bool databasemanger::createTable()
{
    if(!m_db.isOpen())
    {
        qDebug() << "数据库未连接";
        return false;
    }

    QSqlQuery query;  //创建查询对象(QSqlQuery用于执行SQL语句)
    //创建用户表
    if(!query.exec("CREATE TABLE IF NOT EXISTS USERS( "
             "ID INTEGER PRIMARY KEY AUTOINCREMENT," 
             "USERNAME TEXT NOT NULL,"
             "PASSWORD TEXT NOT NULL,"
             "EMAIL TEXT NOT NULL UNIQUE)"
             ))
    {
        qDebug() << "创建用户表失败:" << query.lastError().text();
        return false;
    } else {
        qDebug() << "创建用户表成功！";
    }
    
    //创建球员表
    if(!query.exec("CREATE TABLE IF NOT EXISTS PLAYERS( "
             "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
             "USERNAME TEXT NOT NULL, "
             "PLAYER_NAME TEXT NOT NULL, "
             "POSITION TEXT, "
             "TEAM TEXT, "
             "AGE INTEGER, "
             "HEIGHT REAL, "
             "WEIGHT REAL, "
             "FOREIGN KEY (USERNAME) REFERENCES USERS(USERNAME)"
             ")"))
    {
        qDebug() << "创建球员表失败:" << query.lastError().text();
        return false;
    } else {
        qDebug() << "创建球员表成功！";
    }
    
    qDebug() << "创建表成功";
    return true;
}
