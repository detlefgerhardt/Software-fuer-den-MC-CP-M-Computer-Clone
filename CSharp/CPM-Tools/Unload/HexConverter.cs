using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Security.AccessControl;
using System.Security.Cryptography;
using System.Text;

namespace Unload
{
	internal class HexConverter
	{
		/// <summary>
		/// Returns a byte-array that contains the data of the IntelHex file from the lowest to
		/// the heighest address.
		/// </summary>
		/// <param name="filename"></param>
		/// <returns></returns>
		public byte[] Hex2Bin(string[] intelhex)
		{
			byte[] data = new byte[0x10000];
			int highestAddr = 0;
			int lowestAddr = 0x10000;
			bool eof = false;
			for (int l = 0; l<intelhex.Length && !eof; l++)
			{
				string line = intelhex[l];

				// check for CP/M eof character
				int pos = line.IndexOf('\x1A');
				if (pos >= 0)
				{
					line = line.Substring(0, pos);
					eof = true;
				}

				line = line.TrimEnd();

				if (line == ":00000001FF") break;

				if (string.IsNullOrWhiteSpace(line)) continue; // skip empty lines

				if (line.Length < 11)
				{
					Console.WriteLine($"line to short in line {l+1}");
					return null;
				}
				if (line[0] != ':')
				{
					Console.WriteLine($"Error: missing ':' in {l+1}");
					return null;
				}

				// length
				int? n = GetHexVal(line.Substring(1, 2));
				if (n == null || line.Length != n * 2 + 11)
				{
					Console.WriteLine($"Error: wrong line length {n} in {l+1}");
					return null;
				}
				int? lineAdress = GetHexVal(line.Substring(3, 4));
				if (lineAdress == null)
				{
					Console.WriteLine($"Error: invalid address in {l+1}");
					return null;
				}

				int? recType = GetHexVal(line.Substring(7, 2));
				if (recType == null)
				{
					Console.WriteLine($"Error: invalid record type in {l+1}");
					return null;
				}
				int? chk = GetHexVal(line.Substring(line.Length - 2, 2));
				if (chk == null)
				{
					Console.WriteLine($"Error: invalid chksum in {l+1}");
					return null;
				}
				int chksum = n.Value + (lineAdress.Value >> 8) + (lineAdress.Value & 0xFF) + recType.Value;

				if (n<32)
				{
					Debug.Write("");
				}

				for (int i = 0; i < n; i++)
				{
					int addr = lineAdress.Value + i;

					if (addr >= 0x10000)
					{
						Console.WriteLine($"Error: out of range in {l+1}");
						return null;
					}
					int? val = GetHexVal(line.Substring(9 + i * 2, 2));
					if (val == null)
					{
						Console.WriteLine($"Error: invalid hex character in {l+1}");
						return null;
					}
					chksum += val.Value;

					data[addr] = (byte)val.Value;
					if (addr > highestAddr) highestAddr = addr;
					if (addr < lowestAddr) lowestAddr = addr;
				}

				chksum = CalcChecksum(chksum);
				if (chksum != chk)
				{
					Console.WriteLine($"Error: chksum mismatch in {l+1}");
					return null;
				}
			}

			byte[] newData = new byte[highestAddr - lowestAddr + 1];
			Buffer.BlockCopy(data, lowestAddr, newData, 0, newData.Length);
			return newData;
		}

		public string[] Bin2Hex(byte[] data, int startAddr)
		{
			const int BYTES_PER_LINE = 32;
			const int REC_TYPE = 0;

			List<string> intelhex = new List<string>();

			int hexaddr = startAddr;
			int binaddr = 0x0000;
			bool end = false;
			while (!end)
			{
				int byteCnt = BYTES_PER_LINE;
				for (int i = 0; i < BYTES_PER_LINE; i++)
				{
					if (binaddr + i >= data.Length)
					{
						byteCnt = i;
						end = true;
						break;
					}
				}
				if (byteCnt == 0) break;

				int chksum = byteCnt + (hexaddr >> 8) + (hexaddr & 0xFF) + REC_TYPE;
				StringBuilder sb = new StringBuilder();
				sb.Append($":{byteCnt:X02}{hexaddr:X04}{REC_TYPE:X02}");
				for (int i=0; i< byteCnt; i++)
				{
					sb.Append($"{data[binaddr + i]:X02}");
					chksum += data[binaddr + i];
				}
				chksum = CalcChecksum(chksum);
				sb.Append($"{chksum:X02}");

				intelhex.Add(sb.ToString());

				binaddr += byteCnt;
				hexaddr += byteCnt;
			}
			intelhex.Add(":00000001FF");

			return intelhex.ToArray();
		}


		public static int? GetHexVal(string hex)
		{
			int val;
			if (int.TryParse(hex, NumberStyles.HexNumber, CultureInfo.InvariantCulture, out val))
				return val;
			else
				return null;
		}

		public static int CalcChecksum(int checksum)
		{
			checksum %= 256;
			if (checksum > 0) checksum = 256 - checksum;
			return checksum;
		}
	}
}
