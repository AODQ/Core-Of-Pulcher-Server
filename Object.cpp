/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#include <algorithm>

#include "Object.h"
#include "Utility.h"


// ------------------------------ OBJECT ------------------------------

using Obj = AOD::Object;
using AABB = AOD::AABBObj;
using AOD::Vector;
using CI = AOD::Collision_Info;

Obj::Object(Obj::Type _t) : type(_t) {
  alpha = 1;
  velocity = Vector(0,0);
  layer = 0;
  is_coloured = 0;
  rotation = 0;
  scale = {1, 1};
  Refresh_Transform();
}

Obj::Object(const AOD::Vector& _p, Type _t) :
                     type(_t), position(_p) {
  alpha = 1;
  velocity = Vector(0,0);
  layer = 0;
  is_coloured = 0;
  rotation = 0;
  scale = {1, 1};
  Refresh_Transform();
}

void Obj::Set_Position(float x, float y) {
  position = {x, y};
  Refresh_Transform();
}
void Obj::Set_Position(const Vector& vx) {
  position = vx;
  Refresh_Transform();
}
void Obj::Add_Position(float x, float y) {
  position += {x, y};
  Refresh_Transform();
}
void Obj::Add_Position(const Vector& vx) {
  position += vx;
  Refresh_Transform();
}
Vector Obj::R_Position() const { return position; }

void Obj::Set_Rotation(float ang) {
  rotation  = ang;
  Refresh_Transform();
}
void Obj::Add_Rotation(float ang) {
  rotation += ang;
  Refresh_Transform();
}
float Obj::R_Rotation() const { return rotation; }

void Obj::Apply_Force(const Vector& force) {
  velocity.x += force.x;
  velocity.y += force.y;
}
void Obj::Apply_Force(const float& x, const float& y) {
  velocity.x += x;
  velocity.y += y;
}
void Obj::Set_Velocity(const Vector& vel) {
  velocity = vel;
}
void Obj::Set_Velocity_X(const float& x) { velocity.x = x; }
void Obj::Set_Velocity_Y(const float& y) { velocity.y = y; }
void Obj::Set_Velocity(const float& x, const float& y) {
  velocity.x = x;
  velocity.y = y;
}
void Obj::Set_Torque (float torq) { rotation_velocity = torq; }

Vector Obj::R_Velocity() const { return velocity; }
float Obj::R_Torque()    const { return rotation_velocity; }
  
Vector Obj::R_Size()     { return size; }
const Obj::Type& Obj::R_Type() const {
  return type;
}
Matrix Obj::R_Matrix() const {
  return matrix;
}

void Obj::Set_Size(const Vector& vec) {
  size = vec;
  Refresh_Transform();
}
void Obj::Set_Size(int x, int y) {
  Set_Size({(float)x, (float)y});
}

void Obj::Refresh_Transform() {
  matrix.Compose( position, rotation, scale );
  transformed = true;
}
  
void Obj::Update() { }

CI Obj::Collision(Obj* coll) {
  switch ( type ) {
    case Type::nil: return false;
    case Type::Polygon:
      switch ( coll->R_Type() ){ 
        case Type::nil: return false;
        case Type::Polygon:
          return reinterpret_cast<AOD::PolyObj*>(this)->Collide(
                  reinterpret_cast<AOD::PolyObj*>(coll), velocity);
        break;
        case Type::AABB:

        break;
      }
    break;
    case Type::AABB:

    break;
  }
  return false;
}

// ------------------------- COLLISION INFO -----------------------------------

CI::Collision_Info() : collision(1), will_collide(0) {}
CI::Collision_Info(bool c) : collision(c), will_collide(0) {}
CI::Collision_Info(const AOD::Vector& v, bool c, bool wc) :
                           collision(c), will_collide(wc) {
  translation = v;
}

// ------------------------- POLYGON ------------------------------------------

using Poly = AOD::PolyObj;
using AODVecs = std::vector<AOD::Vector>;


void Poly::Build_Transform() {
}

Poly::PolyObj(): Obj(Type::Polygon) {
  vertices.clear();
}

