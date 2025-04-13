// FULL DISCLOSURE: 
// This is a C++ implementation of my labs and thus a majority Connor Smiley's physics code from GAME 2005

#pragma once
#include <DirectXMath.h>
#include <future>
#include <iostream>
#include <string>
#include <vector>
#include "../../../Common/MathHelper.h"

enum ShapeType
{
    SPHERE,
    PLANE,
    AABB
};

struct HitPair
{
    int a = -1;
    int b = -1;
    DirectX::XMVECTOR mtv = DirectX::XMVectorZero();
};

struct AABB3D
{
    float MinX;
    float MaxX;
    float MinY;
    float MaxY;
    float MinZ;
    float MaxZ;
    
    AABB3D(float minX = 0, float maxX = 0, float minY = 0, float maxY = 0, float minZ = 0, float maxZ = 0)
    {
        this->MinX = minX;
        this->MaxX = maxX;
        this->MinY = minY;
        this->MaxY = maxY;
        this->MinZ = minZ;
        this->MaxZ = maxZ;
    }

    void XMFloat3Clamp(DirectX::XMFLOAT3& float3)
    {
        float3.x = MathHelper::Clamp(float3.x, MinX, MaxX);
        float3.y = MathHelper::Clamp(float3.y, MinY, MaxY);
        float3.z = MathHelper::Clamp(float3.z, MinZ, MaxZ);
    }
};

struct Body
{
private:
    // Stores radius of sphere or dimensions of box collider
    // Radius/Width, Height, Length
    DirectX::XMFLOAT3 rwhl = {};
    std::vector<Body*> Collisions;  
    
public:

    Body(DirectX::XMFLOAT3 location, ShapeType collider, DirectX::XMFLOAT3 forwardDirection = {0, 0, 1}, DirectX::XMFLOAT3 dimensions = {}, bool isDynamic = false, float invMass = 1, float drag = 0.5)
        : position(location), ShapeType(collider), forwardDirection(forwardDirection), rwhl(dimensions), isDynamic(isDynamic), invMass(invMass), drag(drag)
    {
        isDynamic = !(invMass == 0);
    }

    Body() = default;
    
    // Is this object moved by forces or collision
    bool isDynamic = false;

    // Direction and magnitude of local forces applied this frame
    DirectX::XMFLOAT3 localImpulse = {};

    // Position of the Body at the time of the physics update
    DirectX::XMFLOAT3 position = {};
    
    // Direction and distance traveled between physics updates
    DirectX::XMFLOAT3 velocity = {};
    
    // Direction the body is facing
    DirectX::XMFLOAT3 forwardDirection = {0, 0, 1};

    // Shape used for collision detection
    ShapeType ShapeType = SPHERE;

    // Collision happened
    bool collision = false;

    // 1.0 means mass of 1, 0 means mass of infinity
    float invMass = 1.0f;

    // Coefficient of friction (both static & kinetic for simplicity)
    float frictionCoefficient = 0.2f;

    // Coefficient of restitution (1.0 = no energy lost, 0.0 = all energy lost)
    float restitutionCoefficient = 0.2f;

    float maxSpeed = 10;

    // Friction force as an impulse
    DirectX::XMFLOAT3 frictionImpulse = {};

    // Normal force as an impulse
    //DirectX::XMFLOAT3 normalImpulse = {};

    // Drag
    float drag = 0.5;
    
    // Mutators for collision shape dimensions
    void SetRadius(float radius) { rwhl = {radius, 0, 0}; }
    void SetDimensions(DirectX::XMFLOAT3 dimensions) { rwhl = dimensions; }

    // Accessors for collision shape dimensions
    float GetRadius() { return rwhl.x; }
    DirectX::XMFLOAT3 GetDimensions() { return rwhl; }

    AABB3D GetAABB()
    {
        return {
            position.x - rwhl.x * 0.5f, position.x + rwhl.x * 0.5f,
            position.y - rwhl.y * 0.5f, position.y + rwhl.y * 0.5f,
            position.z - rwhl.z * 0.5f, position.z + rwhl.z * 0.5f
            };
    }
};

