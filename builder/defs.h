#ifndef DEFS_H
#define DEFS_H

// common
#include <cstdint>
typedef signed int uaddr_t;
#define uaddr_sec(x) (((x)>>28)&7)
#define uaddr_off(x) ((x)&0x0FFFFFFF)
#define uaddr_mk(x,y) ((((x)&7)<<28)|((y)&0x0FFFFFFF))

int addr_to_ida(uaddr_t addr);
uaddr_t ida_to_addr(int addr);

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
Buffer_t *BUF_createfromfilecstr(const char* filename);
bool BUF_savetofile(const Buffer_t *buffer, const char* filename);
void BUF_reset(Buffer_t *buffer);
void BUF_writecstr(Buffer_t *buffer, const char* text);
void BUF_writeomfstr(Buffer_t *buffer, const char* text);

// image.cpp
struct Image_t;

void IMG_addsection(Image_t *img, int secid, int secuse, int secvirt);
void IMG_addsecsize(Image_t *img, int secid, int size);
int IMG_getsecsize(Image_t *img, int secid);
Image_t *IMG_create(void);
void IMG_free(Image_t *img);
void IMG_begintransaction(Image_t *img);
void IMG_endtransaction(Image_t *img);
bool IMG_dbopen(Image_t *img, const char *dbname);
void IMG_dbclose(Image_t *img);
void IMG_sqlfile(Image_t *img, const char *filename);
void IMG_sql(Image_t *img, const char *sql);
void *IMG_getptr(Image_t *img, uaddr_t addr);
bool IMG_loadexe(Image_t *img, const char *filename, bool addrelocs, bool createsections, bool loaddata, bool compare);
void IMG_addreloc(Image_t *img, uaddr_t afrom, uaddr_t ato, int fix_size, int disp, int type);
void IMG_addlabel(Image_t *img, uaddr_t addr, const char *name, int type);
void IMG_linklabout(Image_t *img, const char *name);
void IMG_linkrelout(Image_t *img, uaddr_t addr, const char *name, const char *segname);
void IMG_movelabel(Image_t *img, int labid, uaddr_t oldaddr, uaddr_t newaddr);
void IMG_fixrelocations(Image_t *img);
void IMG_createseg(Image_t *img, const char *name, const char *sclass, int align, int use, uaddr_t addr, int priority, int size);
void IMG_createslice(Image_t *img, const char *name, uaddr_t addr);
void IMG_fixlabels(Image_t *img, int secid);
void IMG_addrlabels(Image_t *img, int secid);
void IMG_setmodules(Image_t *img);
void IMG_setsegments(Image_t *img);
void IMG_setlabname(Image_t *img, uaddr_t addr, const char *name, int type);

#define LABEL_STATIC    0
#define LABEL_PUBLIC    1
#define LABEL_CONSTANT  16

// disasm
void IMG_disasm(Image_t *img);
#define RELOC_ABS   0
#define RELOC_REL   1
#define RELOC_CONST 2

#define PLAN_INSIDE     0x80
#define PLAN_MASK       0xF8
#define PLAN_MAXSIZE    0x1FFFFF        // enough room for more, but needs to adjust plan_size() and plan_setsize
#define PLAN_UNKNOWN    0x00
#define PLAN_DATA       0x10
#define PLAN_STR        0x18
#define PLAN_ALIGN      0x20
#define PLAN_CODE       0x30
#define PLAN_HASSIZE(p,x) (p[x] >= PLAN_DATA && p[x] < (PLAN_CODE+7))
#define PLAN_ISCODE(p,x) ((p[x] & PLAN_MASK) == PLAN_CODE)
#define PLAN_ISDATA(p,x) ((p[x] >= PLAN_DATA && p[x] < PLAN_CODE)
#define PLAN_ISSTR(p,x) ((p[x] & PLAN_MASK) == PLAN_STR)
#define PLAN_ISALIGN(p,x) ((p[x] & PLAN_MASK) == PLAN_ALIGN)
int plan_size(const uint8_t *p, int index);
void plan_setsize(uint8_t *p, int index, int size);
void plan_convert(const char *oldname, const char *newname);
void IMG_loadplan(Image_t *img, int secid, const char *filename);

// compare
bool IMG_comparesection(Image_t *img, int secid, int size, void *data);
void IMG_comparestats(Image_t *img, int num_sections, int num_relocs);
bool IMG_comparereloc(Image_t *img, uaddr_t afrom, uaddr_t ato, int fix_size);

// walk
void IMG_walkplan(Image_t *img, int secid);
bool my_isprintable(char c);

#endif
