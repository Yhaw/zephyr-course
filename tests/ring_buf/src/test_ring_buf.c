/*
 * Ring Buffer Module - Homework Test Skeleton
 *
 * test_fresh_state is provided as a worked example. Fill in the remaining
 * 7 ZTEST bodies according to TEST_SPEC.md. Stubs call ztest_test_skip()
 * so the binary builds and runs cleanly before each test is implemented.
 *
 * Run:
 *   west twister -T tests/ring_buf -p native_sim
 */

#include <zephyr/ztest.h>
#include <errno.h>

#include "ring_buf.h"

/*
 * Shared before hook: every suite reinitialises the ring buffer with a
 * capacity of 4 so tests start from a clean, known state. Capacity 4 is
 * enough to exercise FIFO order (push 1, 2, 3) and overflow (full at 4).
 */
static void before(void *f)
{
	ARG_UNUSED(f);
	rb_init(4);
}

/*
 * ============================================================================
 * Test Suite: ring_buf_init
 *
 * Initial state and re-initialization behaviour.
 * ============================================================================
 */
ZTEST_SUITE(ring_buf_init, NULL, NULL, before, NULL, NULL);

/* PROVIDED — study this test before writing the rest. */
ZTEST(ring_buf_init, test_fresh_state)
{
	zassert_true(rb_is_empty(), "Fresh buffer must be empty");
	zassert_equal(rb_count(), 0, "Fresh buffer count must be 0");
}

ZTEST(ring_buf_init, test_reinit_clears_state)
{
	zassert_ok(rb_push(99), "Push should succeed");
	zassert_ok(rb_init(4), "Reinit should succeed");
	zassert_true(rb_is_empty(), "Buffer must be empty after reinit");
	zassert_equal(rb_count(), 0, "Buffer count must be 0 after reinit");
}

/*
 * ============================================================================
 * Test Suite: ring_buf_push_pop
 *
 * Single push/pop round-trip, FIFO order, full error path.
 * ============================================================================
 */
ZTEST_SUITE(ring_buf_push_pop, NULL, NULL, before, NULL, NULL);

ZTEST(ring_buf_push_pop, test_single_push_pop)
{
	int v;

	zassert_ok(rb_push(42), "Push should succeed");
	zassert_ok(rb_pop(&v), "Pop should succeed");
	zassert_equal(v, 42, "Popped value mismatch");
	zassert_true(rb_is_empty(), "Buffer should be empty");
}

ZTEST(ring_buf_push_pop, test_fifo_order)
{
	int v;

	zassert_ok(rb_push(1), "Push 1 should succeed");
	zassert_ok(rb_push(2), "Push 2 should succeed");
	zassert_ok(rb_push(3), "Push 3 should succeed");

	zassert_ok(rb_pop(&v), "First pop should succeed");
	zassert_equal(v, 1, "Expected first value 1");

	zassert_ok(rb_pop(&v), "Second pop should succeed");
	zassert_equal(v, 2, "Expected second value 2");

	zassert_ok(rb_pop(&v), "Third pop should succeed");
	zassert_equal(v, 3, "Expected third value 3");

	zassert_true(rb_is_empty(), "Buffer should be empty after pops");
}

ZTEST(ring_buf_push_pop, test_push_full_returns_enospc)
{
	for (int i = 1; i <= 4; i++) {
		zassert_ok(rb_push(i), "Push %d should succeed", i);
	}

	zassert_true(rb_is_full(), "Buffer should be full");
	zassert_equal(rb_push(99), -ENOSPC, "Expected -ENOSPC when full");
	zassert_equal(rb_count(), 4, "Count should still be 4");
}

/*
 * ============================================================================
 * Test Suite: ring_buf_boundaries
 *
 * Peek semantics and NULL-pointer boundary conditions.
 * ============================================================================
 */
ZTEST_SUITE(ring_buf_boundaries, NULL, NULL, before, NULL, NULL);

ZTEST(ring_buf_boundaries, test_peek_does_not_consume)
{
	int v;

	zassert_ok(rb_push(7), "Push should succeed");

	zassert_ok(rb_peek(&v), "First peek should succeed");
	zassert_equal(v, 7, "Expected 7");

	zassert_ok(rb_peek(&v), "Second peek should succeed");
	zassert_equal(v, 7, "Expected 7");

	zassert_equal(rb_count(), 1, "Peek must not consume element");
}

ZTEST(ring_buf_boundaries, test_pop_null_returns_einval)
{
	zassert_equal(rb_pop(NULL), -EINVAL, "rb_pop(NULL) should return -EINVAL");
}

ZTEST(ring_buf_boundaries, test_is_full_after_fill)
{
	for (int i = 1; i <= 4; i++) {
		zassert_ok(rb_push(i), "Push %d should succeed", i);
	}

	zassert_true(rb_is_full(), "Buffer must be full");
	zassert_equal(rb_count(), 4, "Buffer count must be 4");
}
