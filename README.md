# Haze - A new concept for DAWs.

![Haze](assets/ico.png)

Haze is a DAW (Digital Audio Workstation) focused on extreme cross-platform flexibility—and it is completely free!

## Archtecture

Haze was designed to work with a "kernel-based" architecture, where the kernel manages audio, state, and generated files completely independently.

The way to connect to the Haze kernel is by creating a MsgPack-RPC client.

``` mermaid
graph TD;
    Client["Client (UI)"]
    RPC["MessagePack-RPC"]
    Core["HazeCore (Engine)"]
    OS["Audio Drivers<br/>"]

    Client <-->|Get / Set Session State| RPC;
    RPC <--> Core;

    Core <-->|Pipes · Files · IO| OS;

    Core --> Project["In-Memory Session State"];
    Core --> File[".hz Project"];

    OS ===>|Real-Time Audio Stream| Client;

    classDef highlight fill:#f9f,stroke:#333,stroke-width:2px;
    classDef audio stroke:#333,stroke-width:3px;


```
