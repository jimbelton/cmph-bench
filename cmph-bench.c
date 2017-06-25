#include <assert.h>
#include <malloc.h>
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

static size_t GetMemAlloced(void)
{
    struct mallinfo mallInfo;
    mallInfo = mallinfo();
    return mallInfo.uordblks;    /* Total allocated space (bytes) */
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

    // Open the dump file and get the time and memory stamps
    FILE *   mphf_fd = fopen("temp.mph", "w");
    size_t   memBase = GetMemAlloced();
    uint64_t start   = GetTimeStamp();

    //Create minimal perfect hash function using the chosen algorithm.
    cmph_config_t *config = cmph_config_new(source);
    cmph_config_set_algo(config, CMPH_BRZ);
    cmph_config_set_mphf_fd(config, mphf_fd);
    cmph_t *hash = cmph_new(config);
    cmph_config_destroy(config);
    //cmph_io_vector_adapter_destroy(source);
    uint64_t elapsed = GetTimeStamp()  - start;
    size_t   memUsed = GetMemAlloced() - memBase;
    printf("Constructed MPH in %lu.%06lu seconds, memory %lu\n", elapsed / 1000000, elapsed % 1000000, memUsed);

    // Save the hash
    cmph_dump(hash, mphf_fd);
    cmph_destroy(hash);
    fclose(mphf_fd);

    // Benchmark the loader
    mphf_fd = fopen("temp.mph", "r");
    start   = GetTimeStamp();
    memBase = GetMemAlloced();
    hash    = cmph_load(mphf_fd);
    fclose(mphf_fd);
    elapsed = GetTimeStamp()  - start;
    memUsed = GetMemAlloced() - memBase;
    printf("Loaded MPH in %lu.%06lu seconds, memory %lu\n", elapsed / 1000000, elapsed % 1000000, memUsed);

    // Slurp in corpus
    struct stat corpusStat;
    assert(stat("corpus-words", &corpusStat) == 0);
    char * corpus;
    assert(corpus = malloc(corpusStat.st_size));
    assert(corpusStream = fopen("corpus-words", "r"));
    assert(fread(corpus, 1, corpusStat.st_size, corpusStream) == corpusStat.st_size);

    start   = GetTimeStamp();
    memUsed = corpusStat.st_size;
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

    elapsed = GetTimeStamp()  - start;
    printf("Looked up %u keys in MPH in %lu.%06lu seconds, corpus size %lu\n",
           key, elapsed / 1000000, elapsed % 1000000, memUsed);

    //Destroy hash
    cmph_destroy(hash);
    cmph_io_vector_adapter_destroy(source);    // Must be here. Moving it up to where it's commented out makes exit hang
    return 0;
}
