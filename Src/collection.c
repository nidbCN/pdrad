#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>

#define UTILS_MIXED_LIST_UNIT_LEN (64)
#define UTILS_MIXED_LIST_UNIT_FAST_INDEX (true)

typedef struct _Utils_MixedListNode {
    void *data[UTILS_MIXED_LIST_UNIT_LEN];
    struct _Utils_MixedListNode *prev;
    struct _Utils_MixedListNode *next;
} Utils_MixedListNode;

typedef struct _Utils_MixedList {
    size_t count;
    Utils_MixedListNode *head;
    Utils_MixedListNode *tail;
} Utils_MixedList;

const Utils_MixedList *_Utils_MixedListCreate() {
}

void **Utils_MixedListIndex(const Utils_MixedList *list, uint index) {
    if (list == NULL || index > list->count - 1)
        return NULL;

    Utils_MixedListNode *ptr = NULL;

    if (index < (list->count - 1) / 2) {
        // pre-order
        uint unitSeq = index / UTILS_MIXED_LIST_UNIT_LEN;

        ptr = list->head;
        for (uint i = 0; i < unitSeq; ++i)
            ptr = ptr->next;
    } else {
        uint unitSeq = (list->count - 1 - index) / UTILS_MIXED_LIST_UNIT_LEN;

        ptr = list->tail;
        for (uint i = 0; i < unitSeq; ++i)
            ptr = ptr->prev;
    }

    uint uintIndex = UTILS_MIXED_LIST_UNIT_FAST_INDEX
                     ? index & (UTILS_MIXED_LIST_UNIT_LEN - 1)
                     : index % UTILS_MIXED_LIST_UNIT_LEN;

    return &(ptr->data[uintIndex]);
}

void *Utils_MixedListGet(const Utils_MixedList *list, uint index) {
    return *(Utils_MixedListIndex(list, index));
}

void Utils_MixedListSet(const Utils_MixedList *list, uint index, void *ptr) {
    if (list == NULL) {
        return;
    }

    if (index <= list->count - 1) {
        Utils_MixedListIndex(list, index);
    }

}

// NOTE: this method only free MixedList itself, call it will NOT free your pointer data.
void Utils_MixedListFree(Utils_MixedList *list) {

}