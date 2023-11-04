#include "page.h"

Page::Page(PageName name_, QWidget *parent)
    : QWidget{parent}
    , name(name_)
{

}

PageName Page::getPageName() const {
    return name;
}
