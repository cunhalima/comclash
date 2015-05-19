#include <cstdlib>
#include <cassert>
#include <sqlite3.h>
#include "defs.h"
#include "sqlq.h"

sqlq_t::sqlq_t(void) : sta(NULL), db(NULL), is_ready(false), index_col(-1), sentinel(0), res(0) {
}

sqlq_t::sqlq_t(sqlite3 *db_arg) : sta(NULL), db(db_arg), is_ready(false), index_col(-1), sentinel(0), res(0) {
    assert(db != NULL);
}

sqlq_t::sqlq_t(sqlite3 *db_arg, const char *str, int index_col_arg, int sentinel_arg) :
       sta(NULL), db(db_arg), is_ready(false), index_col(-1), sentinel(0), res(0) {
    assert(db != NULL);
    prepare(str, index_col_arg, sentinel_arg);
}

sqlq_t::~sqlq_t() {
    if (sta != NULL) {
        sqlite3_clear_bindings(sta);
        sqlite3_reset(sta);
        sqlite3_finalize(sta);
        sta = NULL;
    }
}

int sqlq_t::col_int(int pos) {
    return sqlite3_column_int(sta, pos);
}

const char *sqlq_t::col_str(int pos) {
    return (const char *)sqlite3_column_text(sta, pos);
}

void sqlq_t::bind_int(int pos, int v) {
    sqlite3_bind_int(sta, pos, v);
}

void sqlq_t::bind_null(int pos) {
    sqlite3_bind_null(sta, pos);
}

void sqlq_t::bind_str(int pos, const char *v) {
    sqlite3_bind_text(sta, pos, v, -1, SQLITE_TRANSIENT);
}

bool sqlq_t::ready(void) {
    return (res == SQLITE_ROW || res == SQLITE_DONE);
}

void sqlq_t::prepare(const char *str, int index_col_arg, int sentinel_arg) {
    sqlite3_prepare_v2(db, str, -1, &sta, NULL);
    index_col = index_col_arg;
    sentinel = sentinel_arg;
}

void sqlq_t::prepare(sqlite3* db_arg, const char *str, int index_col_arg, int sentinel_arg) {
    db = db_arg;
    prepare(str, index_col_arg, sentinel_arg);
}

int sqlq_t::answer(int sentinel) {
    int res = sqlite3_step(sta);
    if (res == SQLITE_ROW) {
        return sqlite3_column_int(sta, 0);
    }
    return sentinel;
}

int sqlq_t::run(void) {
    int res = sqlite3_step(sta);
    return (res == SQLITE_DONE);
}

int sqlq_t::step(int *pindex) {
    int res = sqlite3_step(sta);
    if (index_col >= 0) {
        if (res == SQLITE_ROW) {
            res = sqlite3_column_int(sta, index_col);
        } else {
            res = sentinel;
        }
        if (pindex != NULL) {
            *pindex = res;
        }
        return res;
    }
    return res == SQLITE_ROW;
}
