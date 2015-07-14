#include "sample.h"
#include "mt19937.h"

int main(int argc, char** argv) 
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> main()\n");
#endif

    long k;
    offset_reservoir *offset_reservoir_ptr = NULL;
    char *in_filename = NULL;
    FILE *in_file_ptr = NULL;
    file_mmap *in_file_mmap_ptr = NULL;
    boolean preserve_output_order;
    boolean mmap_in_file;
    boolean cstdio_in_file;
    boolean hybrid_in_file;
    boolean sample_without_replacement;
    boolean sample_with_replacement;
    boolean sample_size_specified;
    int rng_seed_value;
    boolean rng_seed_specified;
    int lines_per_offset;

    parse_command_line_options(argc, argv);
    k = sample_global_args.k;
    in_filename = sample_global_args.filenames[0];
    mmap_in_file = sample_global_args.mmap;
    cstdio_in_file = sample_global_args.cstdio;
    hybrid_in_file = sample_global_args.hybrid;
    preserve_output_order = sample_global_args.preserve_order;
    sample_without_replacement = sample_global_args.sample_without_replacement;
    sample_with_replacement = sample_global_args.sample_with_replacement;
    sample_size_specified = sample_global_args.sample_size_specified;
    lines_per_offset = sample_global_args.lines_per_offset;
    rng_seed_value = sample_global_args.rng_seed_value;
    rng_seed_specified = sample_global_args.rng_seed_specified;

    /* seed the Twister random number generator */
    if (rng_seed_specified)
        mt19937_seed_rng(rng_seed_value);
    else
        mt19937_seed_rng(time(NULL));

    /* set up a blank reservoir pool */
    offset_reservoir_ptr = new_offset_reservoir_ptr(k);

    /* sample and shuffle offsets */
    if (sample_without_replacement) 
        {
            if ((hybrid_in_file) || (cstdio_in_file)) {
                in_file_ptr = new_file_ptr(in_filename);
                if (sample_size_specified)
                    sample_reservoir_offsets_without_replacement_via_cstdio_with_fixed_k(in_file_ptr, &offset_reservoir_ptr, lines_per_offset);
                else {
                    sample_reservoir_offsets_without_replacement_via_cstdio_with_unspecified_k(in_file_ptr, &offset_reservoir_ptr, lines_per_offset);
                    shuffle_reservoir_offsets_via_fisher_yates(&offset_reservoir_ptr);
                }
            }
            else if (mmap_in_file) {
                in_file_mmap_ptr = new_file_mmap(in_filename);
                if (sample_size_specified)
                    sample_reservoir_offsets_without_replacement_via_mmap_with_fixed_k(in_file_mmap_ptr, &offset_reservoir_ptr, lines_per_offset);
                else {
                    sample_reservoir_offsets_without_replacement_via_mmap_with_unspecified_k(in_file_mmap_ptr, &offset_reservoir_ptr, lines_per_offset);
                    shuffle_reservoir_offsets_via_fisher_yates(&offset_reservoir_ptr);
                }
            }
        }
    else if (sample_with_replacement) 
        {
            if ((hybrid_in_file) || (cstdio_in_file)) {
                in_file_ptr = new_file_ptr(in_filename);
                sample_reservoir_offsets_without_replacement_via_cstdio_with_fixed_k(in_file_ptr, &offset_reservoir_ptr, lines_per_offset);
                if (sample_size_specified)
                    sample_reservoir_offsets_with_replacement_via_cstdio_with_fixed_k(&offset_reservoir_ptr, k);
                else
                    sample_reservoir_offsets_with_replacement_via_cstdio_with_unspecified_k(&offset_reservoir_ptr);
            }
            else if (mmap_in_file) {
                in_file_mmap_ptr = new_file_mmap(in_filename);
                sample_reservoir_offsets_without_replacement_via_mmap_with_unspecified_k(in_file_mmap_ptr, &offset_reservoir_ptr, lines_per_offset);
                if (sample_size_specified)
                    sample_reservoir_offsets_with_replacement_via_mmap_with_fixed_k(&offset_reservoir_ptr, k);
                else
                    sample_reservoir_offsets_with_replacement_via_mmap_with_unspecified_k(&offset_reservoir_ptr);
            }
        }

