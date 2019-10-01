#!/usr/bin/env python
"""
Graphical sensor viewer
"""

__author__ = "J.R. Versteegh"
__copyright__ = "Damen Shipyards Gorinchem"
__contact__ = "<j.r.versteegh@orca-st.com>"
__version__ = "0.1"
__license__ = """
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3
 as published by the Free Software Foundation.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program. If not, see <https://www.gnu.org/licenses/>.
"""

import sys

from sensor_viewer.gui.main_window import run_main

if __name__ == '__main__':
    run_main(sys.argv)
