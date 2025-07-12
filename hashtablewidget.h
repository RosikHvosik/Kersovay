#ifndef HASHTABLEWIDGET_H
#define HASHTABLEWIDGET_H

#include <QWidget>
#include <QTableWidget>

class hashtablewidget : public QTableWidget
{
    Q_OBJECT
public:
    hashtablewidget(QWidget *parent = nullptr);
};

#endif // HASHTABLEWIDGET_H
