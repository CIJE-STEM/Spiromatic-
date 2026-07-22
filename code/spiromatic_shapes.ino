/*
 * ============================================================
 *  SPIROMATIC — SHAPE RECIPES
 *  One sketch, six shapes. Change ONE number to switch.
 *  (stepper platter + servo arm, same wiring as always)
 * ============================================================
 *
 *  HOW EVERY SHAPE WORKS:
 *  The machine only knows two things — how far the paper has
 *  turned, and how far out the pen is. So every shape here is
 *  just a rule: "when the paper is at THIS angle, put the pen
 *  at THAT distance." Each recipe below is one of those rules.
 *
 *  DIFFICULTY LADDER (try them in order):
 *    1 = flower     easiest, smooth and forgiving
 *    2 = heart      one-line change from the flower
 *    3 = star       sharp moves — go slow!
 *    4 = ellipse    the actual equation of a planet's orbit
 *    5 = mandala    20 turns of drifting petals — the showstopper
 *    6 = spiral     any shape, growing outward each turn
 */

#include <Servo.h>

// ---------- WIRING (same as the spiral sketch) ----------
const int IN1 = 8, IN2 = 9, IN3 = 10, IN4 = 11;   // stepper driver
const int SERVO_PIN = 6;                          // servo signal
const int STEPS_PER_TURN = 2048;                  // one platter revolution

// ============================================================
//  PICK YOUR SHAPE — this is the only line you must change!
// ============================================================
int SHAPE = 1;        // 1=flower 2=heart 3=star 4=ellipse
                      // 5=mandala 6=breathing spiral

// ---------- TUNE THESE (fit the drawing to YOUR machine) ----
int stepDelay   = 12;   // ms per platter step. Slow = sharper shapes.
int centerAngle = 90;   // servo angle that puts the pen at a
                        // "medium" distance from the center
int spread      = 35;   // how many servo degrees one unit of
                        // distance is worth. Bigger = bigger shape.
                        // Too big = pen flies off the paper!
// ------------------------------------------------------------

Servo arm;
int coilStep = 0;
long stepCount = 0;     // counts EVERY step ever taken — this is
                        // how the mandala and spiral know how many
                        // turns have gone by, not just the angle.

void setup() {
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  arm.attach(SERVO_PIN);
  arm.write(centerAngle);
  delay(500);
}

void loop() {
  platterStep();
  aimPen();
  delay(stepDelay);
}

// Spin the platter one step (the classic 4-coil sequence).
void platterStep() {
  coilStep = coilStep + 1;
  if (coilStep > 3) coilStep = 0;
  digitalWrite(IN1, coilStep == 0);
  digitalWrite(IN2, coilStep == 1);
  digitalWrite(IN3, coilStep == 2);
  digitalWrite(IN4, coilStep == 3);
  stepCount = stepCount + 1;
}

// ------------------------------------------------------------
// aimPen(): asks the current recipe how far out the pen should
// be, then turns that distance into a servo angle.
// "distance" is in machine units: 1.0 = medium, 0.5 = close in,
// 1.5 = far out. We clamp it so the pen can never leave the paper.
// ------------------------------------------------------------
void aimPen() {
  // The platter's angle within this turn (0–360)...
  float angle = (stepCount % STEPS_PER_TURN) * 360.0 / STEPS_PER_TURN;
  // ...and the TOTAL angle since we started (keeps growing past
  // 360 — the mandala and spiral need this to drift and grow).
  float totalAngle = stepCount * 360.0 / STEPS_PER_TURN;

  float distance = 1.0;

  // ================= THE SIX RECIPES =================

  if (SHAPE == 1) {
    // FLOWER: cosine with 3 bumps per turn = 3 petals.
    // Change the 3 to 5 for five petals. That's it!
    distance = 1.0 + 0.4 * cos(3 * angle * DEG_TO_RAD);
  }

  if (SHAPE == 2) {
    // HEART (cardioid): ONE bump per turn, big and round.
    // Compare to the flower — we changed 3 to 1 and made it bigger.
    distance = 0.5 + 0.5 * (1 + cos(angle * DEG_TO_RAD));
  }

  if (SHAPE == 3) {
    // STAR: the square trick (1/cos pushes pen out at corners),
    // but corners every 72 degrees = 5 points, and we exaggerate
    // the push by 3x so the points become spikes.
    float slice = fmod(angle, 72.0) - 36.0;
    float push = 1.0 / cos(slice * DEG_TO_RAD);
    if (push > 1.25) push = 1.25;             // don't overdo it
    distance = 1.0 + (push - 1.0) * 3.0;      // exaggerate = spiky
  }

  if (SHAPE == 4) {
    // ELLIPSE: this is the real orbit equation from astronomy!
    // The 0.4 is the "eccentricity" — how squashed the oval is.
    // 0 = perfect circle. Try 0.2 and 0.6.
    distance = 0.9 / (1.0 + 0.4 * cos(angle * DEG_TO_RAD));
  }

  if (SHAPE == 5) {
    // MANDALA: a flower with 6.1 petals. Because 6.1 doesn't fit
    // evenly into a circle, every turn lands slightly rotated.
    // Let it run 20+ turns and the overlaps weave a mandala.
    // NOTE: uses totalAngle so the drift carries across turns.
    distance = 1.0 + 0.35 * cos(6.1 * totalAngle * DEG_TO_RAD);
  }

  if (SHAPE == 6) {
    // BREATHING SPIRAL: a 5-petal flower whose size slowly grows.
    // totalAngle/360 = how many turns so far. Each turn adds a
    // little size, so the shape spirals outward as it draws.
    float grow = 0.5 + 0.05 * (totalAngle / 360.0);
    if (grow > 1.2) grow = 1.2;               // stop at the edge
    distance = grow * (1.0 + 0.3 * cos(5 * angle * DEG_TO_RAD));
  }

  // ===================================================

  // Safety clamp: whatever the recipe says, stay on the paper.
  if (distance < 0.5) distance = 0.5;
  if (distance > 1.6) distance = 1.6;

  // Distance --> servo angle, using your two TUNE numbers.
  arm.write((int)(centerAngle + (distance - 1.0) * spread));
}

/*
 *  STUDENT CHALLENGES:
 *  - Make a 7-petal flower. (Recipe 1, change one number.)
 *  - Make the ellipse rounder, then more squashed. (Recipe 4.)
 *  - Recipe 5: try 6.5 petals instead of 6.1. Why does the
 *    mandala fill in faster? (Hint: how many turns until 6.5
 *    petals line up again?)
 *  - Invent recipe 7. Any rule that turns an angle into a
 *    distance is a shape nobody has ever drawn before.
 */
