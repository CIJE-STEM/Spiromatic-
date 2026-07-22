/*
 * ============================================================
 *  SPIROMATIC 2.0 — ALL SHAPES
 *  (two steppers + IR homing sensor)
 * ============================================================
 *
 *  WHAT CHANGED FROM THE SERVO VERSION?
 *
 *  The pen arm is now a STEPPER, not a servo. Why is that better?
 *    - A servo guesses its position from a little potentiometer.
 *    - A stepper MOVES IN EXACT COUNTED STEPS. If we know where
 *      it started, we know exactly where it is. Forever.
 *
 *  But that's a big IF. Where did it start? That's what the
 *  IR SENSOR is for: at power-up, the arm slowly sweeps until
 *  the sensor sees it. That spot is "step zero" — HOME.
 *  Every drawing starts from the same physical place, so
 *  drawings come out clean and repeatable.
 *  (This is exactly how 3D printers home with endstops!)
 *
 *  WIRING (per the CIJE Spiromatic Wiring Guide):
 *    Drawing platter stepper:  pins 9, 10, 11, 12
 *    Pen arm stepper:          pins 5, 6, 7, 8
 *    IR homing sensor:         D0 -> pin 3   (A0 -> A3, optional)
 *
 * ============================================================
 *  HOW TO USE: set the numbers in the two TUNE blocks below,
 *  upload, and watch. The machine homes itself, draws, then
 *  stops and releases the motors.
 * ============================================================
 */

// ============================================================
//  TUNE BLOCK 1 — WHAT TO DRAW
// ============================================================

// Pick ONE shape (write the name exactly as shown):
//   CIRCLE   — one round ring
//   SPIRAL   — grows outward the whole time
//   POLYGON  — straight sides! set 'sides' below
//              (3 = triangle, 4 = square, 6 = hexagon...)
//   FLOWER   — petals! set 'petals' below
#define SHAPE POLYGON

int sides  = 4;          // for POLYGON: how many sides
int petals = 5;          // for FLOWER: how many petals

// CONCENTRIC: draw the same shape nested inside itself.
// Example: SHAPE POLYGON, sides = 4, repeats = 5
//          = five squares, one inside the next.
// For SPIRAL, 'repeats' is simply how many loops it makes.
int repeats     = 5;     // how many nested copies (1 = just one)
int ringSpacing = 60;    // arm steps between copies. Bigger =
                         // rings farther apart. If the pen runs
                         // off the paper, make this smaller.

// ============================================================
//  TUNE BLOCK 2 — FIT THE DRAWING TO YOUR MACHINE
//  (calibrate once, then leave alone)
// ============================================================

int armStart = 100;      // arm steps from HOME to where the pen
                         // should start drawing (innermost ring).

int shapeSize = 80;      // arm steps from a shape's closest point
                         // to its farthest (corner/petal height).
                         // Bigger = pointier, more dramatic shape.

int armMax = 900;        // SAFETY LIMIT: the arm will never go
                         // more than this many steps from home.
                         // Find it: how many steps until the pen
                         // reaches the edge of the paper?

int stepDelay = 12;      // ms between platter steps. Slower = the
                         // arm keeps up better = crisper corners.

// Which way is "away from the sensor, toward the paper edge"?
// If your arm homes fine but then drives the WRONG WAY, flip
// this from +1 to -1.
int ARM_OUT = +1;

// Most IR obstacle modules output LOW when they see something.
// If homing never stops, flip LOW to HIGH.
#define IR_SEES LOW

// ============================================================
//  WIRING (matches the guide — don't change unless you rewired)
// ============================================================
const int P1 = 9,  P2 = 10, P3 = 11, P4 = 12;  // platter stepper
const int A1_ = 5, A2_ = 6, A3_ = 7, A4_ = 8;  // arm stepper
const int IR_PIN = 3;                          // IR sensor D0

const int STEPS_PER_TURN = 2048;   // 28BYJ-48 steps per revolution

