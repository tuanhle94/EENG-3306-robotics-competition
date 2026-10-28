# Teammate operator guide

Status: competition procedure, updated 2026-10-21. One complete two-round run
has passed on the practice board.

The robot drives itself. In round one it explores the maze until it finds the
black finish dot and remembers the shortest path it found. In round two it
drives that path straight to the dot. Operators never need a laptop or the code.

## The three things you touch

| Control | What it does |
|---|---|
| Power switch | ON starts the robot up. Keep it still for about 2 seconds while it measures itself. |
| Start button | The small user pushbutton, **not** RESET. A quick press starts; holding 10 s erases memory. |
| Status light | The light that blinks during the 3-second countdown. |

## What the status light means

| Light | Meaning |
|---|---|
| Solid on, robot still | Ready. A quick press starts. |
| Flickering while you hold the button | Keep holding to 10 s to erase memory, or let go to cancel. |
| Blinking about twice a second | 3-second countdown. Hands off. |
| Flickering fast, robot twisting left and right | Measuring this maze's black and white after the countdown (about 2 s). Normal. |
| Robot stopped on the dot, light on | Round finished. |
| Robot stopped elsewhere, light off | Safety stop. |

## Five rules

- Use exactly the same START spot and direction for both rounds. Photograph it
  before round one.
- Press once, quickly, then hands off. Touching it during the countdown makes
  it refuse to start.
- Never hold the button between rounds. Holding it 10 seconds erases the
  saved path; only do that before round one.
- No USB cable during runs.
- Don't touch the robot during a run unless a judge allows it.

## Before round one: clear the old path

There is no calibration step. After every countdown the robot twists about
50 degrees each way over the START line to measure black and white, then
turns back and drives.

1. Fit fresh batteries if available. Switch ON and wait for a solid light.
2. Hold the button for 10 seconds. After about 2 seconds the light flickers;
   keep holding.
3. Three slow blinks, then a solid light, means everything is cleared. Let go.
4. Switch OFF.

Letting go before 10 seconds erases nothing. A press longer than 2 seconds
does not start the robot either.

## Round one

1. Robot OFF. Place it at START facing into the maze, with the line under the
   middle of the front edge, white on both sides, not on a crossing.
2. Photograph the placement.
3. Switch ON without touching the button. Wait for a solid light.
4. On the judge's signal press once and take your hands away. It blinks for
   3 seconds, twists left and right for about 2 seconds, then drives.
5. It explores, including dead ends, and stops on the finish dot with the light
   on. The path is saved.

## Between rounds

Switch OFF. Don't hold the button; if the light flickers because you are
holding it, let go, since nothing is erased before 10 seconds. The path
survives power off and battery changes.

## Round two

1. Robot OFF. Place it at the same START spot and direction as round one.
2. Switch ON without touching the button. Wait for a solid light.
3. On the judge's signal press once and take your hands away.
4. It drives straight to the dot and stops with the light on.

## If something goes wrong

To stop it immediately, press the button while it drives or switch OFF.

| What you see | What to do |
|---|---|
| After the countdown (and maybe the twist), solid light, no driving | Start refused: line not straight and centered under the front, or robot touched. Re-center on a straight part of the line and press again. |
| Refuses every start | Use a straight stretch with plain white on both sides, a hand's width from other tape. Try fresh batteries. |
| Stopped in the maze, light off | Safety stop. If the judge allows a retry: OFF, back to START, ON (no button), wait for solid light, press once. The same round restarts. |
| Pressed, nothing happened, light still solid | The press was over 2 seconds. Press again, quickly. |
| Light off after switching on, button does nothing | Switch OFF and ON. If still off, it is a hardware problem; tell the judge. |
| Round two stops early, light off | Usually a different start spot or direction. Re-place exactly from the photo and retry. The saved path is still there. |

Film every run. The robot also remembers its last run until the next one
starts; after a bad run, don't start it again until the code owner has read
it over USB, if you can wait.

## For the code owner before handover

- Upload the competition firmware from branch `competition-two-round-run`
  (plain upload, no inspection option).
- Run both rounds once without USB.
- Show the team the start button and status light; mark each with tape.
- Pack fresh AA batteries and a printed copy of this guide.
