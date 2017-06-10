#include "simulator.h"
#include "ui_simulator.h"
#include "global.h"
#include <bits/stdc++.h>
#include <QString>
#include <QFile>
#include <QDebug>
#include <QMessageBox>
#include <QTextStream>
#include <QElapsedTimer>
#include <windows.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->Button_reset, &QPushButton::clicked, this, &MainWindow::init);
    connect(ui->Button_load, &QPushButton::clicked, this, &MainWindow::load);
    connect(ui->Button_next, &QPushButton::clicked, this, &MainWindow::next);
    connect(ui->Button_run, &QPushButton::clicked, this, &MainWindow::run);
    connect(ui->Button_stop, &QPushButton::clicked, this, &MainWindow::stop);
    connect(ui->Button_code, &QPushButton::clicked, this, &MainWindow::load_code);
    connect(ui->Button_clear, &QPushButton::clicked, this, &MainWindow::clear);
    connect(this, SIGNAL(need_refresh()), this, SLOT(clock_END()));

    F = new  QProcess();
    F->start("F:/2017Spring/ICS/Y86-Lab/CPU-parallel-process/source-code/FDEMW/F.exe");
    connect(F, SIGNAL(readyRead()), this, SLOT(F_work()));

    D = new  QProcess();
    D->start("F:/2017Spring/ICS/Y86-Lab/CPU-parallel-process/source-code/FDEMW/D.exe");
    connect(D, SIGNAL(readyRead()), this, SLOT(D_work()));

    E = new  QProcess();
    E->start("F:/2017Spring/ICS/Y86-Lab/CPU-parallel-process/source-code/FDEMW/E.exe");
    connect(E, SIGNAL(readyRead()), this, SLOT(E_work()));

    M = new  QProcess();
    M->start("F:/2017Spring/ICS/Y86-Lab/CPU-parallel-process/source-code/FDEMW/M.exe");
    connect(M, SIGNAL(readyRead()), this, SLOT(M_work()));

    W = new  QProcess();
    W->start("F:/2017Spring/ICS/Y86-Lab/CPU-parallel-process/source-code/FDEMW/W.exe");
    connect(W, SIGNAL(readyRead()), this, SLOT(W_work()));
}

MainWindow::~MainWindow(){
    F -> kill();
    F -> waitForFinished();
    delete F;
    D -> kill();
    D -> waitForFinished();
    delete D;
    E -> kill();
    E -> waitForFinished();
    delete E;
    M -> kill();
    M -> waitForFinished();
    delete M;
    W -> kill();
    W -> waitForFinished();
    delete W;

    delete ui;
}

QString MainWindow::get_reg(int x)
{
    switch (x)
    {
    case 0:return "%eax";
    case 1:return "%ecx";
    case 2:return "%edx";
    case 3:return "%ebx";
    case 4:return "%esp";
    case 5:return "%ebp";
    case 6:return "%esi";
    case 7:return "%edi";
    }
    return "RNONE";
}

void MainWindow::clear()
{
    ui->Code->setText("");
}

void MainWindow::load_code()
{
    QString path;
    path = ui->path_code->text();
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this,"Warnning","File can't open!",QMessageBox::Yes);
    }
    QString outcode;
    outcode = "";
    while(!file.atEnd())
    {
        QByteArray line = file.readLine();
        QString str(line);
        outcode += str;
    }
    ui->Code->setText(outcode);
}

void MainWindow::refresh_register()
{
    ui->text_eax->setText(getHex(s.Reg[0]));
    ui->text_ecx->setText(getHex(s.Reg[1]));
    ui->text_edx->setText(getHex(s.Reg[2]));
    ui->text_ebx->setText(getHex(s.Reg[3]));
    ui->text_esi->setText(getHex(s.Reg[4]));
    ui->text_edi->setText(getHex(s.Reg[5]));
    ui->text_esp->setText(getHex(s.Reg[6]));
    ui->text_ebp->setText(getHex(s.Reg[7]));
}

void MainWindow::refresh_memory()
{
    QString ss;
    ss = "";
    for (std::map<int, int> :: iterator it = mem.begin(); it != mem.end(); it++)
    {
        ss+= getHex(it->first) + " : " + getHexM(it->second) + "\n";
    }
    ui->memorymonitor->setText(ss);
}
\
void MainWindow::clock_END(){
    s.Forward_Deal();
    s.circle_time++;
    refresh();
    if (s.running)
        runA();
}

