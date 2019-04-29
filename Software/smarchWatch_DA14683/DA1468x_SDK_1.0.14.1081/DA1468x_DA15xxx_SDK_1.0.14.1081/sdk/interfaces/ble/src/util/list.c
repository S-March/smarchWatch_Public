/**
 ****************************************************************************************
 *
 * @file list.c
 *
 * @brief Simple helper to manage single-linked list
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdbool.h>
#include <stddef.h>
#include "osal.h"
#include "util/list.h"

void list_add(void **head, void *elem)
{
        struct list_elem *e = elem;

        e->next = *head;
        *head = e;
}

void *list_pop_back(void **head)
{
        struct list_elem *e = *head;
        struct list_elem *p = NULL;

        if (e) {
                while (e->next) {
                        p = e;
                        e = e->next;
                }

                if (p) {
                        p->next = NULL;
                } else {
                        *head = NULL;
                }
        }

        return e;
}

void *list_peek_back(void **head)
{
        struct list_elem *e = *head;

        if (e) {
                while (e->next) {
                        e = e->next;
                }
        }

        return e;
}

uint8_t list_size(void *head)
{
        uint8_t n = 0;
        struct list_elem *e = head;

        while (e) {
                ++n;
                e = e->next;
        }

        return n;
}

void list_append(void **head, void *elem)
{
        struct list_elem *e = *head;

        if (!e) {
                list_add(head, elem);
                return;
        }

        while (e->next) {
                e = e->next;
        }

        e->next = elem;

        e = elem;
        e->next = NULL;
}

void *list_find(void *head, list_elem_match_t match, const void *ud)
{
        struct list_elem *e = head;

        while (e && !match(e, ud)) {
                e = e->next;
        }

        return e;
}

void *list_unlink(void **head, list_elem_match_t match, const void *ud)
{
        struct list_elem *e = *head;
        struct list_elem *p = NULL;

        while (e && !match(e, ud)) {
                p = e;
                e = e->next;
        }

        if (e) {
                if (p) {
                        p->next = e->next;
                } else {
                        *head = e->next;
                }
        }

        return e;
}

void list_remove(void **head, list_elem_match_t match, const void *ud)
{
        void *e = list_unlink(head, match, ud);

        if (e) {
                OS_FREE(e);
        }
}

void list_filter(void **head, list_elem_match_t match, const void *ud)
{
        struct list_elem *e = *head;
        struct list_elem *p = NULL;

        while (e) {
                if (match(e, ud)) {
                        if (p) {
                                p->next = e->next;
                        } else {
                                *head = e->next;
                        }

                        // p does not change!
                        e = e->next;

                        OS_FREE(e);
                } else {
                        p = e;
                        e = e->next;
                }
        }
}

void list_foreach(void *head, list_elem_cb_t cb, const void *ud)
{
        struct list_elem *e = head;

        while (e) {
                cb(e, ud);
                e = e->next;
        }
}

void list_free(void **head, list_elem_cb_t cb, const void *ud)
{
        while (*head) {
                struct list_elem *e = *head;
                *head = e->next;

                if (cb) {
                        cb(e, ud);
                }

                OS_FREE(e);
        }
}
