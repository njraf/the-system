#ifndef PAGENAVIGATOR_H
#define PAGENAVIGATOR_H

#include <QObject>
#include <QSharedPointer>
#include <QStack>
#include <QMap>

#include "page.h"

class PageNavigator : public QObject
{
    Q_OBJECT
public:
    explicit PageNavigator(QObject *parent = nullptr);

    static PageNavigator* getInstance();
    void populateRoutes(QMap<PageName, std::function<QSharedPointer<Page>(void)>> routes_);
    void navigate(PageName page);

private:
    static PageNavigator *instance;
    QStack<Page> backStack;
    QSharedPointer<Page> currentPage;
    QMap<PageName, std::function<QSharedPointer<Page>(void)>> routes;

signals:
    void pageChanged(QSharedPointer<Page> page);

};

#endif // PAGENAVIGATOR_H
