#include "device_5.h"
#include "deviceinfo_5.h"
#include "global.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QtEndian>
#include <QProcess>
#include <QMessageBox>
#include <QSysInfo>
#include <QTimer>
#include <QThread>
#include <QButtonGroup>
#include <math.h>
#include "clickablelabel.h"
#define OS_LINUX

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    status_led(new QLabel),
    etat_label(new QLabel),
    dev_label(new ClickableLabel),
    status_text(new QLabel),
    ui(new Ui::MainWindow)
{
    setWindowIcon(QIcon(":/alim.png"));
    ui->setupUi(this);
    timer = new QTimer(this); // timer pour timeout scan
    timer_read = new QTimer(this);
    connect(timer_read, SIGNAL(timeout()),this, SLOT(readble_timer()));
    buttonGroup = new QButtonGroup;
    buttonGroup->addButton(ui->App1);
    buttonGroup->addButton(ui->App2);
    buttonGroup->addButton(ui->App3);
    connect(buttonGroup,SIGNAL(buttonClicked(int)),this,SLOT(app_changed(int)));
    //barre status
    status_led->setFixedWidth(ui->statusBar->height()-4);
    status_led->setMargin(3);
    status_led->setAlignment(Qt::AlignRight);
    status_text->setAlignment(Qt::AlignRight);
    //     etat_label->setAlignment(Qt::AlignRight);
    //  etat_label->setIndent(0);
    ui->statusBar->addWidget(dev_label);
    ui->statusBar->addWidget(status_text,1);
    ui->statusBar->addWidget(status_led, 2);
    //   ui->statusBar->addWidget(etat_label,2);
    ui->App1->setEnabled(true);
    ui->App2->setEnabled(true);
    ui->App3->setEnabled(true);
    status_text->setText("Non Connecté");
    set_led(RED);
    dev_label->setText("XG - Enise ECL - CT");
    dev_label->setFixedWidth(120);
    connect(dev_label,SIGNAL(clicked()),this,SLOT(on_dev_label_clicked()));
    connect(&ble_device, SIGNAL(device_error()),this, SLOT(error_ble()));
    connect(&ble_device, SIGNAL(power_error()),this, SLOT(error_power_ble()));
    connect(&ble_device,SIGNAL(new_val()),this,SLOT(recup_val()));
    connect(&ble_device,SIGNAL(scan_fin()),this,SLOT(finish_to_scan()));
    connect(&ble_device,SIGNAL(signal_disconnected()),this,SLOT(disconnectedd()));
    connect(&ble_device,SIGNAL(service_connected()),this,SLOT(service_connected()));
    set_app_led(ui->AC_led,1);
    set_app_led(ui->DC_led,1);
    ui->Champ_max->hide();
    ui->lcd_max->hide();
    ui->Bmax->hide();
    ui->Champ_moy->hide();
    ui->lcd_moy->hide();
    ui->Bmoy->hide();
    ui->Champ_eff->hide();
    ui->lcd_eff->hide();
    ui->Beff->hide();
    ui->Frequence->hide();
    ui->lcd_f->hide();
    ui->Hz->hide();
    //on lance le scan des devices
    launch_scan();
    //on_PB_Scan_clicked();
/*  QPixmap cy;
    cy.load(":/Logo_ecl.png");
    ui->Enise->setPixmap(cy.scaled(ui->Enise->width(),ui->Enise->height(),Qt::KeepAspectRatioByExpanding));*/
}

MainWindow::~MainWindow()
{
    // usb_send_command("C",0);
#ifdef Q_OS_LINUX
    QProcess::execute("bluetoothctl remove "  + QString(App1_MAC));
    QProcess::execute("bluetoothctl remove "  + QString(App2_MAC));
    QProcess::execute("bluetoothctl remove "  + QString(App3_MAC));
#endif
    ble_device.disconnectService();
    delete ui;
}

void MainWindow::disconnectedd()
{
    set_led(RED);
    status_text->setText("Non Connecté");
    timer_read->stop();
    ui->Champ_moy->hide();
    ui->lcd_moy->hide();
    ui->Bmoy->hide();
    ui->Champ_eff->hide();
    ui->lcd_eff->hide();
    ui->Beff->hide();
    ui->Frequence->hide();
    ui->lcd_f->hide();
    ui->Hz->hide();
    ui->AC_led->setEnabled(false);
    ui->DC_led->setEnabled(false);
    ui->AC->setEnabled(false);
    ui->DC->setEnabled(false);
}

void MainWindow::set_led(unsigned char i)
{
    QPixmap led;
    if (i==2)
    {
        led.load(":/blue_led.png","png");
    }
    else if (i==1)
    {
        led.load(":/green_led.png","png");
    }
    else
    {
        led.load(":/red_led.png","png");
    }
    led=led.scaled(20,20);
    status_led->setPixmap(led);
    status_led->setAlignment(Qt::AlignRight);
}

