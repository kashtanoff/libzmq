using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace MtHash {

    class Program {

        static void Main(string[] args) {
            var path = GetMtPath();
            var hash = GetMtHash(path);
            Console.Write(hash);
        }

        static string GetMtPath() {
            try {
                var regkey = @"SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\Alpari Limited MT4";
                var regval = "InstallLocation";
                return Registry.LocalMachine.OpenSubKey(regkey).GetValue(regval).ToString().ToUpper();
            } catch (Exception ex) {
                Environment.Exit(1010);
            }

            return String.Empty;
        }

        static string GetMtHash(string path) {
            var encodedPath = Encoding.GetEncoding("UTF-16LE").GetBytes(path);
            var hashBytes   = ((HashAlgorithm)CryptoConfig.CreateFromName("MD5")).ComputeHash(encodedPath);
            return BitConverter.ToString(hashBytes).Replace("-", String.Empty);
        }

    }

}