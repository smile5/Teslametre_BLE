#include "device_5.h"
#include "global.h"
#include <QRandomGenerator>

device::device() : m_control(nullptr), m_serv_mesu(nullptr),m_serv_mode(nullptr),m_serv_pass(nullptr),m_currentDevice(QBluetoothDeviceInfo())
{
    timer_device = new QTimer(this);
    scan_complet=false;
    dev_connected=false;
    qDeleteAll(m_devices);
    m_devices.clear();

    discoveryagent = new QBluetoothDeviceDiscoveryAgent(this);
    discoveryagent->setLowEnergyDiscoveryTimeout(3000);
    discoveryagent->setInquiryType(QBluetoothDeviceDiscoveryAgent::LimitedInquiry);
    connect(discoveryagent, SIGNAL(deviceDiscovered(const QBluetoothDeviceInfo&)),this, SLOT(addDevice(const QBluetoothDeviceInfo&)));
    connect(discoveryagent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)),this, SLOT(deviceScanError(QBluetoothDeviceDiscoveryAgent::Error)));
    connect(discoveryagent, SIGNAL(finished()), this, SLOT(scanFinished()));
}

void device::timeout()
{
    scan_fin();
    //scan_complet=true;
    discoveryagent->stop();
    timer_device->stop();
}

device::~device()
{
    delete discoveryagent;
    qDeleteAll(m_devices);
    m_devices.clear();
}

void device::addDevice(const QBluetoothDeviceInfo &device)
{ bool valid,cached;
    QString array;
    qint16 RSSI;

    if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
            DeviceInfo *dev = new DeviceInfo(device);
            valid=device.isValid();
            cached=device.isCached();
            array=device.name();
            RSSI=device.rssi();
            //qWarning() << "Discovered LE Device name: " << device.name() << " Address: " << device.address().toString() << "RSSI: " << RSSI ;
            if(device.isValid())
            {
                qWarning() << "Discovered LE Device name: " << device.name() << " Address: " << device.address().toString() << "RSSI: " << RSSI;
                m_devices.append(dev);
            }
            qWarning() << "Low Energy device found. Scanning for more...";
    }
}
void device::deviceScanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    if (error == QBluetoothDeviceDiscoveryAgent::PoweredOffError){
        power_error();
        qWarning() << "The Bluetooth adaptor is powered off, power it on before doing discovery.";
    }
    else if (error == QBluetoothDeviceDiscoveryAgent::InputOutputError)
        qWarning() <<"Writing or reading from the device resulted in an error.";
    else
        qWarning() << "An unknown error has occurred.";
}


void device::scanFinished()
{
    if (m_devices.size() == 0)
    {
        qWarning() << "No Low Energy devices found";
    }
    else
    {
    }
    scan_complet=true;
}

