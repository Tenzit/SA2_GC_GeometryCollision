#include "Structs.h"
#include "Enums.h"


typedef void (*DrawHitboxFunc)(taskwk *actionwk, CollisionElement *hitbox);

static void SetHitboxConstantMaterial(taskwk *action, CollisionElement *hitbox);
static void DrawHitboxGeneric(NJS_CNK_OBJECT *obj, float sx, float sy, float sz, const int mul_enum);
static void DrawSetup(NJS_VECTOR *pos);
static void do_nothing(taskwk *actionwk, CollisionElement *hitbox);
static void DrawRect2Hitbox(taskwk *actionwk, CollisionElement *hitbox);
static void DrawEllipsoidHitbox(taskwk *actionwk, CollisionElement *hitbox);
static void RotateFunc(taskwk *actionwk, CollisionElement *hitbox);
static void RotateY(taskwk *actionwk, CollisionElement *hitbox);
static void DrawPlayerHitbox(taskwk *actionwk, CollisionElement *hitbox);
static void DrawPushWallHitbox(taskwk *actionwk, CollisionElement *hitbox);
static void DrawPushWallHitboxSub(const short xRot);
static void DrawHitbox(task *obj1, CollisionElement *obj1Hitbox, task *obj2, CollisionElement *obj2Hitbox);

typedef int (*MatrixFunc)();
#define NJ_PUSH_STORED_MATRIX ((MatrixFunc)0x80116098)
#define NJ_POP_1_MATRIX ((MatrixFunc)0x801160e4)

typedef void (*VectorFunc)(NJS_VECTOR *vec);
#define NJ_TRANSLATE_EX ((VectorFunc)0x8011616c)

typedef void (*MatModFunc)(float *mat, float x, float y, float z);
#define NJ_SCALE ((MatModFunc)0x80115178)
#define SOME_FUNC ((MatModFunc)0x80115bd8)

typedef void (*SimpleDrawObjFunc)(NJS_CNK_OBJECT *obj);
#define NJ_CNK_SIMPLE_DRAW_OBJECT ((SimpleDrawObjFunc)0x8011df44)

typedef void (*MatRotFunc)(float *mat, unsigned int rot);
#define NJ_ROTATE_X ((MatRotFunc)0x80115c60)
#define NJ_ROTATE_Y ((MatRotFunc)0x80115828)
#define NJ_ROTATE_Z ((MatRotFunc)0x801158a8)

__attribute__((section(".text"), used))
void DrawHitboxWrapper(task *obj1, CollisionElement *obj1Hitbox, task *obj2, CollisionElement *obj2Hitbox) {
    typedef void (*DoCollisionFunc)(task *obj1, CollisionElement *obj1Hitbox, task *obj2, CollisionElement *obj2Hitbox);
    DoCollisionFunc DoCollision = (DoCollisionFunc)0x800075c0;

    DoCollision(obj1, obj1Hitbox, obj2, obj2Hitbox);
    volatile unsigned short *playerInput = (unsigned short *)0x801cc1a6;
    //volatile unsigned char *inLevelCutscene = (unsigned char *)0x801cc1a1;


    //if (*inLevelCutscene)
    //    return;

    if (*playerInput == 0x0101) {
        DrawHitbox(obj1, obj1Hitbox, obj2, obj2Hitbox);
        DrawHitbox(obj2, obj2Hitbox, obj1, obj1Hitbox);
    }
    return;
}

typedef void (*NinjaControl3DFunc)(NJD_CONTROL_3D newFlags);
#define njControl3D ((NinjaControl3DFunc)0x8002b2e8)
#define njControl3D_unset ((NinjaControl3DFunc)0x8002b2d8)

typedef void (*VoidVoidFunc)();
#define njSaveControl3D ((VoidVoidFunc)0x8002b304)
#define njRestoreControl3D ((VoidVoidFunc)0x8002b2f8)

typedef void (*GetHitboxPosFunc)(taskwk *actionwk, CollisionElement *collision, NJS_VECTOR *adjPos);
#define GetHitboxPos1 ((GetHitboxPosFunc)0x8000ecac)
#define GetHitboxPos2 ((GetHitboxPosFunc)0x8000eb90)
#define GetHitboxPos4 ((GetHitboxPosFunc)0x8000eaa0)

static void GetFixedRectHitboxPos(taskwk *actionwk, CollisionElement *collision, NJS_VECTOR *adjPos) {
    adjPos->x = collision->center.x; 
    adjPos->y = collision->center.y;
    adjPos->z = collision->center.z; 
    if ((collision->flags & 0x20) == 0) {
        adjPos->x += actionwk->position.x;
        adjPos->y += actionwk->position.y;
        adjPos->z += actionwk->position.z;
    }
}

