#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QMainWindow>
#include <QString>
#include "global.h"
#include "CPU.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    CPU s;
    QProcess *F, *D, *E, *M, *W;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QString get_reg(int);

    void refresh();
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
    void runA();
    void run();
    void clock();

    QString getState(int);
    QString getIns(int);
    QString getDec(int);
    QString getHex(int);
    QString getHexM(int);
    QString getHexI(int);

signals:
    void need_refresh();

public slots:
    void F_work();
    void D_work();
    void E_work();
    void M_work();
    void W_work();
    void clock_END();

private slots:
    void on_speed_sliderMoved(int position);

private:
    Ui::MainWindow *ui;
};

#endif // SIMULATOR_H
