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

    static const uint16_t permanent_dtcs[] = {
        0x0420,  // P0420: Catalyst efficiency below threshold, Bank 1
        0x0301,  // P0301: Cylinder 1 misfire
        0x0171   // P0171: System too lean, Bank 1
    };

    can_MsgTx.buf[1] = MODEA_RESPONSE;
    if (ecu.permanent_dtc) {
        can_MsgTx.buf[0] = 1 + sizeof(permanent_dtcs);
        for (uint8_t i = 0; i < sizeof(permanent_dtcs) / sizeof(permanent_dtcs[0]); i++) {
            can_MsgTx.buf[2 + i * 2] = permanent_dtcs[i] >> 8;
            can_MsgTx.buf[3 + i * 2] = permanent_dtcs[i] & 0xFF;
        }
    } else {
        can_MsgTx.buf[0] = 0x01;
    }

    can1.write(can_MsgTx);
    return true;
}

static ModeRegistrar mode_0A_registrar(MODEA, handle_mode_0A, "Request Permanent Emissions DTCs");
