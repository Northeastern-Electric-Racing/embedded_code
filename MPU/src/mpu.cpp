#include "mpu.h"

DRIVERIO driverio;
GPIO gpio;
PEDALS pedals;
CASCADIAMC motorController;
ORIONBMS bms;
WDT_T4<WDT1> wdt;
bool isShutdown = false;
bool ssReady = false;
Timer canTest_wait;
Timer boosting_debounce;
Timer spinningCheck_wait;


void driverioProcess()
{
    // Serial.println("DriverIO process...");
    driverio.handleSSButton();
    driverio.handleSSLED();
    driverio.handleReverseSwitch();
    driverio.handleErrorLights();
}


void pedalsProcess()
{
    //isShutdown = isShutdown ? true : !verifyMotorSpinning();
    // TEMP TODO: reimplement
    // if(!verifyMotorSpinning())
    // {Serial.println("MOTOR NOT SPINNING");}
    // Serial.println("Pedals process...");
    pedals.readBrake();
    bool accelFault = pedals.readAccel();
    isShutdown = isShutdown ? true : accelFault;
    if(accelFault){Serial.println("ACCEL FAULT");}
}


void gpioProcess()
{
    bool faultReset = gpio.handleMCHVFault();
    gpio.handlePump();
    gpio.handleRadiatorFan();

    isShutdown = isShutdown ? true : !isCANLineOK();
    if(!isCANLineOK()){Serial.println("CAN FAULT");}
}

bool isCANLineOK()
{
    if(canTest_wait.isTimerExpired() && (digitalRead(SS_READY_SEN) == HIGH))
    {
        Serial.println("CAN FUCKED@$#^!$^@$#&");
        return false;
    }
    return true;
}


void setCANLineOK()
{
    canTest_wait.startTimer(1000);
}


void checkShutdownStatus()
{
    if(isShutdown)
    {
        shutOffCar();
    }
    else
    {
        writeFaultLatch(FAULT_OK);
    }
}


void shutOffCar()
{
    writeFaultLatch(TRIGGER_FAULT);
    motorController.emergencyShutdown();
    Serial.println("Shutting off Car!!!!!");
    while(1){}
    writeFaultLatch(FAULT_OK);
}


void writeFaultLatch(bool status)
{
    digitalWrite(RELAY_PIN, status);
}


bool verifyMotorSpinning()
{
    if(motorController.shouldMotorBeSpinning() && motorController.getIsOn())
    {
        if(motorController.isMotorMoving())
        {
            spinningCheck_wait.startTimer(5000);
        }
        if(spinningCheck_wait.isTimerExpired()) {
            return false;
        }
    }
    return true;
}