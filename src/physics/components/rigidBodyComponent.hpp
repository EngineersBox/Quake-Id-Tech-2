#pragma once

#include <boost/enable_shared_from_this.hpp>

#include "../../platform/game/components/gameComponent.hpp"
#include "../../models/pose.hpp"
#include "../../scene/structure/aabb.hpp"

class btRigidBody;
class btCollisionShape;

namespace Physics::Components {
    struct RigidBodyComponent : public Platform::Game::Components::GameComponent, public boost::enable_shared_from_this<RigidBodyComponent> {
        static const char* component_name;

        RigidBodyComponent();

        virtual void onTick(float dt) override;
        virtual void onCreate() override;
        virtual void onDestroy() override;

        Models::Pose3<float> getPose() const;
        float getMass() const;
        glm::vec3 getCenterOfMass() const;
        Scene::Structure::AABB3<float> getAABB() const;

        void setPose(const Models::Pose3<float>& pose);
        void setMass(float mass);
        void setCenterOfMass(const glm::vec3& center_of_mass);
        void setCollisionShape(btCollisionShape* collision_shape);

        btRigidBody* getPtr() const { return this->ptr; }
    private:
        btRigidBody* ptr;
    };
}
