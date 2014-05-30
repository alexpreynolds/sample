#include "reservoir-sample.h"

const Boolean kTrue = 1;
const Boolean kFalse = 0;

struct offset_node {
    off_t start_offset;
};

struct reservoir {
    int length;
    offset_node **off_node_ptrs;
};

int main(int argc, char** argv) 
{
    int k = -1;
    reservoir *reservoir_ptr = NULL;
    char *in_filename = NULL;
    int in_file_line_idx = 0;

    parse_command_line_options(argc, argv);
    k = reservoir_sample_client_global_args.k;
    in_filename = reservoir_sample_client_global_args.filenames[0];

    if ((k == 0) || (!in_filename)) {
        print_usage(stderr);
        return EXIT_FAILURE;
    }

    reservoir_ptr = new_reservoir_ptr(k);
    reservoir_sample_input(in_filename, &reservoir_ptr, &in_file_line_idx);
#ifdef DEBUG
    print_reservoir_ptr(reservoir_ptr);
#endif
    sort_reservoir_ptr_offset_node_ptrs(&reservoir_ptr);
#ifdef DEBUG
    print_reservoir_ptr(reservoir_ptr);
#endif
    print_sorted_reservoir_sample(in_filename, reservoir_ptr);
    free_reservoir_ptr(&reservoir_ptr);

    return EXIT_SUCCESS;
}

void print_sorted_reservoir_sample(const char *in_fn, reservoir *res_ptr)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: print_reservoir_sample()\n");
#endif

    int not_stdin = 0;
    FILE *in_file_ptr = NULL;
    int idx;
    char in_line[LINE_LENGTH_VALUE + 1];
    off_t previous_offset = 0;
    size_t previous_line_length = 0;

    not_stdin = strcmp(in_fn, "-");
    in_file_ptr = (not_stdin) ? fopen(in_fn, "r") : stdin;
    
    if (in_file_ptr == stdin) {
        fprintf(stderr, "Error: Stdin not yet supported with this function\n");
        exit(EXIT_FAILURE);
    }

    for (idx = 0; idx < res_ptr->length; ++idx) {
        /* we use SEEK_CUR to jump from wherever the file pointer is now, instead of from the start of the file */
        fseek(in_file_ptr, res_ptr->off_node_ptrs[idx]->start_offset - previous_offset - previous_line_length, SEEK_CUR);
        fgets(in_line, LINE_LENGTH_VALUE + 1, in_file_ptr);
        fprintf(stdout, "%s", in_line);
        previous_line_length = strlen(in_line);
        previous_offset = res_ptr->off_node_ptrs[idx]->start_offset;
    }

    fclose(in_file_ptr);
}

void sort_reservoir_ptr_offset_node_ptrs(reservoir **res_ptr) 
{
#ifdef DEBUG
    fprintf(stderr, "Debug: sort_reservoir_ptr_offset_node_ptrs()\n");
#endif
    qsort( (*res_ptr)->off_node_ptrs, (*res_ptr)->length, sizeof(offset_node *), node_ptr_offset_compare );
}

int node_ptr_offset_compare(const void *node1, const void *node2)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: node_ptr_offset_compare()\n");
    fprintf(stderr, "Debug: Comparing: %012lld and %012lld\n", (*(offset_node **)node1)->start_offset, (*(offset_node **)node2)->start_offset);
#endif
    int off_diff = (*(offset_node **)node1)->start_offset - (*(offset_node **)node2)->start_offset;
    return (off_diff > 0) ? 1 : -1;
} 

void reservoir_sample_input(const char *in_fn, reservoir **res_ptr, int *ln_idx)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: reservoir_sample_input_offsets()\n");
#endif

    int not_stdin = 0;
    FILE *in_file_ptr = NULL;
    char in_line[LINE_LENGTH_VALUE + 1];
    off_t start_offset = 0;
    off_t stop_offset = 0;
    int k = (*res_ptr)->length;
    double p_replacement = 0.0;
    int rand_node_idx = 0;

    not_stdin = strcmp(in_fn, "-");
    in_file_ptr = (not_stdin) ? fopen(in_fn, "r") : stdin;

    if (in_file_ptr == stdin) {
        fprintf(stderr, "Error: Stdin not yet supported with this function\n");
        exit(EXIT_FAILURE);
    }

    in_line[LINE_LENGTH_VALUE] = '1';
    while (fgets(in_line, LINE_LENGTH_VALUE + 1, in_file_ptr)) 
        {
            if (*ln_idx < k) {
#ifdef DEBUG
                fprintf(stderr, "Debug: Adding node at idx %012d with offset %012lld\n", *ln_idx, start_offset);
#endif
                (*res_ptr)->off_node_ptrs[*ln_idx] = new_offset_node_ptr(start_offset);
            }
            else {
                p_replacement = (double)k/((*ln_idx) + 1);
                rand_node_idx = arc4random() % k;
                if (p_replacement > ((double)arc4random()/RAND_MAX)) {
#ifdef DEBUG
                    fprintf(stderr, "Debug: Replacing random node %012d for line %012d with probability %f\n", rand_node_idx, *ln_idx, p_replacement);
#endif
                    (*res_ptr)->off_node_ptrs[rand_node_idx]->start_offset = start_offset;
                }
            }
            stop_offset = ftell(in_file_ptr);
#ifdef DEBUG
            fprintf(stderr, "Debug: [%012lld - %012lld]\n", start_offset, stop_offset);
#endif
            start_offset = stop_offset;
            (*ln_idx)++;
        }

    /* for when there are fewer lines than the sample size */
    if (*ln_idx < k)
        (*res_ptr)->length = *ln_idx;

    fclose(in_file_ptr);
}

