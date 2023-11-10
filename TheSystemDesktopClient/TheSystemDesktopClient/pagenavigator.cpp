#include "pagenavigator.h"
#include "pagefactory.h"

#include <algorithm>

#include <QDebug>

PageNavigator* PageNavigator::instance = nullptr;

PageNavigator::PageNavigator(QObject *parent)
    : QObject{parent}
{

}

PageNavigator* PageNavigator::getInstance() {
    if (nullptr == PageNavigator::instance) {
        PageNavigator::instance = new PageNavigator();
    }
    return PageNavigator::instance;
}

void PageNavigator::navigate(PageName page_) const {
    currentPage = PageFactory::createPage(page_);
    backStack.push(currentPage);
    emit changedPage(currentPage);
}

void PageNavigator::navigateBackTo(PageName page_) const {
    // check if the requested page is in the backstack
    const bool PAGE_FOUND = std::any_of(backStack.begin(), backStack.end(), [page_](Page* backStackPage_) { return (backStackPage_->getPageName() == page_); });
    if (!PAGE_FOUND) {
        return;
    }

    // pop back to the requested page
    int pagesPopped = 0;
    QList<Page*> pagesToDelete;
    while (backStack.size() > 1) {
        auto top = backStack.top();
        pagesToDelete.push_back(top);
        backStack.pop_back();
        pagesPopped++;
        if (backStack.top()->getPageName() == page_) {
            break;
        }
    }

    emit poppedPages(pagesPopped);
}
