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

typedef uint8_t PGByte;
typedef PGByte  *PGBytePtr;
typedef char    PGBool;

#define TRUE            ((char)(1))
#define FALSE           ((char)(0))
#define PG_DEFAULT_SIZE ((size_t)(64 * 1024))

typedef struct __pg_dynamic_byte_queue_struct {
    PGBytePtr queue;
    size_t    head;
    size_t    tail;
    size_t    size;
    PGBool    dealloc;
}               PGDynamicByteQueueStruct;

PGDynamicByteQueueStruct *pgDynamicQueueInit(PGDynamicByteQueueStruct *queue, size_t initialSize);

PGDynamicByteQueueStruct *pgDynamicQueueDealloc(PGDynamicByteQueueStruct *queue);

PGBool pgDynamicQueueIsFull(PGDynamicByteQueueStruct *queue);

PGBool pgDynamicQueueIsEmpty(PGDynamicByteQueueStruct *queue);

int pgDynamicQueueDequeue(PGDynamicByteQueueStruct *queue);

int pgDynamicQueuePop(PGDynamicByteQueueStruct *queue);

PGBool pgDynamicQueueRequeue(PGDynamicByteQueueStruct *queue, uint8_t byte);

PGBool pgDynamicQueueEnqueue(PGDynamicByteQueueStruct *queue, uint8_t byte);

PGBool pgDynamicQueueEnqueueAll(PGDynamicByteQueueStruct *queue, PGBytePtr buffer, size_t len);

PGBool pgDynamicQueueRequeueAll(PGDynamicByteQueueStruct *queue, PGBytePtr buffer, size_t len);

#if __has_extension(blocks)

typedef PGBool (^PGDynamicByteQueueOpBlock)(PGBytePtr buffer, size_t size, size_t *head, size_t *tail, PGBool *error, PGBool *eof);

PGBool pgDynamicQueueOperation(PGDynamicByteQueueStruct *queue, PGDynamicByteQueueOpBlock opBlock, PGBool *error, PGBool *eof);

#endif

__END_DECLS

#endif /* DynamicByteQueue_h */
