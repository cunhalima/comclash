#include <stdio.h>

typedef unsigned char uchar;
typedef unsigned char ubyte;
typedef unsigned char bool;

void gd_init();
void ss_init();
void game_frame();
void ss_start_game(int);
void gd_close();
void CFG_load(int a, int b);
int CFG_getstr(const char *name, char *pvalue, int size);
void dochkheap(void);
void change_gstate(short *state);

void DBG_locate( int l, int c );
void DBG_print( const char* msg );

extern short gstate;
typedef void (*fn_t)(void);
extern fn_t gs_fnlist_one[];

extern union {
    struct {
        char cata, pora;
    } a;
    int b;
} porco, regua;

extern char showthanks;

void aqui(void) {
    char msg[100];

    DBG_locate(1, 1);
    sprintf(msg, "gstate = %04X\n", gstate);
    DBG_print(msg);

    DBG_locate(2, 1);
    sprintf(msg, "porco = %08X regua = %08X\n", porco.b, regua.b);
    DBG_print(msg);
}

#if 0

void ss_start_game(int argc) {
    mem_size = getMemSize(a1, a2, a3, a1);
    v4 = 0;
    if (mem_size < 540000) {
        error_quit(12289);
    }

    ResInit();
    palmito = (int)&respeito;
    remeco = 68320;
    AddPathCITHOME();
    CFG_load(argc, argv);
    if (CFG_getstr("norun", &v8, 1)) {
        if (v8 == '1') {
            error_quit(0x5001);
        }
    }
    if (CFG_getstr("fault_off", NULL, 0)) {
        if (v8 == '1') {
            SYS_initexhan(0x7141);
        }
    }
    kb_startup(0)
    set_key('T', ESF_ABORTGAME);
    atexit(kb_shutdown);
    rolobo();
    arroto(&sepom, 0x173FD);
    maruca = &sepom;
    some_tog = &first_tog_data;
    
  CFG_load(v7, v6, 1);
  if ( (unsigned __int8)CFG_getstr(aNorun, &v62, 1) && v62 == 49 )
    error_quit(20481);
  if ( !(unsigned __int8)CFG_getstr(aFault_off, 0, 0) )
    SYS_initexhan(28993, v8, 0);
  kb_startup(0, ESF_ABORTGAME, 0);
  set_key(84);
  v9 = atexit(kb_shutdown);
  rolobo(v9);
  arroto(sepom);
  maruca = (int)sepom;
  some_tog = (int)first_tog_data;
  some_tog_size = 0x4000;
  MemStackInit(&some_tog);
  v10 = temp_mem_init(&some_tog);
  v11 = game_callbiostod(v10);
  v12 = UI_loadstrings(v11);
  zemera(v12);
  atexit(showThanks);
  gd_init();
  gr_set_malloc(CallMalloc);
  gr_set_free(CallFree);
  atexit(gd_close);
  setScr(0, v13);
  vidRefPtr = VID_allocref(320, 200);
  setVideRefShitPtr(vidRefPtr, 4, 0x100000);
  v14 = sub_252543(3);
  v15 = MAP_create(v14);
  gempepa(v15);
  v16 = atexit(piralim);
  v17 = load_resources(v16);
  v18 = beveta(v17);
  v19 = capisca(v18);
  v20 = zagora(v19);
  v21 = init_pufas(v20);
  v22 = init_glonar(v21);
  v23 = region_begin_sequence(v22);
  v24 = snd_init(v23);
  fornica(v24);
  v25 = reg_timerincproc(280, 0x118u, (int)timer_inc);
  v27 = SND_init(v25, v26, timer_inc);
  coisa_digiparm(v27);
  input_init(a4);
  v28 = ResOpenResFile((int)aSplash_res, 0, 0, a5);
  v59 = v28;
  v29 = geto(v28, v28, 0);
  v30 = calmipa(v29);
  if ( v31 > 0 )
  {
    (*(void (__fastcall **)(_DWORD, int, _DWORD, signed int))(ximbau + 32))(0, ximbau, 0, 1);
    v30 = relepa(121241600, 0);
  }
  uiFlush(v30);
  v32 = init_videomodes(timer_addr, *(_DWORD *)timer_addr);
  v33 = regstuff1(v32);
  v34 = init_bosta(v33);
  v35 = respoca(v34);
  fabota(v35, v36 + 840);
  v38 = wait_sometime(v37);
  if ( (_BYTE)v38 )
    v63 = 1;
  if ( neplo )
    v38 = poloca(v38);
  if ( v59 > 0 )
  {
    (*(void (__fastcall **)(_DWORD, int, _DWORD, signed int))(ximbau + 32))(0, ximbau, 0, 1);
    v38 = relepa(121241601, 0);
  }
  uiFlush(v38);
  v60 = *(_DWORD *)timer_addr;
  if ( v63 )
    v61 = v60 + 280;
  else
    v61 = v60 + 840;
  v39 = xempa();
  v40 = sampa(v39);
  flamengo(v40);
  v42 = 0;
  v43 = 0;
  do
  {
    xorume[v43] = v42++;
    ++v43;
  }
  while ( v42 < 54 );
  v44 = encosta(v42, v41, 0, v43 * 2);
  resvala(v44);
  v45 = (_UNKNOWN *)esteio;
  v46 = region_flush_sequence(0);
  R_LoadTables(v46, v47, esteio);
  araponga = 0;
  _STOSB(&nilo);
  if ( !(unsigned __int8)CFG_getint(aArchive) )
  {
    esteio[0] = *(_DWORD *)aArchive_dat;
    esteio[1] = *(_DWORD *)&aArchive_dat[4];
    esteio[2] = *(_DWORD *)&aArchive_dat[8];
  }
  lauro = 1;
  init_kbchain();
  fecho = 95;
  v48 = wait_sometime(v61);
  if ( (_BYTE)v48 )
    v63 = 1;
  if ( neplo )
    v48 = poloca(v48);
  v49 = capula(v48);
  if ( v59 > 0 )
  {
    v45 = 0;
    (*(void (__fastcall **)(_DWORD, _DWORD, _DWORD))(ximbau + 32))(0, 0, 0);
    relepa(121241602, v50);
    v49 = ResCloseFile(v59, v51, 0);
  }
  v52 = uiFlush(v49);
  v54 = *(_DWORD *)timer_addr;
  if ( v63 )
    v55 = v54 + 280;
  else
    v55 = v54 + 840;
  if ( gstate != 4 && gstate != 6 )
  {
    poorFunc(v52, v53, v45, v55);
    v45 = &abate;
    (*(void (__fastcall **)(_DWORD, int, _UNKNOWN *))(ximbau + 32))(0, ximbau, &abate);
    v52 = copoca(0, 256, &abate);
    ulopo = 0;
  }
  if ( gstate != 4 && gstate != 6 )
    emprega(v52, v53, v45, v55);
  if ( gstate != 2 )
  {
    v56 = wait_sometime(v55);
    if ( neplo )
      poloca(v56);
  }
  ((void (__fastcall *)(int, int, int))localista[gstate])(gstate, 0, 0);
  result = CFG_getstr(aMem_check, v57, 0);
  if ( (_BYTE)result )
    result = suborna(0, 0, 0);
  bemtri = 1;
  return result;
}
#endif

