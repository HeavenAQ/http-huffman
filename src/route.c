#include "../include/route.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * init_route - Initialize a new route
 * @param endpoint uri
 * @param template the template file to render for the endpoint
 * @return Route object
 */
static Route *init_route(char *key, char *value)
{
    Route *new_route = (Route *)malloc(sizeof(Route));

    new_route->key = key;
    new_route->value = value;
    new_route->left = new_route->right = NULL;
    return new_route;
}

/**
 * list_routes - List all the routes in the router
 * @param root Root node of the router
 */
static void list_routes(Route *root)
{

    if (root != NULL) {
        list_routes(root->left);
        printf("%s -> %s \n", root->key, root->value);
        list_routes(root->right);
    }
}

/**
 * add_route - Add a new route to the router
 * @param root Root node of the router
 * @param endpoint uri
 * @param template the template file to render for the endpoint
 * @return Route object
 */
static void add_route(Route **root, char *key, char *value)
{
    if (*root == NULL) {
        *root = init_route(key, value);
        return;
    }

    if (strcmp(key, (*root)->key) == 0) {
        printf("============ WARNING ============\n");
        printf("A Route For \"%s\" Already Exists\n", key);
    } else if (strcmp(key, (*root)->key) > 0) {
        add_route(&(*root)->right, key, value);
    } else {
        add_route(&(*root)->left, key, value);
    }
}

/**
 * search_route - Search for a route in the router
 * @param root Root node of the router
 * @param endpoint uri
 * @return Route object
 */
static Route *search_route(Route *root, const char *key)
{
    if (root == NULL) {
        return NULL;
    }

    if (strcmp(key, root->key) == 0) {
        return root;
    } else if (strcmp(key, root->key) > 0) {
        return search_route(root->right, key);
    } else if (strcmp(key, root->key) < 0) {
        return search_route(root->left, key);
    }
    return NULL;
}

/**
 * init_router - Initialize a new router
 * @param self Router object
 */
void init_router(Router **self)
{
    if ((*self = (Router *)malloc(sizeof(Router))) == NULL) {
        printf("Failed to allocate memory for router\n");
        exit(1);
    }
    (*self)->root = NULL;
    (*self)->add_route = &add_route;
    (*self)->search_route = &search_route;
    (*self)->list_routes = &list_routes;
}