#ifdef DEBUG
    /* print reservoir offsets */
    print_offset_reservoir_ptr(offset_reservoir_ptr);
#endif

    /* sort offsets, if needed */
    if (preserve_output_order) {
        sort_offset_reservoir_ptr_offsets(&offset_reservoir_ptr);
#ifdef DEBUG
        print_offset_reservoir_ptr(offset_reservoir_ptr);
#endif
    }

    /* print reservoir offset line references */
    if (hybrid_in_file)
        print_offset_reservoir_sample_via_mmap(in_file_mmap_ptr, offset_reservoir_ptr, lines_per_offset);    
    else if (cstdio_in_file) {
        if (preserve_output_order)
            print_sorted_offset_reservoir_sample_via_cstdio(in_file_ptr, offset_reservoir_ptr, lines_per_offset);
        else
            print_unsorted_offset_reservoir_sample_via_cstdio(in_file_ptr, offset_reservoir_ptr, lines_per_offset);
    }
    else if (mmap_in_file)
        print_offset_reservoir_sample_via_mmap(in_file_mmap_ptr, offset_reservoir_ptr, lines_per_offset);


    /* clean up */
    if (offset_reservoir_ptr)
        delete_offset_reservoir_ptr(&offset_reservoir_ptr);
    if (in_file_mmap_ptr)
        delete_file_mmap(&in_file_mmap_ptr);
    if (in_file_ptr)
        delete_file_ptr(&in_file_ptr);

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> main()\n");
#endif

    return EXIT_SUCCESS;
}

offset_reservoir * new_offset_reservoir_ptr(const long len)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> new_offset_reservoir_ptr()\n");
#endif

    offset_reservoir *res = NULL;
    off_t *offsets = NULL;
    
    offsets = malloc(sizeof(off_t) * len);
    if (!offsets) {
        fprintf(stderr, "Debug: offsets instance is NULL\n");
        exit(EXIT_FAILURE);
    }

    res = malloc(sizeof(offset_reservoir));
    if (!res) {
        fprintf(stderr, "Debug: offset_reservoir instance res is NULL\n");
        exit(EXIT_FAILURE);
    }

    res->num_offsets = len;
    res->offsets = offsets;

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> new_offset_reservoir_ptr()\n");
#endif

    return res;
}

void delete_offset_reservoir_ptr(offset_reservoir **res_ptr)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> delete_offset_reservoir_ptr()\n");
#endif

    if (!*res_ptr) {
        fprintf(stderr, "Error: Offset reservoir pointer instance is NULL\n");
        exit(EXIT_FAILURE);
    }

    if ((*res_ptr)->offsets) {
        free((*res_ptr)->offsets);
        (*res_ptr)->offsets = NULL;
        (*res_ptr)->num_offsets = 0;
    }

    free(*res_ptr);
    *res_ptr = NULL;

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> delete_offset_reservoir_ptr()\n");
#endif
}

void print_offset_reservoir_ptr(const offset_reservoir *res_ptr)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> print_offset_reservoir_ptr()\n");
#endif

    int idx;

    for (idx = 0; idx < res_ptr->num_offsets; ++idx)
        fprintf(stdout, "[%012d] %012lld\n", idx, (long long int) res_ptr->offsets[idx]);

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving --> print_offset_reservoir_ptr()\n");
#endif
}

