#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    Ui::MainWindow *ui;

    QProcess *calculator;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


signals:
public slots:
    void slot_cal();
    void slot_btn();

};

#endif // MAINWINDOW_H
