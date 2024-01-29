#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "debug.h"
#include "readpng.h"

static int load_png_data(png_structp *ppng, png_infop *pinfo, FILE *fp);
static bool file_exists(char *path)
{
    struct stat st;
    int ret;

    ret = stat(path, &st);
    if (ret)
        return false;

    return S_ISREG(st.st_mode);
}

void free_png(uint32_t **rgb)
{
    if (rgb)
    {
        free(rgb);
    }
}
int load_png(char *fname, uint32_t ***prgb, int32_t *width, int32_t *height)
{
    int ret = -1;

    ret = load_png_with_alpha(fname, prgb, width, height);
    // error_return:
    return ret;
}
int load_png_with_alpha(char *fname, uint32_t ***prgb, int32_t *pwidth, int32_t *pheight)
{
    int ret = -1;
    int i, j;
    FILE *fp = NULL;
    int32_t width, height;
    png_structp png = NULL;
    png_infop info = NULL;
    png_bytepp data = NULL;
    //png_byte type;

    ERRRET(!prgb || !pwidth || !pheight, "output pointer is null.");
    ERRRET(!fname || !(*fname), "no file name.");

    ERRRET(!file_exists(fname), "file not exists.");

    fp = fopen(fname, "rb");
    ERRRET(!fp, "file open error");

    ret = load_png_data(&png, &info, fp);
    ERRRET(ret, "load_png_data failed.");

    width = png_get_image_width(png, info);
    height = png_get_image_height(png, info);
    data = png_get_rows(png, info);
    // type = png_get_color_type(png, info);

    // d("%dx%d d:%p t:%x", width, height, data, type);
    *prgb = calloc(sizeof(uint32_t *) * width + sizeof(uint32_t) * width * height, 1);
    for (i = 0; i < height; i++)
    {
        (*prgb)[i] = (uint32_t *)(((uint8_t *)(*prgb)) + sizeof(uint32_t *) * width + sizeof(uint32_t) * width * i);
    }

    *pwidth = width;
    *pheight = height;

    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            (*prgb)[i][j] = (data[i][j * 3 + 0] << 16) | (data[i][j * 3 + 1] << 8) | (data[i][j * 3 + 2] << 0);
        }
    }
    ret = 0;
error_return:
    if (fp)
        fclose(fp);
    return ret;
}

static bool is_png(FILE *fp)
{
    bool ret = false;
    png_byte header[8];
    fseek(fp, 0, SEEK_SET);

    ERRRET(fread(header, 8, 1, fp) < 1, "file read error");
    ret = png_sig_cmp(header, 0, 8) == 0;

error_return:
    return ret;
}

static int load_png_data(png_structp *ppng, png_infop *pinfo, FILE *fp)
{
    int ret = -1;
    png_structp png;
    png_infop info;

    ERRRET(!is_png(fp), "");

    png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    ERRRET(!png, "png_create_read_struct failed.");

    info = png_create_info_struct(png);
    ERRRET(!info, "png_create_info_struct failed.");

    png_init_io(png, fp);
    fseek(fp, 0, SEEK_SET);

    png_read_png(png, info, PNG_TRANSFORM_PACKING | PNG_TRANSFORM_STRIP_16, NULL);

    *ppng = png;
    *pinfo = info;
    ret = 0;

error_return:
    return ret;
}