Poly::PolyObj(const AODVecs& vert, AOD::Vector offs) :
                      Obj(Type::Polygon) {
  vertices.clear();
  vertices = vert;
  Set_Position(offs);
}


static void Order_Vertices(std::vector<AOD::Vector>& verts) {
  // get centroid, same time preparing to calculate angle of verts
  float centx = 0, centy = 0;
  std::vector<std::pair< float, AOD::Vector >> va;
  for ( auto i : verts ) {
    centx += i.x;
    centy += i.y;
    va.push_back(std::pair<float, AOD::Vector>(0, i));
  }
  centx /= verts.size();
  centy /= verts.size();

  for ( auto& i : va ) {
    i.first = std::atan2f(i.second.y - centy, i.second.x - centx);
    //std::cout << "ATAN2F( " << i.second.y << " - " << centy << ", "
    //                        << i.second.x << " - " << centx << ") = "
    //                        << i.first << '\n';
  }
  std::sort( va.begin(), va.end(),
    [](std::pair<float, AOD::Vector>& x,
       std::pair<float, AOD::Vector>& y) {
           //std::cout << x.first << " > " << y.first << '\n';
      return x.first < y.first;
    });
  // put back in vector
  verts.clear();
  for ( auto i : va )
    verts.push_back ( i.second );

  /*// double check that it is in sorted CCW order
  int count = 0;
  for ( int i = 0; i != verts.size(); ++ i ) {
    int pt1 = ( i + 1 ) % verts.size(),
        pt2 = ( i + 2 ) % verts.size();
    int z = ( verts[pt1].x - verts[i].x ) * ( verts[pt2].y - verts[pt1].y );
    z -=    ( verts[pt1].y - verts[i].y ) * ( verts[pt2].x - verts[pt1].x );
    if      ( z < 0 ) -- count;
    else if ( z > 0 ) ++ count;
  }
  if ( count <= 0 ) {
    std::cout << "ERROR, polygon is clockwise: " << count << '\n';
    std::cout << "CENT: " << centx << ", " << centy << '\n';
    for ( int i = 0; i != verts.size(); ++ i )
      std::cout << verts[i] << '\n';
    std::cout << "----------------------------\n";
  }*/
}

void Poly::Set_Vertices(const std::vector<AOD::Vector>& vert, bool reorder) {
  vertices = vert;
  if ( reorder ) {
    Order_Vertices(vertices);
  }
  Build_Transform();
}

const AODVecs& Poly::R_Vertices() const {
  return vertices;
}

const AODVecs& Poly::R_Transformed_Vertices() {
  // check if transform needs to be updated
  if ( transformed ) {
    transformed = 0;
    vertices_transform.clear();

    for ( auto i : vertices ) {
      vertices_transform.push_back(
        AOD::Vector::Transform(R_Matrix(), i));
    }
  }

  return vertices_transform;
}

// ------------------------- AABB ---------------------------------------------

AOD::AABBObj::AABBObj(AOD::Vector size) : PolyObj() {
  type = Type::AABB;
  Set_Vertices({{-size.x/2.f, -size.y/2.f},
                {-size.x/2.f,  size.y/2.f},
                { size.x/2.f,  size.y/2.f},
                { size.x/2.f, -size.y/2.f}});
}
AOD::AABBObj::AABBObj(AOD::Vector size, AOD::Vector pos) : PolyObj() {
  
}

// ------------------------- POLYGON ON POLYGON -------------------------------

// -- helper functions (prototyped)
static AOD::Vector Get_Axis(const AODVecs& vertices, int i);
// if a polygon is intersecting another, sets CI properly.
static CI Intersect_Poly(const Poly&, const Poly&, bool inverse);
// Projects a Poly on an axis, sets min and max (last two args)
static void Project_Poly(const AOD::Vector&, const AODVecs&, float&, float&);
// Tests for gap between two distances (the results of Project_Poly) and returns
// the distance between the two
static float Project_Dist(float, float, float, float);

