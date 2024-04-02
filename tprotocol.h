#ifndef _TPROTOCOL_H_
#define _TPROTOCOL_H_

#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#define PREAMB_SIZE         ( 4 )
#define ADDR_SIZE           ( 1 )
#define SIZE_SIZE           ( 1 )
#define DATASIZE_OFFSET     ( PREAMB_SIZE )
#define SRCADDR_OFFSET      ( DATASIZE_OFFSET + SIZE_SIZE )  // 5
#define DSTADDR_OFFSET      ( SRCADDR_OFFSET + ADDR_SIZE )   // 6
#define DATA_OFFSET         ( DSTADDR_OFFSET + ADDR_SIZE )   // 7
#define CRC_SIZE            ( 1 )

// Packet structure 
//
typedef enum 
{
    PRS_INIT = 0,
    PRS_PREAMB,
    PRS_DATASIZE,
    PRS_SRCADDR,
    PRS_DESTADDR,
    PRS_DATA,
    PRS_CHECKSUM,
    PRS_RCVOK,        // Final state - packet succsessfully received
    PRS_TIMEOUT,      // Final state - timeout occurred
} 
prState_t;

//  FSM step result code
//
typedef enum 
{
    PR_NOERROR = 0, // No error, waiting next byte
    PR_RCVOK,       // Packet succsessfully received
    PR_FAIL,    // 
                //  PRS_DESTADDR    - Destination address does not match
                //  PRS_CHECKSUM    - Invalid checksum
                //  PRS_TIMEOUT     - Did not received full packet within time frame
}
prResult_t;

typedef struct _prListener prListener_t;

typedef prResult_t  prGetByte_cb( prListener_t *listener, uint8_t byte );
typedef void        prTimer_cb( prListener_t *listener );

// the protocol receives the data stream from the UART by byte from ISR
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
    uint8_t       crc;
    
    uint8_t       pktFrame[ UCHAR_MAX + DATA_OFFSET + CRC_SIZE ];

    uint8_t       lstAddr;
    uint8_t       remAddr;
    uint8_t       dataSize;
    
    prGetByte_cb  *lstGetByte;
    
    uint16_t      rcvTimeout;
    prTimer_cb    *lstTimer;
    
    bool          autoInit; // Auto initiates listener 
};

#define     prGetData( lst )      ( &lst->pktFrame[DATA_OFFSET] ) // Get DATA pointer

void        prInit( prListener_t* listener, uint8_t localAddr, uint16_t timeout, bool bAuto );
prResult_t  prGetByte( prListener_t* listener, uint8_t byte );
void        prTimer( prListener_t* listener );

#include <intrinsics.h>


#endif