# Haze Project Schema

## Overview

The Haze Project Schema defines the structure of a `.hz` project file.

A Haze project is serialized using **MessagePack** (compact binary serialization format) and is divided into logical blocks. Each block represents an independent part of the project state, allowing the file to be extended without requiring a complete rewrite of unrelated data.

The schema is designed around four principles:

* Compact binary representation
* Fast serialization and deserialization
* Append-friendly project data
* Backward-compatible versioning

## File Structure

```text
HZ Project
│
├── Header
│   ├── Schema Version
│   ├── Haze Version
│   ├── Project Name
│   ├── Author
│   ├── Created At
│   └── Project Path
│
├── Project State
│   ├── Working Time
│   └── Project Settings
│
├── Samples
│   └── Sample[]
│
├── Channels
│   └── Channel[]
│
├── Patterns
│   └── Pattern[]
│
└── Metadata
```

## Root Object

The root object is a MessagePack map.

```text
{
    "schema": ...,
    "haze": ...,
    "project": ...,
    "state": ...,
    "samples": ...,
    "channels": ...,
    "patterns": ...,
    "metadata": ...
}
```

### Root Fields

| Field      | Type  | Required | Description                       |
| ---------- | ----- | -------: | --------------------------------- |
| `schema`   | map   |      Yes | Project schema information        |
| `haze`     | map   |      Yes | Haze version information          |
| `project`  | map   |      Yes | Basic project information         |
| `state`    | map   |      Yes | Runtime-independent project state |
| `samples`  | array |      Yes | Project samples                   |
| `channels` | array |      Yes | Mixer channels                    |
| `patterns` | array |      Yes | Musical patterns                  |
| `metadata` | map   |       No | Additional project metadata       |

## Schema

```text
"schema": {
    "version": uint,
    "revision": uint
}
```

| Field      | Type | Description           |
| ---------- | ---- | --------------------- |
| `version`  | uint | Major schema version  |
| `revision` | uint | Minor schema revision |

The schema version determines structural compatibility.

Example:

```text
version = 1
revision = 0
```

## Haze Information

```text
"haze": {
    "version": string
}
```

Example:

```text
"haze": {
    "version": "0.1.0"
}
```

This identifies the Haze version that created or last saved the project.

## Project

```text
"project": {
    "name": string,
    "author": string,
    "created_at": uint64,
    "path": string
}
```

| Field        | Type   | Description                        |
| ------------ | ------ | ---------------------------------- |
| `name`       | string | Project name                       |
| `author`     | string | Project author                     |
| `created_at` | uint64 | Unix timestamp of project creation |
| `path`       | string | Last known project location        |

The path is informational and must not be treated as a permanent reference (the project may be moved).

## State

```text
"state": {
    "working_time": uint64
}
```

| Field          | Type   | Description                           |
| -------------- | ------ | ------------------------------------- |
| `working_time` | uint64 | Total project working time in seconds |

Runtime-only state must not be stored here.

## Samples

```text
"samples": [
    {
        "id": uint,
        "name": string,
        "path": string
    }
]
```

### Sample

| Field  | Type   | Description              |
| ------ | ------ | ------------------------ |
| `id`   | uint   | Unique sample identifier |
| `name` | string | Sample display name      |
| `path` | string | Sample file path         |

Samples are referenced by ID instead of duplicating sample information throughout the project.

## Channels

```text
"channels": [
    {
        "id": uint,
        "name": string,
        "volume": float,
        "pan": float,
        "samples": [uint],
        "patterns": [uint]
    }
]
```

| Field      | Type   | Description               |
| ---------- | ------ | ------------------------- |
| `id`       | uint   | Unique channel identifier |
| `name`     | string | Channel name              |
| `volume`   | float  | Channel volume            |
| `pan`      | float  | Channel stereo position   |
| `samples`  | uint[] | Referenced sample IDs     |
| `patterns` | uint[] | Referenced pattern IDs    |

A channel stores references to samples and patterns rather than embedding their complete objects.

## Patterns

```text
"patterns": [
    {
        "id": uint,
        "name": string,
        "length": uint,
        "data": ...
    }
]
```

| Field    | Type                   | Description               |
| -------- | ---------------------- | ------------------------- |
| `id`     | uint                   | Unique pattern identifier |
| `name`   | string                 | Pattern name              |
| `length` | uint                   | Pattern length            |
| `data`   | implementation-defined | Pattern event data        |

`data` is intentionally implementation-defined so the pattern representation can evolve independently from the project container.

## Metadata

```text
"metadata": {
    ...
}
```

Metadata contains optional information that does not belong to the core project schema.

Unknown metadata fields must be safely ignored by readers.

## IDs

Objects inside the project use numeric IDs.

IDs must be:

* Unique within their object type
* Stable during serialization
* Used for references between objects

For example:

```text
Sample #12
Channel #3
Pattern #7
```

A channel may reference:

```text
samples:  [12]
patterns: [7]
```

This avoids duplicating objects and makes references compact.

## Compatibility

Readers must follow these rules:

1. Unknown fields must be ignored.
2. Missing optional fields must use their default values.
3. Existing fields must not change their semantic meaning.
4. New fields should be added rather than modifying existing fields.
5. Breaking structural changes require a schema version increment.

A project with an unsupported major schema version must be rejected or explicitly migrated.

## Evolution

The schema is designed to evolve incrementally.

Example:

```text
Schema 1.0
    ↓
Schema 1.1
    + tempo
    + time_signature
    ↓
Schema 1.2
    + automation
    ↓
Schema 2.0
    structural changes
```

Minor revisions should remain backward-compatible.

Major versions may introduce incompatible structural changes.

## Serialization

The `.hz` file uses **MessagePack** as its serialization format.

The logical schema is independent of the physical MessagePack encoding.

Conceptually:

```text
.hz
│
└── MessagePack
    │
    └── Root Map
        ├── schema
        ├── haze
        ├── project
        ├── state
        ├── samples
        ├── channels
        ├── patterns
        └── metadata
```

## Design Goals

The project format should provide:

* Small file size
* Low serialization overhead
* Fast loading
* Fast saving
* Stable object references
* Easy schema migration
* Forward-compatible extensions
* Independent evolution of project components
