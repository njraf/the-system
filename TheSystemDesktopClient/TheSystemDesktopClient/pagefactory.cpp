#include "pagefactory.h"
#include "signinpage.h"
#include "signuppage.h"
#include "homepage.h"
#include "blankpage.h"

PageFactory::PageFactory()
{

}

Page* PageFactory::createPage(PageName page_) {
    switch (page_) {
    case PageName::SIGN_IN:
        return new SignInPage();
    case PageName::SIGN_UP:
        return new SignUpPage();
    case PageName::HOME:
        return new HomePage();
    default:
        return new BlankPage();
    }
    return new BlankPage();
}
