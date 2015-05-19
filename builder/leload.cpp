#include <cassert>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "defs.h"
#include "image.h"

#define EFT_NONE    0
#define EFT_16B     1

#define FIX_SIZE_16 0x02
#define FIX_SIZE_32 0x07

//  EFT_SEG = 1, // Not offset, not 32 bits
//  EFT_REL = 2, // Not absolute
//  EFT_EXT = 4  // Not internal

struct LEHeader {
    uint32_t magic_ord;       // 0x00
    uint32_t format_level;    // 0x04
    uint32_t cpu_os_type;     // 0x08
    uint32_t mod_version;     // 0x0C
    uint32_t mod_flags;       // 0x10
    uint32_t mod_num_pages;   // 0x14
    uint32_t eip_obj;         // 0x18
    uint32_t eip;             // 0x1C
    uint32_t esp_obj;         // 0x20
    uint32_t esp;             // 0x24
    uint32_t page_size;       // 0x28
    uint32_t last_page_bytes; // 0x2C
    uint32_t fixup_sec_size;  // 0x30
    uint32_t fixup_sec_chk;   // 0x34
    uint32_t loader_sec_size; // 0x38
    uint32_t loader_sec_chk;  // 0x3C
    uint32_t obj_tab_off;     // 0x40
    uint32_t obj_tab_num;     // 0x44
    uint32_t obj_ptb_off;     // 0x48
    uint32_t obj_ipg_off;     // 0x4C
    uint32_t res_tab_off;     // 0x50
    uint32_t res_tab_num;     // 0x54
    uint32_t rsn_tab_off;     // 0x58
    uint32_t ent_tab_off;     // 0x5C
    uint32_t mod_dir_off;     // 0x60
    uint32_t mod_dir_num;     // 0x64
    uint32_t fix_ptb_off;     // 0x68
    uint32_t fix_rtb_off;     // 0x6C
    uint32_t imm_tab_off;     // 0x70
    uint32_t imm_tab_num;     // 0x74
    uint32_t imp_tab_off;     // 0x78
    uint32_t ppg_chk_off;     // 0x7C
    uint32_t dat_pag_off;     // 0x80
    uint32_t pld_pag_num;     // 0x84
    uint32_t nrn_tab_off;     // 0x88
    uint32_t nrn_tab_len;     // 0x8C
    uint32_t nrn_tab_chk;     // 0x90
    uint32_t ads_obj_num;     // 0x94
    uint32_t dbg_inf_off;     // 0x98
    uint32_t dbg_inf_len;     // 0x9C
    uint32_t num_ins_pld;     // 0xA0
    uint32_t num_ins_dem;     // 0xA4
    uint32_t heap_size;       // 0xA8
    uint32_t unknown[ 6 ];    // 0xAC
};

static Buffer_t *inbuf;
static int mzpos = -1;
static int lepos = -1;
static int code_section;
static int data_section;
static Buffer_t * obj_pag_idx;
static Buffer_t * obj_pag_num;
static Buffer_t * fix_page;
static const LEHeader *header;
static int num_sections;
static int total_sections;
static int bss_size;
static int data_size;
static int code_size;
static int data_addr;
static int code_addr;

static void findmzheader()
{
    mzpos = 0;
    for (;;) {
        uint32_t size = BUF_get16(inbuf, mzpos + 4) * 512 + BUF_get16(inbuf, mzpos + 2);
        if (BUF_get16(inbuf, mzpos + 0) == 0x5A4D)
        size -= 512;
        if (BUF_get16(inbuf, mzpos + 0) == 0x454C)
        return; // It's definetely LE
        if (BUF_get16(inbuf, mzpos + 0) == 0x5A4D) { // Is it MZ?
            if (BUF_get16(inbuf, mzpos + 0x18) == 0x40)
            return;
        }
        else if (BUF_get16(inbuf, mzpos + 0) == 0x5742) {
            //--
        }
        else {
            mzpos = -1;
            return;
        }
        mzpos += size;
    }
}

