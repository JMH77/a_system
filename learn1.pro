QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    auth/authmanager.cpp \
    auth/userinfo.cpp \
    config/configmanager.cpp\
    database/databasemanager.cpp \
    widgets/loginwidget.cpp \
    main.cpp \
    widgets/maincontentwidget.cpp \
    mainwindow.cpp \
    widgets/registerwidget.cpp \
    widgets/permissionmanagementwidget.cpp


HEADERS += \
    auth/authmanager.h \
    auth/userinfo.h \
    config/configmanager.h \
    database/databasemanager.h \
    widgets/loginwidget.h \
    widgets/maincontentwidget.h \
    mainwindow.h \
    widgets/registerwidget.h \
    widgets/permissionmanagementwidget.h



# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    resources/loginWidget.png
