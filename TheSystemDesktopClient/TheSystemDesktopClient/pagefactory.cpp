#include "pagefactory.h"
#include "signinpage.h"
#include "signuppage.h"
#include "homepage.h"

PageFactory::PageFactory()
{

}

QSharedPointer<Page> PageFactory::createPage(PageName page_) {
    QSharedPointer<Page> newPage;
    switch (page_) {
    case PageName::SIGN_IN:
        newPage = QSharedPointer<SignInPage>::create();
        break;
    case PageName::SIGN_UP:
        newPage = QSharedPointer<SignUpPage>::create();
        break;
    case PageName::HOME:
        newPage = QSharedPointer<HomePage>::create();
        break;
    default:
        break;
    }
    return newPage;
}
