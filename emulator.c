//
//  emulator.c
//  Emulator
//
//  Created by Dariusz Adamczyk on 16/08/2020.
//  Copyright © 2020 Dariusz Adamczyk. All rights reserved.
//

#include "emulator.h"

// An array containing the relay structures
struct relay relays_array[NUM_OF_USED_RELAYS];

// Callback pointer
void (*emulator_callback_ptr)(unsigned int*);

// Variable containing the relay index
unsigned int relays_number;

// Create and initialize relay objects
void emulatorInit(unsigned int num_of_relays)
{
    struct relay relay_0;
    struct relay relay_1;
    
    relay_0.relay_num = RELAY_0;
    relay_0.state = OFF;
    
    relay_1.relay_num = RELAY_1;
    relay_1.state = OFF;
    
    relays_array[RELAY_0] = relay_0;
    relays_array[RELAY_1] = relay_1;
    
    setRelaysNumber(num_of_relays);
    
}

// Assign callback function to the pointer
void registerEmulatorCallback(void (*function_ptr)(unsigned int * data))
{
    emulator_callback_ptr = function_ptr;
}

//Enforce the received instructions. In case of an error, it will return an error code.
err_code_t processIncommingPacket(unsigned int * incoming_packet)
{
    unsigned int tx_buffer[TX_BUFFER_LEN];
    static unsigned int last_sequence_num, curr_tx_sequence_num;
    unsigned int curr_sequence_num = incoming_packet[SEQUENCE_IDX];
    
    // Clean tx buffer
    memset(tx_buffer, 0x00, TX_BUFFER_LEN);
    
    // Check if the incoming command with ACK confirmation
    if(incoming_packet[SIGNAL_IDX] == ACK)
    {
        // Check if the current ack count is incremented by one and not equal to zero
        if((curr_sequence_num == ( ++curr_tx_sequence_num)) && curr_sequence_num != 0)
        {
            printf("Received ACK\r\n");
            
            return OK;
        }
        else
            return SEQ_ERROR;
    }
    
    // If it is not ACK, compare if the value has increased by one and it is not equal to zero. Write the counter value
    // as the last known value for the next command
    else if((curr_sequence_num == ( ++last_sequence_num)) && curr_sequence_num != 0)
    {
        last_sequence_num = curr_sequence_num;
    }
    
    // If there is a sequence error, re-synchronize counter by comparing to the last known value reduced by one
    else
    {
        curr_sequence_num = -- last_sequence_num;
        
        return SEQ_ERROR;
    }
    
    // Send ACK confirmation with sequence counter increased by one
    sendAck(tx_buffer, ++curr_sequence_num);
    
    // Match the signal value in the packet to known commands
    switch(incoming_packet[SIGNAL_IDX])
    {
        case RELAY_SIG:
        {
            printf("Received Relay Signal\r\n");
            
            // Check if the declared number of settings is within the range.
            // The comparison is based on the number of relays, previously declared during initialization.
            // If not, return "RANGE ERROR".
            if(isInRange(incoming_packet[SETTINGS_NUM_IDX], 1, (getRelaysNumber())) == OK)
            {
                // Do the loop as many times as declared settings.
                // If more than once, add the offset value for the subsequent setting values
                for(int i = 0, j = 0; i < incoming_packet[SETTINGS_NUM_IDX]; i++, j+= IDX_OFFSET)
                {
                    // Check if the declared relay exists by checking the range.
                    // The range is compared to the maximum relay index.
                    // Relay index is the number of relays reduced by one (indexes counted from zero)
                    // Additionally, we check if the setting range is between ON and OFF.
                    // If it is out of range, return "RANGE ERROR".
                    if((isInRange(incoming_packet[RELAY_TO_SET_IDX + j], 0, getRelayMaxIndex()) == OK)
                       && ((isInRange(incoming_packet[RELAY_STATE_IDX + j], OFF, ON) == OK)))
                    {
                        // We assign a value to the appropriate relay in the object array.
                        relays_array[incoming_packet[RELAY_TO_SET_IDX + j]].state = incoming_packet[RELAY_STATE_IDX + j];
                        
                        // We perform the operation on a mock output
                        performOutputAction(relays_array[incoming_packet[RELAY_TO_SET_IDX + j]]);
                    }
                    else
                        return RANGE_ERROR;
                }
            }
            else
                return RANGE_ERROR;
            
            return OK;
        }
            
        case GET_STATE_SIG:
        {
            printf("Received Get State\r\n");
            
            unsigned int relays_bitmap = 0;
            
            // Assign to the variable the values ​​of the current state of the relays, through a bit shift.
            for(int i = 0; i < getRelaysNumber(); i++)
                relays_bitmap |= (getRelayCurrentState(relays_array[i]) << i);
            
            // Fill the transmitt buffer with the remaining parameters
            tx_buffer[SIGNAL_IDX] = REL_STATE_SIG;
            tx_buffer[SEQUENCE_IDX] = ++ curr_tx_sequence_num;
            tx_buffer[BITMAP_IDX] = relays_bitmap;
            
            // Call the callback function to send the data
            emulator_callback_ptr(tx_buffer);
            
            return OK;
        }
        case QUIT_SIG:
        {
            printf("Received Quit\r\n");
            
            return APP_CLOSE;
        }
        default:
        {
           return UNKNOWN_CODE;
        }
    }
    
}

// The function is used to send the ACK confirmation with current sequence number to the client.
void sendAck(unsigned int * outgoing_packet, unsigned int seq_num)
{
    outgoing_packet[SIGNAL_IDX] = ACK;
    outgoing_packet[SEQUENCE_IDX] = seq_num;
    
    emulator_callback_ptr(outgoing_packet);
}

// Return current relay state - ON or OFF
unsigned int getRelayCurrentState(struct relay obj)
{
    return obj.state;
}

// Check if the typed port number is valid
err_code_t portIsValid(char * port)
{
    char temp_buff [PORT_BUFF_LEN];
    strcpy(temp_buff, port);
    
    unsigned long len = strlen(temp_buff);
    unsigned int val = atoi(temp_buff);
    
    for(int i = 0; i < len; i++)
    {
        if(!isdigit(temp_buff[i]))
        {
            printf("Syntax error\r\n");
            return SYNTAX_ERROR;
        }
    }
    
    if(val > PORT_VALUE_LIMIT || len > PORT_INPUT_MAX_LEN)
    {
        printf("Range error\r\n");
        return RANGE_ERROR;
    }
    
    return OK;
}

// A simple function for testing and checking the ranges of received data against the declared ones
unsigned int isInRange(unsigned int variable, unsigned int min_val, unsigned int max_val)
{
    if(variable >= min_val && variable <= max_val)
        return OK;
    else
        return RANGE_ERROR;
}

// Set number of relays
void setRelaysNumber(unsigned int relays_num)
{
    relays_number = relays_num;
}

// Get number of relays
unsigned int getRelaysNumber(void)
{
    return relays_number;
}

// Get relays max index
unsigned int getRelayMaxIndex(void)
{
    return getRelaysNumber() - 1;
}

// Output mockup - 0: relay OFF, 1: relay ON
void performOutputAction(struct relay obj)
{
    obj.state? printf("RELAY %d: Open\r\n", obj.relay_num): printf("RELAY %d: Close\r\n", obj.relay_num);
}

