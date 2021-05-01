struct Node {
  int value;
  struct Node *next;
};

struct Queue {
  struct Node *head;
  struct Node *tail;
};

struct Queue q;

void enqueue(struct Queue* queue, struct Node *node) {
  if (!queue->head) {
    queue->head = node;
    queue->tail = node;
    return;
  }
  queue->tail->next = node;
  queue->tail = node;
  return;
}

struct Node *dequeue(struct Queue* queue) {
  struct Node *node = queue->head;
  queue->head = queue->head->next;
  return node;
}

int main() {
  struct Node n1;
  struct Node n2;
  struct Node n3;

  n1.value = 1;
  n2.value = 2;
  n3.value = 3;

  n1.next = 0;
  n2.next = 0;
  n3.next = 0;

  enqueue(&q, &n1);
  enqueue(&q, &n2);
  enqueue(&q, &n3);

  printf("%d\n", dequeue(&q)->value);
  printf("%d\n", dequeue(&q)->value);
  printf("%d\n", dequeue(&q)->value);
  return 0;
}