var WshShell = WScript.CreateObject("WScript.Shell");
var WshEnv = WshShell.Environment("USER");
var FSO = WScript.CreateObject("Scripting.FileSystemObject");
var StdOut = WScript.StdOut;
var CD = WshShell.CurrentDirectory;
var ForReading = 1;
var ForWriting = 2;

var verfile = FSO.OpenTextFile(CD + "\\..\\Project\\VersionInfo.ini", ForReading);
var verlines = verfile.ReadAll().split("\r\n");
var version = "0.0.0.0";

for (i = 0; i < verlines.length; i++) {
  StdOut.WriteLine(verlines[i]);
  var verline = verlines[i].split('=');
  StdOut.WriteLine(verline[0]);
  if (verline.length > 1 && verline[0] == 'Version') {
    StdOut.WriteLine(verline[1]);
    version=verline[1];
  }
}

StdOut.WriteLine("Found version: " + version);

var verdigits = version.split(".");
var shortver = verdigits[0] + "." + verdigits[1] + "." + verdigits[2];

var outfile = FSO.OpenTextFile(CD + "\\version.iss", ForWriting, true, 0);
outfile.WriteLine("VersionInfoVersion=" + version);
outfile.WriteLine("AppVerName=\"Damen Research Plate Vibration " + version + "\"");
outfile.WriteLine("OutputBaseFilename=PlateVibration_v" + shortver);
outfile.close();

outfile = FSO.OpenTextFile(CD + "\\setup.resp", ForWriting, true, 0);
outfile.WriteLine("-dVersion=" + version);
outfile.close();

outfile = FSO.OpenTextFile(CD + "\\msi.resp", ForWriting, true, 0);
outfile.WriteLine("-o Output\\PlateVibration_v" + shortver + ".msi");
outfile.close();
