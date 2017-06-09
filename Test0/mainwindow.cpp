#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QString>
#include <QProcess>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	calculator = new QProcess(this);
    QStringList list;
    calculator->start("F:/2017Spring/ICS/Y86-Lab/Test0/aPlusB.exe", list);
    //连接信号与槽
    connect(ui->start, SIGNAL(clicked(bool)), this, SLOT(slot_btn()));
    connect(calculator, SIGNAL(readyRead()), this, SLOT(slot_cal()));

}

MainWindow::~MainWindow()
{
    calculator -> terminate();
    calculator -> waitForFinished();
    delete calculator;
    delete ui;
}

void MainWindow::slot_btn(){
    QString a = ui->TextA->text();
    QString b = ui->TextB->text();
    QString c = a + " " + b + "\n";
    qDebug() << c;
    calculator -> write(c.toUtf8());
}

void MainWindow::slot_cal(){
    qDebug() << calculator->readAllStandardOutput();
}
