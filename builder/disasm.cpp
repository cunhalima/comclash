#include <cstring>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <libdis.h>
#include "defs.h"
#include "image.h"
#include "sqlq.h"

// After files get smaller, this is going to be nice
//static char vim_modeline[] = "; vim:syntax=nasm filetype=nasm\n";
static char vim_modeline[] = "";//; vim:syntax=nasm filetype=nasm\n";
//static char vim_modeline[] = "; vim:syntax=nasm\n";
//static char vim_modeline[] = "; vim:filetype=nasm\n";

class Disasm {
private:
    Image_t *img;
    Buffer_t *curr_string;
    char dgroup[256];
    char modname[64];
    char filename[64];
    char labname[64];
    void getfilename(int modid);
    void copyfiles(void);
    void copyfile(const char *name);
    void makemkfile(void);
    void disasmmodules(void);
    void disasmslices(void);
    void disasmstack(void);
    void writeexterns(int modid);
    void writeglobals(int modid);
    void makelabname(uaddr_t addr, const char *name, int disp = 0, int type = RELOC_ABS, int size = 0);
    void printseg(int segid, bool detail = false);
    void makedgroup(void);
    void disasmslice(int modid, uaddr_t start, uaddr_t end);
    void qlab_next(void);
    void qrel_next(void);
    void qseg_next(void);
    void trywritelabel(uaddr_t addr);
    void build_string(const uint8_t *str, int size);
    int disasmcode(uaddr_t addr, int offset, const uint8_t *data, const uint8_t *plan);
    int disasmdata(uaddr_t addr, int offset, const uint8_t *data, const uint8_t *plan);
    int disasmvirt();
    void dischargevirtual(void);
    void linestart(void);
    int curr_secsize;
    int virt_bytes;
    int dump_col;
    bool first_slice;
    FILE *fd;
    sqlq_t qseg;
    sqlq_t qlab;
    sqlq_t qrel;

    uaddr_t qlab_addr;
    const char *qlab_name;
    uaddr_t qrel_addr;
    uaddr_t qrel_laddr;
    uaddr_t qrel_size;
    uaddr_t qrel_disp;
    uaddr_t qrel_type;
    const char *qrel_lname;
    uaddr_t qseg_addr;
    uaddr_t qseg_paddr;
    int qseg_segid;
    int qseg_psegid;
public:
    Disasm(void);
    ~Disasm(void);
    void run(Image_t *p_img);
};

void Disasm::makelabname(uaddr_t addr, const char *name, int disp, int type, int size) {
    char *s = labname;
    s[0] = '\0';
    if (type == RELOC_ABS && size == 2) {
        strcat(s, "seg ");
        s += 4;
    }
    if (name == NULL) {
        sprintf(s, "L%08X", addr);
    } else {
        strcpy(s, name);
    }
    s = strchr(s, '\0');
    if (disp != 0) {
        sprintf(s, "%+d", disp);
    }
}

void Disasm::getfilename(int modid) {
    sqlq_t q(img->db, "SELECT name FROM tab_module WHERE modid=@A");
    q.bind_int(1, modid);
    q.step();
    strcpy(modname, q.col_str(0));
    sprintf(filename, "src/%s.asm", modname);
}

