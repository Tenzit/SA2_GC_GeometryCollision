typedef struct {
    float x,y,z;
} NJS_VECTOR;

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

    typedef void (*DrawPolyFunc)(NJS_VECTOR *points, NJS_VECTOR *normal, int color);
    DrawPolyFunc njDrawPolyQ = (DrawPolyFunc)0x800c3040;

    NJS_VECTOR *points = (NJS_VECTOR *)0x80294a98;

    NJS_VECTOR norm;
    norm.x = norm.y = norm.z = FLOAT(0.0);

    njDrawPolyQ(points, &norm, 1);
}