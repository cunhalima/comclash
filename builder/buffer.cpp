#include <cassert>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include "defs.h"

// default granularity for capcity allocation
// default granularity for capcity allocation
#define DEF_GRAN    4096

struct Buffer_t {
    int pos;
    int size;
    int capacity;
    char *data;
    int grana;
    int granp;
};

int BUF_tell(const Buffer_t *buffer) {
    assert(buffer != NULL);
    return buffer->pos;
}

void BUF_addpos(Buffer_t *buffer, int amount) {
    assert(buffer != NULL);
    BUF_seek(buffer, amount, 1);
}

void BUF_writebuffer(Buffer_t *buffer, const Buffer_t *src) {
    assert(buffer != NULL);
    BUF_write(buffer, src->data, src->size);
}

char *BUF_getptr(Buffer_t *buffer) {
    assert(buffer != NULL);
    return buffer->data;
}

char *BUF_getposptr(Buffer_t *buffer) {
    assert(buffer != NULL);
    return &buffer->data[buffer->pos];
}

char *BUF_getptrat(Buffer_t *buffer, int offset) {
    assert(buffer != NULL);
    return &buffer->data[offset];
}

int BUF_getsize(const Buffer_t *buffer) {
    assert(buffer != NULL);
    return buffer->size;
}

void BUF_reset(Buffer_t *buffer) {
    assert(buffer != NULL);
    buffer->size = 0;
    buffer->pos = 0;
}

void BUF_setgran(Buffer_t *buffer, int gran) {
    assert(buffer != NULL);
    buffer->granp = gran - 1;
    buffer->grana = ~buffer->granp;
}

int BUF_seek(Buffer_t *buffer, int pos, int from) {
    assert(buffer != NULL);
    if (from == 0)
        buffer->pos = pos;
    else if (from == 1)
        buffer->pos += pos;
    else if (from == 2)
        buffer->pos = buffer->size + pos;
    return buffer->pos;
}

void BUF_setcapacity(Buffer_t *buffer, int cap) {
    assert(buffer != NULL);
    int oldcap;
    
    oldcap = buffer->capacity;
    if (cap < 0)
        return;
    cap += buffer->granp;
    cap &= buffer->grana;

    buffer->data = (char *)realloc(buffer->data, cap);
    buffer->capacity = cap;
    if (buffer->size > buffer->capacity)
        buffer->size = buffer->capacity;
    if (cap > oldcap) {
        memset(&buffer->data[oldcap], 0, cap - oldcap);
    }
}

void BUF_setsize(Buffer_t *buffer, int size) {
    assert(buffer != NULL);
    buffer->size = size;
    if (buffer->size < 0) {
        buffer->size = 0;
    } else if (buffer->size > buffer->capacity) {
        BUF_setcapacity(buffer, buffer->size);
    }
}

void BUF_addsize(Buffer_t *buffer, int amount) {
    assert(buffer != NULL);
    BUF_setsize(buffer, buffer->size + amount);
}

uint8_t BUF_get8(const Buffer_t *buffer, int pos) {
    assert(buffer != NULL);
    if ((pos < 0) || (pos >= buffer->size))
        return 0;
    return buffer->data[pos];
}

uint16_t BUF_get16(const Buffer_t *buffer, int pos) {
    assert(buffer != NULL);
    return BUF_get8(buffer, pos) | (BUF_get8(buffer, pos + 1) << 8);
}

uint32_t BUF_get32(const Buffer_t *buffer, int pos) {
    assert(buffer != NULL);
    return BUF_get16(buffer, pos) | (BUF_get16(buffer, pos + 2) << 16);
}

void BUF_put8(Buffer_t *buffer, int pos, uint8_t val) {
    assert(buffer != NULL);
    if ((pos < 0) || (pos >= buffer->size))
        return;
    buffer->data[pos] = val;
}

void BUF_put16(Buffer_t *buffer, int pos, uint16_t val) {
    assert(buffer != NULL);
    BUF_put8(buffer, pos, val & 0xFF);
    BUF_put8(buffer, pos + 1, (val >> 8) & 0xFF);
}

void BUF_put32(Buffer_t *buffer, int pos, uint32_t val) {
    assert(buffer != NULL);
    BUF_put16(buffer, pos, val & 0xFFFF);
    BUF_put16(buffer, pos + 2, (val >> 16) & 0xFFFF);
}

uint8_t BUF_read8(Buffer_t *buffer) {
    assert(buffer != NULL);
    uint8_t v = BUF_get8(buffer, buffer->pos);
    BUF_addpos(buffer, sizeof(v));
    return v;
}

uint16_t BUF_read16(Buffer_t *buffer) {
    assert(buffer != NULL);
    uint16_t v = BUF_get16(buffer, buffer->pos);
    BUF_addpos(buffer, sizeof(v));
    return v;
}