void device::deviceSearch()
{
    discoveryagent->stop();
    qDeleteAll(m_devices);
    m_devices.clear();
    disconnect(timer_device, SIGNAL(timeout()),this, SLOT(timeout()));
    connect(timer_device, SIGNAL(timeout()),this, SLOT(timeout()));
    timer_device->start(discoveryagent->lowEnergyDiscoveryTimeout());

    discoveryagent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

void device::connectToService(const QString &address)
{
    bool deviceFound = false;
    this->error=false;
    for (int i = 0; i < m_devices.size(); i++) {
        if (((DeviceInfo*)m_devices.at(i))->getAddress() == address )
        {
            m_currentDevice.setDevice(((DeviceInfo*)m_devices.at(i))->getDevice());
            deviceFound = true;
            break;
        }
    }

    if (!deviceFound) {
        qWarning() <<"No device stop";
        return;
    }

    if (m_control) {
        m_control->disconnectFromDevice();
        delete m_control;
   //     m_control = 0;

    }
    //! [Connect signals]
    m_control = new QLowEnergyController(m_currentDevice.getDevice(), this);
    m_control->setRemoteAddressType(QLowEnergyController::RandomAddress);
    connect(m_control, SIGNAL(serviceDiscovered(QBluetoothUuid)),this, SLOT(serviceDiscovered(QBluetoothUuid)));
    connect(m_control, SIGNAL(discoveryFinished()),this, SLOT(serviceScanDone()));
    connect(m_control, SIGNAL(error(QLowEnergyController::Error)),this, SLOT(controllerError(QLowEnergyController::Error)));
    connect(m_control, SIGNAL(connected()),this, SLOT(deviceConnected()));
    connect(m_control, SIGNAL(disconnected()),this, SLOT(deviceDisconnected()));

    m_control->connectToDevice();
    service_connected();
    //! [Connect signals]
}
//! [Connecting to service]

void device::deviceConnected()
{
    m_control->discoverServices();
}

void device::deviceDisconnected()
{
    dev_connected=false;
    signal_disconnected();
    qWarning() << "BLE Disconnected";
}
void device::disconnectService()
{
 /*   const QLowEnergyCharacteristic hrChar = num2.last();
    const QLowEnergyDescriptor m_notificationDesc = hrChar.descriptor(
                QBluetoothUuid::ClientCharacteristicConfiguration);
    if (m_notificationDesc.isValid())
        m_service->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0000"));  */
    m_control->disconnectFromDevice();
}

//! [Connecting to service]

//! [Filter device service 1]
void device::serviceDiscovered(const QBluetoothUuid &gatt)
{
    liste_service = m_control->services();
    if (gatt.toString() == Serv_Mode_UUID) {
        qWarning() << "Custom service discovered. Mode";
        serv_mode=gatt;
        found_serv_mode = true;
    }
    if (gatt.toString() == Serv_Passkey_UUID) {
        qWarning() << "Custom service discovered. PassKey";
        serv_pass=gatt;
        found_serv_pass = true;
    }
    if (gatt.toString() == Serv_Mesures_UUID) {
        qWarning() << "Custom service discovered. Mesure";
        serv_mesu=gatt;
        found_serv_mesu = true;
    }
}
//! [Filter device service 1]

void device::serviceScanDone()
{
    delete m_serv_mesu;
    delete m_serv_mode;
    delete m_serv_pass;
    m_serv_mesu = nullptr;
    m_serv_mode = nullptr;
    m_serv_pass = nullptr;

    //! [Filter device service 2]
    if (found_serv_mesu) {
           m_serv_mesu = m_control->createServiceObject(serv_mesu,this);
    }
    if (found_serv_mode) {
           m_serv_mode = m_control->createServiceObject(serv_mode,this);
    }
    if (found_serv_pass) {
           m_serv_pass = m_control->createServiceObject(serv_pass,this);
    }

    if (!m_serv_mesu && !m_serv_mode && !m_serv_pass) {
        return;
    }
    if (m_serv_pass)
    {
        connect(m_serv_pass, SIGNAL(stateChanged(QLowEnergyService::ServiceState)),this, SLOT(serv_pass_serviceStateChanged(QLowEnergyService::ServiceState)));
        connect(m_serv_pass, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)),this, SLOT(serv_pass_read(QLowEnergyCharacteristic,QByteArray)));
        connect(m_serv_pass,SIGNAL(characteristicRead(QLowEnergyCharacteristic,QByteArray)),this,SLOT(serv_pass_read(QLowEnergyCharacteristic,QByteArray)));
        m_serv_pass->discoverDetails();
    }
    if (m_serv_mode)
    {
        connect(m_serv_mode, SIGNAL(stateChanged(QLowEnergyService::ServiceState)),this, SLOT(serv_mode_serviceStateChanged(QLowEnergyService::ServiceState)));
        connect(m_serv_mode, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)),this, SLOT(serv_mode_read(QLowEnergyCharacteristic,QByteArray)));
        connect(m_serv_mode,SIGNAL(characteristicRead(QLowEnergyCharacteristic,QByteArray)),this,SLOT(serv_mode_read(QLowEnergyCharacteristic,QByteArray)));
        m_serv_mode->discoverDetails();
    }
    if (m_serv_mesu)
    {
        connect(m_serv_mesu, SIGNAL(stateChanged(QLowEnergyService::ServiceState)),this, SLOT(serv_mesu_serviceStateChanged(QLowEnergyService::ServiceState)));
        connect(m_serv_mesu, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)),this, SLOT(serv_mesu_read(QLowEnergyCharacteristic,QByteArray)));
        connect(m_serv_mesu,SIGNAL(characteristicRead(QLowEnergyCharacteristic,QByteArray)),this,SLOT(serv_mesu_read(QLowEnergyCharacteristic,QByteArray)));
        m_serv_mesu->discoverDetails();
    }

}

