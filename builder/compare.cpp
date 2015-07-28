#include <cstring>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <libdis.h>
#include "defs.h"
#include "image.h"
#include "sqlq.h"

// FIXME: 1009438C (27238C) ta com problem (devia ser dado mas eh codigo)

// problem with MODRM?   2B C2
//                       2B D0
static bool modrmproblem(const uint8_t *a, const uint8_t *b, const uint8_t *cpdata, int i) {
    if (!PLAN_ISCODE(cpdata, i)) {
        return false;
    }
    uint8_t modrma = a[i + 1];
    uint8_t modrmb = b[i + 1];
    if (a[i] == b[i]) {
        if (modrma == modrmb) {
            return true;
        }
    }
    if ((a[i] == (b[i] ^ 0x02)) || (a[i] == 0x86) || (a[i] == 0x87)) {
        //if (a[i] == 0x87) {
        //    printf("oiz\n");
        //}
        if ((modrma & modrmb & 0xC0) == 0xC0) {
            modrma = 0xC0 | ((modrma << 3) & 0x38) | ((modrma >> 3) & 7);
            if (modrma == modrmb) {
                return true;
            }
        }
    }
    if (a[i] == 0x66) {
        modrma = a[i + 2];
        modrmb = b[i + 2];
        if (a[i + 1] == b[i + 1]) {
            if (modrma == modrmb) {
                return true;
            }
        } else if (a[i + 1] == (b[i + 1] ^ 0x02)) {
            if ((modrma & modrmb & 0xC0) == 0xC0) {
                modrma = 0xC0 | ((modrma << 3) & 0x38) | ((modrma >> 3) & 7);
                if (modrma == modrmb) {
                    return true;
                }
            }
        }
    }
    modrma = a[i + 2];
    modrmb = b[i + 2];
    if (modrma == (modrmb ^ 0x80)) {
        if ((a[i + 3] == 0x45) && (b[i + 3] == 0x00)) {
            return true;
        }
    }
    return false;
}

bool IMG_comparesection(Image_t *img, int secid, int size, void *data) {
    assert(img != NULL);
    const char *tail = 0;

    bool skipjumps = true;

    Section_t *sec = &img->sections[secid];
    Buffer_t *cplan = sec->buf_plan;
    const uint8_t *cpdata = NULL;
    if (cplan) {
        cpdata  = (const uint8_t*)BUF_getptr(cplan);
    }
    if (size != sec->size) {
        printf("Section %d sizes differ: %d (original) and %d (rebuilt)\n", secid, sec->size, size);
    }
    if (size > sec->size) {
        size = sec->size;
    }
    if (data == NULL) {
        return true;
    }
    sqlq_t qrel(img->db, "SELECT address, size FROM tab_reloc "
                         "WHERE type = 0 AND address >= @A AND address < @B ORDER BY address ASC", 0, 0);
    qrel.bind_int(1, uaddr_mk(secid, 0));
    qrel.bind_int(2, uaddr_mk(secid + 1, 0));
    int qrel_addr = qrel.step();

    const uint8_t *a = (const uint8_t *)IMG_getptr(img, uaddr_mk(secid, 0));
    const uint8_t *b = (const uint8_t *)data;

/////////////////////////////
    for (int i = 0; i < size; i++) {
        if (qrel_addr == uaddr_mk(secid, i)) {
            int rel_size = qrel.col_int(1);
            i += rel_size - 1;
            qrel_addr = qrel.step();
        } else if (a[i] != b[i]) {
            bool problem = true;
            if (cplan) {
                #if 1
                if (skipjumps && PLAN_ISCODE(cpdata, i - 1)) {
                    switch(a[i-1]) {
                        case 0x72:
                        case 0x73:
                        case 0x74:
                        case 0x75:
                        case 0x76:
                        case 0x77:
                        case 0xEB:
                            problem = false;
                            break;
                    }
                }
                if (problem) {
                    for (int k = 1; k < 3 && problem; k++) {
                        switch(a[i-k]) {
                            case 0x84:
                            case 0x85:
                            case 0x8E:
                            case 0x8F:
                                if (a[i-k-1] == 0x0F) {
                                    if (PLAN_ISCODE(cpdata, i-k-1)) {
                                        problem = false;
                                    }
                                }
                                break;
                            case 0xE8:
                            case 0xE9:
                                if (PLAN_ISCODE(cpdata, i-k)) {
                                    problem = false;
                                }
                                break;
                        }
                    }
                }
                #endif
////////////////////////////
                if (problem) {
                    if (modrmproblem(a, b, cpdata, i)) {
                        problem = false;
                    }
                    else if (modrmproblem(a, b, cpdata, i - 1)) {
                        problem = false;
                    }
                    else if (modrmproblem(a, b, cpdata, i - 2)) {
                        problem = false;
                    }
                    else if (modrmproblem(a, b, cpdata, i - 3)) {
                        problem = false;
                    }
                }

                if (a[i] == 0x26 && b[i] == 0x66) {
                    if (a[i - 1] == 0x66 && b[i - 1] == 0x26) {
                        if (PLAN_ISCODE(cpdata, i-1)) {
                            problem = false;
                        }
                    }
                } else if (a[i] == 0x66 && b[i] == 0x26) {
                    if (a[i + 1] == 0x26 && b[i + 1] == 0x66) {
                        if (PLAN_ISCODE(cpdata, i)) {
                            problem = false;
                        }
                    }
                }
                if (a[i] == 0x36 && b[i] == 0x66) {
                    if (a[i - 1] == 0x66 && b[i - 1] == 0x36) {
                        if (PLAN_ISCODE(cpdata, i-1)) {
                            problem = false;
                        }
                    }
                } else if (a[i] == 0x66 && b[i] == 0x36) {
                    if (a[i + 1] == 0x36 && b[i + 1] == 0x66) {
                        if (PLAN_ISCODE(cpdata, i)) {
                            problem = false;
                        }
                    }
                }
                if (a[i] == 0x2E && b[i] == 0x66) {
                    if (a[i - 1] == 0x66 && b[i - 1] == 0x2E) {
                        if (PLAN_ISCODE(cpdata, i-1)) {
                            problem = false;
                        }
                    }
                } else if (a[i] == 0x66 && b[i] == 0x2E) {
                    if (a[i + 1] == 0x2E && b[i + 1] == 0x66) {
                        if (PLAN_ISCODE(cpdata, i)) {
                            problem = false;
                        }
                    }
                }
            }
            if (problem) {
                // 14 45 addr  -- 00 010 100 = SIB MODE            45 = sdword+ eax*2
                // 94 00 addr  -- 10 010 100 = SIB + disp32 MODE   00 = eax * 1 + eax
                uaddr_t praddr = uaddr_mk(secid, i);
                printf("Difference found at %08X (%08X).\n", praddr, addr_to_ida(praddr));
                printf("                                   **");
                printf("\nOriginal: ");
                for (int j = 0; j < 32; j++) {
                    printf(" %02X", a[i + j - 8]);
                }
                printf("\nRebuilt:  ");
                for (int j = 0; j < 32; j++) {
                    printf(" %02X", b[i + j - 8]);
                }
                printf("\n");
                return false;
            }
        }
    }
    return true;
}