static void DrawHitbox(task *drawTask, CollisionElement *drawHitbox, task *collidedTask, CollisionElement *collidedHitbox)
{
    static const GetHitboxPosFunc getPosFnLUT[10] = {
        GetHitboxPos1, GetHitboxPos2, GetHitboxPos2,
        GetHitboxPos2, GetHitboxPos4, GetFixedRectHitboxPos,
        GetHitboxPos2, GetHitboxPos2, GetHitboxPos1,
        GetHitboxPos4
    };

    static const DrawHitboxFunc setupDrawHitboxFnLUT[10] = {
        do_nothing, do_nothing, RotateFunc,
        RotateY, DrawRect2Hitbox, do_nothing,
        DrawEllipsoidHitbox, DrawPlayerHitbox, do_nothing,
        DrawPushWallHitbox
    };

    if ((drawHitbox->flags & 0x80000000) != 0) {
        return;
    }
    if (drawHitbox->type > PUSHWALL) {
        return;
    }
    njSaveControl3D();
    njControl3D(NJD_CONTROL_3D_CONSTANT_MATERIAL);
    njControl3D_unset(NJD_CONTROL_3D_CONSTANT_TEXTURE_MATERIAL);

    NJS_VECTOR pos;
    COLLISIONTYPE type = drawHitbox->type;
    getPosFnLUT[type](drawTask->actionwk, drawHitbox, &pos);
    NJ_PUSH_STORED_MATRIX();
    NJ_TRANSLATE_EX(&pos);
    setupDrawHitboxFnLUT[type](drawTask->actionwk, drawHitbox);
    SetHitboxConstantMaterial(drawTask->actionwk, drawHitbox);
    switch(type) {
        case SPHERE:
        case UNKNOWNSPHERE:
            DrawHitboxGeneric((NJS_CNK_OBJECT *)0x80184ce0, drawHitbox->size.x, drawHitbox->size.x, drawHitbox->size.x, 0);
            break;
        case VERTCYL:
        case ROTCYL:
        case PLAYER:
            DrawHitboxGeneric((NJS_CNK_OBJECT *)0x80183c70, drawHitbox->size.x, drawHitbox->size.y, drawHitbox->size.x, 2);
            break;
        case RECT1:
        case RECT2:
        case FIXEDRECT:
            DrawHitboxGeneric((NJS_CNK_OBJECT *)0x801839ec, drawHitbox->size.x, drawHitbox->size.y, drawHitbox->size.z, 1);
            break;
        case PUSHWALL:
            DrawHitboxGeneric((NJS_CNK_OBJECT *)0x801839ec, drawHitbox->size.x, drawHitbox->size.y, 0.05, 1);
            break;
    };
    drawHitbox->flags |= 0x80000000;
    njRestoreControl3D();
    return;
}

typedef void (*ConstMatWrapperFunc)(float a, float r, float g, float b);
#define NJ_SET_CONSTANT_MATERIAL_WRAPPER ((ConstMatWrapperFunc)0x800156fc)


static void SetHitboxConstantMaterial(taskwk *action, CollisionElement *hitbox) {
    const unsigned char dmg = hitbox->damage & 0x3;
    // dmg == 3
    static const float aLUT[4] = {0.0, 0.5, 0.5, 1.0};
    static const float bLUT[4] = {1.0, 0.5, 0.25, 0.0};

    float r = 0.0;

    typedef int (*GetPlayerNumFunc)(task * t);
    GetPlayerNumFunc GetPlayerNumFromTask = (GetPlayerNumFunc)0x80037edc;

    if (action->collision_data != 0 && 
        GetPlayerNumFromTask(action->collision_data->collisionOwner) != -1 &&
        action->collision_data->hitObject != 0)
        r = 1.0;
    if (action->collision_data->hitObject != 0 &&
        GetPlayerNumFromTask(action->collision_data->hitObject->collisionOwner) != -1) {
            r = 1.0;        
    }

    NJ_SET_CONSTANT_MATERIAL_WRAPPER(1.0, r, 0.4, aLUT[dmg]);
    return;
}

static void ScaleAndDraw(NJS_CNK_OBJECT *obj, float sx, float sy, float sz) {
    NJ_SCALE((float *)0, sx, sy, sz);
    NJ_CNK_SIMPLE_DRAW_OBJECT(obj);
    NJ_POP_1_MATRIX();
}

static void DrawHitboxGeneric(NJS_CNK_OBJECT *obj, float sx, float sy, float sz,
                              const int mul_enum) {
    if (mul_enum == 1) {
        sx *= 2.0;
        sz *= 2.0;
        sy *= 2.0;
    } else if (mul_enum == 2) {
        sy *= 2.0;
    }
    ScaleAndDraw(obj, sx, sy, sz);
}

