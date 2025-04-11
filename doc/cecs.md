```mermaid

graph TD

registry-->archetype
registry-->EntityInfo


subgraph system filed
system_manager
end

subgraph single entity field
EntityInfo-->Entity
EntityInfo-->in_chunk_index
end

subgraph archetype filed
archetype
selector
end

subgraph base
chunk-->memory
archetype-->chunk
end

EntityInfo-->chunk
```