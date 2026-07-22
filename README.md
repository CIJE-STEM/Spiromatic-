<img src="images/cije_logo.png" alt="CIJE" width="220">

# The Spiromatic

A stepper + servo motor powered art machine that uses math to draw flowers, hearts, squares, mandalas, and shapes nobody has invented yet. Built from upcycled materials around an Arduino, and taught as a full curriculum unit (engineering → code → math → art).

**The core idea students learn:** the machine only knows two numbers — how far the paper has turned (SPIN) and how far out the pen is (REACH). Every shape is a rule connecting them, and one function, `cos(angle)`, generates almost all of them.

## Repository layout

| Folder | What's inside |
|---|---|
| `curriculum/` | Teachers guide, answer key, and both classroom decks |
| `code/` | All Arduino sketches + the browser simulator |
| `images/` | Wiring diagrams and other graphics |

## Teacher materials (`curriculum/`)

| File | Description |
|---|---|
| `Spiromatic_Teachers_Guide_v2.docx` | The full curriculum guide: objectives, materials, stepper motor background, Lessons 1–5, build progression reference photos, pen setup, extensions, and a troubleshooting appendix. **Start here.** |
| `spiromatic_teacher_answer_key.docx` | Worked answers for all six Lesson 5 challenges, grading guidance for the open-ended challenge, and in-class troubleshooting. |
| `spiromatic_math_deck.pptx` | 25-slide classroom deck for Lesson 5 ("Drawing with Math"): the math explained gently with animations, stepper vs. servo, the code line by line, and leveled challenges. GIF animations play in slideshow mode (F5). |
| `Spiromatic_genesis_v2.pptx` | The workshop/PD story deck: the Spiromatic's evolution with real classroom photos and videos, ending with the cosine reboot and a handoff to the math deck. Present this first, then the math deck. (Media compressed for web.) |

## Student-facing tools (`code/`)

| File | Description |
|---|---|
| `spiromatic_simulator.html` | The machine on screen — open in any browser, works offline. Nine shape recipes with adjustable dials (petal count, push size, platter speed) and a live formula readout. Students find their favorite numbers here, then transfer them to the Arduino. |

## Arduino sketches (`code/`)

| File | Description |
|---|---|
| `spiromatic_stepper_servo.ino` | The simplified build (stepper platter + servo arm): a smooth spiral pattern. Fully commented for 9th graders. Good first upload. |
| `spiromatic_square.ino` | Draws a square on a spinning platter using `distance = 1 ÷ cos(angle)`. Includes discussion prompts and a triangle challenge. |
| `spiromatic_shapes.ino` | One sketch, six shape recipes (flower, heart, star, ellipse, mandala, breathing spiral). Students change `SHAPE = 1–6` and tune the marked numbers. |
| `spiromatic_shapes_2stepper_IRhome.ino` | **Spiromatic 2.0** — the two-stepper build (per the wiring guide): pen arm on a second stepper, IR sensor homes it to "step zero" on power-up, so drawings are precise and repeatable. Draws circles, spirals, any polygon, any flower, and nested concentric copies of each (`repeats = 5` = five squares inside each other). |

## Hardware

Arduino Uno, 28BYJ-48 stepper motor(s) with ULN2003 driver board(s), SG90 micro servo, IR sensor module (for the two-stepper build), jumper wires, and an upcycled cardboard/popsicle-stick body. Felt-tip markers strongly recommended (see the Pen Setup section of the guide). Wiring diagrams for every build are in `images/`.

## Quick start

1. Read the Teachers Guide (`curriculum/Spiromatic_Teachers_Guide_v2.docx`).
2. Open `code/spiromatic_simulator.html` in a browser to see what the machine will do.
3. Build Stage 1 (platter only), upload the platter test code, and pass the checkpoint: *spins smoothly both directions*.
4. Grow the machine stage by stage — each stage in the guide has a checkpoint gate.
5. Art day: complete Pen Setup first, then draw.

---

*Developed by CIJE. Student work photos are from TABC.*
