var WshShell = WScript.CreateObject("WScript.Shell");
var WshEnv = WshShell.Environment("USER");
var FSO = WScript.CreateObject("Scripting.FileSystemObject");
var StdOut = WScript.StdOut;
var CD = WshShell.CurrentDirectory;
var ForReading = 1;
var ForWriting = 2;

var verfile = FSO.OpenTextFile(CD + "\\..\\version.ini", ForReading);
var verlines = verfile.ReadAll().split("\r\n");
var version = "0.0.0.0";
var product = "Unknown"
var gitrevfile = FSO.OpenTextFile(CD + "\\gitrev.txt", ForReading);
var gitrev = gitrevfile.ReadAll();
var today = new Date();
// On windows only release versions are being build
var build_type = "Release"

var dd = today.getDate()
if (dd < 10)
	dd = '0' + dd
else
	dd = '' + dd
var mm = today.getMonth() + 1;  //January is 0!
if (mm < 10)
	mm = '0' + mm
else
	m = '' + mm

var yyyy = today.getFullYear();
var date = yyyy + '-' + mm + '-' + dd;

String.prototype.trim = function()
{
  return this.replace(/^\s+|\s+$/g, '');
};
gitrev = gitrev.trim();


for (i = 0; i < verlines.length; i++) {
  var verline = verlines[i].split('=');
  if (verline.length > 1 && verline[0] == 'Version') {
    version=verline[1];
  }
  if (verline.length > 1 && verline[0] == 'Name') {
    product=verline[1];
  }
}

StdOut.WriteLine("Found product: " + product);
StdOut.WriteLine("Found version: " + version);
StdOut.WriteLine("Git revision : " + gitrev);
StdOut.WriteLine("Build date   : " + date);
StdOut.WriteLine("Build type   : " + build_type);

var verdigits = version.split(".");
var shortver = verdigits[0] + "." + verdigits[1] + "." + verdigits[2];

var outfile = FSO.OpenTextFile(CD + "\\version.rc", ForWriting, true, 0);
outfile.WriteLine("#include <winresrc.h>");
outfile.WriteLine("1 VERSIONINFO");
outfile.WriteLine("FILEVERSION " + verdigits[0] + ", " + verdigits[1] + ", " + verdigits[2] + ", " + verdigits[3]);
outfile.WriteLine("PRODUCTVERSION " + verdigits[0] + ", " + verdigits[1] + ", " + verdigits[2] + ", 0");
outfile.WriteLine("FILEFLAGSMASK VS_FFI_FILEFLAGSMASK");
outfile.WriteLine("FILEOS VOS__WINDOWS32");
outfile.WriteLine("FILETYPE VFT_APP");
outfile.WriteLine("{");
outfile.WriteLine("BLOCK \"StringFileInfo\"");
outfile.WriteLine("{");
outfile.WriteLine("BLOCK \"040904E4\"");
outfile.WriteLine("{");
outfile.WriteLine("VALUE \"CompanyName\", \"Damen Shipyards\"");
outfile.WriteLine("VALUE \"FileVersion\", \"" + version + "\"");
outfile.WriteLine("VALUE \"FileDescription\", \"Sensor logging service\"");
outfile.WriteLine("VALUE \"InternalName\", \"SensorHub\"");
outfile.WriteLine("VALUE \"LegalCopyright\", \"\251 2018-2018 Damen Shipyards\"");
outfile.WriteLine("VALUE \"OriginalFilename\", \"sensor_hub.exe\"");
outfile.WriteLine("VALUE \"Comments\", \"Log and redistribute sensor data\"");
outfile.WriteLine("VALUE \"ProductName\", \"" + product + "\"");
outfile.WriteLine("VALUE \"ProductVersion\", \"" + shortver + ".0\"");
outfile.WriteLine("VALUE \"SourceRevision\", \"" + gitrev + "\"");
outfile.WriteLine("}");
outfile.WriteLine("}");
outfile.WriteLine("BLOCK \"VarFileInfo\"");
outfile.WriteLine("{");
outfile.WriteLine("VALUE \"Translation\", 1033, 1252");
outfile.WriteLine("}");
outfile.WriteLine("END");
outfile.close();

outfile = FSO.OpenTextFile(CD + "\\..\\src\\version.h", ForWriting, true, 0);
outfile.WriteLine("#define GITREV " + gitrev);
outfile.WriteLine("#define VERSION " + version);
outfile.WriteLine("#define BUILD_DATE " + date);
outfile.WriteLine("#define BUILD_TYPE " + build_type);
outfile.close();
