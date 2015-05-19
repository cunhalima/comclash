#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libdis.h"
#include <inttypes.h>

static const char *match_reloc(unsigned int x);
static const char *match_reloc_force(void);

static int num_relocs_in = 0;
static int num_relocs_out = 0;
static char reloc_name[2][64];
static unsigned int reloc_val[2];

#ifdef _MSC_VER
        #define snprintf        _snprintf
        #define inline          __inline
#endif

#define FILLTAB( buf, len, numwords )   \
numwords++;                             \
if (numwords == 1) {                    \
	int _i = strlen(buf);               \
    while(_i < 8) {                     \
        buf[_i++] = ' ';                \
        len--;                          \
    }                                   \
    buf[_i] = '\0';                     \
}

/*
 * concatenation macros.  STRNCATF concatenates a format string, buf
 * only with one argument.
 */
#define STRNCAT( buf, str, len ) do {   				\
	int _i = strlen(str), _blen = strlen(buf), _len = len - 1;  	\
	if ( len ) {							\
        	strncat( buf, str, _len );  				\
		if ( _len <= _i ) {					\
			buf[_blen+_len] = '\0';				\
			len = 0;					\
		} else {						\
			len -= _i;					\
		}							\
	}								\
} while( 0 )

#define STRNCATF( buf, fmt, data, len ) do {        \
        char _tmp[MAX_OP_STRING];                   \
                                                    \
        snprintf( _tmp, sizeof _tmp, fmt, data );   \
        STRNCAT( buf, _tmp, len );                  \
} while( 0 )


#define PRINT_DISPLACEMENT( ea ) do {                           \
    if ( ea->disp_size && ea->disp ) {                          \
        if ( ea->disp_sign ) {                                  \
            STRNCATF( buf, "-0x%" PRIX32, -ea->disp, len );     \
        } else {                                                \
            const char *n = match_reloc(ea->disp);              \
            if (n) {                                            \
                STRNCATF( buf, "%s", n, len);                   \
            } else {                                            \
                STRNCATF( buf, "0x%" PRIX32, ea->disp, len );   \
            }                                                   \
        }                                                       \
    }                                                           \
} while( 0 )

static int format_expr( x86_ea_t *ea, char *buf, int len,
                        enum x86_asm_format format ) {
    //STRNCAT( buf, "[", len );
    if (!(ea->base.name[0]) && (ea->index.name[0]) && (ea->scale == 2)) {
        STRNCAT( buf, "nosplit ", len );
    }
    if ( ea->base.name[0] ) {
        STRNCAT( buf, ea->base.name, len );
        if ( ea->index.name[0] || (ea->disp_size && ! ea->disp_sign && ea->disp) ) {
            STRNCAT( buf, "+", len );
        }
    }
    if ( ea->index.name[0] ) {
        STRNCAT( buf, ea->index.name, len );
        if ( ea->scale > 1 ) {
            STRNCATF( buf, "*%" PRId32, ea->scale, len );
        }
        if ( (ea->disp_size && ea->disp) && ! ea->disp_sign ) {
            STRNCAT( buf, "+", len );
        }
    }
    if ( ea->disp_size || (! ea->index.name[0] && !ea->base.name[0] ) ) {
        PRINT_DISPLACEMENT(ea);
    }
    //STRNCAT( buf, "]", len );
    return( strlen(buf) );
}

static int format_seg( x86_op_t *op, char *buf, int len,
                       enum x86_asm_format format ) {
    int len_orig = len;
    char *reg = "";

    if (! op || ! buf || ! len || ! op->flags) {
        return(0);
    }
    if ( op->type != op_offset && op->type != op_expression ){
        return(0);
    }
    if (! (int) op->flags & 0xF00 ) {
        return(0);
    }
    switch (op->flags & 0xF00) {
        case op_es_seg: reg = "es"; break;
        case op_cs_seg: reg = "cs"; break;
        case op_ss_seg: reg = "ss"; break;
        case op_ds_seg: reg = "ds"; break;
        case op_fs_seg: reg = "fs"; break;
        case op_gs_seg: reg = "gs"; break;
        default:
            break;
    }
    if (! reg[0] ) {
        return( 0 );
    }
    STRNCATF( buf, "%s:", reg, len );
    return( len_orig - len ); /* return length of appended string */
}

