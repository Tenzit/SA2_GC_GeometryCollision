#include "Structs.h"
#include "Enums.h"

#define FLOAT(val) (*(volatile float *)&(unsigned int){ ((union { float f; unsigned int u; }){ (val) }).u })

void SetHitboxConstantMaterial(CollisionElement *hitbox);
void DrawSphereHitbox(CollisionElement *hitbox, NJS_VECTOR *pos);
void DrawCylHitbox(taskwk *actionwk, CollisionElement *hitbox, NJS_VECTOR *pos);
void DrawRectHitbox(taskwk *actionwk, CollisionElement *hitbox, NJS_VECTOR *pos);
void DrawFixedRectHitbox(CollisionElement *hitbox, NJS_VECTOR *pos);
void DrawEllipsoidHitbox(taskwk *actionwk, CollisionElement *hitbox, NJS_VECTOR *pos);
void RotateFunc(CollisionElement *hitbox, taskwk *actionwk);
void RotateY(CollisionElement *hitbox, taskwk *actionwk);
void DrawPlayerHitbox(CollisionElement *hitbox, NJS_VECTOR *pos);
void DrawPushWallHitbox(taskwk *actionwk, CollisionElement *hitbox, NJS_VECTOR *pos);
void DrawHitbox(task *obj1, CollisionElement *obj1Hitbox, task *obj2, CollisionElement *obj2Hitbox);

