call venv/Scripts/Activate.bat
pyinstaller --name=modbus_viewer --onefile --windowed --version-file=version.txt --icon=app.ico app.py