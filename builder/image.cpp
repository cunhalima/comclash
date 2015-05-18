#include <cassert>
#include <cstdio>
#include <cstdlib>
#include "defs.h"
#include "image.h"
#include "sqlq.h"

void *IMG_getptr(Image_t *img, uaddr_t addr) {
    assert(img != NULL);
    uint8_t *ptr;
    Section_t *sec;

    sec = IMG_getsection(img, uaddr_sec(addr));
    if (sec->data == NULL) {
        return NULL;
    }
    return &sec->data[uaddr_sec(addr)];
}

Section_t *IMG_getsection(Image_t *img, int secid) {
    assert(img != NULL);
    assert(secid > 0 && secid <= MAX_SECTIONS);
    return &img->sections[secid];
}

void IMG_addsection(Image_t *img, int secid, int secuse, int secvirt) {
    assert(img != NULL);
    Section_t *sec;

    sec = IMG_getsection(img, secid);
    sec->use = secuse;
    sec->virt = secvirt;
    sec->buf_data = NULL;
    sec->buf_plan = NULL;
    sec->data = NULL;
    sec->plan = NULL;
    sec->size = 0;
}

void IMG_addsecsize(Image_t *img, int secid, int size) {
    assert(img != NULL);
    Section_t *sec;

    sec = IMG_getsection(img, secid);
    if (!sec->virt) {
        if (sec->buf_data == NULL) {
            sec->buf_data = BUF_create(0);
        }
        BUF_addsize(sec->buf_data, size);
        sec->data = (uint8_t *)BUF_getptr(sec->buf_data);
    }
    sec->size += size;
}

Image_t *IMG_create(void) {
    Image_t *img;

    img = (Image_t *)calloc(1, sizeof(Image_t));
    return img;
}

void IMG_free(Image_t *img) {
    assert(img != NULL);

    IMG_dbclose(img);
    for (int i = 1; i <= MAX_SECTIONS; i++) {
        Section_t *sec;

        sec = IMG_getsection(img, i);
        if (sec->buf_data != NULL) {
            BUF_free(sec->buf_data);
        }
        if (sec->buf_plan != NULL) {
            BUF_free(sec->buf_plan);
        }
        sec->data = NULL;
        sec->plan = NULL;
    }
    free(img);
}

void IMG_begintransaction(Image_t *img) {
    assert(img != NULL);
    sqlite3_exec(img->db, "BEGIN TRANSACTION", NULL, NULL, NULL);
}

void IMG_endtransaction(Image_t *img) {
    assert(img != NULL);
    sqlite3_exec(img->db, "END TRANSACTION", NULL, NULL, NULL);
}

bool IMG_dbopen(Image_t *img, const char *dbname) {
    assert(img != NULL);
    printf("Opening DB\n");
    sqlite3_open(dbname, &img->db);
    return true;
}

void IMG_dbclose(Image_t *img) {
    assert(img != NULL);
    if (img->db != NULL) {
        sqlite3_close(img->db);
        img->db = NULL;
    }
}

void IMG_sqlfile(Image_t *img, const char *filename) {
    assert(img != NULL);
    Buffer_t *b = BUF_createfromfilecstr(filename);
    if (b != NULL) {
        sqlite3_exec(img->db, BUF_getptr(b), NULL, NULL, NULL);
        BUF_free(b);
    }
}

void IMG_sql(Image_t *img, const char *sql) {
    assert(img != NULL);
    sqlite3_exec(img->db, sql, NULL, NULL, NULL);
}

void IMG_addlabel(Image_t *img, uaddr_t addr, const char *name) {
    assert(img != NULL);
    // add the label
    sqlq_t sqlab(img->db, "INSERT OR IGNORE INTO tab_label(address, name) VALUES(@A, @N)");
    sqlab.bind_int(1, addr);
    if (name == NULL) {
        sqlab.bind_null(2);
    } else {
        sqlab.bind_str(2, name);
    }
    sqlab.run();
}

