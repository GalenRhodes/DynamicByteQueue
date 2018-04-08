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

size_t pgDynamicQueueGrowBuffer(PGDynamicByteQueueStruct *queue, char *err) {
    size_t  osize   = queue->size;
    size_t  nsize   = (osize * 2);
    uint8_t *nqueue = realloc(queue->queue, nsize);

    if(nqueue) {
        if(err) *err = 0;
        queue->queue = nqueue;
        queue->size  = nsize;

        if(queue->tail < queue->head) {
            if(queue->tail) memcpy((nqueue + osize), nqueue, queue->tail);
            queue->tail += osize;
        }
    }
    else if(err) {
        *err = 1;
    }

    return queue->tail;
}

PGDynamicByteQueueStruct *pgDynamicQueueInit(PGDynamicByteQueueStruct *queue, size_t initialSize) {
    char f = 0;

    if(!queue) {
        f     = 1;
        queue = malloc(sizeof(PGDynamicByteQueueStruct));
    }

    if(queue) {
        memset(queue, 0, sizeof(PGDynamicByteQueueStruct));
        queue->size  = (initialSize ?: (64 * 1024));
        queue->queue = calloc(1, queue->size);

        if(!queue->queue) {
            if(f) free(queue);
            return NULL;
        }
    }

    return queue;
}

char pgDynamicQueueIsFull(PGDynamicByteQueueStruct *queue) {
    return (pgDynamicQueueNextTail(queue) == queue->head);
}

char pgDynamicQueueIsEmpty(PGDynamicByteQueueStruct *queue) {
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

char pgDynamicQueueRequeue(PGDynamicByteQueueStruct *queue, uint8_t byte) {
    char error = 0;
    if(pgDynamicQueueIsFull(queue)) pgDynamicQueueGrowBuffer(queue, &error);
    if(!error) {
        queue->head = pgDynamicQueuePrevHead(queue);
        queue->queue[queue->head] = byte;
    }
    return error;
}

char pgDynamicQueueEnqueue(PGDynamicByteQueueStruct *queue, uint8_t byte) {
    char error = 0;
    if(pgDynamicQueueIsFull(queue)) pgDynamicQueueGrowBuffer(queue, &error);
    if(!error) {
        queue->queue[queue->tail] = byte;
        queue->tail = pgDynamicQueueNextTail(queue);
    }
    return error;
}

char pgDynamicQueueEnqueueAll(PGDynamicByteQueueStruct *queue, uint8_t *buffer, size_t len) {
    char error = 0;

    for(size_t i = 0; ((i < len) && (error == 0)); i++) {
        if(pgDynamicQueueIsFull(queue)) pgDynamicQueueGrowBuffer(queue, &error);
        if(!error) {
            queue->queue[queue->tail] = buffer[i];
            queue->tail = pgDynamicQueueNextTail(queue);
        }
    }

    return error;
}

char pgDynamicQueueRequeueAll(PGDynamicByteQueueStruct *queue, uint8_t *buffer, size_t len) {
    char error = 0;

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

char pgDynamicQueueOperation(PGDynamicByteQueueStruct *queue, PGDynamicByteQueueOpBlock opBlock, char *error, char *eof) {
    size_t _head  = queue->head;
    size_t _tail  = queue->tail;
    char   _error = 0;
    char   _eof   = 0;
    char   _res   = opBlock(queue->queue, queue->size, &_head, &_tail, &_error, &_eof);

    queue->head = _head;
    queue->tail = _tail;

    if(error) *error = _error;
    if(eof) *eof     = _eof;

    return (_res || _error || _eof);
}

#endif
