#include <unity.h>

#include "activities/pomodoro/PomodoroModel.h"

void test_duration_clamp() {
  PomodoroModel model;
  model.setDurationMinutes(2);
  TEST_ASSERT_EQUAL(PomodoroModel::kMinMinutes, model.getDurationMinutes());

  model.setDurationMinutes(90);
  TEST_ASSERT_EQUAL(PomodoroModel::kMaxMinutes, model.getDurationMinutes());
}

void test_state_transitions() {
  PomodoroModel model;
  model.setDurationMinutes(25);
  model.start(0);
  TEST_ASSERT_EQUAL(PomodoroState::Running, model.getState());

  model.pause();
  TEST_ASSERT_EQUAL(PomodoroState::Paused, model.getState());

  model.resume(1000);
  TEST_ASSERT_EQUAL(PomodoroState::Running, model.getState());

  model.resetToIdle();
  TEST_ASSERT_EQUAL(PomodoroState::Idle, model.getState());
}

void test_tick_completion() {
  PomodoroModel model;
  model.setDurationMinutes(5);
  model.start(0);

  const bool completed = model.tick(300000);
  TEST_ASSERT_TRUE(completed);
  TEST_ASSERT_EQUAL(PomodoroState::Done, model.getState());
  TEST_ASSERT_EQUAL(5 * 60, model.getRemainingSeconds());
}

void test_pause_freezes_time() {
  PomodoroModel model;
  model.setDurationMinutes(5);
  model.start(0);
  model.tick(120000);
  TEST_ASSERT_EQUAL(180, model.getRemainingSeconds());

  model.pause();
  model.tick(240000);
  TEST_ASSERT_EQUAL(180, model.getRemainingSeconds());
}

void test_reset_double_press() {
  PomodoroModel model;
  model.setDurationMinutes(25);
  model.start(0);

  const bool first = model.requestReset(1000);
  TEST_ASSERT_FALSE(first);
  TEST_ASSERT_EQUAL(PomodoroState::Running, model.getState());

  const bool second = model.requestReset(2500);
  TEST_ASSERT_TRUE(second);
  TEST_ASSERT_EQUAL(PomodoroState::Idle, model.getState());
}

void test_reset_window_expiry() {
  PomodoroModel model;
  model.setDurationMinutes(25);
  model.start(0);

  model.requestReset(0);
  TEST_ASSERT_FALSE(model.isResetPromptActive(2501));

  const bool reset = model.requestReset(3000);
  TEST_ASSERT_FALSE(reset);
  TEST_ASSERT_EQUAL(PomodoroState::Running, model.getState());
}

void test_seconds_bucket() {
  PomodoroModel model;
  model.setDurationMinutes(5);
  model.start(0);
  model.tick(245000);
  TEST_ASSERT_TRUE(model.shouldShowSeconds());
  TEST_ASSERT_EQUAL(50, model.getDisplaySecondsBucket());

  model.tick(254000);
  TEST_ASSERT_EQUAL(40, model.getDisplaySecondsBucket());

  PomodoroModel model2;
  model2.setDurationMinutes(5);
  model2.start(0);
  model2.tick(291000);
  TEST_ASSERT_EQUAL(10, model2.getDisplaySecondsBucket());
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_duration_clamp);
  RUN_TEST(test_state_transitions);
  RUN_TEST(test_tick_completion);
  RUN_TEST(test_pause_freezes_time);
  RUN_TEST(test_reset_double_press);
  RUN_TEST(test_reset_window_expiry);
  RUN_TEST(test_seconds_bucket);
  return UNITY_END();
}
