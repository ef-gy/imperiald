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

#if !defined(IMPERIALD_PROCFS_METRIC_H)
#define IMPERIALD_PROCFS_METRIC_H

#include <prometheus/metric.h>
#include <fstream>
#include <thread>
#include <chrono>

namespace imperiald {
namespace metric {
template <typename T = long long>
class file : public prometheus::collector::hub {
public:
  file(const T &updateInterval, const std::string &pFile,
       prometheus::collector::registry<prometheus::collector::base> &pRegistry =
           prometheus::collector::registry<
               prometheus::collector::base>::common())
      : stop(false), source(pFile), prometheus::collector::hub(pRegistry) {
    updateThread = std::thread([this, updateInterval]() {
      while (!stop) {
        update();
        std::this_thread::sleep_for(std::chrono::seconds(updateInterval));
      }
    });
  }

  virtual ~file() {
    stop = true;
    updateThread.join();
  }

protected:
  virtual bool processLine(std::string &) = 0;

  bool update(void) {
    std::ifstream i(source);
    std::string line;

    while (std::getline(i, line)) {
      processLine(line);
    }

    return true;
  }

  static T asNumber(const std::string &s) {
    std::istringstream iss(s);
    T bt;

    iss >> bt;
    return bt;
  }

  bool stop;
  std::thread updateThread;
  const std::string source;
};
}
}

#endif
