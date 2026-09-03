# Haze Paths

The Haze application stores its data inside a dedicated `Haze` directory located in the user's home directory. This directory contains Haze projects, imported samples, configuration files, cache data, and application logs.

## Root

| Platform | Path |
|---|---|
| Windows | `%USERPROFILE%\Haze` |
| Linux | `$HOME/Haze` |
| macOS | `$HOME/Haze` |
| BSD | `$HOME/Haze` |

## Structure

```text
Haze/
├── projects/
├── samples/
├── cache/
├── config/
├── community/
└── logs/