/*
 * sprint's an operand's data to string str.
 */
static void get_operand_data_str( x86_op_t *op, char *str, int len ){
    if ( op->flags & op_signed ) {
        switch ( op->datatype ) {
            case op_byte:
                snprintf( str, len, "%" PRId8, op->data.sbyte );
                return;
            case op_word:
                snprintf( str, len, "%" PRId16, op->data.sword );
                return;
            case op_qword:
                snprintf( str, len, "%" PRId64, op->data.sqword );
                return;
            default:
                snprintf( str, len, "%" PRId32, op->data.sdword );
                return;
        }
    }
    switch ( op->datatype ) {
        case op_byte:
            snprintf( str, len, "0x%02" PRIX8, op->data.byte );
            return;
        case op_word:
            {
                const char *n = match_reloc(op->data.word);
                if (n) {
                    snprintf( str, len, "%s", n );
                } else {
                    snprintf( str, len, "0x%04" PRIX16, op->data.word );
                }

            }
            return;
        case op_qword:
            snprintf( str, len, "0x%08" PRIX64,op->data.sqword );
            return;
        default:
            {
                const char *n = match_reloc(op->data.dword);
                if (n) {
                    snprintf( str, len, "%s", n );
                } else {
                    snprintf( str, len, "0x%08" PRIX32, op->data.dword );
                }

            }
            return;
    }
}

static void my_get_operand_data_str( x86_op_t *op, char *str, int len ){
    x86_insn_t* insn = (x86_insn_t*)op->insn;
    // if 240
    //if (x86_lineno == 240) {
    if (op == x86_operand_2nd(op->insn)) {
        if ( insn->type != insn_shl &&
                    insn->type != insn_shr &&
                    insn->type != insn_rol &&
                    insn->type != insn_ror ) {
        x86_op_t *op1 = x86_operand_1st(op->insn);
        if (((op1->datatype == op_dword) && (op->datatype == op_byte)) || 
            ((op1->datatype == op_word) && (op->datatype == op_byte))) {
            if (op->data.byte & 0x80) {
                snprintf( str, len, "%d", ((signed char)op->data.byte));
                return;
            }
        }
        }
    } else if (((x86_insn_t*)op->insn)->type == insn_push) {
        if (op->datatype == op_byte && op->data.byte & 0x80) {
            snprintf( str, len, "%d", ((signed char)op->data.byte));
            return;
        }
    } else if (((x86_insn_t*)op->insn)->type == insn_mul) {
        if (op == x86_operand_3rd(op->insn)) {
            x86_op_t *op1 = x86_operand_1st(op->insn);
            if ((((op1->datatype == op_word) || (op1->datatype == op_dword)) && (op->datatype == op_byte)) &&
               (op->datatype == op_byte && op->data.byte & 0x80)) {
                snprintf( str, len, "%d", ((signed char)op->data.byte));
                return;
            }
        }
    }
    //}
    get_operand_data_str(op, str, len);
}

