/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#ifndef OBJECT_H_
#define OBJECT_H_
#pragma once

#include <vector>
#include "Matrix.h"
#include "Vector.h"

namespace AOD {
  class Collision_Info;

  // If you want collision, youshould probably use AABBObj or PolyObj
  class Object {
  private:
    void Refresh_Transform();
  public:
    enum class Type { Circle, AABB, Polygon, Ray, nil };
  protected:
    AOD::Vector position, scale;
    float rotation;
    Matrix matrix;
    Type type;
    float rotation_velocity;
    Vector velocity,
           size;
    int layer;
    float alpha;
    bool is_coloured;
    float red, green, blue;
    
    bool transformed;
  public:
    Object(Type = Type::nil);
    Object(const AOD::Vector& position, Type = Type::nil);

    // ---- ret/set ----
    void Set_Position(float, float);
    void Set_Position(const Vector&);
    void Add_Position(float, float);
    void Add_Position(const Vector&);
    Vector R_Position() const;

    void Set_Rotation(float);
    void Add_Rotation(float);
    float R_Rotation() const;

    void Apply_Force(const Vector&);
    void Apply_Force(const float& x, const float& y);
    void Set_Velocity(const Vector&);
    void Set_Velocity_X(const float& x);
    void Set_Velocity_Y(const float& y);
    void Set_Velocity(const float& x, const float& y);
    void Set_Torque(float);
    Vector R_Velocity() const;
    float R_Torque() const;

    void Set_Size(const Vector&);
    void Set_Size(int x, int y);
    inline int R_Layer() const { return layer; }
    Vector R_Size();

    const Type& R_Type() const;
    Matrix R_Matrix() const;

    // ---- utility ----

    virtual void Update();

    Collision_Info Collision(Object*);
  };

  using AODVecs = std::vector<AOD::Vector>;
  class AABBObj;

  class PolyObj : public Object {
  protected:
    AODVecs vertices, vertices_transform;
    void Build_Transform();
  public:
    PolyObj(); // empty poly
    PolyObj(const AODVecs& vertices, AOD::Vector off = {0,0});

    // ---- ret/set ----
    // will override previous vectors
    void Set_Vertices(const AODVecs& , bool reorder = 1);
    const AODVecs& R_Vertices() const;
    const AODVecs& R_Transformed_Vertices();

    // ---- utility ----

    // Returns information on current collision state with another poly
    Collision_Info Collide(PolyObj* poly, AOD::Vector velocity);
    Collision_Info Collide(AABBObj* aabb, AOD::Vector velocity);
  };

  class AABBObj : public PolyObj {
  public:
    AABBObj(AOD::Vector size = {0,0});
    AABBObj(AOD::Vector size = {0,0}, AOD::Vector pos = {0,0});

    // ---- utility ----

    // Returns information on current collision with an AABB
    Collision_Info Collide(AABBObj* aabb, AOD::Vector velocity);
    Collision_Info Collide(PolyObj* poly, AOD::Vector velocity);
  };

  // Valuable information from a collision, "translation"
  // could mean different things dependent on the collision type  
  class Collision_Info {
  public:
    bool collision,
         will_collide;
    AOD::Vector translation,
                projection, normal;
    AOD::PolyObj* obj;
    // collision will always be true in def constructor
    Collision_Info();
    Collision_Info(bool collision);
    Collision_Info(const AOD::Vector&, bool collision,
                                       bool will_collide);
  };
}

#endif