reservoir * new_reservoir_ptr(int len)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: new_reservoir_ptr()\n");
#endif

    int idx;
    reservoir *res = NULL;
    offset_node **off_node_ptrs = NULL;
    
    off_node_ptrs = malloc(sizeof(offset_node **) * len);

    for (idx = 0; idx < len; ++idx)
        off_node_ptrs[idx] = NULL;

    res = malloc(sizeof(reservoir));
    res->length = len;
    res->off_node_ptrs = off_node_ptrs;

    return res;
}

offset_node * new_offset_node_ptr(off_t val) 
{
    offset_node *node_ptr = NULL;

    node_ptr = malloc(sizeof(offset_node *));
    if (!node_ptr) {
        fprintf(stderr, "Error: Node pointer is NULL\n");
        exit(EXIT_FAILURE);
    }
    node_ptr->start_offset = val;

    return node_ptr;
}

void print_reservoir_ptr(reservoir *res_ptr)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: print_reservoir_ptr()\n");
#endif

    int idx;

    for (idx = 0; idx < res_ptr->length; ++idx)
        fprintf(stdout, "[%012d] %012lld\n", idx, res_ptr->off_node_ptrs[idx]->start_offset);
}

void free_reservoir_ptr(reservoir **res_ptr)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: free_reservoir_ptr()\n");
#endif

    int idx; 
    int len = (*res_ptr)->length;

    if (!*res_ptr) {
        fprintf(stderr, "Error: Reservoir pointer is NULL\n");
        exit(EXIT_FAILURE);
    }

    if ((*res_ptr)->off_node_ptrs) {
        for (idx = 0; idx < len; ++idx) {
            if ((*res_ptr)->off_node_ptrs[idx]) {
                free((*res_ptr)->off_node_ptrs[idx]);
                (*res_ptr)->off_node_ptrs[idx] = NULL;
            }
        }
        free((*res_ptr)->off_node_ptrs);
        (*res_ptr)->off_node_ptrs = NULL;
    }

    free(*res_ptr);
    *res_ptr = NULL;
}

void initialize_globals()
{
    reservoir_sample_client_global_args.shuffle = kFalse;
    reservoir_sample_client_global_args.k = 0;
    reservoir_sample_client_global_args.filenames = NULL;
    reservoir_sample_client_global_args.num_filenames = 0;
}

void parse_command_line_options(int argc, char **argv)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: parse_command_line_options()\n");
#endif

    int client_long_index;
    int client_opt = getopt_long(argc, 
                                 argv, 
                                 reservoir_sample_client_opt_string, 
                                 reservoir_sample_client_long_options, 
                                 &client_long_index);

    opterr = 0; /* disable error reporting by GNU getopt -- we handle this */
    initialize_globals();
    
    while (client_opt != -1) 
        {
            switch (client_opt) 
                {
                case 'k':
                    reservoir_sample_client_global_args.k = atoi(optarg);
                    break;
                case 's':
                    reservoir_sample_client_global_args.shuffle = kTrue;
                    break;
                case 'h':
                    print_usage(stdout);
                    exit(EXIT_SUCCESS);
                case '?':
                    print_usage(stderr);
                    exit(EXIT_FAILURE);
                default:
                    break;
                }
            client_opt = getopt_long(argc, 
                                     argv, 
                                     reservoir_sample_client_opt_string, 
                                     reservoir_sample_client_long_options, 
                                     &client_long_index);
        }

    reservoir_sample_client_global_args.filenames = argv + optind;
    reservoir_sample_client_global_args.num_filenames = argc - optind;

    /* check input */

    if ((reservoir_sample_client_global_args.k < 0) ||
        (reservoir_sample_client_global_args.num_filenames != 1))
        {
            print_usage(stderr);
            exit(EXIT_FAILURE);
        }
}

void print_usage(FILE *stream)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: print_usage()\n");
#endif

    fprintf(stream, 
            "%s\n" \
            "  version: %s\n" \
            "  author:  %s\n" \
            "%s\n", 
            name, 
            version,
            authors,
            usage);
}
