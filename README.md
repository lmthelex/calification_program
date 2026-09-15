# Calification program

The program evaluates one lab at a time. A lab has this layout:

```text
data/<course>/<lab>/
├── meta/
│   ├── rubric.txt
│   ├── students.txt
│   └── observations.txt
├── projects/
└── raw/
```

Build and run it from the repository root:

```sh
cmake -S . -B build
cmake --build build
./build/calification_program --eval prog2/lab1
./build/calification_program --eval prog2/lab1 ---report_widith 85
```

Automatic project extraction uses the `unzip` command, which must be available
on the machine running the program.

The program also finds `data/` by walking up from the current directory, so it
can be launched from `build/` with the same evaluation argument.

The default report width is 85 characters. `---report_widith` changes the
terminal rubric and PDF width; `--report-width` is also accepted. Rubric item
descriptions are word-wrapped and their continuation lines align below the
first description line.

## Input formats

`rubric.txt` contains criteria and deductions. Criterion base scores must total
exactly 20.00 or startup fails.

```text
Puntaje
01;[1.50];Criterion description
02;[18.50];Another criterion
Descuentos
a;[-1.00];Deduction description
```

`students.txt` accepts either of these formats. Multiple email addresses are
comma-separated.

```text
20220421;ROJAS, ANDERSON;student@example.com,other@example.com
20220421;ROJAS;ANDERSON;student@example.com,other@example.com
```

`observations.txt` uses one stable observation id per line:

```text
O1;A reusable observation
O2;Another reusable observation
```

## Generated files and states

Selecting **Show students** creates each expected student directory under
`projects/`. If it cannot find a directory tree with at least two `main.cpp`
files, it finds a `.zip` whose name contains the student code in `raw/` and
extracts it. A valid project receives a `raw_note.txt` initialized from the
rubric.

Scores, deductions, and observations are written atomically to `raw_note.txt`
after every grading action. A student is `BLANK` until an action is recorded,
`INCOMPLETE` while some criteria remain ungraded, and `READY` when every
criterion has a score. In the grading screen, `<` moves forward, `>` moves
backward, and `cod` prompts for a direct student-code jump. The list is
circular; use `q` to return to the main menu.

At every startup, existing `projects/**/raw_note.txt` files are synchronized
with the loaded rubric. Only the second column containing each base score is
replaced; achieved scores, descriptions, observations, and line endings are
preserved. Previously achieved scores are never discarded if a rubric weight
is lowered.

**Write feedback** creates or replaces `final_note.pdf` for every student whose
project is `READY`.

The email menu entry is reserved for the future email transport; no email is
sent by this version.

## Delete raw notes

To remove every `raw_note.txt` for one evaluation, run:

```sh
./scripts/DeleteRawNotes.sh --eval prog2/lab1
```

The script reports how many files it found and requires two confirmations
before deleting anything: `y`/`yes`, followed by the exact word `DELETE`.