void Disasm::writeexterns(int modid) {
    #if 0
    // EXTERN --- referenced by relocs with our modid to labels with different modid
    sqlq_t x(img->db, "SELECT DISTINCT l.address, l.name FROM tab_label AS l INNER JOIN tab_reloc AS r "
                      "ON r.label = l.labid "
                      "WHERE (r.module = @A) AND ((l.module <> r.module) OR ((l.module IS NULL) AND (l.address <= 1))) "
                      "ORDER BY l.address ASC"); // hack edata e end addr 0 e 1
                      //"WHERE (r.module = @A) AND ((l.module IS NOT NULL) OR (l.address <= 1)) AND (l.module <> r.module)"); // hack edata e end addr 0 e 1
    x.bind_int(1, modid);
    while(x.step()) {
        makelabname(x.col_int(0), x.col_str(1));
        fprintf(fd, "extern %s\n", labname);
    }
    #endif
    // EXTERN --- referenced by relocs with our modid to labels with different modid
    // FIXME coloquei type 16 pra nao gerar extern pras constantes LABEL_CONSTANT
    sqlq_t x(img->db, "SELECT l.address, l.name, s.name, r.address "
                      "FROM tab_label AS l "
                      "INNER JOIN tab_reloc AS r ON r.label = l.labid "
                      "INNER JOIN tab_segment AS s ON l.segment = s.segid "
                      "WHERE (r.module = @A) AND ((l.module <> r.module) OR ((l.module IS NULL))) "
                      "ORDER BY l.address ASC"); // hack edata e end addr 0 e 1
                      //"WHERE (r.module = @A) AND ((l.module IS NOT NULL) OR (l.address <= 1)) AND (l.module <> r.module)"); // hack edata e end addr 0 e 1
                      //"WHERE (r.module = @A) AND ((l.module <> r.module) OR ((l.module IS NULL) AND (l.type < 16))) "
    x.bind_int(1, modid);
    uaddr_t last_laddr = -1;
    while(x.step()) {
        uaddr_t laddr;
        if (x.col_null(0)) {
            laddr = -1;
        } else {
            laddr = x.col_int(0);
            if (laddr == last_laddr) {
                continue;
            }
            last_laddr = laddr;
        }
        const char *lname = x.col_str(1);
        const char *sname = x.col_str(2);
        uaddr_t raddr = x.col_int(3);
        makelabname(laddr, lname);
        #if 0
        if (uaddr_sec(laddr) == 3 || uaddr_sec(laddr) == 4 || uaddr_sec(laddr) == 1) {
            //fprintf(fd, "extern %s:wrt DGROUP\n", labname);
            fprintf(fd, "extern %s\n", labname);
        } else {
            fprintf(fd, "extern %s:wrt %s\n", labname, sname);
        }
        #endif
        #if 0
        // HORRIBLE HACK FIXME
        int lsec = uaddr_sec(laddr);
        int rsec = uaddr_sec(raddr);
        if ((lsec == 2) || ((rsec == 3) && (lsec == 1))) {
            fprintf(fd, "extern %s:wrt %s\n", labname, sname);
        } else {
            fprintf(fd, "extern %s\n", labname);
        }
        #endif
            fprintf(fd, "extern %s:wrt %s\n", labname, sname);
    }
}

void Disasm::writeglobals(int modid) {
    // LABEL_PUBLIC = 1 se for diferente, mudar o SQL Abaixo!!! FIXME
    // GLOBAL --- referenced by relocs with different modid to our labels (our modid)
    sqlq_t x(img->db, "SELECT DISTINCT l.address, l.name FROM tab_label AS l INNER JOIN tab_reloc AS r "
                      "ON r.label = l.labid "
                      "WHERE (l.module = @A) AND ((l.type = 1) OR (l.module <> r.module)) ORDER BY l.address ASC");
    x.bind_int(1, modid);
    while(x.step()) {
        makelabname(x.col_int(0), x.col_str(1));
        fprintf(fd, "global %s\n", labname);
    }
}

void Disasm::disasmmodules(void) {
    //printf("modules\n");
    sqlq_t modules(img->db, "SELECT modid FROM tab_module");
    while (modules.step()) {
        int modid = modules.col_int(0);
        getfilename(modid);
        //printf("%s\n", filename);
        fd = fopen(filename, "w");
        fprintf(fd, "%s", vim_modeline);
        fprintf(fd, ";---------------------------------\n");
        fprintf(fd, "; %s\n", modname);
        fprintf(fd, ";---------------------------------\n");
        fprintf(fd, "        cpu 386\n");
        fprintf(fd, "%%include \"xinst.inc\"\n");
        fprintf(fd, "%%include \"segs.inc\"\n");
        writeexterns(modid);
        writeglobals(modid);
        fclose(fd);
    }
}

