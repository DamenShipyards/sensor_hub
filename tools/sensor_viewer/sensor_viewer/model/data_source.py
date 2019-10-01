"""
Application data provider
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

import random
import numpy
from numpy.matlib import rand

class DataProvider():

    @staticmethod
    def get_random() -> list:
        """ Return array of 25 random numbers
        :rtype: list
        """
        return [random.random() for i in range(25)]

    @staticmethod
    def get_random_from_matrix() -> numpy.flatiter:
        """
        Construct matrix of 25 by 25 random numbers
        and return its diagonal as an array
        :rtype: numpy.flatiter
        """
        mat = rand((25, 25))
        return mat.diagonal().flat
