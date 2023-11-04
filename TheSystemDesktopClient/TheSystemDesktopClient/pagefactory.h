#ifndef PAGEFACTORY_H
#define PAGEFACTORY_H

#include <QSharedPointer>
#include "page.h"

class PageFactory
{
public:
    PageFactory();

    static QSharedPointer<Page> createPage(PageName page_);
};

#endif // PAGEFACTORY_H
