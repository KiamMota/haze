
# SampleList

| Function            | Description                          |
|---------------------|--------------------------------------|
| `samplelist/import` | Import a sample into the session     |
| `samplelist/delete` | Remove a sample from the session     |
| `samplelist/get`    | Get sample metadata by name          |
| `samplelist/list`   | List all sample names                |

---

## `samplelist/import`

Import an audio file into the current session.

### Parameters

| Name | Type   | Required | Description            |
|------|--------|----------|------------------------|
| file | string | yes      | Path to the audio file |

### Returns

`nil` on success.

### Errors

| Code               | Meaning                     |
|--------------------|-----------------------------|
| FILE_NOT_FOUND     | File does not exist         |
| UNSUPPORTED_FORMAT | Format not supported        |
| IMPORT_FAILED      | Could not import the sample |

### Example

```
→ [0, 42, "samplelist/import", ["kick.wav"]]
← [1, 42, nil, nil]
```

---

## `samplelist/delete`

Delete a sample by name.

### Parameters

| Name | Type   | Required | Description |
|------|--------|----------|-------------|
| name | string | yes      | Sample name |

### Returns

`nil` on success.

### Errors

| Code             | Meaning               |
|------------------|-----------------------|
| SAMPLE_NOT_FOUND | Sample does not exist |

### Example

```
→ [0, 43, "samplelist/delete", ["kick.wav"]]
← [1, 43, nil, nil]
```

---

## `samplelist/get`

Return metadata for a sample.

### Parameters

| Name | Type   | Required | Description |
|------|--------|----------|-------------|
| name | string | yes      | Sample name |

### Returns

```
{
  "name": string,
  "path": string,
  "duration": float,   // seconds
  "sample_rate": int,
  "channels": int
}
```

### Errors

| Code             | Meaning               |
|------------------|-----------------------|
| SAMPLE_NOT_FOUND | Sample does not exist |

### Example

```
→ [0, 44, "samplelist/get", ["kick.wav"]]
← [1, 44, nil, {
     "name": "kick.wav",
     "path": "/samples/kick.wav",
     "duration": 0.32,
     "sample_rate": 44100,
     "channels": 1
   }]
```

---

## `samplelist/list`

List all sample names in the current session.

### Parameters

None.

### Returns

`string[]` — array of sample names.

### Errors

None (returns empty array if no samples exist).

### Example

```
→ [0, 45, "samplelist/list", []]
← [1, 45, nil, ["kick.wav", "snare.wav", "hat.wav"]]
```
