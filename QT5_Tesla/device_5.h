#ifndef DEVICE_H
#define DEVICE_H

#include <QDebug>

#include <qtbluetoothglobal.h>
#include <qbluetoothlocaldevice.h>
#include <QObject>
#include <QVariant>
#include <QList>
#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QLowEnergyController>
#include <QBluetoothServiceInfo>
#include <QBluetoothDeviceInfo>
#include "deviceinfo_5.h"
#include <QTimer>

class device:public QObject
{
Q_OBJECT
public:
   explicit device();
    ~device();
    void deviceSearch();
    void connectToService(const QString &address);
    void disconnectService();
    void write_pass_char(QByteArray command);
    void write_mode_char(QByteArray command);
    void read_pass_char();
    void read_mesu_char();
    void read_mode_char();
    QList<DeviceInfo*> m_devices;
    QList<QLowEnergyCharacteristic>  num2;
    bool scan_complet;
    bool dev_connected;
    bool error;
    QList<QBluetoothUuid> liste_service;
    QByteArray result_glob,b_a_passkey,b_a_mode,b_a_mesu;
public slots:
    //QBluetothDeviceDiscoveryAgent
    void addDevice(const QBluetoothDeviceInfo&);
    void scanFinished();
    void deviceScanError(QBluetoothDeviceDiscoveryAgent::Error);
    //QLowEnergyController
    void serviceDiscovered(const QBluetoothUuid &);
    void serviceScanDone();
    void controllerError(QLowEnergyController::Error);
    void deviceConnected();
    void deviceDisconnected();
    //QLowEnergyService
    void serv_pass_serviceStateChanged(QLowEnergyService::ServiceState s);
    void serv_pass_read(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void serviceError(QLowEnergyService::ServiceError e);
    void serv_mode_serviceStateChanged(QLowEnergyService::ServiceState s);
    void serv_mode_read(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void serv_mesu_serviceStateChanged(QLowEnergyService::ServiceState s);
    void serv_mesu_read(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void timeout();
private:
    QTimer *timer_device;
    QBluetoothDeviceDiscoveryAgent *discoveryagent;
    QLowEnergyController *m_control;
    QLowEnergyService *m_serv_mesu,*m_serv_mode,*m_serv_pass;
    DeviceInfo m_currentDevice;
    bool found_serv_mesu,found_serv_mode,found_serv_pass;
    QBluetoothUuid serv_mode,serv_mesu,serv_pass;

    void encrypt (uint32_t v[2], uint32_t k[4]);
    void decrypt (uint32_t v[2], uint32_t k[4]);
    uint32_t key[4]={0x5896,0x9696,0x9898,0x1234};
signals:
    void scan_fin();
    void service_connected();
    void device_error();
    void power_error();
    void new_val();
    void signal_disconnected();
};

#endif // DEVICE_H
