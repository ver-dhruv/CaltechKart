#include "list.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

const double GROWTH_FACTOR = 2;

typedef struct list {
  void **data;
  size_t size;
  size_t capacity;
  free_func_t freer;
} list_t;

list_t *list_init(size_t initial_capacity, free_func_t freer) {
  assert(initial_capacity >= 0);
  list_t *list = (list_t *)malloc(sizeof(list_t));
  assert(list != NULL);
  list->size = 0;
  list->capacity = initial_capacity;
  list->data = malloc(initial_capacity * sizeof(void *));
  assert(list->data != NULL);
  list->freer = freer;
  return list;
}

void list_free(list_t *list) {
  if (list->freer != NULL) {
    for (size_t i = 0; i < list->size; i++) {
      list->freer(list->data[i]);
    }
  }
  free(list->data);
  free(list);
}

size_t list_size(list_t *list) { return list->size; }

void *list_get(list_t *list, size_t index) {
  assert(index < list->size && index >= 0);
  return list->data[index];
}

void *list_remove(list_t *list, size_t index) {
  assert(0 <= index && index < list->size);
  void *removed = list->data[index];
  for (size_t i = index + 1; i < list->size; i++) {
    list->data[i - 1] = list->data[i];
  }
  list->data[list->size - 1] = NULL;
  list->size--;
  return removed;
}

void list_add(list_t *list, void *value) {
  assert(value != NULL);
  if (list->size == list->capacity) {
    void **old_data = list->data;
    list->capacity = GROWTH_FACTOR * list->capacity + 1;
    list->data = malloc(list->capacity * sizeof(void *));
    assert(list->data != NULL);
    for (size_t i = 0; i < list->size; i++) {
      list->data[i] = old_data[i];
    }
    free(old_data);
  }
  assert(list->size < list->capacity);
  list->data[list->size] = value;
  list->size++;
}