int gmain(int argc, char **argv) {
    ss_init();
    ss_start_game(argc);
    while (gstate >= 0) {
        //aqui();
        if (!(regua.a.pora & 0x20)) {
            game_frame();
        }
        gs_fnlist_one[gstate]();
        if (regua.a.pora & 0xF0) {
            if (regua.a.pora & 0x10) {
                //dochkheap();
            }
            if (regua.a.pora & 0x80) {
                change_gstate(&gstate);
            }
            regua.a.pora &= 0x7F;
        }
        regua.b |= porco.b;
    }
    showthanks = 1;
    return 0;
}


static unsigned char font[] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x30,0x78,0x78,0x30,0x30,0x00,0x30,0x00,
  0x6C,0x6C,0x6C,0x00,0x00,0x00,0x00,0x00,
  0x6C,0x6C,0xFE,0x6C,0xFE,0x6C,0x6C,0x00,
  0x30,0x7C,0xC0,0x78,0x0C,0xF8,0x30,0x00,
  0x00,0xC6,0xCC,0x18,0x30,0x66,0xC6,0x00,
  0x38,0x6C,0x38,0x76,0xDC,0xCC,0x76,0x00,
  0x60,0x60,0xC0,0x00,0x00,0x00,0x00,0x00,
  0x18,0x30,0x60,0x60,0x60,0x30,0x18,0x00,
  0x60,0x30,0x18,0x18,0x18,0x30,0x60,0x00,
  0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00,
  0x00,0x30,0x30,0xFC,0x30,0x30,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x60,
  0x00,0x00,0x00,0xFC,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,
  0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00,
  0x7C,0xC6,0xCE,0xDE,0xF6,0xE6,0x7C,0x00,
  0x30,0x70,0x30,0x30,0x30,0x30,0xFC,0x00,
  0x78,0xCC,0x0C,0x38,0x60,0xCC,0xFC,0x00,
  0x78,0xCC,0x0C,0x38,0x0C,0xCC,0x78,0x00,
  0x1C,0x3C,0x6C,0xCC,0xFE,0x0C,0x1E,0x00,
  0xFC,0xC0,0xF8,0x0C,0x0C,0xCC,0x78,0x00,
  0x38,0x60,0xC0,0xF8,0xCC,0xCC,0x78,0x00,
  0xFC,0xCC,0x0C,0x18,0x30,0x30,0x30,0x00,
  0x78,0xCC,0xCC,0x78,0xCC,0xCC,0x78,0x00,
  0x78,0xCC,0xCC,0x7C,0x0C,0x18,0x70,0x00,
  0x00,0x30,0x30,0x00,0x00,0x30,0x30,0x00,
  0x00,0x30,0x30,0x00,0x00,0x30,0x30,0x60,
  0x18,0x30,0x60,0xC0,0x60,0x30,0x18,0x00,
  0x00,0x00,0xFC,0x00,0x00,0xFC,0x00,0x00,
  0x60,0x30,0x18,0x0C,0x18,0x30,0x60,0x00,
  0x78,0xCC,0x0C,0x18,0x30,0x00,0x30,0x00,
  0x7C,0xC6,0xDE,0xDE,0xDE,0xC0,0x78,0x00,
  0x30,0x78,0xCC,0xCC,0xFC,0xCC,0xCC,0x00,
  0xFC,0x66,0x66,0x7C,0x66,0x66,0xFC,0x00,
  0x3C,0x66,0xC0,0xC0,0xC0,0x66,0x3C,0x00,
  0xF8,0x6C,0x66,0x66,0x66,0x6C,0xF8,0x00,
  0xFE,0x62,0x68,0x78,0x68,0x62,0xFE,0x00,
  0xFE,0x62,0x68,0x78,0x68,0x60,0xF0,0x00,
  0x3C,0x66,0xC0,0xC0,0xCE,0x66,0x3E,0x00,
  0xCC,0xCC,0xCC,0xFC,0xCC,0xCC,0xCC,0x00,
  0x78,0x30,0x30,0x30,0x30,0x30,0x78,0x00,
  0x1E,0x0C,0x0C,0x0C,0xCC,0xCC,0x78,0x00,
  0xE6,0x66,0x6C,0x78,0x6C,0x66,0xE6,0x00,
  0xF0,0x60,0x60,0x60,0x62,0x66,0xFE,0x00,
  0xC6,0xEE,0xFE,0xFE,0xD6,0xC6,0xC6,0x00,
  0xC6,0xE6,0xF6,0xDE,0xCE,0xC6,0xC6,0x00,
  0x38,0x6C,0xC6,0xC6,0xC6,0x6C,0x38,0x00,
  0xFC,0x66,0x66,0x7C,0x60,0x60,0xF0,0x00,
  0x78,0xCC,0xCC,0xCC,0xDC,0x78,0x1C,0x00,
  0xFC,0x66,0x66,0x7C,0x6C,0x66,0xE6,0x00,
  0x78,0xCC,0xE0,0x70,0x1C,0xCC,0x78,0x00,
  0xFC,0xB4,0x30,0x30,0x30,0x30,0x78,0x00,
  0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xFC,0x00,
  0xCC,0xCC,0xCC,0xCC,0xCC,0x78,0x30,0x00,
  0xC6,0xC6,0xC6,0xD6,0xFE,0xEE,0xC6,0x00,
  0xC6,0xC6,0x6C,0x38,0x38,0x6C,0xC6,0x00,
  0xCC,0xCC,0xCC,0x78,0x30,0x30,0x78,0x00,
  0xFE,0xC6,0x8C,0x18,0x32,0x66,0xFE,0x00,
  0x78,0x60,0x60,0x60,0x60,0x60,0x78,0x00,
  0xC0,0x60,0x30,0x18,0x0C,0x06,0x02,0x00,
  0x78,0x18,0x18,0x18,0x18,0x18,0x78,0x00,
  0x10,0x38,0x6C,0xC6,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,
  0x30,0x30,0x18,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x78,0x0C,0x7C,0xCC,0x76,0x00,
  0xE0,0x60,0x60,0x7C,0x66,0x66,0xDC,0x00,
  0x00,0x00,0x78,0xCC,0xC0,0xCC,0x78,0x00,
  0x1C,0x0C,0x0C,0x7C,0xCC,0xCC,0x76,0x00,
  0x00,0x00,0x78,0xCC,0xFC,0xC0,0x78,0x00,
  0x38,0x6C,0x60,0xF0,0x60,0x60,0xF0,0x00,
  0x00,0x00,0x76,0xCC,0xCC,0x7C,0x0C,0xF8,
  0xE0,0x60,0x6C,0x76,0x66,0x66,0xE6,0x00,
  0x30,0x00,0x70,0x30,0x30,0x30,0x78,0x00,
  0x0C,0x00,0x0C,0x0C,0x0C,0xCC,0xCC,0x78,
  0xE0,0x60,0x66,0x6C,0x78,0x6C,0xE6,0x00,
  0x70,0x30,0x30,0x30,0x30,0x30,0x78,0x00,
  0x00,0x00,0xCC,0xFE,0xFE,0xD6,0xC6,0x00,
  0x00,0x00,0xF8,0xCC,0xCC,0xCC,0xCC,0x00,
  0x00,0x00,0x78,0xCC,0xCC,0xCC,0x78,0x00,
  0x00,0x00,0xDC,0x66,0x66,0x7C,0x60,0xF0,
  0x00,0x00,0x76,0xCC,0xCC,0x7C,0x0C,0x1E,
  0x00,0x00,0xDC,0x76,0x66,0x60,0xF0,0x00,
  0x00,0x00,0x7C,0xC0,0x78,0x0C,0xF8,0x00,
  0x10,0x30,0x7C,0x30,0x30,0x34,0x18,0x00,
  0x00,0x00,0xCC,0xCC,0xCC,0xCC,0x76,0x00,
  0x00,0x00,0xCC,0xCC,0xCC,0x78,0x30,0x00,
  0x00,0x00,0xC6,0xD6,0xFE,0xFE,0x6C,0x00,
  0x00,0x00,0xC6,0x6C,0x38,0x6C,0xC6,0x00,
  0x00,0x00,0xCC,0xCC,0xCC,0x7C,0x0C,0xF8,
  0x00,0x00,0xFC,0x98,0x30,0x64,0xFC,0x00,
  0x1C,0x30,0x30,0xE0,0x30,0x30,0x1C,0x00,
  0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x00,
  0xE0,0x30,0x30,0x1C,0x30,0x30,0xE0,0x00,
  0x76,0xDC,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x10,0x38,0x6C,0xC6,0xC6,0xFE,0x00
};