void sample_reservoir_offsets_without_replacement_via_cstdio_with_fixed_k(FILE *in_file_ptr, offset_reservoir **res_ptr, const int lines_per_offset)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> sample_reservoir_offsets_without_replacement_via_cstdio_with_fixed_k()\n");
#endif

    char in_line[LINE_LENGTH_VALUE + 1];
    off_t start_offset = 0;
    off_t stop_offset = 0;
    long k = (*res_ptr)->num_offsets;
    double p_replacement = 0.0;
    unsigned long rand_idx = 0;
    long ln_idx = 0;
    long grp_idx = 0;

    in_line[LINE_LENGTH_VALUE] = '1';

    /* read offsets into reservoir, replacing random offsets when the line-grouping counter is greater than k */
    while (fgets(in_line, LINE_LENGTH_VALUE + 1, in_file_ptr)) 
        {
            if ((++ln_idx) % lines_per_offset)
                continue;

            if (grp_idx < k) {
#ifdef DEBUG
                fprintf(stderr, "Debug: Adding node at idx %012ld with offset %012lld\n", grp_idx, (long long int) start_offset);
#endif
                (*res_ptr)->offsets[grp_idx] = start_offset;
            }
            else {
                p_replacement = (double) k / (grp_idx + 1);
                //rand_idx = rand() % k;
                rand_idx = mt19937_generate_random_ulong() % k;
                if (p_replacement > mt19937_generate_random_double()) {
#ifdef DEBUG
                    fprintf(stderr, "Debug: Replacing random offset %012ld for line %012ld with probability %f\n", rand_idx, grp_idx, p_replacement);
#endif
                    (*res_ptr)->offsets[rand_idx] = start_offset;
                }
            }
            stop_offset = ftell(in_file_ptr);
#ifdef DEBUG
            fprintf(stderr, "Debug: [%012lld - %012lld]\n", (long long int) start_offset, (long long int) stop_offset);
#endif
            start_offset = stop_offset;
            grp_idx++;
        }

    /* for when there are fewer line-groupings than the sample size */
    if (grp_idx < k)
        (*res_ptr)->num_offsets = grp_idx;

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> sample_reservoir_offsets_without_replacement_via_cstdio_with_fixed_k()\n");
#endif
}

void sample_reservoir_offsets_with_replacement_via_cstdio_with_fixed_k(offset_reservoir **res_ptr, const int sample_size)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> sample_reservoir_offsets_with_replacement_via_cstdio_with_fixed_k()\n");
#endif

    sample_reservoir_offsets_with_replacement_with_fixed_k(res_ptr, sample_size);

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> sample_reservoir_offsets_with_replacement_via_cstdio_with_fixed_k()\n");
#endif
}

void sample_reservoir_offsets_with_replacement_via_cstdio_with_unspecified_k(offset_reservoir **res_ptr)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> sample_reservoir_offsets_with_replacement_via_cstdio_with_unspecified_k()\n");
#endif

    /* 
       we sample with replacement using the original reservoir's offset count 
       as the new reservoir's sample size (note that this temporarily doubles 
       the application's memory usage)
    */
    sample_reservoir_offsets_with_replacement_via_cstdio_with_fixed_k(res_ptr, (*res_ptr)->num_offsets);

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> sample_reservoir_offsets_with_replacement_via_cstdio_with_unspecified_k()\n");
#endif
}

void sample_reservoir_offsets_without_replacement_via_cstdio_with_unspecified_k(FILE *in_file_ptr, offset_reservoir **res_ptr, const int lines_per_offset)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> sample_reservoir_offsets_without_replacement_via_cstdio_with_unspecified_k()\n");
#endif
    
    char in_line[LINE_LENGTH_VALUE + 1];
    off_t start_offset = 0;
    off_t stop_offset = 0;
    long k = (*res_ptr)->num_offsets;
    long ln_idx = 0;
    long grp_idx = 0;
    off_t *resized_offsets = NULL;

    in_line[LINE_LENGTH_VALUE] = '1';

    /* read all offsets into reservoir, reallocating memory as needed */
    while (fgets(in_line, LINE_LENGTH_VALUE + 1, in_file_ptr)) 
        {
            if ((++ln_idx) % lines_per_offset)
                continue;

            if (grp_idx == k) 
                {
                    k += DEFAULT_SAMPLE_SIZE_INCREMENT;
                    resized_offsets = realloc((*res_ptr)->offsets, sizeof(off_t) * k);
                    if (!resized_offsets) {
                        fprintf(stderr, "Error: Could not allocate memory for resized offset array\n");
                        exit(EXIT_FAILURE);
                    }
                    (*res_ptr)->offsets = resized_offsets;
                }
            (*res_ptr)->offsets[grp_idx] = start_offset;
            stop_offset = ftell(in_file_ptr);
            start_offset = stop_offset;
            grp_idx++;
        }

    (*res_ptr)->num_offsets = grp_idx;

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> sample_reservoir_offsets_without_replacement_via_cstdio_with_unspecified_k()\n");
#endif
}

