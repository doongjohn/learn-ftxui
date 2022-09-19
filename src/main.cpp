#include <thread>
#include <atomic>
#include <chrono>
#include <cmath>
#include <functional>
#include <vector>
#include <string>

#include "ftxui/component/component.hpp"
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/event.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/dom/flexbox_config.hpp"

#include "fmt/format.h"
#include "utils.hpp"

using namespace utils;
using namespace ftxui;

constexpr const int thread_count = 3;
constexpr const int fps_limit = 30;
std::atomic_bool run_thread(true);

std::mutex mutex_worker_data;
std::vector<int> worker_avg_tick;
std::vector<int> worker_max_tick;
std::vector<int> worker_cur_fps;
std::vector<std::vector<int>> worker_fps;
std::vector<std::thread> worker_threads;

auto worker(int index, int target_fps) -> void {
  Timer tick_timer;
  const long tick_time_budget = (1.0f / (float)target_fps) * 1000; // milliseconds
  long tick_time_remaining = tick_time_budget;

  int tick_count = 0;
  float all_tick_time = 0;
  float avg_tick_time = 0;
  float max_tick_time = 0;
  float cur_fps = 0;

  Timer log_timer;
  int log_time_frame_count = 0;
  float all_fps_log_time_frame = 0;
  float avg_fps_log_time_frame = 0;

  // server main loop
  while (run_thread) {
    ++tick_count;
    ++log_time_frame_count;

    // reset timer
    tick_timer.reset();
    tick_time_remaining = tick_time_budget;

    // do some work
    if (random_range(0, 10) == 0) {
      sleep_ms(random_range(35, 60)); // lag
    } else {
      sleep_ms(random_range(0, 5));
    }

    // calcuate remaining time
    long tick_time_took = tick_timer.get_elapsed_ms();
    tick_time_remaining -= tick_time_took;

    // calcuate avg tick time
    all_tick_time += tick_time_took;
    avg_tick_time = all_tick_time / float(tick_count);

    // calcuate max tick time
    if (max_tick_time < tick_time_took) {
      max_tick_time = tick_time_took;
    }

    // calculate fps
    if (tick_time_remaining > 0) {
      cur_fps = target_fps;
      sleep_ms(tick_time_remaining);
    } else {
      cur_fps = 1.0f / (tick_time_took / 1000.0f);
    }
    all_fps_log_time_frame += cur_fps;

    // log every 5 seconds
    if (log_timer.get_elapsed_ms() >= 1 * 1000) {
      log_timer.reset();

      avg_fps_log_time_frame = round(all_fps_log_time_frame / float(log_time_frame_count));
      all_fps_log_time_frame = 0;
      log_time_frame_count = 0;

      std::scoped_lock lock(mutex_worker_data);
      worker_avg_tick[index] = avg_tick_time;
      worker_max_tick[index] = max_tick_time;
      worker_cur_fps[index] = avg_fps_log_time_frame;
      worker_fps[index].push_back(avg_fps_log_time_frame);
    }
  }
}

auto screen = ScreenInteractive::Fullscreen();

int main(int argc, const char* argv[]) {
  for (int i = 0; i < thread_count; ++i) {
    worker_avg_tick.push_back(0);
    worker_max_tick.push_back(0);
    worker_cur_fps.push_back(0);
    worker_fps.push_back(std::vector<int>());
    worker_threads.push_back(std::thread(worker, i, fps_limit));
  }

  std::vector<Component> contents(thread_count);
  for (int index = 0; index < thread_count; ++index) {
    contents[index] = Renderer([index] {
      return vbox({
        text(fmt::format("Worker thread {}", index)) | hcenter,
        text(fmt::format("avg tick: {}ms", worker_avg_tick[index])),
        text(fmt::format("max tick: {}ms", worker_max_tick[index])),
        text(fmt::format("fps: {}", worker_cur_fps[index])),
        separator(),
        text("FPS") | hcenter,
        hbox({
          vbox({
            text("30 "),
            filler(),
            text("20 "),
            filler(),
            text("10 "),
            filler(),
            text("0 "),
          }),
          graph([index](int width, int height) {
            std::vector<int> output(width);
            std::fill(output.begin(), output.end(), -1);

            std::unique_lock lock(mutex_worker_data);
            // handle screen width overflow
            int sample_pos = 0;
            if (worker_fps[index].size() > width / 2) {
              sample_pos += worker_fps[index].size() - width / 2;
            }
            // plot data
            for (int x = 0; x < width; x += 2) {
              if (worker_fps[index].size() > sample_pos) {
                output[x] = height * (worker_fps[index][sample_pos] / float(fps_limit));
                ++sample_pos;
              }
            }
            lock.unlock();
            return output;
          }) | flex,
        }) | flex,
      });
    });
  }

  std::vector<std::string> tab_names(thread_count);
  for (int i = 0; i < thread_count; ++i) {
    tab_names[i] = fmt::format("worker thread {} ", i);
  }

  int tab_index = 0;
  auto tab_selection = Menu(&tab_names, &tab_index, MenuOption::Vertical());
  auto tab_content = Container::Tab(
    contents,
    &tab_index
  );

  auto main_container = Container::Horizontal({
    tab_selection,
    tab_content,
  });

  auto main_renderer = Renderer(main_container, [&] {
    return vbox({
      text("Server status") | bold | hcenter,
      separator(),
      hbox({
        tab_selection->Render(),
        separator(),
        tab_content->Render() | flex,
      }) | flex
    });
  });

  std::thread refresh_ui([&] {
    while (run_thread) {
      using namespace std::chrono_literals;
      std::this_thread::sleep_for(50ms);
      // After updating the state, request a new frame to be drawn. This is done
      // by simulating a new "custom" event to be handled.
      screen.Post(Event::Custom);
    }
  });

  screen.Loop(main_renderer);

  run_thread = false;
  refresh_ui.join();

  // join all worker threads
  for (int i = 0; i < thread_count; ++i) {
    worker_threads[i].join();
  }

  return 0;
}
