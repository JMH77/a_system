#ifndef PERMISSIONMANAGEMENTWIDGET_H
#define PERMISSIONMANAGEMENTWIDGET_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>

class AuthManager;

class PermissionManagementWidget : public QDialog
{
    Q_OBJECT

public:
    explicit PermissionManagementWidget(AuthManager *authManager, QWidget *parent = nullptr);

private slots:
    void onSaveClicked();
    void onCancelClicked();

private:
    void setupUI();
    void loadUsers();
    
    AuthManager *m_authManager;
    QTableWidget *m_userTable;
    QPushButton *m_saveButton;
    QPushButton *m_cancelButton;
    
    // 列索引
    static const int COL_USERNAME = 0;
    static const int COL_EMAIL = 1;
    static const int COL_FUNC1 = 2;
    static const int COL_FUNC2 = 3;
    static const int COL_FUNC3 = 4;
    static const int COL_FUNC4 = 5;
    static const int COL_FUNC5 = 6;
};

#endif // PERMISSIONMANAGEMENTWIDGET_H

