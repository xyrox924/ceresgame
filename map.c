#include "map.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gamestate.h"
#include "actor.h"
#include "resources.h"

#include "config.h"

Map *loadMap(const char *path, Texture tileset)
{
    // file reading
    FILE *f = fopen(path, "rb");
    if (!f)
    {
        printf("File not found: %s\n", path);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (length <= 0)
    {
        printf("Error: Empty or invalid file\n");
        fclose(f);
        return NULL;
    }

    char *content = malloc((long long)length + 1);
    if (!content)
    {
        printf("Memory allocation for content failed\n");
        fclose(f);
        return NULL;
    }

    fread(content, 1, length, f);
    fclose(f);
    content[length] = '\0';

    cJSON *root = cJSON_Parse(content);
    free(content);

    if (!root)
    {
        printf("Error parsing JSON\n");
        return NULL;
    }

    // actual parsing now
    Map *m = malloc(sizeof(Map));
    if (!m)
    {
        printf("Memory allocation for map failed\n");
        return NULL;
    }
    m->x = 0;
    m->y = 0;
    m->w = 0;
    m->h = 0;
    m->tw = 0;
    m->th = 0;
    m->layerCount = 0;
    m->tileset = tileset;

    cJSON *w = cJSON_GetObjectItem(root, "width");
    cJSON *h = cJSON_GetObjectItem(root, "height");
    cJSON *tw = cJSON_GetObjectItem(root, "tilewidth");
    cJSON *th = cJSON_GetObjectItem(root, "tileheight");
    if (!w || !h || !tw || !th)
    {
        printf("Error: Missing root attributes\n");
        cJSON_Delete(root);
        free(m);
        return NULL;
    }

    m->w = w->valueint;
    m->h = h->valueint;
    m->tw = tw->valueint;
    m->th = th->valueint;

    cJSON *layersArray = cJSON_GetObjectItem(root, "layers");
    if (!layersArray || !cJSON_IsArray(layersArray))
    {
        printf("Error: No layers found\n");
        cJSON_Delete(root);
        free(m);
        return NULL;
    }

    m->layerCount = cJSON_GetArraySize(layersArray);
    m->layers = malloc(sizeof(Layer) * m->layerCount);
    if (!m->layers)
    {
        printf("Error: failed to allocate layers\n");
        cJSON_Delete(root);
        free(m);
        return NULL;
    }

    int i = 0;
    cJSON *element;
    cJSON_ArrayForEach(element, layersArray)
    {
        loadLayer(element, &m->layers[i], m->tileset.w, m->tw, m->w, m->h);
        i++;
    }

    cJSON_Delete(root);

    return m;
}

void loadLayer(cJSON *in, Layer *l, int tsw, int tw, int mw, int mh)
{
    l->w = 0;
    l->h = 0;
    l->objectCount = 0;
    l->tileCount = 0;
    l->objects = NULL;
    l->tileData = NULL;
    l->tiles = NULL;

    if (!in)
    {
        printf("Error: Null layer\n");
        return;
    }

    cJSON *id = cJSON_GetObjectItem(in, "id");
    cJSON *type = cJSON_GetObjectItem(in, "type");
    if (!id || !type)
    {
        printf("Error: Missing layer attributes in general (id, type)\n");
        return;
    }

    l->id = id->valueint;
    strncpy(l->type, type->valuestring, 32);
    l->type[31] = '\0';

    if (strcmp(l->type, "tilelayer") == 0) // only tile layers have these attributes
    {
        cJSON *w = cJSON_GetObjectItem(in, "width");
        cJSON *h = cJSON_GetObjectItem(in, "height");
        if (!w || !h)
        {
            printf("Error: Missing layer attributes in tile layer (width, height)\n");
            return;
        }

        l->w = w->valueint;
        l->h = h->valueint;

        // level data
        cJSON *data = cJSON_GetObjectItem(in, "data");
        if (!data || !cJSON_IsArray(data))
        {
            printf("Error: Missing tile data\n");
            return;
        }

        int dataSize = cJSON_GetArraySize(data);
        l->tileData = malloc(sizeof(int) * dataSize);
        if (!l->tileData)
        {
            printf("Error: Memory allocation failed for tile data\n");
            return;
        }

        // count tiles
        for (int i = 0; i < dataSize; i++)
        {
            cJSON *tile = cJSON_GetArrayItem(data, i);
            if (tile)
            {
                l->tileData[i] = tile->valueint;
                if (l->tileData[i] != 0)
                {
                    l->tileCount++;
                }
            }
        }

        l->tiles = malloc(sizeof(SDL_Rect) * l->tileCount);

        // it's so bad
        // populate tiles...
        int j = 0;
        for (int i = 0; i < mw * mh; i++)
        {
            if (l->tileData[i] != 0)
            {
                // row * th but tw and th are the same so whatever...
                SDL_Rect dest = { i % mw * tw, i / mw * tw, tw, tw };
                l->tiles[j] = dest;
                j++;
            }
        }
    }
    else if (strcmp(l->type, "objectgroup") == 0)
    {
        cJSON *objects = cJSON_GetObjectItem(in, "objects");
        if (!objects || !cJSON_IsArray(objects))
        {
            printf("Error: No objects in layer\n");
            return;
        }

        l->objectCount = cJSON_GetArraySize(objects);
        if (l->objectCount == 0)
        {
            printf("Error: Object count is 0\n");
            return;
        }

        // single object from array
        cJSON *element;
        // allocate memory once before looping
        l->objects = malloc(sizeof(Object) * l->objectCount);
        if (!l->objects)
        {
            printf("Memory allocation failed\n");
            l->objects = NULL;
            return;
        }

        int i = 0;
        // iterate over objects
        cJSON_ArrayForEach(element, objects)
        {
            cJSON *id = cJSON_GetObjectItem(element, "id");
            cJSON *name = cJSON_GetObjectItem(element, "name");
            cJSON *x = cJSON_GetObjectItem(element, "x");
            cJSON *y = cJSON_GetObjectItem(element, "y");
            cJSON *w = cJSON_GetObjectItem(element, "width");
            cJSON *h = cJSON_GetObjectItem(element, "height");

            if (!id || !name || !x || !y || !w || !h)
            {
                printf("Error: Bad object data\n");
                continue;
            }

            // assign object values
            l->objects[i].body.x = (float)x->valuedouble;
            l->objects[i].body.y = (float)y->valuedouble;
            l->objects[i].body.w = (float)w->valuedouble;
            l->objects[i].body.h = (float)h->valuedouble;
            l->objects[i].id = id->valueint;

            strncpy(l->objects[i].name, name->valuestring, 32);
            l->objects[i].name[31] = '\0';

            i++; // move to the next object
        }
    }
}

void populateMap(Map *m)
{
    vector_init(m->actors);
    for (int i = 0; i < m->layerCount; i++)
    {
        if (strcmp(m->layers[i].type, "objectgroup") == 0)
        {
            for (int j = 0; j < m->layers[i].objectCount; j++)
            {
                Object o = m->layers[i].objects[j]; // said object

                Actor *a = createActorNoTex(o.body.x, o.body.y, o.body.w, o.body.h);

                if (strcmp(o.name, "killbox") == 0)
                {
                    a->id = ID_KILLBOX;
                }
                else if (strcmp(o.name, "ledge") == 0)
                {
                    a->id = ID_LEDGE;
                }
                else if (strcmp(o.name, "end") == 0)
                {
                    a->id = ID_END;
                    a->tex = texFire;
                    a->ownsTexture = false;
                    a->w = 16;
                    a->h = 16;
                    a->animSpeed = 0.15f;
                    a->animate = true;
                }
                else if (strcmp(o.name, "enemy") == 0)
                {
                    a->id = ID_ENEMY;
                    a->tex = texSlime;
                    a->ownsTexture = false;
                    a->offsetX = -1;
                    a->body.w = 15;
                    a->body.h = 12;
                    a->w = 17;
                    a->h = 13;
                    a->animSpeed = 0.05f;
                    a->animate = true;

                    // bah
                    a->offsetX = -3;
                    a->body.w -= 3;
                    a->body.h -= 2;
                    a->offsetY = -2;
                    a->vx = -1;
                    a->moving = true;
                }

                vector_push_back(m->actors, a);
            }
        }
    }
}

void depopulateMap(Map *m)
{
    for (int i = 0; i < m->actors.size; i++)
    {
        destroyActor(vector_at(m->actors, i));
    }
    vector_free(m->actors);
}

void doMapStuff(Map *m, Actor *p)
{
    // enemy ai and movement
    for (int i = 0; i < m->actors.size; i++)
    {
        Actor *enemy = vector_at(m->actors, i);

        if (enemy->id == ID_ENEMY)
        {
            if (!enemy->dead)
            {
                if (enemy->vx == 0)
                {
                    enemy->vx = -1;
                }

                float patrolVx = enemy->vx < 0 ? -1.0f : 1.0f;
                enemy->vx = patrolVx;
                enemy->moving = true;

                moveX(enemy);
                resolveMapCollisionsX(enemy, m);

                for (int j = 0; j < m->actors.size; j++)
                {
                    Actor *ledge = vector_at(m->actors, j);

                    if (ledge->id == ID_LEDGE && SDL_HasIntersectionF(&enemy->body, &ledge->body))
                    {
                        enemy->body.x = enemy->ox;
                        enemy->vx = -patrolVx;
                        break;
                    }
                }

                moveY(enemy);
                resolveMapCollisionsY(enemy, m);
            }
        }
    }

    for (int i = 0; i < m->actors.size; i++)
    {
        Actor *other = vector_at(m->actors, i);

        if (other->id == ID_ENEMY && !other->dead && SDL_HasIntersectionF(&other->body, &p->body))
        {
            float previousBottom = p->oy + p->body.h;
            float currentBottom = p->body.y + p->body.h;
            float enemyTop = other->body.y;
            bool falling = p->body.y >= p->oy;
            bool stomped = falling && previousBottom <= enemyTop + 6 && currentBottom >= enemyTop;

            if (stomped)
            {
                p->vy = -3;
                p->jumping = true;
                other->dead = true;
                if (sfxBam)
                {
                    Mix_PlayChannel(-1, sfxBam, 0);
                }
                return;
            }
        }
    }

    for (int i = 0; i < m->actors.size; i++)
    {
        Actor *other = vector_at(m->actors, i);

        if (SDL_HasIntersectionF(&other->body, &p->body))
        {
            if (other->id == ID_ENEMY)
            {
                if (!other->dead)
                {
                    gameState = GS_OVER;
                }
            }
            else if (other->id == ID_KILLBOX)
            {
                gameState = GS_OVER;
            }
            else if (other->id == ID_END)
            {
                gameState = GS_EXIT;
            }
        }
    }
}

void renderMap(SDL_Renderer *r, Map *m)
{
    if (!m || !m->layers || !m->tileset.data || m->tw <= 0 || m->th <= 0)
    {
        return;
    }

    int tilesPerRow = m->tileset.w / m->tw;
    if (tilesPerRow <= 0)
    {
        return;
    }

    // layer width and height has to be the same as the map for this to be okay
    for (int j = 0; j < m->layerCount; j++)
    {
        if (strcmp(m->layers[j].type, "tilelayer") == 0)
        {
            if (!m->layers[j].tileData)
            {
                continue;
            }

            for (int i = 0; i < m->w * m->h; i++)
            {
                int col = (m->layers[j].tileData[i] - 1) % tilesPerRow; // column in the tileset
                int row = (m->layers[j].tileData[i] - 1) / tilesPerRow; // row in the tileset

                SDL_Rect src = { col * m->tw, row * m->th, m->tw, m->th };
                SDL_Rect dest = { i % m->w * m->tw, i / m->w * m->tw, m->tw, m->tw };
                SDL_RenderCopy(r, m->tileset.data, &src, &dest);
            }
        }
        else if (strcmp(m->layers[j].type, "objectgroup") == 0)
        {
            for (int i = 0; i < m->actors.size; i++)
            {
                if (vector_at(m->actors, i)->id == ID_ENEMY)
                {
                    if (!vector_at(m->actors, i)->dead)
                    {
                        SDL_RenderDrawRectF(r, &vector_at(m->actors, i)->body);
                    }
                }
            }
        }
    }
}

void renderMapOffset(SDL_Renderer *r, Map *m, int x, int y, Actor *a)
{
    if (!m || !a || !m->layers)
    {
        return;
    }

    // the hackiest hack to make everything render in the correct order btw 
    // now i can't support any amount of layers but only 0,1 tile and 2 object
    for (int i = 0; i < m->actors.size; i++)
    {
        if (vector_at(m->actors, i)->id == ID_ENEMY)
        {
            if (!vector_at(m->actors, i)->superdead)
            {
                renderActorOffsetA(r, vector_at(m->actors, i), x, y);
            }
        }
    }

    if (!m->tileset.data || m->tw <= 0 || m->th <= 0 || m->layerCount < 2 ||
        !m->layers[0].tileData || !m->layers[1].tileData)
    {
        return;
    }

    int tilesPerRow = m->tileset.w / m->tw;
    if (tilesPerRow <= 0)
    {
        return;
    }

    for (int i = 0; i < m->w * m->h; i++)
    {
        int col = (m->layers[0].tileData[i] - 1) % tilesPerRow; // column in the tileset
        int row = (m->layers[0].tileData[i] - 1) / tilesPerRow; // row in the tileset

        if (fabsf((float)(i % m->w * m->tw) - a->body.x) < 208.0f) // bad culling
        {
            SDL_Rect src = { col * m->tw, row * m->th, m->tw, m->th };
            SDL_Rect dest = { i % m->w * m->tw + x, i / m->w * m->tw + y, m->tw, m->tw };
            SDL_RenderCopy(r, m->tileset.data, &src, &dest);
        }
    }
    for (int i = 0; i < m->w * m->h; i++)
    {
        int col = (m->layers[1].tileData[i] - 1) % tilesPerRow; // column in the tileset
        int row = (m->layers[1].tileData[i] - 1) / tilesPerRow; // row in the tileset

        if (fabsf((float)(i % m->w * m->tw) - a->body.x) < 208.0f) // bad culling
        {
            SDL_Rect src = { col * m->tw, row * m->th, m->tw, m->th };
            SDL_Rect dest = { i % m->w * m->tw + x, i / m->w * m->tw + y, m->tw, m->tw };
            SDL_RenderCopy(r, m->tileset.data, &src, &dest);
        }
    }

    for (int i = 0; i < m->actors.size; i++)
    {
        if (vector_at(m->actors, i)->id == ID_END)
        {
            renderActorOffsetA(r, vector_at(m->actors, i), x, y);
        }
    }

    // layer width and height has to be the same as the map for this to be okay
    /*for (int j = 0; j < m->layerCount; j++)
    {
        
        if (strcmp(m->layers[j].type, "objectgroup") == 0)
        {
            for (int i = 0; i < m->actors.size; i++)
            {
                if (vector_at(m->actors, i)->id == ID_ENEMY)
                {
                    if (!vector_at(m->actors, i)->dead)
                    {
                        renderActorOffset(r, vector_at(m->actors, i), x, y);
                    }
                }
            }
        }
        else if (strcmp(m->layers[j].type, "tilelayer") == 0)
        {
            for (int i = 0; i < m->w * m->h; i++)
            {
                int tilesPerRow = m->tileset.w / m->tw; // number of tiles in a row
                int col = (m->layers[j].tileData[i] - 1) % tilesPerRow; // column in the tileset
                int row = (m->layers[j].tileData[i] - 1) / tilesPerRow; // row in the tileset

                SDL_Rect src = { col * m->tw, row * m->th, m->tw, m->th };
                SDL_Rect dest = { i % m->w * m->tw + x, i / m->w * m->tw + y, m->tw, m->tw };
                SDL_RenderCopy(r, m->tileset.data, &src, &dest);
            }
        }
    }*/
}

// i hardcodeded it. the index 0, 1 are tiles and 2 is objects
void resolveMapCollisionsX(Actor *a, Map *m)
{
    for (int i = 0; i < m->layers[0].tileCount; i++)
    {
        resolveCollisionRectX(a, &m->layers[0].tiles[i]);
    }
}

void resolveMapCollisionsY(Actor *a, Map *m)
{
    for (int i = 0; i < m->layers[0].tileCount; i++)
    {
        resolveCollisionRectY(a, &m->layers[0].tiles[i]);
    }
}

void freeMap(Map *m)
{
    for (int i = 0; i < m->layerCount; i++)
    {
        if (m->layers[i].tileData)
        {
            free(m->layers[i].tileData);
        }
        if (m->layers[i].objects)
        {
            free(m->layers[i].objects);
        }
        if (m->layers[i].tiles)
        {
            free(m->layers[i].tiles);
        }
    }

    free(m->layers);
    free(m);
}
