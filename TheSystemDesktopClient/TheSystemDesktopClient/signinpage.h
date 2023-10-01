#ifndef SIGNINPAGE_H
#define SIGNINPAGE_H

#include <QWidget>

#include "page.h"

namespace Ui {
class SignInPage;
}

class SignInPage : public Page
{
    Q_OBJECT

public:
    explicit SignInPage(QWidget *parent = nullptr);
    ~SignInPage();

private:
    Ui::SignInPage *ui;
};

#endif // SIGNINPAGE_H
