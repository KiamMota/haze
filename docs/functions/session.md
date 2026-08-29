
# SampleList Functions

## Functions

| Function                                 | Description                                       |
| ---------------------------------------- | ------------------------------------------------- |
| [`samplelist/import`](#samplelistimport) | Imports an audio sample into the current session. |
| [`samplelist/delete`](#samplelistdelete) | Deletes an audio sample from the current session. |
| [`samplelist/get`](#samplelistget)       | Retrieves an audio sample by name.                |
| [`samplelist/list`](#samplelistlist)     | Lists all samples in the current session.         |


## `samplelist/import`

### Description

Imports an audio sample into the current session's `SampleList`.

### Parameters

| Name   | Type     | Required | Description                                   |
| ------ | -------- | -------- | --------------------------------------------- |
| `file` | `string` | Yes      | Path or filename of the audio file to import. |

### Request

```text
[1, <msgid>, "samplelist/import", ["rock_with_you.mp3"]]
```

### Returns

**Type:** `null`

Returns successfully when the sample is imported.

### Errors

| Error                          | Description                          |
| ------------------------------ | ------------------------------------ |
| `Expected a string parameter.` | The first parameter is not a string. |
| `Invalid parameter object.`    | The parameter object is invalid.     |
| `<Result message>`             | The sample could not be imported.    |


## `samplelist/delete`

### Description

Deletes an audio sample from the current session's `SampleList`.

### Parameters

| Name   | Type     | Required | Description                   |
| ------ | -------- | -------- | ----------------------------- |
| `name` | `string` | Yes      | Name of the sample to delete. |

### Request

```text
[1, <msgid>, "samplelist/delete", ["rock_with_you.mp3"]]
```

### Returns

**Type:** `null`

Returns successfully when the sample is deleted.

### Errors

| Error                          | Description                          |
| ------------------------------ | ------------------------------------ |
| `Expected a string parameter.` | The first parameter is not a string. |
| `Invalid parameter object.`    | The parameter object is invalid.     |
| `<Result message>`             | The sample could not be deleted.     |


## `samplelist/get`

### Description

Retrieves a sample from the current session's `SampleList` by name.

### Parameters

| Name   | Type     | Required | Description                     |
| ------ | -------- | -------- | ------------------------------- |
| `name` | `string` | Yes      | Name of the sample to retrieve. |

### Request

```text
[1, <msgid>, "samplelist/get", ["rock_with_you.mp3"]]
```

### Returns

**Type:** `string`

Returns the name of the requested sample.

### Errors

| Error                          | Description                               |
| ------------------------------ | ----------------------------------------- |
| `Expected a string parameter.` | The first parameter is not a string.      |
| `Invalid parameter object.`    | The parameter object is invalid.          |
| `sample not found`             | No sample with the specified name exists. |

### Example

```text
Request:
[1, 67, "samplelist/get", ["rock_with_you.mp3"]]

Response:
[0, 67, nil, "rock_with_you.mp3"]
```


## `samplelist/list`

### Description

Lists all samples currently stored in the current session's `SampleList`.

### Parameters

None.

### Request

```text
[1, <msgid>, "samplelist/list", []]
```

### Returns

**Type:** `string[]`

An array containing the names of all samples in the current `SampleList`.

### Errors

| Error                          | Description                                           |
| ------------------------------ | ----------------------------------------------------- |
| `failed to create sample list` | The server failed to create the list of sample names. |

### Example

```text
Request:
[1, 67, "samplelist/list", []]

Response:
[0, 67, nil, ["foo", "foo2", "foo3"]]
```
