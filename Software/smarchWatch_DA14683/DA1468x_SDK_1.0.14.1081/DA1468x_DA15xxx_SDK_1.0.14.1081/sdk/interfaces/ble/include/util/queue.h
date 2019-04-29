/**
 ****************************************************************************************
 *
 * @file queue.h
 *
 * @brief Simple helper to manage queue
 *
 * Copyright (C) 2015 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#include <stdbool.h>
#include <stddef.h>

struct queue_elem;

typedef struct {
        size_t size;
        struct queue_elem *head;
        struct queue_elem *tail;
} queue_t;

typedef void (* queue_foreach_func_t) (void *data, void *user_data);

typedef bool (* queue_match_func_t) (const void *data, const void *match_data);

typedef void (* queue_destroy_func_t) (void *data);

/**
 * Initialize queue structure
 *
 * \param [in] q        queue structure
 *
 */
void queue_init(queue_t *q);

/**
 * Add element on the front of queue
 *
 * \param [in] q        queue structure
 * \param [in] data     queue element
 *
 */
void queue_push_front(queue_t *q, void *data);

/**
 * Add element on the back of queue
 *
 * \param [in] q        queue structure
 * \param [in] data     queue element
 *
 */
void queue_push_back(queue_t *q, void *data);

/**
 * Remove element from the front of queue
 *
 * \param [in] q        queue structure
 *
 * \return queue element
 *
 */
void *queue_pop_front(queue_t *q);

/**
 * Get element from the front of queue
 *
 * Element is not removed from queue
 *
 * \param [in] q        queue structure
 *
 * \return queue element
 *
 */
void *queue_peek_front(const queue_t *q);

/**
 * Get element from the back of queue
 *
 * Element is not removed from queue
 *
 * \param [in] q        queue structure
 *
 * \return queue element
 *
 */
void *queue_peek_back(const queue_t *q);

/**
 * Get number of elements on queue
 *
 * \param [in] q        queue structure
 *
 * \return number of elements in queue
 *
 */
size_t queue_length(const queue_t *q);

/**
 * Execute callback for each element of queue
 *
 * \param [in] q        queue structure
 * \param [in] func     callback function
 * \param [in] user_data any pointer passed to callback function
 *
 */
void queue_foreach(const queue_t *q, queue_foreach_func_t func, void *user_data);

/**
 * Find element in queue
 *
 * First element which matches using \p func is returned. Element IS NOT removed from queue
 *
 * \param [in] q        queue structure
 * \param [in] func     callback matching function
 * \param [in] match_data any pointer passed to callback function
 *
 * \return queue element
 *
 */
void *queue_find(const queue_t *q, queue_match_func_t func, const void *match_data);

/**
 * Remove element from queue
 *
 * First element which matches using \p func is returned. Element IS removed from queue
 *
 * \param [in] q        queue structure
 * \param [in] func     callback matching function
 * \param [in] match_data any pointer passed to callback function
 *
 * \return queue element
 *
 */
void *queue_remove(queue_t *q, queue_match_func_t func, const void *match_data);

/**
 * Remove all elements from queue
 *
 * Callback is called for each element and it should be freed there. Queue is empty after this call.
 *
 * \param [in] q        queue structure
 * \param [in] func     callback function to free element
 *
 */
void queue_remove_all(queue_t *q, queue_destroy_func_t func);

/**
 * Remove all matching elements from queue
 *
 * Every element which matches using \p m_func is removed from queue.
 * \p d_func callback is called for each element and it should be freed there.
 *
 * \param [in] q        queue structure
 * \param [in] m_func   callback matching function
 * \param [in] match_data any pointer passed to callback function
 * \param [in] d_func   callback function to free element
 *
 */
void queue_filter(queue_t *q, queue_match_func_t m_func, const void *match_data,
                                                                        queue_destroy_func_t d_func);

#endif /* QUEUE_H_ */
