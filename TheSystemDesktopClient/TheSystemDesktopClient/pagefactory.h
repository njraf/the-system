#ifndef PAGEFACTORY_H
#define PAGEFACTORY_H

#include "page.h"

class PageFactory
{
public:
    PageFactory();

    static Page* createPage(PageName page_);
};

#endif // PAGEFACTORY_H
