/*
 * ============================================================
 *  SPIROMATIC — Draw a SQUARE
 *  (stepper platter + servo arm)
 * ============================================================
 *
 *  WAIT — how do you draw a SQUARE on a SPINNING piece of paper?
 *
 *  Think about it: if the pen just stays still while the paper
 *  spins, you get a CIRCLE. Circles are free on this machine.
 *
 *  To get a square, picture standing at the center of the square
 *  and turning around while pointing at its edge:
 *    - Pointing at the MIDDLE of a side  --> the edge is CLOSE
 *    - Pointing at a CORNER              --> the edge is FAR
 *      (about 1.4 times farther, to be exact)
 *
 *  So while the platter spins, the arm must slide the pen
 *  out-in-out-in, four times per turn: OUT at every corner,
 *  IN at the middle of every side. That perfectly-timed
 *  in-and-out motion is what this code does.
 *
 *  HONEST WARNING: your first square will look like a blobby
 *  square with rounded corners. That's normal! Real engineers
 *  calibrate. Use the two TUNE numbers to sharpen it up.
 */

#include <Servo.h>

// ---------- WIRING (same as the spiral sketch) ----------
const int IN1 = 8;         // stepper driver IN1 --> pin 8
const int IN2 = 9;         // stepper driver IN2 --> pin 9
const int IN3 = 10;        // stepper driver IN3 --> pin 10
const int IN4 = 11;        // stepper driver IN4 --> pin 11
const int SERVO_PIN = 6;   // servo signal (orange) --> pin 6

// Our stepper takes this many steps to spin the platter
// around exactly once. (That's the 28BYJ-48 motor's gearing.)
const int STEPS_PER_TURN = 2048;

// ============================================================
//  TUNE THESE — the two numbers that shape your square
// ============================================================
int stepDelay = 12;      // ms between platter steps.
                         // For squares, go SLOW (10–20) — the arm
                         // needs time to push out to each corner.
                         // Too fast = corners get cut off!

int sideAngle = 90;      // Servo angle when the pen is at its
                         // CLOSEST point (the middle of a side).
                         // Find it: what angle puts the pen about
                         // halfway between center and paper edge?

int cornerPush = 25;     // How many EXTRA degrees the servo turns
                         // to reach a corner. Bigger = pointier,
                         // bigger square. Start ~25 and adjust.
                         // Too big and the pen flies off the paper!
// ============================================================

Servo arm;
int coilStep = 0;        // which stepper coil fires next (0–3)
long stepCount = 0;      // total platter steps taken — this is how
                         // we always know the paper's exact angle.
                         // (This is why we HOME things in robotics:
                         // if you know where you started and count
                         // every step, you know where you are!)

void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  arm.attach(SERVO_PIN);
  arm.write(sideAngle + cornerPush);  // start at a corner
  delay(500);                         // let the servo get there
}

void loop() {
  platterStep();      // platter turns one tiny step...
  aimAtSquare();      // ...arm moves pen to the right distance
  delay(stepDelay);
}

// ------------------------------------------------------------
// platterStep(): fire the next coil in the 1-2-3-4 sequence.
// Same as always — this is what spins the platter.
// ------------------------------------------------------------
void platterStep() {
  coilStep = coilStep + 1;
  if (coilStep > 3) coilStep = 0;

  digitalWrite(IN1, coilStep == 0);
  digitalWrite(IN2, coilStep == 1);
  digitalWrite(IN3, coilStep == 2);
  digitalWrite(IN4, coilStep == 3);

  stepCount = stepCount + 1;   // keep count so we know our angle
}

// ------------------------------------------------------------
// aimAtSquare(): the math that makes a square. Three moves:
//
//  1. Turn our step count into the platter's angle (0–360).
//  2. Figure out how far we are from the nearest corner.
//     Corners happen every 90 degrees, so we only ever need
//     to think about one 90-degree slice at a time.
//  3. Use that to slide the pen: near a corner = push OUT,
//     middle of a side = pull IN.
// ------------------------------------------------------------
void aimAtSquare() {
  // 1. Platter angle right now, in degrees (0 to 360).
  //    The % symbol means "remainder" — it makes the count
  //    wrap back to 0 after every full turn.
  float platterAngle = (stepCount % STEPS_PER_TURN) * 360.0 / STEPS_PER_TURN;

  // 2. Where are we within the current 90-degree slice?
  //    This gives a number from -45 to +45:
  //       0   = middle of a side (pen closest)
  //      -45 or +45 = at a corner (pen farthest)
  float sliceAngle = fmod(platterAngle, 90.0) - 45.0;

  // 3. The geometry rule for a square (from trigonometry):
  //    distance = closest distance / cos(angle from side middle)
  //    cos() needs radians, so we convert with DEG_TO_RAD.
  float distance = 1.0 / cos(sliceAngle * DEG_TO_RAD);
  //    distance is 1.0 at a side middle, about 1.414 at a corner.

  // Turn that distance into a servo angle:
  //    howFarOut = 0.0 at a side middle, 1.0 at a corner
  float howFarOut = (distance - 1.0) / 0.414;
  float servoAngle = sideAngle + howFarOut * cornerPush;

  arm.write((int)servoAngle);
}

/*
 *  CLASS DISCUSSION, once it works (or almost works):
 *
 *  - Why are circles free on this machine but squares hard?
 *    (The machine speaks "spin language" — polar coordinates.
 *     A printer speaks "left-right, up-down" — for a printer,
 *     squares are free and circles are hard!)
 *
 *  - Why are the corners rounded? (The servo can only move so
 *    fast, and it can only move in whole degrees.)
 *
 *  - Challenge: change ONE number in aimAtSquare() to draw a
 *    TRIANGLE instead. Hint: corners every 90 degrees made a
 *    square. How often do a triangle's corners come around?
 */
