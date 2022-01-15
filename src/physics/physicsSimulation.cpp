#include "physicsSimulation.hpp"
//#include "rigidBodyComponent.hpp"

#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletDynamics/Featherstone/btMultiBodyDynamicsWorld.h>
#include <bullet/BulletDynamics/Featherstone/btMultiBodyConstraintSolver.h>

namespace Physics {
    PhysicsSimulation::PhysicsSimulation() {
        this->collisionConfiguration = new btDefaultCollisionConfiguration();
        this->collisionDispatcher = new btCollisionDispatcher(this->collisionConfiguration);
        this->broadphaseInterface = new btDbvtBroadphase();
        this->multiBodyConstraintSolver = new btMultiBodyConstraintSolver();
        this->dynamicsWorld = new btMultiBodyDynamicsWorld(
                this->collisionDispatcher,
                this->broadphaseInterface,
                this->multiBodyConstraintSolver,
                this->collisionConfiguration
        );
    }

    void PhysicsSimulation::step(float dt) {
        dynamicsWorld->stepSimulation(dt);
        auto manifold_count = dynamicsWorld->getDispatcher()->getNumManifolds();

        for (int i = 0; i < manifold_count; ++i) {
            auto contact_manifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
            auto body0 = contact_manifold->getBody0();
            auto body1 = contact_manifold->getBody1();

            auto contact_count = contact_manifold->getNumContacts();
            for (int j = 0; j < contact_count; ++j) {
                auto contact_point = contact_manifold->getContactPoint(j);
                body0->getUserPointer();
            }
        }
    }

    void PhysicsSimulation::setGravity(const glm::vec3& gravity) {
        dynamicsWorld->setGravity(btVector3(gravity.x, gravity.y, gravity.z));
    }

    glm::vec3 PhysicsSimulation::getGravity() const {
        const auto gravity = dynamicsWorld->getGravity();
        return vec3(gravity.getX(), gravity.getY(), gravity.getZ());
    }

    void PhysicsSimulation::addRigidBody(const boost::shared_ptr<RigidBodyComponent>& rigid_body) {
        dynamicsWorld->addRigidBody(rigid_body->get_ptr());
    }

    void PhysicsSimulation::removeRigidBody(const boost::shared_ptr<RigidBodyComponent>& rigid_body) {
        dynamicsWorld->removeRigidBody(rigid_body->get_ptr());
    }

    Rendering::Query::TraceResult PhysicsSimulation::trace(const glm::vec3& start, const glm::vec3& end) const {
        Rendering::Query::TraceResult trace_result;
        const btVector3 rayFromWorld = btVector3(start.x, start.y, start.z);
        const btVector3 rayToWorld = btVector3(end.x, end.y, end.z);

        btCollisionWorld::ClosestRayResultCallback rayResultCallback(rayFromWorld, rayToWorld);
        dynamicsWorld->rayTest(rayFromWorld, rayToWorld, rayResultCallback);

        if (rayResultCallback.hasHit()) {
            trace_result.didHit = true;
            trace_result.location.x = rayResultCallback.m_hitPointWorld.x();
            trace_result.location.y = rayResultCallback.m_hitPointWorld.y();
            trace_result.location.z = rayResultCallback.m_hitPointWorld.z();
            trace_result.normal.x = rayResultCallback.m_hitNormalWorld.x();
            trace_result.normal.y = rayResultCallback.m_hitNormalWorld.y();
            trace_result.normal.z = rayResultCallback.m_hitNormalWorld.z();
        }

        return trace_result;
    }
}