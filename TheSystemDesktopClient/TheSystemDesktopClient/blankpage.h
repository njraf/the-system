#ifndef BLANKPAGE_H
#define BLANKPAGE_H

#include <QWidget>

#include "page.h"

namespace Ui {
class BlankPage;
}

class BlankPage : public Page
{
    Q_OBJECT

public:
    explicit BlankPage(QWidget *parent = nullptr);
    ~BlankPage();

private:
    Ui::BlankPage *ui;
};

#endif // BLANKPAGE_H
