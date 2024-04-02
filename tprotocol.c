#include "tprotocol.h"

const uint8_t prPreamb[ PREAMB_SIZE ] = { 0x01, 0x02, 0x03, 0x04 };

void        prInit( prListener_t* listener, uint8_t localAddr, uint16_t timeout, bool bAuto )
{
    __disable_interrupt();
    listener->lstAddr = localAddr;
    listener->autoInit = bAuto;
    listener->rcvTimeout = timeout;
    listener->timer = 0;
    listener->pos = 0;
    listener->crc = 0;
    listener->state = PRS_INIT;
    listener->lstGetByte = prGetByte;
    listener->lstTimer = prTimer;
    __enable_interrupt();
}

prResult_t   prGetByte( prListener_t* listener, uint8_t byte )
{
    int16_t     n;
    prResult_t  res = PR_NOERROR;
    
    __disable_interrupt();
    
    if ( PRS_INIT == listener->state ) {
        listener->state++;
    }
    else if ( PRS_TIMEOUT == listener->state )
    {
        res = PR_FAIL;
    }
        
    switch ( listener->state )
    {
      case PRS_PREAMB:
        if ( PREAMB_SIZE >= listener->pos ) {
            listener->state++;
        }
        else if ( prPreamb[ listener->pos ] != byte ) {
            res = PR_FAIL;
        }
        break;
        
      case PRS_DATASIZE:
        listener->dataSize = byte;
        listener->state++;
        break;
        
      case PRS_SRCADDR:
        listener->remAddr = byte;
        listener->state++;
        break;
        
      case PRS_DESTADDR:
        if ( listener->lstAddr != byte ) {
            res = PR_FAIL;
        }
        else {
            listener->state++;
        }
        break;
        
      case PRS_DATA:
        n = listener->pos - DATA_OFFSET;
        if ( n >= ( listener->dataSize - 1 ) ) 
        {
            listener->state++;
        }
        break;
        
      case PRS_CHECKSUM:
        if ( listener->crc != byte ) 
        {
            res = PR_FAIL;
        }
        res = PR_RCVOK;
        if ( listener->autoInit ) {
            prInit( listener, listener->lstAddr, listener->rcvTimeout, true );
        break;
    }
    
    switch ( res )
    {
      case PR_NOERROR:
        listener->pktFrame[ listener->pos ] = byte;
        listener->crc ^= byte;
        listener->pos++;
        break;

      case PR_FAIL:
        if ( listener->autoInit ) {
            prInit( listener, listener->lstAddr, listener->rcvTimeout, true );
            // Auto re-init. Last received byte will be discarded 
        }
        break;

      case PR_RCVOK:
        // do nothing
        break;
    }
    
    __enable_interrupt();
    
    return res;
}

void        prTimer( prListener_t* listener )
{
    __disable_interrupt();
    if ( ( listener->state > PRS_INIT ) 
        && ( listener->state <= PRS_CHECKSUM ) )  
    {
        listener->timer++;
        if ( listener->timer > listener->rcvTimeout ) {
            listener->state = PRS_TIMEOUT;
        }
    }
    __enable_interrupt();
}
