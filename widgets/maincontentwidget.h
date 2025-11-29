#ifndef MAINCONTENTWIDGET_H
#define MAINCONTENTWIDGET_H
#include <QWidget>
#include <QPixmap>
#include <QList>

class QPushButton;
class AuthManager;

class MainContentWidget : public QWidget
{
    Q_OBJECT

public:
    MainContentWidget(QWidget *parent = nullptr);
    
    // 根据用户权限更新按钮状态
    void updateButtonsByPermissions(AuthManager *authManager, const QString &username);

signals:
    // 权限管理按钮点击信号
    void permissionManagementRequested();
    // 退出登录信号
    void logoutRequested();
    // 功能按钮点击信号
    void function1Requested();  // 工单管理
    void function2Requested();  // 我的任务
    void function3Requested();  // 验收任务
    void function4Requested();  // 备件消耗
    void function5Requested();  // 日志报告

protected:
    // 重写paintEvent事件，绘制背景图
    void paintEvent(class QPaintEvent *event) override;

private slots:
    void onPermissionManagementClicked();
    void onLogoutButtonClicked();
    void onFunction1Clicked();
    void onFunction2Clicked();
    void onFunction3Clicked();
    void onFunction4Clicked();
    void onFunction5Clicked();

private:
    void setupUI();
    void applyStyles();
    void setBackgroundImage();
    void updateButtonState(QPushButton *button, bool enabled);

private:
    AuthManager *m_authManager;
    QString m_currentUsername;
    
    QPushButton *m_functionButton1;
    QPushButton *m_functionButton2;
    QPushButton *m_functionButton3;
    QPushButton *m_functionButton4;
    QPushButton *m_functionButton5;
    QPushButton *m_permissionButton;  // 权限管理按钮
    QPushButton *m_logoutButton;  // 退出登录按钮
    QPixmap m_bgPixmap;
};

#endif // MAINCONTENTWIDGET_H
