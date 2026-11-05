#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include "../connect.h"

void MainWindow::fun1(bool check){
    Gethist A = Gethist(Data);
    QMessageBox::information(this, "提示", "来是come");
}

void MainWindow::fun2(bool check) {
    QMessageBox::information(this, "提示", "去是go");
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->come, &QPushButton::clicked, this, &MainWindow::fun1);
    connect(ui->go, &QPushButton::clicked, this, &MainWindow::fun2);
}

MainWindow::~MainWindow()
{
    delete ui;
}
