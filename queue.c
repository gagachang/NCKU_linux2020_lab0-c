#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "natsort/strnatcmp.h"
#include "queue.h"

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
queue_t *q_new()
{
    queue_t *q = malloc(sizeof(queue_t));
    if (!q) {
        return NULL;
    }

    q->head = NULL;
    q->tail = NULL;
    q->size = 0;

    return q;
}

/* Free all storage used by queue */
void q_free(queue_t *q)
{
    list_ele_t *temp;

    if (!q) {
        return;
    }

    while ((temp = q->head) != NULL) {
        if (temp->value) {
            free(temp->value);
        }
        q->head = q->head->next;
        free(temp);
    }

    free(q);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(queue_t *q, char *s)
{
    list_ele_t *newh;
    size_t length;

    if (!q) {
        return false;
    }

    newh = malloc(sizeof(list_ele_t));
    if (!newh) {
        return false;
    }

    length = strlen(s);
    newh->value = malloc(length + 1);
    if (!newh->value) {
        free(newh);
        newh = NULL;
        return false;
    }

    strncpy(newh->value, s, length);
    newh->value[length] = '\0';

    newh->next = q->head;
    q->head = newh;
    q->size++;

    /* if there is only one element in queue, set the newh as q->tail too */
    if (!q->tail) {
        q->tail = newh;
    }

    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(queue_t *q, char *s)
{
    list_ele_t *newt;
    size_t length;

    if (!q) {
        return false;
    }

    newt = malloc(sizeof(list_ele_t));
    if (!newt) {
        return false;
    }

    length = strlen(s);
    newt->value = malloc(length + 1);
    if (!newt->value) {
        free(newt);
        newt = NULL;
        return false;
    }

    strncpy(newt->value, s, length);
    newt->value[length] = '\0';

    newt->next = NULL;

    if (q->tail) {
        q->tail->next = newt;
    }
    q->tail = newt;
    q->size++;

    /* if there is only one element in queue, set the newt as q->head too */
    if (q->size == 1) {
        q->head = newt;
    }

    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return true if successful.
 * Return false if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 * The space used by the list element and the string should be freed.
 */
bool q_remove_head(queue_t *q, char *sp, size_t bufsize)
{
    list_ele_t *original_head;

    if (!q || q->size == 0) {
        return false;
    }

    if (sp && q->head->value) {
        size_t length;
        length = strlen(q->head->value);
        length = length <= bufsize - 1 ? length : bufsize - 1;
        strncpy(sp, q->head->value, length);
        sp[length] = '\0';
    }

    original_head = q->head;
    q->head = q->head->next;

    if (original_head->value) {
        free(original_head->value);
    }
    free(original_head);
    original_head = NULL;
    q->size--;

    /* if there is no element in queue, just set the q->tail to NULL too */
    if (q->size == 0) {
        q->tail = NULL;
    }
    return true;
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(queue_t *q)
{
    return q == NULL ? 0 : q->size;
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(queue_t *q)
{
    if (!q || q->size == 0 || q->size == 1) {
        return;
    }

    list_ele_t *former, *latter;
    former = q->head;
    latter = NULL;

    q->tail = q->head;
    while (former) {
        q->head = q->head->next;
        former->next = latter;
        latter = former;
        former = q->head;
    }
    q->head = latter;
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(queue_t *q)
{
    if (!q || q->head == NULL || q->size < 2) {
        return;
    }

    q->head = merge_sort_list(q->head);

    // find the new q->tail
    list_ele_t *tail = q->head;
    while (tail->next) {
        tail = tail->next;
    }
    q->tail = tail;
}

list_ele_t *merge(list_ele_t *left, list_ele_t *right)
{
    // merge with recursive
    if (!right)
        return left;
    if (!left)
        return right;

    list_ele_t *head, *temp;

    // find the head of the two splitted lists
    if (strnatcmp(left->value, right->value) < 0) {
        head = left;
        left = left->next;
    } else {
        head = right;
        right = right->next;
    }

    // keep sorting
    temp = head;

    while (left && right) {
        if (strnatcmp(left->value, right->value) < 0) {
            temp->next = left;
            left = left->next;
        } else {
            temp->next = right;
            right = right->next;
        }
        temp = temp->next;
    }

    // check if there is a rest element
    if (left)
        temp->next = left;
    if (right)
        temp->next = right;

    return head;
}

list_ele_t *merge_sort_list(list_ele_t *head)
{
    if (!head || !head->next)
        return head;

    list_ele_t *fast = head->next;
    list_ele_t *slow = head;

    // split list
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    fast = slow->next;
    slow->next = NULL;

    // sort each list
    list_ele_t *left = merge_sort_list(head);
    list_ele_t *right = merge_sort_list(fast);

    // merge
    return merge(left, right);
}
