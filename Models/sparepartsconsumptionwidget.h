#ifndef SPAREPARTSCONSUMPTIONWIDGET_H
#define SPAREPARTSCONSUMPTIONWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QTableWidget>

class AuthManager;
class SparePartsManager;
class databasemanager;

class SparePartsConsumptionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SparePartsConsumptionWidget(AuthManager *authManager, const QString &currentUsername, QWidget *parent = nullptr);
    ~SparePartsConsumptionWidget();

private slots:
    void onNewConsumptionClicked();
    void onSearchTextChanged(const QString &text);

private:
    void setupUI();
    void applyStyles();
    void loadConsumptions();
    void displayConsumptions(const QList<QStringList> &consumptions);

private:
    AuthManager *m_authManager;
    QString m_currentUsername;
    databasemanager *m_dbManager;
    SparePartsManager *m_sparePartsManager;
    
    QPushButton *m_newButton;
    QLineEdit *m_searchEdit;
    QTableWidget *m_consumptionTable;
    
    QList<QStringList> m_currentConsumptions;  // 存储当前显示的消耗记录
};

#endif // SPAREPARTSCONSUMPTIONWIDGET_H