void MainWindow::clock()
{
    s.Control();
    s.Send();
    s._ZF = s.ZF;
    s._SF = s.SF;
    s._OF = s.OF;
    s._CF = s.CF;
    s.Fetch(F);
    s.Decode(D);
    s.Execute(E);
    s.Memory(M);
    s.Write(W);
}

void MainWindow::refresh(){
    refresh_register();
    refresh_all();
    refresh_memory();
    refresh_operation();
}

void MainWindow::init()
{
    s.prepare();

    ui->speed->setValue(50);
    ui->nowspeed->setNum(50);
    s.speed = 50;
    refresh();
}

void MainWindow::load()
{
    QString file_path;
    init();
    file_path = ui->path->text();
    s.read_in(file_path);
    refresh();
}

void MainWindow::next()
{
    if (s.Stat != SAOK)
        QMessageBox::warning(this,"Warnning","Program is not running!",QMessageBox::Yes);
    else
    {
        clock();
    }
}

void MainWindow::runA(){
    if (!s.running) return;
    QElapsedTimer t;
    t.start();
    while(t.elapsed()<1000/s.speed)
        QCoreApplication::processEvents();
    if (s.Stat != SAOK)
    {
        QMessageBox::warning(this,"Warnning","Program is not running!",QMessageBox::Yes);
        s.running = false;
        return;
    }
    clock();
}

void MainWindow::run(){
    s.running = true;
    runA();
}

void MainWindow::stop()
{
    s.running = false;
}

QString MainWindow::getHex(int x) {
    QString ans = "0x";
    int tmp;
    for (int i = 7; i >= 0; i--) {
        tmp = x >>(i*4) & (0xf);
        if (tmp <= 9) ans = ans + (tmp + '0');
        else ans = ans + (tmp - 10 + 'a');
    }
    return ans;
}

QString MainWindow::getHexM(int x)
{
    QString ans = "";
    int tmp;
    for (int i = 1; i >= 0; i--) {
        tmp = x >>(i*4) & (0xf);
        if (tmp <= 9) ans = ans + (tmp + '0');
        else ans = ans + (tmp - 10 + 'a');
    }
    return ans;
}

QString MainWindow::getHexI(int x)
{
    QString ans = "";
    int tmp;
    tmp = x & (0xf);
    if (tmp <= 9) ans = ans + (tmp + '0');
    else ans = ans + (tmp - 10 + 'a');
    return ans;
}


QString MainWindow::getDec(int x)
{
    QString ans;
    ans = "";
    if (!x)
        return "0";
    int tmp;
    tmp = x;
    while (tmp)
    {
        ans = (tmp % 10 + '0' + ans);
        tmp /= 10;
    }
    return ans;
}

QString MainWindow::getState(int stat)
{
    if (stat == 1)
        return "SAOK";
    if (stat == 2)
        return "SADR";
    if (stat == 3)
        return "SINS";
    if (stat == 4)
        return "SHLT";
    return "SBUB";
}

QString MainWindow::getIns(int x)
{
    switch (x)
    {
    case 0: return "IHALT";
    case 1: return "INOP";
    case 2: return "IRRMOVL";
    case 3: return "IIRMOVL";
    case 4: return "IRMMOVL";
    case 5: return "IMRMOVL";
    case 6: return "IOPL";
    case 7: return "IJXX";
    case 8: return "ICALL";
    case 9: return "IRET";
    case 10: return "IPUSHL";
    case 11: return "IPOPL";
    }
    return "ERROR!";
}