void IMG_comparestats(Image_t *img, int num_sections, int num_relocs) {
    assert(img != NULL);

    sqlq_t q(img->db, "SELECT COUNT(*) FROM tab_reloc");
    int oldrels = q.answer();
    printf("old relocations: %d       new relocations %d\n", oldrels, num_relocs);
    printf("old numsections: %d       new numsections %d\n", img->numsections, num_sections);
}

bool IMG_comparereloc(Image_t *img, uaddr_t afrom, uaddr_t ato, int fix_size) {
    assert(img != NULL);
    bool ret = true;
    char err[200] = "";

    sqlq_t q(img->db, "SELECT r.size, r.disp, l.address  FROM tab_reloc AS r "
                      "INNER JOIN tab_label AS l ON r.label = l.labid "
                      "WHERE r.address = @AFROM");// AND ((l.address + r.disp = @ATO) OR (l.address = 0)) AND r.size = @SZ");
    q.bind_int(1, afrom);
    sprintf(err, "Relocation mismatch at %08X (%06X):\n", afrom, addr_to_ida(afrom));
    if (!q.step()) {
        printf("%s", err);
        printf("    * Relocation not found in original\n");
        ret = false;
    } else {
        bool errprinted = false;
        int size = q.col_int(0);
        int disp = q.col_int(1);
        uaddr_t laddr = q.col_int(2) + disp;
        if (fix_size != size) {
            if (!errprinted) {
                errprinted = true;
                printf("%s", err);
            }
            printf("    * Original size = %d and rebuilt size = %d\n", size, fix_size);
            ret = false;
        }
        if (ato != laddr) {
            if (!errprinted) {
                errprinted = true;
                printf("%s", err);
            }
            printf("    * Original target = %08X(%06X) and rebuilt target = %08X(%06X)\n", laddr, addr_to_ida(laddr), ato, addr_to_ida(ato));
            ret = false;
        }
    }
    return ret;
    #if 0




    if (!q.step()) {
        printf("Relocation mismatch at %08X (%08X) --> rebuilt points to %08X (%08X)\n", afrom, afrom - uaddr_mk(1, 0) + 0x1DE000,
            ato, ato - uaddr_mk(4, 0) + 0x2C6770);
        ret = false;
    }
    return ret;
    #endif
 
    #if 0
    const char *tail = 0;
    sqlite3_stmt *stmt;
    bool ret = true;

    sqlite3_prepare_v2(img->db, "SELECT * FROM tab_reloc AS r "
                                "INNER JOIN tab_label AS l ON r.label = l.labid "
                                "WHERE r.address = @AFROM AND ((l.address + r.disp = @ATO) OR (l.address = 0)) AND r.size = @SZ",
                                -1, &stmt, &tail);
    sqlite3_bind_int(stmt, 1, afrom);
    sqlite3_bind_int(stmt, 2, ato);
    sqlite3_bind_int(stmt, 3, fix_size);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        printf("Relocation mismatch at %08X (%08X) --> rebuilt points to %08X (%08X)\n", afrom, afrom - uaddr_mk(1, 0) + 0x1DE000,
            ato, ato - uaddr_mk(4, 0) + 0x2C6770);
        ret = false;
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return ret;
    #endif
    return true;
}
