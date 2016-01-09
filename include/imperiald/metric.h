/**\file
 *
 * \copyright
 * This file is part of the imperiald project, which is released as open source
 * under the terms of an MIT/X11-style licence, described in the COPYING file.
 *
 * \see Documentation: https://ef.gy/documentation/imperiald
 * \see Source Code: https://github.com/ef-gy/imperiald
 * \see Licence Terms: https://github.com/ef-gy/imperiald/COPYING
 */

#if !defined(IMPERIALD_METRIC_H)
#define IMPERIALD_METRIC_H

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

  bool stop;
  std::thread updateThread;
  const std::string source;
};
}
}

#endif
