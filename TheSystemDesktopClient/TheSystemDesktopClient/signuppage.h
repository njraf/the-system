#ifndef SIGNUPPAGE_H
#define SIGNUPPAGE_H

#include <QWidget>

#include "page.h"

namespace Ui {
class SignUpPage;
}

class SignUpPage : public Page
{
    Q_OBJECT

public:
    explicit SignUpPage(QWidget *parent = nullptr);
    ~SignUpPage();

private:
    Ui::SignUpPage *ui;
};

#endif // SIGNUPPAGE_H
