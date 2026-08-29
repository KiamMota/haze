# Haze Function Specification

## 1. Purpose

Haze exposes its functionality through named RPC functions.

A function represents a single operation that can be invoked by a Haze client through the RPC interface.

This document defines the naming conventions and documentation requirements for Haze functions.

The detailed contract of each function is documented separately under:

```text
func/<module>.md
```

For example:

```text
func/session.md
func/samplelist.md
func/audio.md
```

The goal is to make every Haze function understandable without requiring knowledge of its implementation.

---

## 2. Function Names

Every Haze function has a hierarchical name:

```text
<module>/<method>
```

The `module` identifies the subsystem or resource.

The `method` identifies the operation performed on that module.

Examples:

```text
session/create
session/get_name
session/get_worktime

samplelist/get
samplelist/import
samplelist/remove

audio/play
audio/stop
```

Function names are API identifiers. Their names should therefore remain stable once exposed to clients.

---

## 3. Method Naming

Haze uses three primary method forms.

### 3.1 Actions

Actions describe an operation performed on a module.

```text
<module>/<action>
```

Examples:

```text
session/create
session/delete

samplelist/import
samplelist/remove

audio/play
audio/stop
```

Actions normally use a verb that describes what the operation does.

---

### 3.2 Getters

Getters retrieve a property or value.

```text
<module>/get_<property>
```

Examples:

```text
session/get_name
session/get_worktime
```

A getter MUST NOT modify the state of the resource it accesses.

---

### 3.3 Setters

Setters modify a property or value.

```text
<module>/set_<property>
```

Examples:

```text
session/set_name
session/set_worktime
```

A setter MUST clearly identify the property being modified.

---

## 4. Function Contract

Every public Haze function MUST have a documented contract.

The contract consists of:

1. Name
2. Description
3. Parameters
4. Return value
5. Errors
6. Examples, when useful
7. Notes, when necessary

A function is considered documented only when its behavior can be understood from its contract without reading the implementation.

---

## 5. Name

The complete RPC function name.

Example:

```text
session/create
```

The name MUST exactly match the name registered by the Haze server.

Names are case-sensitive.

---

## 6. Description

The description explains what the function does.

It should describe behavior, not implementation.

Good:

> Creates a new session and makes it the active session.

Avoid:

> Calls `SessionCreate()` internally.

The implementation may change without changing the function contract.

---

## 7. Parameters

Every parameter MUST be documented.

Each parameter should define:

| Field       | Meaning                                |
| ----------- | -------------------------------------- |
| Name        | Parameter identifier                   |
| Type        | Expected data type                     |
| Required    | Whether the parameter must be provided |
| Description | Meaning and expected behavior          |

Example:

| Name | Type   | Required | Description                      |
| ---- | ------ | -------: | -------------------------------- |
| name | string |      yes | Name assigned to the new session |

Parameters should be documented in the same order expected by positional RPC calls.

When named parameters are supported, their names must also match the documented names.

JSON-RPC 2.0 supports both positional parameters and named parameters, represented respectively as an Array and an Object.

---

## 8. Return Value

Every function that produces a result MUST document its return type and meaning.

Example:

```text
Type: string
```

> The name of the current session.

For structured results, every field should be documented.

Example:

| Field    | Type    | Description                        |
| -------- | ------- | ---------------------------------- |
| name     | string  | Session name                       |
| worktime | integer | Total session work time in seconds |

The return value belongs to the function contract, not to its implementation.

---

## 9. Errors

Functions MUST document errors that clients are expected to handle.

Each documented error should contain:

| Field         | Meaning                                 |
| ------------- | --------------------------------------- |
| Code          | Machine-readable error identifier       |
| Meaning       | Condition that caused the error         |
| Client action | What the client should understand or do |

Example:

| Code                | Meaning                  | Client action               |
| ------------------- | ------------------------ | --------------------------- |
| `SESSION_NOT_FOUND` | No active session exists | Request or create a session |

RPC-level errors such as an unknown method or invalid parameters should remain distinguishable from application-level errors. JSON-RPC 2.0 defines standard errors including `Method not found`, `Invalid params`, and `Internal error`.

---

## 10. Examples

Examples are RECOMMENDED for functions whose behavior is not immediately obvious.

An example should show the RPC method and its parameters.

Example:

```text
Method:
session/get_name

Parameters:
[]

Result:
"My Session"
```

Examples should represent valid requests and responses and should not introduce behavior that is not defined elsewhere in the contract.

---

## 11. Documentation Structure

The Haze documentation is divided into two levels.

### General Specification

```text
functions.md
```

This document defines:

* naming conventions;
* function categories;
* parameter rules;
* return value rules;
* error documentation;
* compatibility rules.

### Module Documentation

```text
func/
├── session.md
├── samplelist.md
└── audio.md
```

Each module document contains the complete contract of its functions.

For example:

```text
func/session.md
```

contains:

```text
session/create
session/get_name
session/set_name
session/get_worktime
```

---

## 12. Standard Function Entry

Every function in a module document SHOULD use the following format:

````markdown
## session/get_name

### Description

Returns the name of the current session.

### Parameters

None.

### Returns

**Type:** `string`

The current session name.

### Errors

| Code | Meaning |
|---|---|
| `SESSION_NOT_FOUND` | No active session exists. |

### Example

**Request**

```text
session/get_name
[]
````

**Result**

```text
"My Session"
```

### Notes

Additional behavior or compatibility information.

```

This format intentionally keeps the information in the same order for every function.

A developer should be able to open any `func/*.md` file and immediately know where to find the parameters, result, and errors.

---

## 13. Stability

Function names, parameter meanings, and return structures form part of the Haze API contract.

Breaking changes SHOULD NOT be introduced without updating the corresponding documentation.

When a function becomes obsolete, it SHOULD be marked as deprecated before being removed.

---

## 14. Design Principle

Haze function documentation should answer five questions immediately:

> What does this function do?

> What does it require?

> What does it return?

> What can go wrong?

> How is it called?

The function documentation exists to describe the external behavior of Haze, not its internal implementation.
```
