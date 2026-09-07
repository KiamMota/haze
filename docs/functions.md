# Haze RPC API — Modules, Accessors, and Client Conventions

This document describes how public Haze core functions are named and classified.
It is intended for anyone implementing a **client** that speaks MessagePack-RPC to HazeCore.

Haze does not expose a flat list of unrelated procedures. Every public call belongs to a
**module** and names an **accessor** on that module.

---

## 1. Call shape

Wire format follows MessagePack-RPC. The method string is always:

```text
<module>/<accessor>
```

Examples:

| Method              | Role                          |
| ------------------- | ----------------------------- |
| `session/create`    | action on `session`           |
| `session/get_name`  | read property on `session`    |
| `session/set_name`  | write property on `session` |
| `samplelist/import` | action on `samplelist`        |
| `audio/play`        | action on `audio`             |

- **Module** — namespace for related state and behavior (`session`, `samplelist`, `audio`, …).
- **Accessor** — the operation exposed on that module (read, write, or action).

The client sends:

```text
[type, msgid, method, params]
```

where `method` is the string `<module>/<accessor>` and `params` is an array whose
shape is defined per accessor in `func/<module>.md`.

---

## 2. What an accessor is

An **accessor** is any public RPC entry that reads, mutates, or acts on a module’s
state or capability.

There are three kinds:

| Kind     | Name pattern            | Mutates state? | Purpose                                      |
| -------- | ----------------------- | -------------- | -------------------------------------------- |
| Getter   | `<module>/get_<prop>`   | No             | Read a property or snapshot                  |
| Setter   | `<module>/set_<prop>`   | Yes            | Write a property                             |
| Action   | `<module>/<verb>`       | Depends        | Operation (create, import, play, …)          |

Rules:

- **Getters always start with `get_`** after the slash (e.g. `get_name`, `get_list`).
- **Setters always start with `set_`** after the slash (e.g. `set_name`).
- **Actions** use a bare verb (e.g. `create`, `import`, `play`). They do not use the
  `get_` / `set_` prefixes.
- Getters are **pure** with respect to project/session state: no intentional mutation.
- Setters **mutate** state held by the core.
- Actions follow their documented contract: some only command the engine (`audio/play`),
  some create or import state (`session/create`, `samplelist/import`).

Anything that gets or modifies something owned by a module is an accessor in this sense—
including actions, not only `get_` / `set_`.

---

## 3. Naming summary

```text
| Form   | Pattern               | Example            |
| ------ | --------------------- | ------------------ |
| Action | `<module>/<verb>`     | `session/create`   |
| Getter | `<module>/get_<prop>` | `session/get_name` |
| Setter | `<module>/set_<prop>` | `session/set_name` |
```

Conventions for clients:

1. Parse `method` as two path segments: module and accessor.
2. Treat `get_*` as safe to call for UI refresh without expecting state change.
3. Treat `set_*` and most actions as state-changing; refresh dependent getters afterward.
4. Do not invent method names; only call accessors documented under `func/`.

---

## 4. Contract (per accessor)

Every public accessor is specified in `func/<module>.md` with:

| Field         | Content                                              |
| ------------- | ---------------------------------------------------- |
| Name          | Full method string (`module/accessor`)               |
| Description   | Behavior only (not implementation)                   |
| Parameters    | Name, type, required flag, meaning                   |
| Return value  | Type and meaning                                     |
| Errors        | Machine-readable code + meaning                      |
| Example       | Optional request/result illustration                 |

The core is the source of truth. Clients do not mirror project logic; they call
accessors and apply results and errors.

---

## 5. Parameters and results

- `params` is always a **MessagePack array**, even when empty (`[]`).
- Order and types are fixed by the accessor contract; there are no free-form kwargs
  on the wire unless a specific accessor documents a map parameter.
- On success, MessagePack-RPC returns the result in the response `result` field.
- On failure, the response uses the `error` field; prefer documented error **codes**
  for branching, not only human-readable strings.

---

## 6. Standard entry format (documentation)

Authors of API docs and client stubs should follow this shape:

```markdown
## session/get_name

### Description
Returns the name of the current session.

### Parameters
None.

### Returns
`string` — current session name

### Errors
| Code              | Meaning                  |
| ----------------- | ------------------------ |
| SESSION_NOT_FOUND | No active session exists |

### Example
Request: method `session/get_name`, params `[]`  
Result: `"My Session"`
```

---

## 7. Mental model for client authors

```text
HazeCore
├── session
│   ├── create          (action)
│   ├── get_name        (getter)
│   └── set_name        (setter)
├── samplelist
│   ├── import          (action)
│   └── get_…           (getters)
└── audio
    └── play            (action)
```

- **Module** groups state and capability.
- **Accessor** is the RPC door into that module.
- Your client is a UI, script, or tool that only speaks these doors over MessagePack-RPC.

---

## 8. Implementation checklist

When adding support for a module in a client:

1. Read `func/<module>.md`.
2. Map each documented accessor to a typed function in your language.
3. Enforce getter vs setter vs action in your wrapper API if useful (e.g. no silent
   mutation on “get” helpers).
4. Correlate responses with `msgid`.
5. Handle listed error codes explicitly.
6. After setters/actions that change state, re-fetch getters the UI depends on
   (unless the accessor return value already carries the new snapshot).

---

## 9. Out of scope

This document does not define:

- Transport (TCP, pipe, etc.)
- Authentication
- Audio streaming payload formats
- Plugin package layout (`Community/`, manifests)

Those are covered in separate docs. The rule here is only:

> Public Haze API surface = **modules** + **accessors** (`get_`, `set_`, or action verbs).
