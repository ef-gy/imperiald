/**\file
 *
 * \copyright
 * Copyright (c) 2015, Magnus Achim Deininger <magnus@ef.gy>
 * \copyright
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * \copyright
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * \copyright
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * \see Documentation: https://ef.gy/documentation/imperiald
 * \see Source Code: https://github.com/jyujin/imperiald
 * \see Licence Terms: https://github.com/jyujin/imperiald/COPYING
 */

#if !defined(IMPERIALD_PROCFS_LINUX_H)
#define IMPERIALD_PROCFS_LINUX_H

#include <prometheus/metric.h>
#include <fstream>
#include <regex>
#include <thread>
#include <chrono>

namespace imperiald {
namespace linux {
template <typename T = long long>
class stat : public prometheus::collector::hub {
public:
  stat(const T &updateInterval,
       prometheus::collector::registry<prometheus::collector::base> &pRegistry =
           prometheus::collector::registry<
               prometheus::collector::base>::common(),
       const std::string &pFile = "/proc/stat")
      : file(pFile), prometheus::collector::hub(pRegistry),
        bootTime("system_boot_time_seconds", {}, *this),
        contextSwitches("system_context_switches_total", {}, *this),
        interrupts("system_interrupts_total", {}, *this),
        forks("system_forks_total", {"state"}, *this),
        processes("system_processes", {"state"}, *this),
        CPUTime("system_cpu_user_hz", {"mode", "cpu"}, *this),
        pages("system_paging_pages", {"io"}, *this),
        swaps("system_swapping_pages", {"io"}, *this) {
    updateThread = std::thread([this, updateInterval]() {
      while (!stop) {
        update();
        std::this_thread::sleep_for(std::chrono::seconds(updateInterval));
      }
    });
  }

  ~stat() {
    stop = true;
    updateThread.join();
  }

  static T asNumber(const std::string &s) {
    std::istringstream iss(s);
    T bt;

    iss >> bt;
    return bt;
  }

protected:
  bool update(void) {
    std::ifstream i(file);
    std::string line;

    while (std::getline(i, line)) {
      static std::regex btime("btime ([0-9]+).*");
      static std::regex ctxt("ctxt ([0-9]+).*");
      static std::regex intr("intr ([0-9]+).*");
      static std::regex procs("processes ([0-9]+).*");
      static std::regex procs_running("procs_running ([0-9]+).*");
      static std::regex procs_blocked("procs_blocked ([0-9]+).*");
      static std::regex cpu(
          "cpu([0-9]+)\\s+([0-9]+) ([0-9]+) ([0-9]+) ([0-9]+).*");
      static std::regex page("page ([0-9]+) ([0-9]+).*");
      static std::regex swap("swap ([0-9]+) ([0-9]+).*");
      std::smatch matches;

      if (std::regex_match(line, matches, btime)) {
        bootTime.set(asNumber(matches[1]));
      } else if (std::regex_match(line, matches, ctxt)) {
        contextSwitches.set(asNumber(matches[1]));
      } else if (std::regex_match(line, matches, intr)) {
        interrupts.set(asNumber(matches[1]));
      } else if (std::regex_match(line, matches, procs)) {
        forks.set(asNumber(matches[1]));
      } else if (std::regex_match(line, matches, procs_running)) {
        processes.labels({"running"}).set(asNumber(matches[1]));
      } else if (std::regex_match(line, matches, procs_blocked)) {
        processes.labels({"blocked"}).set(asNumber(matches[1]));
      } else if (std::regex_match(line, matches, cpu)) {
        CPUTime.labels({"user", matches[1]}).set(asNumber(matches[2]));
        CPUTime.labels({"nice", matches[1]}).set(asNumber(matches[3]));
        CPUTime.labels({"system", matches[1]}).set(asNumber(matches[4]));
        CPUTime.labels({"idle", matches[1]}).set(asNumber(matches[5]));
      } else if (std::regex_match(line, matches, page)) {
        pages.labels({"in"}).set(asNumber(matches[1]));
        pages.labels({"out"}).set(asNumber(matches[2]));
      } else if (std::regex_match(line, matches, swap)) {
        swaps.labels({"in"}).set(asNumber(matches[1]));
        swaps.labels({"out"}).set(asNumber(matches[2]));
      }
    }

    return true;
  }

  bool stop;
  const std::string file;
  prometheus::metric::counter<T> bootTime;
  prometheus::metric::counter<T> contextSwitches;
  prometheus::metric::counter<T> interrupts;
  prometheus::metric::gauge<T> processes;
  prometheus::metric::counter<T> forks;
  prometheus::metric::counter<T> CPUTime;
  prometheus::metric::gauge<T> pages;
  prometheus::metric::gauge<T> swaps;
  std::thread updateThread;
};
}
}

#endif
