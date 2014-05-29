#ifndef RESERVOIR_SAMPLE_H
#define RESERVOIR_SAMPLE_H

#define DEFAULT_OFFSET_VALUE -1
#define LINE_LENGTH_VALUE 65536

typedef struct offset_node offset_node;
typedef struct reservoir reservoir;

void print_reservoir_sample(const char *in_fn, reservoir *res_ptr);
void sort_reservoir_ptr_offset_node_ptrs(reservoir **res_ptr);
int node_ptr_offset_compare(const void *off1, const void *off2);
void reservoir_sample_input(const char *in_fn, reservoir **res_ptr, int *res_idx);
reservoir * new_reservoir_ptr(int len);
offset_node * new_offset_node_ptr(off_t val);
void print_reservoir_ptr(reservoir *res_ptr);
void free_reservoir_ptr(reservoir **res_ptr);

#endif
