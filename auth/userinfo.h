#ifndef USERINFO_H
#define USERINFO_H
#include <QString>

struct userinfodata
{
    QString username;
    QString password;
    QString email;
    QString name;
};

class userinfo
{
public:
    userinfo();
    
    void setUserData(const userinfodata &data);

    userinfodata getUserData() const;

private:
    userinfodata userdata;
};

#endif // USERINFO_H
