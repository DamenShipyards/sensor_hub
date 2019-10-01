call venv/Scripts/Activate.bat
pyinstaller --name=sensor_hub_monitor --onefile --windowed --version-file=version.txt --icon=app.ico app.py