//! [Error handling]
void device::controllerError(QLowEnergyController::Error error)
{
    device_error();
}

//! [service handling]

void device::serv_mesu_serviceStateChanged(QLowEnergyService::ServiceState s)
{
    num2=m_serv_mesu->characteristics();
    switch (s) {
    case QLowEnergyService::ServiceDiscovered:
    {
        const QLowEnergyCharacteristic hrChar1 = num2.first();
        const QLowEnergyCharacteristic hrChar = num2.last();

        if (!hrChar.isValid()) {
            break;
        }

        const QLowEnergyDescriptor m_notificationDesc = hrChar.descriptor(
                    QBluetoothUuid::ClientCharacteristicConfiguration);
        if (m_notificationDesc.isValid()) {
            m_serv_mesu->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0100"));
            //m_start = QDateTime::currentDateTime();
          //  dev_connected=true;
        }

        //dev_connected=true;
        qWarning() << "notification active";
        read_pass_char();
        break;
    }
    default:
        //nothing for now
        break;
    }

}

void device::serv_pass_serviceStateChanged(QLowEnergyService::ServiceState s)
{
    switch (s) {
    case QLowEnergyService::ServiceDiscovered:
    {
        write_pass_char(0);
        dev_connected=true;
        qWarning() << "send password";
        break;
    }
    default:
        //nothing for now
        break;
    }

}

void device::serv_mode_serviceStateChanged(QLowEnergyService::ServiceState s)
{

}

void device::serv_pass_read(const QLowEnergyCharacteristic &c,
                               const QByteArray &value)
{
        b_a_passkey=value;
        new_val();
        qWarning() << "hey hey" << value;
}
void device::serv_mesu_read(const QLowEnergyCharacteristic &c,
                               const QByteArray &value)
{
        b_a_mesu=value;
        new_val();
       // qWarning() << "mesu mesu" << value ;
}
void device::serv_mode_read(const QLowEnergyCharacteristic &c,
                               const QByteArray &value)
{
    b_a_mode=value;
    qWarning() << "mode mode" << value;
}

