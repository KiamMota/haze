# hazectl

Command-line control tool for the [Haze](https://github.com/KiamMota/haze) audio workstation.

`hazectl` manages Haze instances, projects, and related resources from the terminal.

## Features

* Start and stop Haze instances
* List running Haze instances
* Inspect instance status
* Create and manage projects
* Interact with the Haze Core server
* Manage Haze-related resources from a single CLI

## Installation

Clone the repository:

```bash
git clone https://github.com/KiamMota/hazectl.git
cd hazectl
```

Build it with Go:

```bash
go build -o hazectl .
```

Then move the binary somewhere in your `PATH`:

```bash
sudo mv hazectl /usr/local/bin/
```

## Usage

List running Haze instances:

```bash
hazectl ps
```

Start a Haze instance:

```bash
hazectl start
```

Stop an instance:

```bash
hazectl stop <id>
```

Inspect an instance:

```bash
hazectl inspect <id>
```

Create a request:

```bash
hazectl request <ip:port> <function>[<params,>]
```

## Instances

Haze supports multiple Core instances running simultaneously.
The `instances` directory contains the runtime information required to identify and communicate with active Haze processes.

## Architecture

`hazectl` is a control client, not the Haze audio engine.

The audio engine remains inside Haze Core. `hazectl` communicates with Core through its control interface.
