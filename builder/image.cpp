#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <libdis.h>
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
    return &sec->data[uaddr_off(addr)];
}

Section_t *IMG_getsection(Image_t *img, int secid) {
    assert(img != NULL);
    assert(secid > 0 && secid <= MAX_SECTIONS);
    return &img->sections[secid];
}

void IMG_addsection(Image_t *img, int secid, int secuse, int secvirt) {
    assert(img != NULL);
    Section_t *sec;

    img->numsections++;
    sec = IMG_getsection(img, secid);
    sec->use = secuse;
    sec->virt = secvirt;
    sec->buf_data = NULL;
    sec->buf_plan = NULL;
    sec->data = NULL;
    sec->plan = NULL;
    sec->size = 0;
}

int IMG_getsecsize(Image_t *img, int secid) {
    assert(img != NULL);
    Section_t *sec;

    sec = IMG_getsection(img, secid);
    return sec->size;
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

void IMG_linklabout(Image_t *img, const char *name) {
    assert(img != NULL);

    sqlq_t q(img->db, "UPDATE tab_label SET address=NULL,module=NULL WHERE name=@A");
    q.bind_str(1, name);
    q.run();
}

void IMG_linkrelout(Image_t *img, uaddr_t addr, const char *name, const char *segname) {
    assert(img != NULL);

    sqlq_t qi(img->db, "INSERT OR IGNORE INTO tab_label(name, type, module, segment) VALUES(@N, 0, NULL, "
        "(SELECT segid FROM tab_segment WHERE name=@B))");
    qi.bind_str(1, name);
    qi.bind_str(2, segname);
    qi.run();
    sqlq_t qu(img->db, "UPDATE tab_reloc SET label=(SELECT labid FROM tab_label WHERE name=@A) "
                       "WHERE address=@C");
    qu.bind_str(1, name);
    qu.bind_int(2, addr);
    qu.run();
}


void IMG_addlabel(Image_t *img, uaddr_t addr, const char *name, int type) {
    assert(img != NULL);
    // add the label
    sqlq_t sqlab(img->db, "INSERT OR IGNORE INTO tab_label(address, name, type) VALUES(@A, @N, @T)");
    sqlab.bind_int(1, addr);
    if (name == NULL) {
        sqlab.bind_null(2);
    } else {
        sqlab.bind_str(2, name);
    }
    sqlab.bind_int(3, type);
    sqlab.run();
}

void IMG_addreloc(Image_t *img, uaddr_t afrom, uaddr_t ato, int fix_size, int disp, int type) {
    assert(img != NULL);

    IMG_addlabel(img, ato, NULL, LABEL_STATIC);
    // add the relocation
    sqlq_t sqrel(img->db, "INSERT INTO tab_reloc(address, size, label, disp, type) "
                          "VALUES(@AFROM, @SIZE, (SELECT labid FROM tab_label WHERE address=@ATO), @DI, @REL)");
    sqrel.bind_int(1, afrom);
    sqrel.bind_int(2, fix_size);
    sqrel.bind_int(3, ato);
    sqrel.bind_int(4, disp);
    sqrel.bind_int(5, type);
    sqrel.run();
}

void IMG_movelabel(Image_t *img, int labid, uaddr_t oldaddr, uaddr_t newaddr) {
    assert(img != NULL);
    
    //printf("moving label %d -- %08X to %08X\n", labid, oldaddr, newaddr);
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

/** uaddr_t IMG_getnextsegaddr(Image_t *img, int secid)
 */
uaddr_t IMG_getnextsegaddr(Image_t *img, int secid) {
    uaddr_t avail = uaddr_mk(secid, 0);
    sqlq_t s(img->db, "SELECT address, size FROM tab_segment WHERE address >= @A AND address < @B ORDER BY address, priority ASC");
    s.bind_int(1, uaddr_mk(secid, 0));
    s.bind_int(2, uaddr_mk(secid + 1, 0));
    while(s.step()) {
        uaddr_t addr = s.col_int(0);
        int size = s.col_int(1);
        if (avail == addr) {
            avail += size;
        } else {
            if (avail < addr) {
                return avail;
            }
            printf("segment collision in section %d: address of collision = %08X\n", secid, addr);
            return addr;
        }
    }
    return avail;
}

void IMG_createseg(Image_t *img, const char *name, const char *sclass, int align, int use, uaddr_t addr, int priority, int size) {
    assert(img != NULL);

    int secid = uaddr_sec(addr);
    int secsize = IMG_getsecsize(img, secid);
    if (secid > 1) {
        // Check if previous section is full
        int psecsize = IMG_getsecsize(img, secid - 1);
        int prevsize = uaddr_off(IMG_getnextsegaddr(img, secid - 1));
        if (prevsize != psecsize) {
            printf("segment not full: %d should be %d bytes but is %d bytes\n", secid - 1, psecsize, prevsize);
            return;
        }
        //printf("reaa\n");
    }
    uaddr_t nextaddr = IMG_getnextsegaddr(img, secid);
    if (addr != nextaddr) {
        printf("invalid segment address: should be %08X asked for %08X\n", nextaddr, addr);
        return;
    }
    sqlq_t s(img->db, "INSERT INTO tab_segment(name, class, align, use, address, priority, size) VALUES(@A, @B, @C, @D, @E, @F, @S)");
    s.bind_str(1, name);
    s.bind_str(2, sclass);
    s.bind_int(3, align);
    s.bind_int(4, use);
    s.bind_int(5, addr);
    s.bind_int(6, priority);
    s.bind_int(7, size);
    if (!s.run()) {
        printf("; Create segment error\n");
    } else {
        //printf("; Created segment %s at %08X size %d\n", name, addr, size);
    }
}

void IMG_createslice(Image_t *img, const char *name, uaddr_t addr) {
    assert(img != NULL);
    #if 1

    //printf("creating slice for %s\n", name);
    // Create module if not created yet
    sqlq_t sqsel(img->db, "SELECT modid FROM tab_module WHERE name = @N");
    sqsel.bind_str(1, name);
    int modid = sqsel.answer();
    if (modid == 0) {
        sqlq_t ssi(img->db, "INSERT INTO tab_module(name) VALUES(@A)");
        ssi.bind_str(1, name);
        ssi.run();
        modid = sqsel.answer();
        if (modid == 0) {
            printf("ERROR\n");
        }
    }
    // get segment for this slice
    sqlq_t segq(img->db, "SELECT segid FROM tab_segment WHERE address <= @A AND address + size > @B");
    segq.bind_int(1, addr);
    segq.bind_int(2, addr);
    int segid = segq.answer();
    if (segid == 0) {
        return;
    }

    // adjust previous slices' sizes

    int secid = uaddr_sec(addr);
    int secsize = IMG_getsecsize(img, secid);
    uaddr_t secstart = uaddr_mk(secid, 0);
    uaddr_t secend = uaddr_mk(secid, secsize);
    // passa por todos os slices deste seg
    sqlq_t sq(img->db, "SELECT address,size FROM tab_modslice WHERE address >= @A AND address < @B ORDER BY address ASC");
    sq.bind_int(1, secstart);
    sq.bind_int(2, secend);
    int size = secend - addr;
    while(sq.step()) {
        int this_start = sq.col_int(0);
        int this_size = sq.col_int(1);
        int this_end = this_start + this_size;
        if (this_end <= addr) {
            // do nothing
        // Achei um slice antes do meu que tem que ser cortado
        } else if (this_start < addr && this_end > addr) {
            //if (this_start > addr) {
            // muda o size do atual
            sqlq_t upd(img->db, "update tab_modslice set size=@a where address=@b");
            uaddr_t this_new_end = addr;
            int this_new_size = this_new_end - this_start;

            upd.bind_int(1, this_new_size);
            upd.bind_int(2, this_start);
            if (!upd.run()) {
                printf("; update slice size error\n");
            }
            //size = this_end - addr;
            //break;
        // Achei um slice depois do meu, entao vou me aparar
        } else if (addr < this_start) {
            size = this_start - addr;
            break;
        } else {
            printf("SLICE POSITIONING ERROR\n");
        }
    }
    // Insert this slice
    sqlq_t ins(img->db, "INSERT INTO tab_modslice(address, size, module, segment) VALUES(@A, @B, @C, @D)");
    ins.bind_int(1, addr);
    ins.bind_int(2, size);
    ins.bind_int(3, modid);
    ins.bind_int(4, segid);
    if (!ins.run()) {
        printf("; Create slice error\n");
    } else {
        //printf("; Created slice %s\n", name);
    }
    #endif

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

void IMG_fixlabels(Image_t *img, int secid) {
    assert(img != NULL);

    Section_t *sec = IMG_getsection(img, secid);
    assert(sec->plan != NULL);
    const uint8_t *plan = sec->plan;
    IMG_begintransaction(img);
    sqlq_t q(img->db, "SELECT labid, address FROM tab_label WHERE address >= @AA AND address < @BB");
    q.bind_int(1, uaddr_mk(secid, 0));
    q.bind_int(2, uaddr_mk(secid + 1, 0));
    while(q.step()) {
        int labid = q.col_int(0);
        uaddr_t laddr = q.col_int(1);
        int disp = 0;
        int off = uaddr_off(laddr);
        while (plan[off - disp] & PLAN_INSIDE) {
            disp++;
        }
        if (disp != 0) {
            //printf("fixed 1 label\n");
            IMG_movelabel(img, labid, laddr, laddr - disp);
        }
    }
    IMG_endtransaction(img);
}

void IMG_addrlabels(Image_t *img, int secid) {
    assert(img != NULL);
    Section_t *sec;
    x86_insn_t insn;

    sec = IMG_getsection(img, secid);
    if (sec->plan == NULL) {
        return;
    }
    #if 0
    sec->data = (uint8_t *)BUF_getptr(sec->buf_data);
    printf("%d %02X\n", secid, sec->data[0]);
    printf("%d %02X\n", 1, *((uint8_t *)IMG_getptr(img, uaddr_mk(1, 0))));
    return;
    #endif
    uaddr_t min_target = uaddr_mk(secid, 0);
    uaddr_t max_target = uaddr_mk(secid, IMG_getsecsize(img, secid));
    IMG_begintransaction(img);
    //for (int pos = 0; pos < 349; pos++) {
    int count = 0;
    for (int pos = 0; pos < sec->size; pos++) {
        int len = plan_size(sec->plan, pos);
        if (PLAN_ISCODE(sec->plan, pos)) {
            x86_disasm((unsigned char *)sec->data, sec->size, uaddr_mk(secid, 0), pos, &insn);
            x86_op_t *op = x86_get_branch_target(&insn);
            int rel;
            int relsize = 0;
            bool found = false;
            // ve se essa instrucao tem algum operando
            if (op != NULL) {
                // so nos interessam operandos relativos (jumps)
                if (op->type == op_relative_near) {
                    found = true;
                    rel = (int)op->data.relative_near;
                    relsize = 1;
                } else if (op->type == op_relative_far) {
                    found = true;
                    rel = op->data.relative_far;
                    relsize = 4;                   //// FIXME pode ser 2 em 16 bits
                }
            }
            if (found) {
                int disp = 0;
                uaddr_t target = insn.addr + insn.size + rel;
                //printf("%d ADD LABEL L%08X\n", pos, target);
                //IMG_addlabel(img, target, NULL);
                // Se necessário, desloca o label até o início de uma instrução
                if ((target < min_target) || (target >= max_target)) {
                    printf("ERROR: relative jump/call at %08X points to %08X\n", uaddr_mk(secid, pos), target);
                    x86_oplist_free(&insn);
                    IMG_endtransaction(img);
                    return;
                }
        #if 1
                for (;;) {
                    uint8_t uuu = sec->plan[uaddr_off(target)];
                    //if (!((uuu & CP_INSIDE) || (uuu == CP_DATA2))) {
                    if (!(uuu & PLAN_INSIDE)) {
                        break;
                    }
                    disp++;
                    target--;
                }
        #endif
                //printf("%d\n", disp);
                //printf("%08X %08X\n", uaddr_mk(secid, pos), target);
                //printf("%08X %08X\n", uaddr_mk(secid, pos), target);
        #if 1
                uaddr_t afrom = uaddr_mk(secid, pos) + 1;
                if (sec->data[pos] == 0x0F) {
                    afrom += 1;
                }
                IMG_addreloc(img, afrom, target, relsize, disp, RELOC_REL); // FIXME::: SIZE 0??
        #endif
                count++;
            }
            x86_oplist_free(&insn);
        }
        pos += len - 1;
    }
    IMG_endtransaction(img);
    //printf("; %d labels added\n", count);
}

void IMG_setmodules(Image_t *img) {
    assert(img != NULL);
    
    IMG_begintransaction(img);
    sqlq_t slices(img->db, "SELECT module, address, size FROM tab_modslice");
    while(slices.step()) {
        int modid = slices.col_int(0);
        uaddr_t addr = slices.col_int(1);
        int size = slices.col_int(2);
        sqlq_t label(img->db, "UPDATE tab_label SET module=@A WHERE address >= @B AND address < @C");
        //printf("slice %d %08X %d\n", modid, addr, size);
        label.bind_int(1, modid);
        label.bind_int(2, addr);
        label.bind_int(3, addr + size);
        label.run();
        sqlq_t reloc(img->db, "UPDATE tab_reloc SET module=@A WHERE address >= @B AND address < @C");
        reloc.bind_int(1, modid);
        reloc.bind_int(2, addr);
        reloc.bind_int(3, addr + size);
        reloc.run();
    }
    IMG_endtransaction(img);
}

void IMG_setsegments(Image_t *img) {
    assert(img != NULL);
    
    IMG_begintransaction(img);
    sqlq_t q(img->db, "SELECT segid, address, size FROM tab_segment");
    while(q.step()) {
        int segid = q.col_int(0);
        uaddr_t addr = q.col_int(1);
        int size = q.col_int(2);
        sqlq_t label(img->db, "UPDATE tab_label SET segment=@A WHERE address >= @B AND address < @C");
        //printf("setting segment %d %08X %d\n", segid, addr, size);
        label.bind_int(1, segid);
        label.bind_int(2, addr);
        label.bind_int(3, addr + size);
        label.run();
        sqlq_t reloc(img->db, "UPDATE tab_reloc SET segment=@A WHERE address >= @B AND address < @C");
        reloc.bind_int(1, segid);
        reloc.bind_int(2, addr);
        reloc.bind_int(3, addr + size);
        reloc.run();
    }
    IMG_endtransaction(img);
}

void IMG_setlabname(Image_t *img, uaddr_t addr, const char *name, int type) {
    assert(img != NULL);

    //printf("AAA\n");
    
    sqlq_t q(img->db, "UPDATE tab_label SET name=@A, type=@D WHERE address=@B");
    q.bind_str(1, name);
    q.bind_int(2, type);
    q.bind_int(3, addr);
    if (!q.run()) {
        printf("Error setting %08X name to %s\n", addr, name);
    }
}