static void putpixel( int x, int y, char color )
{
  if ( x < 0 || x >= 320 ) return;
  if ( y < 0 || y >= 200 ) return;
  *((char*)( 0xA0000 + ( y * 320 ) + x )) = color;
}

static int line = 0;
static int col = 0;

void DBG_putc( unsigned char c )
{
  int xs, ys;
  int xc, yc;
  uchar b;

  if ( c == '\n' )
  {
    line++;
    col = 0;
    return;
  }
  
  if ( c < 32 ) c = 32;
  if ( c > 0x7F ) c = 0x7F;
  c -= 32;

  ys = line * 8;
  xs = col * 8;

  for ( yc = 0; yc < 8; yc++ )
  {
    b = font[ c * 8 + yc ];
      
    for ( xc = 0; xc < 8; xc++ )
    {
      putpixel( xs + xc, ys + yc, ( b & 0x80 ) ? 100 : 0x00 );
      b <<= 1;
    }
  }
  col++;
  if ( col >= 40 )
  {
    line++;
    col = 0;
  }
}

void DBG_locate( int l, int c )
{
  line = l;
  col = c;
}

void DBG_print( const char* msg )
{
  while ( *msg )
  {
    DBG_putc( *msg++ );
  }
}

#if 0

static int count = 0;
static int last_fn = 0;
//static char dbs[1024] = "";

