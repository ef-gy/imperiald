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

#if !defined(IMPERIALD_PROCFS_LINUX_H)
#define IMPERIALD_PROCFS_LINUX_H

#include <imperiald/metric.h>
#include <regex>
#include <algorithm>
#include <iterator>

namespace imperiald {
namespace linux {
template <typename T = long long> class stat : public metric::file<T> {
public:
  stat(prometheus::collector::registry<prometheus::collector::base> &pRegistry =
           prometheus::collector::registry<
               prometheus::collector::base>::common(),
       const std::string &pFile = "/proc/stat")
      : metric::file<T>(pFile, pRegistry),
        bootTime("system_boot_time_seconds", {}, *this),
        contextSwitches("system_context_switches_total", {}, *this),
        interrupts("system_interrupts_total", {}, *this),
        forks("system_forks_total", {"state"}, *this),
        processes("system_processes", {"state"}, *this),
        CPUTime("system_cpu_user_hz", {"mode", "cpu"}, *this),
        pages("system_paging_pages", {"io"}, *this),
        swaps("system_swapping_pages", {"io"}, *this) {}

protected:
  virtual bool processLine(std::string &line) {
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
      bootTime.set(std::stoll(matches[1]));
    } else if (std::regex_match(line, matches, ctxt)) {
      contextSwitches.set(std::stoll(matches[1]));
    } else if (std::regex_match(line, matches, intr)) {
      interrupts.set(std::stoll(matches[1]));
    } else if (std::regex_match(line, matches, procs)) {
      forks.set(std::stoll(matches[1]));
    } else if (std::regex_match(line, matches, procs_running)) {
      processes.labels({"running"}).set(std::stoll(matches[1]));
    } else if (std::regex_match(line, matches, procs_blocked)) {
      processes.labels({"blocked"}).set(std::stoll(matches[1]));
    } else if (std::regex_match(line, matches, cpu)) {
      CPUTime.labels({"user", matches[1]}).set(std::stoll(matches[2]));
      CPUTime.labels({"nice", matches[1]}).set(std::stoll(matches[3]));
      CPUTime.labels({"system", matches[1]}).set(std::stoll(matches[4]));
      CPUTime.labels({"idle", matches[1]}).set(std::stoll(matches[5]));
    } else if (std::regex_match(line, matches, page)) {
      pages.labels({"in"}).set(std::stoll(matches[1]));
      pages.labels({"out"}).set(std::stoll(matches[2]));
    } else if (std::regex_match(line, matches, swap)) {
      swaps.labels({"in"}).set(std::stoll(matches[1]));
      swaps.labels({"out"}).set(std::stoll(matches[2]));
    }

    return true;
  }

  prometheus::metric::counter<T> bootTime;
  prometheus::metric::counter<T> contextSwitches;
  prometheus::metric::counter<T> interrupts;
  prometheus::metric::gauge<T> processes;
  prometheus::metric::counter<T> forks;
  prometheus::metric::counter<T> CPUTime;
  prometheus::metric::gauge<T> pages;
  prometheus::metric::gauge<T> swaps;
};

template <typename T = long long> class meminfo : public metric::file<T> {
public:
  meminfo(prometheus::collector::registry<prometheus::collector::base> &
              pRegistry = prometheus::collector::registry<
                  prometheus::collector::base>::common(),
          const std::string &pFile = "/proc/meminfo")
      : metric::file<T>(pFile, pRegistry),
        mem("system_memory_kibibytes", {"property"}, *this) {}

protected:
  virtual bool processLine(std::string &line) {
    static std::regex mi("(.*):\\s+([0-9]+)\\s+kB");
    std::smatch matches;

    if (std::regex_match(line, matches, mi)) {
      mem.labels({matches[1]}).set(std::stoll(matches[2]));
    }

    return true;
  }

  prometheus::metric::gauge<T> mem;
};

template <typename T = long long> class netstat : public metric::file<T> {
public:
  netstat(prometheus::collector::registry<prometheus::collector::base> &
              pRegistry = prometheus::collector::registry<
                  prometheus::collector::base>::common(),
          const std::string &pFile = "/proc/net/netstat")
      : metric::file<T>(pFile, pRegistry),
        net("system_netstat", {"ext", "property"}, *this) {}

protected:
  virtual bool processLine(std::string &line) {
    static std::regex header("(.*):((\\s+[a-zA-Z0-9]+)+)\\s*");
    static std::regex value("(.*):((\\s+[0-9]+)+)\\s*");
    std::smatch matches;

    if (std::regex_match(line, matches, value)) {
      std::istringstream iss(matches[2]);
      std::vector<std::string> values{std::istream_iterator<std::string>{iss},
                                      std::istream_iterator<std::string>{}};

      const auto &ext = matches[1];
      const auto &h = headers[ext];
      for (int i = 0; i < values.size() && i < h.size(); i++) {
        net.labels({ext, h[i]}).set(std::stoll(values[i]));
      }
    } else if (std::regex_match(line, matches, header)) {
      std::istringstream iss(matches[2]);
      headers[matches[1]] =
          std::vector<std::string>{std::istream_iterator<std::string>{iss},
                                   std::istream_iterator<std::string>{}};
    }

    return true;
  }

  prometheus::metric::gauge<T> net;
  std::map<std::string, std::vector<std::string>> headers;
};
}
}

#endif
