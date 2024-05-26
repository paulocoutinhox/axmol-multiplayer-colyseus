# Axmol Multiplayer

This is a project that shows how to create a simple multiplayer game with Colyseus library.

Web demo: https://axmp.netlify.app/

## Files To Add

These are the new files that you need:

- /ColyseusLib
- /ColyseusServer

## Code Changes

There are some small changes that you need do to your project to include this.

### CMake root file: CMakeLists.txt

1 - Add Colyseus directory:

```cmake
add_subdirectory(ColyseusLib)
```

## Game Server

The game server is inside `ColyseusServer` folder.

To start the server execute the following commands:

```bash
cd ColyseusServer
npm install
npm run dev
```

or simply:

```bash
make start-colyseus
```