void DBG_draw()
{
    char s[16];
    int i;
    
    sprintf(s, "fn=%08X count=%d", last_fn, count);
    DBG_locate(1, 1);
    DBG_print(s);
//    DBG_locate(3, 1);
//    DBG_print(dbs);
    //for (i=0; i<20000; i++)
    //putpixel(i % 320, i / 320, 0);
}

typedef struct DBG_REGS {
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax, flags;
} DBG_REGS;

void DBG_getvars(DBG_REGS *regs)
{
    char s[16];
    u16 fn;
  
    fn = *((u16*)(regs->ebp + 0x0C));
    sprintf(s, "%04X ", fn & 0xFFFF); // 300 301 303 304 305  
    //strcat(dbs, s);
    
    if (fn == 0x405) {
        last_fn = fn;
        count++;
    }
}

/*
DIG
   Função 0x300 {
      Chamada somente uma vez
   }
   Função 0x301 {
      Chamada somente uma vez
   }
   Função 0x302 {
      Não vi ser chamada
   }
   Função 0x303 {
      Chamada somente uma vez
   }
   Função 0x304 {
      Chamada somente uma vez
   }
   Função 0x305 {
      Chamada somente uma vez
   }
   Função 0x306 {
      Não vi ser chamada, mas deve ser no final
   }
   Função 0x400 {
      Não vi ser chamada
   }
   Função 0x401 {
      Chamada a cada vez q não tem som e um som começa a ser tocado
   }
   Função 0x402 {
      Chamada a cada vez q todos os sons param de tocar
   }
   Função 0x403 {
      Não vi ser chamada
   }
   Função 0x404 {
      Não vi ser chamada
   }
   Função 0x405 {
      Não vi ser chamada
   }
*/

