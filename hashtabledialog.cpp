#include "hashtabledialog.h"
#include "ui_hashtabledialog.h"

hashtableDialog::hashtableDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::hashtableDialog)
{
    ui->setupUi(this);
}

hashtableDialog::~hashtableDialog()
{
    delete ui;
}
