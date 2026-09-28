using System;
using System.IO;

namespace HexReset
{
    internal class Program
    {
		static void Main(string[] args)
		{
			Console.WriteLine("\r\nHexReset 1.0: set intelhex start address to 0000h\r\n");
			if (args.Length < 1)
			{
				Console.WriteLine("usage: HexReset inputfile [inputfile]");
				return;
			}

			string src = args[0];
			string dest = args.Length > 1 ? args[1] : null;

			if (Path.GetExtension(src) == "") src += ".hex";
			if (string.IsNullOrEmpty(dest))
			{
				dest = src;
			}
			else
			{
				if (Path.GetExtension(dest) == "") dest += ".hex";
			}

			IntelHex hex = new IntelHex();
			hex.Convert(src, dest);
        }
    }
}
