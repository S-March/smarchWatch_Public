/**
 ****************************************************************************************
 *
 * @file list.h
 *
 * @brief Simple helper to manage single-linked list
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef LIST_H_
#define LIST_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * List element
 *
 * Each list element can be of any user defined type as long as its first member is pointer to
 * structure of the same type.
 *
 */
struct list_elem {
        struct list_elem *next;
};

/**
 * Callback for elements
 *
 * \param [in] elem  evaluated element
 * \param [in] ud    user data
 *
 */
typedef void (* list_elem_cb_t) (const void *elem, const void *ud);

/**
 * Callback for matching elements
 *
 * \param [in] elem  evaluated element
 * \param [in] ud    user data
 *
 * \return true if element is matches, false otherwise
 *
 */
typedef bool (* list_elem_match_t) (const void *elem, const void *ud);

/**
 * Add element to list
 *
 * Element is added to the beginning of list.
 *
 * \param [in,out] head   list head
 * \param [in]     elem   new element
 *
 */
void list_add(void **head, void *elem);

/**
 * Delete element from the end of the list
 *
 * \param [in,out] head   list head
 *
 * \return last element in list
 *
 */
void *list_pop_back(void **head);

/**
 * Peek element from the end of the list
 *
 * \param [in,out] head   list head
 *
 * \return last element in list
 *
 */
void *list_peek_back(void **head);

/**
 * Get number of elements in list
 *
 * \param [in] head   list head
 *
 * \return number of elements in list
 *
 */
uint8_t list_size(void *head);

/**
 * Append element to list
 *
 * It's recommended to use list_add() whenever possible since it works in constant time.
 *
 * \param [in,out] head   list head
 * \param [in]     elem   new element
 *
 */
void list_append(void **head, void *elem);

/**
 * Find element in list
 *
 * \param [in]     head   list head
 * \param [in]     match  callback to match element
 * \param [in]     ud     user data
 *
 * \return found element or NULL if not found
 *
 */
void *list_find(void *head, list_elem_match_t match, const void *ud);

/**
 * Unlink element from list
 *
 * \param [in,out] head   list head
 * \param [in]     match  callback to match element
 * \param [in]     ud     user data
 *
 * \return unlinked element or NULL if not found
 *
 */
void *list_unlink(void **head, list_elem_match_t match, const void *ud);

/**
 * Remove element from list
 *
 * \param [in,out] head   list head
 * \param [in]     match  callback to match element
 * \param [in]     ud     user data
 *
 */
void list_remove(void **head, list_elem_match_t match, const void *ud);

/**
 * Removes all matched elements from list
 *
 * \param [in,out] head   list head
 * \param [in]     match  callback to match element
 * \param [in]     ud     user data
 *
 */
void list_filter(void **head, list_elem_match_t match, const void *ud);

/**
 * Iterates over entire list
 *
 * \param [in]     head   list head
 * \param [in]     cb     callback to be called for each element
 * \param [in]     ud     user data
 *
 */
void list_foreach(void *head, list_elem_cb_t cb, const void *ud);

/**
 * Remove all elements from list
 *
 * \param [in,out] head   list head
 * \param [in]     cb     callback to be called for each element before removing it
 * \param [in]     ud     user data
 *
 */
void list_free(void **head, list_elem_cb_t cb, const void *ud);

#endif /* LIST_H_ */
