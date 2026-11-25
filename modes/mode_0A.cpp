/*
 * OBD-II Mode 0A - Request Permanent Emissions-Related DTCs
 */

#include "../mode_registry.h"
#include <FlexCAN_T4.h>

extern FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;
extern ecu_t ecu;

bool handle_mode_0A(CAN_message_t& can_MsgRx, CAN_message_t& can_MsgTx, ecu_simClass* ecu_sim) {
    if (can_MsgRx.buf[1] != MODEA) {
        return false;
    }

    can_MsgTx.id = PID_REPLY;
    can_MsgTx.len = 8;
    for (uint8_t i = 0; i < 8; i++) {
        can_MsgTx.buf[i] = 0x00;
    }

    can_MsgTx.buf[1] = MODEA_RESPONSE;
    if (ecu.dtc) {
        can_MsgTx.buf[0] = 0x05;
        can_MsgTx.buf[2] = 0x01;
        can_MsgTx.buf[3] = 0x00;
        can_MsgTx.buf[4] = 0x02;
        can_MsgTx.buf[5] = 0x00;
    } else {
        can_MsgTx.buf[0] = 0x01;
        can_MsgTx.buf[2] = 0x00;
    }

    can1.write(can_MsgTx);
    return true;
}

static ModeRegistrar mode_0A_registrar(MODEA, handle_mode_0A, "Request Permanent Emissions DTCs");