void sample_reservoir_offsets_without_replacement_via_mmap_with_fixed_k(file_mmap *in_mmap, offset_reservoir **res_ptr, const int lines_per_offset) 
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> sample_reservoir_offsets_without_replacement_via_mmap_with_fixed_k()\n");
#endif

    size_t offset_idx;
    off_t start_offset = 0;
    off_t stop_offset = 0;
    long k = (*res_ptr)->num_offsets;
    long ln_idx = 0;
    long grp_idx = 0;
    double p_replacement = 0.0;
    long rand_idx = 0;

    for (offset_idx = 0; offset_idx < in_mmap->size; ++offset_idx) 
        {
            if (in_mmap->map[offset_idx] == '\n') 
                {
                    if ((++ln_idx) % lines_per_offset)
                        continue;
                    
                    if (grp_idx < k) {
#ifdef DEBUG
                        fprintf(stderr, "Debug: Adding offset at idx %012ld with offset value %012lld\n", grp_idx, (long long int) start_offset);
#endif
                        (*res_ptr)->offsets[grp_idx] = start_offset;
                    }
                    else {
                        p_replacement = (double) k / (grp_idx + 1);
                        rand_idx = mt19937_generate_random_ulong() % k;
                        if (p_replacement > mt19937_generate_random_double())
                            (*res_ptr)->offsets[rand_idx] = start_offset;
                    }
                    stop_offset = offset_idx;
                    start_offset = stop_offset + 1;
                    grp_idx++;
                }
        }

    if (grp_idx < k)
        (*res_ptr)->num_offsets = grp_idx;

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> sample_reservoir_offsets_without_replacement_via_mmap_with_fixed_k()\n");
#endif
}

void sample_reservoir_offsets_without_replacement_via_mmap_with_unspecified_k(file_mmap *in_mmap, offset_reservoir **res_ptr, const int lines_per_offset) 
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> sample_reservoir_offsets_without_replacement_via_mmap_with_unspecified_k()\n");
#endif

    size_t offset_idx;
    off_t start_offset = 0;
    off_t stop_offset = 0;
    long k = (*res_ptr)->num_offsets;
    long ln_idx = 0;
    long grp_idx = 0;
    off_t *resized_offsets = NULL;

    for (offset_idx = 0; offset_idx < in_mmap->size; ++offset_idx) 
        {
            if (in_mmap->map[offset_idx] == '\n') 
                {
                    if ((++ln_idx) % lines_per_offset)
                        continue;
                    
                    if (grp_idx == k) 
                        {
                            k += DEFAULT_SAMPLE_SIZE_INCREMENT;
                            resized_offsets = realloc((*res_ptr)->offsets, sizeof(off_t) * k);
                            if (!resized_offsets) {
                                fprintf(stderr, "Error: Could not allocate memory for resized offset array\n");
                                exit(EXIT_FAILURE);
                            }
                            (*res_ptr)->offsets = resized_offsets;
                        }
                    (*res_ptr)->offsets[grp_idx] = start_offset;
                    stop_offset = offset_idx;
                    start_offset = stop_offset + 1;
                    grp_idx++;
                }
        }
    (*res_ptr)->num_offsets = grp_idx;

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> sample_reservoir_offsets_without_replacement_via_mmap_with_unspecified_k()\n");
#endif
}