static int format_operand_native( x86_op_t *op, x86_insn_t *insn, char *buf, int len){
    char str[MAX_OP_STRING];
    switch (op->type) {
        case op_register:
            STRNCAT( buf, op->data.reg.name, len );
            break;
        case op_immediate:
            my_get_operand_data_str( op, str, sizeof str );
            STRNCAT( buf, str, len );
            break;
        case op_relative_near:
            {
                const char *n = match_reloc_force();
                if (n) {
                    int noshort = 0;
                    if (insn->mnemonic[2] == 'x') {
                        if (strcmp(insn->mnemonic, "jcxz") == 0) {
                            noshort = 1;
                        }
                    } else if (insn->mnemonic[3] == 'x') {
                        if (strcmp(insn->mnemonic, "jecxz") == 0) {
                            noshort = 1;
                        }
                    } else if (insn->mnemonic[0] == 'l') {
                        if (strcmp(insn->mnemonic, "loop") == 0) {
                            noshort = 1;
                        }
                    }
                    if (!noshort) {
                        STRNCAT(buf, "short ", len);
                    }
                    STRNCATF(buf, "%s", n, len);
                } else {
                    STRNCATF( buf, "0x%08" PRIX32, (unsigned int)(op->data.sbyte + insn->addr + insn->size), len );
                }
            }
            break;
        case op_relative_far:
            {
                const char *n = match_reloc_force();
                if (n) {
                    STRNCATF(buf, "near %s", n, len);
                } else {
                    if ( op->datatype == op_word ) {
                        STRNCATF( buf, "0x%08" PRIX32, (unsigned int)(op->data.sword + insn->addr + insn->size), len );
                    } else {
                        STRNCATF( buf, "0x%08" PRIX32, op->data.sdword + insn->addr + insn->size, len );
                    }
                }
            }
            break;
        case op_absolute:
            STRNCATF( buf, "$0x%04" PRIX16 ":", op->data.absolute.segment, len );
            if (op->datatype == op_descr16) {
                            STRNCATF( buf, "0x%04" PRIX16, 
                    op->data.absolute.offset.off16, len );
            } else {
                            STRNCATF( buf, "0x%08" PRIX32, 
                    op->data.absolute.offset.off32, len );
            }
            break;
        case op_offset:
            STRNCAT( buf, "[", len );
            len -= format_seg( op, buf, len, native_syntax );
            //STRNCAT( buf, "nosplit ", len );
            {
                const char *n = match_reloc(op->data.sdword);
                if (n) {
                    STRNCATF( buf, "%s", n, len);
                } else {
                    STRNCATF( buf, "0x%08" PRIX32 "", op->data.sdword, len );
                }
            }
            STRNCAT( buf, "]", len );
            break;
        case op_expression:
            //len -= format_seg( op, buf, len, native_syntax );
            //len -= format_expr( &op->data.expression, buf, len, native_syntax );
            //break;
            STRNCAT( buf, "[", len );
            len -= format_seg( op, buf, len, native_syntax );
            len -= format_expr( &op->data.expression, buf, len, native_syntax );
            STRNCAT( buf, "]", len );
            break;

        case op_unused:
        case op_unknown:
            /* return 0-truncated buffer */
            break;
    }
    return( strlen( buf ) );
}

int x86_format_operand( x86_op_t *op, char *buf, int len, enum x86_asm_format format ){
	x86_insn_t *insn;
    if ( ! op || ! buf || len < 1 ) {
        return(0);
    }
	/* insn is stored in x86_op_t since .21-pre3 */
	insn = (x86_insn_t *) op->insn;
    memset( buf, 0, len );
    return format_operand_native( op, insn, buf, len );
}

#define is_imm_jmp(op)   (op->type == op_absolute   || \
                          op->type == op_immediate  || \
                          op->type == op_offset)
#define is_memory_op(op) (op->type == op_absolute   || \
                          op->type == op_expression || \
                          op->type == op_offset)

struct op_string { char *buf; size_t len; };

static int prehack(x86_insn_t *insn, x86_op_t *dst, x86_op_t *src, char *buf, int len) {
    if (insn->type == insn_div && dst) {
        dst->flags |= op_implied;
    }
    if (insn->group == insn_string) {
        const char *suffix = "";
        if (dst) {
            dst->flags |= op_implied;
            switch(dst->datatype) {
                case op_byte:  suffix = "b"; break;
                case op_word:  suffix = "w"; break;
                case op_dword: suffix = "d"; break;
                default: break;
            }
        }
        if (src) {
            src->flags |= op_implied;
        }
        strcat(insn->mnemonic, suffix);
    } else if (insn->bytes[0] == 0xE3) {
        strcpy(insn->mnemonic, "jecxz");
    } else if (insn->bytes[0] == 0x66) {
        int hack = 0;
        if (insn->bytes[1] == 0x8C) {
            switch(insn->bytes[2]) {
                case 0x5B:
                    if (insn->bytes[3] == 0x04) {
                        hack = 1;
                    }
                    break;
                case 0x1D:
                case 0x15:
                    hack = 1;
                    break;
            }
        } else if (insn->bytes[1] == 0x2E && insn->bytes[2] == 0x8E) {
            if ((insn->bytes[3] == 0x1D) || (insn->bytes[3] == 0x05)) {
                hack = 1;
            }
        } else if (insn->bytes[1] == 0x8E) {
            if ((insn->bytes[2] == 0x05) || (insn->bytes[2] == 0x15)) {
                hack = 1;
            }
        }
        if (hack) {
            strcat(buf, "o16 ");
            return 3;
        }
    }
    return 0;
}