void MainWindow::set_app_led(QLabel *lab, unsigned char i)
{
    QPixmap led;
    if (i==2)
    {
        led.load(":/blue_led.png","png");
    }
    else if (i==1)
    {
        led.load(":/green_led.png","png");
    }
    else
    {
        led.load(":/red_led.png","png");
    }
    led=led.scaled(16,16);
    lab->setPixmap(led);
   // lab->setAlignment(Qt::AlignRight);
}

void MainWindow::on_dev_label_clicked()
{
    QMessageBox::about(this,"Labo Mécatronique",
                       "<h4>Labo Mécatronique 342</h4>\n\n"
                       "Teslamêtre Bluetooth<br>"
                       "Champs Maximal : 2,5 T<br>"
                       "Copyright 2023 Enise.<br>"
                       "Support: Galtier Granjon Thévenon - Centrale Lyon - ENISE<br>"
                       "<a href=\"http://www.enise.fr\">http://www.enise.fr</a>" );

}

void MainWindow::launch_scan()
{
    set_app_led(ui->App1_led,0);
    set_app_led(ui->App1_led,0);
    set_app_led(ui->App3_led,0);
    ble_device.deviceSearch();
}
void MainWindow::finish_to_scan()
{
    unsigned char i;
    unsigned char app_found;
    app_found=0;
    if(ble_device.m_devices.count()>0)
    {
        for(i=0;i<ble_device.m_devices.count();i++)
        {
            if(ble_device.m_devices[i]->getAddress()==App1_MAC)
            {
                set_app_led(ui->App1_led,1);
                app_found+=1;
            }
            else if(ble_device.m_devices[i]->getAddress()==App2_MAC)
            {
                set_app_led(ui->App2_led,1);
                app_found+=2;
            }
            else if(ble_device.m_devices[i]->getAddress()==App3_MAC)
            {
                set_app_led(ui->App3_led,1);
                app_found+=4;
            }
        }
        // si on a trouve l'appareil selectionne alors on se connecte au service
        connect_radiobutton();
    }
    else
    {
        // si rien trouve on relance le scan des appareils
        launch_scan();
    }
}
void MainWindow::service_connected()
{
    QByteArray test;
    ui->AC->setEnabled(true);
    ui->DC->setEnabled(true);
    ui->DC_led->setEnabled(true);
    ui->AC_led->setEnabled(false);
    ui->Champ_moy->show();
    ui->lcd_moy->show();
    ui->Bmoy->show();
    ui->Champ_eff->hide();
    ui->lcd_eff->hide();
    ui->Beff->hide();
    ui->Frequence->hide();
    ui->lcd_f->hide();
    ui->Hz->hide();
    timer_read->start(300);
    test.append(0xDC);
    ble_device.write_mode_char(test);
}
void MainWindow::app_changed(int button)
{
    if(ble_device.dev_connected==true)
    {
        ble_device.disconnectService();
    }
    launch_scan();
}

void MainWindow::error_power_ble()
{
    set_led(BLEU);
    status_text->setText("Pb Bluetooth");
 //   alim_connected=false;
    timer->stop();
}

void MainWindow::error_ble()
{
    set_led(BLEU);
    status_text->setText("Pb Bluetooth");
  //  alim_connected=false;
    timer->stop();
    qDebug() << "error BLE scan";
}

void MainWindow::readble_timer()
{
    ble_device.read_mesu_char();
}

