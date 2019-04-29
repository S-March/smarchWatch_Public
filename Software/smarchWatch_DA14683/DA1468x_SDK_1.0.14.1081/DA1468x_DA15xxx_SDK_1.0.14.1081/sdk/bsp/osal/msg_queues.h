/**
 * \addtogroup BSP
 * \{
 * \addtogroup OSAL
 * \{
 * \addtogroup MSG_QUEUES
 * 
 * \brief OSAL message queues
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file msg_queues.h
 *
 * @brief Message queue API
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef MSG_QUEUES_H_
#define MSG_QUEUES_H_

#include <osal.h>

typedef uint16_t MSG_SIZE; /**< Type for content message size */
typedef uint16_t MSG_ID;   /**< Type for message id */
typedef uint16_t MSG_TYPE; /**< Type for message type */

typedef void (*MSG_FREE)(void *); /**< Message content free callback */

typedef void *(*MSG_ALLOC)(size_t); /**< Message content allocator */

/**
 * \brief Default memory allocation function for queues
 *
 * If not otherwise specified, default memory allocation function used by queues
 * will be taken from OS.
 *
 */
#ifndef MSG_QUEUE_MALLOC
#define MSG_QUEUE_MALLOC OS_MALLOC_FUNC
#endif

/**
 * \brief Default memory free function for queues
 *
 * If not otherwise specified, default memory free function used by queues
 * will be taken from OS.
 *
 */
#ifndef MSG_QUEUE_FREE
#define MSG_QUEUE_FREE OS_FREE_FUNC
#endif

/**
 * \brief Message queue content allocator
 *
 * Content allocator consist of two functions that will allocate and release memory needed
 * by messages in cases when data needs to be copied from sender to receiver.
 * Allocator will be associated with msg_queue so user can initialize messages using correct
 * allocation functions.
 * Those function will be used when user calls msg_queue_send() function.
 *
 * If CONFIG_MSG_QUEUE_USE_ALLOCATORS is not defined queues will not have dedicated allocators.
 * In this case MSG_QUEUE_MALLOC and MSG_QUEUE_FREE macros will be used to allocate and free
 * memory.
 *
 * \sa msg_queue_create
 * \sa msg_queue_send
 *
 */
typedef struct content_allocator {
        MSG_ALLOC content_alloc; /**< Pointer to allocate memory function */
        MSG_FREE content_free;   /**< Pointer to free memory function */
} content_allocator;

#if CONFIG_MSG_QUEUE_USE_ALLOCATORS

/**
 * \brief Default memory allocator
 *
 * This allocator should be used when user is satisfied with OS specific memory allocation
 * functions.
 *
 */
#define DEFAULT_OS_ALLOCATOR (&default_os_allocator)
extern const content_allocator default_os_allocator;

#else

#define DEFAULT_OS_ALLOCATOR NULL

#endif

/**
 * \brief Message queue structure
 *
 * This structure wraps OS specific queue with additional data need to handle memory allocations.
 * If user knows that data send over specific queue will never need to be copied, memory
 * allocator can be NULL.
 *
 */
typedef struct msq_queue {
        OS_QUEUE queue;                /**< OS specific queue */
#if CONFIG_MSG_QUEUE_USE_ALLOCATORS
        content_allocator *allocator;  /**< Memory allocator, can be NULL */
#endif
} msg_queue;

/**
 * \brief Structure for messages with id, type, data
 *
 * Structure that will be passed to message queues. Content of this structure will be copied
 * to queue and can be released by sender except data pointed by \p data.
 * When message is sent with msg_queue_put() entire \p msg structure is copied.
 * The data part of variable size pointed by \p data is not copied at this time.
 * When receiving a message, msg_release() will be called, which will in turn call
 * free_cb(data) if free_cb() is not NULL.
 * \p free_cb is set by message queue allocator when msg_queue_init_msg() is used to initialize
 * message. It is also set with msg_init() or msq_queue_send_zero_copy() function calls.
 *
 */
typedef struct msg {
        MSG_ID id;        /**< Message ID - not touched by queues */
        MSG_TYPE type;    /**< Message type - not touched by queues */
        MSG_SIZE size;    /**< Size of data pointed by \p data */
        uint8_t *data;    /**< Variable part of message */
        MSG_FREE free_cb; /**< Pointer to function to call when receiver is done with message */
} msg;

