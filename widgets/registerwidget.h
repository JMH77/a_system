#ifndef REGISTERWIDGET_H
#define REGISTERWIDGET_H
#include <QWidget>
#include <QPixmap>
#include "auth/userinfo.h"

class RegisterWidget : public QWidget
{
    Q_OBJECT
public:
    RegisterWidget(QWidget *parent = nullptr);
    void clearInputFields();  // 清空所有输入框

signals:
    void backToLogin();
    void registerRequest(const userinfodata &data);

private:
    void setupUI();
    void applyStyles();
    void setBackgroundImage();
    void paintEvent(class QPaintEvent *event) override;
    bool validateInput();

private slots:
    void onBackButtonClicked();
    void onRegisterButtonClicked();

private:
    class QLabel *m_usernameLabel;
    class QLabel *m_passwordLabel;
    class QLabel *m_nameLabel;
    class QLabel *m_emailLabel;
    class QLineEdit *m_usernameEdit;
    class QLineEdit *m_passwordEdit;
    class QLineEdit *m_nameEdit;
    class QLineEdit *m_emailEdit;
    class QPushButton *m_backButton;
    class QPushButton *m_registerButton;
    class QWidget *m_centerPanel;
    QPixmap m_bgPixmap;
};

#endif // REGISTERWIDGET_H