void MainWindow::recup_val()
{
    //qDebug() << "recu_val " << ble_device.b_a_passkey;
    double b_moy,b_eff,freq;
    uint8_t t_etat;
    QString etat_text="Etat: ";
    unsigned long cpt_mes=0;
    long moy;
    unsigned long long test;
    unsigned char tab[8];
    if(ble_device.b_a_passkey=="CLES OK! LETS GO")
    {
        set_led(VERT);
         status_text->setText("Connecté");
       // alim_connected=true;
    }
    if(ui->DC->isChecked())
    {
        b_moy = 2048.0*((double)(((((unsigned char)ble_device.b_a_mesu[3]<<24)+
                                    ((unsigned char)ble_device.b_a_mesu[2]<<16)+
                                    ((unsigned char)ble_device.b_a_mesu[1]<<8)+
                                    (unsigned char)ble_device.b_a_mesu[0]))))/(65535.0*2500.0);
        b_eff=0;
        freq=0;
        cpt_mes=0;
    }
    else
    {
        b_moy=0;
        moy=(long)(((unsigned char)ble_device.b_a_mesu[3]<<24)+
                ((unsigned char)ble_device.b_a_mesu[2]<<16)+
                ((unsigned char)ble_device.b_a_mesu[1]<<8)+
                ((unsigned char)ble_device.b_a_mesu[0]));
        tab[0]=(unsigned char)ble_device.b_a_mesu[4];
        tab[1]=(unsigned char)ble_device.b_a_mesu[5];
        tab[2]=(unsigned char)ble_device.b_a_mesu[6];
        tab[3]=(unsigned char)ble_device.b_a_mesu[7];
        tab[4]=(unsigned char)ble_device.b_a_mesu[8];
        tab[5]=(unsigned char)ble_device.b_a_mesu[9];
        tab[6]=(unsigned char)ble_device.b_a_mesu[10];
        tab[7]=(unsigned char)ble_device.b_a_mesu[11];
        test=tab[7]*pow(2,(7*8))+tab[6]*pow(2,(6*8))+tab[5]*pow(2,(5*8))+tab[4]*pow(2,(4*8))+tab[3]*pow(2,(3*8))+tab[2]*pow(2,(2*8))+tab[1]*pow(2,(1*8))+tab[7];
        cpt_mes=(unsigned long)(((unsigned char)ble_device.b_a_mesu[15]<<24)+
                                   ((unsigned char)ble_device.b_a_mesu[14]<<16)+
                                   ((unsigned char)ble_device.b_a_mesu[13]<<8)+
                                   ((unsigned char)ble_device.b_a_mesu[12]));

        if(cpt_mes==0)
        {

        }
        else
        {

        test=test/cpt_mes;
        b_eff = 2048.0*sqrt(test)/(65535.0);
        b_moy=2048.0*moy/(cpt_mes*65535);
        //qDebug() << "test: " << cpt_mes;
        cpt_mes=(unsigned long)(((unsigned char)ble_device.b_a_mesu[19]<<24)+
                ((unsigned char)ble_device.b_a_mesu[18]<<16)+
                ((unsigned char)ble_device.b_a_mesu[17]<<8)+
                ((unsigned char)ble_device.b_a_mesu[16]));
        freq=1.0/(cpt_mes*1e-6);
        }
    }
    //qDebug() << "test: " << cpt_mes;
    ui->lcd_moy->display(b_moy);
    ui->lcd_eff->display(b_eff);
    ui->lcd_f->display(freq);

}

void MainWindow::connect_radiobutton()
{
    if(ble_device.dev_connected==false)
    {
        if(ui->App1->isChecked())
        {
            ble_device.connectToService(App1_MAC);
            timer_read->start(300);
        }
        else if(ui->App2->isChecked())
        {
            ble_device.connectToService(App2_MAC);
            timer_read->start(300);
        }
        else if(ui->App3->isChecked())
        {
            ble_device.connectToService(App3_MAC);
            timer_read->start(300);
        }
    }
    else
    {
        ble_device.disconnectService();
    }
}

QByteArray MainWindow::int_toqba(uint32_t value,uint size)
{
    QByteArray result(size,0);
    qToLittleEndian(value,result.data());
    return result;
}

void MainWindow::on_AC_toggled(bool checked)
{
    QByteArray test;
    if(checked==true)
    {
        test.append(0xAC);
        ble_device.write_mode_char(test);
        //ui->AC->setEnabled(true);
        ui->AC_led->setEnabled(true);
        ui->DC_led->setEnabled(false);
        //ui->DC->setEnabled(false);
        ui->Champ_moy->show();
        ui->lcd_moy->show();
        ui->Bmoy->show();
        ui->Champ_eff->show();
        ui->lcd_eff->show();
        ui->Beff->show();
        ui->Frequence->show();
        ui->lcd_f->show();
        ui->Hz->show();
    }
    else
    {
        test.append(0xDC);
        ble_device.write_mode_char(test);
        //ui->DC->setEnabled(true);
        ui->DC_led->setEnabled(true);
        //ui->AC->setEnabled(false);
        ui->AC_led->setEnabled(false);
        ui->Champ_moy->show();
        ui->lcd_moy->show();
        ui->Bmoy->show();
        ui->Champ_eff->hide();
        ui->lcd_eff->hide();
        ui->Beff->hide();
        ui->Frequence->hide();
        ui->lcd_f->hide();
        ui->Hz->hide();
    }
}

void MainWindow::on_RAZButton_clicked()
{
    QByteArray test;
    if(ble_device.dev_connected==true)
    {
        test.append(0xF0);
        ble_device.write_mode_char(test);
        QThread::msleep(100);
        test[0]=0xDC;
        ble_device.write_mode_char(test);
        QThread::msleep(100);
        ui->DC->setChecked(true);
    }
  //  buttonGroup->setExclusive(false);
  //  buttonGroup->checkedButton()->setChecked(false);
  //  buttonGroup->setExclusive(true);
}
