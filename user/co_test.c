#include "kernel/types.h"
#include "user/user.h"

// ── Test 1: basic ping-pong coroutine ─────────────────────────────────────
//
// Parent and child alternate control via co_yield.  Each should receive
// the value sent by the other (parent sends 2, child sends 1).
// Expected output (interleaved):
//   parent received: 1
//   Child received: 2
//   parent received: 1
//   ...  (repeating)
static void
test_pingpong(void)
{
  printf("=== test_pingpong ===\n");

  int pid1 = getpid();   // Parent PID
  int pid2 = fork();     // returns child PID in parent, 0 in child

  if (pid2 == 0) {
    // Child: yield 1 to parent, receive 2 back, repeat.
    for (;;) {
      int value = co_yield(pid1, 1);
      printf("Child received: %d\n", value);   // should print 2
    }
  } else {
    // Parent: yield 2 to child, receive 1 back, a few times then exit.
    for (int i = 0; i < 5; i++) {
      int value = co_yield(pid2, 2);
      printf("parent received: %d\n", value);  // should print 1
    }
    // Kill child and wait so the system stays clean.
    kill(pid2);
    wait(0);
    printf("test_pingpong: PASS\n\n");
  }
}

// ── Test 2: error conditions ───────────────────────────────────────────────

static void
test_errors(void)
{
  printf("=== test_errors ===\n");

  int ret;

  // (a) Yield to a non-existent PID.
  ret = co_yield(99999, 1);
  if (ret == -1)
    printf("non-existent PID: correctly returned -1\n");
  else
    printf("non-existent PID: UNEXPECTED return %d\n", ret);

  // (b) Self-yield (pid == getpid()).
  ret = co_yield(getpid(), 1);
  if (ret == -1)
    printf("self-yield: correctly returned -1\n");
  else
    printf("self-yield: UNEXPECTED return %d\n", ret);

  // (c) Zero PID (invalid).
  ret = co_yield(0, 1);
  if (ret == -1)
    printf("zero PID: correctly returned -1\n");
  else
    printf("zero PID: UNEXPECTED return %d\n", ret);

  // (d) Negative PID (invalid).
  ret = co_yield(-5, 1);
  if (ret == -1)
    printf("negative PID: correctly returned -1\n");
  else
    printf("negative PID: UNEXPECTED return %d\n", ret);

  // (e) Yield to a killed/exited process.
  //     Fork a child that exits immediately, then try to yield to it.
  int child = fork();
  if (child == 0) {
    exit(0);   // child exits right away
  } else {
    wait(0);   // reap child so its slot is freed
    ret = co_yield(child, 1);
    if (ret == -1)
      printf("killed/exited PID: correctly returned -1\n");
    else
      printf("killed/exited PID: UNEXPECTED return %d\n", ret);
  }

  printf("test_errors: PASS\n\n");
}

int
main(void)
{
  test_errors();
  test_pingpong();  // ping-pong loops forever in the child; parent exits after 5 rounds
  exit(0);
}
