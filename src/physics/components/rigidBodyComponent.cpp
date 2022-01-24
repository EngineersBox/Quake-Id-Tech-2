#include "rigidBodyComponent.hpp"
#include "../../platform/game/objects/gameObject.hpp"
#include "../../scene/scene.hpp"
#include "../physicsSimulation.hpp"

#include <bullet/btBulletDynamicsCommon.h>

namespace Physics::Components {
    const char* RigidBodyComponent::component_name = "RigidBodyComponent";

    RigidBodyComponent::RigidBodyComponent() {
        auto mass = 0.0f;
        auto interia = btVector3(0, 0, 0);

        auto ground_shape = new btBoxShape(btVector3(btScalar(50.0), btScalar(50.0), btScalar(50.0)));

        if (mass != 0.0f) {
            ground_shape->calculateLocalInertia(mass, interia);
        }

        auto motion_state = new btDefaultMotionState();

        btRigidBody::btRigidBodyConstructionInfo construction_info(mass, motion_state, ground_shape, interia);

        ptr = new btRigidBody(construction_info);
        ptr->setUserPointer(this);
    }

    void RigidBodyComponent::onTick(float dt) {
        // TODO: perhaps some sort of movement reponse might be more applicable,
        // otherwise we're going to be doing a lot of unnecessary pose
        // recalculations.

        getOwner()->pose = getPose();
    }

    void RigidBodyComponent::onCreate() {
        getOwner()->getScene()->getPhysics()->addRigidBody(shared_from_this());
    }

    void RigidBodyComponent::onDestroy() {
        getOwner()->getScene()->getPhysics()->removeRigidBody(shared_from_this());
    }

    Platform::Game::Models::Pose3<float> RigidBodyComponent::getPose() const {
        const auto& world_transform = ptr->getWorldTransform();
        const auto& location = world_transform.getOrigin();
        const auto& rotation = world_transform.getRotation();

        Platform::Game::Models::Pose3<float> pose;
        pose.location.x = location.x();
        pose.location.y = location.y();
        pose.location.z = location.z();
        pose.rotation.x = rotation.x();
        pose.rotation.y = rotation.y();
        pose.rotation.z = rotation.z();
        pose.rotation.w = rotation.w();

        return pose;
    }

    float RigidBodyComponent::getMass() const {
        if (ptr->getInvMass() == 0.0f) return 0.0f;
        return 1.0f / ptr->getInvMass();
    }

    //https://studiofreya.com/game-maker/bullet-physics/bullet-physics-how-to-change-body-mass/
    void RigidBodyComponent::setMass(float mass) {
        getOwner()->getScene()->getPhysics()->removeRigidBody(shared_from_this());

        btVector3 inertia;
        ptr->getCollisionShape()->calculateLocalInertia(mass, inertia);
        ptr->setMassProps(mass, inertia);

        getOwner()->getScene()->getPhysics()->addRigidBody(shared_from_this());
    }

    glm::vec3 RigidBodyComponent::getCenterOfMass() const {
        auto& com = ptr->getCenterOfMassPosition();
        return glm::vec3(com.x(), com.y(), com.z());
    }

    void RigidBodyComponent::setCenterOfMass(const glm::vec3& center_of_mass) {
        btTransform xform;
        xform.setOrigin(btVector3(center_of_mass.x, center_of_mass.y, center_of_mass.z));
        ptr->setCenterOfMassTransform(xform);
    }

    Scenes::Structure::AABB3<float> RigidBodyComponent::getAABB() const {
        btVector3 min, max;
        ptr->getAabb(min, max);

        return Scenes::Structure::AABB3<float>(glm::vec3(min.x(), min.y(), min.z()), glm::vec3(max.x(), max.y(), max.z()));
    }

    void RigidBodyComponent::setPose(const Models::Pose3<float>& pose) {
        const auto& q = pose.rotation;
        const auto& c = pose.location;
        auto worldTrans = btTransform(btQuaternion(q.x, q.y, q.z, q.w), btVector3(c.x, c.y, c.z));
        ptr->setWorldTransform(worldTrans);
    }

    void RigidBodyComponent::setCollisionShape(btCollisionShape* collision_shape) {
        ptr->setCollisionShape(collision_shape);
    }
}