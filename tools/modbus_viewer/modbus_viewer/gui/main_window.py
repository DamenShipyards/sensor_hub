"""
Main window for demo application
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

from PyQt5.QtWidgets import QApplication, QMainWindow, QWidget, QPushButton, QHBoxLayout, QVBoxLayout

from .plot_canvas import PlotCanvas


class MainWindow(QMainWindow):

    def __init__(self):
        super().__init__()
        self.title = 'Damen Sensor Hub Modbus Viewer'

        self.setWindowTitle(self.title)

        widget = QWidget()
        self.setCentralWidget(widget)
        layout = QHBoxLayout()
        widget.setLayout(layout)

        canvas = PlotCanvas(self, width=6, height=6)
        panel = QWidget()

        layout.addWidget(canvas)
        layout.addWidget(panel)

        layout = QVBoxLayout()
        panel.setLayout(layout)

        def add_button(caption, hint, handler):
            button = QPushButton(caption, self)
            button.clicked.connect(handler)
            button.setToolTip(hint)
            layout.addWidget(button)

        add_button("New Random 1", "Plot random values", canvas.plot_random)
        add_button("New Random 2", "Plot random values from matrix", canvas.plot_random_from_matrix)
        add_button("Clear", "Clear the plot figure", canvas.clear)
        layout.addWidget(QWidget())


def run_main(arguments):
    app = QApplication(arguments)
    win = MainWindow()
    win.show()
    app.exec_()