//TODO
/*
 *  -Add SIMD optimisations, create data structure with optimised copying for bodies on physics updates.
 *  -B side should store locations and velocities contiguously in separate XMVector arrays (potential AVX optimisation).
 *  -B side should be xmvectors, a side can potentially be float3s, maybe not.
 *  -Dyanamic bodies should have separate containers to simplify updates.
 *  -Maybe something similar for static bodies.
 *  -Create AABB 2D initial proximity check on XZ Plane
 *
 *  -Find or build math library to remove DirectX dependency from physics module.
 *  -New library vector should reinterpret_cast to XMVector anyway
 *  
 *  -Create a system for adding and removing global forces and bodies on the physics update, between thread dispatches.
 *  potentially a queue or command pattern if more stuff is needed.
 */
class PhysicsEngine
{
    // Async do not touch on main thread
    std::vector<Body> bodiesB;
    //static std::mutex bodiesBMutex;
    std::future<void> ASYNCPhysicsCalc;

public:
    static PhysicsEngine* Instance;
    PhysicsEngine()
    {
        if(PhysicsEngine::Instance != nullptr)
        {
            delete this;
        }
        else
        {
            PhysicsEngine::Instance = this;
        }
    }
    
private: 
    // can optimise
    static void Integrate(DirectX::XMVECTOR& value, const DirectX::XMVECTOR& change, float dt)
    {
        value = DirectX::XMVectorAdd(value, DirectX::XMVectorScale(change,dt));
    }

    static float Distance(const DirectX::XMVECTOR& v1, const DirectX::XMVECTOR& v2)
    {
        return DirectX::XMVector3Length(DirectX::XMVectorSubtract(v1, v2)).m128_f32[0];
    }
    
    static float DistanceSq(const DirectX::XMVECTOR& v1, const DirectX::XMVECTOR& v2)
    {
        return DirectX::XMVector3LengthSq(DirectX::XMVectorSubtract(v1, v2)).m128_f32[0];
    }
    
    static float Dot(const DirectX::XMVECTOR& v1, const DirectX::XMVECTOR& v2)
    {
        return DirectX::XMVector3Dot(v1,v2).m128_f32[0];
    }

    static void ResolveVelocities(std::vector<HitPair> collisions, std::vector<Body>* bodies)
    {
        for(int i = 0; i < collisions.size(); i++)
        {
            Body* a = &bodies->at(collisions[i].a);
            Body* b = &bodies->at(collisions[i].b);

            // No motion to resolve if both bodies are static (should never happen, but still)
            float invMassSum = a->invMass + b->invMass;
            if (invMassSum <= FLT_EPSILON)
                continue;
            
            // Velocity of A relative to B
            DirectX::XMVECTOR velBA = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&a->velocity), DirectX::XMLoadFloat3(&b->velocity));
            DirectX::XMVECTOR mtvDir = DirectX::XMVector3Normalize(collisions[i].mtv);
            float mtvMag = DirectX::XMVector3Length(collisions[i].mtv).m128_f32[0];

            // How similar motion of A relative to B is to the direction we want to move A and/or B
            float t = Dot(velBA, mtvDir);

            // Don't change velocities if object are already moving away from each other
            // (Only change if velocities within are less than 90 degrees of each other)
            if (t > 0.0f)
                continue;

            float restitution = MathHelper::Min(a->restitutionCoefficient, b->restitutionCoefficient);
            float normalImpulseMagnitude = -(1.0f + restitution) * t / invMassSum;