void MainWindow::refresh_all()
{
    ui->STAT->setText(getState(s.Stat));
    ui->cycle->setText(getDec(s.circle_time));
    ui->ZF->setText(getDec(s.ZF));
    ui->OF->setText(getDec(s.OF));
    ui->SF->setText(getDec(s.SF));
    ui->CF->setText(getDec(s.CF));
    //ui->cycle->setText(QVariant(Current_cycle).toString());

    ui->F_STAT->setText(getState(s.f_stat));
    ui->F_predPC->setText(getHex(s.F_predPC));
    ui->f_predPC->setText(getHex(s.f_predPC));
    ui->F_ifun->setText(getHexI(s.f_ifun));
    ui->F_icode->setText(getIns(s.f_icode));
    ui->F_stall->setText(s.F_stall?"true":"false");
    ui->F_bubble->setText(s.F_bubble?"true":"false");

    ui->D_STAT->setText(getState(s.D_stat));
    ui->D_icode->setText(getIns(s.D_icode));
    ui->D_ifun->setText(getHexI(s.D_ifun));
    ui->D_rA->setText(get_reg(s.D_rA));
    ui->D_rB->setText(get_reg(s.D_rB));
    ui->D_valC->setText(getHex(s.D_valC));
    ui->D_valP->setText(getHex(s.D_valP));
    ui->D_stall->setText(s.D_stall?"true":"false");
    ui->D_bubble->setText(s.D_bubble?"true":"false");

    ui->E_STAT->setText(getState(s.E_stat));
    ui->E_icode->setText(getIns(s.E_icode));
    ui->E_ifun->setText(getHexI(s.E_ifun));
    ui->E_valC->setText(getHex(s.E_valC));
    ui->E_valA->setText(getHex(s.E_valA));
    ui->E_valB->setText(getHex(s.E_valB));
    ui->E_srcA->setText(get_reg(s.E_srcA));
    ui->E_srcB->setText(get_reg(s.E_srcB));
    ui->E_dstE->setText(get_reg(s.E_dstE));
    ui->E_dstM->setText(get_reg(s.E_dstM));
    ui->E_stall->setText(s.E_stall?"true":"false");
    ui->E_bubble->setText(s.E_bubble?"true":"false");

    ui->M_STAT->setText(getState(s.M_stat));
    ui->M_icode->setText(getIns(s.M_icode));
    ui->M_Cnd->setText((s.M_Cnd?"true":"false"));
    ui->M_valE->setText(getHex(s.M_valE));
    ui->M_valA->setText(getHex(s.M_valA));
    ui->M_dstE->setText(get_reg(s.M_dstE));
    ui->M_dstM->setText(get_reg(s.M_dstM));
    ui->M_stall->setText(s.M_stall?"true":"false");
    ui->M_bubble->setText(s.M_bubble?"true":"false");

    ui->W_STAT->setText(getState(s.W_stat));
    ui->W_icode->setText(getIns(s.W_icode));
    ui->W_valE->setText(getHex(s.W_valE));
    ui->W_valM->setText(getHex(s.W_valM));
    ui->W_dstE->setText(get_reg(s.W_dstE));
    ui->W_dstM->setText(get_reg(s.W_dstM));
    ui->W_stall->setText(s.W_stall?"true":"false");
    ui->W_bubble->setText(s.W_bubble?"true":"false");
}

void MainWindow::refresh_operation()
{
    QString f_op= QString::fromStdString(F_op);
    QString d_op= QString::fromStdString(D_op);
    QString e_op= QString::fromStdString(E_op);
    QString m_op= QString::fromStdString(M_op);
    QString w_op= QString::fromStdString(W_op);
    ui->f_opt->setText(f_op);
    ui->d_opt->setText(d_op);
    ui->e_opt->setText(e_op);
    ui->m_opt->setText(m_op);
    ui->w_opt->setText(w_op);
}

void MainWindow::on_speed_sliderMoved(int position)
{
    ui->nowspeed->setNum(position);
    s.speed = position;
}

void MainWindow::F_work(){
    qDebug() << "F";
    if (s.F_ret(F)){
        qDebug() << 1;
        emit need_refresh();
    }
}

void MainWindow::D_work(){
    qDebug() << "D";
    if (s.D_ret(D)){
        qDebug() << 1;
        emit need_refresh();
    }
}

void MainWindow::E_work(){
    qDebug() << "E";
    if (s.E_ret(E)){
        qDebug() << 1;
        emit need_refresh();
    }
}

void MainWindow::M_work(){
    qDebug() << "M";
    if (s.M_ret(M)){
        qDebug() << 1;
        emit need_refresh();
    }
}

void MainWindow::W_work(){
    qDebug() << "W";
/*    qDebug() << "F_done :" << s.F_done;
    qDebug() << "D_done :" << s.D_done;
    qDebug() << "E_done :" << s.E_done;
    qDebug() << "M_done :" << s.M_done;
    qDebug() << "W_done :" << s.W_done;*/
    if (s.W_ret(W)){
        qDebug() << 1;
        emit need_refresh();
    }
}
