#ifndef PAGE_H
#define PAGE_H

#include <QWidget>

enum PageName {
    SIGN_IN,
    SIGN_UP,
    HOME
};

class Page : public QWidget
{
    Q_OBJECT
public:
    explicit Page(PageName name_, QWidget *parent = nullptr);

    PageName getPageName() const;

private:
    PageName name;
};

#endif // PAGE_H
