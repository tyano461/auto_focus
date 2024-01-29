#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <math.h>
#include "debug.h"
#include "readpng.h"

#define FRAME_SIZE 1024
#define MAX_PATH 256

static void usage(void);
void realfft(int n, double **a);
bool is_valid_arg(int argc, char *argv[]);
bool has_sequential_number(const char *filename);
bool sequential_file(char *fpath, int seq, const char *template);
bool next_file(char *next, char *fpath);
static inline double elapsed(struct timespec *st, struct timespec *en);
static inline double luminosity(uint8_t r, uint8_t g, uint8_t b);
static void tobmp(double **gray, int size, const char *fname);
static void tocsv(double **gray, int size, const char *fname, int count);
static void to8(double **gray, int size);

double **memory_allocate_2d_double(int size)
{
    int i;
    double **p = malloc(sizeof(double *) * size + sizeof(double) * size * size);
    if (p)
    {
        for (i = 0; i < size; i++)
        {
            p[i] = (double *)(((char *)p) + sizeof(double *) * size + sizeof(double) * size * i);
        }
    }
    return p;
}

int main(int argc, char *argv[])
{
    char fpath[MAX_PATH];
    char next_fpath[MAX_PATH];
    char gray_file[MAX_PATH];
    char fft_file[MAX_PATH];
    char csv_file[MAX_PATH];
    int32_t i, j, k, l, w, h;
    uint32_t **rgb;
    uint8_t r, g, b;
    double **data;
    // struct timespec start_time, end_time;
    int count;

    ERRRETf(!is_valid_arg(argc, argv), usage());

    if (!sequential_file(fpath, 0, argv[1]))
    {
        ERRRET(!sequential_file(fpath, 1, argv[1]), " start file not exists");
    }

    data = memory_allocate_2d_double(FRAME_SIZE);
    ERRRETnp(!data);

    count = 0;
    strcpy(next_fpath, fpath);
    unlink("data/data.csv");
    do
    {
        d("%s", next_fpath);
        load_png(next_fpath, &rgb, &w, &h);
        for (i = 0; i < h; i += FRAME_SIZE)
        {
            for (j = 0; j < w; j += FRAME_SIZE)
            {
                // copy
                for (k = i; k < (i + FRAME_SIZE); k++)
                {
                    for (l = j; l < (j + FRAME_SIZE); l++)
                    {
                        if (k < h && l < w)
                        {
                            r = ((uint8_t *)&rgb[k][l])[0];
                            g = ((uint8_t *)&rgb[k][l])[1];
                            b = ((uint8_t *)&rgb[k][l])[2];
                            data[k - i][l - j] = luminosity(r, g, b);
                        }
                        else
                        {
                            data[k - i][l - j] = 0;
                        }
                    }
                }
                // clock_gettime(CLOCK_REALTIME, &start_time);
                sprintf(gray_file, "data/gray_%03d.bmp", count);
                tobmp(data, 1024, gray_file);
                realfft(FRAME_SIZE, data);
                to8(data, 1024);
                // clock_gettime(CLOCK_REALTIME, &end_time);
                // d("elapsed:%f", elapsed(&start_time, &end_time));
                sprintf(fft_file, "data/fft_%03d.bmp", count);
                tobmp(data, 1024, fft_file);
                sprintf(csv_file, "data/data.csv");
                tocsv(data, 1024, csv_file, count);
            }
        }
        free_png(rgb);
        strcpy(fpath, next_fpath);
        count++;
    } while (next_file(next_fpath, fpath));

error_return:
    return 0;
}

static inline double elapsed(struct timespec *st, struct timespec *en)
{
    return (en->tv_sec - st->tv_sec) + ((en->tv_nsec - st->tv_nsec) / (1000 * 1000 * 1000));
}

static void get_prog(char *prog)
{
    char path[MAX_PATH * 2];
    pid_t pid;
    char *separator;
    int i, len, offset;

    pid = getpid();
    sprintf(path, "/proc/%d/exe", pid);
    readlink(path, prog, MAX_PATH);

    separator = strrchr(prog, '/');
    if (separator)
    {
        len = strlen(prog);
        offset = separator - prog;
        for (i = offset; i < len; i++)
        {
            prog[i - offset] = prog[i];
        }
        prog[len - offset] = '\0';
    }
}

static void usage(void)
{
    const char *msg = R"(
usage:
    %s [image file]
)";
    char prog[MAX_PATH];

    get_prog((char *)prog);
    printf(msg, prog);
}
bool file_exists(char *fpath)
{
    bool ret = false;
    struct stat st;

    ERRRETnp(!fpath || !(*fpath));
    ERRRETnp(stat(fpath, &st));
    ret = S_ISREG(st.st_mode);

error_return:
    return ret;
}
bool is_valid_arg(int argc, char *argv[])
{
    bool ret = false;

    ERRRET(argc < 2, "argument error");
    ERRRET(!file_exists(argv[1]), "file not found %s", argv[1]);

    ret = has_sequential_number(argv[1]);

error_return:
    return ret;
}

