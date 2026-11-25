/*
 * OBD-II Mode 07 - Request Pending Emissions-Related DTCs
 */

#include "../mode_registry.h"
#include <FlexCAN_T4.h>

extern FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;
extern ecu_t ecu;

bool handle_mode_07(CAN_message_t& can_MsgRx, CAN_message_t& can_MsgTx, ecu_simClass* ecu_sim) {
    if (can_MsgRx.buf[1] != MODE7) {
        return false;
    }

    can_MsgTx.id = PID_REPLY;
    can_MsgTx.len = 8;
    for (uint8_t i = 0; i < 8; i++) {
        can_MsgTx.buf[i] = 0x00;
    }

    static const uint16_t pending_dtcs[] = {
        0x0133,  // P0133: O2 sensor slow response, Bank 1 Sensor 1
        0x0442,  // P0442: Small EVAP-system leak
        0x0113   // P0113: Intake air temperature sensor circuit high
    };

    can_MsgTx.buf[1] = MODE7_RESPONSE;
    if (ecu.dtc) {
        can_MsgTx.buf[0] = 1 + sizeof(pending_dtcs);
        for (uint8_t i = 0; i < sizeof(pending_dtcs) / sizeof(pending_dtcs[0]); i++) {
            can_MsgTx.buf[2 + i * 2] = pending_dtcs[i] >> 8;
            can_MsgTx.buf[3 + i * 2] = pending_dtcs[i] & 0xFF;
        }
    } else {
        can_MsgTx.buf[0] = 0x01;
    }

    can1.write(can_MsgTx);
    return true;
}

static ModeRegistrar mode_07_registrar(MODE7, handle_mode_07, "Request Pending Emissions DTCs");
