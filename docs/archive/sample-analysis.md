# Analysis of the supplied Pololu sample repository

## Plain-language conclusion

The Pololu examples are useful starting points for reading sensors, following a
line, and controlling the motors. They are not a complete solution for this
competition. In particular, the supplied maze example cannot correctly map a
maze containing loops, does not safely handle every timeout, and does not save
the learned route through a reset. This project reuses the sound low-level
ideas and replaces the unsuitable maze logic.

Sources inspected:

- `C:\Users\Hang Minh Nguyen\source\repos\zumo-shield-master` (deprecated
  split-library release 1.2.3)
- `C:\Users\Hang Minh Nguyen\source\repos\zumo-shield-arduino-library-master`
  (consolidated release 2.1.0)

The newer supplied repository is byte-identical in the inspected core files to
the `ZumoShield` 2.1.0 PlatformIO dependency used by this project. Its line and
maze examples preserve the same basic algorithms as release 1.2.3. Release
2.1.0 consolidates the includes and adds `ZumoIMU`, which detects and abstracts
the sensor set used by original, v1.2, and v1.3 shields.

"Byte-identical" means the compared files contain exactly the same data. This
confirms that the PlatformIO dependency and the inspected local copy implement
the same library behavior for those files.

## Patterns adopted

These are ideas from the examples that fit our requirements:

- Calibrate by rotating the robot so every sensor sees both line and
  background. Validate all six calibrated spans before permitting a run.
- Treat the calibrated line position as 0..5000 with 2500 at center.
- Begin line following with a proportional-plus-derivative controller. Keep
  its gains configurable and clamp both outputs to the potentiometer-derived
  speed limit.
- Detect side branches with the outer sensors and straight travel with the
  inner sensors.
- Execute turns using sensor transitions and line reacquisition, with a
  timeout added to every loop.
- Move through a junction before final exit classification, but characterize
  that distance on the actual course rather than relying on the sample's
  battery-dependent timing constant.

## Patterns intentionally rejected

These example behaviors are unsafe or incompatible with the competition:

- The sample `MazeSolver` explicitly assumes a maze with no loops. Its
  left-hand rule and three-turn `L/R/S/B` reduction cannot satisfy the 2026
  loop requirement.
- Its path array has no bounds enforcement and is not persisted for a restart.
- Several sensor and turning loops can block forever; competition firmware
  requires timeouts that stop both motors.
- The example's finish detection is not sufficient as the final contract. Its
  comments describe a large black area covering all sensors, while its code
  checks the four middle sensors after a timed overshoot. The competition's
  final course definition must decide the accepted marker.
- Magnetometer-only absolute heading is avoided. The sample itself warns that
  motors, batteries, and environmental metal distort it; the v1.3 gyro is a
  better source for relative turns.

## Release 2.1.0 details adopted

These details describe how our program uses the current Pololu library:

- Include only `Wire.h` and `ZumoShield.h`; do not separately include bundled
  component headers that could collide with standalone libraries.
- Let `ZumoIMU::init()` detect the physical IMU revision and reject an
  unrecognized device before enabling autonomous motion.
- Use the library-selected reflectance emitter pin: D2 for ATmega328P/168 or
  A4 for ATmega32U4. The physical LEDON jumper must agree.
- Account for the reflectance calibration arrays' 48-byte dynamic allocation
  when all six sensors are calibrated with emitters on and off. Allocation is
  allowed only during initialization, never in the control loop.

## Terms used in this analysis

- **Gain:** a number that controls how strongly the steering calculation reacts.
- **Clamp:** restrict a number so it cannot exceed a safe minimum or maximum.
- **Line reacquisition:** detecting the new line under the center sensors after
  a turn.
- **Blocking loop:** code that can wait forever and prevent all other work.
- **Magnetometer:** an electronic compass that senses magnetic fields.
- **Dynamic allocation:** requesting working memory while the program runs.
  This is minimized because the Uno has only 2 KB of SRAM.