#if 0
static const char *OP_SIZE_CODE(x86_op_t *op) {
    if (op->datatype == op_byte) {
        return "b";
    } else if (op->datatype == op_word) {
        return "w";
    } else if (op->datatype == op_dword) {
        return "d";
    }
    return "";
}

#endif

static const char *OP_SIZE_NAME(x86_op_t *op) {
    if (op->datatype == op_byte) {
        return "byte";
    } else if (op->datatype == op_word) {
        return "word";
    } else if (op->datatype == op_dword) {
        return "dword";
    }
    return "";
}

static int sizehack(x86_insn_t *insn, x86_op_t *op, char *buf, int len) {
    if (insn->mnemonic[0] != 'l' &&
        insn->type != insn_shl &&
        insn->type != insn_shr &&
        insn->type != insn_rol &&
        insn->type != insn_ror &&
        insn->type != insn_enter ) {

        const char *o = OP_SIZE_NAME(op);
        strcat(buf, o);
        strcat(buf, " ");
        return strlen(o) + 1;
    }
    return 0;
}

static void posthack(x86_insn_t *insn, char *buf) {
    // MOVEFX HACK
    if (insn->bytes[0] == 0x66 && insn->bytes[1] == 0xF7 && insn->bytes[2] == 0xE9) {
        strcpy(buf, "imul    cx");
    }
    // MOVEFX HACK
    if (insn->bytes[0] == 0x66 && insn->bytes[1] == 0xF3 && insn->bytes[2] == 0xA4) {
        strcpy(buf, "o16 rep movsb");
    }
    // MOVEFX HACK
    if (insn->bytes[0] == 0x8C && insn->bytes[1] == 0xC0) {
        strcpy(buf, "mov     eax, es");
    }
    // MOVEFX HACK
    if (insn->bytes[0] == 0x8C && insn->bytes[1] == 0xC2) {
        strcpy(buf, "mov     edx, es");
    }
    // MOVEFX HACK
    if (insn->bytes[0] == 0x8C && insn->bytes[1] == 0xC3) {
        strcpy(buf, "mov     ebx, es");
    }
    // MOVEFX HACK
    if (insn->bytes[0] == 0x8C && insn->bytes[1] == 0xE0) {
        strcpy(buf, "mov     eax, fs");
    }
    // MOVEFX HACK
    if (insn->bytes[0] == 0x8C && insn->bytes[1] == 0xE8) {
        strcpy(buf, "mov     eax, gs");
    }
    // MOVEFX HACK
    if (insn->bytes[0] == 0x8C && insn->bytes[1] == 0xCB) {
        strcpy(buf, "mov     ebx, cs");
    }
    // MOVEFX HACK
    if (insn->bytes[0] == 0x8C && insn->bytes[1] == 0xC8) {
        strcpy(buf, "mov     eax, cs");
    }
    // MOVEFX HACK
    if (insn->bytes[0] == 0x8C && insn->bytes[1] == 0xD8) {
        strcpy(buf, "mov     eax, ds");
    }
    // MOVEFX HACK
    if (insn->bytes[0] == 0x8C && insn->bytes[1] == 0xD9) {
        strcpy(buf, "mov     ecx, ds");
    }
    // MOVEFX HACK
    if (insn->bytes[0] == 0x8C && insn->bytes[1] == 0xD0) {
        strcpy(buf, "mov     eax, ss");
    }
    // MOVEFX HACK
    if (insn->bytes[0] == 0x8C && insn->bytes[1] == 0xDB) {
        strcpy(buf, "mov     ebx, ds");
    }
    // MOVEFX HACK
    if (insn->bytes[0] == 0x8C && insn->bytes[1] == 0xC9) {
        strcpy(buf, "mov     ecx, cs");
    }
    // MOVEFX HACK
    if (insn->bytes[0] == 0x8C && insn->bytes[1] == 0xCA) {
        strcpy(buf, "mov     edx, cs");
    }
    // MOVEFX HACK
    if (insn->bytes[0] == 0x8C && insn->bytes[1] == 0xDA) {
        strcpy(buf, "mov     edx, ds");
    }
    // MOVEFX HACK
    if (insn->bytes[0] == 0x8E && insn->bytes[1] == 0xC0) {
        strcpy(buf, "mov     es, eax");
    }
    // MOVEFX HACK
    if (insn->bytes[0] == 0x8E && insn->bytes[1] == 0xC2) {
        strcpy(buf, "mov     es, edx");
    }
    // MOVEFX HACK
    if (insn->bytes[0] == 0x8E && insn->bytes[1] == 0xC7) {
        strcpy(buf, "mov     es, edi");
    }
    // MOVEFX HACK
    if (insn->bytes[0] == 0x8E && insn->bytes[1] == 0xC3) {
        strcpy(buf, "mov     es, ebx");
    }
    // MOVEFX HACK
    if (insn->bytes[0] == 0x8E && insn->bytes[1] == 0xD0) {
        strcpy(buf, "mov     ss, eax");
    }
    // MOVEFX HACK
    if (insn->bytes[0] == 0x8E && insn->bytes[1] == 0xD1) {
        strcpy(buf, "mov     ss, ecx");
    }
    // MOVEFX HACK
    if (insn->bytes[0] == 0x8E && insn->bytes[1] == 0xD9) {
        strcpy(buf, "mov     ds, ecx");
    }
    // MOVEFX HACK
    if (insn->bytes[0] == 0x8E && insn->bytes[1] == 0xDB) {
        strcpy(buf, "mov     ds, ebx");
    }
    // MOVEFX HACK
    if (insn->bytes[0] == 0x8E && insn->bytes[1] == 0xC6) {
        strcpy(buf, "mov     es, esi");
    }
    // MOVEFX HACK
    if (insn->bytes[0] == 0x8E && insn->bytes[1] == 0xDA) {
        strcpy(buf, "mov     ds, edx");
    }
    // MOVEFX HACK
    if (insn->bytes[0] == 0x8E && insn->bytes[1] == 0xDD) {
        strcpy(buf, "mov     ds, ebp");
    }
    // CBW HACK
    if (insn->bytes[0] == 0x66 && insn->bytes[1] == 0x99) {
        strcpy(buf, "cwd");
    }
    // CBW HACK
    if (insn->bytes[0] == 0x66 && insn->bytes[1] == 0x98) {
        strcpy(buf, "cbw");
    }
    // CBW HACK
    if (insn->bytes[0] == 0x66 && insn->bytes[1] == 0x9C) { // pushfw
        strcpy(buf, "pushfw");
    }
    // CBW HACK
    if (insn->bytes[0] == 0x66 && insn->bytes[1] == 0xCB) {
        strcpy(buf, "o16 retf");
    }
    // LEA [EAX] HACK
    if (insn->bytes[0] == 0x2E && insn->bytes[1] == 0x8B && insn->bytes[2] == 0xC0)  {
        strcpy(buf, "db      0x2E, 0x8B, 0xC0 ; 3-fill cs mov eax,eax");
    }
    // LEA [EAX] HACK
    if (insn->bytes[0] == 0x8D && insn->bytes[1] == 0x40 && insn->bytes[2] == 0x00)  {
        strcpy(buf, "lea     eax,[byte eax+0] ; 3-fill lea eax,[eax+0]");
    }
    // LEA [EDX] HACK
    if (insn->bytes[0] == 0x8D && insn->bytes[1] == 0x52 && insn->bytes[2] == 0x00)  {
        strcpy(buf, "lea     edx,[byte edx+0] ; 3-fill lea edx,[edx+0]");
    }
    // LEA [EAX] HACK
    if (insn->bytes[0] == 0x8D && insn->bytes[1] == 0x80 && insn->bytes[2] == 0x00 && insn->bytes[3] == 0x00 && insn->bytes[4] == 0x00 && insn->bytes[5] == 0x00)  {
        strcpy(buf, "lea     eax,[dword eax+0] ; 6-fill lea eax,[eax+0]");
    }
    // LEA [EDX] HACK
    if (insn->bytes[0] == 0x8D && insn->bytes[1] == 0x92 && insn->bytes[2] == 0x00 && insn->bytes[3] == 0x00 && insn->bytes[4] == 0x00 && insn->bytes[5] == 0x00)  {
        strcpy(buf, "lea     edx,[dword edx+0] ; 6-fill lea edx,[edx+0]");
    }
    // LEA [EDX] HACK
    if ((insn->bytes[0] == 0x89 && insn->bytes[1] == 0x9A) ||
        (insn->bytes[0] == 0x89 && insn->bytes[1] == 0x8F) ||
        (insn->bytes[0] == 0x88 && insn->bytes[1] == 0x8A) ||
        (insn->bytes[0] == 0x88 && insn->bytes[1] == 0x83) ||
        (insn->bytes[0] == 0x88 && insn->bytes[1] == 0x97) ||
        (insn->bytes[0] == 0x89 && insn->bytes[1] == 0x9F) ||
        (insn->bytes[0] == 0x88 && insn->bytes[1] == 0x9F) ||
        (insn->bytes[0] == 0x88 && insn->bytes[1] == 0x8F) ||
        (insn->bytes[0] == 0x89 && insn->bytes[1] == 0xAF) ||
        (insn->bytes[0] == 0x89 && insn->bytes[1] == 0xB7) ||
        (insn->bytes[0] == 0x89 && insn->bytes[1] == 0x87) ||
        (insn->bytes[0] == 0x88 && insn->bytes[1] == 0x93) ||
        (insn->bytes[0] == 0xC6 && insn->bytes[1] == 0x87) ||
        (insn->bytes[0] == 0x89 && insn->bytes[1] == 0x81) ||
        (insn->bytes[0] == 0x89 && insn->bytes[1] == 0x99) ||
        (insn->bytes[0] == 0x89 && insn->bytes[1] == 0xB2) ||
        (insn->bytes[0] == 0x89 && insn->bytes[1] == 0xA9) ||
        (insn->bytes[0] == 0x89 && insn->bytes[1] == 0xAA)) {
        if (num_relocs_in == 0) {
            int x = insn->bytes[2] | (insn->bytes[3] << 8) | (insn->bytes[4] << 16) | (insn->bytes[5] << 24);
            if (x >= -128 && x <= 127) {
                char *pos = strchr(buf, '[');
                pos++;
                memmove(pos + 6, pos, strlen(pos) + 1);
                strncpy(pos, "dword ", 6);
            }
        }
    }
    // LEA [EDX] HACK
    if (insn->bytes[0] == 0x8A && insn->bytes[1] == 0x87 && insn->bytes[2] == 0x00 && insn->bytes[3] == 0x00 && insn->bytes[4] == 0x00 && insn->bytes[5] == 0x00)  {
        strcpy(buf, "mov     al, [dword edi+0]");
    }
    // LEA [EDX] HACK
    if (insn->bytes[0] == 0x8A && insn->bytes[1] == 0x47 && insn->bytes[2] == 0x00)  {
        strcpy(buf, "mov     al, [byte edi+0]");
    }
    // LEA [EAX] HACK
    if (insn->bytes[0] == 0x88 && insn->bytes[1] == 0x53 && insn->bytes[2] == 0x00)  {
        strcpy(buf, "mov     [byte ebx+0], dl");
    }
    // LEA [EAX] HACK
    if (insn->bytes[0] == 0x88 && insn->bytes[1] == 0x43 && insn->bytes[2] == 0x00)  {
        strcpy(buf, "mov     [byte ebx+0], al");
    }
    if (insn->bytes[0] == 0xFF && insn->bytes[1] == 0xC3)  {
        //MUDAR
        // inc ebx
        //buf[3] = '_';
        strcpy(buf, "inc_ebx");
    }
    // TEST EAX HACK
    if (insn->bytes[0] == 0xF7 && insn->bytes[1] == 0xC0)  {
        //MUDAR
        // test eax, dword 0x
        //buf[4] = '_';
        //strncpy(&buf[8], "        ", 8);
        // test    eax, dword
        strncpy(&buf[4], "_eax          ", 14);
    }
    // ADD EAX HACK
    if (insn->bytes[0] == 0x81 && insn->bytes[1] == 0xC0)  {
        //MUDAR
        // add eax, dword 0x
        //buf[3] = '_';
        //strncpy(&buf[7], "        ", 8);
        // add     eax, dword
        strncpy(&buf[3], "_eax           ", 15);
    }
    // ADC EAX HACK
    if (insn->bytes[0] == 0x81 && insn->bytes[1] == 0xD0)  {
        //MUDAR
        // adc eax, dword 0x
        //buf[3] = '_';
        //strncpy(&buf[7], "        ", 8);
        // adc     eax, dword
        strncpy(&buf[3], "_eax           ", 15);
    }
    // AND EAX HACK
    if (insn->bytes[0] == 0x81 && insn->bytes[1] == 0xE0)  {
        //MUDAR
        // and eax, dword 0x
        //buf[3] = '_';
        //strncpy(&buf[7], "        ", 8);
        // and     eax, dword
        strncpy(&buf[3], "_eax           ", 15);
    }
    // SUB EAX HACK
    if (insn->bytes[0] == 0x81 && insn->bytes[1] == 0xE8)  {
        //MUDAR
        // sub eax, dword 0x
        //buf[3] = '_';
        //strncpy(&buf[7], "        ", 8);
        // sub     eax, dword
        strncpy(&buf[3], "_eax           ", 15);
    }
    // SUB EAX HACK
    if (insn->bytes[0] == 0x80 && insn->bytes[1] == 0xF8)  {
        //MUDAR
        // cmp al, byte 0x
        //buf[3] = '_';
        //strncpy(&buf[6], "      ", 6);
        // cmp     al, byte 0x
        strncpy(&buf[3], "_al          ", 13);
    }
}

