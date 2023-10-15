#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>

#include "page.h"

namespace Ui {
class HomePage;
}

class HomePage : public Page
{
    Q_OBJECT

public:
    explicit HomePage(QWidget *parent = nullptr);
    ~HomePage();

private:
    Ui::HomePage *ui;
};

#endif // HOMEPAGE_H
