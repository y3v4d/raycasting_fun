#include "map.h"

#include <stdio.h>
#include <stdlib.h>

map_t* map_load(const char *path) {
    FILE *file = fopen(path, "rb");
    if(!file) {
        fprintf(stderr, "Couldn't open %s\n", path);
        return NULL;
    }

    map_t *temp = (map_t*)malloc(sizeof(map_t));
    if(!temp) {
        fclose(file);

        return NULL;
    }

    fread(&temp->width, 1, 1, file);
    fread(&temp->height, 1, 1, file);

    temp->data = (uint8_t*)malloc(sizeof(uint8_t) * temp->width * temp->height);
    if(!temp->data) {
        fprintf(stderr, "Couldn't allocate memory for the map data\n");
        fclose(file);

        return NULL;
    }

    fread(temp->data, 1, temp->width * temp->height, file);

    fclose(file);

    return temp;
}

void map_close(map_t *p) {
    if(!p) return;

    if(p->data) free(p->data);
    free(p);
}

int map_dump(const map_t *p, const char *path) {
    if(!p) return 0;
    
    FILE *file = fopen(path, "wb");
    if(!file) {
        fprintf(stderr, "Couldn't open %s\n", path);
        return 0;
    }

    fwrite(&p->width, 1, 1, file);
    fwrite(&p->height, 1, 1, file);

    fwrite(p->data, 1, p->width * p->height, file);

    fclose(file);
    return 1;
}