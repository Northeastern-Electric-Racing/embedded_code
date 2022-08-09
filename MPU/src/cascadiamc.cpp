#include "cascadiamc.h"


CASCADIAMC::CASCADIAMC(){}


CASCADIAMC::~CASCADIAMC(){}


void CASCADIAMC::disableMCLockout()
{
    sendMessage(CANMSG_ACCELERATIONCTRLINFO, 8, mcOff); // release lockout / OFF
    isMCLocked = false;
}


void CASCADIAMC::writeMCState()
{
    if(isMCLocked)
    {
        disableMCLockout();
    }

#ifdef DEBUG
    Serial.println("CAN MC Message:");
    for(uint8_t i=0; i<8; i++)
    {
        Serial.println(mcMsg.canMsg[i]);
    }
    Serial.println("");
#endif

    sendMessage(CANMSG_ACCELERATIONCTRLINFO, 8, mcMsg.canMsg);
}


void CASCADIAMC::toggleDirection()
{
    if (mcMsg.config.isOn) {
        togglePower();
    }
    mcMsg.config.isForward = !mcMsg.config.isForward;
}


void CASCADIAMC::togglePower()
{
    mcMsg.config.isOn = !mcMsg.config.isOn;
    isMCLocked = true;
}


bool CASCADIAMC::getIsOn()
{
    return mcMsg.config.isOn;
}


bool CASCADIAMC::getDirection()
{
    return mcMsg.config.isForward;
}


void CASCADIAMC::changeTorque(uint16_t p_accelTorque)
{
    mcMsg.config.accelTorque = p_accelTorque;
}


uint16_t CASCADIAMC::getTorque()
{
    return mcMsg.config.accelTorque;
}


void CASCADIAMC::clearFault()
{
    for(int tries = 0; tries < FAULT_CLEAR_ATTEMPTS; tries++)
    {
        sendMessage(CANMSG_MC_SETPARAMETER, 8, FAULT_CLEAR);
    }
    isFaulted = false;
}


void CASCADIAMC::raiseFault()
{
    if (isFaulted == false) {
        Serial.println("MC Faulted");
    }
    isFaulted = true;
}


bool CASCADIAMC::checkFault()
{
    return isFaulted;
}


void CASCADIAMC::emergencyShutdown()
{
    mcMsg.config.isOn = false;
    mcMsg.config.accelTorque = 0;
    writeMCState();
}


void CASCADIAMC::setMotorSpeed(int16_t p_motorSpeed)
{
    motorSpeed = p_motorSpeed;
}

bool CASCADIAMC::isMotorMoving()
{
    return abs(motorSpeed) > 0;
}


bool CASCADIAMC::shouldMotorBeSpinning()
{
    return mcMsg.config.accelTorque > NOT_SPINNING_TORQUE_LIMIT;
}


int16_t CASCADIAMC::getMotorSpeed()
{
    return motorSpeed;
}

void CASCADIAMC::setRadiatorTemp(int16_t temp)
{
    radiatorTemp = temp;
}

int16_t CASCADIAMC::getRadiatorTemp()
{
    return radiatorTemp;
}