void sample_reservoir_offsets_with_replacement_via_mmap_with_fixed_k(offset_reservoir **res_ptr, const int sample_size)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> sample_reservoir_offsets_with_replacement_via_mmap_with_fixed_k()\n");
#endif

    sample_reservoir_offsets_with_replacement_with_fixed_k(res_ptr, sample_size);

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> sample_reservoir_offsets_with_replacement_via_mmap_with_fixed_k()\n");
#endif
}

void sample_reservoir_offsets_with_replacement_via_mmap_with_unspecified_k(offset_reservoir **res_ptr)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> sample_reservoir_offsets_with_replacement_via_mmap_with_unspecified_k()\n");
#endif

    /* 
       we sample with replacement using the original reservoir's offset count 
       as the new reservoir's sample size (note that this temporarily doubles 
       the application's memory usage)
    */
    sample_reservoir_offsets_with_replacement_via_mmap_with_fixed_k(res_ptr, (*res_ptr)->num_offsets);

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> sample_reservoir_offsets_with_replacement_via_mmap_with_unspecified_k()\n");
#endif
}

void sample_reservoir_offsets_with_replacement_with_fixed_k(offset_reservoir **res_ptr, const int sample_size)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> sample_reservoir_offsets_with_replacement_with_fixed_k()\n");
#endif

    offset_reservoir *original_offset_reservoir_ptr = *res_ptr;
    offset_reservoir *sample_offset_reservoir_ptr = NULL;
    long original_sample_size = original_offset_reservoir_ptr->num_offsets;
    long sample_offset_idx = 0;
    long original_random_idx = 0;

    /* build a new reservoir of size k */
    sample_offset_reservoir_ptr = new_offset_reservoir_ptr(sample_size);

    /* 
       pick random integers between 0..(original_sample_size - 1) and 
       copy original offset values to sample reservoir's offsets array 
    */
    for (sample_offset_idx = 0; sample_offset_idx < sample_size - 1; ++sample_offset_idx) {
        original_random_idx = mt19937_generate_random_double() * original_sample_size;
        sample_offset_reservoir_ptr->offsets[sample_offset_idx] = original_offset_reservoir_ptr->offsets[original_random_idx];
    }

    /* clean up the original reservoir */
    delete_offset_reservoir_ptr(&original_offset_reservoir_ptr);
    original_offset_reservoir_ptr = NULL;

    /* point the current reservoir pointer at the new sample reservoir */
    *res_ptr = sample_offset_reservoir_ptr;

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> sample_reservoir_offsets_with_replacement_with_fixed_k()\n");
#endif
}

void shuffle_reservoir_offsets_via_fisher_yates(offset_reservoir **res_ptr)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> shuffle_reservoir_offsets_via_fisher_yates()\n");
#endif

    long ln_idx = (*res_ptr)->num_offsets;
    long shuf_idx = 0;
    long rand_idx = 0;
    off_t temp_offset = 0;
    
    /* cf. http://blog.codinghorror.com/the-danger-of-naivete/ for an interesting discussion about Fisher-Yates */
    for (shuf_idx = ln_idx - 1; shuf_idx > 0; --shuf_idx) {
        rand_idx = mt19937_generate_random_double() * (shuf_idx + 1);
        temp_offset = (*res_ptr)->offsets[shuf_idx];
        (*res_ptr)->offsets[shuf_idx] = (*res_ptr)->offsets[rand_idx];
        (*res_ptr)->offsets[rand_idx] = temp_offset;
    }

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> shuffle_reservoir_offsets_via_fisher_yates()\n");
#endif    
}

void sort_offset_reservoir_ptr_offsets(offset_reservoir **res_ptr) 
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> sort_offset_reservoir_ptr_offsets()\n");
#endif

    qsort( (*res_ptr)->offsets, (*res_ptr)->num_offsets, sizeof(off_t), offset_compare );

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> sort_offset_reservoir_ptr_offsets()\n");
#endif
}