            // p = mv --> v = p / m
            DirectX::XMVECTOR normalImpulse = DirectX::XMVectorScale(mtvDir, normalImpulseMagnitude);
            DirectX::XMStoreFloat3(&a->velocity, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&a->velocity), DirectX::XMVectorScale(normalImpulse, a->invMass)));
            DirectX::XMStoreFloat3(&b->velocity, DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&b->velocity), DirectX::XMVectorScale(normalImpulse, b->invMass)));

            DirectX::XMVECTOR frictionImpulseDirection = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(velBA, DirectX::XMVectorScale(mtvDir, t)));
            float frictionImpulseMagnitude = -Dot(velBA, frictionImpulseDirection) / invMassSum;
            float mu = sqrt(a->frictionCoefficient * b->frictionCoefficient); // <-- Coulomb's Law (how to combine friction coefficients)
            frictionImpulseMagnitude = MathHelper::Clamp(frictionImpulseMagnitude, -normalImpulseMagnitude * mu, normalImpulseMagnitude * mu);

            // p = mv --> v = p / m
            DirectX::XMVECTOR frictionImpulse = DirectX::XMVectorScale(frictionImpulseDirection, frictionImpulseMagnitude);
            DirectX::XMStoreFloat3(&a->velocity, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&a->velocity), DirectX::XMVectorScale(frictionImpulse, a->invMass)));
            DirectX::XMStoreFloat3(&b->velocity, DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&b->velocity), DirectX::XMVectorScale(frictionImpulse, b->invMass)));

            // Update impulses for visualization (impulses applied as velocities, these are just for rendering)
            //a.normalImpulse = normalImpulse;
            //b.normalImpulse = normalImpulse;
            //a.frictionImpulse = frictionImpulse;
            //b.frictionImpulse = -frictionImpulse;
        }
    }

    // Apply mtv to A and B
    static void ResolvePositions(std::vector<HitPair> collisions, std::vector<Body>* bodies)
    {
        for (int i = 0; i < collisions.size(); i++)
        {
            Body* a = &bodies->at(collisions[i].a);
            Body* b = &bodies->at(collisions[i].b);

            if (!b->isDynamic)
            {
                DirectX::XMStoreFloat3(&a->position, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&a->position), collisions[i].mtv));
            }
            else
            {
                DirectX::XMStoreFloat3(&a->position, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&a->position), DirectX::XMVectorScale(collisions[i].mtv, 0.5)));
                DirectX::XMStoreFloat3(&b->position, DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&b->position), DirectX::XMVectorScale(collisions[i].mtv, 0.5)));
            }
        }
    }

    static bool SphereSphere(const DirectX::XMVECTOR& position1, float radius1, const DirectX::XMVECTOR& position2, float radius2)
    {
        // Collision if distance between centres is less than radii sum
        float distance = Distance(position1, position2);
        float radiiSum = radius1 + radius2;
        return distance <= radiiSum;
    }

    static bool SpherePlane(DirectX::XMVECTOR spherePosition, float radius, DirectX::XMVECTOR planePosition, DirectX::XMVECTOR normal)
    {
        // Collision if distance of circle projected onto plane normal is less than radius
        float distance = Dot(DirectX::XMVectorSubtract(spherePosition, planePosition), normal);
        return distance <= radius;
    }

    // Ensure MTV resolves A from B (to 1 from 2)
    static bool SphereSphere(DirectX::XMVECTOR position1, float radius1, DirectX::XMVECTOR position2, float radius2, DirectX::XMVECTOR& mtv)
    {
        bool collision = SphereSphere(position1, radius1, position2, radius2);
        if (collision)
        {
            DirectX::XMVECTOR direction = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(position1, position2));
            float radiiSum = radius1 + radius2;
            float distance = Distance(position1, position2);
            float depth = radiiSum - distance;
            mtv = DirectX::XMVectorScale(direction, depth);
            // Expressed in one line (same math as above):
            //mtv = Vector3.Normalize(position1 - position2) * ((radius1 + radius2) - Vector3.Distance(position1, position2));
        }
        else
        {
            mtv = DirectX::XMVectorZero();
        }
        return collision;
    }

    // Ensure MTV points FROM plane TO sphere
    static bool SpherePlane(DirectX::XMVECTOR spherePosition, float radius, DirectX::XMVECTOR planePosition, DirectX::XMVECTOR normal, DirectX::XMVECTOR& mtv)
    {
        bool collision = SpherePlane(spherePosition, radius, planePosition, normal);
        if (collision)
        {
            float distance = Dot(DirectX::XMVectorSubtract(spherePosition, planePosition), normal);
            float depth = radius - distance;
            mtv = DirectX::XMVectorScale(normal, depth);
            // Expressed in one line (same math as above):
            //mtv = normal * (radius - Vector3.Dot(spherePosition - planePosition, normal));
        }
        else
        {
            mtv = DirectX::XMVectorZero();
        }
        return collision;
    }
    // Still broken for 3D
    //bool AABBPlane(AABB aabb, DirectX::XMVECTOR planePosition, DirectX::XMVECTOR normal, DirectX::XMVECTOR& mtv)
    //{
    //    std::vector<DirectX::XMVECTOR> verts = 
    //    {
    //        DirectX::XMVectorSet(aabb.MinX, aabb.MinY, 0, 0),
    //        DirectX::XMVectorSet(aabb.MaxX, aabb.MinY, 0, 0),
    //        DirectX::XMVectorSet(aabb.MaxX, aabb.MaxY, 0, 0),
    //        DirectX::XMVectorSet(aabb.MinX, aabb.MaxY, 0, 0),
    //    };
