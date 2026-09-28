using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NDiskDef
{
	internal class Program
	{
		static void Main(string[] args)
		{
			//Drives drives = new Drives();
			//drives.Test();

			Console.WriteLine($"\r\n{Constants.ProgName} {Constants.ProgVersion} *dg*\r\n");

			if (args.Length == 0)
			{
				Console.WriteLine("'NDiskDef BIOS.DEF' creates BIOS1.INC and BIOS2.INC");
				return;
			}

			if (!File.Exists(args[0]))
			{
				Console.WriteLine($"{args[0]} not found.");
				return;
			}

			Drives drives = new Drives();
			drives.Convert(args[0]);
			//drives.Test();
		}
	}
}
