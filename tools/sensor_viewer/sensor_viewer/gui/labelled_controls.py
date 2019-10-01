"""
LineEdit with label
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

from PyQt5.QtWidgets import QWidget, QHBoxLayout, QLabel, QLineEdit, QComboBox
from PyQt5.QtCore import Qt


class LabelledEdit(QWidget):

    def __init__(self, *args, **kwargs):
        super().__init__()
        layout = QHBoxLayout()
        self.setLayout(layout)
        self.label = QLabel()
        self.edit = QLineEdit()
        layout.addWidget(self.label)
        layout.addWidget(self.edit)
        if 'label' in kwargs:
            self.label.setText(kwargs['label'])
        if 'mask' in kwargs:
            self.edit.setInputMask(kwargs['mask'])
        if 'text' in kwargs:
            self.edit.setText(kwargs['text'])
        if 'align' in kwargs:
            self.edit.setAlignment(Qt.AlignRight if kwargs['align'] == 'right' else Qt.AlignLeft)
        if 'enabled' in kwargs:
            self.setEnabled(bool(kwargs['enabled']))


class LabelledCombo(QWidget):
    def __init__(self, *args, **kwargs):
        super().__init__()
        layout = QHBoxLayout()
        self.setLayout(layout)
        self.label = QLabel()
        self.combo = QComboBox()
        layout.addWidget(self.label)
        layout.addWidget(self.combo)
        if 'label' in kwargs:
            self.label.setText(kwargs['label'])
        if 'items' in kwargs:
            for item in kwargs['items']:
                self.combo.addItem(item)
        if 'enabled' in kwargs:
            self.setEnabled(bool(kwargs['enabled']))
