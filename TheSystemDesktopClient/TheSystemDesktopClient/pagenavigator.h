#ifndef PAGENAVIGATOR_H
#define PAGENAVIGATOR_H

#include <QObject>
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
    mutable QStack<Page*> backStack;
    mutable Page* currentPage;

signals:
    void changedPage(Page* page) const;
    void poppedPages(int pagesPopped) const;

};

#endif // PAGENAVIGATOR_H
