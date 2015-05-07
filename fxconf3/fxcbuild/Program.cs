using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Text.RegularExpressions;

namespace fxcbuild {

	class Program {

		const string DEFINE_FILE  = "Defines.h";
		const string INPUTS_FILE  = "Inputs.mq4";
		const string COMPILE_FILE = "{0}.compile.{1}";
		const string COMPILER     = "mql64.exe";
		const string MQL_PATH     = @"mq4\";
		const string EA_PATH      = @"{0}\MetaQuotes\Terminal\{1}\MQL4\Experts\fxc\";

        static void Main(string[] args) {
			var useBuildNaming = false;

		#if DEBUG
			var cd  = @"D:\work\olsen\fxconf3\fxconf3\";
			var ea  = "Expert";
		#else
			var cd  = Environment.CurrentDirectory + @"\";
			var ea  = args[0];
			var dll = args[1];

			if (args.Length > 2) {
				useBuildNaming = args[2] == "1";
			}
		#endif

			var libDef    = ReadDef(cd + DEFINE_FILE);
			var stratPath = cd + Path.GetDirectoryName(libDef["STRAT_PATH"]) + @"\";
			var stratDef  = ReadDef(stratPath + DEFINE_FILE);

			var outFile = Regex.Replace(stratDef["EXPERT_NAME"], @"[^\w\d]", "");
			var outDir  = String.Format(EA_PATH, 
				Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), 
				GetMtHash(GetMtPath())
			);

			var mqlPath = cd + MQL_PATH;
			var srcFile = mqlPath + ea + ".mq4";
			var midFile = String.Format(COMPILE_FILE, mqlPath + ea, "mq4");

			File.Copy(srcFile, midFile, true);
			File.Copy(stratPath + INPUTS_FILE, mqlPath + INPUTS_FILE, true);

			var mergeDefs = new [] { libDef, stratDef }
				.SelectMany(x => x)
				.ToDictionary(x => x.Key, x => x.Value);

			mergeDefs["BUILD"] = mergeDefs["EXPERT_VERSION"] + "." + mergeDefs["LIB_VERSION"];
			mergeDefs["FILE"]  = outFile + (useBuildNaming ? "_" + mergeDefs["BUILD"] : "");

			ReplacePlaceHolders(midFile, mergeDefs);

			var start = new ProcessStartInfo() {
				FileName  = AppDomain.CurrentDomain.BaseDirectory + COMPILER,
				Arguments = midFile,
				UseShellExecute        = false,
				RedirectStandardOutput = true
			};
			
			using (var process = Process.Start(start)) {
				using (var reader = process.StandardOutput) {
					Console.Write(reader.ReadToEnd());
				}
			}

		#if !DEBUG
			midFile = Regex.Replace(midFile, @"\.mq4$", ".ex4", 
					RegexOptions.Compiled 
				  | RegexOptions.CultureInvariant 
				  | RegexOptions.IgnoreCase
			);
			File.Copy(midFile, outDir + mergeDefs["FILE"] + ".ex4", true);
			File.Copy(args[1], outDir + mergeDefs["FILE"] + ".dll", true);
		#endif
        }

		static Dictionary<string, string> ReadDef(string file) {
			var dic = new Dictionary<string, string>();

			string line;
			var regex  = new Regex(@"^\s*#define\s+([^\s]+)\s*(.+)$", RegexOptions.Compiled | RegexOptions.CultureInvariant | RegexOptions.IgnoreCase);
			var reader = new StreamReader(file);
			while (null != (line = reader.ReadLine())) {
				var match = regex.Match(line);

				if (match.Success) {
					dic[ match.Groups[1].Value ] = ParseDef(match.Groups[2].Value);
				}
			}
			reader.Close();

			return dic;
		}

		static string ParseDef(string str) {
			var len = 
					str.StartsWith("\"", StringComparison.InvariantCulture)  ? 1 : 
					str.StartsWith("L\"", StringComparison.InvariantCulture) ? 2 : 0;

			if (len > 0) {
				for (var i = len; i < str.Length; i++) {
					if (str[i] == '\\') {
						i++;
					}
					else if (str[i] == '"') {
						str = str.Substring(len, i - len);
					}
				}
				return str;
			}
			else {
				return Regex.Replace(str, @"\s.+$", "");
			}
		}

		static void ReplacePlaceHolders(string file, Dictionary<string, string> defs) {
			var regex   = new Regex(@"\{\{([\w\d]+)\}\}", RegexOptions.Compiled | RegexOptions.CultureInvariant);
			var content = File.ReadAllText(file);
			var matches = regex.Matches(content);
			
			content = regex.Replace(content, x => {
				return defs.ContainsKey(x.Groups[1].Value) ? defs[x.Groups[1].Value] : x.Value;
			});

			File.WriteAllText(file, content);
		}

        static string GetMtPath() {
            try {
                var regkey = @"SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\Alpari Limited MT4";
                var regval = "InstallLocation";
                return Registry.LocalMachine.OpenSubKey(regkey).GetValue(regval).ToString().ToUpper();
            } catch (Exception) {
                Environment.Exit(1010);
            }

            return String.Empty;
        }

        static string GetMtHash(string path) {
            var encodedPath = Encoding.GetEncoding("UTF-16LE").GetBytes(path);
            var hashBytes   = ((HashAlgorithm) CryptoConfig.CreateFromName("MD5")).ComputeHash(encodedPath);
            return BitConverter.ToString(hashBytes).Replace("-", String.Empty);
        }

	}

}
