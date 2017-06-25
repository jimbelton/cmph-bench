#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "cmph.h"

static uint64_t GetTimeStamp(void)
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec * (uint64_t)1000000 + tv.tv_usec;
}

// Create minimal perfect hash function from in-memory vector
int main(int argc, char **argv)
{
    CMPH_ALGO algo = CMPH_COUNT;

    if (argc > 1) {
        if      (strcmp(argv[1], "bdz"   ) == 0) algo = CMPH_BDZ;
        else if (strcmp(argv[1], "brz-ph") == 0) algo = CMPH_BDZ_PH;
        else if (strcmp(argv[1], "bmz"   ) == 0) algo = CMPH_BMZ;
        else if (strcmp(argv[1], "bmz8"  ) == 0) algo = CMPH_BMZ8;
        else if (strcmp(argv[1], "brz"   ) == 0) algo = CMPH_BRZ;
        else if (strcmp(argv[1], "chd"   ) == 0) algo = CMPH_CHD;
        else if (strcmp(argv[1], "chd-ph") == 0) algo = CMPH_CHD_PH;
        else if (strcmp(argv[1], "chm"   ) == 0) algo = CMPH_CHM;
        else if (strcmp(argv[1], "fch"   ) == 0) algo = CMPH_FCH;
    }

    if (algo == CMPH_COUNT) {
        fprintf(stderr, "usage: cmph-bench (bdz|bdz-ph|bmz|bmz8|brz|chd|chd-ph|chm|fch)\n");
        exit(1);
    }

    // Use the corpus-words as keys
    FILE * corpusStream;
    assert(corpusStream = fopen("corpus-words", "r"));
    cmph_io_adapter_t *source = cmph_io_nlfile_adapter(corpusStream);

    // Creating a filled vector
    FILE *   mphf_fd = fopen("temp.mph", "w");
    uint64_t start   = GetTimeStamp();

    //Create minimal perfect hash function using the brz algorithm.
    cmph_config_t *config = cmph_config_new(source);
    cmph_config_set_algo(config, CMPH_BRZ);
    cmph_config_set_mphf_fd(config, mphf_fd);
    cmph_t *hash = cmph_new(config);
    cmph_config_destroy(config);
    cmph_dump(hash, mphf_fd);
    cmph_destroy(hash);
    fclose(mphf_fd);

    uint64_t elapsed = GetTimeStamp() - start;
    printf("Constructed MPH in %lu.%lu seconds\n", elapsed / 1000000, elapsed % 1000000);

    //Find key
    mphf_fd = fopen("temp.mph", "r");
    start   = GetTimeStamp();
    hash    = cmph_load(mphf_fd);
    elapsed = GetTimeStamp() - start;
    printf("Loaded MPH in %lu.%lu seconds\n", elapsed / 1000000, elapsed % 1000000);

    // Slurp in corpus
    struct stat corpusStat;
    assert(stat("corpus-words", &corpusStat) == 0);
    char * corpus;
    assert(corpus = malloc(corpusStat.st_size));
    assert(corpusStream = fopen("corpus-words", "r"));
    assert(fread(corpus, 1, corpusStat.st_size, corpusStream) == corpusStat.st_size);

    start = GetTimeStamp();
    unsigned key;

    for (key = 0; corpusStat.st_size; key++) {
        char * end;
        assert(end = strchr(corpus, '\n'));    // Corpus must end in a '\n'
        *end = '\0';
        unsigned int id = cmph_search(hash, corpus, (cmph_uint32)(end - corpus));
        //fprintf(stderr, "key:%s -- hash:%u\n", corpus, id);
        corpusStat.st_size -= end - corpus + 1;
        corpus              = end + 1;
    }

    elapsed = GetTimeStamp() - start;
    printf("Looked up %u keys in MPH in %lu.%lu seconds\n", key, elapsed / 1000000, elapsed % 1000000);

    //Destroy hash
    cmph_destroy(hash);
    cmph_io_vector_adapter_destroy(source);
    fclose(mphf_fd);
    return 0;
}