void Disasm::printseg(int segid, bool detail) {
    //fprintf(fd, "SEG %d\n", segid);
    //sqlq_t q(img->db, "SELECT name FROM tab_segment WHERE segid=@A");
    //q.bind_int(1, segid);
    //q.step();
    //linestart();
    //fprintf(fd, "[segment %s]\n", q.col_str(0));
    sqlq_t q(img->db, "SELECT name, class, align, use FROM tab_segment WHERE segid=@A");
    q.bind_int(1, segid);
    if (!q.step()) {
        printf("ERROR!!\n");
        return;
    }
    linestart();

    const char *name = q.col_str(0);
    if (detail) {
        const char *sclass = q.col_str(1);
        int align = q.col_int(2);
        int use = q.col_int(3);

        const char *type = "";
        const char *suse = "";
        const char *spublic= "public";
        if (strcmp(sclass, "STACK") == 0) {
            type = "stack ";
        }
        if (use == 32) {
            suse = " use32";
        } else if (use == 16) {
            suse = " use16";
            //spublic = "private";
        }
        fprintf(fd, "[segment %-12s %s align=%d %sclass=%s%s]\n", name, spublic, align, type, sclass, suse);
    } else {
        fprintf(fd, "[segment %s]\n", name);
    }
}

void Disasm::qseg_next(void) {
    if (qseg_addr == 0) {
        return;
    }
    qseg_paddr = qseg_addr;
    qseg_psegid = qseg_segid;
    if (qseg.step()) {
        qseg_segid = qseg.col_int(0);
        qseg_addr = qseg.col_int(1);
    } else {
        qseg_segid = 0;
        qseg_addr = 0;
    }
}

void Disasm::qlab_next(void) {
    if (qlab_addr == 0) {
        return;
    }
    if (qlab.step()) {
        qlab_addr = qlab.col_int(0);
        qlab_name = qlab.col_str(1);
    } else {
        qlab_addr = 0;
        qlab_name = NULL;
    }
}

void Disasm::qrel_next(void) {
    if (qrel_addr == 0) {
        return;
    }
    if (qrel.step()) {
        qrel_addr = qrel.col_int(0);
        qrel_laddr = qrel.col_int(1);
        qrel_lname = qrel.col_str(2);
        qrel_size = qrel.col_int(3);
        qrel_disp = qrel.col_int(4);
        qrel_type = qrel.col_int(5);
    } else {
        qrel_addr = 0;
        qrel_laddr = 0;
        qrel_lname = NULL;
        qrel_size = 0;
        qrel_disp = 0;
        qrel_type = 0;
    }
}

void Disasm::trywritelabel(uaddr_t addr) {
    // Write labels
    if (qlab_addr == addr) {
        makelabname(qlab_addr, qlab_name);
        linestart();
        dischargevirtual();
        fprintf(fd, "%s:\n", labname);
        qlab_next();
    }
}

#define MAX_DUMP_COLS 16

void Disasm::linestart(void) {
    if (dump_col > 0) {
        dump_col = 0;
        fprintf(fd, "\n");
    }
}

void Disasm::dischargevirtual(void) {
    if (virt_bytes > 0) {
        linestart();
        fprintf(fd, "        resb      %d\n", virt_bytes);
        virt_bytes = 0;
    }
}

void Disasm::build_string(const uint8_t *str, int size) {
    int k = 0;
    int state = 0;
    bool first = true;
    BUF_setsize(curr_string, 16);
    char *out = (char *)BUF_getptr(curr_string);
    while(size--) {
        BUF_setsize(curr_string, k + 16);
        out = (char *)BUF_getptr(curr_string);
        char c = *str++;
        if (my_isprintable(c)) {
            if (state == 0) {
                if (!first) {
                    out[k++] = ',';
                    out[k++] = ' ';
                }
                out[k++] = '\"';
                state = 1;
            }
            out[k++] = c;
        } else {
            if (state == 1) {
                out[k++] = '\"';
                state = 0;
            }
            if (!first) {
                out[k++] = ',';
                out[k++] = ' ';
            }
            sprintf(&out[k], "0x%02X", c);
            k += 4;
        }
        first = false;
    }
    if (state == 1) {
        out[k++] = '\"';
    }
    out[k] = '\0';
}

