#ifndef _ROUTE_H_
#define _ROUTE_H_
typedef struct Router Router;
typedef struct Route Route;
struct Route {
    char *key;
    char *value;
    Route *left, *right;
};

struct Router {
    Route *root;
    void (*list_routes)(Route *self);
    void (*add_route)(Route **self, char *key, char *value);
    Route *(*search_route)(Route *self, const char *key);
};

void init_router(Router **self);
#endif
