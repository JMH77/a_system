#include "userinfo.h"

userinfo::userinfo() {}

void userinfo::setUserData(const userinfodata &data)
{
    userdata = data;
}

userinfodata userinfo::getUserData() const
{
    return userdata;
}
