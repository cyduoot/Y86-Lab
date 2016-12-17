#include "simulator.h"
#include "ui_simulator.h"
#include "CPU.h"
#include "global.h"
#include <bits/stdc++.h>
#include <QString>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QElapsedTimer>

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
    return "";
}

QString MainWindow::get_opl(int x)
{
    switch (x)
    {
    case 0:return "addl    ";
    case 1:return "subl    ";
    case 2:return "andl    ";
    case 3:return "xorl    ";
    }
    return "opl     ";
}

QString MainWindow::get_jxx(int x)
{
    switch (x)
    {
    case 0:return "jmp     ";
    case 1:return "jle     ";
    case 2:return "jl      ";
    case 3:return "je      ";
    case 4:return "jne     ";
    case 5:return "jge     ";
    case 6:return "jg      ";
    }
    return "jxx     ";
}

QString MainWindow::get_cmovxx(int x)
{
    switch (x)
    {
    case 0:return "rrmovl  ";
    case 1:return "cmovle  ";
    case 2:return "cmovl   ";
    case 3:return "cmove   ";
    case 4:return "cmovne  ";
    case 5:return "cmovge  ";
    case 6:return "comvg   ";
    }
    return "cmovxx  ";
}

void MainWindow::make_ass()
{
    QString ass_code;
    int head = code_head;
    while (head < len)
    {
        ass_code += getHex(head);
        ass_code += "  |  ";
        int V;
        switch (0xf0 & s.bin_code[head])
        {
        case 0:
            ass_code += "halt\n";
            head++;
            continue;
            break;
        case 0x10:
            ass_code += "nop\n";
            head++;
            continue;
            break;
        case 0x20:
            ass_code += get_cmovxx(0xf & s.bin_code[head]);
            head++;
            ass_code += get_reg((0xf0 & s.bin_code[head]) >> 4);
            ass_code += ", ";
            ass_code += get_reg(0xf & s.bin_code[head]);
            ass_code += "\n";
            head++;
            break;
        case 0x30:
            ass_code += "irmovl  ";
            head++;
            V = 0;
            for (int i = 4; i >= 1; i --)
                V = (V << 4) + s.bin_code[head + i];
            ass_code += "$" + getHex(V);
            ass_code += ", ";
            ass_code += get_reg(0xf & s.bin_code[head]) + "\n";
            head += 5;
            break;
        case 0x40:
            ass_code += "rmmovl  ";
            head++;
            V = 0;
            for (int i = 4; i >= 1; i --)
                V = (V << 4) + s.bin_code[head + i];

            ass_code += get_reg((0xf0 & s.bin_code[head]) >> 4) + ", ";
            ass_code += "$" + getHex(V);
            ass_code += "(" + get_reg(0xf & s.bin_code[head]) + ")" + "\n";
            head += 5;
            break;
        case 0x50:
            ass_code += "mrmovl  ";
            head++;
            V = 0;
            for (int i = 4; i >= 1; i --)
                V = (V << 4) + s.bin_code[head + i];

            ass_code += "$" + getHex(V);
            ass_code += "(" + get_reg(0xf & s.bin_code[head]) + ")";
            ass_code +=", " + get_reg((0xf0 & s.bin_code[head]) >> 4) + "\n";
            head += 5;
            break;
        case 0x60:
            ass_code += get_opl(0xf & s.bin_code[head]);
            head++;
            ass_code += get_reg((0xf0 & s.bin_code[head]) >> 4);
            ass_code += ", ";
            ass_code += get_reg(0xf & s.bin_code[head]);
            ass_code += "\n";
            head++;
            break;
        case 0x70:
            ass_code += get_jxx(0xf & s.bin_code[head]);
            head++;
            V = 0;
            for (int i = 3; i >=0; i --)
                V = (V << 4) + s.bin_code[head + i];
            ass_code += "$" + getHex(V) + "\n";
            head += 4;
            break;
        case 0x80:
            ass_code += "call    ";
            head++;
            V = 0;
            for (int i = 3; i >=0; i --)
                V = (V << 4) + s.bin_code[head + i];
            ass_code += "$" + getHex(V) + "\n";
            head += 4;
            break;
        case 0x90:
            ass_code += "ret\n";
            head++;
            break;
        case 0xA0:
            ass_code += "pushl   ";
            head++;
            ass_code += get_reg((0xf0 & s.bin_code[head]) >> 4) + "\n";
            head++;
            break;
        case 0xB0:
            ass_code += "ret     ";
            head++;
            ass_code += get_reg((0xf0 & s.bin_code[head]) >> 4) + "\n";
            head++;
            break;
        default:
            ass_code += "error!\n";
        }
    }
    ui->Code->setText(ass_code);
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

void MainWindow::clock()
{
    s.Control();
    s.Send();
    s.Write();
    s.Memory();
    s.Execute();
    s.Decode();
    s.Fetch();
    s.circle_time++;
}

void MainWindow::init()
{
    s.prepare();
    ui->speed->setValue(50);
    ui->nowspeed->setNum(50);
    s.speed = 50;
    refresh_register();
    refresh_all();
    refresh_memory();
}

void MainWindow::load()
{
    QString file_path;
    init();
    file_path = ui->path->text();
    s.read_in(file_path);
    make_ass();
    refresh_register();
    refresh_all();
    refresh_memory();
}

void MainWindow::next()
{
    if (s.Stat != SAOK)
        QMessageBox::warning(this,"Warnning","Program is not running!",QMessageBox::Yes);
    else
    {
        clock();
        refresh_register();
        refresh_all();
        refresh_memory();
    }
}

void MainWindow::run()
{
    s.running = true;
    while (s.running)
    {
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
        refresh_register();
        refresh_all();
        refresh_memory();
    }
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
    ui->F_ifun->setText(getHex(s.f_ifun));
    ui->F_icode->setText(getIns(s.f_icode));
    ui->F_stall->setText(s.F_stall?"true":"false");
    ui->F_bubble->setText(s.F_bubble?"true":"false");

    ui->D_STAT->setText(getState(s.D_stat));
    ui->D_icode->setText(getIns(s.D_icode));
    ui->D_ifun->setText(getHex(s.D_ifun));
    ui->D_rA->setText(getHex(s.D_rA));
    ui->D_rB->setText(getHex(s.D_rB));
    ui->D_valC->setText(getHex(s.D_valC));
    ui->D_valP->setText(getHex(s.D_valP));
    ui->D_stall->setText(s.D_stall?"true":"false");
    ui->D_bubble->setText(s.D_bubble?"true":"false");

    ui->E_STAT->setText(getState(s.E_stat));
    ui->E_icode->setText(getIns(s.E_icode));
    ui->E_ifun->setText(getHex(s.E_ifun));
    ui->E_valC->setText(getHex(s.E_valC));
    ui->E_valA->setText(getHex(s.E_valA));
    ui->E_valB->setText(getHex(s.E_valB));
    ui->E_srcA->setText(getHex(s.E_srcA));
    ui->E_srcB->setText(getHex(s.E_srcB));
    ui->E_dstE->setText(getHex(s.E_dstE));
    ui->E_dstM->setText(getHex(s.E_dstM));
    ui->E_stall->setText(s.E_stall?"true":"false");
    ui->E_bubble->setText(s.E_bubble?"true":"false");

    ui->M_STAT->setText(getState(s.M_stat));
    ui->M_icode->setText(getIns(s.M_icode));
    ui->M_Cnd->setText((s.M_Cnd?"true":"false"));
    ui->M_valE->setText(getHex(s.M_valE));
    ui->M_valA->setText(getHex(s.M_valA));
    ui->M_dstE->setText(getHex(s.M_dstE));
    ui->M_dstM->setText(getHex(s.M_dstM));
    ui->M_stall->setText(s.M_stall?"true":"false");
    ui->M_bubble->setText(s.M_bubble?"true":"false");

    ui->W_STAT->setText(getState(s.W_stat));
    ui->W_icode->setText(getIns(s.W_icode));
    ui->W_valE->setText(getHex(s.W_valE));
    ui->W_valM->setText(getHex(s.W_valM));
    ui->W_dstE->setText(getHex(s.W_dstE));
    ui->W_dstM->setText(getHex(s.W_dstM));
    ui->W_stall->setText(s.W_stall?"true":"false");
    ui->W_bubble->setText(s.W_bubble?"true":"false");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_speed_sliderMoved(int position)
{
    ui->nowspeed->setNum(position);
    s.speed = position;
}
