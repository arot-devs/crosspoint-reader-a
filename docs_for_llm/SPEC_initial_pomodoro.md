Below is a **text-first design sheet** for a **60-minute max Pomodoro** on a **4.3" e-ink (220 ppi)** device with **4 buttons: Up / Down / Select / Exit**. It’s essentially a faithful port of the square “Time Timer” feel: **a circular dial inside a square frame**, with a **filled wedge** representing remaining time.

------

## Product intent

A single-purpose timer that feels like a physical kitchen timer:

- **Set duration (5–60 min, step = 5)**
- **Start / Pause / Resume**
- **Clear “what can I do now?” hints on-screen**
- E-ink-friendly: minimal updates, high legibility, low confusion
- Completion signal is **visual**: **full-screen invert flash 5 cycles**

------

# UI Spec Sheet (visual + layout)

## Display assumptions

- **Monochrome-first** (works in pure black/white). If grayscale exists, use it only to soften fills.
- Prefer **high-contrast, thick strokes**: e-ink benefits from bold shapes and generous spacing.

## Overall layout

Portrait, with three zones:

1. **Header (thin)**

- Center: `POMODORO`
- Optional top-right: small battery icon (if available)

1. **Primary dial area (dominant)**

- Square container with rounded corners (echo the physical timer body)
- Circular dial centered inside

1. **Footer (controls + status)**

- One **status line** (contextual hints / confirmations)
- One **button legend line** (always present, state-dependent)

### Wireframe (conceptual)

```
┌──────────────────────────┐
│        POMODORO       🔋  │
│                          │
│   ┌──────────────────┐   │
│   │     ◜█████        │   │  ← wedge (remaining)
│   │   55  0   5       │   │  ← numerals every 5
│   │  50   ●    10     │   │  ← center dot + pointer
│   │   45      15      │   │
│   │     40 35 30 25   │   │
│   └──────────────────┘   │
│                          │
│ Status: Ready (25 min)   │
│ ▲ +5   ▼ -5   SEL Start  │
│ EXIT×2 Reset   HOLD Quit │
└──────────────────────────┘
```

## Dial design (port of Image 1)

### Components

- **Outer minute ticks**: 60 ticks
  - Major tick every 5 minutes (thicker/longer)
  - Minor ticks thin/short
- **Numerals**: `0, 5, 10, … 55` around the dial (like the physical timer)
- **Center hub**: small filled circle (the “mechanical” anchor)
- **Pointer**: a short triangular pointer (or a notch) indicating “current remaining boundary” at the wedge edge

### Wedge (the key affordance)

- Represents **remaining time** (not elapsed)
- Starts at “0” (top) and extends clockwise for the amount remaining
- **Fill style:**
  - If only black/white: use a **dither pattern** (checker/ordered) for wedge so ticks remain visible
  - If grayscale available: wedge in light gray, ticks/numerals in black
- Wedge should be visually dominant but not swallow the numerals:
  - Keep wedge in a **ring band** (outer 25–30% of radius), leaving center clean for text

## Center readout (clarity without constant updates)

E-ink doesn’t love per-second ticking. The physical timer doesn’t show seconds either. So:

- Idle: show **set duration** (e.g., `25 min`)
- Running: show **remaining minutes** large (e.g., `17 min`)
- Optional: when remaining < 1 minute, switch to **seconds-only** (e.g., `45 s`) and update more frequently

Recommended center typography:

- Big number (primary): `25`
- Small label beneath: `min`
- When paused: overlay `PAUSED` (all caps) above the number, or replace label with `PAUSED`

## Footer: make interactions self-explanatory

Two lines:

1. **Status line** (dynamic microcopy)

- Examples:
  - `Ready (25 min)`
  - `Running`
  - `Paused`
  - `Press EXIT again to reset`
  - `Reset`
  - `Quit? SEL=Yes EXIT=No`

1. **Button legend** (dynamic mapping by state)

- Always show all four actions, even if some are disabled (but mark disabled with “—” or a note like “Pause to adjust”)

------

# Interaction Spec Sheet (state machine + button behavior)

## Button gestures (definitions)

- **Tap**: < 0.5s
- **Long press**: ≥ 1.2s (Exit only)
- **Double press**: two taps within 0.4s (Exit only)

(Those thresholds are conservative and reduce accidental triggers.)

## States

1. **IDLE / SET**
2. **RUNNING**
3. **PAUSED**
4. **DONE (after completion flash)**
5. **CONFIRM RESET** (lightweight, time-limited)
6. **CONFIRM QUIT** (optional but recommended)

------

## State: IDLE / SET

**Purpose:** choose duration, then start.

**Screen**

- Dial wedge shows the chosen duration (like setting a physical timer)
- Status: `Ready (25 min)` (or whatever value)

**Buttons**

- **Up**: +5 minutes (clamp at 60)
- **Down**: -5 minutes (clamp at 5)
- **Select**: Start
- **Exit**
  - Double press: Reset to default duration (recommend default = 25)
  - Long press: Quit app

**Helpful feedback**

- If user tries past limits:
  - Status: `Max 60 min` / `Min 5 min` for ~1.5s

------

## State: RUNNING

**Purpose:** countdown with minimal distraction.

**Screen**

