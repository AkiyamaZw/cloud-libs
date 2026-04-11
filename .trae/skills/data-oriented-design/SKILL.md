---
name: "data-oriented-design"
description: "Provides guidance on data-oriented design principles and practices. Invoke when user needs help with optimizing data structures, memory layouts, or performance-critical code."
---

# Data-Oriented Design

This skill provides guidance on data-oriented design (DOD) principles and practices, helping developers optimize their code for performance by focusing on data structures and memory layouts.

## What is Data-Oriented Design?

Data-oriented design is an approach to software development that prioritizes data structures and memory access patterns over traditional object-oriented design. It emphasizes:

- **Data locality**: Organizing data to maximize cache hits
- **Memory layout optimization**: Reducing memory fragmentation and overhead
- **Parallelism**: Designing data structures that enable efficient parallel processing
- **Cache-friendly access patterns**: Minimizing cache misses

## Key Principles

1. **Data-first approach**: Start with data structures, not classes
2. **Separation of data and behavior**: Store data in contiguous blocks, separate from logic
3. **Optimize for access patterns**: Design data structures based on how data is accessed
4. **Minimize indirection**: Reduce pointers and references that cause cache misses
5. **Batch processing**: Process similar data together for better cache utilization

## Use Cases

Data-oriented design is particularly useful for:

- **Game development**: Physics, rendering, and AI systems
- **High-performance computing**: Scientific simulations and data processing
- **Real-time systems**: Embedded systems and control software
- **Data-intensive applications**: Databases and data processing pipelines

## Implementation Guidelines

### 1. Data Structure Design

- Use contiguous arrays for homogeneous data
- Group related data together
- Avoid inheritance hierarchies for performance-critical code
- Use structs or flat data layouts instead of objects with virtual methods

### 2. Memory Management

- Preallocate memory where possible
- Use object pools for frequently created/destroyed objects
- Consider memory alignment for better cache performance
- Avoid dynamic memory allocation in performance-critical paths

### 3. Access Patterns

- Process data in linear fashion
- Avoid random memory access
- Use SoA (Structure of Arrays) vs AoS (Array of Structures) where appropriate
- Batch similar operations together

## Examples

### Before: Object-Oriented Approach

```cpp
class Entity {
public:
    Vector3 position;
    Vector3 velocity;
    float health;
    bool active;

    void update(float deltaTime) {
        position += velocity * deltaTime;
        // Other update logic
    }
};

std::vector<Entity> entities;

for (auto& entity : entities) {
    if (entity.active) {
        entity.update(deltaTime);
    }
}
```

### After: Data-Oriented Approach

```cpp
struct EntityData {
    std::vector<Vector3> positions;
    std::vector<Vector3> velocities;
    std::vector<float> healths;
    std::vector<bool> actives;
};

void updateEntities(EntityData& data, float deltaTime) {
    for (size_t i = 0; i < data.positions.size(); i++) {
        if (data.actives[i]) {
            data.positions[i] += data.velocities[i] * deltaTime;
        }
    }
}
```

## Common Pitfalls

- Overcomplicating data structures
- Ignoring platform-specific memory considerations
- Failing to profile before optimizing
- Applying DOD principles where they aren't needed

## When to Use

- When performance is critical
- When dealing with large datasets
- When memory access patterns are predictable
- When parallel processing is important

## Tools and Resources

### Profiling and Memory Tools
- **Profiling tools**: VTune, Tracy, Chrome Tracing
- **Memory analyzers**: Valgrind, AddressSanitizer
- **Memory profilers**: HeapTrack, Massif

### Books and Publications
- **Books**: "Data-Oriented Design" by Richard Fabian
- **Online resources**: Game Dev Talks on DOD, CppCon presentations

### Online Resources
- **GitHub Repositories**: [Data Oriented Design Resources](https://github.com/dbartolini/data-oriented-design) - A curated list of awesome data oriented design resources
- **Presentations**:
  - "Practical Examples In Data Oriented Design" - Niklas Frykholm
  - "Introduction To Data Oriented Design" - DICE
  - "A Step Towards Data Orientation" - Johan Torp
  - "Memory Optimization" - Christer Ericson
  - "Typical C++ Bullshit" - Mike Acton
  - "Three Big Lies" - Mike Acton
- **Blog Posts**:
  - "What is Data-Oriented Game Engine Design?"
  - "Data-Oriented Design and C++"
  - "The Pitfalls of Object-Oriented Programming"

### Communities
- **Forums**: GameDev.net, Reddit r/gamedev
- **Discord**: Game Development communities
- **Conferences**: GDC, CppCon, Game Access Conference

This skill provides guidance on implementing data-oriented design principles to optimize performance-critical code, focusing on efficient data structures and memory access patterns.