#ifndef SQL_H
#define SQL_H

#include <sqlite3.h>

class sqlq_t {
private:
    sqlite3_stmt *sta;
    sqlite3 *db;
    bool is_ready;
    int index_col;
    int sentinel;
    int res;
public:
    sqlq_t(void);
    sqlq_t(sqlite3 *db_arg);
    sqlq_t(sqlite3 *db, const char *str, int index_col_arg = -1, int sentinel_arg = 0);
    ~sqlq_t();
    void prepare(const char *str, int index_col_arg = -1, int sentinel_arg = 0);
    void bind_int(int pos, int v);
    void bind_null(int pos);
    void bind_str(int pos, const char *v);
    int col_int(int pos);
    const char *col_str(int pos);
    bool ready(void);
    int run(void);
    int step(int *pindex = NULL);
    int answer(int sentinel = 0);
};

#endif

