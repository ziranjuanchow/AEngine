# Project Workflow

## Guiding Principles

1. **The Plan is the Source of Truth:** All work must be tracked in `plan.md`
2. **The Tech Stack is Deliberate:** Changes to the tech stack must be documented in `tech-stack.md` *before* implementation
3. **Test-Driven Development:** Write unit tests before implementing functionality
4. **High Code Coverage:** Aim for >80% code coverage for all modules
5. **User Experience First:** Every decision should prioritize user experience
6. **Non-Interactive & CI-Aware:** Prefer non-interactive commands. Use `CI=true` for watch-mode tools (tests, linters) to ensure single execution.

## Task Workflow

All tasks follow a strict lifecycle, but code commits are batched by phase.

### Standard Task Workflow (Per Task)

1. **Select Task:** Choose the next available task from `plan.md` in sequential order.
2. **Mark In Progress:** Before beginning work, edit `plan.md` and change the task from `[ ]` to `[~]`.
3. **Write Failing Tests (Red Phase):**
   - Create a new test file for the feature or bug fix.
   - Write one or more unit tests.
   - **CRITICAL:** Run the tests and confirm that they fail as expected.
4. **Implement to Pass Tests (Green Phase):**
   - Write the minimum amount of application code necessary to make the failing tests pass.
   - Run the test suite again and confirm that all tests now pass.
5. **Refactor (Optional but Recommended):**
   - Refactor the implementation and test code while ensuring tests still pass.
6. **Verify Coverage:** Run coverage reports. Target: >80% coverage for new code.
7. **Document Deviations:** If implementation differs from tech stack, update `tech-stack.md` before proceeding.
8. **Stage Changes:** `git add` all code changes related to the task. **Do not commit yet.**
9. **Update Plan:** Read `plan.md`, update task status to `[x]`, and add a brief summary of changes directly in the task description or a sub-bullet.

### Phase Completion Verification and Checkpointing Protocol

**Trigger:** Executed immediately after a task is completed that concludes a phase in `plan.md`.

1. **Announce Protocol Start:** Inform the user that the phase is complete.
2. **Ensure Test Coverage for Phase Changes:**
   - List all changed files in the current phase (since last commit).
   - Verify corresponding test files exist and cover the new logic.
3. **Execute Automated Tests with Proactive Debugging:**
   - Run the full test suite for the phase.
   - Debug and fix up to two times if tests fail.
4. **Propose a Detailed, Actionable Manual Verification Plan:**
   - Generate a step-by-step plan for the user to verify the phase's output.
5. **Await Explicit User Feedback:**
   - **PAUSE** and await the user's "yes" or feedback.
6. **Commit Phase Changes:**
   - **Action:** Stage all changes (including the updated `plan.md`).
   - **Action:** Perform a single commit for the entire phase.
   - **Commit Message Format:**
     ```
     feat/fix(phase): <Phase Name> Completion

     Summary of Tasks:
     - Task 1: <Description & Key Changes>
     - Task 2: <Description & Key Changes>
     ...
     
     Verification:
     - Automated tests passed (Command: <cmd>)
     - Manual verification confirmed by user.
     ```
7. **Record Phase Checkpoint SHA:**
   - Update `plan.md` with the commit hash at the phase heading: `[checkpoint: <sha>]`.
   - Commit the `plan.md` update: `conductor(plan): Mark phase '<PHASE NAME>' as complete`.
8. **Announce Completion:** Inform the user that the phase and its checkpoint are finalized.

## Quality Gates

Before finishing a phase, verify:
- [ ] All tests pass
- [ ] Code coverage >80%
- [ ] Code follows project style guidelines
- [ ] All public API is documented
- [ ] No linting errors

## Development Commands (C++ / vcpkg / CMake)

### Setup
```bash
# Initialize vcpkg
# git clone https://github.com/microsoft/vcpkg.git
# ./vcpkg/bootstrap-vcpkg.sh
# ./vcpkg/vcpkg install imgui glfw3 glm rttr spdlog assimp stb

# Configure CMake
# cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[path/to/vcpkg]/scripts/buildsystems/vcpkg.cmake
```

### Daily Development
```bash
# Build
# cmake --build build

# Run Tests
# cd build && ctest
```

## Commit Guidelines
- Use the batched phase commit message format defined in the Phase Completion protocol.
- Individual types (`feat`, `fix`, etc.) should be used in the phase summary.
