//
//  DynamicByteQueue.c
//  DynamicByteQueue
//
//  Created by Galen Rhodes on 4/7/18.
//  Copyright © 2018 Project Galen. All rights reserved.
//

#include "DynamicByteQueue.h"
#include <stdlib.h>
#include <string.h>

#if !defined(NS_INLINE)
    #if defined(__GNUC__)
        #define NS_INLINE static __inline__ __attribute__((always_inline))
    #elif defined(__MWERKS__) || defined(__cplusplus)
        #define NS_INLINE static inline
    #elif defined(_MSC_VER)
        #define NS_INLINE static __inline
    #elif TARGET_OS_WIN32
        #define NS_INLINE static __inline__
    #endif
#endif

NS_INLINE size_t pgDynamicQueueNextHead(PGDynamicByteQueueStruct *queue) {
    return ((queue->head + 1) % queue->size);
}

NS_INLINE size_t pgDynamicQueueNextTail(PGDynamicByteQueueStruct *queue) {
    return ((queue->tail + 1) % queue->size);
}

NS_INLINE size_t pgDynamicQueuePrevHead(PGDynamicByteQueueStruct *queue) {
    return ((queue->head ?: queue->size) - 1);
}

NS_INLINE size_t pgDynamicQueuePrevTail(PGDynamicByteQueueStruct *queue) {
    return ((queue->tail ?: queue->size) - 1);
}

size_t pgDynamicQueueGrowBuffer(PGDynamicByteQueueStruct *queue, PGBool *err) {
    size_t    osize  = queue->size;
    size_t    nsize  = (osize * 2);
    PGBytePtr nqueue = realloc(queue->queue, nsize);

    if(nqueue) {
        if(err) *err = FALSE;
        queue->queue = nqueue;
        queue->size  = nsize;

        if(queue->tail < queue->head) {
            if(queue->tail) memcpy((nqueue + osize), nqueue, queue->tail);
            queue->tail += osize;
        }
    }
    else if(err) {
        *err = TRUE;
    }

    return queue->tail;
}

PGDynamicByteQueueStruct *pgDynamicQueueInit(PGDynamicByteQueueStruct *queue, size_t initialSize) {
    PGBool f = FALSE;

    if(!queue) {
        f     = TRUE;
        queue = malloc(sizeof(PGDynamicByteQueueStruct));
    }

    if(queue) {
        memset(queue, 0, sizeof(PGDynamicByteQueueStruct));
        queue->size  = (initialSize ?: PG_DEFAULT_SIZE);
        queue->queue = calloc(1, queue->size);

        if(!queue->queue) {
            if(f) free(queue);
            return NULL;
        }
    }

    return queue;
}

PGBool pgDynamicQueueIsFull(PGDynamicByteQueueStruct *queue) {
    return (pgDynamicQueueNextTail(queue) == queue->head);
}

PGBool pgDynamicQueueIsEmpty(PGDynamicByteQueueStruct *queue) {
    return (queue->head == queue->tail);
}

int pgDynamicQueueDequeue(PGDynamicByteQueueStruct *queue) {
    if(pgDynamicQueueIsEmpty(queue)) return EOF;
    size_t i = queue->head;
    queue->head = pgDynamicQueueNextHead(queue);
    return (int)queue->queue[i];
}

int pgDynamicQueuePop(PGDynamicByteQueueStruct *queue) {
    if(pgDynamicQueueIsEmpty(queue)) return EOF;
    queue->tail = pgDynamicQueuePrevTail(queue);
    return queue->queue[queue->tail];
}

PGBool pgDynamicQueueRequeue(PGDynamicByteQueueStruct *queue, PGByte byte) {
    PGBool error = FALSE;
    if(pgDynamicQueueIsFull(queue)) pgDynamicQueueGrowBuffer(queue, &error);
    if(!error) {
        queue->head = pgDynamicQueuePrevHead(queue);
        queue->queue[queue->head] = byte;
    }
    return error;
}

PGBool pgDynamicQueueEnqueue(PGDynamicByteQueueStruct *queue, PGByte byte) {
    PGBool error = FALSE;
    if(pgDynamicQueueIsFull(queue)) pgDynamicQueueGrowBuffer(queue, &error);
    if(!error) {
        queue->queue[queue->tail] = byte;
        queue->tail = pgDynamicQueueNextTail(queue);
    }
    return error;
}

PGBool pgDynamicQueueEnqueueAll(PGDynamicByteQueueStruct *queue, PGBytePtr buffer, size_t len) {
    PGBool error = FALSE;

    for(size_t i = 0; ((i < len) && (error == FALSE)); i++) {
        if(pgDynamicQueueIsFull(queue)) pgDynamicQueueGrowBuffer(queue, &error);
        if(!error) {
            queue->queue[queue->tail] = buffer[i];
            queue->tail = pgDynamicQueueNextTail(queue);
        }
    }

    return error;
}

PGBool pgDynamicQueueRequeueAll(PGDynamicByteQueueStruct *queue, PGBytePtr buffer, size_t len) {
    PGBool error = FALSE;

    while(len && !error) {
        if(pgDynamicQueueIsFull(queue)) pgDynamicQueueGrowBuffer(queue, &error);
        if(!error) {
            queue->head = pgDynamicQueuePrevHead(queue);
            queue->queue[queue->head] = buffer[--len];
        }
    }

    return error;
}

#if __has_extension(blocks)

PGBool pgDynamicQueueOperation(PGDynamicByteQueueStruct *queue, PGDynamicByteQueueOpBlock opBlock, PGBool *error, PGBool *eof) {
    size_t _head  = queue->head;
    size_t _tail  = queue->tail;
    PGBool _error = FALSE;
    PGBool _eof   = FALSE;
    PGBool _res   = opBlock(queue->queue, queue->size, &_head, &_tail, &_error, &_eof);

    queue->head = _head;
    queue->tail = _tail;

    if(error) *error = _error;
    if(eof) *eof     = _eof;

    return (_res || _error || _eof);
}

#endif