// ------------------------------------------------------------
//  Machine state — the "we always know where we are" variables
// ------------------------------------------------------------
long platterCount = 0;   // total platter steps taken
long armPos = 0;         // arm steps from HOME (set to 0 by homing)
int  platterCoil = 0;    // which platter coil fires next
int  armCoil = 0;        // which arm coil fires next
bool done = false;

void setup() {
  pinMode(P1, OUTPUT);  pinMode(P2, OUTPUT);
  pinMode(P3, OUTPUT);  pinMode(P4, OUTPUT);
  pinMode(A1_, OUTPUT); pinMode(A2_, OUTPUT);
  pinMode(A3_, OUTPUT); pinMode(A4_, OUTPUT);
  pinMode(IR_PIN, INPUT);

  homeTheArm();                 // find "step zero"
  moveArmTo(targetFor(0.0));    // glide out to the starting radius
}

void loop() {
  if (done) return;             // finished — motors released

  platterStep();

  // Where should the pen be for THIS platter angle?
  float angle = (platterCount % STEPS_PER_TURN) * 360.0 / STEPS_PER_TURN;
  long target = targetFor(angle);

  // CLEAN SEAM TRICK: when we jump to the next concentric ring,
  // the arm has a long way to go. Instead of smearing that move
  // across several degrees of rotation, STOP the platter and let
  // the arm glide straight out. Every ring then connects with one
  // crisp radial line, all at the same angle — it looks intentional.
  if (labs(target - armPos) > 10) {
    moveArmTo(target);
  }

  // Normal drawing: nudge the arm toward its target (up to 3
  // steps per platter step — that's the arm's "speed limit").
  for (int i = 0; i < 3 && armPos != target; i++) {
    armStepToward(target);
    delayMicroseconds(1500);
  }

  delay(stepDelay);

  // A full drawing = 'repeats' platter revolutions. Then stop.
  if (platterCount >= (long)STEPS_PER_TURN * repeats) {
    finishDrawing();
  }
}

// ============================================================
//  THE SHAPES — each answers one question:
//  "at this platter angle, how far out is the pen, 0.0 to 1.0?"
//  (0.0 = shape's closest point, 1.0 = its farthest point)
// ============================================================

#define CIRCLE  1
#define SPIRAL  2
#define POLYGON 3
#define FLOWER  4

float howFarOut(float angle) {
#if SHAPE == CIRCLE
  return 0.0;   // constant radius — the ring offset does the rest

#elif SHAPE == SPIRAL
  // Grow steadily across the WHOLE drawing (all revolutions),
  // not just one turn. progress: 0.0 at start, 1.0 at the end.
  float progress = (float)platterCount / ((float)STEPS_PER_TURN * repeats);
  return progress;

#elif SHAPE == POLYGON
  {
    // Same trick as the square, for ANY number of sides:
    // corners every (360/sides) degrees.
    float slice = 360.0 / sides;
    float a = fmod(angle, slice) - slice / 2.0;   // -slice/2..+slice/2
    float dist = 1.0 / cos(a * DEG_TO_RAD);       // 1.0 at side middle
    float maxDist = 1.0 / cos((slice / 2.0) * DEG_TO_RAD);  // at corner
    return (dist - 1.0) / (maxDist - 1.0);        // normalized 0..1
  }

#elif SHAPE == FLOWER
  // One smooth cosine wave, 'petals' bumps per revolution.
  return (1.0 + cos(petals * angle * DEG_TO_RAD)) / 2.0;
#endif
}

