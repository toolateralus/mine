## Physics:

- in the Collider classes, we can cache a model space mesh for the collision shape and just update the transformed
version instead of regenerating the shape each time it moves.

- The octree suffers from some memory issues, it could be less recursive and use a better allocation strategy, somehow.

- Rotation in dynamic rigidbody collision resolution is neccesary : things need to tip over.

## Renderer: 

- instanced batching is a must, at least to have the option.
- already-compiled shaders should be cached and reused: we have some duplicate shaders being compiled
- hot reloading shaders is somewhat easy to implement, and is useful for designing new shaders.

## Node:

- right now, Component serialization / deserialization is plagued by the need to recompile the node class with if statements and hard coded strategy for adding components to nodes on deserialization. we should just have some factory functions that can get registered in some way so users don'thave to recompile the engine code or modify it when creating new types.

