"""
Plot widget with matplotlib figure
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

from PyQt5.QtWidgets import QSizePolicy

from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure
import matplotlib.pyplot as plt

from ..model.data_source import DataProvider

import numpy as np


class PlotCanvas(FigureCanvas):

    def __init__(self, parent=None, width=5, height=4, dpi=100):
        self.figure = Figure(figsize=(width, height), dpi=dpi)

        FigureCanvas.__init__(self, self.figure)
        self.setParent(parent)

        FigureCanvas.setSizePolicy(
            self,
            QSizePolicy.Expanding,
            QSizePolicy.Expanding)
        FigureCanvas.updateGeometry(self)

        self.axes = self.add_plot()

    def add_plot(self):
        axes = self.figure.add_subplot(111)
        axes.set_title("Random data")
        return axes

    def plot_random(self):
        data = DataProvider.get_random()
        self.plot(data)

    def plot_random_from_matrix(self):
        data = DataProvider.get_random_from_matrix()
        self.plot(data)

    def plot(self, data):
        self.axes.plot(data, 'r-')
        self.draw()

    def clear(self):
        self.figure.clear()
        self.axes = self.add_plot()
        self.draw()
