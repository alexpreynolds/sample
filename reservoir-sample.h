#ifndef RESERVOIR_SAMPLE_H
#define RESERVOIR_SAMPLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <getopt.h>

#define RS_VERSION "1.0"
#define DEFAULT_OFFSET_VALUE -1
#define LINE_LENGTH_VALUE 65536

typedef int Boolean;
extern const Boolean kTrue;
extern const Boolean kFalse;

typedef struct offset_node offset_node;
typedef struct reservoir reservoir;

static const char *name = "reservoir-sample";
static const char *version = RS_VERSION;
static const char *authors = "Alex Reynolds";
static const char *usage = "\n" \
    "Usage: reservoir-sample --sample-size=n [--shuffle] <newline-delimited-file>\n" \
    "\n" \
    "  Performs reservoir sampling (http://dx.doi.org/10.1145/3147.3165) on very large input\n" \
    "  files that are delimited by newline characters. The approach used in this application\n" \
    "  reduces memory usage by storing a pool of byte offsets to the start of each line, instead\n" \
    "  of the line elements themselves.\n\n" \
    "  Process Flags:\n\n" \
    "  --sample-size=n | -k n      Number of samples to retrieve (n = positive integer)\n" \
    "  --shuffle                   Shuffle sample before printing to standard output (optional)\n" \
    "  --help                      Show this usage message\n";

static struct reservoir_sample_client_global_args_t {
    Boolean shuffle;
    int k;
    char **filenames;
    int num_filenames;
} reservoir_sample_client_global_args;

static struct option reservoir_sample_client_long_options[] = {
    { "sample-size",	required_argument,	NULL,	'k' },
    { "shuffle",	no_argument,		NULL,	's' },
    { "help",		no_argument,		NULL,	'h' },
    { NULL,		no_argument,		NULL,	 0  }
}; 

static const char *reservoir_sample_client_opt_string = "k:sh?";

void print_sorted_reservoir_sample(const char *in_fn, reservoir *res_ptr);
void sort_reservoir_ptr_offset_node_ptrs(reservoir **res_ptr);
int node_ptr_offset_compare(const void *off1, const void *off2);
void reservoir_sample_input(const char *in_fn, reservoir **res_ptr, int *res_idx);
reservoir * new_reservoir_ptr(int len);
offset_node * new_offset_node_ptr(off_t val);
void print_reservoir_ptr(reservoir *res_ptr);
void free_reservoir_ptr(reservoir **res_ptr);
void initialize_globals();
void parse_command_line_options(int argc, char **argv);
void print_usage(FILE *stream);

#endif