/**
 * \brief Create message queue
 *
 * Function creates message queue that will handle messages of type \p msg with small fixed
 * size part and variable size data part.
 *
 * Typical usage for task owning queue:
 * \code{.c}
 * {
 *         msg_queue q;
 *         msg_queue_create(&q, 5, DEFAULT_OS_ALLOCATOR);
 *         while (1) {
 *                msg m;
 *
 *                msg_queue_get(&q, &m, OS_QUEUE_FOREVE);
 *
 *                switch(m.type) {
 *                ....
 *                }
 *
 *                msg_release(&m);
 *         }
 *         msg_queue_delete(&q);
 * }
 * \endcode
 *
 * \param [in] queue pointer to queue to initialize
 * \param [in] queue_size max number of elements that queue can have
 * \param [in] allocator structure with allocate/free functions for message content memory
 *
 * \sa msq_queue_delete
 * \sa msg_queue_get
 * \sa msg_release
 *
 */
void msg_queue_create(msg_queue *queue, int queue_size, content_allocator *allocator);

/**
 * \brief Delete message queue
 *
 * Function deletes message queue created with msq_queue_create().
 *
 * \param [in] queue message queue to delete
 *
 * \sa msg_queue_create
 *
 */
void msg_queue_delete(msg_queue *queue);

/**
 * \brief Put message in queue
 *
 * Function adds message to the queue.
 * If queue is full function waits for specified time to put message. If in this
 * time queue is still full function fails.
 *
 * In case function is called from ISR, functions fails immediately if there's not free
 * space in queue (\p timeout parameter has no effect).
 *
 * If message was not put in queue, free_cb() will be called immediately.
 *
 * Typical usage for task posting to queue using msg_queue_put():
 * \code{.c}
 * {
 *         msg_queue *q = ....;
 *         while (1) {
 *                msg m;
 *                char msg_data[10]
 *
 *                msg_queue_init_msg(q, &m, (MSG_ID)1, (MSG_TYPE)2, sizeof(msg_data));
 *                memcpy(m.data, msg_data, sizeof(msg_data));
 *                msg_queue_put(q, &m, OS_QUEUE_FOREVE);
 *                ...
 *                ...
 *                msg_init(&m, (MSG_ID)1, (MSG_TYPE)2, msg_data, sizeof(msg_data), my_free_cb);
 *                msg_queue_put(q, &m, OS_QUEUE_FOREVE);
 *         }
 * }
 * \endcode
 *
 * \param [in] queue queue to put message to
 * \param [in] msg message to send
 * \param [in] timeout time to wait before failure if queue is full in ticks
 *             OS_QUEUE_NO_WAIT - do not wait at all if queue is full fail
 *             OS_QUEUE_FOREVER - wait till message can be put
 *
 * \returns OS_QUEUE_OK if message was put in queue
 *          OS_QUEUE_FULL if message was not put in queue
 *
 * \sa msg_queue_get
 *
 */
int msg_queue_put(msg_queue *queue, msg *msg, OS_TICK_TIME timeout);

/**
 * \brief Get message from queue
 *
 * Function gets message from the queue.
 * If queue is empty function waits for specified time for message. If in this
 * time queue is still empty function fails.
 * When receiver is done with message it must call msg_release(msg).
 *
 * See description of msg_queue_create() for usage example.
 *
 * \param [in] queue queue to get message from
 * \param [in,out] msg pointer for message
 * \param [in] timeout time to wait before failure if queue is empty in ticks
 *             OS_QUEUE_NO_WAIT - do not wait at all if queue is full fail
 *             OS_QUEUE_FOREVER - wait till message can be put
 *
 * \returns OS_QUEUE_OK if message was taken from queue
 *          OS_QUEUE_EMPTY if there was no message
 *
 * \sa msg_queue_put
 * \sa msg_queue_create
 *
 */
int msg_queue_get(msg_queue *queue, msg *msg, OS_TICK_TIME timeout);

/**
 * \brief Prepare message with freeing callback
 *
 * Basic function to initialize message.
 *
 * \param [in] msg pointer to message to initialize
 * \param [in] id message id
 * \param [in] type message type
 * \param [in] buf data to associate with message
 * \param [in] size size of data pointed by \p buf
 * \param [in] free_cb function to call from msg_release()
 *
 * \sa msg_queue_put
 * \sa msg_queue_get
 * \sa msg_release
 * \sa msg_queue_init_msg
 *
 */
void msg_init(msg *msg, MSG_ID id, MSG_TYPE type, void *buf, MSG_SIZE size, MSG_FREE free_cb);

/**
 * \brief Release message data
 *
 * This function must be called by received when message is no longer needed.
 * Depending of how freeing callback of message is done, it can release memory pinter by \p data,
 * or it can notify sender that memory can be reused.
 *
 * \param [in] msg pointer to message to free
 *
 * \sa msg_queue_put
 * \sa msg_queue_get
 * \sa msg_queue_init
 *
 */