// -- collision function
static CI PolyPolyColl(Poly* polyA, Poly* polyB, AOD::Vector velocity) {
  // -- variable definitions --
  // the minimum distance needed to translate out of collision
  float min_dist = std::numeric_limits<float>::max();
  AOD::Vector trans_vec ( {0, 0} );

  const AODVecs& vertsA = polyA->R_Transformed_Vertices(),
               & vertsB = polyB->R_Transformed_Vertices();
  CI ci;
  ci.will_collide = true;
  // -- loop/coll detection --
  // loop through all vertices.
  for ( int i = 0; i != vertsA.size() + vertsB.size(); ++ i ) {
    bool vA = (i<vertsA.size());
    // get the axis from the edge (we have to build the edge from vertices tho)
    auto& axis = Get_Axis((vA?vertsA:vertsB),
                          (vA?i: i - vertsA.size()));
    // project polygons onto axis
    float minA, minB, maxA, maxB;
    Project_Poly(axis, vertsA, minA, maxA);
    Project_Poly(axis, vertsB, minB, maxB);

    // check for a gap between the two distances
    if ( Project_Dist(minA, maxA, minB, maxB) > 0 ) {
      ci.collision = false;
    }

    // get velocity's projection
    float velP = axis.Dot_Product ( velocity );
    if ( velP < 0 ) minA += velP;
    else            maxA += velP;

    float dist = Project_Dist(minA, maxA, minB, maxB);
    if ( dist > 0 ) ci.will_collide = false;

    if ( !ci.will_collide && !ci.will_collide) break;

    // check if this is minimum translation
    dist = abs(dist);
    if ( dist < min_dist ) {
      min_dist = dist;
      trans_vec = axis;
      ci.projection = axis;
      auto d = (polyA->R_Position() - polyB->R_Position());
      if ( d.Dot_Product( trans_vec ) < 0 )
        trans_vec *= -1;
    }
  }

  // -- collision occurred, (hoor|na)ay --
  if ( ci.will_collide )
    ci.translation = trans_vec * min_dist;
  return ci;
}

// ---- helper functions (defined) ----

static void Project_Poly(const AOD::Vector& axis, const AODVecs& poly,
                         float& min, float& max) {
  min = axis.Dot_Product(poly[0]);
  max = min;

  for ( int i = 1; i != poly.size(); ++ i ) {
    float t = poly[i].Dot_Product ( axis );
    if ( t < min ) min = t;
    if ( t > max ) max = t;
  }
}
static float Project_Dist(float minA, float maxA, float minB, float maxB) {
  if ( minA < minB ) return minB - maxA;
  else               return minA - maxB;
}
static AOD::Vector Get_Axis(const AODVecs& vertices, int i) {
  auto vec1 = vertices[i],
       vec2 = vertices[(i+1)%vertices.size()];
  AOD::Vector axis ({ -(vec2.y - vec1.y), vec2.x - vec1.x });
  axis.Normalize();
  return axis;
}

// ------------------------- POLYGON ON AABB ----------------------------------
static CI PolyAABBColl(Poly* poly, AABB* aabb, AOD::Vector velocity) {
  
}

static CI AABBPolyColl(AABB* aabb, Poly* poly, AOD::Vector velocity) {

}

// ------------------------- AABB ON AABB      --------------------------------
static CI AABBAABBColl(AABB* aabbA, AABB* aabbB, AOD::Vector velocity) {
  if ((aabbA->R_Position().x + aabbA->R_Size().x < aabbB->R_Position().x - aabbB->R_Size().x ||
       aabbA->R_Position().x - aabbA->R_Size().x > aabbB->R_Position().x + aabbB->R_Size().x) ||
      (aabbA->R_Position().y + aabbA->R_Size().y < aabbB->R_Position().y - aabbB->R_Size().y ||
       aabbA->R_Position().y - aabbA->R_Size().y > aabbB->R_Position().y + aabbB->R_Size().y) )
  { return false; }
  return true;
}

// ------------------------- COLLIDE FUNCTIONS --------------------------------
// (call collision)

CI Poly::Collide(Poly* other, AOD::Vector velocity) {
  return PolyPolyColl(this, other, velocity);
}

/*CI Poly::Collide(AABB* other, AOD::Vector velocity) {
  
}

CI AABB::Collide(Poly* other, AOD::Vector velocity) {

}

CI AABB::Collide(AABB* other, AOD::Vector velocity) {

}*/