/*
 * ============================================================
 *  SPIROMATIC — Simplified Build
 *  Stepper motor spins the platter + servo motor sweeps the arm
 * ============================================================
 *
 *  HOW THIS MACHINE WORKS (the big picture):
 *
 *  1. The PLATTER sits on a stepper motor. A stepper motor has
 *     4 electromagnets (coils) inside. If we turn them on one
 *     at a time, in order — 1, 2, 3, 4, 1, 2, 3, 4... — the
 *     motor turns a tiny bit each time. Do it fast and the
 *     platter spins smoothly. WE control the coils ourselves.
 *
 *  2. The ARM holds the pen and is moved by a servo motor.
 *     A servo is easier: we just tell it an angle, like
 *     "go to 90 degrees," and it goes there and stays there.
 *
 *  3. The pen stays still while the paper spins under it, and
 *     the arm slowly slides the pen in and out. Spinning +
 *     sliding at the same time = spiral patterns!
 */

#include <Servo.h>   // borrows ready-made servo instructions

// ---------- WIRING (which wire goes where) ----------
// The stepper's driver board has 4 inputs: IN1, IN2, IN3, IN4.
// Each one turns on one coil inside the motor.
const int IN1 = 8;         // driver IN1 --> Arduino pin 8
const int IN2 = 9;         // driver IN2 --> Arduino pin 9
const int IN3 = 10;        // driver IN3 --> Arduino pin 10
const int IN4 = 11;        // driver IN4 --> Arduino pin 11
const int SERVO_PIN = 6;   // servo signal wire (orange) --> pin 6
// (servo red wire --> 5V, servo brown/black wire --> GND)

// ============================================================
//  TUNE THESE — this is YOUR control panel.
//  Change these numbers to change the art. Nothing else needs
//  to be edited to make a completely different drawing!
// ============================================================
int stepDelay  = 3;    // Milliseconds to wait between platter steps.
                       // SMALLER number = FASTER spin.
                       // Keep it between 2 and 15.
                       // WARNING: below 2 the motor can't keep up —
                       // it will just buzz and vibrate, not spin!

int platterDir = 1;    // Which way the platter spins.
                       // 1 = clockwise, -1 = counterclockwise.

int armMin     = 30;   // The arm sweeps back and forth between
int armMax     = 150;  // these two angles (in degrees, 0 to 180).
                       // Closer together = a thin ring of drawing.
                       // Far apart = pen travels across more paper.

int armSpeed   = 20;   // Milliseconds between each 1-degree arm move.
                       // BIGGER number = SLOWER sweep.
                       // Slow arm + fast platter = tight spiral rings.
// ============================================================

Servo arm;             // creates our servo and names it "arm"

int coilStep = 0;      // remembers which coil's turn it is (0,1,2,3)

int armPos;            // the arm's current angle right now
int armDir = 1;        // which way the arm is sweeping:
                       // 1 = counting up toward armMax,
                       // -1 = counting down toward armMin

unsigned long lastArmMove = 0;  // the clock time when the arm last
                                // moved (used so we know when it's
                                // due to move again)

// ------------------------------------------------------------
// setup() runs ONCE when the Arduino turns on.
// ------------------------------------------------------------
void setup() {
  // Tell the Arduino these 4 pins are OUTPUTS (we send signals
  // out of them to the motor driver).
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  arm.attach(SERVO_PIN);  // connect our "arm" servo to its pin

  armPos = armMin;        // start the arm at one end of its sweep...
  arm.write(armPos);      // ...and actually send it there
  delay(500);             // wait half a second so it arrives
}

// ------------------------------------------------------------
// loop() runs over and over, forever, very fast.
// Each trip through the loop:
//   - the platter moves ONE tiny step
//   - we check if the arm is due to move 1 degree
// ------------------------------------------------------------
void loop() {
  platterStep();      // move the platter one step
  updateArm();        // move the arm IF it's time
  delay(stepDelay);   // short pause — this sets the platter's speed
}

// ------------------------------------------------------------
// platterStep(): turn on the NEXT coil in the sequence.
// Calling this over and over is what spins the motor.
// ------------------------------------------------------------
void platterStep() {
  // Move to the next coil (adding -1 goes through the
  // sequence backwards — that's how direction works!)
  coilStep = coilStep + platterDir;

  // The sequence is a loop: after coil 3 comes coil 0 again.
  if (coilStep > 3) coilStep = 0;
  if (coilStep < 0) coilStep = 3;

  // Turn ON only this step's coil; the other three go OFF.
  // (For example: coilStep == 1 is "true" only for IN2,
  //  so IN2 gets HIGH and the rest get LOW.)
  digitalWrite(IN1, coilStep == 0);
  digitalWrite(IN2, coilStep == 1);
  digitalWrite(IN3, coilStep == 2);
  digitalWrite(IN4, coilStep == 3);

  // Watch the 4 LEDs on the driver board — you can SEE this
  // sequence happening if you make stepDelay big (like 100).
}

// ------------------------------------------------------------
// updateArm(): move the arm 1 degree — but only if enough time
// has passed since its last move.
//
// WHY NOT JUST USE delay()? Because delay() freezes EVERYTHING.
// If the arm waited using delay(), the platter would stop
// spinning every time. Instead we peek at the clock (millis()
// = milliseconds since the Arduino turned on) and only move
// when the arm's waiting time is up. The platter never stops!
// ------------------------------------------------------------
void updateArm() {
  // Has it been at least armSpeed ms since the arm last moved?
  if (millis() - lastArmMove >= armSpeed) {
    lastArmMove = millis();     // remember: the arm moved just now

    armPos = armPos + armDir;   // take one step (1 degree)

    // If the arm reached either end of its sweep, turn around.
    if (armPos >= armMax || armPos <= armMin) {
      armDir = -armDir;         // flips 1 to -1, or -1 to 1
    }

    arm.write(armPos);          // tell the servo its new angle
  }
}