//
    //    // Check if vert direction returns a negative dot
    //    float distance = FLT_MAX;
    //    for (int i = 0; i < 4; i++)
    //    {
    //        float vertDist = Dot(DirectX::XMVectorSubtract(verts[i], planePosition), normal);
    //        if (vertDist < distance)
    //            distance = vertDist;
    //    }
    //    
    //    if (distance > 0)
    //    {
    //        mtv = DirectX::XMVectorZero();
    //        return false;
    //    }
//
    //    mtv = DirectX::XMVectorScale(normal, -distance);
    //    return true;
    //}
    //
    //bool AABBAABB(DirectX::XMVECTOR position1, DirectX::XMVECTOR position2, AABB aABB1, AABB aABB2, DirectX::XMVECTOR& mtv)
    //{
    //    // Dertermine order-dependent direction
    //    DirectX::XMVECTOR dir = DirectX::XMVectorSubtract(position1, position2);
    //    bool flipX = DirectX::XMVectorGetX(dir) < 0;
    //    bool flipY = DirectX::XMVectorGetY(dir) < 0;
    //    float minMaxX = MathHelper::Min(aABB1.MaxX, aABB2.MaxX);
    //    float maxMinX = MathHelper::Max(aABB1.MinX, aABB2.MinX);
    //    float minMaxY = MathHelper::Min(aABB1.MaxY, aABB2.MaxY);
    //    float maxMinY = MathHelper::Max(aABB1.MinY, aABB2.MinY);
    //    // Calculate the overlap in each axis
    //    float overlapX = minMaxX - maxMinX;
    //    float overlapY = minMaxY - maxMinY;
    //    
    //    if (overlapX > 0 && overlapY > 0)
    //    {
    //        if (overlapX < overlapY)
    //        {
    //            mtv = flipX ? DirectX::XMVectorSet(-overlapX, 0.0f, 0.0f,0.0f) : DirectX::XMVectorSet(overlapX, 0.0f, 0.0f,0.0f);
    //        }
    //        else
    //        {
    //            mtv = flipY ? DirectX::XMVectorSet(0.0f, -overlapY, 0.0f,0.0f) : DirectX::XMVectorSet(0.0f, overlapY, 0.0f,0.0f);
    //        }
//
    //        return true;
    //    }