static inline bool is_digit(char a)
{
    return '0' <= a && a <= '9';
}

bool has_sequential_number(const char *filename)
{
    size_t len = strlen(filename);
    const char *suffix;
    const char *p;
    bool ret = false;

    ERRRETnp(len < 2);
    suffix = strrchr(filename, '.');
    ERRRETnp(!suffix || filename == suffix);

    for (p = (suffix - 1); p > filename; p--)
    {
        if (!is_digit(*p))
            break;
    }

    p++;

    ret = p != suffix;

error_return:
    return ret;
}

bool sequential_file(char *fpath, int seq, const char *template)
{
    char *suffix;

    strcpy(fpath, template);

    suffix = strrchr(fpath, '.');
    *(--suffix) = '0' + seq;
    while (is_digit(*(--suffix)))
    {
        *suffix = '0';
    }

    return file_exists(fpath);
}

bool next_file(char *next, char *fpath)
{
    char *suffix;
    bool ret = false;

    ERRRETnp(!next || !fpath || !(*fpath));
    strcpy(next, fpath);

    suffix = strrchr(next, '.');
    ERRRETnp(!suffix);
    while (suffix > next)
    {
        suffix--;
        *suffix += 1;
        if (is_digit(*suffix))
            break;
        else
            *suffix = '0';
    }
    ERRRETnp(!strcmp(fpath, next));

    ret = file_exists(next);

error_return:
    return ret;
}

static inline double luminosity(uint8_t r, uint8_t g, uint8_t b)
{
    return (0.2126 * r + 0.7152 * g + 0.0722 * b);
}

static inline void write1(FILE *fp, uint8_t v)
{
    uint8_t a = v;
    fwrite(&a, 1, 1, fp);
}

static inline void write2(FILE *fp, uint16_t v)
{
    uint16_t a = v;
    fwrite(&a, 2, 1, fp);
}

static inline void write4(FILE *fp, uint32_t v)
{
    uint32_t a = v;
    fwrite(&a, 4, 1, fp);
}

static void tobmp(double **gray, int size, const char *fname)
{

    int g, i, j;
    uint32_t parette;
    FILE *fp;

    fp = fopen(fname, "wb");
    fwrite("BM", 2, 1, fp);
    write4(fp, size * size + 54 + 4 * 256);
    write4(fp, 0);
    write4(fp, 54 + 4 * 256);
    write4(fp, 40);
    write4(fp, size);
    write4(fp, size);
    write2(fp, 1);
    write2(fp, 8);
    write4(fp, 0);
    write4(fp, size * size);
    write4(fp, 3780);
    write4(fp, 3780);
    write4(fp, 256);
    write4(fp, 256);
    for (i = 0; i < 256; i++)
    {
        parette = (i << 16) | (i << 8) | i;
        write4(fp, parette);
    }

    for (i = 0; i < size; i++)
    {
        for (j = 0; j < size; j++)
        {
            g = (int)gray[size - i - 1][j];
            write1(fp, (uint8_t)(g & 0xff));
        }
        for (j = 0; j < (size % 4); j++)
        {
            write1(fp, 0);
        }
    }
    fclose(fp);
}

static void tocsv(double **gray, int size, const char *fname, int count)
{
    FILE *fp;
    int i, j;
    double maxval, minval;
    int dist[256] = {0};
    int64_t sum = 0;

    fp = fopen(fname, "a");

    maxval = 0;
    minval = 99999;
    for (i = 0; i < size; i++)
    {
        for (j = 0; j < size; j++)
        {
            if (maxval < gray[i][j])
            {
                maxval = gray[i][j];
            }
            if (minval > gray[i][j])
            {
                minval = gray[i][j];
            }
            dist[(int)gray[i][j]]++;
        }
    }

    // fprintf(fp, "max:%f min:%f\n", maxval, minval);
    for (i = 0; i < 256; i++)
    {
        sum += (dist[i] * i);
    }
    fprintf(fp, "%3d: %8ld\n", count, sum);
    fclose(fp);
}

static void to8(double **gray, int size)
{
    int i, j;
    double orig;
    for (i = 0; i < size; i++)
    {
        for (j = 0; j < size; j++)
        {
            orig = abs(gray[i][j]);
            if (orig < 0.001)
            {
                gray[i][j] = 0;
            }
            else
            {
                gray[i][j] = 10 * log(abs(gray[i][j]));
                gray[i][j] = gray[i][j] / 2 + 128;
            }
        }
    }
}