static void findleheader() {
    lepos = mzpos = -1;
    if (mzpos == -1) {
        findmzheader();
        if (mzpos == -1) {
            lepos = -1;
            printf("MZ header not found\n");
            return;
        }
    }
    lepos = mzpos;
    if (BUF_get16(inbuf, lepos + 0) == 0x454C) return;
    if (BUF_get16(inbuf, lepos + 0x18) != 0x40) {
        lepos = -1;
        return;
    }
    lepos += BUF_get32(inbuf, lepos + 0x3C);
    if (BUF_get16(inbuf, lepos + 0) != 0x454C) {
        lepos = -1;
        return;
    }
}

/*
static void le_close()
{
    BUF_free(fix_page);
    BUF_free(obj_pag_num);
    BUF_free(obj_pag_idx);
    BUF_free(inbuf);
}
*/

/*
Image_t *IMG_load(const char *filename)
{
    Image_t *image;
    int csize, dsize, bsize, cstart;

    image = malloc(sizeof(Image_t));
    if (le_open(filename, &csize, &dsize, &bsize, &cstart) == FALSE) {
        free(image);
        return NULL;
    }
    image->section[1-1] = BUF_create(csize);
    image->section[3-1] = BUF_create(dsize);
    image->virtual_size[4-1] = bsize;
    le_read(BUF_getptr(image->section[1-1]), BUF_getptr(image->section[3-1]));
    le_close();
    return image;
}
*/


