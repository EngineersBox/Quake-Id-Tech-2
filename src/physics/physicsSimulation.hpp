#pragma once

#ifndef QUAKE_PHYSICSSIMULATION_HPP
#define QUAKE_PHYSICSSIMULATION_HPP

#include <glm/glm.hpp>
#include <boost/shared_ptr.hpp>
#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletCollision/BroadphaseCollision/btDispatcher.h>
#include <bullet/BulletCollision/CollisionDispatch/btCollisionDispatcher.h>
#include <bullet/bulletDynamics/featherstone/btMultiBodyDynamicsWorld.h>
#include <bullet/bulletDynamics/featherstone/btMultiBodyConstraintSolver.h>

#include "../rendering/query/traceResult.hpp"

namespace Physics {
    struct RigidBodyComponent;

    struct PhysicsSimulation {
        PhysicsSimulation();

        void step(float dt);

        void setGravity(const glm::vec3& gravity);
        [[nodiscard]] glm::vec3 getGravity() const;

        void addRigidBody(const boost::shared_ptr<RigidBodyComponent>& rigid_body);
        void removeRigidBody(const boost::shared_ptr<RigidBodyComponent>& rigid_body);

        [[nodiscard]] Rendering::Query::TraceResult trace(const glm::vec3& start, const glm::vec3& end) const;

    private:
        btMultiBodyDynamicsWorld* dynamicsWorld;
        btDbvtBroadphase* broadphaseInterface;
        btCollisionDispatcher* collisionDispatcher;
        btDefaultCollisionConfiguration* collisionConfiguration;
        btMultiBodyConstraintSolver* multiBodyConstraintSolver;
    };
}

#endif //QUAKE_PHYSICSSIMULATION_HPP
