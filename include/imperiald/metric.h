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
  file(const std::string &pFile,
       prometheus::collector::registry<prometheus::collector::base> &pRegistry =
           prometheus::collector::registry<
               prometheus::collector::base>::common())
      : source(pFile), prometheus::collector::hub(pRegistry) {}

  virtual std::string text(void) const {
    /* note that we're being delibrately evil here: yes, we normally want this
     * function to be const, but in this case it saves us a thread to not be.
     */
    ((file*)(this))->update();
    return prometheus::collector::hub::text();
  }

  bool update(void) {
    std::ifstream i(source);
    std::string line;

    while (std::getline(i, line)) {
      processLine(line);
    }

    return true;
  }

protected:
  virtual bool processLine(std::string &) = 0;

  const std::string source;
};
}
}

#endif
