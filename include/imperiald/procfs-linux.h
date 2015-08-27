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

namespace imperiald {
namespace linux {
template <typename T = long long>
class stat : public prometheus::collector::hub {
public:
  stat(const T &updateInterval,
       prometheus::collector::registry &pRegistry =
         prometheus::collector::registry::common(),
       const std::string &prefix = "system_",
       const std::string &pFile = "/proc/stat")
    : file(pFile),
      prometheus::collector::hub(prefix + "linux_procfs_stats", pRegistry,
      "hub"),
      bootTime(prefix + "boot_time_seconds", pRegistry)
    {
      update();
    }

  bool update (void) {
    std::ifstream i(file);
    std::string line;

    while (std::getline(i, line)) {
      static std::regex btime("btime ([0-9]+).*");
      std::smatch matches;

      if (std::regex_match(line, matches, btime)) {
        std::istringstream iss(matches[1]);
        T bt;

        iss >> bt;

        bootTime.set(bt);
      }
    }

    return true;
  }

protected:
  const std::string file;
  prometheus::metric::gauge<T> bootTime;
};
}
}

#endif
