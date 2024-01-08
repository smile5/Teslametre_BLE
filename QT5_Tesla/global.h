#ifndef GLOBAL_H
#define GLOBAL_H

//Adresse MAC des alimentations

#define App1_MAC "00:A0:50:E0:B5:10"
#define App2_MAC "00:A0:50:D0:5A:65"
#define App3_MAC "54:65:73:6C:61:33"

// UUID des Services BLE
#define Serv_Mesures_UUID "{a77ffe58-18db-474e-bc47-baed187b52c3}"
#define Serv_Mode_UUID "{7594e831-192c-4521-9332-b99a4e7e29cc}"
#define Serv_Passkey_UUID "{19eda552-ac8d-4b1c-9204-59afff525234}"

// UUID Characteristiques
#define Car_Mesures_UUID "{69358dfb-f3e2-4177-af7b-f062f29043b0}"
#define Car_Mode_UUID "{8dcbb408-e267-4f41-b890-f3c4eec83e6a}"
#define Car_Passkey_UUID "{65bc83b1-fec9-48ed-89cc-7c0f6035e85b}"

// contient le mode de fct
// bit0=1 => auto sinon manu
// bit1= 1 => regul I sinon V
// bit2 = 0 arret, 1 marche
// bit3 = 0 comande normale + diphase,1 triphase
// bits 4-5-6-7 => forme 0= constant, 1= sinus, 2=triangle, 3= dents de scie, 4 = carree

// consigne[2] : 0 amplitude 1 offset

#endif // GLOBAL_H
