#include "pagefactory.h"
#include "signinpage.h"
#include "signuppage.h"
#include "homepage.h"
#include "blankpage.h"

PageFactory::PageFactory()
{

}

QSharedPointer<Page> PageFactory::createPage(PageName page_) {
    switch (page_) {
    case PageName::SIGN_IN:
        return QSharedPointer<SignInPage>::create();
    case PageName::SIGN_UP:
        return QSharedPointer<SignUpPage>::create();
    case PageName::HOME:
        return QSharedPointer<HomePage>::create();
    default:
        return QSharedPointer<BlankPage>::create();
    }
    return QSharedPointer<BlankPage>::create();
}