bool IMG_loadexe(Image_t *img, const char *filename, bool addrelocs, bool createsections, bool loaddata, bool compare) {
    assert(img != NULL);
/*
    int csize, dsize, bsize, cstart;

    if (le_open(filename, &csize, &dsize, &bsize, &cstart) == FALSE) {
        le_close();
        return FALSE;
    }
    // FIXME these should not be hardcoded    !!!!!
    IMG_addsection(img, 1, ST_TEXT, 0);
    IMG_addsection(img, 2, ST_16BIT, 0);
    IMG_addsection(img, 3, ST_DATA, 0);
    IMG_addsection(img, 4, ST_BSS | ST_VIRTUAL, 0);
    IMG_addsecsize(img, 1, csize);
    //IMG_addsecsize(img, 1, csize);
    IMG_addsecsize(img, 3, dsize);
    IMG_addsecsize(img, 4, bsize);
    le_read(IMG_getptr(img, uaddr_mk(1, 0)), IMG_getptr(img, uaddr_mk(3, 0)));
    le_close();
    return TRUE;
*/
    int secpos;
    int stype;
    uint32_t id;
    int i;
    int ptr_end = 0x21B;  // hardcoded values for Watcom 9.5c
    int ptr_edata = 0x220;
    uint32_t size;
    uint32_t bss_start;
    uint32_t bss_end;
    uint32_t vrsize;
    uint32_t old_vrsize;
    uint32_t reloc_base;
    uint32_t secflags;
    uint32_t pag_tab_idx;
    uint32_t pag_tab_num;
    int mysecflags;

    inbuf = BUF_createfromfile(filename);
    if (inbuf == NULL) {
        return false;
    }
    findleheader();
    BUF_seek(inbuf, lepos, 0);
    header = (const LEHeader*)BUF_getposptr(inbuf);
    if (((header->magic_ord & 0xFF) != 'L') || (((header->magic_ord >> 8) & 0xFF) != 'E')) {
        printf("Invalid LE EXE\n");
        BUF_free(inbuf);
        return false;
    }
    if (addrelocs || createsections) {
        IMG_begintransaction(img);
        printf("; Loading %s\n", filename);
    }
    if (compare) {
        printf("Comparing against %s\n", filename);
    }
    obj_pag_idx = BUF_create(0);
    obj_pag_num = BUF_create(0);
    fix_page = BUF_create(0);

    // read the header
    num_sections = header->obj_tab_num;
    total_sections = 0;

    //
    // Lê o Sector Table e as páginas
    //
    BUF_seek(inbuf, lepos + header->obj_tab_off, 0);
    code_size = 0;
    bss_size = 0;
    data_size = 0;
    code_section = 0;
    data_section = 0;
    for (i = 0; i < num_sections; i++) {
        stype = ST_NONE;
        mysecflags = 0;
        id = i + 1;
        if (id == header->eip_obj) {
            stype = ST_TEXT;
            code_section = id;
        }
        if (id == header->ads_obj_num) {
            stype = ST_DATA;
            data_section = id;
        }
        old_vrsize = vrsize = BUF_read32(inbuf);
        reloc_base = BUF_read32(inbuf);
        (void)reloc_base;
        secflags = BUF_read32(inbuf);
        printf("; %08X\n", secflags);
        mysecflags = 32;
        if (!(secflags & 0x2000)) {
            mysecflags = 16;
        }
        (void)secflags;
        pag_tab_idx = BUF_read32(inbuf);
        pag_tab_num = BUF_read32(inbuf);
        BUF_read32(inbuf); // reserved, must be zero

        BUF_write32(obj_pag_idx, pag_tab_idx);
        BUF_write32(obj_pag_num, pag_tab_num);

        // Lê os dados deste objeto
        secpos = (pag_tab_idx - 1) * header->page_size + mzpos + header->dat_pag_off;
        size = pag_tab_num * header->page_size;
        //printf("%08X %08X\n", size, vrsize);
        if (i == num_sections - 1) {
            size -= header->page_size;
            size += header->last_page_bytes;
        } else {
            if (vrsize < size)
                size = vrsize;
        }
        if (i == num_sections - 1) {
            vrsize = data_size;
        }
        total_sections++;
        if (createsections) {
            IMG_addsection(img, id, mysecflags, false);
            IMG_addsecsize(img, id, vrsize);
        }
        if (loaddata) {
            //memcpy(IMG_getptr(img, uaddr_mk(id, 0)), BUF_getptrat(inbuf, secpos), vrsize);
            memcpy(IMG_getptr(img, uaddr_mk(id, 0)), BUF_getptrat(inbuf, secpos), size);
        }
        if (compare) {
            #if 1
            if (!IMG_comparesection(img, id, vrsize, BUF_getptrat(inbuf, secpos))) {
                printf("problem found\n");
                BUF_free(fix_page);
                BUF_free(obj_pag_num);
                BUF_free(obj_pag_idx);
                BUF_free(inbuf);
                return false;
            }
            #endif
        }
        if (i == num_sections - 1) {
            if (createsections) {
                IMG_addsection(img, id + 1, mysecflags, true);
                IMG_addsecsize(img, id + 1, bss_size);            
            }
            if (compare) {
                #if 1
                if (!IMG_comparesection(img, id, vrsize, NULL)) {
                    printf("problem found\n");
                    BUF_free(fix_page);
                    BUF_free(obj_pag_num);
                    BUF_free(obj_pag_idx);
                    BUF_free(inbuf);
                    return false;
                }
                #endif
            }
            total_sections++;
        }
        if (stype == ST_DATA) {
            data_addr = secpos;
        }
        if (stype == ST_TEXT) {
            code_addr = secpos;
            code_size = vrsize;
            bss_start = 0;
            bss_end = 0;
            if (ptr_edata != 0) {
                bss_start = BUF_get32(inbuf, secpos + header->eip + ptr_edata);
            }
            if (ptr_end != 0) {
                bss_end = BUF_get32(inbuf, secpos + header->eip + ptr_end);
            }
            data_size = bss_start;
            bss_size = bss_end - bss_start;
        }
    }
    //*code_start = header->eip;
    int num_eff_fixups;
    
    {
        uaddr_t data_endaddr;
        int o;
        int ro;
        int pg_data_pos;
        int start;
        int end;
        int pos;
        uint8_t src;
        uint8_t flags;
        uint32_t off_size;
        uint32_t obj_size;
        int fix_type;
        int16_t srcoff;
        uint16_t obnum;
        uint32_t off;
        uint32_t data_pos;
        int fix_size;
        //int mod_name_size;
        //const char* mod_name;
        int num_fixups;
        int first;
        int last;
        uaddr_t afrom, ato;

        num_fixups = 0;
        num_eff_fixups = 0;
        data_endaddr = uaddr_mk(data_section, data_size);
        // Read fix pages
        //
        BUF_seek(inbuf, lepos + header->fix_ptb_off, 0);
        for (uint32_t i = 0; i < header->mod_num_pages + 1; i++) {
            BUF_write32(fix_page, BUF_read32(inbuf));
        }
        // Read fixups
        //
        for (o = 0; o < num_sections; o++) {
            ro = o;
            first = BUF_get32(obj_pag_idx, ro * sizeof(int)) - 1;
            last = first + BUF_get32(obj_pag_num, ro * sizeof(int));
            for (int i = first; i < last; i++)
            {
                pg_data_pos = (i - first) * header->page_size;
                start = BUF_get32(fix_page, i * sizeof(int));
                end = BUF_get32(fix_page, (i + 1) * sizeof(int));
                BUF_seek(inbuf, lepos + header->fix_rtb_off + start, 0);
                pos = start;
                while(pos < end)
                {
                    num_fixups++;                
                    src = BUF_read8(inbuf);
                    flags = BUF_read8(inbuf);
                    off_size = (flags & 0x10) ? 4 : 2;
                    obj_size = (flags & 0x40) ? 2 : 1;
                    if (src == FIX_SIZE_16)
                        off_size = 0;
                    fix_type = EFT_NONE;
                    if (src == FIX_SIZE_16) {
                        fix_type = EFT_16B;
                    }
                    srcoff = BUF_read16(inbuf);
                    obnum = (obj_size == 1) ? BUF_read8(inbuf) : BUF_read16(inbuf);
                    if (off_size > 0)                
                        off = (off_size == 2) ? BUF_read16(inbuf) : BUF_read32(inbuf);
                    else
                        off = 0;
                    fix_size = (fix_type == EFT_16B) ? 2 : 4;
                    pos += 2 + 2 + obj_size + off_size;
                    data_pos = srcoff + pg_data_pos;
                    if (srcoff >= 0) {      // Ignore negative fixups (because they're repetitive)
                        afrom = uaddr_mk(o + 1, data_pos);
                        ato = uaddr_mk(obnum, off);
                        if (ato >= data_endaddr) {
                            ato -= data_endaddr;
                            ato = uaddr_mk(obnum + 1, ato);
                        }                    
                        //IMG_addfixup_fast(img, afrom, ato, 0, 0, NULL, fix_size);
                        if (addrelocs) {
                            //printf("%d\n", num_eff_fixups);
                            IMG_addreloc(img, afrom, ato, fix_size, 0, RELOC_ABS);
                        }
                        if (compare) {
                            #if 1
                            if (!IMG_comparereloc(img, afrom, ato, fix_size)) {
                                printf("problem found\n");
                                BUF_free(fix_page);
                                BUF_free(obj_pag_num);
                                BUF_free(obj_pag_idx);
                                BUF_free(inbuf);
                                return false;
                            }
                            #endif
                        }
                        num_eff_fixups++;
                    }
                }
            }
        }
    }
    if (addrelocs) {
        IMG_addlabel(img, uaddr_mk(code_section, header->eip), "..start");
    }
    if (createsections || addrelocs) {
        IMG_endtransaction(img);
    }
    if (compare) {
        IMG_comparestats(img, num_sections, num_eff_fixups);
        printf("Done\n");
    }
    BUF_free(fix_page);
    BUF_free(obj_pag_num);
    BUF_free(obj_pag_idx);
    BUF_free(inbuf);
    return true;
}

