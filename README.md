# Hanoi tower game

This is a computer graphics project from the [SUPSI](supsi.ch) bachelor of computer Science, featuring a fully independent graphic engine and a game client.

![til](./doc/towerhanoi.gif)

## Graphic engine

The engine is built on top of `opengl` & `freeglut` capable of the following features: 
- scene load from `*.ovo` files
- scene nodes manipulations
- lights management
- cameras management
- single and multiple animations

## Hanoi tower client

This is the client that build the game, the following actions are given:

### camera movements

- press `w` to move the active camera forward
- press `a` to rotate the active camera leftward
- press `s` to rotate the active camera backward
- press `d` to move the active camera rightward

- press `q` to rotate the active camera downward
- press `e` to rotate the active camera upward
- press `SHIFT` to move the active camera upward
- press `CTRL` to move the active camera downward

- press `r` to make a demo rotation of 360 degree around the tower
- press `m` to make a demo movement of 10 units foreward
- press `f` to change active camera and switch between perspective and orthographic


### game movements

#### Hanoi light 
- press `right_arrow` & `left_arrow` to move the light between sticks

#### Selected disk movements
- press `up_arrow` to select a disk
- press `right_arrow` to move the disk right
- press `left_arrow` to move disk to the left stick
- press `down_arrow` to move disk to the right stick

### memento pattern
- press `i` one or multiple times to undo movements
- press `o` one or multiple times to redo movements
- press `l` to reset movements

### off-topic
- press `k` to enable/disable wire-frame mode

## Theme

The scene theme is inspired by the timeless [What is the Matrix? scene](https://www.youtube.com/watch?v=O5b0ZxUWNf0) 
## Usage

### Windows

`cd ./demo`
`./client.exe`

### Linux

`chmod +x ./run.sh`
`./run.sh`

## Authors and acknowledgment

- [Pietro Brusadelli](https://github.com/PietroBrusa)
- [Giona Valsecchi](https://github.com/GionaValse)
- [Filippo De Simoni](https://github.com/mrdeshi)
