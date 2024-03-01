#pragma once
#include "bbox.hpp"
#include "tostring.hpp"
#include "usings.hpp"
#include "component.hpp"
#include "node.hpp"
#include <cfloat>
#include <iostream>

namespace physics {
    struct SATProjection {
        bool did_collide() {return mtv == glm::zero<vec3>(); }
        vec3 mtv = {}, point = {}, normal = {};
        ~SATProjection() = default;
        SATProjection() = default;
        SATProjection(const vec3 &mtv, const vec3 &point, const vec3 &normal)
            : mtv(mtv), point(point), normal(normal) {}
    };

    struct Collision {
    shared_ptr<Node> a, b;
    vec3 mtv, normal, point;
    Collision(shared_ptr<Node> &a, shared_ptr<Node> &b, const SATProjection &proj)
        : a(a), b(b), mtv(proj.mtv), normal(proj.normal), point(proj.point) {
        }
    };
    // TODO: make more complex collision shapes.
    enum struct ColliderType {
        BOX,
    };
    struct Collider : public Component {
    ColliderType type = ColliderType::BOX;
    vec3 size;
    vec3 center;
    vector<vec3> points, axes;
    BoundingBox bounds;
    bool is_dirty = true;
    
    bool draw_collider = false;
    vec4 gizmo_color = {1.0f, 0.0f, 0.0f, 1.0f};
    
    vec3 last_position;
    Collider(vec3 center, vec3 size) : size(size), center(center) {}
    ~Collider() override {}
    
    void awake() override {}
    void update(const float &dt) override;
    void on_collision(const physics::Collision &collision) override;

    void serialize(YAML::Emitter &out) override;
    void deserialize(const YAML::Node &in) override;
    void transform_collider();
    void project(const vec3 &axis, float &min, float &max);
    vec3 get_closest_point_to(const vec3 &point);
    vec3 get_center() const {
        return node.lock()->get_position() + center;
    }
    vector<vec3> get_indices();
    vector<vec3> get_points();
    vector<vec3> get_axes();
    BoundingBox get_bounds();
    };
}
