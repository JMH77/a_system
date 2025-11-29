#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H
#include <QWidget>
#include <QPixmap>

class AuthManager;

class LoginWidget :public QWidget
{
    Q_OBJECT
public:
    LoginWidget(QWidget *parent = nullptr);
    void setAuthManager(AuthManager *authManager);
    void clearInputFields();  // 清空所有输入框

	void setBackgroundImage();

signals:
    void loginSuccess(const QString &username);
    void loginFailed(const QString &errorMessage);
    void changeToRegister();

private:
    void setupUI();
    void applyStyles();
    //重写paintEvent事件，绘制背景图
    void paintEvent(class QPaintEvent *event) override;

    //验证输入格式是否正确
    bool validateInput();


private slots:
    //登录按钮点击槽函数
    void onLoginButtonClicked();
    //注册按钮点击槽函数
    void onRegisterButtonClicked();

private:
	class QLabel *m_usernameLabel;
	class QLabel *m_passwordLabel;
	class QLineEdit *m_usernameEdit;
	class QLineEdit *m_passwordEdit;
	class QPushButton *m_loginButton;
	class QPushButton *m_registerButton;
	class QWidget *m_centerPanel;
	QPixmap m_bgPixmap;
	AuthManager *m_authManager;  // 认证管理器
};

#endif // LOGINWIDGET_H
