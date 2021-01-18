#ifndef TRANSFER_QUEUE_H
#define TRANSFER_QUEUE_H

#include "queue.h"
#include "stdint.h"

typedef enum{
  TQ_SUCCESS,
  TQ_FULL,
  TQ_ERROR
}tq_ret_t;

typedef struct{
  uint16_t len;
  uint8_t  buf[256];
}transfer_packet_t;

typedef tq_ret_t (*transfer_function_t) (uint8_t* buf, uint16_t len);

typedef struct{
  Queue** queues;
  uint8_t npriorities;
  transfer_function_t transfer_function;
}transfer_queues_t;

/* Functions */
transfer_queues_t* transfer_queue_init(transfer_function_t tf, uint8_t npriority, uint16_t qsize);

void transfer_queue_loop(transfer_queues_t* tq);

void transfer_queue_clear(transfer_queues_t* tq);

tq_ret_t transfer_queue_add(transfer_queues_t* tq, uint8_t* buf, uint8_t len, uint8_t priority);

#endif