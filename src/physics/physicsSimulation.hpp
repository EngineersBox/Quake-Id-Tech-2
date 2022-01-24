#pragma once

#ifndef QUAKE_PHYSICSSIMULATION_HPP
#define QUAKE_PHYSICSSIMULATION_HPP

#include <glm/glm.hpp>
#include <boost/shared_ptr.hpp>

#include "../rendering/query/traceResult.hpp"

class btMultiBodyDynamicsWorld;
class btDbvtBroadphase;
class btCollisionDispatcher;
class btDefaultCollisionConfiguration;
class btMultiBodyConstraintSolver;

namespace Physics {
    namespace Components {
        struct RigidBodyComponent;
    }

    struct PhysicsSimulation {
        PhysicsSimulation();

        void step(float dt);

        void setGravity(const glm::vec3& gravity);
        [[nodiscard]] glm::vec3 getGravity() const;

        void addRigidBody(const boost::shared_ptr<Components::RigidBodyComponent>& rigid_body);
        void removeRigidBody(const boost::shared_ptr<Components::RigidBodyComponent>& rigid_body);

        [[nodiscard]] Rendering::Query::TraceResult trace(const glm::vec3& start, const glm::vec3& end) const;

    private:
        btDbvtBroadphase* broadphaseInterface;
        btCollisionDispatcher* collisionDispatcher;
        btMultiBodyConstraintSolver* multiBodyConstraintSolver;
        btDefaultCollisionConfiguration* collisionConfiguration;
        btMultiBodyDynamicsWorld* dynamicsWorld;
    };
}

#endif //QUAKE_PHYSICSSIMULATION_HPP
