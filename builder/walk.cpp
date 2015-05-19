#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <libdis.h>
#include <curses.h>
#include "defs.h"
#include "image.h"
#include "sqlq.h"

static void IMG_recordplan(Image_t *img, int secid) {
    char filename[64];
    assert(img != NULL);
    Section_t *sec = IMG_getsection(img, secid);
    // FIXME: harcoded name

    sprintf(filename, "patch/meta/%dplan.bin", secid);
    BUF_savetofile(sec->buf_plan, filename);
}

const char *perc_done(Buffer_t *b) {
    static char ccc[100];
    double p;
    uint8_t *d = (uint8_t *)BUF_getptr(b);
    int size = BUF_getsize(b);
    int count = 0;
    for (int i = 0; i < size; i++) {
        if (d[i] != 0) {
            count++;
        }
    }
    p = (double)count * 100.0 / (double)size;
    sprintf(ccc, "%.2f%% (%d/%d)", p, count, size);
    return ccc;
}

static bool my_isasc(char c) {
    return (c >= 0 && c <= 0x7E);
}

bool my_isprintable(char c) {
    if ((c >= 32 && c <= 0x7E) && (c != '\"')) {
        return true;
    }
    return false;
}

void replacezeros(uint8_t *plan, int offset, uint8_t opcode, bool singleblock) {
    if (plan[offset] != 0) {
        return;
    }
    plan[offset] = opcode;
    int i;
    for (i = 1; plan[offset + i] == 0x00; i++) {
        if (!singleblock) {
            plan[offset + i] = opcode;
        } else {
            plan[offset + i] = PLAN_INSIDE;
        }
    }
    if (singleblock) {
        plan_setsize(plan, offset, i);
    }
}

