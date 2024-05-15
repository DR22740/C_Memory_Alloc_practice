#define _XOPEN_SOURCE_EXTENDED 1
#include <unistd.h>
#include <stdio.h>


typedef char ALIGN[16];

union header {
    struct {
        size_t size;
        unsigned is_free;
        union header *next;
    } s;
    ALIGN stub;
};

typedef union header header_t;

header_t *head, *tail;

header_t *get_free_block(size_t size) {
    header_t *current = head;
    while (current) {
        if (current->s.is_free && current->s.size >= size)
            return current;
        current = current->s.next;
    }
    return NULL;
}

void *my_malloc(size_t size) {
    size_t total_size;
    void *block;
    header_t *header;
    if (!size)
        return NULL;
    header = get_free_block(size);
    if (header) {
        header->s.is_free = 0;
        return (void*)(header + 1);
    }
    total_size = sizeof(header_t) + size;
    block = sbrk(total_size);
    if (block == (void*) -1) {
        return NULL;
    }
    header = block;
    header->s.size = size;
    header->s.is_free = 0;
    header->s.next = NULL;
    if (!head)
        head = header;
    if (tail)
        tail->s.next = header;
    tail = header;
    return (void*)(header + 1);
}

void my_free(void *block) {
    header_t *header, *tmp;
    void *programbreak;

    if (!block)
        return;
    header = (header_t*)block - 1;
    programbreak = sbrk(0);
    if ((char*)block + header->s.size == programbreak) {
        if (head == tail) {
            head = tail = NULL;
        } else {
            tmp = head;
            while (tmp) {
                if (tmp->s.next == header) {
                    tmp->s.next = NULL;
                    tail = tmp;
                }
                tmp = tmp->s.next;
            }
        }
        sbrk(0 - sizeof(header_t) - header->s.size);
        return;
    }
    header->s.is_free = 1;
}


int main() {
    printf("Hacky Alloc in C\n");
    int *num = (int*) my_malloc(sizeof(int));
    *num = 123;
    printf("Num = %d\n", *num);
    printf("Address:: %p\n", num);
    printf("Size: %lu bytes\n", sizeof(int));


    my_free(num);
    printf("Memory block freed\n");
    return 0;
}
