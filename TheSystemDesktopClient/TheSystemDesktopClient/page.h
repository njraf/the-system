#ifndef PAGE_H
#define PAGE_H

#include <QWidget>

enum PageName {
    SIGN_IN,
    SIGN_UP
};

class Page : public QWidget
{
    Q_OBJECT
public:
    explicit Page(QWidget *parent = nullptr);

signals:

};

#endif // PAGE_H
