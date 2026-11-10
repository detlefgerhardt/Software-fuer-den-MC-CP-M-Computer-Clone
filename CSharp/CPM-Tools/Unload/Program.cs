using System;
using System.CodeDom;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Unload
{
	internal class Program
	{
		static void Main(string[] args)
		{
			Console.WriteLine($"\r\n{Constants.ProgName} {Constants.ProgVersion} *dg*\r\n");

			if (args.Length < 2)
			{
				Console.WriteLine($"usage: {Constants.ProgName} filename start-address");
				Console.WriteLine($"examples:");
				Console.WriteLine($"  convert binary to hex");
				Console.WriteLine($"    {Constants.ProgName} TEST.COM A000");
				Console.WriteLine($"  rewrite hex with different start address");
				Console.WriteLine($"    {Constants.ProgName} TEST.HEX 0000");
				return;
			}

			string filename = args[0].ToUpper();

			if (!File.Exists(filename))
			{
				Console.WriteLine($"{filename} not found.");
				return;
			}

			HexConverter hc = new HexConverter();
			int? startAddr = HexConverter.GetHexVal(args[1]);
			if (startAddr == null)
			{
				Console.WriteLine($"invalid start address {args[1]}");
				return;
			}

			string ext = Path.GetExtension(filename);
			string[] outfile;

			string outName = Path.Combine(Path.GetDirectoryName(filename),
					Path.GetFileNameWithoutExtension(filename)) + ".hex";

			if (ext == ".HEX")
			{
				string[] intelhex;
				try
				{
					intelhex = File.ReadAllLines(filename);
				}
				catch (Exception)
				{
					Console.WriteLine($"error reading '{filename}'.");
					return;
				}

				byte[] bin = hc.Hex2Bin(intelhex);
				outfile = hc.Bin2Hex(bin, startAddr.Value);
			}
			else
			{
				byte[] bindata;
				try
				{
					bindata = File.ReadAllBytes(filename);
				}
				catch (Exception)
				{
					Console.WriteLine($"error reading '{filename}'.");
					return;
				}

				outfile = hc.Bin2Hex(bindata, startAddr.Value);
			}

			if (outfile != null)
			{
				try
				{
					File.WriteAllLines(outName, outfile);
				}
				catch (Exception)
				{
					Console.WriteLine($"error reading '{filename}'.");
					return;
				}
			}
		}
	}
}
