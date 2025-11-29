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

protected:
    // 重写paintEvent事件，绘制背景图
    void paintEvent(class QPaintEvent *event) override;

private slots:
    void onPermissionManagementClicked();

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
    QPixmap m_bgPixmap;
};

#endif // MAINCONTENTWIDGET_H
