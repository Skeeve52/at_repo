#ifndef _TPROTOCOL_H_
#define _TPROTOCOL_H_

#include <stdint.h>
//#include <stdio.h>
#include <stdbool.h>
#include <limits.h>


typedef enum 
{
    PRS_INIT = 0,
    PRS_PREAMB,       // size 4b
    PRS_PKTSIZE,      // size 1b
    PRS_SRCADDR,      // size 1b
    PRS_DESTADDR,     // size 1b
    PRS_DATA,         // size PCKT_SIZE - SRC_ADDR - DEST_ADDR
    PRS_CHECKSUM,     // size 2b
    PRS_RCVOK,        // Final state - packet succsessfully received
    PRS_TIMEOUT,      // Final state - timeout occurred
} 
prState_t;


typedef enum 
{
    PR_NOERROR = 0, // No error, waiting next byte
    PR_RCVOK,       // Packet succsessfully received, FSM state set to PRS_RCVOK
    PR_FAIL,    // See FSM state for detail:
                //  PRS_DESTADDR    - Destination address does not match
                //  PRS_CHECKSUM    - Invalid checksum
                //  PRS_TIMEOUT     - Did not received full packet within time frame
}
prResult_t;

typedef struct _prListener prListener_t;

typedef prResult_t  prGetByte_cb( prListener_t *listener, uint8_t byte );
typedef void        prTimer_cb( prListener_t *listener );



// the protocol receives the data stream from the UART by byte from ISR,
// the byte is transmitted to the "state machine". State is analyzed 
// by the calling code and contains RCV_OK in case of success,
// otherwise its current state or error code.
// Any way, the calling code either reinitializes the receiver itself,
// or it initializes automatically before receiving the next byte.

struct _prListener
{
    prState_t     state;
    uint16_t      pos;      // Current FSM stream position
    uint16_t      timer;    // Time left
    
    uint8_t       lstAddr;
    uint8_t       remAddr;
    
    uint8_t       pktDataSize;
    uint8_t       pktData [ UCHAR_MAX + 1 ];
	uint8_t		  pktFrame[ UCHAR_MAX + 7 ];
    
    prGetByte_cb  *lstGetByte;
    
    uint16_t      rcvTimeout;
    prTimer_cb    *lstTimer;
    
    bool          autoInit; // Auto initiates listener in case of dropped packet
};

void        prInit( prListener_t* listener, uint8_t localAddr, uint16_t timeout, bool bAuto );
prResult_t  prGetByte( prListener_t* listener, uint8_t byte );
void        prTimer( prListener_t* listener );
uint16_t    prChecksum( uint8_t* buffer, uint8_t size );

#include <intrinsics.h>


#endif