int Disasm::disasmdata(uaddr_t addr, int offset, const uint8_t *data, const uint8_t *plan) {
    if (qrel_addr == addr) {
        makelabname(qrel_laddr, qrel_lname, qrel_disp, qrel_type, qrel_size);
        linestart();
        fprintf(fd, "        d");
        switch(qrel_size) {
            case 4:  fprintf(fd, "d      "); break;
            case 2:  fprintf(fd, "w      "); break;
            default: fprintf(fd, "b      "); break;
        }
        fprintf(fd, "%s\n", labname);
        int size = qrel_size;
        qrel_next();
        return size;
    }
    if (plan) {
        if (PLAN_HASSIZE(plan, offset)) {
            int size = plan_size(plan, offset);
            linestart();
            if (PLAN_ISSTR(plan, offset)) {
                build_string(&data[offset], size);
                fprintf(fd, "        db      %s\n", (const char *)BUF_getptr(curr_string));
            } else {
                fprintf(fd, "        db      ");
                for (int i = 0; i < size; i++) {
                    if (i != 0) {
                        fprintf(fd, ",");
                    }
                    fprintf(fd, "0x%02X", data[offset+i]);
                }
                if (PLAN_ISALIGN(plan, offset)) {
                    fprintf(fd, " ; align");
                }
                fprintf(fd, "\n");
            }
            return size;
        }
        //printf("ERROR NOT DONE YET\n");
    }
    if (dump_col == 0) {
        fprintf(fd, "        db      ");
    } else {
        fprintf(fd, ",");
    }
    fprintf(fd, "0x%02X", data[offset]);
    dump_col++;
    if (dump_col == MAX_DUMP_COLS) {
        linestart();
    }
    return 1;
}

int Disasm::disasmcode(uaddr_t addr, int offset, const uint8_t *data, const uint8_t *plan) {
    // TODO
    // Engole os relocs:
    char disasm_line[256];
    x86_insn_t insn;
    int instlen = plan_size(plan, offset);
    while (qrel_addr != 0 && qrel_addr < addr + instlen) {
        makelabname(qrel_laddr, qrel_lname, qrel_disp, qrel_type, qrel_size);
        // UGLY HACK
        uint32_t mark;
        if (qrel_size == 4) {
            mark = uaddr_off(qrel_laddr + 1);
            *((uint32_t *)(data+uaddr_off(qrel_addr))) = mark;
        } else if (qrel_size == 2) {
            mark = uaddr_off(qrel_laddr + 1) & 0xFFFF;
            *((uint16_t *)(data+uaddr_off(qrel_addr))) = mark;
        } else {
            mark = qrel_addr - (addr + instlen);
        }
        x86_reloc_send(mark, labname);
        qrel_next();
    }
    x86_disasm((unsigned char *)data, curr_secsize, 0, offset, &insn);
    x86_format_insn(&insn, disasm_line, sizeof(disasm_line), intel_syntax);
    x86_oplist_free(&insn);
    if (!x86_reloc_check()) {
        printf("!!!!!!! RELOCATION PROBLEM\n");
    }
    x86_reloc_reset();
    linestart();
    fprintf(fd, "        %s\n", disasm_line);
    return instlen;
}

int Disasm::disasmvirt(void) {
    virt_bytes++;
    return 1;
}

void Disasm::disasmstack(void) {
    sprintf(filename, "src/stack.asm");
    fd = fopen(filename, "w");

    fprintf(fd, "%s", vim_modeline);
    fprintf(fd, ";---------------------------------\n");
    fprintf(fd, "; stack\n");
    fprintf(fd, ";---------------------------------\n");
    fprintf(fd, "        cpu 386\n");
    fprintf(fd, "%%include \"segs.inc\"\n");

    sqlq_t q(img->db, "SELECT segid FROM tab_segment WHERE class='STACK'");
    q.step();
    int segid = q.col_int(0);
    printseg(segid, false);

    fprintf(fd, "        resb    0x1000\n");
    fclose(fd);
}

