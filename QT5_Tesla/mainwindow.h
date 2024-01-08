#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QButtonGroup>
//#include "clickableqlabel.h"
#include "device_5.h"

#define RED 0
#define VERT 1
#define BLEU 2

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    device ble_device;
public slots:
    void error_ble();
    void error_power_ble();
    void recup_val();
private slots:
    void on_dev_label_clicked();
    void disconnectedd();
    //void on_PB_Scan_clicked();
    //void scan_event_timer();
    //void scan_timeout();
   // void on_PB_Connect_clicked();
    void connect_radiobutton();
   // void connect_timer();
    void readble_timer();
    void on_AC_toggled(bool checked);
    void on_RAZButton_clicked();
    void finish_to_scan();
    void service_connected();
    void app_changed(int button);

private:
  //  bool alim_connected;
    QLabel *status_led,*etat_label,*dev_label,*status_text;
    void set_led(unsigned char i); // 0 rouge, 1 vert 2 bleu
    void set_app_led(QLabel *lab, unsigned char i);
    Ui::MainWindow *ui;
    QTimer *timer,*timer_read;
    QByteArray int_toqba(uint32_t value,uint size);
    QButtonGroup *buttonGroup;
    void launch_scan();

};


#endif // MAINWINDOW_H
