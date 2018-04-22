typedef struct minHeap minHeap;

minHeap initMinHeap(int size);
void insertNode(minHeap *hp, int data);
int deleteNode(minHeap *hp);