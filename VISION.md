# PROJECT VISION DOCUMENT

## Working Title: **Pong: Terminal Velocity**

A minimal, source-available noncommercial, low-resolution Pong variant with continuous paddle aiming, persistent spin physics, speed escalation, and a symbolic “demon AI” trained via self-play.

The goal is not nostalgia.
The goal is to create a deterministic mechanical opponent that becomes progressively inhuman.

---

# 1. Core Design Philosophy

* Low resolution.
* No art polish.
* Deterministic simulation.
* Physics > UI.
* AI is symbolic, not ML.
* Fully reproducible matches.
* Input-flexible (keyboard / mouse / controller / custom hardware).

The project is about *mechanics and AI purity*, not visuals.

---

# 2. Game Ruleset

## 2.1 Court

* Fixed rectangular court.
* Top/bottom bounce.
* Left/right = score.

Low resolution recommendation:

* 640×360 or 800×450 logical resolution.
* Integer scaling for fullscreen.

---

## 2.2 Ball Physics

### State:

* position (x, y)
* velocity (vx, vy)
* spin (scalar)
* speed scalar

### Motion:

* Position integrates continuously.
* Speed is mostly constant except ramp.
* Spin continuously modifies vertical velocity:

  vy += spin * k_curve * dt

### Spin:

* Injected at paddle contact:

  spin += k_take * paddle_velocity
  spin = clamp(spin, -spin_max, spin_max)

* Decay each frame:

  spin *= exp(-dt / tau_spin)

* Optional small bleed on wall bounce:

  spin *= wall_spin_retention

Spin always exists unless actively neutralized.

---

## 2.3 Paddle Physics

### Movement:

* Finite max speed.
* Finite acceleration.
* No teleport.
* Designed for ~90% coverage under typical rally speed.

### Contact:

* Continuous offset mapping:

  u = (ball_y - paddle_center_y) / paddle_half_height
  clamp u ∈ [-1,1]

* Outgoing angle:

  theta = u * theta_max

* Ball velocity re-normalized to current speed.

No discrete zones.

---

## 2.4 Speed Escalation (Reflex Hell)

Each paddle hit:

speed *= (1 + ramp_rate)

Optional ramp tiers:

* Early: slow ramp.
* After N hits: higher ramp.

No hard cap (optional config flag).

Goal:

* Midgame = strategic trap building.
* Late game = mechanical breakdown.
* Eventually even AI loses.

---

# 3. AI Design

## 3.1 Philosophy

The AI is not reflex-based.
It predicts.

It uses forward simulation and minimax-style evaluation.

No neural networks.

---

## 3.2 Defensive Layer

* Predict intercept time and location.
* Solve reachable set under paddle acceleration limits.
* Move optimally toward intercept.

---

## 3.3 Offensive Layer (Demon Brain)

At paddle contact:

1. Generate candidate contact offsets (continuous sampling).
2. Generate candidate paddle flick velocities (spin injection).
3. For each candidate:

   * Simulate forward.
   * Assume opponent plays optimally.
   * Evaluate outcome.

Score function should minimize opponent margin:

* Required travel distance.
* Required acceleration.
* Intercept timing.
* Precision demand.
* Future trap value (2-ply).

AI chooses the candidate maximizing opponent difficulty.

---

## 3.4 Self-Improvement Mode

Optional executable mode:

* CPU vs CPU tournament.
* Log rally outcomes.
* Detect failure patterns.
* Adjust evaluation weights manually or via scripted parameter search.

No random reinforcement learning required.
This is deterministic parameter refinement.

---

# 4. Input System

## 4.1 Supported Control Modes

Each paddle independently configurable:

* Keyboard (W/S or Arrow keys)
* Mouse (vertical position mapped to paddle)
* Gamepad (analog stick Y)
* Custom serial device (Arduino knob / potentiometer)
* AI

---

## 4.2 Serial Controller Support

Optional:

* Listen on serial port.
* Expect simple normalized float (0.0–1.0).
* Map directly to paddle target.

This allows:

* Physical spin knobs.
* DIY arcade builds.
* Custom controllers.

---

# 5. Simulation Architecture

## 5.1 Deterministic Core

* Fixed timestep (e.g., 240 Hz).
* Render at 60 Hz.
* Use swept collision or substepping to avoid tunneling.

Single source of truth simulation.
AI uses same step function as gameplay.

---

## 5.2 Module Layout

```
/sim
    physics.cpp
    collision.cpp
    spin.cpp

/ai
    intercept.cpp
    evaluator.cpp
    planner.cpp

/input
    keyboard.cpp
    mouse.cpp
    gamepad.cpp
    serial.cpp

/app
    main.cpp
    renderer.cpp
```

---

# 6. Game Modes

1. Human vs Human
2. Human vs AI
3. AI vs AI
4. Training Mode (accelerated sim, no rendering)

---

# 7. Configuration File

Allow tweaking:

* paddle_max_speed
* paddle_accel
* theta_max
* spin_max
* k_take
* k_curve
* tau_spin
* ramp_rate
* ramp_threshold
* wall_spin_retention

Expose via simple JSON or TOML.

---

# 8. Visual Style

* Solid background.
* Simple paddles.
* Glowing ball.
* Minimal HUD:

  * Score
  * Speed multiplier
  * Spin magnitude

No particle effects.
No unnecessary assets.

---

# 9. Success Criteria

Project is successful when:

* Human players struggle.
* AI vs AI rallies can escalate to absurd speeds.
* The AI clearly sets traps.
* Spin manipulation is visible and meaningful.
* Hardware knob control feels natural.
* Codebase remains under ~3–5k LOC core.

---

# Final Intent

This is not nostalgia Pong.

This is:

> A mechanical duel simulator that starts simple and escalates into computational violence.