void msg_release(msg *msg);

/**
 * \brief Initialize message with queue specific freeing callback
 *
 * This function should be called when buffer with specified size should be allocate from queue
 * allocator.
 *
 * \param [in] queue queue to use
 * \param [in] msg pointer to message to initialize
 * \param [in] id message id
 * \param [in] type message type
 * \param [in] size required data size
 *
 * \return 0 if memory can't be allocated, 1 on success
 *
 * \sa msg_init
 * \sa msg_release
 *
 */
int msg_queue_init_msg(msg_queue *queue, msg *msg, MSG_ID id, MSG_TYPE type, MSG_SIZE size);

/**
 * \brief Send data to queue
 *
 * This function will allocate data of \p size and send it to queue.
 * \p buf is free to use by sender as soon as function returns, data is copied to additional
 * buffer allocated with queue specific allocator.
 * Function can fail in case when queue is full or there is no memory to allocate.
 *
 * Typical usage for task posting to queue using msq_queue_send():
 * \code{.c}
 * {
 *         msg_queue *q = ....;
 *         while (1) {
 *                char msg_data[10]
 *
 *                strcpy(msg_data, "Simple1");
 *                msq_queue_send(q, (MSG_ID)1, (MSG_TYPE)2, msg_data, sizeof(msg_data),
 *                                                              OS_QUEUE_FOREVE);
 *                // No need to wait here, msg_data was copied to message buffer
 *                strcpy(msg_data, "Simple2");
 *                msq_queue_send(q, (MSG_ID)2, (MSG_TYPE)2, msg_data, sizeof(msg_data),
 *                                                              OS_QUEUE_FOREVE);
 *                ...
 *                ...
 *         }
 * }
 * \endcode
 *
 * \param [in] queue queue to use
 * \param [in] id message id
 * \param [in] type message type
 * \param [in] buf data to associate with message
 * \param [in] size required data size
 * \param [in] timeout time to wait before failure if queue is empty in ticks
 *             OS_QUEUE_NO_WAIT - do not wait at all if queue is full fail
 *             OS_QUEUE_FOREVER - wait till message can be put
 *
 * \returns OS_QUEUE_OK if message was put in queue
 *          OS_QUEUE_FULL if message was not put in queue
 *
 */
int msg_queue_send(msg_queue *queue, MSG_ID id, MSG_TYPE type, void *buf, MSG_SIZE size,
                                                                        OS_TICK_TIME timeout);

/**
 * \brief Send data to queue with zero copy
 *
 * This function will send message to queue and specify callback to call when message
 * was received or not sent at all.
 * This function should be used if no data copy is needed and sender can keep data untouched
 * till free_cb() is called. In this case free_cb() is used as signaling mechanism,
 * it can signal sender by means of OS_EVENT.
 * If message was not put in queue because it was full for specified time, free_cb() will be called
 * anyway.
 * If user don't want to provide freeing callback for synchronization and copy data is ok,
 * msg_queue_send() can be simpler choice.
 *
 * Typical usage for task posting to queue using msq_queue_send_zero_copy():
 * \code{.c}
 * {
 *         msg_queue *q = ....;
 *         while (1) {
 *                char msg_data[10]
 *
 *                msq_queue_send_zero_copy(q, (MSG_ID)1, (MSG_TYPE)2, msg_data, sizeof(msg_data),
 *                                                              OS_QUEUE_FOREVE, my_free_cb);
 *                ...
 *                ...
 *                // Wait for event set by my_free_cb, before using msg_data again
 *                OS_WAIT_EVENT(event, OS_EVENT_FOREVERE);
 *         }
 * }
 * \endcode
 *
 * \param [in] queue queue to use
 * \param [in] id message id
 * \param [in] type message type
 * \param [in] buf data to associate with message
 * \param [in] size required data size
 * \param [in] timeout time to wait before failure if queue is empty in ticks
 *             OS_QUEUE_NO_WAIT - do not wait at all if queue is full fail
 *             OS_QUEUE_FOREVER - wait till message can be put
 * \param [in] free_cb function to call from msg_release()
 *
 * \returns OS_QUEUE_OK if message was put in queue
 *          OS_QUEUE_FULL if message was not put in queue
 *
 * \sa msg_queue_send
 */
int msq_queue_send_zero_copy(msg_queue *queue, MSG_ID id, MSG_TYPE type, void *buf,
                                        MSG_SIZE size, OS_TICK_TIME timeout, MSG_FREE free_cb);

#endif /* MSG_QUEUES_H_ */

/**
 * \}
 * \}
 * \}
 */
