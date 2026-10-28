# Documentation

| Read this | When |
|---|---|
| [team-handout.md](team-handout.md) | Start here. How the robot works, competition rules, running it, troubleshooting, and answers for judges. Written for teammates who do not code. |
| [known-good-config.md](known-good-config.md) | The exact versions and settings that completed the rounds on 2026-10-23, with the run logs in [runs/](runs/), and how to restore them. Start every speed-up from here. |
| [hardware-pin-map.md](hardware-pin-map.md) | Which Arduino pin does what, including the speed knob on A1. |
| [tuning-log.md](tuning-log.md) | Dated record of every test, measurement and change: our evidence of systematic calibration, debugging and improvement. |
| [archive/](archive/) | Documents for our first firmware (gyro turns and a grid map). Kept as testing history; they do not describe the competition program. That program, its tests, bench tests, scripts and simulator were removed by 2026-10-24 and remain in git history. |

## The program

The repository holds one program: `src/pololu_maze_firmware.cpp`, build
`uno_pololu_maze`, which is Pololu's MazeSolver example with our additions.
Upload it with `scripts\run-competition-firmware.ps1 -Port COM4`.
