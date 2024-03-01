#pragma once
#include "usings.hpp"

namespace physics {
    struct BoundingBox {
    vec3 min, max;
    BoundingBox() : min(vec3(0)), max(vec3(1)) {}
    BoundingBox(const vec3 &min, const vec3 &max) : min(min), max(max) {
    }
    ~BoundingBox() {}
    vec3 get_center() const;
    vec3 get_size() const;
    bool contains(const vec3 &point) const;
    bool intersects(const BoundingBox &other) const;
    void expand(const vec3 &point);
    std::string to_string() const;
    BoundingBox merge(const BoundingBox &other) const;
    };
}