int x86_format_insn( x86_insn_t *insn, char *buf, int len, enum x86_asm_format format ){
    char str[MAX_OP_STRING];
    x86_op_t *src, *dst, *imm;
    int numwords = 0;
    dst = x86_operand_1st(insn);
    src = x86_operand_2nd(insn);
    imm = x86_operand_3rd(insn);

    memset(buf, 0, len);
    len -= prehack(insn, dst, src, buf, len);
    STRNCAT(buf, insn->prefix_string, len);
    if (insn->prefix_string[0] != '\0') {
        FILLTAB(buf, len, numwords);
    }
    STRNCAT(buf, insn->mnemonic, len);
    /* dest */
    if (dst && !(dst->flags & op_implied)) {
        FILLTAB(buf, len, numwords);
        if (dst->type == op_offset || dst->type == op_expression || dst->type == op_immediate) {
            if ( insn->type != insn_enter && insn->type != insn_int ) {
                STRNCAT( buf, OP_SIZE_NAME(dst), len );
                STRNCAT( buf, " ", len );
            }
        }
        x86_format_operand(dst, str, MAX_OP_STRING, format);
        STRNCAT(buf, str, len);
    }
    /* src */
    if (src && !(src->flags & op_implied)) {
        FILLTAB(buf, len, numwords);
        if (!(dst->flags & op_implied)) {
            STRNCAT(buf, ", ", len);
        }
        if (src->type == op_offset || src->type == op_expression || src->type == op_immediate) {
            len -= sizehack(insn, src, buf, len);
        }
        x86_format_operand(src, str, MAX_OP_STRING, format);
        STRNCAT(buf, str, len);
    }
    /* imm */
    if (imm) {
        //FILLTAB(buf, len, numwords);
        STRNCAT(buf, ", ", len);
        len -= sizehack(insn, imm, buf, len);
        x86_format_operand(imm, str, MAX_OP_STRING, format);
        STRNCAT(buf, str, len);
    }
    posthack(insn, buf);
    return(strlen(buf));
}

void x86_reloc_reset(void) {
    num_relocs_in = 0;    
    num_relocs_out = 0;    
}

void x86_reloc_send(unsigned int x, const char *name) {
    reloc_val[num_relocs_in] = x;
    strcpy(reloc_name[num_relocs_in], name);
    ++num_relocs_in;
}

int x86_reloc_check(void) {
    if (num_relocs_in == num_relocs_out) {
        return 1;
    }
    return 0;
}

static const char *match_reloc(unsigned int x) {
    if (num_relocs_in > num_relocs_out) {
        for (int i = 0; i < 2; i++) {
            if (reloc_val[i] == x) {
                num_relocs_out++;
                return reloc_name[i];
            }
        }
    }
    return NULL;
}

static const char *match_reloc_force(void) {
    if (num_relocs_in > num_relocs_out) {
        num_relocs_out++;
        return reloc_name[0];
    }
    return NULL;
}