void device::serviceError(QLowEnergyService::ServiceError e)
{
    qWarning() << 'X' << e;
}
//! read write handling
//!
void device::write_pass_char(QByteArray command)
{
    QByteArray test;
    quint64 rand;
    quint64 sav_rand;
    uint32_t v[2],v2[2];
    rand =QRandomGenerator::global()->generate64();
    v[0]= rand;
    v[1]=rand >> 32;
    v2[0]=v[0];
    v2[1]=v[1];
    sav_rand=rand;
    //rand=0;
    test.append(v[0]);
    test.append(v[0]>>8);
    test.append(v[0]>>16);
    test.append(v[0]>>24);
    test.append(v[1]);
    test.append(v[1]>>8);
    test.append(v[1]>>16);
    test.append(v[1]>>24);

    encrypt(v2,key);
    test.append(v2[0]);
    test.append(v2[0]>>8);
    test.append(v2[0]>>16);
    test.append(v2[0]>>24);
    test.append(v2[1]);
    test.append(v2[1]>> 8);
    test.append(v2[1] >> 16);
    test.append(v2[1]>>24);
   // test[0]=0x00; test[1]=0x00; test[2]=0x00; test[3]=0x00; test[4]=0x00; test[5]=0x00; test[6]=0x00; test[7]=0x00;
   // test[8]=0x7F; test[9]=0x63; test[10]=0x60; test[11]=0x72; test[12]=0x88; test[13]=0x9F; test[14]=0x7E; test[15]=0x5E;
    //test=command;

    m_serv_pass->writeCharacteristic(m_serv_pass->characteristics().first(),test,QLowEnergyService::WriteWithResponse);
//    qWarning() << rand << " "<< v[0] << " " << v[1] << " " <<quint64((quint64)v[0]+((quint64)v[1]<<32));
//    qWarning() << ((uint32_t)test[0]+((uint32_t)test[1]<<8)+((uint32_t)test[2]<<16)+((uint32_t)test[3]<<24));
//    qWarning() << ((uint32_t)test[4]+((uint32_t)test[5]<<8)+((uint32_t)test[6]<<16)+((uint32_t)test[7]<<24));
//    qWarning() << test.toHex();
//    qWarning() << v2[0] << " " << v2[1] << " "<< quint64((quint64)v2[0]+((quint64)v2[1]<<32));

}
void device::write_mode_char(QByteArray command)
{
    if(dev_connected)
        m_serv_mode->writeCharacteristic(m_serv_mode->characteristics().first(),command,QLowEnergyService::WriteWithResponse);
}

void device::read_pass_char()
{
    QByteArray test;
    m_serv_pass->readCharacteristic(m_serv_pass->characteristics().first());
}

void device::read_mode_char()
{
    QByteArray test;
    if(dev_connected)
    {
         m_serv_mode->readCharacteristic(m_serv_mode->characteristics().first());
    }
}

void device::read_mesu_char()
{
    if(dev_connected)
    {
        if (m_serv_mesu->characteristics().isEmpty())
        {
        }
        else
        {
         m_serv_mesu->readCharacteristic(m_serv_mesu->characteristics().first());
        }
    }

//    QLowEnergyCharacteristic hrChar;// = num2.last();
//    if(m_serv_mesu!=nullptr)
//        hrChar=m_serv_mesu->characteristic(QBluetoothUuid(QString(Car_Mesures_UUID)));
//    else
//        return;

//   if (!hrChar.isValid()) {

//   }

//   const QLowEnergyDescriptor m_notificationDesc = hrChar.descriptor(
//               QBluetoothUuid::ClientCharacteristicConfiguration);
//   if (m_notificationDesc.isValid()) {
//       m_serv_mesu->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0100"));
//       //m_start = QDateTime::currentDateTime();
//     //  dev_connected=true;
//   }
}

void device::encrypt (uint32_t v[2], uint32_t k[4]) {
    uint32_t v0=v[0], v1=v[1], sum=0, i;           /* set up */
    uint32_t delta=0x9E3779B9;                     /* a key schedule constant */
    uint32_t k0=k[0], k1=k[1], k2=k[2], k3=k[3];   /* cache key */
    for (i=0; i<32; i++) {                         /* basic cycle start */
        sum += delta;
        v0 += ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
        v1 += ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);
    }                                              /* end cycle */
    v[0]=v0; v[1]=v1;
}

void device::decrypt (uint32_t v[2], uint32_t k[4]) {
    uint32_t v0=v[0], v1=v[1], sum=0xC6EF3720, i;  /* set up; sum is 32*delta */
    uint32_t delta=0x9E3779B9;                     /* a key schedule constant */
    uint32_t k0=k[0], k1=k[1], k2=k[2], k3=k[3];   /* cache key */
    for (i=0; i<32; i++) {                         /* basic cycle start */
        v1 -= ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);
        v0 -= ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
        sum -= delta;
    }                                              /* end cycle */
    v[0]=v0; v[1]=v1;
}