uint32_t BUF_read32(Buffer_t *buffer) {
    assert(buffer != NULL);
    uint32_t v = BUF_get32(buffer, buffer->pos);
    BUF_addpos(buffer, sizeof(v));
    return v;
}

void BUF_write8(Buffer_t *buffer, uint8_t val) {
    assert(buffer != NULL);
    if (buffer->pos >= 0) {
        if (buffer->pos >= buffer->size) {
            BUF_setsize(buffer, buffer->pos + sizeof(val));
        }
        BUF_put8(buffer, buffer->pos, val);
    }
    BUF_addpos(buffer, sizeof(val));
}

void BUF_write16(Buffer_t *buffer, uint16_t val) {
    assert(buffer != NULL);
    BUF_write8(buffer, val & 0xFF);
    BUF_write8(buffer, (val >> 8) & 0xFF);
}

void BUF_write32(Buffer_t *buffer, uint32_t val) {
    assert(buffer != NULL);
    BUF_write16(buffer, val & 0xFFFF);
    BUF_write16(buffer, (val >> 16) & 0xFFFF);
}

void BUF_read(Buffer_t *buffer, void *data, int size) {
    assert(buffer != NULL);
    int npos = buffer->pos + size;
    if (npos > buffer->size) {
        npos = buffer->size;
        size = npos - buffer->pos;
    }
    if (size < 0)
        return;
    memcpy(data, &buffer->data[buffer->pos], size);
    buffer->pos = npos;
}

void BUF_write(Buffer_t *buffer, const void *data, int size) {
    assert(buffer != NULL);
    int npos = buffer->pos + size;
    if (npos > buffer->size) {
        BUF_setsize(buffer, npos);
    }
    memcpy(&buffer->data[buffer->pos], data, size);
    buffer->pos = npos;
}

void BUF_writecstr(Buffer_t *buffer, const char* text) {
    assert(buffer != NULL);
    int size = strlen(text);
    BUF_write(buffer, text, size + 1);
}

void BUF_writeomfstr(Buffer_t *buffer, const char* text) {
    assert(buffer != NULL);
    uint8_t size;

    size = strlen(text);
    BUF_write8(buffer, size);
    BUF_write(buffer, text, size);
}

Buffer_t *BUF_create(int size)
{
    Buffer_t *buffer;
    buffer = (Buffer_t *)malloc(sizeof(Buffer_t));
    if (buffer != NULL) {
        BUF_setgran(buffer, DEF_GRAN);
        buffer->pos = 0;
        if (size == 0)
            size = 4;        
        buffer->capacity = size;
        buffer->size = 0;
        buffer->data = (char *)malloc(size);
        memset(buffer->data, 0, size);
        if (buffer->data == NULL) {
            free(buffer);
            buffer = NULL;
        }
    }
    return buffer;
}

void BUF_zero(Buffer_t *buffer) {
    assert(buffer != NULL);
    memset(buffer->data, 0, buffer->size);
}

void BUF_free(Buffer_t *buffer) {
    assert(buffer != NULL);
    if (buffer != NULL) {
        if (buffer->data != NULL) {
            free(buffer->data);
        }
        free(buffer);
    }
}

void BUF_insertvoid(Buffer_t *buffer, int pos, int size) {
    assert(buffer != NULL);
    int oldsize = buffer->size;
    BUF_setsize(buffer, buffer->size + size);
    memmove(&buffer->data[pos + size], &buffer->data[pos], oldsize - pos);
}

void BUF_removearea(Buffer_t *buffer, int pos, int size) {
    assert(buffer != NULL);
    // possível xploit 
    memmove(&buffer->data[pos], &buffer->data[pos + size],
        buffer->size - (pos + size));
    BUF_setsize(buffer, buffer->size - size);
}

Buffer_t *BUF_createfromfile(const char *filename) {
    FILE *fs;
    Buffer_t *buffer;
    int size;

    fs = fopen(filename, "rb");
    if (fs == NULL)
        return NULL;
    fseek(fs, 0, SEEK_END);
    size = ftell(fs);
    fseek(fs, 0, SEEK_SET);
    buffer = BUF_create(size);
    if (buffer != NULL) {
        BUF_setsize(buffer, size);
        size = fread(buffer->data, 1, size, fs);
    }
    fclose(fs);
    return buffer;
}

bool BUF_savetofile(const Buffer_t *buffer, const char *filename) {
    assert(buffer != NULL);
    FILE *fs = fopen(filename, "wb");
    if (fs == NULL)
        return false;
    fwrite(buffer->data, 1, buffer->size, fs);
    fclose(fs);
    return true;
}

