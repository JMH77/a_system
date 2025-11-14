#include "mainwindow.h"
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QFile>
#include <QDebug>




MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_configManager(new configmanager())  // 创建配置管理器
    , dbManger(new databasemanager(m_configManager))  // 传入配置管理器
{
    this->setWindowTitle("登录");
    initUI();
}

MainWindow::~MainWindow()
{
    delete dbManger;        // 删除数据库管理器
    delete m_configManager; // 删除配置管理器
    delete m_stackedWidget;
}

void MainWindow::initUI()
{
    // 1. 创建堆叠窗口
    m_stackedWidget = new QStackedWidget(this);  // ← 创建，this作为父对象
    
    // 2. 创建三个页面
    m_loginWidget = new LoginWidget(m_stackedWidget);
    m_registerWidget = new RegisterWidget(m_stackedWidget);
    m_mainContentWidget = new MainContentWidget(m_stackedWidget);
    
    // 3. 将页面添加到堆叠窗口
    m_stackedWidget->addWidget(m_loginWidget);        // 索引0
    m_stackedWidget->addWidget(m_registerWidget);     // 索引1
    m_stackedWidget->addWidget(m_mainContentWidget);   // 索引2
    
    // 4. 设置堆叠窗口为中央部件（重要！）
    setCentralWidget(m_stackedWidget);  // ← 这样QStackedWidget才会显示
    
    // 5. 默认显示登录页面
    m_stackedWidget->setCurrentIndex(0);
}






