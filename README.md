# CS 2300 Module 4 Applied Project: Lights Out

Hey! Who turned out the lights?

For this project, you will create a C++ graphics program that allows the user to play the game Lights Out.

## Lights Out
Lights Out was a handheld game in the days of Lisa's childhood. You can play it online here: [https://www.logicgamesonline.com/lightsout/](https://www.logicgamesonline.com/lightsout/).

### How The Game Works
In its simplest form, the 5x5 grid of lights begins with all of the lights lit. When you click on one of the lights, it toggles itself and the (up to) four lights it borders.

Here's a gif of the beginning of the game, where the lights have a red outline hover effect: 

![Lights-Out-Game-Start.gif](Lights-Out-Game-Start.gif)

When you make all the lights go off, you win the game and can no longer click on the lights:

![Lights-Out-Game-End.gif](Lights-Out-Game-End.gif)

### Testing 
Here are the lights you need to click to get from a fully lit start to a fully unlit end:

| | | |*|*|
|-|-|-|-|-|
|*|*| |*|*|
|*|*|*| | |
| |*|*|*| |
|*| |*|*| |
