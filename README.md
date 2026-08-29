# Haze

A DAW core that does not own the UI.

Haze separates the audio engine, project state, and file system from any particular interface. The editor is a client. So is a script, a headless tool, or another application written in a different language.

## Architecture

``` mermaid
graph TD;
    Client["Client"];
    RPC["MessagePack-RPC"];
    Core["HazeCore"];
    OS["OS<br/>Pipes · Files"];
    Hardware["Hardware (Control)"];

    Client <--> RPC;
    RPC <--> Core;
    Core <--> OS;
    OS <--> Hardware;

    Core --> Project["In-Memory Project State"];
    Core --> File[".hz Project"];

    Core -.->|Audio| Client;


```

```
Client  ←→  MessagePack-RPC  ←→  HazeCore  ←→  OS (pipes, files, hardware)
                                  │
                                  ├─ in-memory project state
                                  ├─ .hz project files
                                  └─ audio → Client
```

**HazeCore** owns project state, the audio engine, and persistence. Clients talk to it only through MessagePack-RPC. Nothing in the core depends on a GUI toolkit or a specific OS UI layer.

Because the boundary is a protocol, not a shared process heap, multiple clients can drive the same core: desktop editor, automation, external tools, or a headless instance with no window at all.

## Protocol

Clients call named functions over MessagePack-RPC, for example:

```
session/create
session/get_name
samplelist/import
samplelist/get
audio/play
```

Each function has a fixed contract (parameters, result, errors). The full schema lives in `docs/`.

The core is the source of truth. Clients do not reimplement project logic; they request operations and consume results.

## Why this shape

Most DAWs bind engine, state, and UI into one binary. Haze treats the core as a service:

- UI and engine version and ship independently
- headless and automated use without a fake GUI
- clients in any language that can speak the RPC protocol
- tests hit the same interface production clients use

The editor is not the product kernel. It is one frontend on top of it.

## Status

Haze is under active development. Protocol and module docs are the integration surface; see `docs/` for API rules and per-function specifications.
