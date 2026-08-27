# Haze - A new concept for DAWs.

![Haze](assets/ico.png)

Haze is a DAW (Digital Audio Workstation) focused on extreme cross-platform flexibility—and it is completely free!

## Archtecture

Haze was designed to work with a "kernel-based" architecture, where the kernel manages audio, state, and generated files completely independently.

The way to connect to the Haze kernel is by creating a MsgPack-RPC client.

``` mermaid
graph TD;
    Client["Client"]
    RPC["MessagePack-RPC"]
    Core["HazeCore"]
    OS["OS<br/>Pipes · Files"]
    Hardware["Hardware (Control)"]

    Client <--> RPC;
    RPC <--> Core;
    Core <--> OS;
    OS <--> Hardware;

    Core --> Project["In-Memory Project State"];
    Core --> File[".hz Project"];

    Core -.->|Audio| Client;

```
