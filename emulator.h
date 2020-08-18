//
//  emulator.h
//  Emulator
//
//  Created by Dariusz Adamczyk on 16/08/2020.
//  Copyright © 2020 Dariusz Adamczyk. All rights reserved.
//

#ifndef emulator_h
#define emulator_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define RELAY_0             0
#define RELAY_1             1

#define NUM_OF_USED_RELAYS  2

#define PORT_VALUE_LIMIT    65535
#define PORT_INPUT_MAX_LEN  5

#define SIGNAL_IDX          0
#define SEQUENCE_IDX        1
#define SETTINGS_NUM_IDX    2
#define IDX_OFFSET          2
#define BITMAP_IDX          2
#define RELAY_TO_SET_IDX    3
#define RELAY_STATE_IDX     4


#define TX_BUFFER_LEN       16
#define PORT_BUFF_LEN       10

typedef unsigned int err_code_t;

enum return_codes
{
    OK = 0,
    APP_CLOSE,
    UNKNOWN_CODE,
    RANGE_ERROR,
    SYNTAX_ERROR,
    SEQ_ERROR,
};

enum operations
{
    OFF = 0,
    ON,
};

enum signal_codes
{
    ACK             = 0x11, // ACK signal
    RELAY_SIG       = 0x21, // Relay signal
    GET_STATE_SIG   = 0x31, // Get state signal
    REL_STATE_SIG   = 0x41, // Relay state signal
    QUIT_SIG        = 0x51, // Quit signal
};

struct relay
{
    unsigned int relay_num;
    unsigned int state;
};

void emulatorInit(unsigned int num_of_relays);
void registerEmulatorCallback(void (*function_ptr)(unsigned int * data));
err_code_t processIncommingPacket(unsigned int * incoming_packet);
void sendAck(unsigned int * outgoing_packet, unsigned int seq_num);
unsigned int getRelayCurrentState(struct relay obj);
err_code_t portIsValid(char * port);
unsigned int isInRange(unsigned int variable, unsigned int min_val, unsigned int max_val);
void setRelaysNumber(unsigned int relays_num);
unsigned int getRelaysNumber(void);
unsigned int getRelayMaxIndex(void);
void performOutputAction(struct relay obj);


#endif /* emulator_h */
