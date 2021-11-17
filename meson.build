###############################################################################
# NAME:             meson.build
#
# AUTHOR:           Ethan D. Twardy <ethan.twardy@gmail.com>
#
# DESCRIPTION:      Meson build script for the application
#
# CREATED:          11/06/2021
#
# LAST EDITED:      11/17/2021
#
# Copyright 2021, Ethan D. Twardy
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
###

project('bluez-iot-agent', 'c', version: '0.1.0')

# Dependencies
libmicrohttpd = dependency('libmicrohttpd')
libdbus = dependency('dbus-1')
libev = meson.get_compiler('c').find_library('ev')
libseastar = dependency('libseastar')

# Source Files
agent_files = files([
  'source/bluez-iot-agent.c',
  'source/web-server.c',
  'source/logger.c',
  'source/bluez-proxy.c',
  'source/agent-server.c',
  'source/watch-manager.c',
  'source/timeout-manager.c',
  # 'source/state-machine.c',
  # 'source/states.c',
  # 'source/agent-callbacks.c',
])

# Configuration file
config_data = configuration_data({
  'version': meson.project_version(),
  'name': meson.project_name(),
})
configure_file(input: 'config.h.in', output: 'config.h',
               configuration: config_data)

executable(
  'bluez-iot-agent',
  sources: agent_files,
  dependencies: [libmicrohttpd, libdbus, libev, libseastar],
  install: true,
  c_args: ['-Wall', '-Wextra', '-Werror', '-Wno-unused-parameter'],
  include_directories: ['source'],
)

# Install dbus policy
install_data(
  'dbus-1/bluez-iot-agent.conf',
  install_dir: 'share/dbus-1/system.d'
)

###############################################################################
