# Haze - A new concept for DAWs.

![Haze](assets/ico.png)

Haze is a DAW (Digital Audio Workstation) focused on extreme cross-platform flexibility—and it is completely free!

## Archtecture

Haze was designed to work with a "kernel-based" architecture, where the kernel manages audio, state, and generated files completely independently.

The way to connect to the Haze kernel is by creating a MsgPack-RPC client.

``` mermaid
graph TD
    Client["Client (UI)"]
    RPC["MessagePack-RPC"]
    Core["HazeCore"]
    AudioHW["Audio Hardware / Drivers"]

    Client <-->|Commands / State| RPC
    RPC <--> Core

    Core <-->|Files · IO| OS["Operating System"]
    Core --> Project["In-Memory Session State"]
    Core --> File[".hz Project"]

    Core ==>|Real-Time Audio Stream| AudioHW


```