static void do_nothing(taskwk *actionwk, CollisionElement *hitbox) {

}

static void DrawRect2Hitbox(taskwk *actionwk, CollisionElement *hitbox) {
    RotateY(actionwk, hitbox);
    NJ_ROTATE_Z((float *)0, hitbox->rot.z);
    return;
}

static void RotateY(taskwk *actionwk, CollisionElement *hitbox)
{
    Angle ang = hitbox->rot.y + ((hitbox->flags & 0x8020) == 0) * actionwk->rotation.y;
    NJ_ROTATE_Y((float *)0, ang);
}

static void DrawEllipsoidHitbox(taskwk *actionwk, CollisionElement *hitbox)
{
    const float scl_x = hitbox->size.x * 0.1;
    const float scl_y = hitbox->size.y * 0.1;
    SetHitboxConstantMaterial(actionwk, hitbox);
    RotateFunc(actionwk, hitbox);
    NJ_PUSH_STORED_MATRIX();
    DrawHitboxGeneric((NJS_CNK_OBJECT *)0x8017bd18, scl_x, scl_y, scl_x, 0);

    NJ_PUSH_STORED_MATRIX();
    SOME_FUNC((float *)0, 0.0, hitbox->size.y, 0.0);
    DrawHitboxGeneric((NJS_CNK_OBJECT *)0x8017ba98, scl_x, scl_x, scl_x, 0);

    NJ_PUSH_STORED_MATRIX();
    SOME_FUNC((float *)0, 0.0, -hitbox->size.y, 0.0);
    NJ_ROTATE_X((float *)0, 0x8000);
    DrawHitboxGeneric((NJS_CNK_OBJECT *)0x8017ba98, scl_x, scl_x, scl_x, 0);

    NJ_POP_1_MATRIX();
    return;
}

static __attribute__((noinline))
void RotateFunc(taskwk *actionwk, CollisionElement *hitbox)
{

    int isZXY = ((hitbox->flags & 0x200) == 0);
    int doAction = ((hitbox->flags & 0x8020) == 0);

    if (doAction) {
        if (!isZXY) {
            NJ_ROTATE_Y((float *)0, actionwk->rotation.y);
        }
        NJ_ROTATE_Z((float *)0, actionwk->rotation.z);
        NJ_ROTATE_X((float *)0, actionwk->rotation.x);
        if (isZXY) {
            NJ_ROTATE_Y((float *)0, actionwk->rotation.y);
        }
    }

    if (!isZXY) {
        NJ_ROTATE_Y((float *)0, hitbox->rot.y);
    }
    NJ_ROTATE_Z((float *)0, hitbox->rot.z);
    NJ_ROTATE_X((float *)0, hitbox->rot.x);
    if (isZXY) {
        NJ_ROTATE_Y((float *)0, hitbox->rot.y);
    }
}

static void DrawPlayerHitbox(taskwk *actionwk, CollisionElement *hitbox)
{
    unsigned int *gravityDir = (unsigned int *)0x803ad7b0;
    static const short xRotLUT[6] = {
        0, 0, 0, 0, 0x4000, -0x4000
    };

    static const short zRotLUT[6] = {
     -0x4000, 0x4000, 0x8000, 0, 0, 0
    };
    NJ_ROTATE_X((float *)0, xRotLUT[*gravityDir]);
    NJ_ROTATE_Z((float *)0, zRotLUT[*gravityDir]);
    return;  
}

static void SomeFuncWrapper(const float f)
{
    SOME_FUNC((float *)0, 0, 0, f);
}

static void DrawPushWallHitbox(taskwk *actionwk, CollisionElement *hitbox)
{
    NJ_SET_CONSTANT_MATERIAL_WRAPPER(1.0, 0.4, 1.0, 1.0);
    RotateY(actionwk, hitbox);
    NJ_PUSH_STORED_MATRIX();
    SomeFuncWrapper(10.0);
    DrawHitboxGeneric((NJS_CNK_OBJECT *)0x801839ec, 1.0, 1.0, 20.0, 0);

    DrawPushWallHitboxSub(0x2000);
    DrawPushWallHitboxSub(-0x2000);

    return;
}

static __attribute__((noinline))
void DrawPushWallHitboxSub(const short xRot)
{
    NJ_PUSH_STORED_MATRIX();
    SomeFuncWrapper(20.0);
    NJ_ROTATE_X((float *)0, xRot);
    SomeFuncWrapper(-3.0);
    DrawHitboxGeneric((NJS_CNK_OBJECT *)0x801839ec, 1.0, 1.0, 7.0, 0);
}
     