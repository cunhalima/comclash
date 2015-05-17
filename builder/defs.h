#ifndef DEFS_H
#define DEFS_H

// common
#include <cstdint>
typedef signed int uaddr_t;
#define uaddr_sec(x) (((x)>>28)&7)
#define uaddr_off(x) ((x)&0x0FFFFFFF)
#define uaddr_mk(x,y) ((((x)&7)<<28)|((y)&0x0FFFFFFF))

// buffer.cpp
struct Buffer_t;
int BUF_tell(const Buffer_t *buffer);
void BUF_addpos(Buffer_t *buffer, int amount);
void BUF_writebuffer(Buffer_t *buffer, const Buffer_t *src);
char *BUF_getptr(Buffer_t *buffer);
char *BUF_getposptr(Buffer_t *buffer);
char *BUF_getptrat(Buffer_t *buffer, int offset);
int BUF_getsize(const Buffer_t *buffer);
void BUF_setgran(Buffer_t *buffer, int gran);
int BUF_seek(Buffer_t *buffer, int pos, int from);
void BUF_setcapacity(Buffer_t *buffer, int cap);
void BUF_setsize(Buffer_t *buffer, int size);
void BUF_resetsize(Buffer_t *buffer, int size);
void BUF_addsize(Buffer_t *buffer, int amount);
uint8_t BUF_get8(const Buffer_t *buffer, int pos);
uint16_t BUF_get16(const Buffer_t *buffer, int pos);
uint32_t BUF_get32(const Buffer_t *buffer, int pos);
void BUF_put8(Buffer_t *buffer, int pos, uint8_t val);
void BUF_put16(Buffer_t *buffer, int pos, uint16_t val);
void BUF_put32(Buffer_t *buffer, int pos, uint32_t val);
uint8_t BUF_read8(Buffer_t *buffer);
uint16_t BUF_read16(Buffer_t *buffer);
uint32_t BUF_read32(Buffer_t *buffer);
void BUF_write8(Buffer_t *buffer, uint8_t val);
void BUF_write16(Buffer_t *buffer, uint16_t val);
void BUF_write32(Buffer_t *buffer, uint32_t val);
void BUF_read(Buffer_t *buffer, void* data, int size);
void BUF_write(Buffer_t *buffer, const void* data, int size);
Buffer_t *BUF_create(int size);
void BUF_zero(Buffer_t *buffer);
void BUF_insertvoid(Buffer_t *buffer, int pos, int size);
void BUF_removearea(Buffer_t *buffer, int pos, int size);
void BUF_free(Buffer_t *buffer);
Buffer_t *BUF_createfromfile(const char* filename);
bool BUF_savetofile(const Buffer_t *buffer, const char* filename);
void BUF_reset(Buffer_t *buffer);
void BUF_writecstr(Buffer_t *buffer, const char* text);
void BUF_writeomfstr(Buffer_t *buffer, const char* text);

// image.cpp
struct Image_t;
void *IMG_getptr(Image_t *img, uaddr_t addr);
Image_t *IMG_create(void);
void IMG_free(Image_t *img);

#endif