void IMG_walkplan(Image_t *img, int secid) {
    assert(img != NULL);
    WINDOW * mainwin;
    
    /*  Initialize ncurses  */
    if ((mainwin = initscr()) == NULL ) {
        fprintf(stderr, "Error initialising ncurses.\n");
        return;
    }

    mvaddstr(13, 33, "Hello, world!");
    cbreak();
    refresh();
    noecho();                  /*  Turn off key echoing                 */
    keypad(mainwin, TRUE);     /*  Enable the keypad for non-char keys  */
    int ch;
    uaddr_t first_addr;
    uaddr_t top_addr;
    first_addr = top_addr = uaddr_mk(secid, 0);
    int cur_row = 0;
    bool done = false;
    const int NUM_ROWS = 33;

    /// disasm
    char disasm_line[80];
    Section_t *sec;
    Buffer_t *cp;
    int lineno;

    sec = IMG_getsection(img, secid);
    int bufsize = BUF_getsize(sec->buf_data);
    uint8_t *data = sec->data;
    cp = sec->buf_plan;
    uint8_t *cpdata = sec->plan;
    x86_insn_t insn;

    while (!done) {
        int cursor_offset = 0;
        int pos = 0;
        for (int srow = 0; srow < NUM_ROWS; srow++) {
            int cur_off = top_addr - first_addr + pos;
            bool show_asm = false;
            if (srow == cur_row) {
                attron(A_REVERSE);
                cursor_offset = cur_off;
            }
            // print label
            mvprintw(srow, 0, "%08X", top_addr + pos);
            printw("(%06X):", addr_to_ida(top_addr + pos));
            // print hex dump
            int showlen = plan_size(cpdata, cur_off);
            for (int i = 0; i < 7; i++) {
                if (i < showlen) {
                    printw("%02X ", cpdata[cur_off + i]);
                } else {
                    printw("   ");
                }
            }
            for (int i = 0; i < 7; i++) {
                if (i < showlen) {
                    printw("%02X ", data[cur_off + i]);
                } else {
                    printw("   ");
                }
            }
            // print disasm
            if (PLAN_HASSIZE(cpdata, cur_off)) {
                if (PLAN_ISCODE(cpdata, cur_off)) {
                    int instlen = x86_disasm(data, sec->size, first_addr, cur_off, &insn);
                    if (instlen > 0 && instlen == showlen) {
                        x86_format_insn(&insn, disasm_line, sizeof(disasm_line), intel_syntax);
                        printw("%s\n", disasm_line);
                    } else {
                        printw("db      0x%02X ; Invalid instruction\n", data[cur_off]);
                    }
                    x86_oplist_free(&insn);
                } else {
                    printw("db      ");
                    for (int i = 0; (i < 10) && (i < showlen); i++) {
                        if (i != 0) {
                            printw(",");
                        }
                        printw("0x%02X", cpdata[cur_off + i]);
                    }
                    printw("                                    ");
                }
            } else {
                if (cpdata[cur_off] == 0x00) {
                    printw("db      0x%02X ; !!!!!!!!! UNKNOWNNNN             ", data[cur_off]);
                } else {
                    printw("db      0x%02X                                    ", data[cur_off]);
                }
            }
            #if 0
            //if (cpdata[cur_off] == CP_CODE_1B || (cpdata[cur_off] >= CP_CODE_MB && cpdata[cur_off] < CP_CODE_MB + 0x20)) {
            if (PLAN_ISCODE(cpdata, cur_off)) {
                show_asm = true;
            }
            int instlen = 0;
            if (show_asm) {
                instlen = x86_disasm(data, sec->size, first_addr, cur_off, &insn);
            }
            int showlen = instlen;
            if (showlen == 0) {
                showlen = 1;
                if (cpdata[cur_off] == CP_ASCII) {
                    showlen = cpdata[cur_off + 1] ^ CP_INSIDE;
                }
            }
            mvprintw(srow, 0, "%08X", top_addr + pos);
            //printw("(%06X):", top_addr + pos - uaddr_mk(1, 0) + 0x1DE000);
            printw("(%06X):", addr_to_ida(top_addr + pos));
            for (int i = 0; i < 7; i++) {
                if (i < showlen) {
                    printw("%02X ", cpdata[cur_off + i]);
                } else {
                    printw("   ");
                }
            }
            for (int i = 0; i < 7; i++) {
                if (i < showlen) {
                    printw("%02X ", data[cur_off + i]);
                } else {
                    printw("   ");
                }
            }
            if (showlen <= 7) {
                printw("  ");
            }  else {
                printw("+ ");
            }
            if (show_asm) {
                if (instlen > 0) {
                    x86_format_insn(&insn, disasm_line, sizeof(disasm_line), intel_syntax);
                    printw("%s\n", disasm_line);
                } else {
                    printw("db 0x%02X ; Invalid instruction\n", data[cur_off]);
                }
            } else {
                if (cpdata[cur_off] == CP_ASCII) {
                    bool wasprintable = false;
                    bool first = true;
                    printw("db  ", showlen);
                    for (int i = 0; i < showlen; i++) {
                        bool printable = my_isprintable(data[cur_off + i]);
                        if (printable && wasprintable) {
                            printw("%c", data[cur_off + i]);
                        } else if (printable && !wasprintable) {
                            if (!first) {
                                printw(", ");
                            }
                            printw("\"");
                            printw("%c", data[cur_off + i]);
                        } else if (!printable && wasprintable) {
                            printw("\"");
                            if (!first) {
                                printw(", ");
                            }
                            printw("0x%02X", data[cur_off + i]);
                        } else if (!printable && !wasprintable) {
                            if (!first) {
                                printw(", ");
                            }
                            printw("0x%02X", data[cur_off + i]);
                        }
                        first = false;
                        wasprintable = printable;
                    }
                    if (wasprintable) {
                        printw("\"");
                    }
                } else {
                    printw("db 0x%02X", data[cur_off]);
                    if (cpdata[cur_off] == CP_ALIGN) {
                        printw(" ; align");
                    } else if (cpdata[cur_off] == 0x00) {
                        printw(" ************************* UNKNOWN!!!");
                    }
                }
                printw("\n");
            }
            #endif
            pos += showlen;
            if (srow == cur_row) {
                attroff(A_REVERSE);
            }
        }
        mvprintw(34, 0, "Done: %s", perc_done(sec->buf_plan));
        refresh();
        ch = getch();
        int row_delta = 0;
        int kkk;
        switch(ch) {
            case 's':
                kkk = cursor_offset;
                if (cpdata[kkk] == 0x00 && data[kkk] != 0 && my_isasc(data[kkk])) {
                    cpdata[kkk++] = PLAN_STR;
                    while(cpdata[kkk] == 0 && my_isasc(data[kkk])) {
                        cpdata[kkk] = PLAN_INSIDE;
                        if (data[kkk] == 0) {
                            kkk++;
                            break;
                        }
                        kkk++;
                    }
                    plan_setsize(cpdata, cursor_offset, kkk - cursor_offset);
                    //cpdata[cursor_offset + 1] |= kkk - cursor_offset;
                }
                #if 0
                #endif
                break;
            case 'k':
            case KEY_UP:
                row_delta = -1;
                break;
            case 'j':
            case KEY_DOWN:
                row_delta = +1;
                break;
            case KEY_NPAGE:
                row_delta = NUM_ROWS;
                break;
            case KEY_PPAGE:
                row_delta = -NUM_ROWS;
                break;
            case 'g':
                {
                mvprintw(0, 0, "Go to: ");
                refresh();
                static char str[64] = "0x";
                echo();
                getstr(&str[2]);
                noecho();
                int toaddr = strtol(str, NULL, 0);
                top_addr = first_addr + uaddr_off(ida_to_addr(toaddr));
                pos = 0;
                cursor_offset = top_addr - first_addr;
                cur_row = 0;
                }
                break;
            case 'z':
                cpdata[cursor_offset] = 0;
                break;
            #if 0
            case 'a':
                if (cpdata[cursor_offset] == 0x00) {
                    cpdata[cursor_offset] = CP_ALIGN;
                }
                break;
            #endif
            case 'A':
                replacezeros(cpdata, cursor_offset, PLAN_ALIGN, true);
                break;
            case 'd':
                if (cpdata[cursor_offset] == 0x00) {
                    cpdata[cursor_offset] = PLAN_DATA;
                }
                break;
            case 'D':
                kkk = cursor_offset;
                while (cpdata[kkk] == 0x00) {
                    cpdata[kkk++] = PLAN_DATA;
                }
                break;
            case 'n':
                for (kkk = 0; kkk < 16; kkk++) {
                    if (((cursor_offset + kkk) & 0x0F) == 0) {
                        break;
                    }
                }
                row_delta = kkk;
                break;
            case 'm':
                for (kkk = 0; kkk < bufsize; kkk++) {
                    if (cpdata[kkk] == 0x00) {
                        top_addr = first_addr + kkk;
                        pos = 0;
                        cursor_offset = kkk;
                        cur_row = 0;
                        break;
                    }
                }
                break;
            #if 0
            case 'Y':
                IMG_sanerelocplan(img, secid);
                mvprintw(0, 0, "SANE DONE");
                refresh();
                getch();
                break;
            #endif
            case 'w':
                IMG_recordplan(img, secid);
                mvprintw(0, 0, "RECORD DONE");
                refresh();
                getch();
                break;
            #if 0
            case 'c':
                int error_addr;
                if (cpdata[cursor_offset] == 0) {
                    if (data[cursor_offset] == 0x8D && 
                        data[cursor_offset + 1] == 0x40 &&
                        data[cursor_offset + 2] == 0x00 &&
                        ((cursor_offset + 3) & 0x03) == 0) {


                        cpdata[cursor_offset] = CP_ALIGN;
                        cpdata[cursor_offset + 1] = CP_ALIGN;
                        cpdata[cursor_offset + 2] = CP_ALIGN;
                        break;
                    } else if (data[cursor_offset] == 0x8B && 
                        data[cursor_offset + 1] == 0xC0 &&
                        ((cursor_offset + 2) & 0x03) == 0) {

                        cpdata[cursor_offset] = CP_ALIGN;
                        cpdata[cursor_offset + 1] = CP_ALIGN;
                        break;
                    } else if (data[cursor_offset] == 0x90 && 
                        ((cursor_offset + 1) & 0x03) == 0) {

                        cpdata[cursor_offset] = CP_ALIGN;
                        break;
                    }
                }
                if ((error_addr = discover_code(sec->data, sec->cplan, cursor_offset))) {
                    mvprintw(0, 0, "ERROR AT %08X: ", uaddr_mk(secid, error_addr));
                    done = true;
                    refresh();
                    getch();
                } else {
                    //mvprintw(0, 0, "OK AT %08X: ", uaddr_mk(secid, error_addr));
                }
                break;
            case 'O':
                {
                    mvprintw(0, 0, "Set to code: ");
                    refresh();
                    static char str[64] = "0x";
                    echo();
                    getstr(&str[2]);
                    noecho();
                    int toaddr = strtol(str, NULL, 0);
                    toaddr = uaddr_off(ida_to_addr(toaddr));
                    if ((error_addr = discover_code(sec->data, sec->cplan, toaddr))) {
                        mvprintw(0, 0, "ERROR AT %08X: ", uaddr_mk(secid, error_addr));
                        done = true;
                        refresh();
                        getch();
                    }
                }
                break;
            #endif
            case 'q':
                done = true;
                break;
        }
        if (row_delta != 0) {
            cur_row += row_delta;
            if (cur_row < 0) {
                row_delta = cur_row;
                cur_row = 0;
            } else if (cur_row >= NUM_ROWS) {
                row_delta = cur_row - NUM_ROWS + 1;
                cur_row = NUM_ROWS - 1;
            } else {
                row_delta = 0;
            }
            if (row_delta != 0) {
                if (row_delta < 0) {
                    while (row_delta < 0) {
                        if (top_addr == first_addr) {
                            break;
                        }
                        do {
                            top_addr--;
                        } while(cpdata[top_addr - first_addr] & PLAN_INSIDE);
                        row_delta++;
                    }
                } else {
                    while (row_delta > 0) {
                        do {
                            top_addr++;
                        } while(cpdata[top_addr - first_addr] & PLAN_INSIDE);
                        row_delta--;
                    }
                }
                //top_addr += row_delta;
                //if (top_addr < first_addr) {
                //    top_addr = first_addr;
                //}
            }
        }
    }
    /*  Clean up after ourselves  */

    delwin(mainwin);
    endwin();
    refresh();
}

