#define LOG_HEAD "TQ"
#include "log.h"

#include <string.h>
#include "app_error.h"

#include "transfer_queue.h"

#define  MAX_PACKET_LEN   256

/**
 * @brief Initialize and setup transfer queue
 * @param npriority -> Number of priority levels 0 - N, where 0 is highest priority
 *        qsize     -> Size of each Queue
 */
transfer_queues_t* transfer_queue_init(transfer_function_t tf, uint8_t npriority, uint16_t qsize){
  if(npriority < 1 || (!tf)) APP_ERROR_CHECK(1);

  transfer_queues_t* tq  = (transfer_queues_t*) malloc(sizeof(transfer_queues_t));
  tq->queues             = (Queue**) malloc(sizeof(Queue*) * npriority);
  tq->npriorities        = npriority;
  tq->transfer_function  = tf;

  for(uint8_t i=0; i<npriority; i++){
    tq->queues[i] = initQueue(qsize);
  }

  LOGD("Init complete.");
  return tq;
}

static tq_ret_t process_queue(transfer_queues_t* tq, Queue* q){
  transfer_packet_t* packet = front(q);

  /* If transfer is not successful, do not dequeue */
  //if( (*(tq->transfer_function)) (packet->buf, packet->len)){ return TQ_ERROR;}
  if( ((tq->transfer_function)) (packet->buf, packet->len)){ return TQ_ERROR;}

  dequeue(q); free(packet);   
  return TQ_SUCCESS;
}

/**
 * @brief Called periodically to run transfer loop
 * @param transfer_queue tq
 */
void transfer_queue_loop(transfer_queues_t* tq){
  for(uint16_t i=0; i<tq->npriorities; i++){  
    if(tq->queues[i]->size){
      tq_ret_t ans = process_queue(tq, tq->queues[i]);
      break;
    }
  }
}

/**
 * @brief Clear all packets in transfer queues
 * @param transfer_queue tq
 */
void transfer_queue_clear(transfer_queues_t* tq){
  for(uint8_t i=0; i<tq->npriorities; i++){
    while(tq->queues[i]->size){
      transfer_packet_t* packet = front(tq->queues[i]);
      dequeue(tq->queues[i]);
      free(packet);
    }
  }
}

static transfer_packet_t* packet_generate(uint8_t* buf, uint8_t len){
  if(!buf || len >= MAX_PACKET_LEN || len < 1) return NULL;

  transfer_packet_t* packet = malloc(sizeof(transfer_packet_t));
  memcpy(packet->buf, buf, len);
  packet->len = len;
  return packet;
}

/**
 * @brief Add buffer to transfer queue
 * @param transfer_queue tq, buffer, lenght and packet priority
 */
tq_ret_t transfer_queue_add(transfer_queues_t* tq, uint8_t* buf, uint8_t len, uint8_t priority){
  if(tq->queues[priority]->size >= tq->queues[priority]->capacity) return TQ_FULL;
  transfer_packet_t* packet = packet_generate(buf, len);
  if(packet == NULL) return TQ_ERROR;

  if(tq->npriorities <= priority) return TQ_ERROR;
  enqueue(tq->queues[priority], packet);
  return TQ_SUCCESS;
}