__attribute__((section(".text"), used))
void DrawHitboxWrapper(task *obj1, CollisionElement *obj1Hitbox, task *obj2, CollisionElement *obj2Hitbox) {
    typedef void (*DoCollisionFunc)(task *obj1, CollisionElement *obj1Hitbox, task *obj2, CollisionElement *obj2Hitbox);
    DoCollisionFunc DoCollision = (DoCollisionFunc)0x800075c0;

    DoCollision(obj1, obj1Hitbox, obj2, obj2Hitbox);
    DrawHitbox(obj1, obj1Hitbox, obj2, obj2Hitbox);
    DrawHitbox(obj2, obj2Hitbox, obj1, obj1Hitbox);
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

void DrawHitbox(task *drawTask, CollisionElement *drawHitbox, task *collidedTask, CollisionElement *collidedHitbox)
{
    njSaveControl3D();
    njControl3D(NJD_CONTROL_3D_CONSTANT_MATERIAL);
    njControl3D_unset(NJD_CONTROL_3D_CONSTANT_TEXTURE_MATERIAL);

    //if ((drawHitbox->flags & 0x80000000) == 0) {
        NJS_VECTOR pos;
        COLLISIONTYPE type = drawHitbox->type;
        if (type == SPHERE || type == UNKNOWNSPHERE) {
            GetHitboxPos1(drawTask->actionwk, drawHitbox, &pos);
            DrawSphereHitbox(drawHitbox, &pos);
        }
        else if(type == VERTCYL || type == ROTCYL) {
            GetHitboxPos2(drawTask->actionwk, drawHitbox, &pos);
            DrawCylHitbox(drawTask->actionwk, drawHitbox, &pos);
        }
        else if (type == RECT1) {
            GetHitboxPos2(drawTask->actionwk, drawHitbox, &pos);
            DrawRectHitbox(drawTask->actionwk, drawHitbox, &pos);
        }
        else if (type == RECT2) {
            GetHitboxPos4(drawTask->actionwk, drawHitbox, &pos);
            DrawRectHitbox(drawTask->actionwk, drawHitbox, &pos);
        }
        else if (type == FIXEDRECT) {
            pos.x = drawHitbox->center.x; 
            pos.y = drawHitbox->center.y;
            pos.z = drawHitbox->center.z; 
            if ((drawHitbox->flags & 0x20) == 0) {
                pos.x += drawTask->actionwk->position.x;
                pos.y += drawTask->actionwk->position.y;
                pos.z += drawTask->actionwk->position.z;
            }
            DrawRectHitbox(drawTask->actionwk, drawHitbox, &pos);
        }
        else if (type == ELLIPSOID) {
            GetHitboxPos2(drawTask->actionwk, drawHitbox, &pos);
            DrawEllipsoidHitbox(drawTask->actionwk, drawHitbox, &pos);
        }
        else if (type == PLAYER) {
            GetHitboxPos2(drawTask->actionwk, drawHitbox, &pos);
            DrawPlayerHitbox(drawHitbox, &pos);
        }
        else if (type == PUSHWALL) {
            GetHitboxPos4(drawTask->actionwk, drawHitbox, &pos);
            DrawPushWallHitbox(drawTask->actionwk, drawHitbox, &pos);
        }
        //drawHitbox->flags |= 0x80000000;
    //}
    njRestoreControl3D();
return;
}

typedef void (*ConstMatWrapperFunc)(float a, float r, float g, float b);
#define NJ_SET_CONSTANT_MATERIAL_WRAPPER ((ConstMatWrapperFunc)0x800156fc)


void SetHitboxConstantMaterial(CollisionElement *hitbox) {
    unsigned char dmg = hitbox->damage & 0x3;
    // dmg == 3
    float a = FLOAT(1.0);
    float b = FLOAT(0.0);
    if (dmg == 0) {
        a = FLOAT(0.0);
        b = FLOAT(1.0);
    } else if (dmg == 1) {
        a = FLOAT(0.5);
        b = FLOAT(0.5);
    } else if (dmg == 2) {
        a = FLOAT(0.5);
        b = FLOAT(0.25);
    }
    NJ_SET_CONSTANT_MATERIAL_WRAPPER(FLOAT(1.0), FLOAT(0.0), FLOAT(0.4), a);
    return;
}
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

 //void RotateZXY(Rotation *rot) {
 //   NJ_ROTATE_Z((float *)0, rot->z);
 //   NJ_ROTATE_X((float *)0, rot->x);
 //   NJ_ROTATE_Y((float *)0, rot->y);
 //   return;
 //}

 //void RotateYZX(Rotation *rot) {
 //   NJ_ROTATE_Y((float *)0, rot->y);
 //   NJ_ROTATE_Z((float *)0, rot->z);
 //   NJ_ROTATE_X((float *)0, rot->x);
 //   return;
 //}

 __attribute__((noinline))
 void DrawSetup(NJS_VECTOR *pos) {
    NJ_PUSH_STORED_MATRIX();
    NJ_TRANSLATE_EX(pos);
    return;
 }

void ScaleAndDraw(NJS_CNK_OBJECT *obj, float sx, float sy, float sz) {
    NJ_SCALE((float *)0, sx, sy, sz);
    NJ_CNK_SIMPLE_DRAW_OBJECT(obj);
    NJ_POP_1_MATRIX();
}

__attribute__((noinline))
void Scalex2AndDraw(NJS_CNK_OBJECT *obj, NJS_VECTOR *scale) {
    float two = FLOAT(2.0);
    ScaleAndDraw(obj, scale->x*two, scale->y*two, scale->z*two);
}

void DrawSphereHitbox(CollisionElement *hitbox, NJS_VECTOR *pos)
{
    float scl = hitbox->size.x;

    DrawSetup(pos);
    SetHitboxConstantMaterial(hitbox);
    ScaleAndDraw((NJS_CNK_OBJECT *)0x80184ce0, scl, scl, scl);
    return;
}

void DrawCylHitbox(taskwk *actionwk, CollisionElement *hitbox, NJS_VECTOR *pos) {
    float scl_x = hitbox->size.x;
    float scl_y = hitbox->size.y * FLOAT(2);
    
    DrawSetup(pos);
    if (hitbox->kind == ROTCYL) {
        RotateFunc(hitbox, actionwk);
    }
    SetHitboxConstantMaterial(hitbox);
    ScaleAndDraw((NJS_CNK_OBJECT *)0x80183c70, scl_x, scl_y, scl_x);
    return;
}

void DrawRectHitbox(taskwk *actionwk, CollisionElement *hitbox, NJS_VECTOR *pos) {
    DrawSetup(pos);
    if (hitbox->type != FIXEDRECT) {
        RotateY(hitbox, actionwk);
        if (hitbox->type == RECT2) {
            NJ_ROTATE_Z((float *)0, hitbox->rot.z);
        }
    }
    SetHitboxConstantMaterial(hitbox);
    Scalex2AndDraw((NJS_CNK_OBJECT *)0x801839ec, &hitbox->size);
    return;
}

__attribute__((noinline))
void RotateY(CollisionElement *hitbox, taskwk *actionwk)
{
    Angle ang = hitbox->rot.y;
    if ((hitbox->flags & 0x8020) == 0)
    {
        ang += actionwk->rotation.y;
    }
    NJ_ROTATE_Y((float *)0, ang);
}

void DrawEllipsoidHitbox(taskwk *actionwk, CollisionElement *hitbox, NJS_VECTOR *pos)
{
   float scl_x = hitbox->size.x * FLOAT(0.1);
   float scl_y = hitbox->size.y * FLOAT(0.1);

   DrawSetup(pos);
   RotateFunc(hitbox, actionwk);
   SetHitboxConstantMaterial(hitbox);
   NJ_PUSH_STORED_MATRIX();
   ScaleAndDraw((NJS_CNK_OBJECT *)0x8017bd18, scl_x, scl_y, scl_x);

   NJ_PUSH_STORED_MATRIX();
   SOME_FUNC((float *)0, FLOAT(0.0), hitbox->size.y, FLOAT(0.0));
   ScaleAndDraw((NJS_CNK_OBJECT *)0x8017ba98, scl_x, scl_x, scl_x);

   NJ_PUSH_STORED_MATRIX();
   NJ_ROTATE_X((float *)0, 0x8000);
   SOME_FUNC((float *)0, FLOAT(0.0), -hitbox->size.y, FLOAT(0.0));
   ScaleAndDraw((NJS_CNK_OBJECT *)0x8017ba98, scl_x, scl_x, scl_x);

   NJ_POP_1_MATRIX();
   return;
}

__attribute__((noinline))
void RotateFunc(CollisionElement *hitbox, taskwk *actionwk)
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

void DrawPlayerHitbox(CollisionElement *hitbox, NJS_VECTOR *pos)
{
   unsigned int *gravityDir = (unsigned int *)0x803ad7b0;

   float scl_x = hitbox->size.x;
   float scl_y = hitbox->size.y * FLOAT(2);

   DrawSetup(pos);

   Angle xRot = 0;
   Angle zRot = 0;

   if (*gravityDir == 0) {
       zRot = -0x4000;
   } else if (*gravityDir == 1) {
       zRot = 0x4000;
   } else if (*gravityDir == 2) {
       zRot = 0x8000;
   } else if (*gravityDir == 4) {
       xRot = 0x4000;
   } else if (*gravityDir == 5) {
       xRot = -0x4000;
   }

   if (xRot != 0)
       NJ_ROTATE_X((float *)0, xRot);
   if (zRot != 0)
       NJ_ROTATE_Z((float *)0, zRot);

   SetHitboxConstantMaterial(hitbox);
   ScaleAndDraw((NJS_CNK_OBJECT *)0x80183c70, scl_x, scl_y, scl_x);
   return;  
}

void DrawPushWallHitbox(taskwk *actionwk, CollisionElement *hitbox, NJS_VECTOR *pos)
{
    float scl_x = hitbox->size.x;
    float scl_y = hitbox->size.y;

    NJ_SET_CONSTANT_MATERIAL_WRAPPER(FLOAT(1.0), FLOAT(0.4), FLOAT(1.0), FLOAT(1.0));
    DrawSetup(pos);
    RotateY(hitbox, actionwk);
    NJ_PUSH_STORED_MATRIX();
    SOME_FUNC((float *)0, FLOAT(0.0), FLOAT(0.0), FLOAT(10.0));
    ScaleAndDraw((NJS_CNK_OBJECT *)0x801839ec, FLOAT(1.0), FLOAT(1.0), FLOAT(20.0));

    NJ_PUSH_STORED_MATRIX();
    SOME_FUNC((float *)0, FLOAT(0.0), FLOAT(0.0), FLOAT(20.0));
    NJ_ROTATE_X((float *)0, 0x2000);
    SOME_FUNC((float *)0, FLOAT(0.0), FLOAT(0.0), FLOAT(-3.0));
    ScaleAndDraw((NJS_CNK_OBJECT *)0x801839ec, FLOAT(1.0), FLOAT(1.0), FLOAT(7.0));

    NJ_PUSH_STORED_MATRIX();
    SOME_FUNC((float *)0, FLOAT(0.0), FLOAT(0.0), FLOAT(20.0));
    NJ_ROTATE_X((float *)0, -0x2000);
    SOME_FUNC((float *)0, FLOAT(0.0), FLOAT(0.0), FLOAT(-3.0));
    ScaleAndDraw((NJS_CNK_OBJECT *)0x801839ec, FLOAT(1.0), FLOAT(1.0), FLOAT(7.0));

    SetHitboxConstantMaterial(hitbox);
    ScaleAndDraw((NJS_CNK_OBJECT *)0x801839ec, scl_x * FLOAT(2.0), scl_y * FLOAT(2.0), FLOAT(0.1));

    return;
}
     