void Disasm::makedgroup(void) {
    sprintf(filename, "src/segs.inc");
    fd = fopen(filename, "w");
    sqlq_t q(img->db, "SELECT segid, name, class, use FROM tab_segment ORDER BY address, priority ASC");
    dgroup[0] = '\0';
    while(q.step()) {
        int segid = q.col_int(0);
        const char *name = q.col_str(1);
        const char *sclass = q.col_str(2);
        int use = q.col_int(3);

        const char *type = "";
        if (strcmp(sclass, "STACK") == 0) {
            type = "stack ";
        }
        if ((strstr(sclass, "DATA") != NULL) ||
            (strstr(sclass, "BSS") != NULL) ||
            (type[0] != '\0')) {
            if (dgroup[0] != '\0') {
                strcat(dgroup, " ");
            }
            strcat(dgroup, name);
        }
        // HACK
        //if (use != 16) {
            printseg(segid, true);
        //}
    }
    fprintf(fd, "group DGROUP %s\n\n", dgroup);
    fclose(fd);
}

void Disasm::disasmslice(int modid, uaddr_t start, uaddr_t end) {
    int secid = uaddr_sec(start);
    assert(uaddr_sec(end) == secid);
    Section_t *sec = IMG_getsection(img, secid);
    curr_secsize = sec->size;
    const uint8_t *plan = sec->plan;
    const uint8_t *data = sec->data;
    dump_col = 0;
    virt_bytes = 0;
    fprintf(fd, "        [bits %d]\n", sec->use);
    if (start != qseg_addr) {
        printseg(qseg_psegid);
    }
    for (uaddr_t addr = start; addr < end; addr++) {
        if (addr == qseg_addr) {
            while(addr == qseg_addr) {
                // HACK
                //if (sec->use == 16) {
                //    printseg(qseg_segid, true);
                //} else {
                    printseg(qseg_segid);
                //}
                qseg_next();
            }
        }
        int offset = uaddr_off(addr);
        trywritelabel(addr);
        int len = -1;
        if (sec->virt) {
            len += disasmvirt();
        } else if ((plan != NULL) && (PLAN_ISCODE(plan, offset))) {
            len += disasmcode(addr, offset, data, plan);
        } else {
            len += disasmdata(addr, offset, data, plan);
        }
        addr += len;
    }
    dischargevirtual();
}

void Disasm::disasmslices(void) {
    //printf("slices\n");
    // slices query
    sqlq_t slices(img->db, "SELECT module, address, size FROM tab_modslice");
    // segment query
    qseg.prepare(img->db, "SELECT segid, address FROM tab_segment ORDER BY address, priority ASC");
    qseg_addr = 1;
    qseg_segid = 0;
    qseg_next();
    qseg_paddr = qseg_addr;
    qseg_psegid = qseg_segid;
    // labels query
    qlab.prepare(img->db, "SELECT address, name FROM tab_label WHERE address >= @A ORDER BY address ASC");
    qlab.bind_int(1, uaddr_mk(1, 0));
    qlab_addr = 1;
    qlab_next();
    // relocs query
    qrel.prepare(img->db, "SELECT r.address, l.address, l.name, r.size, r.disp, r.type "
                          "FROM tab_reloc AS r INNER JOIN tab_label as l ON r.label = l.labid "
                          "ORDER BY r.address ASC");
    qrel_addr = 1;
    qrel_next();
    while (slices.step()) {
        int modid = slices.col_int(0);
        uaddr_t addr = slices.col_int(1);
        int size = slices.col_int(2);
        getfilename(modid);
        //printf("%s\n", filename);
        fd = fopen(filename, "a");
        disasmslice(modid, addr, addr + size);
        fclose(fd);
    }
    disasmstack();
}