//
    //    mtv = DirectX::XMVectorZero();
    //    return false;
    //}
    
    static bool SphereAABB(DirectX::XMVECTOR circlePosition, float radius, DirectX::XMVECTOR aabbPos, AABB3D aabb, DirectX::XMVECTOR& mtv)
    {
        DirectX::XMFLOAT3 spherePosClamped;
        DirectX::XMStoreFloat3(&spherePosClamped, circlePosition);
        aabb.XMFloat3Clamp(spherePosClamped);
        DirectX::XMVECTOR nearestPos = DirectX::XMLoadFloat3(&spherePosClamped);

        float distanceSqrd = DistanceSq(nearestPos, circlePosition);
        if(distanceSqrd < radius*radius)
        {
            mtv = DirectX::XMVectorScale(DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(circlePosition, nearestPos)), (radius - sqrt(distanceSqrd)));
            return true;
        }

        mtv = DirectX::XMVectorZero();
        return false;
    }
    
    
    static void PhysicsCalculations(std::vector<Body>* bodies, std::vector<DirectX::XMFLOAT3>* globalForces, float dt)
    {
        // Apply motion //////////////////////////////////////////////////////////////////////
        // Global Forces
        //std::lock_guard<std::mutex> lock(bodiesBMutex);// just to be safe but should not be necessary
        DirectX::XMVECTOR acceleration = DirectX::XMVectorZero();
        for (int i = 0; i < globalForces->size(); i++)
        {
            acceleration = DirectX::XMVectorAdd(acceleration, DirectX::XMLoadFloat3(&globalForces->at(i)));
        }

        for (int i = 0; i < bodies->size(); i++)
        {
            if(bodies->at(i).isDynamic)
            {
                // Local Forces
                DirectX::XMVECTOR localAcceleration = DirectX::XMVectorZero();
            
                localAcceleration = DirectX::XMLoadFloat3(&bodies->at(i).localImpulse);
                localAcceleration = DirectX::XMVectorAdd(localAcceleration, acceleration);
                DirectX::XMFLOAT3 test;
                DirectX::XMStoreFloat3(&test, localAcceleration);
                //std::cout << std::to_string(test.x) << " : " << std::to_string(test.y) << " : " << std::to_string(test.z) << "\n";
                // Load vectors
                DirectX::XMVECTOR velocity = DirectX::XMLoadFloat3(&bodies->at(i).velocity);
                DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&bodies->at(i).position);
            

                // Apply drag
                velocity = DirectX::XMVectorScale(velocity, pow(bodies->at(i).drag, dt));

                // Apply motion
                //Integration potentially another place for SIMD optimizations
                Integrate(velocity, localAcceleration, dt);
                Integrate(position, velocity, dt);

                DirectX::XMFLOAT3 v;
                DirectX::XMFLOAT3 p;
                DirectX::XMStoreFloat3(&v, velocity);
                DirectX::XMStoreFloat3(&p, position);
            
                DirectX::XMStoreFloat3(&bodies->at(i).velocity, velocity);
                DirectX::XMStoreFloat3(&bodies->at(i).position, position);
                
            }
            
        }

        // Collision detection /////////////////////////////////////////////////////////////
        // Reset collision flag before hit-testing
        for (int i = 0; i < bodies->size(); i++)
        {
            bodies->at(i).collision = false;
        }

        std::vector<HitPair> collisions;
        // Test all bodies for collision, store pairs of colliding objects
        // TODO initial loop should only check dyanmic bodies
        //for (int i = 0; i < bodies->size(); i++)
        int i = 0;
        {
            for (int j = i + 1; j < bodies->size(); j++)
            {
                Body a = bodies->at(i);
                Body b = bodies->at(j);

                DirectX::XMVECTOR mtv = DirectX::XMVectorZero();
                bool collision = false;
                if (a.ShapeType == ShapeType::SPHERE && b.ShapeType == ShapeType::SPHERE)
                {
                    collision = SphereSphere(DirectX::XMLoadFloat3(&a.position), a.GetRadius(), DirectX::XMLoadFloat3(&b.position), b.GetRadius(), mtv);
                }
                else if (a.ShapeType == ShapeType::SPHERE && b.ShapeType == ShapeType::PLANE)
                {
                    collision = SpherePlane(DirectX::XMLoadFloat3(&a.position), a.GetRadius(), DirectX::XMLoadFloat3(&b.position), DirectX::XMLoadFloat3(&b.forwardDirection), mtv);
                }
                else if (a.ShapeType == ShapeType::SPHERE && b.ShapeType == ShapeType::AABB)
                {
                    collision = SphereAABB(DirectX::XMLoadFloat3(&a.position), a.GetRadius(), DirectX::XMLoadFloat3(&b.position), b.GetAABB(), mtv);
                }
                else if (a.ShapeType == ShapeType::PLANE && b.ShapeType == ShapeType::SPHERE)
                {
                    collision = SpherePlane(DirectX::XMLoadFloat3(&b.position), b.GetRadius(), DirectX::XMLoadFloat3(&a.position), DirectX::XMLoadFloat3(&a.forwardDirection), mtv);
                }
                else if (a.ShapeType == ShapeType::PLANE && b.ShapeType == ShapeType::AABB)
                {
                    collision = false;
                    //collision = AABBPlane(b.GetAABB(), a.pos, a.normal, out mtv);
                }
                else if (a.ShapeType == ShapeType::AABB && b.ShapeType == ShapeType::AABB)
                {
                    collision = false;
                    //collision = AABBAABB(a.pos, b.pos, a.GetAABB(), b.GetAABB(), out mtv);
                }
                else if (a.ShapeType == ShapeType::AABB && b.ShapeType == ShapeType::PLANE)
                {
                    collision = false;
                    //collision = AABBPlane(a.GetAABB(), b.pos, b.normal, out mtv);
                }
                else if (a.ShapeType == ShapeType::AABB && b.ShapeType == ShapeType::SPHERE)
                {
                    collision = SphereAABB(DirectX::XMLoadFloat3(&b.position), b.GetRadius(), DirectX::XMLoadFloat3(&a.position), a.GetAABB(), mtv);
                }
                else
                {
                    std::cout << "Invalid collision test.\n";
                }
                a.collision |= collision;
                b.collision |= collision;

                if (collision)
                {
                    
                    HitPair hitPair;
                    hitPair.a = i; hitPair.b = j;
                    hitPair.mtv = mtv;
                    collisions.push_back(hitPair);
                }
            }
        }
        
        // Resolve Collisions ///////////////////////////////////////////////////////////////////////
        // Pre-pass to ensure A is *always* dynamic and MTV points from B to A
        for (int i = 0; i < collisions.size(); i++)
        {
            if (!bodies->at(collisions[i].a).isDynamic)
            {
                int temp = collisions[i].a;
                collisions[i].a = collisions[i].b;
                collisions[i].b = temp;

                if (Dot(DirectX::XMVector3Normalize(collisions[i].mtv), DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&bodies->at(collisions[i].a).position), DirectX::XMLoadFloat3(&bodies->at(collisions[i].b).position)))) < 0.0f)
                {
                    collisions[i].mtv = DirectX::XMVectorScale(collisions[i].mtv, -1.0f);
                }
            }
        }

        ResolveVelocities(collisions, bodies);
        ResolvePositions(collisions, bodies);

        //for (Body body : *bodies)
        //{
        //    DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&body.velocity);
        //    float l = DirectX::XMVector3Length(v).m128_f32[0];
        //    
        //    if(l > body.maxSpeed)
        //        v = DirectX::XMVectorScale(DirectX::XMVector3Normalize(v), body.maxSpeed);
        //    
        //    DirectX::XMStoreFloat3(&body.velocity, v);
        //}
        //std::lock_guard<std::mutex> lock(bodiesBMutex);
        
    }
    
