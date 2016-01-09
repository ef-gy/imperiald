/**\file
 * \ingroup example-programmes
 * \brief imperiald main Server
 *
 * An example HTTP server that only serves /metrics and a 404 on all other
 * resources.
 *
 * Call it like this:
 * \code
 * $ ./server http:localhost:8080
 * \endcode
 *
 * With localhost and 8080 being a host name and port of your choosing. Then,
 * while the programme is running, open a browser and go to
 * http://localhost:8080/metrics and you should see the rather small list of
 * metrics.
 *
 * \copyright
 * This file is part of the imperiald project, which is released as open source
 * under the terms of an MIT/X11-style licence, described in the COPYING file.
 *
 * \see Documentation: https://ef.gy/documentation/imperiald
 * \see Source Code: https://github.com/ef-gy/imperiald
 * \see Licence Terms: https://github.com/ef-gy/imperiald/COPYING
 */

#define ASIO_DISABLE_THREADS
#include <prometheus/httpd.h>
#include <imperiald/procfs-linux.h>

using namespace efgy;

static imperiald::linux::stat<> linux_procfs_stats;
static imperiald::linux::meminfo<> linux_procfs_meminfo;
static imperiald::linux::netstat<> linux_procfs_netstat;

static httpd::servlet<asio::local::stream_protocol>
    unixQuit("/quit", httpd::quit<asio::local::stream_protocol>);

int main(int argc, char *argv[]) { return io::main(argc, argv); }
