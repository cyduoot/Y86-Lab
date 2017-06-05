#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QMainWindow>
#include <QString>
#include "CPU.h"
#include "global.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QString get_reg(int);
/*
    QString get_opl(int);
    QString get_jxx(int);
    QString get_cmovxx(int);
    void make_ass();
*/
    void refresh_register();
    void refresh_memory();
    void refresh_all();
    void refresh_operation();

    void clear();
    void load_code();
    void init();
    void load();
    void next();
    void stop();
    void run();
    void clock();

    QString getState(int);
    QString getIns(int);
    QString getDec(int);
    QString getHex(int);
    QString getHexM(int);
    QString getHexI(int);


    //void test();

private slots:
    void on_speed_sliderMoved(int position);


private:
    Ui::MainWindow *ui;
    CPU s;
};

#endif // SIMULATOR_H
