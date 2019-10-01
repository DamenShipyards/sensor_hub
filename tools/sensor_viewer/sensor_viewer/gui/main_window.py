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

from PyQt5.QtWidgets import QApplication, QMainWindow
from PyQt5.QtWidgets import QHBoxLayout, QVBoxLayout
from PyQt5.QtWidgets import QWidget, QPushButton, QTabWidget, QTableWidget

from .plot_canvas import PlotCanvas
from .labelled_controls import LabelledEdit, LabelledCombo


class MainWindow(QMainWindow):

    def add_button(self, layout, caption, hint, handler):
        button = QPushButton(caption, self)
        button.clicked.connect(handler)
        button.setToolTip(hint)
        layout.addWidget(button)

    @staticmethod
    def create_tabs():
        tabs = QTabWidget()
        tab_sensors = QWidget()
        tab_modbus = QWidget()       
        tabs.addTab(tab_sensors, "Sensors")
        tabs.addTab(tab_modbus, "Modbus")
        return tabs

    def setup_sensors_controls(self, widget):
        pass

    def setup_modbus_controls(self, widget):
        layout = QVBoxLayout()
        widget.setLayout(layout)

        top_bar = QWidget()
        top_bar_layout = QHBoxLayout()
        top_bar.setLayout(top_bar_layout)
        layout.addWidget(top_bar)

        main = QWidget()
        main_layout = QHBoxLayout()
        main.setLayout(main_layout)
        layout.addWidget(main)

        canvas = PlotCanvas(self, width=6, height=6)
        main_layout.addWidget(canvas)

        host = LabelledEdit(label='Host', mask='000.000.000.000;_', text='127.0.0.1', align='right')
        top_bar_layout.addWidget(host)
        port = LabelledEdit(label='Port', mask='00000;_', text='16502', align='right')
        top_bar_layout.addWidget(port)
        slave_id = LabelledEdit(label='Slave id', mask='000;_', text='0', align='right')
        top_bar_layout.addWidget(slave_id)
        function = LabelledCombo(label='Function',
                                 items=('Read input registers', 'Read holding registers', 'Read coils'))
        top_bar_layout.addWidget(function)
        registers = LabelledEdit(label='Registers', text='0:16')
        top_bar_layout.addWidget(registers)

        self.add_button(top_bar_layout, "Start", "Start polling modbus", canvas.plot_random)
        self.add_button(top_bar_layout, "Stop", "Stop polling modbus", canvas.plot_random_from_matrix)

        right_bar = QWidget()
        right_bar_layout = QVBoxLayout()
        right_bar.setLayout(right_bar_layout)
        main_layout.addWidget(right_bar)

        values = QTableWidget()
        right_bar_layout.addWidget(values)

    def __init__(self):
        super().__init__()
        self.title = 'Damen Sensor Hub Viewer'

        self.setWindowTitle(self.title)

        tabs = self.create_tabs()
        self.setCentralWidget(tabs)

        self.setup_sensors_controls(tabs.widget(0))
        self.setup_modbus_controls(tabs.widget(1))


def run_main(arguments):
    app = QApplication(arguments)
    win = MainWindow()
    win.show()
    app.exec_()
