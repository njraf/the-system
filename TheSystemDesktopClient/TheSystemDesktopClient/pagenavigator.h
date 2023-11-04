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
    void navigate(PageName page_) const;
    void navigateBackTo(PageName page_) const;

private:
    static PageNavigator *instance;
    mutable QStack<QSharedPointer<Page>> backStack;
    mutable QSharedPointer<Page> currentPage;

signals:
    void changedPage(QSharedPointer<Page> page) const;
    void poppedPages(int pagesPopped) const;

};

#endif // PAGENAVIGATOR_H
