#include "pagenavigator.h"
#include "pagefactory.h"

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
    bool pageFound = false;
    for (auto page : backStack) {
        if (page->getPageName() == page_) {
            pageFound = true;
            break;
        }
    }

    if (!pageFound) {
        return;
    }

    // pop back to the requested page
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
