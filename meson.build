###############################################################################
# NAME:             meson.build
#
# AUTHOR:           Ethan D. Twardy <ethan.twardy@gmail.com>
#
# DESCRIPTION:      Meson build script for the application
#
# CREATED:          11/06/2021
#
# LAST EDITED:      11/20/2021
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
gnome = import('gnome')

# GDBus Sources
bluez_agent = gnome.gdbus_codegen(
  'bluez-agent',
  sources: 'gdbus/org.bluez.iot-agent.xml',
  interface_prefix: 'org.bluez.',
  namespace: 'IotAgent',
)

bluez = gnome.gdbus_codegen(
  'bluez',
  sources: 'gdbus/org.bluez.xml',
  interface_prefix: 'org.bluez.',
)

# Dependencies
libglib = dependency('glib-2.0')
libgio_unix = dependency('gio-unix-2.0')

# Source Files
agent_files = files([
  'source/bluez-iot-agent.c',
])
agent_files += bluez_agent
agent_files += bluez

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
  dependencies: [libglib, libgio_unix],
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