void IMG_addreloc(Image_t *img, uaddr_t afrom, uaddr_t ato, int fix_size, int disp) {
    assert(img != NULL);

    IMG_addlabel(img, ato, NULL);
    // add the relocation
    sqlq_t sqrel(img->db, "INSERT INTO tab_reloc(address, size, label, disp) VALUES(@AFROM, @SIZE, (SELECT labid FROM tab_label WHERE address=@ATO), @DI)");
    sqrel.bind_int(1, afrom);
    sqrel.bind_int(2, fix_size);
    sqrel.bind_int(3, ato);
    sqrel.bind_int(4, disp);
    sqrel.run();
}

void IMG_movelabel(Image_t *img, int labid, uaddr_t oldaddr, uaddr_t newaddr) {
    assert(img != NULL);
    
    printf("moving label %d -- %08X to %08X\n", labid, oldaddr, newaddr);
    int disp = oldaddr - newaddr;
    sqlq_t sqchglab(img->db, "UPDATE tab_label SET address = @RADDR WHERE labid = @LABID");
    sqchglab.bind_int(1, newaddr);
    sqchglab.bind_int(2, labid);
    if (!sqchglab.run()) {
        // Ja existe um label nessa posicao: pegar label, update de todos os relocs pra esse novo label, apagar label antigo
        sqlq_t sqsel(img->db, "SELECT labid FROM tab_label WHERE address = @RADDR");
        sqsel.bind_int(1, newaddr);
        int labid2 = sqsel.answer();

        sqlq_t squp(img->db, "UPDATE tab_reloc SET disp = disp + @DISP, label = @NEWLABEL WHERE label = @LABID");
        squp.bind_int(1, disp);
        squp.bind_int(2, labid2);
        squp.bind_int(3, labid);
        squp.run();

        sqlq_t sqdel(img->db, "DELETE FROM tab_label WHERE labid = @LABID");
        sqdel.bind_int(1, labid);
        sqdel.run();
    } else {
        sqlq_t squp(img->db, "UPDATE tab_reloc SET disp = disp + @DISP WHERE label = @LABID");
        squp.bind_int(1, disp);
        squp.bind_int(2, labid);
        squp.run();
    }
}

void IMG_fixrelocations(Image_t *img) {
    assert(img != NULL);

    IMG_begintransaction(img);
    sqlq_t sqsel(img->db, "SELECT l.labid, l.address, r.address FROM tab_reloc AS r "
                          "INNER JOIN tab_label AS l ON r.address < l.address AND "
                          "r.address + r.size > l.address");
    while(sqsel.step()) {
        int labid = sqsel.col_int(0);
        uaddr_t laddr = sqsel.col_int(1);
        uaddr_t raddr = sqsel.col_int(2);
        IMG_movelabel(img, labid, laddr, raddr);
    }
    IMG_endtransaction(img);
}

void IMG_createseg(Image_t *img, const char *name, const char *sclass, int align, int use, uaddr_t addr, int priority) {
    assert(img != NULL);

    sqlq_t s(img->db, "INSERT INTO tab_segment(name, class, align, use, address, priority) VALUES(@A, @B, @C, @D, @E, @F)");
    s.bind_str(1, name);
    s.bind_str(2, sclass);
    s.bind_int(3, align);
    s.bind_int(4, use);
    s.bind_int(5, addr);
    s.bind_int(6, priority);
    if (!s.run()) {
        printf("; Create segment error\n");
    } else {
        printf("; Created segment %s\n", name);
    }
}

void IMG_createslice(Image_t *img, const char *name, uaddr_t addr) {
    assert(img != NULL);

    printf("creating slice for %s\n", name);

    sqlq_t sqsel(img->db, "SELECT modid FROM tab_module WHERE name = @N");
    sqsel.bind_str(1, name);
    int modid = sqsel.answer();
    if (modid == 0) {
        return;
    }

    #if 0

    sqlq_t s(img->db, "INSERT INTO tab_modslice(address, size, module, segment) VALUES(@A, @B, @C, @D, @E, @F)");
    s.bind_str(1, name);
    s.bind_str(2, sclass);
    s.bind_int(3, align);
    s.bind_int(4, use);
    s.bind_int(5, addr);
    s.bind_int(6, priority);
    if (!s.run()) {
        printf("; Create segment error\n");
    } else {
        printf("; Created segment %s\n", name);
    }
    #endif
}

