# Aim.exe

A 2D turn-based archery game. You play the blue archer on the left facing a computer, which plays the red archer on the right. Take turns firing arrows that arc under gravity and wind. The last one standing wins!

## How to play

From the title screen, choose **Play**, then pick an arena:

- **Forest**: a hill in the middle, Earth gravity, light wind
- **Windy Mesa**: a cliff with low ground on the left and a plateau on the right, Earth gravity, strong wind
- **Moon**: a crater, low gravity, no wind

On your turn you have 45 seconds.

- **Click, drag, then release** to shoot an arrow, advancing a turn.
- **Hold the down key** to zoom out and see the whole arena. You can only aim while zoomed in.
- **1**, **2**, and **3** switch arrow types:
  - **1 Standard**: default arrow
  - **2 Heavy**: slower and shorter, but hits harder
  - **3 Multishot**:  three lighter arrows spread a few degrees apart

Crates sometimes appear on the field. Shoot one until it breaks to recover up to 30 HP.

## Build and run

This game compiles to WebAssembly with [Emscripten](https://emscripten.org/). Install the SDK once, then activate it in the shell you'll build from:

```sh
git clone https://github.com/emscripten-core/emsdk.git
./emsdk/emsdk install latest
./emsdk/emsdk activate latest
source ./emsdk/emsdk_env.sh
```

Then, from the game directory:

```sh
make game
```

You can now open [http://localhost:8000/bin/game.html](http://localhost:8000/bin/game.html) to play the game!
