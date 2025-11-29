#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QStackedWidget>
#include "database/databasemanager.h"
#include "config/configmanager.h"
#include "auth/authmanager.h"
#include "widgets/loginwidget.h"
#include "widgets/registerwidget.h"
#include "widgets/maincontentwidget.h"
#include "widgets/permissionmanagementwidget.h"
#include "Models/workordermanagementwidget.h"
#include "Models/mytaskswidget.h"
#include "Models/acceptancewidget.h"
#include "Models/sparepartsconsumptionwidget.h"
#include "Models/logreportwidget.h"




class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // 初始化UI->创建页面，并添加到堆叠窗口
    void initUI();

    //建立槽函数连接
    void connections();



private:
    //管理器
    configmanager *m_configManager;  // 配置管理器
    databasemanager *dbManger;       // 数据库管理器
    AuthManager *m_authManager;      // 认证管理器

    // 堆叠窗口（页面容器）
    QStackedWidget *m_stackedWidget;  

    // 具体页面
    LoginWidget *m_loginWidget;
    RegisterWidget *m_registerWidget;
    MainContentWidget *m_mainContentWidget;
    
    // 当前登录用户名
    QString m_currentUsername;
};
#endif // MAINWINDOW_H