// ------------------------------------------------------------
// targetFor(): turn "how far out, 0..1" into an ARM POSITION.
//
//   armStart            = where ring #1 begins
//   + ring * ringSpacing = which nested copy we're on
//   + howFarOut * shapeSize = the shape's in-out motion
//
// For SPIRAL the growth is inside howFarOut() instead, scaled
// across all the loops, so ring stays 0.
// ------------------------------------------------------------
long targetFor(float angle) {
  long ring = platterCount / STEPS_PER_TURN;   // 0, 1, 2, ...

#if SHAPE == SPIRAL
  long target = armStart + (long)(howFarOut(angle) * (float)ringSpacing * repeats);
#else
  long target = armStart + ring * (long)ringSpacing
              + (long)(howFarOut(angle) * shapeSize);
#endif

  // SAFETY: never past armMax, never behind home.
  if (target > armMax) target = armMax;
  if (target < 0) target = 0;
  return target;
}

// ============================================================
//  HOMING — the whole reason drawings are repeatable now
// ============================================================
void homeTheArm() {
  // If we're already sitting ON the sensor, back off it first.
  // (Safety net on both loops: give up after 3 full motor turns,
  // so a miswired sensor can't make the arm grind forever.)
  long tries = 0;
  while (digitalRead(IR_PIN) == IR_SEES && tries < 3L * STEPS_PER_TURN) {
    armStepOut(+1);
    delay(3);
    tries++;
  }
  delay(200);

  // Now creep toward the sensor until it sees the arm.
  tries = 0;
  while (digitalRead(IR_PIN) != IR_SEES && tries < 3L * STEPS_PER_TURN) {
    armStepOut(-1);
    delay(3);
    tries++;
  }

  armPos = 0;   // THIS is the magic line. We now KNOW where we are.
  delay(300);
}

// ------------------------------------------------------------
//  Motor plumbing — same wave drive as always, times two.
// ------------------------------------------------------------
void platterStep() {
  platterCoil = (platterCoil + 1) % 4;
  digitalWrite(P1, platterCoil == 0);
  digitalWrite(P2, platterCoil == 1);
  digitalWrite(P3, platterCoil == 2);
  digitalWrite(P4, platterCoil == 3);
  platterCount++;
}

// Move the arm ONE step. dir = +1 means "outward" (away from
// the sensor), -1 means "inward" (toward the sensor).
// ARM_OUT translates that into whichever way the coils must
// actually spin on YOUR machine — the gearing may flip it.
void armStepOut(int dir) {
  int coilDir = dir * ARM_OUT;
  armCoil = armCoil + coilDir;
  if (armCoil > 3) armCoil = 0;
  if (armCoil < 0) armCoil = 3;
  digitalWrite(A1_, armCoil == 0);
  digitalWrite(A2_, armCoil == 1);
  digitalWrite(A3_, armCoil == 2);
  digitalWrite(A4_, armCoil == 3);
  armPos += dir;   // position is always counted in "outward" units
}

// One step toward a target position.
void armStepToward(long target) {
  if (target > armPos) armStepOut(+1);
  else if (target < armPos) armStepOut(-1);
}

// Glide the arm to a position (used once, after homing).
void moveArmTo(long target) {
  while (armPos != target) {
    armStepToward(target);
    delay(3);
  }
}

// Release both motors so nothing overheats while parked.
void finishDrawing() {
  digitalWrite(P1, LOW); digitalWrite(P2, LOW);
  digitalWrite(P3, LOW); digitalWrite(P4, LOW);
  digitalWrite(A1_, LOW); digitalWrite(A2_, LOW);
  digitalWrite(A3_, LOW); digitalWrite(A4_, LOW);
  done = true;
}

/*
 *  CLASS DISCUSSION:
 *  - Why does the stepper arm make cleaner drawings than the
 *    servo arm did? (Counted steps vs. analog guessing; and no
 *    whole-degree quantization — the staircase is gone.)
 *  - Why home at all? Unplug the machine, turn the arm by hand,
 *    plug it back in — how does the code find out where the arm
 *    is? It can't... unless it goes and LOOKS. That's homing.
 *  - Draw the same POLYGON twice without unplugging. Then power
 *    cycle and draw it again. All three match — why?
 *  - Try: FLOWER with petals = 7, repeats = 4. Predict what it
 *    will look like BEFORE you run it.
 */
