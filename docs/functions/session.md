# Session

| Function                | Description                              |
|-------------------------|------------------------------------------|
| `session/create`        | Create a new session and make it active  |
| `session/get_name`      | Get the name of the current session      |
| `session/get_worktime`  | Get the total work time of the session   |

---

## `session/create`

Create a new session and set it as the active session.

### Parameters

| Name | Type   | Required | Description                          |
|------|--------|----------|--------------------------------------|
| name | string | no       | Session name (default: `"Untitled"`) |

### Returns

`nil` on success.

### Errors

| Code              | Meaning                          |
|-------------------|----------------------------------|
| SESSION_EXISTS    | A session with this name already exists |
| CREATE_FAILED     | Could not create the session     |

### Example

```
→ [0, 10, "session/create", ["My Project"]]
← [1, 10, nil, nil]
```

---

## `session/get_name`

Return the name of the current active session.

### Parameters

None.

### Returns

`string` — current session name.

### Errors

| Code              | Meaning                  |
|-------------------|--------------------------|
| SESSION_NOT_FOUND | No active session exists |

### Example

```
→ [0, 11, "session/get_name", []]
← [1, 11, nil, "My Project"]
```

---

## `session/get_worktime`

Return the total work time of the current session in seconds.

### Parameters

None.

### Returns

`integer` — total work time in seconds.

### Errors

| Code              | Meaning                  |
|-------------------|--------------------------|
| SESSION_NOT_FOUND | No active session exists |

### Example

```
→ [0, 12, "session/get_worktime", []]
← [1, 12, nil, 3720]
```
