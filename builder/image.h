#ifndef IMAGE_H
#define IMAGE_H

#define MAX_SECTIONS 8

struct Section_t {
    int use;
    int size;
    bool virt;
    Buffer_t *buf_data;
    Buffer_t *buf_plan;
    uint8_t *data;          // if no data, then virtual
    uint8_t *plan;
};

struct sqlite3;

struct Image_t {
    Section_t sections[1 + MAX_SECTIONS];
    int numsections;
    sqlite3 *db;
};

Section_t *IMG_getsection(Image_t *img, int secid);

#define ST_NONE     0
#define ST_TEXT     1
#define ST_DATA     2
#define ST_BSS      3

#define SF_VIRTUAL  1
#define SF_16BIT    2

#endif
