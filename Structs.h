#include "Enums.h"

#ifndef STRUCTS_H
#define STRUCTS_H

typedef int Angle;

typedef struct {
    Angle x;
    Angle y;
    Angle z;
} __attribute__((packed)) Rotation;

typedef struct {
    float x;
    float y;
    float z;
} __attribute__((packed)) NJS_VECTOR;
typedef NJS_VECTOR NJS_POINT3;

typedef struct {
    unsigned char kind;
    COLLISIONTYPE type;
    unsigned char push;
    unsigned char damage;
    unsigned int flags;
    NJS_VECTOR center;
    NJS_VECTOR size;
    unsigned int unknown;
    Rotation rot;
} __attribute__((packed)) CollisionElement;

typedef struct {
    unsigned int *vlist;
    unsigned short *plist;
    NJS_POINT3 center;
    float r;
} __attribute__((packed)) NJS_CNK_MODEL;

typedef struct cnk_obj {
    NJD_EVAL evalflags;
    NJS_CNK_MODEL *model;
    NJS_VECTOR pos;
    Rotation ang;
    NJS_VECTOR scale;
    struct cnk_obj *child;
    struct cnk_obj *sibling;
} __attribute__((packed)) NJS_CNK_OBJECT;

typedef struct {
    unsigned char action;
    unsigned char subaction;
    unsigned char obj_id_q;
    unsigned char btimer;
    unsigned short statusBitfield;
    unsigned short wtimer;
    Rotation rotation;
    NJS_VECTOR position;
    NJS_VECTOR scale;
    void *collision_data;
} __attribute__((packed)) taskwk;

typedef struct task {
    struct task *prev;
    struct task *next;
    struct task *parent;
    struct task *child;
    void *updateFunc;
    void *displayFunc;
    void *deleteFunc;
    void *dispDelayedFuncs[5];
    void *someFunc;
    taskwk *actionwk;
    void *motionwk;
    void *forcewk;
    void *anywk;
    char *name;
    char *nameAgain;
    void *unknown;
} __attribute__((packed)) task;

#endif // STRUCTS_H