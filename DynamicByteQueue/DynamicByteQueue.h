//
//  DynamicByteQueue.h
//  DynamicByteQueue
//
//  Created by Galen Rhodes on 4/7/18.
//  Copyright © 2018 Project Galen. All rights reserved.
//

#ifndef DynamicByteQueue_h
#define DynamicByteQueue_h

#include <stdio.h>
#include <stdint.h>

__BEGIN_DECLS

typedef struct __pg_dynamic_byte_queue_struct {
    uint8_t *queue;
    size_t  head;
    size_t  tail;
    size_t  size;
} PGDynamicByteQueueStruct;

PGDynamicByteQueueStruct *pgDynamicQueueInit(PGDynamicByteQueueStruct *queue, size_t initialSize);

char pgDynamicQueueIsFull(PGDynamicByteQueueStruct *queue);

char pgDynamicQueueIsEmpty(PGDynamicByteQueueStruct *queue);

int pgDynamicQueueDequeue(PGDynamicByteQueueStruct *queue);

int pgDynamicQueuePop(PGDynamicByteQueueStruct *queue);

char pgDynamicQueueRequeue(PGDynamicByteQueueStruct *queue, uint8_t byte);

char pgDynamicQueueEnqueue(PGDynamicByteQueueStruct *queue, uint8_t byte);

char pgDynamicQueueEnqueueAll(PGDynamicByteQueueStruct *queue, uint8_t *buffer, size_t len);

char pgDynamicQueueRequeueAll(PGDynamicByteQueueStruct *queue, uint8_t *buffer, size_t len);

#if __has_extension(blocks)

typedef char (^PGDynamicByteQueueOpBlock)(uint8_t *buffer, size_t size, size_t *head, size_t *tail, char *error, char *eof);

char pgDynamicQueueOperation(PGDynamicByteQueueStruct *queue, PGDynamicByteQueueOpBlock opBlock, char *error, char *eof);

#endif

__END_DECLS

#endif /* DynamicByteQueue_h */
