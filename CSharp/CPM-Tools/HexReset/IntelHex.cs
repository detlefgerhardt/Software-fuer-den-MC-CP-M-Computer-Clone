using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Text;

namespace HexReset
{
	internal class IntelHex
	{
		public void Convert(string inputname, string outputname)
		{
			string[] hexLines;
			try
			{
				if (!File.Exists(inputname))
				{
					Console.WriteLine($"'{inputname}' not found.");
					return;
				}

				hexLines = File.ReadAllLines(inputname);
			}
			catch(Exception)
			{
				Console.WriteLine($"error reading '{inputname}'.");
				return;
			}

			int lineNr = 0;
			int newAddr = 0;
			List<string> newLines = new List<string>();
			bool eof = false;
			for(int l=0; l<hexLines.Length; l++)
			{
				if (eof) break;

				lineNr++;
				string line = hexLines[l];

				// check for CP/M eof character
				int pos = line.IndexOf('\x1A');
				if (pos >= 0)
				{
					line = line.Substring(0, pos);
					eof = true;
				}

				if (line == ":00000001FF")
				{
					newLines.Add(line);
					continue;
				}

				if (string.IsNullOrWhiteSpace(line)) continue; // skip empty lines

				if (line.Length < 11)
				{
					Console.WriteLine($"line to short in line {lineNr}");
					return;
				}
				if (line[0] != ':')
				{
					Console.WriteLine($"Error: missing ':' in {lineNr}");
					return;
				}
				int? n = GetHexVal(line.Substring(1, 2));
				if (n == null || line.Length != n * 2 + 11)
				{
					Console.WriteLine($"Error: wrong line length in {lineNr}");
					return;
				}
				int? addr = GetHexVal(line.Substring(3, 4));
				if (addr == null)
				{
					Console.WriteLine($"Error: invalid address in {lineNr}");
					return;
				}

				int? recType = GetHexVal(line.Substring(7, 2));
				if (recType == null)
				{
					Console.WriteLine("Error: invalid record type in {lineNr}");
					return;
				}
				int? chk = GetHexVal(line.Substring(line.Length - 2, 2));
				if (chk == null)
				{
					Console.WriteLine($"Error: invalid chksum in {lineNr}");
					return;
				}
				int chksum = n.Value + (addr.Value >> 8) + (addr.Value & 0xFF) + recType.Value;

				int[] values = new int[n.Value];
				for (int i = 0; i < n; i++)
				{
					if (addr + i >= 0x10000)
					{
						Console.WriteLine($"Error: invalid address in {lineNr}");
						return;
					}
					int? val = GetHexVal(line.Substring(9 + i * 2, 2));
					if (val == null)
					{
						Console.WriteLine($"Error: invalid hex character in {lineNr}");
						return;
					}
					values[i] = val.Value;
					chksum += val.Value;
				}

				chksum = CalcChecksum(chksum);
				if (chksum != chk)
				{
					Console.WriteLine($"Error: chksum mismatch in {lineNr}");
					return;
				}

				int newChkSum = n.Value + (newAddr >> 8) + (newAddr & 0xFF) + recType.Value;

				StringBuilder newLine = new StringBuilder();
				newLine.Append($":{n:X02}{newAddr:X04}{recType:X02}");
				for(int i=0; i<n; i++)
				{
					newLine.Append($"{values[i]:X02}");
					newChkSum += values[i];
				}
				newLine.Append($"{CalcChecksum(newChkSum):X02}");
				newLines.Add(newLine.ToString());

				newAddr += n.Value;
			}

			try
			{
				File.Delete(outputname);
				File.WriteAllLines(outputname, newLines);
			}
			catch(Exception)
			{
				Console.WriteLine($"Error writing '{outputname}'.");
				return;
			}

			Console.WriteLine($"'{outputname}' written.");
		}

		private int? GetHexVal(string hex)
		{
			int val;
			if (int.TryParse(hex, NumberStyles.HexNumber, CultureInfo.InvariantCulture, out val))
				return val;
			else
				return null;
		}

		private int CalcChecksum(int checksum)
		{
			checksum %= 256;
			if (checksum > 0)
			{
				checksum = 256 - checksum;
			}
			return checksum;
		}
	}
}