public:
    
    std::vector<Body> bodiesA;
    DirectX::XMFLOAT3 gravity = {0, -9.81, 0};

    // This should not be changed between physics updates for thread safety
    std::vector<DirectX::XMFLOAT3> globalForces = {gravity};
    
    Body& AddBody(Body body)
    {
        int length = bodiesA.size();
        bodiesA.emplace_back(body);
        return bodiesA[length];
    }

    void Init()
    {
        ASYNCPhysicsCalc = std::async(std::launch::async, PhysicsCalculations, &bodiesB, &globalForces, 0);
        for (int i = 0; i < bodiesA.size(); i++)
        {
           bodiesB.push_back(bodiesA[i]);
        }
    }
    
    void PhysicsUpdate(float dt)
    {
        ASYNCPhysicsCalc.get();
        // no method to remove bodies yet but this will all be changed anyway.
        int numBodies = bodiesA.size();
        if(bodiesA.size() > bodiesB.size())
        {
            int difference = bodiesA.size() - bodiesB.size();
            int index = bodiesA.size() - 1;

            for(int i = 0; i < difference; i++, index++)
            {
                bodiesB.push_back(bodiesA[index]);
            }
        }
        
        for (int i = 0; i < numBodies; i++)
        {
            // can accelerate copying with intrinsics and more complex data handling
            bodiesA[i].velocity = bodiesB[i].velocity;
            bodiesA[i].position = bodiesB[i].position;
            bodiesB[i].localImpulse = bodiesA[i].localImpulse;
            bodiesA[i].localImpulse = {};
        }
        
        ASYNCPhysicsCalc = std::async(std::launch::async,PhysicsCalculations, &bodiesB, &globalForces, dt);
    }
};
