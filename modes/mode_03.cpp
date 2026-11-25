/*
 * OBD-II Mode 03 - Request Emissions-Related Diagnostic Trouble Codes
 *
 * This mode provides access to stored emissions-related DTCs (Diagnostic Trouble Codes)
 * that have triggered the Malfunction Indicator Lamp (MIL/Check Engine Light).
 *
 * MODE 03 OVERVIEW:
 * - Returns confirmed/matured DTCs only (not pending codes)
 * - DTCs are emissions-related powertrain ("P") codes
 * - Response contains each DTC as two bytes in standardized format
 *
 * DTC FORMAT:
 * Byte 1 (High byte):
 *   Bits 7-6: DTC system (00=Powertrain, 01=Chassis, 10=Body, 11=Network)
 *   Bits 5-4: First digit of code
 *   Bits 3-0: Second digit of code
 * Byte 2 (Low byte):
 *   Bits 7-4: Third digit of code
 *   Bits 3-0: Fourth digit of code
 *
 * EXAMPLE:
 *   P0301 = 0x03 0x01 (Cylinder 1 Misfire)
 *
 * This is an emissions monitoring function required by EPA/CARB for OBD-II compliance.
 */

#include "../mode_registry.h"
#include <FlexCAN_T4.h>

// External CAN bus instance
extern FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;

// External ECU data structures
extern ecu_t ecu;
/*
 * Mode 03 Handler - Request Emissions-Related Trouble Codes
 *
 * Returns stored DTCs that have triggered the MIL (Check Engine Light).
 * These are confirmed emissions faults requiring attention.
 *
 * Response format:
 * - No DTCs: buf[0]=0x01, buf[1]=0x43
 * - With DTCs: buf[0]=length, buf[1]=0x43, followed by DTC byte pairs
 */
bool handle_mode_03(CAN_message_t& can_MsgRx, CAN_message_t& can_MsgTx, ecu_simClass* ecu_sim) {
    // Check if this is a Mode 03 request
    if (can_MsgRx.buf[1] != MODE3) {
        return false;  // Not our mode, let other handlers try
    }

    can_MsgTx.id = PID_REPLY;
    can_MsgTx.len = 8;
    for (uint8_t i = 0; i < 8; i++) {
        can_MsgTx.buf[i] = 0x00;
    }

    static const uint16_t confirmed_dtcs[] = {
        0x0301,  // P0301: Cylinder 1 misfire
        0x0171,  // P0171: System too lean, Bank 1
        0x0420   // P0420: Catalyst efficiency below threshold, Bank 1
    };

    can_MsgTx.buf[1] = MODE3_RESPONSE;
    if (ecu.dtc) {
        can_MsgTx.buf[0] = 1 + sizeof(confirmed_dtcs);
        for (uint8_t i = 0; i < sizeof(confirmed_dtcs) / sizeof(confirmed_dtcs[0]); i++) {
            can_MsgTx.buf[2 + i * 2] = confirmed_dtcs[i] >> 8;
            can_MsgTx.buf[3 + i * 2] = confirmed_dtcs[i] & 0xFF;
        }
    } else {
        can_MsgTx.buf[0] = 0x01;
    }

    // Send response on standard OBD-II reply channel
    can_MsgTx.id = PID_REPLY;  // 0x7E8 - Engine ECU response
    can_MsgTx.len = 8;
    can1.write(can_MsgTx);

    return true;  // Mode 03 request handled successfully
}

// Register Mode 03 handler with the mode registry
// This enables automatic dispatch when Mode 03 requests are received
static ModeRegistrar mode_03_registrar(MODE3, handle_mode_03, "Request Emissions DTCs");