- Wedge shrinks as time decreases
- Center shows remaining minutes (and optionally seconds in final minute)
- Status: `Running`

**Buttons**

- **Select (tap)**: Pause
- **Up/Down**: *No change* (prevents accidental time edits)
  - Status: `Pause to adjust` (brief)
- **Exit (double press)**: Reset flow (see below)
- **Exit (long press)**: Quit flow (see below)

------

## State: PAUSED

**Purpose:** stop time; allow adjust; resume.

**Screen**

- Wedge and number frozen
- Status: `Paused`

**Buttons**

- **Select (tap)**: Resume
- **Up/Down**: adjust remaining time ±5 (clamp 5–60)
- **Exit (double press)**: Reset to IDLE
- **Exit (long press)**: Quit

------

## State: DONE (completion)

**Completion signal first**

- Immediately on reaching 0:
  - **Flash full-screen**: alternate **white ↔ black** for **5 cycles**
  - Allow **any button press** to interrupt the flashing early (important for sanity)

**After flash**
**Screen**

- Big text: `DONE`
- Secondary: `Press SEL for new timer`
- Dial resets to last used duration (or default—see recommendation below)

**Buttons**

- **Select**: Start a new session (uses currently displayed duration)
- **Up/Down**: adjust duration for next session
- **Exit (double press)**: reset to default duration
- **Exit (long press)**: quit

------

## Reset flow (Exit double press)

To avoid surprise resets mid-focus, use a lightweight confirmation:

**First Exit tap (while Running/Paused)**

- Status: `Press EXIT again to reset`
- A 2-second window starts

**Second Exit tap within window**

- Reset: go to IDLE/SET
- Status: `Reset`

(If the window expires, do nothing and clear the hint.)

------

## Quit flow (Exit long press)

Long press already reduces accidents; still, e-ink UIs benefit from explicitness.

**Option A (simplest): immediate quit**

- On long press: quit to launcher
- Status briefly: `Quitting (timer reset)`

**Option B (recommended): confirm quit**

- On long press: show confirm screen:
  - `Quit? Timer will reset`
  - `SEL=Yes EXIT=No`
- Timeout after 5 seconds returns to previous state

------

# Walkthroughs (common user journeys)

## 1) Set 25 minutes and start

- IDLE shows `Ready (25 min)`
- (Optional) Up/Down to adjust
- Press **Select** → RUNNING
- Status changes to `Running`

## 2) Pause and resume

- While RUNNING press **Select** → PAUSED (`Paused`)
- Press **Select** again → RUNNING (`Running`)

## 3) Adjust while paused

- PAUSED: press Up/Down to modify time in ±5 min steps
- Press Select to resume

## 4) Reset mid-session

- RUNNING: press Exit once → `Press EXIT again offer reset`
- Press Exit again within 2 seconds → IDLE (`Reset`)

## 5) Timer completes

- At 0: full-screen invert flashing 5 cycles
- Then DONE screen:
  - `DONE`
  - `SEL=Start` (new session)

------

# Edge handling + “don’t confuse the user” rules

## Power off during a session

You proposed: **reset timer**. Do that—but make it emotionally legible.

**Behavior**

- On restart/app launch: always land in IDLE/SET (no auto-resume)

**Screen message**

- Status line for first 3 seconds:
  - `Previous session ended (power loss)`
- Then: `Ready (25 min)` (or last-set)

## Device sleep

You proposed: **keep it awake**.

**Behavior**

- While RUNNING: acquire a wake lock / disable auto-sleep
- If the OS *still* sleeps (battery-critical, user forces sleep):
  - On wake: show PAUSED with message:
    - `Paused due to sleep`
  - (Reason: if the user thinks time ran while asleep, trust breaks; pausing is safer unless you’re confident in RTC continuity.)

## User exits app and returns

You proposed: **reset timer**.

**Behavior**

- Returning to app always goes to IDLE/SET
- Do not restore a partially elapsed session

**Screen message**

- Status (brief):
  - `Session reset`

## Low battery (if detectable)

**Behavior**

- If battery is critically low while RUNNING:
  - Pause automatically
  - Status: `Low battery — timer paused`

## Accidental presses / “what just happened?”

Use the status line as the single source of truth:

- Any blocked action should say why:
  - `Pause to adjust`
- Any destructive action should confirm or at least be two-step:
  - reset = double-press with prompt
  - quit = long press + optional confirm

------

# E-ink rendering + refresh policy (practical spec)

To keep ghosting low and battery sane:

- **IDLE adjustments:** partial refresh of dial region on each ±5 change
- **RUNNING:** update display **once per minute**
  - Update wedge boundary + center number
- **Final minute (optional):** switch to seconds-only and update every 5s or 1s (your choice; 1s is clearer but heavier)
- **Force full refresh**
  - On Start
  - On Pause/Resume
  - On Done flash completion (already full-screen)
  - Every ~5 minutes during long runs (optional) to clear ghosting

------

# Recommended defaults (small but important)

- Default duration: **25 min**
- After DONE: keep the **last used duration** (it supports repeated sessions)
- After Reset (Exit×2):
  - Either: return to **last used** (fast)
  - Or: return to **default 25** (predictable)
    I’d choose: **last used** + a clear `Reset` status message, because “reset” means “stop and return to ready,” not “forget my preference.”

------

- 