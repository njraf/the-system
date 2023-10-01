#include "pagenavigator.h"

PageNavigator* PageNavigator::instance = nullptr;

PageNavigator::PageNavigator(QObject *parent)
    : QObject{parent}
{

}

void PageNavigator::populateRoutes(QMap<PageName, std::function<QSharedPointer<Page>(void)>> routes_) {
    routes = routes_;
}

PageNavigator* PageNavigator::getInstance() {
    if (nullptr == PageNavigator::instance) {
        PageNavigator::instance = new PageNavigator();
    }
    return PageNavigator::instance;
}

void PageNavigator::navigate(PageName page) {

    currentPage = routes[page]();
    emit pageChanged(currentPage);
}