int offset_compare(const void *off1, const void *off2)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> offset_compare()\n");
    fprintf(stderr, "Debug: Comparing: %012lld and %012lld\n", (long long int) *(off_t *)off1, (long long int) *(off_t *)off2);
#endif

    long long int off_diff = (long long int)((*(off_t *)off1) - (*(off_t *)off2));

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> offset_compare()\n");
#endif

    return (off_diff > 0) ? 1 : -1;
} 

void print_offset_reservoir_sample_via_mmap(const file_mmap *in_mmap, offset_reservoir *res_ptr, const int lines_per_offset)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> print_offset_reservoir_sample_via_mmap()\n");
#endif

    int res_idx;
    off_t mmap_idx;
    off_t current_offset;
    long ln_idx = 0;
    
    for (res_idx = 0; res_idx < res_ptr->num_offsets; ++res_idx) {
        current_offset = res_ptr->offsets[res_idx];
        for (mmap_idx = current_offset; mmap_idx < current_offset + LINE_LENGTH_VALUE; ++mmap_idx) {
            fprintf(stdout, "%c", in_mmap->map[mmap_idx]);
            if ((in_mmap->map[mmap_idx] == '\n') && ((++ln_idx) % lines_per_offset == 0))
                break;
        }
    }

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> print_offset_reservoir_sample_via_mmap()\n");
#endif
}

void print_sorted_offset_reservoir_sample_via_cstdio(FILE *in_file_ptr, offset_reservoir *res_ptr, const int lines_per_offset)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> print_sorted_offset_reservoir_sample_via_cstdio()\n");
#endif

    int idx;
    char in_line[LINE_LENGTH_VALUE + 1];
    char temp_line[LINE_LENGTH_VALUE + 1];
    off_t previous_offset = 0;
    size_t previous_line_length = 0;
    long ln_idx = 0;
    int temp_length = 0;

    /* position the file pointer at the start of the file */
    if (fseek(in_file_ptr, 0, SEEK_SET) == -1) {
        fprintf(stderr, "Error: Could not rewind input file pointer\n");
        exit(EXIT_FAILURE);
    }

    for (idx = 0; idx < res_ptr->num_offsets; ++idx) {
        /* 
           we use SEEK_CUR to jump from wherever the file pointer is now, instead of 
           from the start of the file -- we can do this because the offsets are in
           sorted order
        */
        fseek(in_file_ptr, res_ptr->offsets[idx] - previous_offset - previous_line_length, SEEK_CUR);
        for (ln_idx = 0; ln_idx < lines_per_offset; ++ln_idx) {
            fgets(temp_line, LINE_LENGTH_VALUE + 1, in_file_ptr);
            memcpy(in_line + temp_length, temp_line, strlen(temp_line));
            temp_length += strlen(temp_line);
        }
        in_line[temp_length] = '\0';
        fprintf(stdout, "%s", in_line);
        previous_line_length = strlen(in_line);
        previous_offset = res_ptr->offsets[idx];
        temp_length = 0;
    }

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> print_sorted_offset_reservoir_sample_via_cstdio()\n");
#endif
}

void print_unsorted_offset_reservoir_sample_via_cstdio(FILE *in_file_ptr, offset_reservoir *res_ptr, const int lines_per_offset)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> print_unsorted_offset_reservoir_sample_via_cstdio()\n");
#endif

    int idx;
    long ln_idx = 0;
    char in_line[LINE_LENGTH_VALUE + 1];
    char temp_line[LINE_LENGTH_VALUE + 1];
    int temp_length = 0;

    /* position the file pointer at the start of the file */
    if (fseek(in_file_ptr, 0, SEEK_SET) == -1) {
        fprintf(stderr, "Error: Could not rewind input file pointer\n");
        exit(EXIT_FAILURE);
    }

    for (idx = 0; idx < res_ptr->num_offsets; ++idx) {
        /* 
           we use SEEK_SET to jump from the start of the file, as the offsets are unsorted
        */
        fseek(in_file_ptr, res_ptr->offsets[idx], SEEK_SET);
        for (ln_idx = 0; ln_idx < lines_per_offset; ++ln_idx) {
            fgets(temp_line, LINE_LENGTH_VALUE + 1, in_file_ptr);
            memcpy(in_line + temp_length, temp_line, strlen(temp_line));
            temp_length += strlen(temp_line);
        }
        in_line[temp_length] = '\0';
        fprintf(stdout, "%s", in_line);
        temp_length = 0;
    }

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> print_unsorted_offset_reservoir_sample_via_cstdio()\n");
#endif
}

