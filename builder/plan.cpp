#include <cassert>
#include <cstdio>
#include <cstdlib>
#include "defs.h"
#include "image.h"
#include "sqlq.h"

// Plan stuff
int plan_size(const uint8_t *p, int index) {
    if (PLAN_HASSIZE(p, index)) {
        int b = p[index] & ~PLAN_MASK;
        switch(b) {
            case 1: return p[index + 1] & ~PLAN_INSIDE;
            case 2: return (p[index + 1] & ~PLAN_INSIDE) + ((p[index + 2] & ~PLAN_INSIDE) << 7);
            case 3: return (p[index + 1] & ~PLAN_INSIDE) + ((p[index + 2] & ~PLAN_INSIDE) << 7) + ((p[index + 3] & ~PLAN_INSIDE) << 14);
        }
    }
    return 1;
}

void plan_setsize(uint8_t *p, int index, int size) {
    if (size < 0) {
        size = 1;
    } else if (size > 0x1FFFFF) {
        size = PLAN_MAXSIZE;
    }
    p[index] &= PLAN_MASK;
    if (size == 1) {
        p[index] += 0;
    } else if (size <= 0x7F) {
        p[index] += 1;
        p[index + 1] = 0x80 + size;
    } else if (size <= 0x3FFF) {
        p[index] += 2;
        p[index + 1] = 0x80 + (size & 0x7F);
        p[index + 2] = 0x80 + ((size >> 7) & 0x7F);
    } else {
        p[index] += 3;
        p[index + 1] = 0x80 + (size & 0x7F);
        p[index + 2] = 0x80 + ((size >> 7) & 0x7F);
        p[index + 3] = 0x80 + ((size >> 14) & 0x7F);
    }
}

void IMG_loadplan(Image_t *img, int secid, const char *filename) {
    assert(img != NULL);
    Section_t *sec = IMG_getsection(img, secid);
    if (sec->buf_plan != NULL) {
        BUF_free(sec->buf_plan);
        sec->buf_plan = NULL;
        sec->plan = NULL;
    }
    Buffer_t *b = BUF_createfromfile(filename);
    if (BUF_getsize(b) != IMG_getsecsize(img, secid)) {
        return;
    }
    sec->buf_plan = b;
    sec->plan = (uint8_t *)BUF_getptr(sec->buf_plan);
}

void plan_convert(const char *oldname, const char *newname) {
    Buffer_t *buf_old;
    Buffer_t *buf_new;

    printf("aqui %s %s \n", oldname, newname);

    //return;

    buf_old = BUF_createfromfile(oldname);
    int size = BUF_getsize(buf_old);
    buf_new = BUF_create(0);
    BUF_addsize(buf_new, size);
    const uint8_t *o = (const uint8_t *)BUF_getptr(buf_old);
    uint8_t *n = (uint8_t *)BUF_getptr(buf_new);
    for (int i = 0; i < size; i++) {
        uint8_t c = o[i];

        // DEAL WITH DATA2
        if (i < size - 1) {
            if (c == 0x04 && o[i + 1] == 0x03) {
                int j = i + 1;
                while(j < size && o[j] == 0x03) {
                    j++;
                }
                int len = j - i;
                assert(len > 0);
                n[i] = PLAN_DATA;
                plan_setsize(n, i, len);
                for (int k = 1; k < len; k++) {
                    n[i + k] |= PLAN_INSIDE;
                }
                i += len - 1;
                continue;
            }
        }
        // DEAL WITH ALIGN
        if (c == 0x02) {
            int j = i + 1;
            while(j < size && o[j] == 0x02) {
                j++;
            }
            int len = j - i;
            assert(len > 0);
            n[i] = PLAN_ALIGN;
            plan_setsize(n, i, len);
            for (int k = 1; k < len; k++) {
                n[i + k] |= PLAN_INSIDE;
            }
            i += len - 1;
            continue;
        }
        // DEAL WITH CODE
        if (c == 0x05 || (c >= 0x20 && c < 0x40)) {
            int len = 1;
            if (c >= 0x20 && c < 0x40) {
                len = o[i + 1] - 0x80;
            }
            assert(len > 0);
            n[i] = PLAN_CODE;
            plan_setsize(n, i, len);
            for (int k = 1; k < len; k++) {
                n[i + k] |= PLAN_INSIDE;
            }
            i += len - 1;
            continue;
        }
        // DEAL WITH ASCII
        if (c == 0x06) {
            int len = o[i + 1] - 0x80;
            assert(len > 0);
            n[i] = PLAN_STR;
            plan_setsize(n, i, len);
            for (int k = 1; k < len; k++) {
                n[i + k] |= PLAN_INSIDE;
            }
            i += len - 1;
            continue;
        }
        // DEAL WITH DATA
        if (c == 0x04) {
            n[i] = PLAN_DATA;
            plan_setsize(n, i, 1);
            continue;
        }
        // DEAL WITH UNKNOWN
        if (c == 0x00) {
            n[i] = PLAN_UNKNOWN;
            continue;
        }
        printf("unknown plan opcode at %08X!!!! %02X\n", i, c);
        BUF_free(buf_new);
        BUF_free(buf_old);
        return;
    }
    BUF_savetofile(buf_new, newname);
    BUF_free(buf_new);
    BUF_free(buf_old);
}
