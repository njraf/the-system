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

void PageNavigator::navigate(PageName page_) const {
    if (!routes.contains(page_)) {
        qDebug() << "This page does not have a route" << page_;
        return;
    }
    currentPage = routes[page_]();
    backStack.push(currentPage);
    emit changedPage(currentPage);
}

void PageNavigator::navigateBackTo(PageName page_) const {
    int pagesPopped = 0;
    while (backStack.size() > 1) {
        auto top = backStack.top();
        backStack.pop_back();
        //NOTE: delete top here if we stop using QSharedPointer
        pagesPopped++;
        if (backStack.top()->getPageName() == page_) {
            break;
        }
    }

    emit poppedPages(pagesPopped);
}
