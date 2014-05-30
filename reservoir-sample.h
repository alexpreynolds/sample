#ifndef RESERVOIR_SAMPLE_H
#define RESERVOIR_SAMPLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <getopt.h>

#define RS_VERSION "1.0"
#define DEFAULT_OFFSET_VALUE -1
#define LINE_LENGTH_VALUE 65536

typedef int boolean;
extern const boolean kTrue;
extern const boolean kFalse;

const boolean kTrue = 1;
const boolean kFalse = 0;

typedef struct offset_node offset_node;
typedef struct reservoir reservoir;
typedef struct file_mmap file_mmap;

struct offset_node {
    off_t start_offset;
};

struct reservoir {
    int length;
    offset_node **off_node_ptrs;
};

struct file_mmap {
    int fd;
    struct stat s;
    int status;
    size_t size;
    char *map;
};

static const char *name = "reservoir-sample";
static const char *version = RS_VERSION;
static const char *authors = "Alex Reynolds";
static const char *usage = "\n" \
    "Usage: reservoir-sample --sample-size=n [--shuffle] [--mmap] <newline-delimited-file>\n" \
    "\n" \
    "  Performs reservoir sampling (http://dx.doi.org/10.1145/3147.3165) on very large input\n" \
    "  files that are delimited by newline characters. The approach used in this application\n" \
    "  reduces memory usage by storing a pool of byte offsets to the start of each line, instead\n" \
    "  of the line elements themselves.\n\n" \
    "  Process Flags:\n\n" \
    "  --sample-size=n | -k n      Number of samples to retrieve (n = positive integer)\n" \
    "  --shuffle                   Shuffle sample before printing to standard output (optional)\n" \
    "  --mmap                      Use memory mapping for input file  (optional)\n" \
    "  --help                      Show this usage message\n";

static struct reservoir_sample_client_global_args_t {
    boolean shuffle;
    boolean mmap;
    int k;
    char **filenames;
    int num_filenames;
} reservoir_sample_client_global_args;

static struct option reservoir_sample_client_long_options[] = {
    { "sample-size",	required_argument,	NULL,	'k' },
    { "shuffle",	no_argument,		NULL,	's' },
    { "mmap",	        no_argument,		NULL,	'm' },
    { "help",		no_argument,		NULL,	'h' },
    { NULL,		no_argument,		NULL,	 0  }
}; 

static const char *reservoir_sample_client_opt_string = "k:smh?";

file_mmap * new_file_mmap(const char *in_fn);
void free_file_mmap(file_mmap **mmap_ptr);
void print_sorted_reservoir_sample_via_cstdio(const char *in_fn, reservoir *res_ptr);
void print_sorted_reservoir_sample_via_mmap(const file_mmap *in_mmap, reservoir *res_ptr);
void sort_reservoir_ptr_offset_node_ptrs(reservoir **res_ptr);
int node_ptr_offset_compare(const void *off1, const void *off2);
void reservoir_sample_input_via_cstdio(const char *in_fn, reservoir **res_ptr);
void reservoir_sample_input_via_mmap(file_mmap *in_mmap, reservoir **res_ptr);
reservoir * new_reservoir_ptr(const int len);
offset_node * new_offset_node_ptr(const off_t val);
void print_reservoir_ptr(const reservoir *res_ptr);
void free_reservoir_ptr(reservoir **res_ptr);
void initialize_globals();
void parse_command_line_options(int argc, char **argv);
void print_usage(FILE *stream);

#endif