FILE * new_file_ptr(const char *in_fn)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> new_file_ptr()\n");
#endif

    FILE *file_ptr = NULL;
    boolean not_stdin = kTrue;

    not_stdin = strcmp(in_fn, "-");
    file_ptr = (not_stdin) ? fopen(in_fn, "r") : stdin;
    if (file_ptr == stdin) {
        fprintf(stderr, "Error: Stdin not yet supported with this application\n");
        exit(EXIT_FAILURE);
    }

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> new_file_ptr()\n");
#endif
    
    return file_ptr;
}

void delete_file_ptr(FILE **file_ptr)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> delete_file_ptr()\n");
#endif

    fclose(*file_ptr);
    *file_ptr = NULL;

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> delete_file_ptr()\n");
#endif
}

file_mmap * new_file_mmap(const char *in_fn)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> new_file_mmap()\n");
#endif
    
    file_mmap *mmap_ptr = NULL;
    boolean not_stdin = kTrue;

    mmap_ptr = malloc(sizeof(file_mmap));
    if (!mmap_ptr) {
        fprintf(stderr, "Error: Mmap pointer is NULL\n");
        exit(EXIT_FAILURE);
    }

    not_stdin = (strcmp(in_fn, "-")) ? kTrue : kFalse;
    
    if (!not_stdin) {
        fprintf(stderr, "Error: Stdin not yet supported with mmap setup function\n");
        exit(EXIT_FAILURE);
    }

    mmap_ptr->fn = NULL;
    mmap_ptr->fn = malloc(strlen(in_fn) + 1);
    if (!mmap_ptr->fn) {
        fprintf(stderr, "Error: Mmap pointer's filename pointer is NULL\n");
        exit(EXIT_FAILURE);
    }
    strncpy(mmap_ptr->fn, in_fn, strlen(in_fn) + 1);
    mmap_ptr->fd = open(mmap_ptr->fn, O_RDONLY);
    mmap_ptr->status = fstat(mmap_ptr->fd, &(mmap_ptr->s));
    mmap_ptr->size = mmap_ptr->s.st_size;
    mmap_ptr->map = (char *) mmap(NULL, 
                                  mmap_ptr->size, 
                                  PROT_READ, 
                                  MAP_SHARED,
                                  mmap_ptr->fd, 
                                  0);
    if (mmap_ptr->map == MAP_FAILED) {
        fprintf(stderr, "Error: Mmap pointer map failed\n");
        exit(EXIT_FAILURE);
    }

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> new_file_mmap()\n");
#endif

    return mmap_ptr;
}

void delete_file_mmap(file_mmap **mmap_ptr)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> delete_file_mmap()\n");
#endif

    close((*mmap_ptr)->fd);
    munmap((*mmap_ptr)->map, (*mmap_ptr)->size);
    free((*mmap_ptr)->fn);
    (*mmap_ptr)->fn = NULL;
    free(*mmap_ptr);
    *mmap_ptr = NULL;

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> delete_file_mmap()\n");
#endif
}

void initialize_globals()
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> initialize_globals()\n");
#endif

    sample_global_args.sample_size_specified = kFalse;
    sample_global_args.sample_without_replacement = kTrue;
    sample_global_args.sample_with_replacement = kFalse;
    sample_global_args.preserve_order = kFalse;
    sample_global_args.hybrid = kFalse;
    sample_global_args.mmap = kTrue;
    sample_global_args.cstdio = kFalse;
    sample_global_args.k = 0;
    sample_global_args.lines_per_offset = 1;
    sample_global_args.rng_seed_value = 1;
    sample_global_args.rng_seed_specified = kFalse;
    sample_global_args.filenames = NULL;
    sample_global_args.num_filenames = 0;

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> initialize_globals()\n");
#endif
}

