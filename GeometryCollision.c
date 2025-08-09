typedef struct obj {
    unsigned int evalflags;
    void *model;
    float pos[3];
    unsigned int ang[3];
    float scl[3];
    struct obj *child;
    struct obj *sibling;
} NJS_OBJECT;

typedef struct {
    unsigned int *vlist;
    unsigned short *plist;
    float center[3];
    float r;
} NJS_CNK_MODEL;

typedef union {
    unsigned int dwords[2];
    struct {
        unsigned short size;
        unsigned short head_chunk;
        unsigned short nbIndices;
        unsigned short indexOffset;
    } members;
} ChunkVertexHead;

typedef union {
    unsigned int dwords[4];
    struct {
        float x, y, z;
        unsigned int argb;
    } members;
} ChunkVertexD8;

#define NJD_CS 64
#define NJD_CV_D8 35
#define NJD_EVAL_UNIT_POS 0x1
#define NJD_EVAL_UNIT_ANG 0x2
#define NJD_EVAL_UNIT_SCL 0x4
#define NJD_EVAL_BREAK 0x10

// Used to prevent the compiler from generating .rodata for floats
#define FLOAT(val) (*(volatile float *)&(unsigned int){ ((union { float f; unsigned int u; }){ (val) }).u })

void DrawCollision() {

    // r31 *should* be the register that's holding a pointer to csts
    // Can compare with player 0's csts

    void *csts;
    __asm__ volatile ("mr %0, %%r31" : "=r" (csts));

    void *playerPhys = *(void **)0x801e7728;
    if (playerPhys == 0) {
        return;
    }
    void *playerCsts = *(void **)((__UINTPTR_TYPE__)playerPhys + 0x90);
    if (playerCsts != csts) {
        return;
    }

    typedef void (*DrawChunkModelFunc)(NJS_OBJECT *obj);
    DrawChunkModelFunc DrawChunkModel = (DrawChunkModelFunc)0x8011e17c;

    //static short collisionPolyPlist[8] = { NJD_CS, (short)0x0005,
    //    (short)0x0001, (short)0x0003, (short)0x0000,
    //    (short)0x0001, (short)0x0002, (short)0x00ff};
    // Have to do it this way to avoid generating .rodata
    unsigned short collisionPolyPlist[8];
    collisionPolyPlist[0] = NJD_CS;
    collisionPolyPlist[1] = 0x0005;
    collisionPolyPlist[2] = 0x0001;
    collisionPolyPlist[3] = 0x0003;
    collisionPolyPlist[4] = 0x0000;
    collisionPolyPlist[5] = 0x0001;
    collisionPolyPlist[6] = 0x0002;
    collisionPolyPlist[7] = 0x00ff;
    
    unsigned int collisionPolyVlist[15];

    ChunkVertexHead cvh;
    cvh.members.head_chunk = NJD_CV_D8;
    cvh.members.indexOffset = 0;
    cvh.members.nbIndices = 3;
    cvh.members.size = 1 + cvh.members.nbIndices * sizeof(ChunkVertexD8) / sizeof(int);

    collisionPolyVlist[0] = cvh.dwords[0];
    collisionPolyVlist[1] = cvh.dwords[1];
    ChunkVertexD8 verts[3];

    float (*points)[3] = (float (*)[3])0x80294a98;

    for (int i = 0; i < 3; i++) {
        verts[i].members.x = points[i][0];
        verts[i].members.y = points[i][1];
        verts[i].members.z = points[i][2];
        verts[i].members.argb = 0x000000ff << (i * 8) | 0xff000000;
    }

    for (int i = 2; i < 14; i++) {
        collisionPolyVlist[i] = verts[(i - 2) / 4].dwords[(i - 2) % 4];
    }

    collisionPolyVlist[14] = 0x000000ff;

    NJS_OBJECT obj;
    NJS_CNK_MODEL model;

    float f = FLOAT(0.0f);
    model.center[0] = f;
    model.center[1] = f;
    model.center[2] = f;
    model.r = f; 
    model.plist = collisionPolyPlist;
    model.vlist = collisionPolyVlist;

    obj.child = 0;
    obj.sibling = 0;
    obj.evalflags = NJD_EVAL_UNIT_POS | NJD_EVAL_UNIT_ANG | NJD_EVAL_UNIT_SCL | NJD_EVAL_BREAK;
    obj.model = &model;

    DrawChunkModel(&obj);
}