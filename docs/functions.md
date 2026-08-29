
Examples: `session/create`, `samplelist/import`, `audio/play`

### Naming

| Form              | Pattern                  | Example                  |
|-------------------|--------------------------|--------------------------|
| Action            | `<module>/<verb>`        | `session/create`         |
| Getter            | `<module>/get_<prop>`    | `session/get_name`       |
| Setter            | `<module>/set_<prop>`    | `session/set_name`       |

Getters are pure. Setters mutate state.

### Contract

Every public function is documented in `func/<module>.md` with:

- Name
- Description (behavior only, not implementation)
- Parameters (name, type, required, meaning)
- Return value
- Errors (machine-readable code + meaning)
- Example (when useful)

### Standard entry format

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
|-------------------|--------------------------|
| SESSION_NOT_FOUND | No active session exists |

### Example
Request: `session/get_name` `[]`  
Result: `"My Session"`
