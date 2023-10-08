#include "pagenavigator.h"

#include <QDebug>

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

void PageNavigator::navigate(PageName page) const {
    if (!routes.contains(page)) {
        qDebug() << "This page does not have a route" << page;
        return;
    }
    currentPage = routes[page]();
    backStack.push(currentPage);
    emit pageChanged(currentPage);
}
