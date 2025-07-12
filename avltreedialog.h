#ifndef AVLTREEDIALOG_H
#define AVLTREEDIALOG_H

#include <QDialog>

namespace Ui {
class AVLtreeDialog;
}

class AVLtreeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AVLtreeDialog(QWidget *parent = nullptr);
    ~AVLtreeDialog();

private:
    Ui::AVLtreeDialog *ui;
};

#endif // AVLTREEDIALOG_H
