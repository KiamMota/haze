# Haze Functions Specification

## 1. Overview

Haze exposes functionality through named RPC functions.

Each function follows the naming convention:

```text
<module>/<method>
```

For property access, the following conventions are used:

```text
<module>/get_<property>
<module>/set_<property>
```

Examples:

```text
session/create
session/get_name
session/set_name
samplelist/import
audio/play
```

Function names are part of the Haze API contract and should remain stable once publicly exposed.

---

## 2. Function Contract

Every documented function should define the following information:

### Name

The complete RPC function name.

Example:

```text
session/get_name
```

### Description

A short description of what the function does.

### Parameters

The parameters accepted by the function.

Each parameter should specify:

* name;
* type;
* required or optional;
* description.

### Returns

The value returned by the function.

The return definition should specify:

* type;
* description;
* structure, when applicable.

### Errors

Errors that the function may return.

Each error should specify:

* error condition;
* error meaning.

### Notes

Optional information about behavior, limitations, or implementation requirements.

---

## 3. Function Documentation Format

Each function should follow this structure:

```text
## <function-name>

### Description

<what the function does>

### Parameters

| Name | Type | Required | Description |
|------|------|----------|-------------|
| ...  | ...  | ...      | ...         |

### Returns

**Type:** `<type>`

<description>

### Errors

- `<error>` — <description>
- `<error>` — <description>

### Notes

<optional notes>
```

---

## 4. Example

## session/get_name

### Description

Returns the name of the current session.

### Parameters

None.

### Returns

**Type:** `string`

The name of the current session.

### Errors

* `SESSION_NOT_FOUND` — No active session exists.

---

## 5. Parameters

Parameters should use explicit types.

Common types include:

```text
string
boolean
integer
float
array
object
null
```

Complex objects should define their structure explicitly.

Example:

```text
| Name | Type | Required | Description |
|------|------|----------|-------------|
| name | string | yes | New session name |
```

---

## 6. Return Values

Return values should always be documented.

For primitive values:

```text
**Type:** `string`
```

For structured values:

```text
**Type:** `object`
```

The object's fields should then be documented.

Example:

```text
| Field | Type | Description |
|-------|------|-------------|
| name | string | Session name |
| worktime | integer | Session duration in seconds |
```

---

## 7. Modules

Functions are grouped by module.

Each module has its own documentation file:

```text
func/
├── session.md
├── samplelist.md
└── audio.md
```

The module documentation contains all functions exposed by that module.

For example:

```text
# Session Functions

## session/create

...

## session/get_name

...

## session/set_name

...
```

---

## 8. Naming Rules

Function names should use lowercase characters and `/` as the module separator.

Property access uses:

```text
get_<property>
set_<property>
```

Actions should use a direct verb:

```text
create
delete
import
remove
play
stop
pause
```

Examples:

```text
session/create
session/get_name
session/set_name

samplelist/get
samplelist/import
samplelist/remove

audio/play
audio/stop
```

The naming convention should describe the operation without exposing its internal implementation.

---

## 9. Compatibility

A function name, parameter meaning, or return structure that is already part of the public Haze API should not be changed without considering client compatibility.

Changes to the API should be reflected in the corresponding documentation before the new behavior is released.
