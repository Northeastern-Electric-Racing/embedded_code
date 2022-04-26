#include "mpu.h"

MPU mpu;

MPU::MPU()
{
    pedals = PEDALS(&motorController, &bms);
    driverio = DRIVERIO(&motorController, &bms);
    gpio = GPIO(&motorController, &bms);

    ioRead_wait.cancelTimer();
    canTest_wait.cancelTimer();
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);
}


MPU::~MPU(){}


void MPU::driverioProcess()
{
    if(!ioRead_wait.isTimerExpired()){return;}
    
    // Serial.println("DriverIO process...");
    driverio.handleSSButton();
    driverio.handleSSLED();
    driverio.handleReverseSwitch();
    driverio.handleErrorLights();
    ioRead_wait.startTimer(100);
}


void MPU::pedalsProcess()
{
    // Serial.println("Pedals process...");
    pedals.readBrake();
    isShutdown = pedals.readAccel();
}


void MPU::gpioProcess()
{
    gpio.handleMCHVFault();
    gpio.handlePump();
    gpio.handleRadiatorFan();
    isShutdown = isCANLineOK();
    if(bms.getChargeMode())
    {
        Serial.println("CHARGING!!!!!!!!!!!!!");
    }
}


void MPU::sendMCMsg()
{
    motorController.writeMCState();
}


void MPU::setBMSAvgTemp(uint8_t p_avgTemp)
{
    bms.setAvgTemp(p_avgTemp);
}


void MPU::setBMSSoC(uint8_t p_soc)
{
    bms.setSoC(p_soc);
}


bool MPU::isCANLineOK()
{
    return !canTest_wait.isTimerExpired();
}


void MPU::CANLineVerified()
{
    canTest_wait.startTimer(500);
}


void MPU::checkShutdownStatus()
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


void MPU::shutOffCar()
{
    writeFaultLatch(TRIGGER_FAULT);
    motorController.emergencyShutdown();
    delay(5000);
    writeFaultLatch(FAULT_OK);
}


void MPU::writeFaultLatch(bool status)
{
    digitalWrite(RELAY_PIN, !status);
}


void MPU::setCurrentLimit(uint16_t currentLimit)
{
    bms.setCurrentLimit(currentLimit);
}


void MPU::bmsCurrentProcess(int16_t currentDraw)
{
    bms.setCurrentDraw(currentDraw);

    if(bms.isCurrentPastLimit())
    {
        bms.setBoosting();
    }
}


void MPU::setMotorSpeed(int16_t motorSpeed)
{
    motorController.setMotorSpeed(motorSpeed);
}


void MPU::enableBMSChargingMode()
{
    bms.enableChargingMode();
}