#endif

typedef struct {
    int nada;
} MFD;

typedef struct {
    int a;
} uiEvent;

#define FALSE 0

extern char full_game_3d;
extern void ss_string(const char *str, int x, int y);
extern void mfd_add_rect(int x, int y, int w, int h);
extern void draw_res_bm(unsigned int res, int x, int y);
extern void mfd_notify_func(int, int, bool, int, bool);
extern void ss_rect(short x1, short y1, short x2, short y2);

//hack
extern int *grd_canvas;
#define gr_set_fcolor(x) grd_canvas[4]=x

#define ORANGE_YELLOW_BASE 0x3D
#define GRAY_8_BASE        0xD6

#define REF_IMG_bmBlankMFD  0x02590002

#define MFD_VIEW_WID    0x4A
#define MFD_VIEW_HGT    0x3A

#define MFD_GAMES_FUNC  0x1B

#define MFD_INFO_SLOT   4

#define MFD_ACTIVE      2

void games_init_cobra(void* game_state) {
   return;
}


void games_expose_cobra(MFD *m, ubyte tac) {
    if (!full_game_3d) {
        draw_res_bm(REF_IMG_bmBlankMFD, 0, 0);
    }
    //ss_string(STRING(NotInstalled),10,20);
    //ss_string(STRING(NotInstalled+1),15,35);
    gr_set_fcolor(ORANGE_YELLOW_BASE);
    ss_rect(2, 3, 13, 17);

    //ss_string("opa",15,35);
    mfd_add_rect(0, 0, MFD_VIEW_WID, MFD_VIEW_HGT);

    // autoreexpose
    mfd_notify_func(MFD_GAMES_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, FALSE);
}

bool games_handle_cobra(MFD* m, uiEvent* ev) {
    return FALSE;
}