void parse_command_line_options(int argc, char **argv)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> parse_command_line_options()\n");
#endif

    int client_long_index;
    int client_opt = getopt_long(argc, 
                                 argv, 
                                 sample_client_opt_string, 
                                 sample_client_long_options, 
                                 &client_long_index);

    int order_type_flags = 0;
    int sample_type_flags = 0;
    int io_type_flags = 0;
    int sample_size_flag = kFalse;

    opterr = 0; /* disable error reporting by GNU getopt */
    initialize_globals();

    while (client_opt != -1) 
        {
            switch (client_opt) 
                {
                case 'k':
		    if (optarg) {
			sample_global_args.k = atoi(optarg);
			sample_size_flag = kTrue;
			break;
		    }
		    else {
			fprintf(stderr, "Error: Sample size option is specified, but its value is unspecified\n");
			print_usage(stderr);
			exit(EXIT_FAILURE);
		    }
                case 'l':
		    if (optarg) {
			sample_global_args.lines_per_offset = atoi(optarg);
			break;
		    }
		    else {
			fprintf(stderr, "Error: Lines-per-offset option is specified, but its value is unspecified\n");
			print_usage(stderr);
			exit(EXIT_FAILURE);
		    }
                case 'o':
                    sample_global_args.sample_without_replacement = kTrue;
                    sample_type_flags++;
                    break;
                case 'r':
                    sample_global_args.sample_with_replacement = kTrue;
                    sample_global_args.sample_without_replacement = kFalse;
                    sample_type_flags++;
                    break;
                case 's':
                    sample_global_args.preserve_order = kFalse;
                    order_type_flags++;
                    break;
                case 'p':
                    sample_global_args.preserve_order = kTrue;
                    order_type_flags++;
                    break;
                case 'y':
                    sample_global_args.hybrid = kTrue;
                    sample_global_args.mmap = kFalse;
                    io_type_flags++;
                    break;
                case 'm':
                    sample_global_args.mmap = kTrue;
                    io_type_flags++;
                    break;
                case 'c':
                    sample_global_args.cstdio = kTrue;
                    sample_global_args.mmap = kFalse;
                    io_type_flags++;
                    break;
                case 'd':
		    if (optarg) {
			sample_global_args.rng_seed_value = atoi(optarg);
			sample_global_args.rng_seed_specified = kTrue;
			break;
		    }
		    else {
			fprintf(stderr, "Error: RNG seed initializer option is specified, but its value is unspecified\n");
			print_usage(stderr);
			exit(EXIT_FAILURE);
		    }
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
                                     sample_client_opt_string, 
                                     sample_client_long_options, 
                                     &client_long_index);
        }

    sample_global_args.filenames = argv + optind;
    sample_global_args.num_filenames = argc - optind;

    /* check input */

    if (!sample_size_flag)
        sample_global_args.k += DEFAULT_SAMPLE_SIZE_INCREMENT;
    else
        sample_global_args.sample_size_specified = kTrue;

    if ((order_type_flags > 1) ||
        (sample_type_flags > 1) ||
        (io_type_flags > 1) ||
        (sample_global_args.lines_per_offset < 1) ||
        (sample_global_args.k < 1) ||
        (sample_global_args.num_filenames != 1) ||
        (sample_global_args.rng_seed_value < 1))
        {
            print_usage(stderr);
            exit(EXIT_FAILURE);
        }

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> parse_command_line_options()\n");
#endif
}

void print_usage(FILE *stream)
{
#ifdef DEBUG
    fprintf(stderr, "Debug: Entering --> print_usage()\n");
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

#ifdef DEBUG
    fprintf(stderr, "Debug: Leaving  --> print_usage()\n");
#endif
}
