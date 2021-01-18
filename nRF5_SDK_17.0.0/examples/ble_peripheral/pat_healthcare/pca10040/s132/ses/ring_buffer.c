#include "ring_buffer.h"

#define MODULE_LOG_ENABLE 1
#define LOG_HEAD "RING_BUFFER"
#include "log.h"

/***
 * Ring Buffer Initialization 
 */
RingBuffer* ring_buffer_init(int len){
  return initQueue(len);
}


void ring_buffer_add(RingBuffer* rbuffer, QueueElement e){
  if(rbuffer->capacity == rbuffer->size){
    QueueElement temp = front(rbuffer);
    dequeue(rbuffer);
    free(temp);           /* Free memory */
    enqueue(rbuffer, e);
  }
  else{
    enqueue(rbuffer, e);
  }
}