void Disasm::makemkfile(void) {
    sprintf(filename, "src/Makefile");
    Buffer_t *fin = BUF_createfromfilecstr(filename);
    if (fin == NULL) {
        return;
    }
    const char *sin = (const char *)BUF_getptr(fin);
    bool colz = true;
    int i = 0;
    int start_eating = 0;
    int end_eating = 0;
    for (int i = 0; (sin[i] != '\0') && (end_eating == 0); i++) {
        char c = sin[i];
        if (!start_eating) {
            if (colz && c == 'S') {
                if (strncmp(&sin[i], "SSOBJ=", 6) == 0) {
                    start_eating = i;
                }
            }
            colz = false;
            if (c == '\n' || c == '\r') {
                colz = true;
            }
        } else {
            if ((c == '\n') && (sin[i - 1] != '\\') && (sin[i - 2] != '\\')) {
                end_eating = i;
            }
        }
    }
    //if (start_eating) {
        //printf("oooo\n");
    //}
    if (start_eating && end_eating) {
        fd = fopen(filename, "w");
        fwrite(sin, 1, start_eating, fd);


        fprintf(fd, "SSOBJ=");
        sqlq_t modules(img->db, "SELECT DISTINCT name FROM tab_modslice INNER JOIN tab_module "
                                "ON tab_modslice.module = tab_module.modid ORDER BY tab_modslice.address");
        bool first = true;
        while (modules.step()) {
            if (!first) {
                fprintf(fd, " \\\n    ");
            }
            first = false;
            fprintf(fd, "%s.obj ", modules.col_str(0));
        }
        fprintf(fd, " \\\n    stack.obj");


        fwrite(&sin[end_eating], 1, strlen(&sin[end_eating]), fd);
        fclose(fd);
    }
    BUF_free(fin);
    #if 0
    sprintf(filename, "src/Makefile");
    fd = fopen(filename, "w");
    //fprintf(fd, "WLINK=/home/alex/ow/binl/wlink\n");
    fprintf(fd, "WLINK=../bin/wlink\n");
    fprintf(fd, "LDFLAGS=option nodefaultlibs option dosseg option map option quiet system dos4g\n");
    fprintf(fd, "EXE=../links/ccpath/ncc.exe\n");


    fprintf(fd, "OBJ=");
    sqlq_t modules(img->db, "SELECT DISTINCT name FROM tab_modslice INNER JOIN tab_module "
                            "ON tab_modslice.module = tab_module.modid ORDER BY tab_modslice.address");
    bool first = true;
    while (modules.step()) {
        if (!first) {
            fprintf(fd, " \\\n    ");
        }
        first = false;
        fprintf(fd, "%s.obj ", modules.col_str(0));
    }
    fprintf(fd, " \\\n    stack.obj\n");


    fprintf(fd, ".PHONY: all clean\n\n");
    fprintf(fd, "all: $(EXE)\n\n");
    fprintf(fd, "clean:\n");
    fprintf(fd, "\t@rm -f $(OBJ) $(EXE)\n\n");
    fprintf(fd, "$(EXE): $(OBJ)\n");
    fprintf(fd, "\t$(WLINK) $(LDFLAGS) name $@ file { $(OBJ) }\n\n");
    fprintf(fd, "%%.obj: %%.asm\n");
    fprintf(fd, "\tnasm -O0 -f obj $<\n");
    fclose(fd);
    #endif
}

void Disasm::copyfile(const char *name) {
    sprintf(filename, "patch/meta/%s", name);
    Buffer_t *b = BUF_createfromfile(filename);
    sprintf(filename, "src/%s", name);
    BUF_savetofile(b, filename);
    BUF_free(b);
}

void Disasm::copyfiles(void) {
    //copyfile("xinst.inc");
    //copyfile("wlsystem.lnk");
    //copyfile("wlink.lnk");
    //copyfile("wstub.exe");
}

Disasm::Disasm(void) {
    curr_string = BUF_create(0);
}

Disasm::~Disasm(void) {
    BUF_free(curr_string);
}

void Disasm::run(Image_t *p_img) {
    img = p_img;
    dump_col = 0;
    first_slice = true;
    makemkfile();
    copyfiles();
    makedgroup();
    disasmmodules();
    disasmslices();
    disasmstack();
}

void IMG_disasm(Image_t *img) {
    assert(img != NULL);
    Disasm d;

    d.run(img);;
}
